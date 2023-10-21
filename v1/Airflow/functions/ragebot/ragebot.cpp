#include "ragebot.h"
#include "../features.h"
#include "../../base/tools/threads.h"
#include "../../additionals/tinyformat.h"

#ifdef _DEBUG
#define DEBUG_LC 0
#define DEBUG_SP 0
#endif

void c_rage_bot::store_global(c_csplayer* player)
{
	auto& backup = this->backup_global[player->index()];
	backup.duck = player->duck_amount();
	backup.lby = player->lby();
	backup.angles = player->eye_angles();
	backup.origin = player->origin();
	backup.absorigin = player->get_abs_origin();
	backup.bbmin = player->bb_mins();
	backup.bbmax = player->bb_maxs();
	backup.velocity = player->velocity();
	backup.ground_entity = player->ground_entity();
	backup.collision_change_time = player->collision_change_time();
	backup.collision_change_origin = player->collision_change_origin();
	player->store_bone_cache(backup.bonecache);
	player->store_poses(backup.poses);

	backup.filled = true;
}

void c_rage_bot::restore_global(c_csplayer* player)
{
	auto& backup = this->backup_global[player->index()];
	if (!backup.filled)
		return;

	auto collideable = player->get_collideable();
	func_ptrs::set_collision_bounds(collideable, &backup.bbmin, &backup.bbmax);

	player->origin() = backup.origin;
	player->ground_entity() = backup.ground_entity;
	player->set_abs_origin(backup.origin);

	player->set_bone_cache(backup.bonecache);
	player->invalidate_bone_cache();
}

void c_rage_bot::store(c_csplayer* player)
{
	auto& backup = this->backup[player->index()];
	backup.duck = player->duck_amount();
	backup.lby = player->lby();
	backup.angles = player->eye_angles();
	backup.origin = player->origin();
	backup.absorigin = player->get_abs_origin();
	backup.bbmin = player->bb_mins();
	backup.bbmax = player->bb_maxs();
	backup.velocity = player->velocity();
	backup.ground_entity = player->ground_entity();
	backup.collision_change_time = player->collision_change_time();
	backup.collision_change_origin = player->collision_change_origin();
	player->store_bone_cache(backup.bonecache);
	player->store_poses(backup.poses);

	backup.filled = true;
}

void c_rage_bot::set_record(c_csplayer* player, records_t* record, matrix3x4_t* matrix)
{
	auto animation = g_animation_fix->get_animation_player(player->index());
	if (!animation)
		return;

	player->origin() = record->origin;
	player->ground_entity() = record->ground_entity;
	player->set_abs_origin(record->origin);

	auto mat = matrix ? matrix : record->sim_orig.bone;

	auto collideable = player->get_collideable();
	func_ptrs::set_collision_bounds(collideable, &record->mins, &record->maxs);

	player->set_bone_cache(mat);

	player->invalidate_bone_cache();
}

void c_rage_bot::restore(c_csplayer* player)
{
	auto& backup = this->backup[player->index()];
	if (!backup.filled)
		return;

	auto collideable = player->get_collideable();
	func_ptrs::set_collision_bounds(collideable, &backup.bbmin, &backup.bbmax);

	player->origin() = backup.origin;
	player->ground_entity() = backup.ground_entity;
	player->set_abs_origin(backup.origin);

	player->set_bone_cache(backup.bonecache);

	player->invalidate_bone_cache();
}

std::vector< int > c_rage_bot::get_hitboxes()
{
	std::vector< int > hitboxes{};

	if (g_ctx.weapon->is_taser())
	{
		hitboxes.emplace_back((int)hitbox_stomach);
		hitboxes.emplace_back((int)hitbox_pelvis);
		return hitboxes;
	}

	if (this->weapon_config.hitboxes & head)
		hitboxes.emplace_back((int)hitbox_head);

	if (this->weapon_config.hitboxes & chest)
	{
		hitboxes.emplace_back((int)hitbox_chest);
		hitboxes.emplace_back((int)hitbox_lower_chest);
		hitboxes.emplace_back((int)hitbox_upper_chest);
	}

	if (this->weapon_config.hitboxes & stomach)
		hitboxes.emplace_back((int)hitbox_stomach);

	if (this->weapon_config.hitboxes & pelvis)
		hitboxes.emplace_back((int)hitbox_pelvis);

	if (this->weapon_config.hitboxes & arms_)
	{
		hitboxes.emplace_back((int)hitbox_left_upper_arm);
		hitboxes.emplace_back((int)hitbox_right_upper_arm);
	}

	if (this->weapon_config.hitboxes & legs)
	{
		hitboxes.emplace_back((int)hitbox_left_thigh);
		hitboxes.emplace_back((int)hitbox_right_thigh);

		hitboxes.emplace_back((int)hitbox_left_calf);
		hitboxes.emplace_back((int)hitbox_right_calf);

		hitboxes.emplace_back((int)hitbox_left_foot);
		hitboxes.emplace_back((int)hitbox_right_foot);
	}

	return hitboxes;
}

