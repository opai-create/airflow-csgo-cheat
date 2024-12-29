#include "globals.hpp"
#include "engine_prediction.hpp"
#include "animations.hpp"
#include "lagcomp.hpp"
#include "movement.hpp"
#include "game_movement.hpp"
#include "entlistener.hpp"
#include "exploits.hpp"
#include "event_logs.hpp"
#include "chams.hpp"
#include "anti_aim.hpp"
#include "penetration.hpp"
#include "resolver.hpp"
#include "ragebot.hpp"

/*
#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif
*/
	
void draw_hitbox__(c_cs_player* player, matrix3x4_t* bones, int idx, int idx2, bool dur = false)
{
	auto studio_model = HACKS->model_info->get_studio_model(player->get_model());
	if (!studio_model)
		return;

	auto hitbox_set = studio_model->hitbox_set(0);
	if (!hitbox_set)
		return;

	for (int i = 0; i < hitbox_set->num_hitboxes; i++)
	{
		auto hitbox = hitbox_set->hitbox(i);
		if (!hitbox)
			continue;

		vec3_t vMin, vMax;
		math::vector_transform(hitbox->min, bones[hitbox->bone], vMin);
		math::vector_transform(hitbox->max, bones[hitbox->bone], vMax);

		if (hitbox->radius != -1.f)
			HACKS->debug_overlay->add_capsule_overlay(vMin, vMax, hitbox->radius, 255, 255 * idx, 255 * idx2, 150, dur ? HACKS->global_vars->interval_per_tick * 2 : 5.f, 0, 1);
	}
}

INLINE bool valid_hitgroup(int index)
{
	if ((index >= HITGROUP_HEAD && index <= HITGROUP_RIGHTLEG) || index == HITGROUP_GEAR)
		return true;

	return false;
}

bool can_hit_hitbox(const vec3_t& start, const vec3_t& end, rage_player_t* rage, int hitbox, matrix3x4_t* matrix, anim_record_t* record)
{
	auto model = rage->player->get_model();
	if (!model)
		return false;

	auto studio_model = HACKS->model_info->get_studio_model(rage->player->get_model());
	auto set = studio_model->hitbox_set(0);

	if (!set)
		return false;

	auto studio_box = set->hitbox(hitbox);
	if (!studio_box)
		return false;

	vec3_t min, max;

	math::vector_transform(studio_box->min, matrix[studio_box->bone], min);
	math::vector_transform(studio_box->max, matrix[studio_box->bone], max);

	if (studio_box->radius != -1.f)
		return math::segment_to_segment(start, end, min, max) < studio_box->radius;

	c_game_trace trace{};
	HACKS->engine_trace->clip_ray_to_entity({ start, end }, MASK_SHOT_HULL | CONTENTS_HITBOX, rage->player, &trace);

	if (auto ent = trace.entity; ent)
	{
		if (ent == rage->player)
		{
			if (valid_hitgroup(trace.hitgroup))
				return true;
		}
	}

	return false;
}

float get_point_accuracy(rage_player_t* rage, vec3_t& eye_pos, const rage_point_t& point, matrix3x4_t* matrix, anim_record_t* record)
{
	static auto weapon_accuracy_nospread = HACKS->convars.weapon_accuracy_nospread;
	if (weapon_accuracy_nospread && weapon_accuracy_nospread->get_bool())
		return 1.f;

	auto predicted_info = ENGINE_PREDICTION->get_networked_vars(HACKS->cmd->command_number);

	auto hits = 0;

	auto spread = predicted_info->spread + predicted_info->inaccuracy;
	auto angle = math::calc_angle(eye_pos, point.aim_point).normalized_angle();

	vec3_t forward, right, up;
	math::angle_vectors(angle, &forward, &right, &up);

	for (auto i = 1; i <= 6; ++i)
	{
		for (auto j = 0; j < 8; ++j)
		{
			auto current_spread = spread * ((float)i / 6.f);

			float value = (float)j / 8.0f * (M_PI * 2.f);

			auto direction_cos = std::cos(value);
			auto direction_sin = std::sin(value);

			auto spread_x = direction_sin * current_spread;
			auto spread_y = direction_cos * current_spread;

			vec3_t direction{};
			direction.x = forward.x + spread_x * right.x + spread_y * up.x;
			direction.y = forward.y + spread_x * right.y + spread_y * up.y;
			direction.z = forward.z + spread_x * right.z + spread_y * up.z;

			auto end = eye_pos + direction * HACKS->weapon_info->range;

			if (can_hit_hitbox(eye_pos, end, rage, point.hitbox, matrix, record))
				hits++;
		}
	}

	return (float)hits / 48.f;
}

bool c_ragebot::can_fire(bool ignore_revolver)
{
	if (!HACKS->local || !HACKS->weapon)
		return false;

	if (HACKS->cmd->weapon_select != 0)
		return false;

	if (!HACKS->weapon_info)
		return false;

	if (HACKS->local->flags().has(FL_ATCONTROLS))
		return false;

	if (HACKS->local->wait_for_no_attack())
		return false;

	if (HACKS->local->is_defusing())
		return false;

	if (HACKS->weapon_info->weapon_type >= 1 && HACKS->weapon_info->weapon_type <= 6 && HACKS->weapon->clip1() < 1)
		return false;

	if (HACKS->local->player_state() > 0)
		return false;

	auto weapon_index = HACKS->weapon->item_definition_index();
	if ((weapon_index == WEAPON_GLOCK || weapon_index == WEAPON_FAMAS) && HACKS->weapon->burst_shots_remaining() > 0)
		return HACKS->predicted_time >= HACKS->weapon->next_burst_shot();

	// TO-DO: auto revolver detection
	if (weapon_index == WEAPON_REVOLVER && !ignore_revolver)
		return revolver_fire;

	float next_attack = HACKS->local->next_attack();
	float next_primary_attack = HACKS->weapon->next_primary_attack();

	return HACKS->predicted_time >= next_attack && HACKS->predicted_time >= next_primary_attack;
}

bool c_ragebot::is_shooting()
{
	if (!HACKS->weapon)
		return false;

	bool attack2 = HACKS->cmd->buttons.has(IN_ATTACK2);
	bool attack = HACKS->cmd->buttons.has(IN_ATTACK);

	short weapon_index = HACKS->weapon->item_definition_index();

	if (weapon_index == WEAPON_C4)
		return false;

	if ((weapon_index == WEAPON_GLOCK || weapon_index == WEAPON_FAMAS) && HACKS->weapon->burst_shots_remaining() > 0)
		return HACKS->predicted_time >= HACKS->weapon->next_burst_shot();

	if (HACKS->weapon->is_grenade())
		return !HACKS->weapon->pin_pulled() && HACKS->weapon->throw_time() > 0.f && HACKS->weapon->throw_time() < HACKS->predicted_time;

	auto can_fire_now = can_fire();
	if (weapon_index == WEAPON_REVOLVER)
		return attack && can_fire_now;

	if (HACKS->weapon->is_knife())
		return (attack || attack2) && can_fire_now;

	return attack && can_fire_now;
}

