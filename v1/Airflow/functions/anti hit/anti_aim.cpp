#include "anti_aim.h"
#include "exploits.h"

#include "../config_vars.h"

#include "../features.h"

#include "../../base/tools/render.h"
#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"
#include "../../base/tools/threads.h"

#include "../extra/cmd_shift.h"

//void draw_debug_angle(float yaw)
//{
//	if (!g_ctx.cmd)
//		return;
//
//	auto angle_line = [&](float ang, int idx = 0, int idx2 = 0)
//	{
//		vector3d src = g_ctx.local->get_render_origin();
//		vector3d forward = {};
//		vector2d sc1, sc2;
//
//		math::angle_to_vectors(vector3d(0, ang, 0), forward);
//		interfaces::debug_overlay->add_line_overlay(src, src + (forward * 40.f), 255, 255 * idx, 255 * idx2, false, interfaces::global_vars->interval_per_tick * 2.f);
//	};
//
//	//angle_line(g_ctx.base_angle.y);
//	angle_line(g_ctx.cmd->viewangles.y);
//	angle_line(g_local_animation_fix->get_updated_netvars()->foot_yaw, 1);
//	angle_line(yaw, 1, 1);
//	// DrawAngle(g_local_animation_fix->local_info.m_abs, 1);
//}

c_csplayer* c_anti_aim::get_closest_player(bool skip, bool local_distance)
{
	auto& player_array = g_listener_entity->get_entity(ent_player);
	if (player_array.empty())
		return nullptr;

	c_csplayer* best = nullptr;
	float best_dist = FLT_MAX;

	vector2d center = vector2d(g_render->screen_size.w * 0.5f, g_render->screen_size.h * 0.5f);

	vector3d view_angles{};
	interfaces::engine->get_view_angles(view_angles);

	vector3d local_eye_pos{};
	//if (g_rage_bot->predicted_eye_pos.valid())
	//	local_eye_pos = g_rage_bot->predicted_eye_pos;
	//else
		local_eye_pos = g_ctx.eye_position;

	for (const auto& player_info : player_array)
	{
		auto player = (c_csplayer*)player_info.entity;
		if (!player)
			continue;

		if (!player->is_alive() || player->gun_game_immunity())
			continue;

		if (player == g_ctx.local || player->team() == g_ctx.local->team())
			continue;

		auto& esp_info = g_esp_store->playerinfo[player->index()];
		if (skip)
		{
			if (player->dormant())
				continue;
		}
		else
		{
			if (!esp_info.valid)
				continue;
		}

		auto base_origin = player->get_abs_origin();
		base_origin += vector3d(0.f, 0.f, player->view_offset().z / 2.f);

		vector2d origin = {};
		g_render->world_to_screen(base_origin, origin);

		auto angle = math::angle_from_vectors(local_eye_pos, base_origin);

		float dist = local_distance ? math::get_fov(view_angles, angle) : center.dist_to(origin);
		if (dist < best_dist)
		{
			best = player;
			best_dist = dist;
		}
	}

	return best;
}

vector3d get_predicted_pos()
{
	const int max_ticks = 13;
	const auto& velocity = g_engine_prediction->unprediced_velocity;

	float speed = std::max< float >(velocity.length(true), 1.f);
	int max_stop_ticks = std::max< int >(((speed / g_movement->get_max_speed()) * 5.f) - 1, 0);
	int max_predict_ticks = std::clamp(max_ticks - max_stop_ticks, 0, max_ticks);
	if (max_predict_ticks == 0)
		return {};

	vector3d last_predicted_velocity = g_engine_prediction->unprediced_velocity;
	for (int i = 0; i < max_predict_ticks; ++i)
	{
		auto pred_velocity = g_engine_prediction->unprediced_velocity * math::ticks_to_time(i + 1);

		vector3d local_origin = g_ctx.eye_position + pred_velocity;
		int flags = g_ctx.local->flags();

		g_utils->extrapolate(g_ctx.local, local_origin, pred_velocity, flags, flags & fl_onground);

		last_predicted_velocity = pred_velocity;
	}

	return last_predicted_velocity;
}