float get_point_accuracy(c_csplayer* player, point_t& point)
{
	if (cvars::weapon_accuracy_nospread->get_int())
		return 1.f;

	auto hits = 0;

	auto angle = math::normalize(math::angle_from_vectors(g_ctx.eye_position, point.position), true);

	vector3d forward, right, up;
	math::angle_to_vectors(angle, forward, right, up);

	for (auto i = 1; i <= 6; ++i)
	{
		for (auto j = 0; j < 8; ++j)
		{
			auto current_spread = g_ctx.spread * ((float)i / 6.f);

			float value = (float)j / 8.0f * (M_PI * 2.f);

			auto direction_cos = std::cos(value);
			auto direction_sin = std::sin(value);

			auto spread_x = direction_sin * current_spread;
			auto spread_y = direction_cos * current_spread;

			vector3d direction{};
			direction.x = forward.x + spread_x * right.x + spread_y * up.x;
			direction.y = forward.y + spread_x * right.y + spread_y * up.y;
			direction.z = forward.z + spread_x * right.z + spread_y * up.z;

			auto end = g_ctx.eye_position + direction * g_ctx.weapon_info->range;

			if (rage_tools::can_hit_hitbox(g_ctx.eye_position, point.position, player, point.hitbox, point.record, point.record->sim_orig.bone))
				hits++;
		}
	}

	return (float)hits / 48.f;
}

int c_rage_bot::get_safety_count(c_csplayer* player, point_t& point)
{
	//draw_hitbox(player, point.record->sim_left.roll_bone, 0, 1, true);
	//draw_hitbox(player, point.record->sim_left.bone, 1, 1, true);
	//draw_hitbox(player, point.record->sim_right.bone, 1, 0, true);
	//draw_hitbox(player, point.record->sim_right.roll_bone, 0, 0, true);

	if (!rage_tools::can_hit_hitbox(g_ctx.eye_position, point.position, player, point.hitbox, point.record, point.record->sim_orig.bone))
		return -1;

	auto safety = 0;
	matrix3x4_t* matrices[]
	{
		point.record->sim_left.roll_bone,
		point.record->sim_left.bone,

		point.record->sim_right.roll_bone,
		point.record->sim_right.bone,

		point.record->sim_zero.bone,
	};

	for (int i = 0; i < 5; ++i)
	{
		if (rage_tools::can_hit_hitbox(g_ctx.eye_position, point.position, player, point.hitbox, point.record, matrices[i]))
			++safety;
	}

	return safety;
}

int c_rage_bot::get_min_damage(c_csplayer* player)
{
	int health = player->health();

	int menu_damage = g_cfg.binds[override_dmg_b].toggled ? this->weapon_config.damage_override : this->weapon_config.mindamage;

	// hp + 1 slider
	if (menu_damage > 99)
		return health + (menu_damage - 100);

	return menu_damage;
}

bool c_rage_bot::should_stop(bool shoot_check)
{
	if (!(this->weapon_config.quick_stop))
		return false;

	if (g_ctx.weapon->is_misc_weapon())
		return false;

	if (!g_utils->on_ground() && !(this->weapon_config.quick_stop_options & in_air))
		return false;

	if (g_ctx.local->velocity().length(true) < 1.f)
		return false;

	if (g_cfg.binds[sw_b].toggled)
		return false;

	bool able_to_shoot = g_utils->is_able_to_shoot(true);

	if (shoot_check)
	{
		bool between_shots_ = this->weapon_config.quick_stop_options & between_shots;
		if (!able_to_shoot)
			return between_shots_;
	}

	return able_to_shoot;
}

void c_rage_bot::start_stop()
{
	if (!g_ctx.weapon || !this->stopping)
		return;

	if (this->pred_stopping && (this->weapon_config.quick_stop_options & early))
	{
		if (this->should_stop(false))
			this->auto_stop();

		if (g_exploits->cl_move.trigger && g_exploits->cl_move.shifting)
			g_engine_prediction->repredict(g_ctx.local, g_ctx.cmd);

		return;
	}

	if (this->should_stop())
		this->force_accuracy = this->auto_stop();

	if (g_exploits->cl_move.trigger && g_exploits->cl_move.shifting)
		g_engine_prediction->repredict(g_ctx.local, g_ctx.cmd);
}