void c_ragebot::update_hitboxes()
{
	if (HACKS->weapon->is_taser())
	{
		hitboxes.emplace_back(HITBOX_STOMACH);
		hitboxes.emplace_back(HITBOX_PELVIS);
		return;
	}

	if (rage_config.hitboxes & head)
		hitboxes.emplace_back(HITBOX_HEAD);

	if (rage_config.hitboxes & chest)
		hitboxes.emplace_back(HITBOX_CHEST);

	if (rage_config.hitboxes & stomach)
		hitboxes.emplace_back(HITBOX_STOMACH);

	if (rage_config.hitboxes & pelvis)
		hitboxes.emplace_back(HITBOX_PELVIS);

	if (rage_config.hitboxes & arms_)
	{
		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
	}

	if (rage_config.hitboxes & legs)
	{
		hitboxes.emplace_back(HITBOX_LEFT_FOOT);
		hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
	}
}

multipoints_t c_ragebot::get_points(c_cs_player* player, int hitbox, matrix3x4_t* matrix)
{
	auto local_anims = ANIMFIX->get_local_anims();
	multipoints_t points;

	const model_t* model = player->get_model();
	if (!model)
		return points;

	studiohdr_t* hdr = HACKS->model_info->get_studio_model(model);
	if (!hdr)
		return points;

	mstudiohitboxset_t* set = hdr->hitbox_set(player->hitbox_set());
	if (!set)
		return points;

	mstudiobbox_t* bbox = set->hitbox(hitbox);
	if (!bbox)
		return points;

	if (bbox->radius <= 0.f)
	{
		matrix3x4_t rot_matrix = { };
		rot_matrix.angle_matrix(bbox->rotation);

		matrix3x4_t mat = { };
		math::contact_transforms(matrix[bbox->bone], rot_matrix, mat);

		auto origin = mat.get_origin();
		auto center = (bbox->min + bbox->max) * 0.5f;

		if (hitbox == HITBOX_LEFT_FOOT || hitbox == HITBOX_RIGHT_FOOT)
		{
			float d1 = (bbox->min.z - center.z) * 0.875f;
			if (hitbox == HITBOX_LEFT_FOOT)
				d1 *= -1.f;

			points.emplace_back(vec3_t{ center.x, center.y, center.z + d1 }, false);

			float d2 = (bbox->min.x - center.x) * 0.5f;
			float d3 = (bbox->max.x - center.x) * 0.5f;

			points.emplace_back(vec3_t{ center.x + d2, center.y, center.z }, false);
			points.emplace_back(vec3_t{ center.x + d3, center.y, center.z }, false);
		}
		else
			points.emplace_back(center, true);

		if (points.empty())
			return points;

		for (auto& p : points)
		{
			p.first = { p.first.dot(mat.mat[0]), p.first.dot(mat.mat[1]), p.first.dot(mat.mat[2]) };
			p.first += origin;
		}
	}
	else
	{
		vec3_t max = bbox->max;
		vec3_t min = bbox->min;
		auto center = (bbox->min + bbox->max) / 2.f;

		auto dynamic_scale = get_dynamic_scale(center, bbox->radius);

		auto head_scale = rage_config.scale_head != -1 ? rage_config.scale_head * 0.01f : dynamic_scale;
		auto body_scale = rage_config.scale_body != -1 ? rage_config.scale_body * 0.01f : dynamic_scale;

		auto body_slider = 0.35f * body_scale;
		auto r = bbox->radius * body_slider, r2 = bbox->radius * (body_slider * 2.f - 1.f);

		switch (hitbox)
		{
		case HITBOX_HEAD:
		{
			points.emplace_back(center, true);

			float head_slider = 0.75f * head_scale;

			float r = bbox->radius * head_slider;
			points.emplace_back(vec3_t{ max.x + (MATRIX_HEAD_ROTATION * r), max.y + (-MATRIX_HEAD_ROTATION * r), max.z }, false);

			vec3_t right{ max.x, max.y, max.z + r };
			points.emplace_back(right, false);

			vec3_t left{ max.x, max.y, max.z - r };
			points.emplace_back(left, false);

			points.emplace_back(vec3_t{ max.x, max.y - r, max.z }, false);

			auto state = player->animstate();
			if (state && state->velocity_length_xy <= 0.1f && player->eye_angles().x <= 75.f)
				points.emplace_back(vec3_t{ max.x - r, max.y, max.z }, false);
		}
		break;
		case HITBOX_UPPER_CHEST:
		{
			points.emplace_back(center, true);

			points.emplace_back(vec3_t{ center.x, center.y, max.z + r }, false);
			points.emplace_back(vec3_t{ center.x, center.y, min.z - r }, false);
		}
		break;
		case HITBOX_PELVIS:
		{
			points.emplace_back(center, true);

			points.emplace_back(vec3_t{ center.x, center.y, max.z + r }, false);
			points.emplace_back(vec3_t{ center.x, center.y, min.z - r }, false);
		}
		break;
		case HITBOX_CHEST:
		case HITBOX_THORAX:
		{
			points.emplace_back(center, true);

			points.emplace_back(vec3_t{ center.x, center.y, max.z + r }, false);
			points.emplace_back(vec3_t{ center.x, center.y, min.z - r }, false);
			points.emplace_back(vec3_t{ center.x, max.y - r, center.z }, false);
		}
		break;
		case HITBOX_STOMACH:
		{
			points.emplace_back(center, true);

			points.emplace_back(vec3_t{ center.x, center.y, min.z + r }, false);
			points.emplace_back(vec3_t{ center.x, center.y, max.z - r }, false);
			points.emplace_back(vec3_t{ center.x, max.y - r, center.z }, false);
		}
		break;
		case HITBOX_RIGHT_CALF:
		case HITBOX_LEFT_CALF:
		{
			points.emplace_back(center, true);
			points.emplace_back(vec3_t{ max.x - (bbox->radius / 2.f), max.y, max.z }, false);
		}
		break;
		case HITBOX_RIGHT_UPPER_ARM:
		case HITBOX_LEFT_UPPER_ARM:
		{
			points.emplace_back(center, true);
			points.emplace_back(vec3_t{ max.x + bbox->radius, center.y, center.z }, false);
		}
		break;
		default:
			points.emplace_back(center, true);
			break;
		}

		if (points.empty())
			return points;

		for (auto& p : points)
			math::vector_transform(p.first, matrix[bbox->bone], p.first);
	}

	return points;
}

void c_ragebot::run_stop()
{
	if (!HACKS->weapon || HACKS->client_state->delta_tick == -1)
		return;

	if (!HACKS->weapon_info || !HACKS->weapon || !trigger_stop)
		return;

	auto force_acc = rage_config.quick_stop_options & force_accuracy;

	auto max_speed = HACKS->local->is_scoped() ? HACKS->weapon_info->max_speed_alt : HACKS->weapon_info->max_speed;
	max_speed *= 0.34f;

	auto velocity = HACKS->local->velocity();
	if (velocity.length_2d() <= 15.f)
		return;

	if (velocity.length_2d() <= max_speed)
	{
		game_movement::modify_move(*HACKS->cmd, velocity, max_speed);

		if (force_acc)
			should_shot = false;

		return;
	}

	game_movement::force_stop();

	if (force_acc)
		should_shot = true;
}