bool c_anti_aim::is_peeking()
{
	auto updated_vars = g_local_animation_fix->get_updated_netvars();

	if (!updated_vars || !g_ctx.local || !g_ctx.weapon || !g_ctx.weapon_info || !updated_vars->foot_yaw || !g_ctx.in_game || interfaces::client_state->delta_tick == -1)
		return false;

	if (!g_utils->is_able_to_shoot() || g_utils->is_firing())
		return false;

	auto& local_cache = g_ctx.local->bone_cache();
	if (!local_cache.base() || !local_cache.count())
		return false;

	auto player = this->get_closest_player();
	if (!player)
		return false;

	auto predicted_velocity = get_predicted_pos();
	auto predicted_eye_pos = g_ctx.eye_position + predicted_velocity;

	bool can_peek = false;
	auto& esp_info = g_esp_store->playerinfo[player->index()];
	if (!predicted_eye_pos.valid() || !esp_info.valid)
		return false;

	auto origin = player->dormant() ? esp_info.dormant_origin : player->get_abs_origin();

	static matrix3x4_t predicted_matrix[128]{};
	std::memcpy(predicted_matrix, local_cache.base(), sizeof(predicted_matrix));

	if (player->dormant())
	{
		vector3d poses[3]{ origin, origin + player->view_offset(), origin + vector3d(0.f, 0.f, player->view_offset().z / 2.f) };

		for (int i = 0; i < 3; ++i)
		{
			c_trace_filter filter{};
			filter.skip = g_ctx.local;

			c_game_trace out{};
			interfaces::engine_trace->trace_ray(ray_t(predicted_eye_pos, poses[i]), mask_shot_hull | contents_hitbox, &filter, &out);

			if (out.fraction >= 0.97f)
			{
				can_peek = true;
				break;
			}
		}

		if (can_peek)
			return true;
	}
	else
	{
		auto weapon = player->get_active_weapon();
		if (!weapon)
			return false;

		auto weapon_info = weapon->get_weapon_info();
		if (!weapon_info)
			return false;

		// detect if you can get dmg by enemy
		auto predicted_origin = g_ctx.local->origin() + predicted_velocity;
		math::change_matrix_position(predicted_matrix, 128, g_ctx.local->origin(), predicted_origin);
		{
			auto head_pos = g_ctx.local->get_hitbox_position(0, predicted_matrix);

			auto old_abs_origin = g_ctx.local->get_abs_origin();
			static matrix3x4_t old_cache[128]{};
			g_ctx.local->store_bone_cache(old_cache);
			{
				g_ctx.local->set_abs_origin(predicted_origin);
				g_ctx.local->set_bone_cache(predicted_matrix);

				for (auto& i : hitbox_list)
				{
					auto hitbox_position = g_ctx.local->get_hitbox_position(i);

					auto awall = g_auto_wall->fire_bullet(player, g_ctx.local, weapon_info, false, player->get_eye_position(), hitbox_position);
				//	interfaces::debug_overlay->add_text_overlay(hitbox_position, interfaces::global_vars->interval_per_tick * 2.f, "%d", awall.dmg);

					if (awall.dmg >= 1)
					{
						can_peek = true;
						break;
					}
				}
			}
			g_ctx.local->set_abs_origin(old_abs_origin);
			g_ctx.local->set_bone_cache(old_cache);
		}
		math::change_matrix_position(predicted_matrix, 128, predicted_origin, g_ctx.local->origin());

		// detect if you can peek enemy backtracked pos
		auto old_record = g_animation_fix->get_oldest_record(player);
		auto last_record = g_animation_fix->get_latest_record(player);

		records_t* record = nullptr;
		if (last_record)
			record = last_record;
		else if (old_record)
			record = old_record;

		for (const auto& i : hitbox_list)
		{
			auto hitbox_pos = player->get_hitbox_position(i, record ? record->sim_orig.bone : nullptr);

			if (record)
			{
				g_rage_bot->store(player);
				g_rage_bot->set_record(player, record);
			}

			bool can_hit_point = g_auto_wall->can_hit_point(player, g_ctx.local, hitbox_pos, predicted_eye_pos, 0);

			if (record)
				g_rage_bot->restore(player);

			if (can_hit_point)
			{
				can_peek = true;
				break;
			}
		}
	}

	return can_peek;
}

bool c_anti_aim::is_fake_ducking()
{
	return this->fake_ducking;
}

bool can_fake_duck()
{
	return g_cfg.binds[fd_b].toggled && (g_utils->on_ground() && !(g_ctx.cmd->buttons & in_jump));
}