bool c_rage_bot::auto_stop()
{
	auto velocity = g_ctx.local->velocity();
	float raw_speed = velocity.length(true);

	int max_speed = (int)g_movement->get_max_speed() * 0.34f;
	int speed = (int)raw_speed;

	if (speed <= max_speed)
	{
		g_movement->force_speed(g_movement->get_max_speed() * 0.34f);
		return true;
	}

	game_movement::force_stop();

	return !(this->weapon_config.quick_stop_options & 4);
}

bool c_rage_bot::knife_is_behind(records_t* record)
{
	vector3d delta{ record->origin - g_ctx.eye_position };
	delta.z = 0.f;
	delta = delta.normalized();

	vector3d target;
	math::angle_to_vectors(record->abs_angles, target);
	target.z = 0.f;

	return delta.dot(target) > 0.475f;
}

bool c_rage_bot::knife_trace(vector3d dir, bool stab, c_game_trace* trace)
{
	float range = stab ? 32.f : 48.f;

	vector3d start = g_ctx.eye_position;
	vector3d end = start + (dir * range);

	c_trace_filter filter{};
	filter.skip = g_ctx.local;
	interfaces::engine_trace->trace_ray(ray_t(start, end), mask_solid, &filter, trace);

	// if the above failed try a hull trace.
	if (trace->fraction >= 1.f) 
	{
		interfaces::engine_trace->trace_ray(ray_t(start, end, { -16.f, -16.f, -18.f }, { 16.f, 16.f, 18.f }), mask_solid, &filter, trace);
		return trace->fraction < 1.f;
	}

	return true;
}

bool c_rage_bot::can_knife(records_t* record, vector3d angle, bool& stab) 
{
	// convert target angle to direction.
	vector3d forward{};
	math::angle_to_vectors(angle, forward);

	// see if we can hit the player with full range
	// this means no stab.
	c_game_trace trace{};
	knife_trace(forward, false, &trace);

	// we hit smthing else than we were looking for.
	if (!trace.entity || trace.entity != record->ptr)
		return false;

	bool armor = record->ptr->armor_value() > 0;
	bool first = g_ctx.weapon->next_primary_attack() + 0.4f < g_ctx.predicted_curtime;
	bool back = this->knife_is_behind(record);

	int stab_dmg = knife_dmg.stab[armor][back];
	int slash_dmg = knife_dmg.swing[first][armor][back];
	int swing_dmg = knife_dmg.swing[false][armor][back];

	int health = record->ptr->health();
	if (health <= slash_dmg)
		stab = false;
	else if (health <= stab_dmg)
		stab = true;
	else if (health > (slash_dmg + swing_dmg + stab_dmg))
		stab = true;
	else
		stab = false;

	// damage wise a stab would be sufficient here.
	if (stab && !this->knife_trace(forward, true, &trace))
		return false;

	return true;
}

std::vector<int> backtrack_hitboxes
{
	hitbox_head,
	hitbox_pelvis,
	hitbox_chest,
};

int get_record_damage(c_csplayer* player, records_t* record)
{
	int total_dmg = 0;

	g_rage_bot->store(player);
	g_rage_bot->set_record(player, record);

	int broken = 0;

	for (auto& hitbox : backtrack_hitboxes)
	{
		auto position = player->get_hitbox_position(hitbox, record->sim_orig.bone);
		auto awall = g_auto_wall->fire_bullet(g_ctx.local, player, g_ctx.weapon_info,
			g_ctx.weapon->is_taser(), g_ctx.eye_position, position);

		if (awall.dmg < 0)
		{
			if (++broken >= backtrack_hitboxes.size() / 2)
				break;
		}

		total_dmg += awall.dmg;
	}

	g_rage_bot->restore(player);

	return total_dmg;
}

records_t* get_best_record(c_csplayer* player)
{
	if (g_ctx.lagcomp)
	{
		auto anim_player = g_animation_fix->get_animation_player(player->index());
		if (anim_player->records.empty())
			return nullptr;

		int highest_damage = INT_MIN;
		records_t* best_record = nullptr;

		for (auto& i : anim_player->records)
		{
			if (!i.valid_tick)
				continue;

			auto damage = get_record_damage(player, &i);
			if (damage > highest_damage)
			{
				highest_damage = damage;
				best_record = &i;
			}
		}

		return best_record;
	}

	return g_animation_fix->get_latest_record(player);
}