void c_ragebot::auto_pistol()
{
	if (!HACKS->weapon)
		return;

	auto index = HACKS->weapon->item_definition_index();
	if (index == WEAPON_C4
		|| index == WEAPON_HEALTHSHOT
		|| index == WEAPON_REVOLVER
		|| (index == WEAPON_GLOCK || index == WEAPON_FAMAS) && HACKS->weapon->burst_shots_remaining() > 0)
		return;

	if (HACKS->weapon->is_misc_weapon() && !HACKS->weapon->is_knife())
		return;

	auto next_attack = HACKS->local->next_attack();
	auto next_primary_attack = HACKS->weapon->next_primary_attack();
	auto next_secondary_attack = HACKS->weapon->next_secondary_attack();

	if (HACKS->predicted_time < next_attack || HACKS->predicted_time < next_primary_attack)
	{
		if (HACKS->cmd->buttons.has(IN_ATTACK))
			HACKS->cmd->buttons.remove(IN_ATTACK);
	}

	if (HACKS->predicted_time < next_secondary_attack)
	{
		if (HACKS->cmd->buttons.has(IN_ATTACK2))
			HACKS->cmd->buttons.remove(IN_ATTACK2);
	}
}

void c_ragebot::force_scope()
{
	if (!rage_config.auto_scope)
		return;

	bool able_to_zoom = HACKS->predicted_time >= HACKS->weapon->next_secondary_attack();
	if (able_to_zoom && HACKS->weapon->zoom_level() < 1 && HACKS->weapon->is_sniper() && !HACKS->cmd->buttons.has(IN_ATTACK2))
		HACKS->cmd->buttons.force(IN_ATTACK2);
}

bool c_ragebot::should_stop(const rage_point_t& point)
{
	if (HACKS->weapon->is_taser())
		return false;

	auto unpredicted_vars = ENGINE_PREDICTION->get_unpredicted_vars();
	if (!unpredicted_vars)
		return false;

	if (!(rage_config.quick_stop_options & between_shots) && !can_fire())
		return false;

	if (!MOVEMENT->on_ground() && !(rage_config.quick_stop_options & in_air))
		return false;

	return true;
}

void c_ragebot::update_predicted_eye_pos()
{
	auto unpredicted_vars = ENGINE_PREDICTION->get_initial_vars();
	if (!unpredicted_vars)
		return;

	auto anim = ANIMFIX->get_local_anims();
	if (!anim)
		return;

	auto max_speed = HACKS->local->is_scoped() ? HACKS->weapon_info->max_speed_alt : HACKS->weapon_info->max_speed;
	auto velocity = unpredicted_vars->velocity;

	auto speed = std::max<float>(velocity.length_2d(), 1.f);
	auto max_stop_ticks = std::max<int>(((speed / max_speed) * 5) - 1, 0);

	auto max_predict_ticks = std::clamp(max_stop_ticks, 0, 14);
	if (max_predict_ticks == 0)
	{
		predicted_eye_pos = anim->eye_pos;
		return;
	}

	auto last_predicted_velocity = velocity;
	for (int i = 0; i < max_predict_ticks; ++i)
	{
		auto pred_velocity = velocity * TICKS_TO_TIME(i + 1);

		auto origin = anim->eye_pos + pred_velocity;
		auto flags = HACKS->local->flags();

		game_movement::extrapolate(HACKS->local, origin, pred_velocity, flags, flags.has(FL_ONGROUND));

		last_predicted_velocity = pred_velocity;
	}

	predicted_eye_pos = anim->eye_pos + last_predicted_velocity;
}

void c_ragebot::prepare_players_for_scan()
{
	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		auto& rage = rage_players[player->index()];

		if (!player->is_alive() || player->dormant() || player->has_gun_game_immunity())
		{
			if (rage.valid)
				rage.reset();

			return;
		}

		if (rage.player != player)
		{
			rage.reset();
			rage.player = player;
			return;
		}

		rage.distance = HACKS->local->origin().dist_to(player->origin());
		rage.valid = true;

		++rage_player_iter;
	});
}

std::vector<rage_point_t> get_hitbox_points(int damage, std::vector<int>& hitboxes, vec3_t& eye_pos, vec3_t& predicted_eye_pos, rage_player_t* rage, anim_record_t* record, bool predicted = false)
{
	if (hitboxes.empty())
		return {};

	std::vector<rage_point_t> out{};
	out.reserve(hitboxes.size());

	auto backup_origin = HACKS->local->get_abs_origin();

	if (predicted)
		HACKS->local->set_abs_origin(predicted_eye_pos);

	auto matrix_to_aim = record->extrapolated ? record->predicted_matrix : record->matrix_orig.matrix;
	LAGCOMP->set_record(rage->player, record, matrix_to_aim);

	auto local_anims = ANIMFIX->get_local_anims();
	auto& start_eye_pos = predicted ? predicted_eye_pos : local_anims->eye_pos;
	
	int wrong_damage_counter = 0;
	for (auto& hitbox : hitboxes)
	{
		auto aim_point = rage->player->get_hitbox_position(hitbox, matrix_to_aim);
		auto bullet = penetration::simulate(HACKS->local, rage->player, start_eye_pos, aim_point);

		if (bullet.traced_target == nullptr
			|| bullet.traced_target != rage->player
			|| HACKS->weapon->is_taser() && bullet.penetration_count < 4)
			continue;

		rage_point_t point{};
		point.center = true;
		point.hitbox = hitbox;
		point.damage = bullet.damage;
		point.aim_point = aim_point;
		point.predicted_eye_pos = predicted;

#ifndef LEGACY
		point.safety = [&]()
		{
			auto safety = 0;

			matrix3x4_t* matrices[]
			{
				record->matrix_left.matrix,
				record->matrix_left.roll_matrix,
				record->matrix_right.matrix,
				record->matrix_right.roll_matrix,
				record->matrix_zero.matrix,
			};

			for (int i = 0; i < 5; ++i)
			{
				if (can_hit_hitbox(start_eye_pos, aim_point, rage, hitbox, matrices[i], record))
					++safety;
			}

			return safety;
		}();
#endif

		out.emplace_back(point);
	}

	if (predicted)
		HACKS->local->set_abs_origin(backup_origin);

	return out;
}