void c_anti_aim::fake_duck()
{
	auto state = g_ctx.local->animstate();
	if (!state)
		return;

	g_ctx.cmd->buttons |= in_bullrush;

	static bool start = true;
	if (interfaces::game_rules->is_valve_ds() || interfaces::game_rules->is_freeze_time())
	{
		this->fake_ducking = false;
		start = true;
		return;
	}

	if (g_ctx.local->flags() & fl_frozen || !g_utils->on_ground())
	{
		this->fake_ducking = false;
		start = true;
		return;
	}

	if (g_cfg.binds[fd_b].toggled)
	{
		if (start)
		{
			if (interfaces::client_state->choked_commands > 0)
			{
				this->fake_ducking = false;
				return;
			}
			else
				g_ctx.cmd->buttons &= ~in_duck;

			start = false;
			this->fake_ducking = false;
			return;
		}

		if (interfaces::client_state->choked_commands < 7)
			g_ctx.cmd->buttons &= ~in_duck;
		else
			g_ctx.cmd->buttons |= in_duck;

		this->fake_ducking = true;

		if (interfaces::client_state->choked_commands > 0)
			g_ctx.cmd->buttons &= ~(in_attack);
	}
	else
	{
		if (!g_ctx.local->ducking())
			this->fake_ducking = false;

		start = true;
	}
}

void c_anti_aim::slow_walk()
{
	if (!g_utils->on_ground())
		return;

	if (g_cfg.binds[sw_b].toggled)
		g_movement->force_speed(g_movement->get_max_speed() * 0.34f);
}

void c_anti_aim::force_move()
{
	c_animstate* animstate = g_ctx.local->animstate();
	if (!animstate)
		return;

	if (!g_cfg.antihit.desync)
		return;

	if (g_movement->peek_move && g_movement->peek_pos.dist_to(g_ctx.local->origin()) > 1.f)
		return;

	float speed = g_ctx.local->velocity().length(true);
	if (speed > 10.f)
		return;

	if (cmd_shift::shifting)
	{
		g_ctx.cmd->forwardmove = 0.f;
		return;
	}

	bool moving = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_forward || g_ctx.cmd->buttons & in_back;

	bool ready_to_move = !moving && g_utils->on_ground();
	if (ready_to_move)
	{
		float modifier = (animstate->anim_duck_amount > 0.f || fake_ducking) ? 3.25f : 1.01f;
		if (g_cfg.antihit.distortion)
			modifier *= -2.f;

		g_ctx.cmd->forwardmove = this->flip_move ? -modifier : modifier;
	}

	this->flip_move = !this->flip_move;
}

void c_anti_aim::fake()
{
	auto state = g_ctx.local->animstate();
	if (!state)
		return;

	if (!g_cfg.antihit.desync || cmd_shift::shifting || g_exploits->cl_move.trigger && g_exploits->cl_move.shifting || g_exploits->defensive.tickbase_choke < 2)
		return;

	auto vars = g_local_animation_fix->get_updated_netvars();

	float angle = 0.f;

	if (g_cfg.antihit.random_amount)
	{
		math::random_seed(interfaces::global_vars->tick_count);
		angle = this->fake_side == 1 ? 58 - math::random_int(0, 58) : math::random_int(0, 58);
	}
	else
		angle = this->fake_side == 1 ? g_cfg.antihit.desync_left : g_cfg.antihit.desync_right;

	float desync_range = angle * vars->aim_matrix_width_range;
	float desync_max = vars->max_desync_range;

	if (!*g_ctx.send_packet)
	{
		float simulated_fake = math::normalize(this->best_yaw + desync_range * this->fake_side);
		float eye_feet_delta = math::normalize(vars->foot_yaw) - simulated_fake;

		if (desync_range < desync_max && std::fabsf(eye_feet_delta) < desync_max)
		{
			if (eye_feet_delta > 0.f)
				this->best_yaw = math::normalize(simulated_fake - desync_max);
			else
				this->best_yaw = math::normalize(simulated_fake + desync_max);
		}
		else
			this->best_yaw = math::normalize(simulated_fake + desync_max * this->fake_side);
	}
}