void thread_predict_eye_pos()
{
	constexpr auto max_ticks = 2;

	auto velocity = g_engine_prediction->unprediced_velocity;
	float speed = std::max<float>(velocity.length(true), 1.f);

	int max_stop_ticks = std::max<int>(((speed / g_movement->get_max_speed()) * 7.f) - 1, 0);
	max_stop_ticks = std::clamp(max_stop_ticks, 0, max_ticks);

	if (max_stop_ticks == 0)
	{
		g_rage_bot->predictive_stop_ticks = 0;
		g_rage_bot->predicted_eye_pos = g_ctx.eye_position;
		return;
	}

	g_rage_bot->predictive_stop_ticks = max_stop_ticks;

	vector3d last_predicted_velocity = velocity;
	for (int i = 0; i < max_stop_ticks; ++i)
	{
		auto pred_velocity = velocity * math::ticks_to_time(i + 1);

		vector3d origin = g_ctx.eye_position + pred_velocity;
		int flags = g_ctx.local->flags();

		g_utils->extrapolate(g_ctx.local, origin, pred_velocity, flags, flags & fl_onground);

		last_predicted_velocity = pred_velocity;
	}

	g_rage_bot->predicted_eye_pos = g_ctx.eye_position + last_predicted_velocity;
}

inline bool is_point_predictive(c_csplayer* player, point_t& point)
{
	thread_predict_eye_pos();

	if (!g_rage_bot->should_stop(false) || g_rage_bot->predictive_stop_ticks == 0)
		return false;

	int dmg = g_rage_bot->get_min_damage(player);
	return g_auto_wall->can_hit_point(player, g_ctx.local, point.position, g_rage_bot->predicted_eye_pos, dmg);
}

struct zeus_point_t
{
	int hitbox{};
	vector3d point{};
	pen_data_t data{};
};

void calculate_body_damages(zeus_point_t& zeus_point, c_csplayer* player)
{
	zeus_point.data = g_auto_wall->fire_bullet(g_ctx.local, player, g_ctx.weapon_info, true, g_ctx.eye_position, zeus_point.point);
}

void c_rage_bot::zeus_bot()
{
	if (g_cfg.legit.enable || !g_cfg.rage.enable)
		return;

	if (g_ctx.weapon->item_definition_index() != weapon_taser)
		return;

	auto nearest_player = g_anti_aim->get_closest_player(true);
	if (!nearest_player)
		return;

	auto ideal_record = get_best_record(nearest_player);
	if (!ideal_record)
		return;

	if (g_ctx.predicted_curtime < g_ctx.weapon->next_primary_attack() || g_ctx.predicted_curtime < g_ctx.weapon->next_secondary_attack())
		return;

	this->store(nearest_player);
	{
		this->set_record(nearest_player, ideal_record);

		float highest_damage = FLT_MIN;
		point_t best_point{};

		std::vector<std::uint64_t> tasks{};

		std::vector<zeus_point_t> points{};
		points.reserve(30);

		for (int i = hitbox_pelvis; i <= hitbox_chest; ++i)
		{
			auto multipoints = rage_tools::get_multipoints(nearest_player, i, ideal_record->sim_orig.bone);

			for (auto& point : multipoints)
			{
				auto& new_point = points.emplace_back();
				new_point.point = point.first;
				new_point.hitbox = i;

				tasks.emplace_back(g_thread_pool->add_task(calculate_body_damages, std::ref(new_point), nearest_player));
			}
		}

		for (auto& task : tasks)
			g_thread_pool->wait(task);

		for (auto& point : points)
		{
			auto& autowall = point.data;
			if ((float)(autowall.dmg * 0.7f) < (float)nearest_player->health())
				continue;

			if (autowall.dmg > highest_damage)
			{
				highest_damage	= autowall.dmg;
				best_point		= point_t{ point.hitbox, false, autowall.dmg, ideal_record, point.point };
			}
		}

		if (best_point.position.valid())
		{
		//	interfaces::debug_overlay->add_text_overlay(best_point.position, 0.1f, "%d", best_point.damage);

			auto aim_angle = math::angle_from_vectors(g_ctx.eye_position, best_point.position)
				- g_ctx.local->aim_punch_angle() * cvars::weapon_recoil_scale->get_float();

			g_ctx.eye_position = g_engine_prediction->get_eye_pos(aim_angle);

			bool able_to_shoot = g_utils->is_able_to_shoot(true);
			bool accuracy_valid = rage_tools::is_accuracy_valid(nearest_player, best_point, 0.6f, &best_point.hitchance);
			if (accuracy_valid && able_to_shoot && this->last_shot_cmd != g_ctx.cmd->command_number)
			{
				if (g_ctx.lagcomp)
					g_ctx.cmd->tickcount = math::time_to_ticks(best_point.record->sim_time + g_ctx.lerp_time);

				g_ctx.cmd->viewangles = math::normalize(aim_angle, true);

				if (g_cfg.rage.auto_fire)
				{
					if (!g_anti_aim->is_fake_ducking() && g_cfg.binds[hs_b].toggled)
						*g_ctx.send_packet = true;

					g_ctx.cmd->buttons |= in_attack;
				}

				if (g_ctx.cmd->buttons & in_attack)
				{
					this->firing = true;

					g_ctx.shot_cmd = g_ctx.cmd->command_number;
					g_ctx.last_shoot_position = g_ctx.eye_position;

					if (g_cfg.visuals.chams[c_onshot].enable)
						g_chams->add_shot_record(nearest_player, best_point.record->sim_orig.bone);

					this->last_shot_cmd = g_ctx.cmd->command_number;

#ifdef _DEBUG
#if DEBUG_LC
					draw_hitbox(target, best_point.record->sim_orig.bone, 0, 0, false);
#endif

#if DEBUG_SP
					main_utils::draw_hitbox(target, best_point.record->sim_left.bone, 0, 0, false);
					main_utils::draw_hitbox(target, best_point.record->sim_right.bone, 1, 0, false);
					main_utils::draw_hitbox(target, best_point.record->sim_zero.bone, 0, 1, false);
#endif
#endif
				}
			}
		}
	}
	this->restore(nearest_player);
}