void player_move(c_cs_player* player, anim_record_t* record)
{
	static auto sv_gravity = HACKS->convars.sv_gravity;
	static auto sv_jump_impulse = HACKS->convars.sv_jump_impulse;

	auto src = record->prediction.origin;
	auto end = src + record->prediction.velocity * HACKS->global_vars->interval_per_tick;

	c_game_trace t;
	c_trace_filter filter;
	filter.skip = player;

	HACKS->engine_trace->trace_ray(ray_t(src, end, record->mins, record->maxs), MASK_PLAYERSOLID, &filter, &t);

	if (t.fraction != 1.f)
	{
		for (auto i = 0; i < 2; i++)
		{
			record->prediction.velocity -= t.plane.normal * record->prediction.velocity.dot(t.plane.normal);

			const auto dot = record->prediction.velocity.dot(t.plane.normal);
			if (dot < 0.f)
				record->prediction.velocity -= vec3_t{ dot* t.plane.normal.x, dot* t.plane.normal.y, dot* t.plane.normal.z };

			end = t.end + record->prediction.velocity * TICKS_TO_TIME(1.f - t.fraction);
 
			HACKS->engine_trace->trace_ray(ray_t(t.end, end, record->mins, record->maxs), MASK_PLAYERSOLID, &filter, &t);

			if (t.fraction == 1.f)
				break;
		}
	}

	src = end = record->prediction.origin = t.end;
	end.z -= 2.f;

	HACKS->engine_trace->trace_ray(ray_t(record->prediction.origin, end, record->mins, record->maxs), MASK_PLAYERSOLID, &filter, &t);

	record->prediction.flags.remove(FL_ONGROUND);

	if (t.fraction != 1.f && t.plane.normal.z > 0.7f)
		record->prediction.flags.force(FL_ONGROUND);
}

bool start_fakelag_fix(c_cs_player* player, anims_t* anims)
{
	if (anims->records.empty())
		return false;

	if (player->dormant())
		return false;

	size_t size{};
	for (const auto& it : anims->records)
	{
		if (it.dormant)
			break;

		++size;
	}

	auto record = &anims->records.front();
	record->extrapolated = false;
	record->predict();

	if (record->choke <= 0)
		return false;

	if (size > 1 && ((record->origin - anims->records[1].origin).length_sqr() > 4096.f
		|| size > 2 && (anims->records[1].origin - anims->records[2].origin).length_sqr() > 4096.f))
		record->break_lc = true;

	if (!record->break_lc)
		return false;

	int simulation = TIME_TO_TICKS(record->sim_time);
	if (std::abs(HACKS->arrival_tick - simulation) >= 128)
		return false;

	int lag = record->choke;

	int updatedelta = HACKS->client_state->clock_drift_mgr.server_tick - record->server_tick_estimation;
	if (TIME_TO_TICKS(HACKS->outgoing) <= lag - updatedelta)
		return false;

	int next = record->server_tick_estimation + 1;
	if (next + lag >= HACKS->arrival_tick)
		return false;

	auto latency = std::clamp(TICKS_TO_TIME(HACKS->ping), 0.0f, 1.0f);
	auto correct = std::clamp(latency + HACKS->lerp_time, 0.0f, HACKS->convars.sv_maxunlag->get_float());
	auto delta_time = correct - (TICKS_TO_TIME(HACKS->tickbase) - record->sim_time);
	auto predicted_tick = ((int)HACKS->client_state->clock_drift_mgr.server_tick + TIME_TO_TICKS(latency) - record->server_tick_estimation) / record->choke;

	if (predicted_tick > 0 && predicted_tick < 20)
	{
		auto max_backtrack_time = std::ceil(((delta_time - 0.2f) / HACKS->global_vars->interval_per_tick + 0.5f) / (float)record->choke);
		auto prediction_ticks = predicted_tick;

		if (max_backtrack_time > 0.0f && predicted_tick >= TIME_TO_TICKS(max_backtrack_time))
			prediction_ticks = TIME_TO_TICKS(max_backtrack_time);

		if (prediction_ticks > 0)
		{
			record->extrapolate_ticks = prediction_ticks;

			do
			{
				for (auto current_prediction_tick = 0; current_prediction_tick < record->choke; ++current_prediction_tick)
				{
					if (record->prediction.flags.has(FL_ONGROUND))
					{
						if (!HACKS->convars.sv_enablebunnyhopping->get_int())
						{
							float max = player->max_speed() * 1.1f;
							float speed = record->prediction.velocity.length();
							if (max > 0.f && speed > max)
								record->prediction.velocity *= (max / speed);
						}

						record->prediction.velocity.z = HACKS->convars.sv_jump_impulse->get_float();
					}
					else
						record->prediction.velocity.z -= HACKS->convars.sv_gravity->get_float() * HACKS->global_vars->interval_per_tick;

					player_move(player, record);
					record->prediction.time += HACKS->global_vars->interval_per_tick;
				}

				--prediction_ticks;
			} while (prediction_ticks);

			auto current_origin = record->prediction.origin;

			clamp_bones_info_t info{};
			info.collision_change_origin = record->collision_change_origin;
			info.collision_change_time = record->collision_change_time;
			info.origin = current_origin;
			info.collision_origin = current_origin;
			info.ground_entity = record->prediction.flags.has(FL_ONGROUND) ? 1 : -1;
			info.view_offset = record->view_offset;

			math::change_bones_position(record->matrix_orig.matrix, 128, record->origin, current_origin);
			math::memcpy_sse(record->predicted_matrix, record->matrix_orig.matrix, sizeof(record->matrix_orig.matrix));
			record->matrix_orig.bone_builder.clamp_bones_in_bbox(player, record->predicted_matrix, 0x7FF00, record->prediction.time, player->eye_angles(), info);

			math::change_bones_position(record->matrix_orig.matrix, 128, current_origin, record->origin);
			record->extrapolated = true;

			return true;
		}
	}

	return false;
}

void pre_cache_centers(int damage, std::vector<int>& hitboxes, vec3_t& predicted_eye_pos, rage_player_t* rage)
{
	rage->reset_hitscan();
	auto anim = ANIMFIX->get_local_anims();

	auto lagcomp = ANIMFIX->get_anims(rage->player->index());
	if (!lagcomp || lagcomp->records.empty())
		return;

	auto get_overall_damage = [&](anim_record_t* record)
	{
		rage->points_to_scan.clear();
		rage->points_to_scan.reserve(MAX_SCANNED_POINTS);

		auto predicted_hitbox_points = get_hitbox_points(damage, hitboxes, predicted_eye_pos, predicted_eye_pos, rage, record, true);
		if (predicted_hitbox_points.empty())
		{
			auto hitbox_points = get_hitbox_points(damage, hitboxes, anim->eye_pos, predicted_eye_pos, rage, record, true);

			int overall_damage = 0;
			for (auto& point : hitbox_points)
			{
				rage->points_to_scan.emplace_back(point);
				overall_damage += point.damage;
			}

			return overall_damage;
		}
		else
		{
			int overall_damage = 0;
			for (auto& point : predicted_hitbox_points)
			{
				rage->points_to_scan.emplace_back(point);
				overall_damage += point.damage;
			}

			return overall_damage;
		}
	};

	rage->restore.store(rage->player);

	anim_record_t* best = nullptr;
	if (start_fakelag_fix(rage->player, lagcomp))
	{
		auto record = &lagcomp->records.front();
		auto record_dmg = get_overall_damage(record);
		best = record;
	}
	else
	{
		auto first_find = std::find_if(lagcomp->records.begin(), lagcomp->records.end(), [&](anim_record_t& record) {
			return record.valid_lc;
			});

		anim_record_t* first = nullptr;
		if (first_find != lagcomp->records.end())
			first = &*first_find;

		auto last_find = std::find_if(lagcomp->records.rbegin(), lagcomp->records.rend(), [&](anim_record_t& record) {
			return record.valid_lc;
			});

		anim_record_t* last = nullptr;
		if (last_find != lagcomp->records.rend())
			last = &*last_find;

		if (last)
		{
			auto last_dmg = get_overall_damage(last);
			auto first_dmg = get_overall_damage(first);

			if (last_dmg > first_dmg)
				best = last;
			else
				best = first;
		}
		else
			best = first;
	}

	rage->restore.restore(rage->player);

	if (best)
	{
		rage->start_scans = true;
		rage->hitscan_record = best;
	}
}