void c_anti_aim::extended_fake()
{
	if (!g_cfg.antihit.distortion || g_ctx.valve_ds)
		return;

	if (!*g_ctx.send_packet)
		return;

	auto speed = g_ctx.local->velocity().length(true);
	auto max_speed = g_movement->get_max_speed();

	bool ensure_lean = (max_speed / 3.4f) >= speed || g_cfg.binds[sw_b].toggled || g_cfg.binds[ens_lean_b].toggled;

	bool moving = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_forward || g_ctx.cmd->buttons & in_back;

	float max_roll = ensure_lean ? 50.f : 47.f;
	if (g_cfg.antihit.distortion_pitch > 0 && g_fake_lag->get_choke_amount() >= 14)
		max_roll = 94.f;

	float roll_angle = max_roll * ((float)g_cfg.antihit.distortion_range / 100.f) * (-fake_side);

	if (g_cfg.binds[ens_lean_b].toggled || !moving)
		g_ctx.cmd->viewangles.z = roll_angle;
}

void c_anti_aim::manual_yaw()
{
	if (g_cfg.binds[left_b].toggled)
		this->best_yaw -= 90.f;

	if (g_cfg.binds[right_b].toggled)
		this->best_yaw += 90.f;
}

void c_anti_aim::automatic_edge()
{
	this->edging = false;

	if (g_cfg.binds[freestand_b].toggled || !g_cfg.binds[edge_b].toggled || !g_utils->on_ground() || g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled)
		return;

	float best_dist = FLT_MAX;
	float best_yaw = FLT_MAX;
	vector3d best_pos = {};

	vector3d start = g_ctx.local->get_abs_origin() + vector3d(0.f, 0.f, g_ctx.local->view_offset().z / 2.f);

	for (float step = 0.f; step <= 2.f * M_PI; step += math::deg_to_rad(18.f))
	{
		float x = 40.f * std::cos(step);
		float y = 40.f * std::sin(step);

		vector3d end = vector3d(start.x + x, start.y + y, start.z);

		c_trace_filter filter;
		filter.skip = g_ctx.local;

		c_game_trace trace = {};
		interfaces::engine_trace->trace_ray(ray_t(start, end), contents_solid | contents_grate, &filter, &trace);

		if (trace.entity && (trace.entity->is_player() || trace.entity->is_weapon()))
			continue;

		if (trace.fraction < 1.f)
		{
			float dist = start.dist_to(trace.end);
			if (best_dist > dist && std::fabsf(best_dist - dist) > 0.5f)
			{
				best_yaw = math::rad_to_deg(step - PI);
				best_pos = trace.end;
				best_dist = dist;
			}
		}
	}

	if (best_yaw == FLT_MAX)
		return;

	this->edging = true;
	this->best_yaw = math::normalize(best_yaw);
}

inline void threaded_pen(c_csplayer* player, vector3d& start, vector3d& point, c_game_trace* trace, pen_data_t* pen)
{
	const auto origin_backup = g_ctx.local->get_abs_origin();

	g_ctx.local->set_abs_origin(vector3d(start.x, start.y, origin_backup.z));

	*pen = g_auto_wall->fire_bullet(g_ctx.local, player, g_ctx.weapon_info, false, start, point, false, trace);

	g_ctx.local->set_abs_origin(origin_backup);
}