void c_rage_bot::knife_bot()
{
	if (g_cfg.legit.enable || !g_cfg.rage.enable)
		return;

	if (!g_ctx.weapon->is_knife())
		return;

	if (g_ctx.predicted_curtime < g_ctx.weapon->next_primary_attack() || g_ctx.predicted_curtime < g_ctx.weapon->next_secondary_attack())
		return;

	auto nearest_player = g_anti_aim->get_closest_player(true);
	if (!nearest_player)
		return;

	if (g_ctx.predicted_curtime < g_ctx.weapon->next_primary_attack() || g_ctx.predicted_curtime < g_ctx.weapon->next_secondary_attack())
		return;

	knife_point_t point{};

	if (g_ctx.lagcomp)
	{
		auto anim_player = g_animation_fix->get_animation_player(nearest_player->index());
		if (anim_player->records.empty())
			return;

		bool found = false;
		for (auto& i : anim_player->records)
		{
			if (!i.valid_tick)
				continue;

			this->store(nearest_player);
			{
				this->set_record(nearest_player, &i);
				{
					for (const auto& a : knife_ang)
					{
						if (this->can_knife(&i, a, point.stab))
						{
							point.angle = a;
							point.record = &i;
							this->restore(nearest_player);
							found = true;
							break;
						}
					}
				}
			}
			this->restore(nearest_player);

			if (found)
				break;
		}

		if (!point.record)
			return;

		g_ctx.cmd->buttons |= point.stab ? in_attack2 : in_attack;

		if (g_ctx.cmd->buttons & (point.stab ? in_attack2 : in_attack) && this->last_shot_cmd != g_ctx.cmd->command_number)
		{
			g_ctx.cmd->viewangles = math::normalize(point.angle, true);

			if (g_ctx.lagcomp)
				g_ctx.cmd->tickcount = math::time_to_ticks(point.record->sim_time + g_ctx.lerp_time);

			this->firing = true;

			g_ctx.shot_cmd = g_ctx.cmd->command_number;
			g_ctx.last_shoot_position = g_ctx.eye_position;

			if (g_cfg.visuals.chams[c_onshot].enable)
				g_chams->add_shot_record(nearest_player, point.record->sim_orig.bone);

			this->last_shot_cmd = g_ctx.cmd->command_number;
		}
	}
	else
	{
		auto last = g_animation_fix->get_latest_record(nearest_player);

		this->store(nearest_player);
		{
			this->set_record(nearest_player, last);
			{
				for (const auto& a : knife_ang)
				{
					if (this->can_knife(last, a, point.stab))
					{
						point.angle = a;
						point.record = last;
						this->restore(nearest_player);
						break;
					}
				}
			}
		}
		this->restore(nearest_player);

		if (!point.record)
			return;

		g_ctx.cmd->buttons |= point.stab ? in_attack2 : in_attack;

		if (g_ctx.cmd->buttons & (point.stab ? in_attack2 : in_attack) && this->last_shot_cmd != g_ctx.cmd->command_number)
		{
			g_ctx.cmd->viewangles = math::normalize(point.angle, true);

			if (g_ctx.lagcomp)
				g_ctx.cmd->tickcount = math::time_to_ticks(point.record->sim_time + g_ctx.lerp_time);

			this->firing = true;

			g_ctx.shot_cmd = g_ctx.cmd->command_number;
			g_ctx.last_shoot_position = g_ctx.eye_position;

			if (g_cfg.visuals.chams[c_onshot].enable)
				g_chams->add_shot_record(nearest_player, point.record->sim_orig.bone);

			this->last_shot_cmd = g_ctx.cmd->command_number;
		}
	}
}