void get_result(bool& out, const vec3_t& start, const vec3_t& end, rage_player_t* rage, int hitbox, matrix3x4_t* matrix, anim_record_t* record)
{
	out = can_hit_hitbox(start, end, rage, hitbox, matrix, record);
}

bool hitchance(vec3_t eye_pos, rage_player_t& rage, const rage_point_t& point, anim_record_t* record, const float& chance, matrix3x4_t* matrix, float* hitchance_out = nullptr)
{
	static auto weapon_accuracy_nospread = HACKS->convars.weapon_accuracy_nospread;
	if (weapon_accuracy_nospread && weapon_accuracy_nospread->get_bool())
		return true;

#ifdef LEGACY
	if (EXPLOITS->enabled() && EXPLOITS->dt_bullet == 1)
		return true;
#endif

	auto current = 0;
	auto networked_vars = ENGINE_PREDICTION->get_networked_vars(HACKS->cmd->command_number);

	auto matrix_to_aim = record->extrapolated ? record->predicted_matrix : record->matrix_orig.matrix;

	auto current_bones = matrix ? matrix : matrix_to_aim;
	auto anim = ANIMFIX->get_local_anims();

	if ((HACKS->ideal_inaccuracy + 0.0005f) >= networked_vars->inaccuracy) {
		*hitchance_out = 1.f;
		return true;
	}

	rage.restore.store(rage.player);
	LAGCOMP->set_record(rage.player, record, current_bones);

	auto start = eye_pos;
	auto aim_angle = math::calc_angle(start, point.aim_point);

	vec3_t forward, right, up;
	math::angle_vectors(aim_angle, &forward, &right, &up);

	vec3_t total_spread, spread_angle, end;
	float inaccuracy, spread_x, spread_y;
	std::tuple<float, float, float>* seed{};

	for (auto i = 0; i < MAX_SEEDS; i++)
	{
		seed = &precomputed_seeds[i];

		inaccuracy = std::get<0>(*seed) * networked_vars->inaccuracy;
		spread_x = std::get<2>(*seed) * inaccuracy;
		spread_y = std::get<1>(*seed) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y).normalized();

		math::vector_angles(total_spread, spread_angle);

		math::angle_vectors(spread_angle, end);
		end = start + end.normalized() * HACKS->weapon_info->range;

		if (can_hit_hitbox(start, end, &rage, point.hitbox, current_bones, record))
			current++;

		if (hitchance_out)
			*hitchance_out = (float)current / (float)MAX_SEEDS;
	}

	rage.restore.restore(rage.player);

	return ((float)current / (float)MAX_SEEDS) >= chance;
}

void collect_damage_from_multipoints(int damage, vec3_t& predicted_eye_pos, rage_player_t* rage, rage_point_t& points, anim_record_t* record, matrix3x4_t* matrix_to_aim, bool predicted)
{
	auto multipoints = RAGEBOT->get_points(rage->player, points.hitbox, matrix_to_aim);
	if (multipoints.empty())
		return;

	auto local_anims = ANIMFIX->get_local_anims();

	int wrong_damage_counter = 0;

	auto& start_eye_pos = predicted ? predicted_eye_pos : local_anims->eye_pos;

	auto backup_origin = HACKS->local->get_abs_origin();

	if (predicted)
		HACKS->local->set_abs_origin({ predicted_eye_pos.x, predicted_eye_pos.y, backup_origin.z });

	for (auto& multipoint : multipoints)
	{
		if (multipoint.second)
			continue;

		bool quit_from_scan = false;
		for (auto& i : rage->points_to_scan)
		{
			if (multipoint.first == i.aim_point)
			{
				quit_from_scan = true;
				break;
			}
		}

		if (quit_from_scan)
			break;

		auto bullet = penetration::simulate(HACKS->local, rage->player, start_eye_pos, multipoint.first);
		if (bullet.damage < damage
			|| bullet.traced_target == nullptr || bullet.traced_target != rage->player
			|| HACKS->weapon->is_taser() && bullet.penetration_count < 4)
			continue;

		rage_point_t point{};
		point.center = false;
		point.hitbox = points.hitbox;
		point.damage = bullet.damage;
		point.aim_point = multipoint.first;
		point.predicted_eye_pos = points.predicted_eye_pos;

#ifndef LEGACY
		point.safety = [&]()
		{
			auto safety = 0;

			matrix3x4_t* matrices[]
			{
				record->matrix_left.matrix,
				record->matrix_left.roll_matrix,
				record->matrix_right.matrix,
				record->matrix_right.roll_matrix,
				record->matrix_zero.matrix,
			};

			for (int i = 0; i < 5; ++i)
			{
				if (can_hit_hitbox(start_eye_pos, multipoint.first, rage, points.hitbox, matrices[i], record))
					++safety;
			}

			return safety;
		}();
#endif
		rage->points_to_scan.emplace_back(point);
	}

	if (predicted)
		HACKS->local->set_abs_origin(backup_origin);
}

void c_ragebot::do_hitscan(rage_player_t* rage)
{
	if (rage->points_to_scan.empty())
		return;

	if (!rage->start_scans)
		return;

	if (!rage->hitscan_record)
		return;

	rage->restore.store(rage->player);
	{
		auto matrix_to_aim = rage->hitscan_record->extrapolated ? rage->hitscan_record->predicted_matrix : rage->hitscan_record->matrix_orig.matrix;
		LAGCOMP->set_record(rage->player, rage->hitscan_record, matrix_to_aim);
	
		int threads_count = 0;

		for (auto& points : rage->points_to_scan)
		{
			++threads_count;

			auto dmg = get_min_damage(rage->player);
			THREAD_POOL->add_task(collect_damage_from_multipoints,
				dmg,
				std::ref(predicted_eye_pos),
				rage,
				std::ref(points),
				rage->hitscan_record,
				matrix_to_aim,
				points.predicted_eye_pos);
		}

		if (threads_count > 0)
			THREAD_POOL->wait_all();
	}
	rage->restore.restore(rage->player);
}