void c_anti_aim::freestanding()
{
	if (!g_cfg.binds[freestand_b].toggled || !g_utils->on_ground() || g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled)
		return;

	auto player = this->get_closest_player();
	if (!player)
		return;

	auto weapon = player->get_active_weapon();
	if (!weapon)
		return;

	auto weapon_info = weapon->get_weapon_info();
	if (!weapon_info)
		return;

	vector3d local_eye_pos{};
	if (g_rage_bot->predicted_eye_pos.valid())
		local_eye_pos = g_rage_bot->predicted_eye_pos;
	else
		local_eye_pos = g_ctx.eye_position;

	float range = 20.f + (15.f * (g_ctx.local->velocity().length(true) / g_movement->get_max_speed()));

	head_pos_t head_poses[2]{};

	auto player_shoot_position = player->get_eye_position();
	auto player_angle = math::angle_from_vectors(local_eye_pos, player_shoot_position);

	{
		auto side_right = player_angle.y + 90.0f;
		auto side_rad = math::deg_to_rad(side_right);

		auto sin_angle = std::sin(side_rad);
		auto cos_angle = std::cos(side_rad);

		head_poses[0] = head_pos_t(side_right, 0.0f, 0.f,
			vector3d(
				range * cos_angle + local_eye_pos.x,
				range * sin_angle + local_eye_pos.y,
				local_eye_pos.z),
			vector3d{});
	}

	{
		auto side_left = player_angle.y - 90.0f;
		auto side_rad = math::deg_to_rad(side_left);

		auto sin_angle = std::sin(side_rad);
		auto cos_angle = std::cos(side_rad);

		head_poses[1] = head_pos_t(side_left, 0.0f, 0.f,
			vector3d(
				range * cos_angle + local_eye_pos.x,
				range * sin_angle + local_eye_pos.y,
				local_eye_pos.z),
			vector3d{});
	}

	pen_data_t pens[2]{};
	std::vector<uint64_t> tasks{};

	for (int i = 0; i < 2; ++i)
	{
		auto& head_position = head_poses[i];
		tasks.emplace_back(g_thread_pool->add_task(threaded_pen, player, std::ref(head_position.position), std::ref(player_shoot_position), &head_position.trace, &head_position.pen));
	}

	for (auto& task : tasks)
		g_thread_pool->wait(task);

	for (int i = 0; i < 2; ++i)
	{
		auto& head_position = head_poses[i];

		head_position.fraction = head_position.trace.fraction;
		head_position.damage = (float)head_position.pen.dmg;
		head_position.end_position = head_position.trace.end;
	}

	if (head_poses[0].damage + 0.5f < head_poses[1].damage)
		this->best_yaw = head_poses[1].angle;
	else if (head_poses[1].damage + 0.5f < head_poses[0].damage)
		this->best_yaw = head_poses[0].angle;
	else
	{
		if (head_poses[0].fraction + 0.5f < head_poses[1].fraction)
			this->best_yaw = head_poses[1].angle;
		else if (head_poses[1].fraction + 0.5f < head_poses[0].fraction)
			this->best_yaw = head_poses[0].angle;
	}
}

void c_anti_aim::at_targets()
{
	if (!g_cfg.antihit.at_targets || g_cfg.binds[edge_b].toggled || g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled)
		return;

	auto player = this->get_closest_player(false, true);
	if (!player)
		return;

	this->best_yaw = math::normalize(math::angle_from_vectors(g_ctx.local->get_abs_origin(), player->get_abs_origin()).y);
}

void c_anti_aim::on_pre_predict()
{
	if (g_ctx.local->move_type() == movetype_noclip || g_ctx.local->move_type() == movetype_ladder)
		return;

	if (interfaces::game_rules->is_freeze_time() || g_ctx.local->flags() & fl_frozen || g_ctx.local->gun_game_immunity())
		return;

	this->fake_duck();
	this->slow_walk();
}