void force_scope()
{
	bool able_to_zoom = g_ctx.predicted_curtime >= g_ctx.weapon->next_secondary_attack();

	if (able_to_zoom && g_rage_bot->weapon_config.auto_scope && g_ctx.weapon->zoom_level() < 1 && g_utils->on_ground() && g_ctx.weapon->is_sniper())
		g_ctx.cmd->buttons |= in_attack2;
}

void thread_build_points(aim_cache_t* cache)
{
	cache->points.clear();

	if (!cache->player)
		return;

	auto best_record = get_best_record(cache->player);
	if (!best_record)
		return;

	g_rage_bot->store(cache->player);
	g_rage_bot->set_record(cache->player, best_record);

	for (auto& hitbox : g_rage_bot->get_hitboxes())
	{
		const auto& pts = rage_tools::get_multipoints(cache->player, hitbox, best_record->sim_orig.bone);
		for (auto& p : pts)
		{
			auto awall = g_auto_wall->fire_bullet(g_ctx.local, cache->player, g_ctx.weapon_info,
				g_ctx.weapon->is_taser(), g_ctx.eye_position, p.first);
			auto new_point = point_t(hitbox, p.second, awall.dmg, best_record, p.first);

			if (p.second)
				new_point.predictive = is_point_predictive(cache->player, new_point);

			new_point.accuracy = get_point_accuracy(cache->player, new_point);
			new_point.safety = g_rage_bot->get_safety_count(cache->player, new_point);

			cache->points.emplace_back(new_point);

#ifdef _DEBUG
			if (g_rage_bot->debug_aimbot)
			{
				interfaces::debug_overlay->add_box_overlay(
					new_point.position, vector3d(-1, -1, -1), vector3d(1, 1, 1), {}, 255, new_point.center ? 255 : 0, new_point.center ? 255 : 0, 200, interfaces::global_vars->interval_per_tick * 2.f);

				interfaces::debug_overlay->add_text_overlay(new_point.position, interfaces::global_vars->interval_per_tick * 2.f, "%d", new_point.damage);
			}
#endif
		}
	}

	g_rage_bot->restore(cache->player);
}

void thread_get_best_point(aim_cache_t* aim_cache)
{
	if (!aim_cache->player)
		return;

	auto weapon_config = g_rage_bot->weapon_config;

	int health = aim_cache->player->health();
	int lethal_dmg = std::max(0, health - 5);

	aim_cache->best_point.reset();

	if (aim_cache->points.empty())
		return;

	auto prefer_baim_on_dt = g_exploits->enabled() && g_exploits->get_exploit_mode() == exploits_dt
		&& (g_ctx.weapon->is_auto_sniper() || g_ctx.weapon->is_heavy_pistols());

	int dmg = g_rage_bot->get_min_damage(aim_cache->player);

	std::sort(aim_cache->points.begin(), aim_cache->points.end(), [&](point_t& a, point_t& b) { return a.accuracy > b.accuracy; });
	std::sort(aim_cache->points.begin(), aim_cache->points.end(), [&](point_t& a, point_t& b) { return a.center > b.center; });
	std::sort(aim_cache->points.begin(), aim_cache->points.end(), [&](point_t& a, point_t& b) { return a.damage > b.damage; });
	
	static int call_count{};
	for (auto& i : aim_cache->points)
	{
		if (i.predictive)
		{
			if (g_rage_bot->weapon_config.quick_stop_options & early)
			{
				force_scope();
				
				g_rage_bot->pred_stopping = true;
				g_rage_bot->stopping = true;
			}

			i.predictive = false;
		}

		call_count = 0;

		if (i.safety == -1 || i.damage < dmg || g_cfg.binds[force_body_b].toggled && !i.body || g_cfg.binds[force_sp_b].toggled && i.safety != 5)
			continue;

		if (weapon_config.prefer_safe && i.safety == 5)
		{
			aim_cache->best_point = i;
			break;
		}
		else if (i.body && (i.damage >= health || weapon_config.prefer_body || prefer_baim_on_dt))
		{
			aim_cache->best_point = i;
			break;
		}
		else
		{
			if (aim_cache->best_point.damage < i.damage)
				aim_cache->best_point = i;
		}
	}
}