void c_ragebot::scan_players()
{
	int threads_count = 0;

	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		if (!player->is_alive() || player->dormant() || player->has_gun_game_immunity())
			return;

		auto rage = &rage_players[player->index()];
		if (!rage || !rage->player || rage->player != player)
			return;

		++threads_count;

		auto dmg = get_min_damage(rage->player);
		THREAD_POOL->add_task(pre_cache_centers, dmg, std::ref(hitboxes), std::ref(predicted_eye_pos), rage);
	});

	if (threads_count < 1)
		return;

	THREAD_POOL->wait_all();

	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		if (!player->is_alive() || player->dormant() || player->has_gun_game_immunity())
			return;

		auto rage = &rage_players[player->index()];
		if (!rage || !rage->player || rage->player != player)
			return;

		do_hitscan(rage);
	});
}

void c_ragebot::choose_best_point()
{
	auto prefer_baim_on_dt = EXPLOITS->enabled() && EXPLOITS->get_exploit_mode() == EXPLOITS_DT
		&& (HACKS->weapon->is_auto_sniper() || HACKS->weapon->is_heavy_pistols());

	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		if (!player->is_alive() || player->dormant() || player->has_gun_game_immunity())
			return;

		auto rage = &rage_players[player->index()];
		if (!rage || !rage->player || rage->player != player)
			return;

		auto damage = get_min_damage(rage->player);
		auto get_best_aim_point = [&]() -> rage_point_t
		{
			rage_point_t best{};
			std::sort(rage->points_to_scan.begin(), rage->points_to_scan.end(), [](const rage_point_t& a, const rage_point_t& b) {
				return a.damage > b.damage;
			});

			for (auto& point : rage->points_to_scan)
			{
				auto is_body = point.hitbox == HITBOX_PELVIS || point.hitbox == HITBOX_STOMACH;

				if (point.damage < damage)
					continue;
				
				if (g_cfg.binds[force_body_b].toggled && !is_body)
					continue;

#ifndef LEGACY
				if (g_cfg.binds[force_sp_b].toggled && point.safety < 5)
					continue;
#endif
				if (point.safety == 5 && rage_config.prefer_safe)
				{
					point.found = true;
					return point;
				}
				else if (is_body && (point.damage >= player->health() || prefer_baim_on_dt || rage_config.prefer_body))
				{
					point.found = true;
					return point;
				}
				else
				{
					if (point.damage > best.damage)
					{
						best = point;
						best.found = true;
					}
				}
			}

			return best;
		};

		auto best_point = get_best_aim_point();
		if (best_point.found)
		{
			rage->best_point = best_point;
			rage->best_record = rage->hitscan_record;
			rage->best_point.found = true;
		}
	});
}

void c_ragebot::auto_revolver()
{
	if (!HACKS->local || !HACKS->weapon || !HACKS->weapon_info)
		return;

	auto next_secondary_attack = HACKS->weapon->next_secondary_attack();

	if (!g_cfg.rage.enable || EXPLOITS->recharge.start && !EXPLOITS->recharge.finish || HACKS->weapon->item_definition_index() != WEAPON_REVOLVER || HACKS->weapon->clip1() <= 0)
	{
		last_checked = 0;
		tick_cocked = 0;
		tick_strip = 0;
		next_secondary_attack = 0.f;

		revolver_fire = false;
		return;
	}

	auto time = TICKS_TO_TIME(HACKS->tickbase - EXPLOITS->tickbase_offset());
	const auto max_ticks = TIME_TO_TICKS(.25f) - 1;
	const auto tick_base = TIME_TO_TICKS(time);

	if (HACKS->local->next_attack() > time)
		return;

	if (HACKS->local->spawn_time() != last_spawn_time)
	{
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
		last_spawn_time = HACKS->local->spawn_time();
	}

	if (HACKS->weapon->next_primary_attack() > time)
	{
		HACKS->cmd->buttons.remove(IN_ATTACK);
		revolver_fire = false;
		return;
	}

	if (last_checked == tick_base)
		return;

	last_checked = tick_base;
	revolver_fire = false;

	if (tick_base - tick_strip > 2 && tick_base - tick_strip < 14)
		revolver_fire = true;

	if (HACKS->cmd->buttons.has(IN_ATTACK) && revolver_fire)
		return;

	HACKS->cmd->buttons.force(IN_ATTACK);

	if (next_secondary_attack >= time)
		HACKS->cmd->buttons.force(IN_ATTACK2);

	if (tick_base - tick_cocked > max_ticks * 2 + 1)
	{
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
	}

	const auto cock_limit = tick_base - tick_cocked >= max_ticks;
	const auto after_strip = tick_base - tick_strip <= max_ticks;

	if (cock_limit || after_strip)
	{
		tick_cocked = tick_base;
		HACKS->cmd->buttons.remove(IN_ATTACK);

		if (cock_limit)
			tick_strip = tick_base;
	}
}

bool c_ragebot::knife_is_behind(c_cs_player* player, anim_record_t* record)
{
	auto origin = record ? record->origin : player->get_abs_origin();
	auto abs_angles = record ? record->abs_angles : player->get_abs_angles();

	auto anim = ANIMFIX->get_local_anims();

	vec3_t delta{ origin - anim->eye_pos };
	delta.z = 0.f;
	delta = delta.normalized();

	vec3_t target;
	math::angle_vectors(abs_angles, target);
	target.z = 0.f;

	return delta.dot(target) > 0.475f;
}

bool c_ragebot::knife_trace(vec3_t dir, bool stab, c_game_trace* trace)
{
	float range = stab ? 32.f : 48.f;

	auto anim = ANIMFIX->get_local_anims();

	vec3_t start = anim->eye_pos;
	vec3_t end = start + (dir * range);

	c_trace_filter filter{};
	filter.skip = HACKS->local;
	HACKS->engine_trace->trace_ray(ray_t(start, end), MASK_SOLID, &filter, trace);

	if (trace->fraction >= 1.f)
	{
		HACKS->engine_trace->trace_ray(ray_t(start, end, { -16.f, -16.f, -18.f }, { 16.f, 16.f, 18.f }), MASK_SOLID, &filter, trace);
		return trace->fraction < 1.f;
	}

	return true;
}

bool c_ragebot::can_knife(c_cs_player* player, anim_record_t* record, vec3_t angle, bool& stab)
{
	vec3_t forward{};
	math::angle_vectors(angle, forward);

	c_game_trace trace{};
	knife_trace(forward, false, &trace);

	if (!trace.entity || trace.entity != player)
		return false;

	bool armor = player->armor_value() > 0;
	bool first = HACKS->weapon->next_primary_attack() + 0.4f < HACKS->predicted_time;
	bool back = knife_is_behind(player, record);

	int stab_dmg = knife_dmg.stab[armor][back];
	int slash_dmg = knife_dmg.swing[first][armor][back];
	int swing_dmg = knife_dmg.swing[false][armor][back];

	int health = player->health();
	if (health <= slash_dmg)
		stab = false;
	else if (health <= stab_dmg)
		stab = true;
	else if (health > (slash_dmg + swing_dmg + stab_dmg))
		stab = true;
	else
		stab = false;

	if (stab && !knife_trace(forward, true, &trace))
		return false;

	return true;
}