void c_anti_aim::on_predict_start()
{
	auto update_tickbase_state = [&]()
	{
		static int old_tickbase = 0;

		if (!g_exploits->enabled() || (g_exploits->get_exploit_mode() != exploits_dt))
		{
		//	g_ctx.cmd->viewangles.x = g_ctx.orig_angle.x;

			defensive_aa = false;
			old_tickbase = 0;
			return;
		}

		auto tickbase_diff = g_ctx.local->tickbase() - old_tickbase;

		switch (g_cfg.antihit.def_aa_mode)
		{
		case 0:
			defensive_aa = tickbase_diff < 0 || tickbase_diff > 1;
			break;
		case 1:
			defensive_aa = g_exploits->defensive.tickbase_choke != 100
				&& g_exploits->defensive.tickbase_choke > 0 && interfaces::client_state->choked_commands;
			break;
		}

		old_tickbase = g_ctx.local->tickbase();
	};

	update_tickbase_state();

	if (g_ctx.local->move_type() == movetype_ladder || g_ctx.local->move_type() == movetype_noclip)
		return;

	if (!g_cfg.antihit.enable)
		return;

	if (interfaces::game_rules->is_freeze_time() || g_ctx.local->flags() & fl_frozen)
		return;

	auto state = g_ctx.local->animstate();
	if (!state)
		return;

	static int duration{}, dsy_duration{};
	static bool random_flipper{}, random_dsy_flipper{};

	int choke_amount = g_fake_lag->get_choke_amount();

	this->force_move();

	if (g_utils->is_firing())
		this->aa_shot_cmd = g_ctx.cmd->command_number;

	if (g_ctx.local->move_type() == movetype_ladder || g_ctx.local->move_type() == movetype_noclip)
		return;

	if (g_ctx.cmd->buttons & in_use)
		return;

	if (this->aa_shot_cmd == g_ctx.cmd->command_number)
		return;

	bool moving = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_forward || g_ctx.cmd->buttons & in_back;

	float add_roll = g_cfg.antihit.distortion_pitch;

	switch (g_cfg.antihit.pitch)
	{
	case 1:
	{
		if (g_cfg.antihit.def_pitch && defensive_aa)
		{
			math::random_seed(interfaces::global_vars->tick_count);
			g_ctx.cmd->viewangles.x = g_cfg.antihit.def_aa_mode == 1 ? math::random_float(-89.f, 89.f) : -89.f;
		}
		else
		{
			if (g_cfg.antihit.distortion_pitch > 0.f && g_fake_lag->get_choke_amount() >= 14)
			{
				const auto choke = interfaces::client_state->choked_commands + 1;
				for (int i = 1; i <= choke; i++)
				{
					auto cmds = interfaces::input->get_user_cmd(g_ctx.cmd->command_number - choke + i);

					cmds->viewangles.x = 89.f + add_roll;
				}
			}
			else
				g_ctx.cmd->viewangles.x = 89.f;
		}
	}
	break;
	case 2:
		if (g_cfg.antihit.def_pitch && defensive_aa)
		{
			math::random_seed(interfaces::global_vars->tick_count);
			g_ctx.cmd->viewangles.x = g_cfg.antihit.def_aa_mode == 1 ? math::random_float(-89.f, 89.f) : 89.f;
		}
		else
		{
				g_ctx.cmd->viewangles.x = -89.f;
		}
		break;
	}

	start_yaw = g_ctx.cmd->viewangles.y;
	this->best_yaw = g_ctx.cmd->viewangles.y;

	auto dsy_flipper = g_cfg.antihit.random_dsy ? random_dsy_flipper : this->flip_side;

	if (g_cfg.antihit.desync_mode)
		this->fake_side = dsy_flipper ? 1 : -1;
	else
		this->fake_side = g_cfg.binds[inv_b].toggled ? -1 : 1;

	this->at_targets();
	this->automatic_edge();
	this->freestanding();
	this->extended_fake();
	this->manual_yaw();

	this->fake();

	switch (g_cfg.antihit.yaw)
	{
	case 1:
		this->best_yaw += 180.f;
		break;
	case 2:
		this->best_yaw += 360.f + 90.f + std::fmod(interfaces::global_vars->cur_time * 360.f, 180.f);
		break;
	}

	static int tick = 0;

	float range = g_cfg.antihit.jitter_range;

	auto jitter_flipper = g_cfg.antihit.random_jitter ? random_flipper : this->flip_jitter;

	switch (g_cfg.antihit.jitter_mode)
	{
	case 1:
		this->best_yaw += jitter_flipper ? range : -range;
		break;
	case 2:
		if (!jitter_flipper)
			this->best_yaw += g_cfg.antihit.jitter_range;
		break;
	case 3:
		this->best_yaw += math::random_float(-g_cfg.antihit.jitter_range, g_cfg.antihit.jitter_range);
		break;
	case 4:
	{
		switch (tick)
		{
		case 0:
			this->best_yaw -= g_cfg.antihit.jitter_range;
			break;
		case 2:
			this->best_yaw += g_cfg.antihit.jitter_range;
			break;
		}
	}break;
	}

	if (*g_ctx.send_packet)
	{
		math::random_seed(interfaces::global_vars->tick_count);

		auto make_random_timer = [&](bool& flipper, int& timer, int min, int max)
		{
			if (std::abs(interfaces::global_vars->tick_count - timer) > math::random_int(min, max))
			{
				timer = interfaces::global_vars->tick_count;
				flipper = !flipper;
			}
		}; 

		make_random_timer(random_dsy_flipper, dsy_duration, 1, 4);
		make_random_timer(random_flipper, duration, 2, 5);

		this->flip_side = !this->flip_side;
		this->flip_jitter = !this->flip_jitter;

		++tick;
		tick %= 3;
	}

	if (!(g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled))
		this->best_yaw += g_cfg.antihit.yaw_add;

	if (g_cfg.antihit.def_yaw && defensive_aa)
	{
		math::random_seed(interfaces::global_vars->tick_count);
		this->best_yaw += math::random_float(-180.f, 180.f);
	}

	g_ctx.cmd->viewangles.y = math::normalize(this->best_yaw);
}

void c_anti_aim::on_predict_end()
{
	g_movement->fix_movement(g_ctx.cmd, g_ctx.base_angle);
}