void c_rage_bot::predict_eye_pos()
{
	

	//interfaces::debug_overlay->add_text_overlay(predicted_eye_pos, 0.1f, "PRED");
}

void c_rage_bot::proceed_aimbot()
{
#ifdef _DEBUG
	if (rage_tools::debug_hitchance)
	{
		rage_tools::spread_point.reset();
		rage_tools::current_spread = 0.f;
		rage_tools::spread_points.clear();
	}
#endif

	this->target = nullptr;
	this->predictive_stop_ticks = 0;
//	this->firing = false;
	this->working = false;
	this->stopping = false;
	this->pred_stopping = false;
	this->reset_data = false;
	this->force_accuracy = true;
	this->predicted_eye_pos.reset();
	this->weapon_config = {};

	if (!g_ctx.weapon || interfaces::game_rules->is_freeze_time() || g_ctx.local->flags() & fl_frozen || g_ctx.local->gun_game_immunity())
	{
		return;
	}

	this->zeus_bot();
	this->knife_bot();

	this->weapon_config = rage_tools::get_weapon_config();

	bool invalid_weapon = g_ctx.weapon->is_misc_weapon();

	if (g_cfg.legit.enable || !g_cfg.rage.enable || invalid_weapon)
	{
		if (this->reset_scan_data)
		{
			for (auto& b : backup)
				b.reset();

			this->target = nullptr;
			this->reset_scan_data = false;
		}

		return;
	}

	this->reset_scan_data = true;

	auto& players = g_listener_entity->get_entity(ent_player);
	if (players.empty())
		return;

	auto thread_task = g_thread_pool->add_task(thread_predict_eye_pos);
	g_thread_pool->wait(thread_task);

	float hitchance = std::clamp(this->weapon_config.hitchance / 100.f, 0.f, 1.f);

	point_t best_point{};

	std::vector<std::uint64_t> tasks{};

	int index_iter = 0;
	for (auto& player : players)
	{
		auto entity = (c_csplayer*)player.entity;
		if (!entity)
			continue;

		if (entity == g_ctx.local || entity->team() == g_ctx.local->team())
			continue;

		auto& cache = this->aim_cache[entity->index()];

		if (!entity->is_alive() || entity->dormant() || entity->gun_game_immunity())
		{
			this->target = nullptr;

			if (!cache.points.empty())
				cache.points.clear();

			if (cache.best_point.filled)
				cache.best_point.reset();

			if (cache.player)
				cache.player = nullptr;

			continue;
		}

		index_iter++;

		cache.player = entity;

		tasks.emplace_back(g_thread_pool->add_task(thread_build_points, &cache));
	}

	if (index_iter < 1)
		return;

	for (auto& task : tasks)
		g_thread_pool->wait(task);

	tasks.clear();

	for (auto& player : players)
	{
		auto entity = (c_csplayer*)player.entity;
		if (!entity || entity == g_ctx.local)
			continue;

		if (entity->team() == g_ctx.local->team() || !entity->is_alive() || entity->dormant() || entity->gun_game_immunity())
			continue;

		auto& cache = this->aim_cache[entity->index()];
		if (!cache.player || cache.player != entity)
			continue;

		thread_get_best_point(&cache);
	}

	float lowest_distance = FLT_MAX;

	should_slide = false;

	for (auto& player : players)
	{
		auto entity = (c_csplayer*)player.entity;
		if (!entity || entity == g_ctx.local)
			continue;

		if (entity->team() == g_ctx.local->team() || !entity->is_alive() || entity->dormant() || entity->gun_game_immunity())
			continue;

		auto cache = &this->aim_cache[entity->index()];
		if (!cache || !cache->player)
			continue;

		if (cache->player != entity || !cache->best_point.filled)
		{
			this->restore(entity);
			continue;
		}

		float dist = g_ctx.local->origin().dist_to(entity->origin());
		if (lowest_distance > dist)
		{
			this->target = entity;
			lowest_distance = dist;
		}
	}

	if (this->target)
		best_point = this->aim_cache[this->target->index()].best_point;

	if (best_point.filled)
	{
		this->working = true;

		this->pred_stopping = false;
		this->stopping = true;

		this->start_stop();
		g_engine_prediction->repredict(g_ctx.local, g_ctx.cmd);

		force_scope();

		auto aim_angle = math::angle_from_vectors(g_ctx.eye_position, best_point.position)
			- g_ctx.local->aim_punch_angle() * cvars::weapon_recoil_scale->get_float();

		//	g_ctx.eye_position = g_engine_prediction->get_eye_pos(aim_angle);

		bool supress_doubletap_choke = true;
		if (g_exploits->enabled() && g_exploits->get_exploit_mode() == exploits_dt)
			supress_doubletap_choke = g_exploits->defensive.tickbase_choke > 2;

		auto hc_thread_id = g_thread_pool->add_task(rage_tools::is_accuracy_valid, this->target, best_point, hitchance, &best_point.hitchance);
		auto hc_result = std::any_cast<bool>(g_thread_pool->wait_result(hc_thread_id));

		bool able_to_shoot = g_utils->is_able_to_shoot(true);
		bool accuracy_valid = this->force_accuracy && hc_result;

		auto lagcomp_simtime = math::time_to_ticks(best_point.record->sim_time);
		if (supress_doubletap_choke && accuracy_valid && able_to_shoot && this->last_shot_cmd != g_ctx.cmd->command_number)
		{
			g_ctx.cmd->viewangles = math::normalize(aim_angle, true);

		//	interfaces::engine->set_view_angles(g_ctx.cmd->viewangles);

			if (g_cfg.rage.auto_fire)
				g_ctx.cmd->buttons |= in_attack;

			if (g_ctx.cmd->buttons & in_attack)
			{
				if (!g_anti_aim->is_fake_ducking())
					*g_ctx.send_packet = true;

				if (g_ctx.lagcomp)
					g_ctx.cmd->tickcount = lagcomp_simtime + math::time_to_ticks(g_ctx.lerp_time);

				this->firing = true;

				g_ctx.shot_cmd = g_ctx.cmd->command_number;
				g_ctx.last_shoot_position = g_ctx.eye_position;

#if ALPHA || BETA || _DEBUG
				//interfaces::debug_overlay->add_text_overlay(best_point.position, 4.f, "%d HP", best_point.damage);
#endif

				if (g_cfg.visuals.chams[c_onshot].enable)
					g_chams->add_shot_record(this->target, best_point.record->sim_orig.bone);

				this->add_shot_record(this->target, best_point);
				this->last_shot_cmd = g_ctx.cmd->command_number;

#ifdef _DEBUG
#if DEBUG_LC
				draw_hitbox(target, best_point.record->sim_orig.bone, 0, 0, false);
#endif

#if DEBUG_SP
				main_utils::draw_hitbox(target, best_point.record->sim_left.bone, 0, 0, false);
				main_utils::draw_hitbox(target, best_point.record->sim_left.roll_bone, 1, 1, false);
				main_utils::draw_hitbox(target, best_point.record->sim_right.bone, 1, 0, false);
				main_utils::draw_hitbox(target, best_point.record->sim_right.roll_bone, 1, 1, false);
				main_utils::draw_hitbox(target, best_point.record->sim_zero.bone, 0, 1, false);
#endif
#endif
			}

			this->restore(this->target);
		}
	}
}