void c_ragebot::knife_bot()
{
	if (!g_cfg.rage.enable)
		return;

	if (HACKS->predicted_time < HACKS->weapon->next_primary_attack() || HACKS->predicted_time < HACKS->weapon->next_secondary_attack())
		return;

	bool supress_doubletap_choke = true;
	if (EXPLOITS->enabled() && EXPLOITS->get_exploit_mode() == EXPLOITS_DT)
		supress_doubletap_choke = EXPLOITS->defensive.tickbase_choke > 2;

	bool best_stab{};
	knife_point_t best{};

	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		if (!player->is_alive() || player->dormant() || player->has_gun_game_immunity())
			return;

		auto anims = ANIMFIX->get_anims(player->index());
		if (!anims || anims->records.empty())
			return;

		auto first_find = std::find_if(anims->records.begin(), anims->records.end(), [&](anim_record_t& record) {
			return record.valid_lc;
			});

		anim_record_t* first = nullptr;
		if (first_find != anims->records.end())
			first = &*first_find;

		restore_record_t backup{};
		backup.store(player);

		if (!first)
		{
			backup.restore(player);
			return;
		}

		{
			{
				LAGCOMP->set_record(player, first, first->matrix_orig.matrix);

				for (auto& a : knife_ang)
				{
					if (!can_knife(player, first, a, best_stab))
						continue;

					best.point = a;
					best.record = first;
					break;
				}
			}

			{
				auto last_find = std::find_if(anims->records.rbegin(), anims->records.rend(), [&](anim_record_t& record) {
					return record.valid_lc;
					});

				anim_record_t* last = nullptr;
				if (last_find != anims->records.rend())
					last = &*last_find;

				if (!last || last == first)
				{
					backup.restore(player);
					return;
				}

				LAGCOMP->set_record(player, last, last->matrix_orig.matrix);

				for (auto& a : knife_ang)
				{
					if (!can_knife(player, last, a, best_stab))
						continue;

					best.point = a;
					best.record = last;
					break;
				}
			}
		}
		backup.restore(player);

		if (best.record)
		{
			backup.restore(player);
			return;
		}
	});

	if (supress_doubletap_choke && best.record)
	{
		HACKS->cmd->viewangles = best.point.normalized_angle();

		if (best.record && !HACKS->cl_lagcomp0)
			HACKS->cmd->tickcount = TIME_TO_TICKS(best.record->sim_time + HACKS->lerp_time);

		HACKS->cmd->buttons.force(best_stab ? IN_ATTACK2 : IN_ATTACK);
	}
}

void c_ragebot::run()
{
	if (!HACKS->weapon || !HACKS->weapon_info || HACKS->client_state->delta_tick == -1)
		return;

	auto_pistol();

	if (EXPLOITS->cl_move.trigger && EXPLOITS->cl_move.shifting)
		return;

	hitboxes.clear();
	hitboxes.reserve(HITBOX_MAX);

	rage_config = main_utils::get_weapon_config();
	update_hitboxes();

	trigger_stop = false;
	should_shot = true;
	reset_rage_hitscan = false;
	firing = false;
	working = false;
	rage_player_iter = 0;
	predicted_eye_pos.reset();
	best_rage_player.reset();

	if (!g_cfg.rage.enable || HACKS->weapon->is_misc_weapon() && !HACKS->weapon->is_taser() && !HACKS->weapon->is_knife())
	{
		reset_rage_players();
		return;
	}

	if (HACKS->weapon->is_knife())
	{
		knife_bot();
		return;
	}

	update_predicted_eye_pos();
	prepare_players_for_scan();

	if (rage_player_iter < 1)
	{
		reset_rage_players();
		return;
	}

	reset_rage_hitscan = true;

	scan_players();
	choose_best_point();

	float lowest_distance = FLT_MAX;

	firing = false;
	working = false;
	best_rage_player.reset();

	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		if (!player->is_alive() || player->dormant() || player->has_gun_game_immunity())
			return;

		auto rage = &rage_players[player->index()];
		if (!rage || !rage->player || rage->player != player)
			return;

		if (!rage->start_scans)
			return;

		if (!rage->best_point.found)
			return;

		if (lowest_distance > rage->distance)
		{
			lowest_distance = rage->distance;
			best_rage_player = *rage;
		}
	});

	const auto& best_point = best_rage_player.best_point;
	if (best_rage_player.player && best_point.found && best_rage_player.start_scans)
	{
		working = true;

		auto local_anims = ANIMFIX->get_local_anims();

		// also, with predictive scans we automatically achieved early auto stop without useless conditions & heavy code (@opai)
		// and auto scope too...

		auto damage = get_min_damage(best_rage_player.player);
		bool already_stooped = false;
		if (best_point.predicted_eye_pos && best_point.damage >= damage && (rage_config.quick_stop_options & early))
		{
			force_scope();

			if (rage_config.quick_stop && should_stop(best_point))
			{
				already_stooped = true;
				trigger_stop = true;
			}
		}

		auto aim_angle = math::calc_angle(local_anims->eye_pos, best_point.aim_point).normalized_angle();
		auto ideal_start = ANIMFIX->get_eye_position(aim_angle.x);

		auto best_record = best_rage_player.best_record;

		// when we have pre-scanned points we won't shoot at it's position from pred eye pos
		// because it's wrong
		// wait untill you will actually see the point and can shoot to it 
		{
			best_rage_player.restore.store(best_rage_player.player);

			auto matrix_to_aim = best_record->extrapolated ? best_record->predicted_matrix : best_record->matrix_orig.matrix;
			LAGCOMP->set_record(best_rage_player.player, best_record, matrix_to_aim);

			auto final_bullet = penetration::simulate(HACKS->local, best_rage_player.player, ideal_start, best_point.aim_point);
			best_rage_player.restore.restore(best_rage_player.player);

			if (final_bullet.damage < damage || HACKS->weapon->is_taser() && final_bullet.penetration_count < 4)
				return;
		}

		if (!should_shot)
			return;

		if (!already_stooped || !(rage_config.quick_stop_options & early))
		{
			force_scope();

			if (rage_config.quick_stop && should_stop(best_point))
				trigger_stop = true;
		}

		bool supress_doubletap_choke = true;
		if (EXPLOITS->enabled() && EXPLOITS->get_exploit_mode() == EXPLOITS_DT)
			supress_doubletap_choke = EXPLOITS->defensive.tickbase_choke > 2;

		if (!supress_doubletap_choke)
			return;

		if (!can_fire())
			return;

		float out_chance = 0.f;
		auto max_hitchance = rage_config.hitchance * 0.01f;

		if (!hitchance(ideal_start, best_rage_player, best_point, best_record, max_hitchance, nullptr, &out_chance))
			return;

		if (g_cfg.rage.auto_fire)
			HACKS->cmd->buttons.force(IN_ATTACK);

		if (HACKS->cmd->buttons.has(IN_ATTACK))
		{
			firing = true;

			auto record_time = best_record->sim_time;

			HACKS->cmd->tickcount = TIME_TO_TICKS(record_time + HACKS->lerp_time);
			auto backtrack_ticks = std::abs(TIME_TO_TICKS(best_rage_player.player->sim_time() - record_time));

			if (g_cfg.visuals.eventlog.logs & 4)
			{
				EVENT_LOGS->push_message(tfm::format(CXOR("Fire to %s [hitbox: %s | hc: %d | sp: %d | dmg: %d | tick: %d]"),
					best_rage_player.player->get_name().c_str(),
					main_utils::hitbox_to_string(best_point.hitbox).c_str(),
					(int)(out_chance * 100.f),
					best_point.safety,
					best_point.damage,
					best_record->extrapolated ? -best_record->extrapolate_ticks : backtrack_ticks), {}, true);
			}

			if (g_cfg.visuals.chams[c_onshot].enable)
				CHAMS->add_shot_record(best_rage_player.player, best_record->matrix_orig.matrix);

			HACKS->cmd->viewangles = math::calc_angle(ideal_start, best_point.aim_point).normalized_angle();
			HACKS->cmd->viewangles -= HACKS->local->aim_punch_angle() * (HACKS->convars.weapon_recoil_scale->get_float());
			HACKS->cmd->viewangles = HACKS->cmd->viewangles.normalized_angle();

			add_shot_record(best_rage_player.player, best_point, best_record, ideal_start);

#ifdef LEGACY
			if (!ANTI_AIM->is_fake_ducking())
			{
				if (g_cfg.binds[hs_b].toggled || g_cfg.binds[dt_b].toggled)
					*HACKS->send_packet = true;
				else
				{
					if (!HACKS->client_state->choked_commands)
						*HACKS->send_packet = false;
				}
			}
#else
			if ((g_cfg.binds[hs_b].toggled || !ANTI_AIM->is_fake_ducking()) && !*HACKS->send_packet)
				*HACKS->send_packet = true;
#endif
		}
	}

	best_rage_player.reset();
}

void c_ragebot::add_shot_record(c_cs_player* player, const rage_point_t& best, anim_record_t* record, vec3_t eye_pos)
{
	auto anims = ANIMFIX->get_local_anims();

	auto& new_shot = shots.emplace_back();
	new_shot.time = HACKS->predicted_time;
	new_shot.init_time = 0.f;
	new_shot.impact_fire = false;
	new_shot.fire = false;
	new_shot.damage = -1;
	new_shot.safety = best.safety;
	new_shot.start = eye_pos;
	new_shot.hitgroup = -1;
	new_shot.hitchance = best.accuracy;
	new_shot.hitbox = best.hitbox;
	new_shot.pointer = player;
	new_shot.record = *record;
	new_shot.index = player->index();
	new_shot.resolver = resolver_info[new_shot.index];
	new_shot.point = best.aim_point;
}

void c_ragebot::weapon_fire(c_game_event* event)
{
	if (shots.empty())
		return;

	if (HACKS->engine->get_player_for_user_id(event->get_int(CXOR("userid"))) != HACKS->engine->get_local_player())
		return;

	auto& shot = shots.front();
	if (!shot.fire)
		shot.fire = true;
}

void c_ragebot::bullet_impact(c_game_event* event)
{
	if (shots.empty())
		return;

	auto& shot = shots.front();

	if (HACKS->engine->get_player_for_user_id(event->get_int(CXOR("userid"))) != HACKS->engine->get_local_player())
		return;

	const auto vec_impact = vec3_t{ event->get_float(CXOR("x")), event->get_float(CXOR("y")), event->get_float(CXOR("z")) };

	bool check = false;
	if (shot.impact_fire)
	{
		if (shot.start.dist_to(vec_impact) > shot.start.dist_to(shot.impact))
			check = true;
	}
	else
		check = true;

	if (!check)
		return;

	shot.impact_fire = true;
	shot.init_time = HACKS->predicted_time;
	shot.impact = vec_impact;
}

void c_ragebot::player_hurt(c_game_event* event)
{
	if (HACKS->engine->get_player_for_user_id(event->get_int(CXOR("attacker"))) != HACKS->engine->get_local_player())
		return;

	if (!shots.empty())
	{
		auto& shot = shots.front();
		shots.erase(shots.begin());
	}
}

void c_ragebot::round_start(c_game_event* event)
{
	for (auto& i : missed_shots)
		i = 0;

	shots.clear();
}

void c_ragebot::on_game_events(c_game_event* event)
{
	auto name = CONST_HASH(event->get_name());

	switch (name)
	{
	case HASH("weapon_fire"):
		weapon_fire(event);
		break;
	case HASH("bullet_impact"):
		bullet_impact(event);
		break;
	case HASH("player_hurt"):
		player_hurt(event);
		break;
	case HASH("round_start"):
		round_start(event);
		break;
	}
}

void c_ragebot::proceed_misses()
{
	if (shots.empty())
		return;

	auto& shot = shots.front();
	if (std::abs(HACKS->predicted_time - shot.time) > 1.f)
	{
		shots.erase(shots.begin());
		return;
	}

	if (shot.init_time != -1.f && shot.index && shot.damage == -1 && shot.fire && shot.impact_fire)
	{
		auto new_player = (c_cs_player*)HACKS->entity_list->get_client_entity(shot.index);
		if (new_player && new_player->is_player() && shot.pointer == new_player)
		{
			const auto studio_model = HACKS->model_info->get_studio_model(new_player->get_model());

			if (studio_model)
			{
				auto& resolver_info = shot.resolver;
				const auto end = shot.impact;

				auto matrix_to_aim = shot.record.extrapolated ? shot.record.predicted_matrix : shot.record.matrix_orig.matrix;

				rage_player_t rage_player{};
				rage_player.player = new_player;
				rage_player.restore.store(new_player);

				LAGCOMP->set_record(new_player, &shot.record, matrix_to_aim);

				if (!can_hit_hitbox(shot.start, end, &rage_player, shot.hitbox, matrix_to_aim, &shot.record))
				{
					float dist = shot.start.dist_to(shot.impact);
					float dist2 = shot.start.dist_to(shot.point);

					if (dist2 > dist)
						EVENT_LOGS->push_message(XOR("Missed shot due to occlusion"));
					else
						EVENT_LOGS->push_message(XOR("Missed shot due to spread"));
				}
				else
				{
					if (new_player->is_alive())
					{
						if (shot.record.extrapolated)
							EVENT_LOGS->push_message(XOR("Missed shot due to extrapolation failure"));
						else if (resolver_info.resolved)
						{
							missed_shots[shot.index]++;
							EVENT_LOGS->push_message(XOR("Missed shot due to resolver"));
						}
						else
							EVENT_LOGS->push_message(XOR("Missed shot due to ?"));
					}
					else
						EVENT_LOGS->push_message(XOR("Missed shot due to death"));
				}

				rage_player.restore.restore(new_player);
			}
		}

		shots.erase(shots.begin());
	}
}