void c_rage_bot::on_cm_start()
{
	auto& players = g_listener_entity->get_entity(ent_player);
	if (players.empty())
		return;

	for (auto& player : players)
	{
		auto entity = (c_csplayer*)player.entity;
		if (!entity)
			continue;

		if (entity == g_ctx.local || entity->team() == g_ctx.local->team())
			continue;

		if (!entity->is_alive() || entity->dormant())
			continue;

		this->store_global(entity);
	}
}

void c_rage_bot::on_cm_end()
{
	auto& players = g_listener_entity->get_entity(ent_player);
	if (players.empty())
		return;

	for (auto& player : players)
	{
		auto entity = (c_csplayer*)player.entity;
		if (!entity)
			continue;

		if (entity == g_ctx.local || entity->team() == g_ctx.local->team())
			continue;

		if (!entity->is_alive() || entity->dormant())
			continue;

		this->restore_global(entity);
	}
}

void c_rage_bot::on_predict_start()
{
	this->proceed_aimbot();
}

void c_rage_bot::on_local_death()
{
	if (this->reset_data)
		return;

	for (auto& i : this->aim_cache)
		i.reset();

	for (auto& j : this->backup_global)
		j.reset();

	for (auto& x : this->backup)
		x.reset();

	for (auto& m : this->missed_shots)
		m = 0;

	this->reset_data = true;
}

void c_rage_bot::on_changed_map()
{
	for (auto& i : this->aim_cache)
		i.reset();

	for (auto& j : this->backup_global)
		j.reset();

	for (auto& x : this->backup)
		x.reset();

	for (auto& m : this->missed_shots)
		m = 0;
}