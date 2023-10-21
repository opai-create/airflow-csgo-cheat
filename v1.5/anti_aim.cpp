#include "globals.hpp"
#include "anti_aim.hpp"
#include "movement.hpp"
#include "game_movement.hpp"
#include "fake_lag.hpp"
#include "penetration.hpp"
#include "engine_prediction.hpp"
#include "animations.hpp"
#include "entlistener.hpp"
#include "esp.hpp"
#include "lagcomp.hpp"
#include "ragebot.hpp"
#include "exploits.hpp"
#include "cmd_shift.hpp"
#include <DirectXMath.h>

bool can_fake_duck()
{
	return g_cfg.binds[fd_b].toggled && (MOVEMENT->on_ground() && !(HACKS->cmd->buttons.has(IN_JUMP)));
}

void c_anti_aim::fake_duck()
{
#ifndef LEGACY
	auto state = HACKS->local->animstate();
	if (!state)
		return;

	HACKS->cmd->buttons.force(IN_BULLRUSH);

	static bool start = true;
	if (HACKS->game_rules->is_valve_ds() || HACKS->game_rules->is_freeze_time())
	{
		fake_ducking = false;
		start = true;
		return;
	}

	if (HACKS->local->flags().has(FL_FROZEN) || !MOVEMENT->on_ground())
	{
		fake_ducking = false;
		start = true;
		return;
	}

	if (g_cfg.binds[fd_b].toggled)
	{
		if (start)
		{
			if (HACKS->client_state->choked_commands > 0)
			{
				fake_ducking = false;
				return;
			}
			else
				HACKS->cmd->buttons.remove(IN_DUCK);

			start = false;
			fake_ducking = false;
			return;
		}

		if (HACKS->client_state->choked_commands < 7)
			HACKS->cmd->buttons.remove(IN_DUCK);
		else
			HACKS->cmd->buttons.force(IN_DUCK);

		fake_ducking = true;
	}
	else
	{
		fake_ducking = false;

		start = true;
	}
#endif
}

INLINE int get_ticks_to_stop()
{
	auto vel = HACKS->local->velocity();

	int ticks_to_stop = 0;
	while (true)
	{
		if (vel.length_2d() < 1.f)
			break;
		
		game_movement::friction(vel);

		ticks_to_stop++;
	}
	return ticks_to_stop;
}

void c_anti_aim::slow_walk()
{
	if (!HACKS->weapon_info || !MOVEMENT->on_ground() || RAGEBOT->trigger_stop)
		return;

	auto velocity = HACKS->local->velocity();

#ifdef LEGACY
	if (g_cfg.binds[sw_b].toggled)
	{
		HACKS->cmd->buttons.remove(IN_SPEED);

		int ticks = get_ticks_to_stop();
		if (ticks > (13 - HACKS->client_state->choked_commands) || !HACKS->client_state->choked_commands)
			game_movement::force_stop();
	}

#else
	auto max_speed = HACKS->local->is_scoped() ? HACKS->weapon_info->max_speed_alt : HACKS->weapon_info->max_speed;

	if (g_cfg.binds[sw_b].toggled)
		game_movement::modify_move(*HACKS->cmd, velocity, max_speed * 0.34f);
	else
	{
		auto max_speed = HACKS->local->is_scoped() ? HACKS->weapon_info->max_speed_alt : HACKS->weapon_info->max_speed;

		float tickrate_rate = 5.f / HACKS->tick_rate;

		float rate = ((HACKS->cmd->command_number % HACKS->tick_rate) * tickrate_rate) + 95.f;
		float strength = std::clamp(rate, 95.f, 100.f);

		float new_max_speed = (strength / 100.0f) * max_speed;
		game_movement::modify_move(*HACKS->cmd, velocity, new_max_speed);
	}
#endif
}

void c_anti_aim::force_move()
{
	auto animstate = HACKS->local->animstate();
	if (!animstate)
		return;

	if (!g_cfg.antihit.desync)
		return;

	auto peek_info = MOVEMENT->get_peek_info();
	if (peek_info.peek_execute && peek_info.start_pos.dist_to(HACKS->local->origin()) > 1.f)
		return;

	float speed = HACKS->local->velocity().length_2d();
	if (speed > 10.f)
		return;

	//if (cmd_shift::shifting)
	//{
	//	g_ctx.cmd->forwardmove = 0.f;
	//	return;
	//}

	auto holding_w = HACKS->cmd->buttons.has(IN_FORWARD);
	auto holding_a = HACKS->cmd->buttons.has(IN_MOVELEFT);
	auto holding_s = HACKS->cmd->buttons.has(IN_BACK);
	auto holding_d = HACKS->cmd->buttons.has(IN_MOVERIGHT);

	auto moving = holding_w || holding_a || holding_s || holding_d;

	bool ready_to_move = !moving && MOVEMENT->on_ground();
	if (ready_to_move)
	{
		float modifier = (animstate->anim_duck_amount > 0.f || fake_ducking) ? 3.25f : 1.01f;
		if (g_cfg.antihit.distortion)
			modifier *= -2.f;

		HACKS->cmd->forwardmove = flip_move ? -modifier : modifier;
	}

	flip_move = !flip_move;
}

void c_anti_aim::extended_fake()
{
	if (!g_cfg.antihit.distortion || HACKS->valve_ds)
		return;

	if (!*HACKS->send_packet)
		return;

	auto max_speed = HACKS->local->is_scoped() ? HACKS->weapon_info->max_speed_alt : HACKS->weapon_info->max_speed;
	max_speed *= 0.34f;

	auto speed = HACKS->local->velocity().length_2d();

	bool ensure_lean = (max_speed / 3.4f) >= speed || g_cfg.binds[sw_b].toggled || g_cfg.binds[ens_lean_b].toggled;

	auto holding_w = HACKS->cmd->buttons.has(IN_FORWARD);
	auto holding_a = HACKS->cmd->buttons.has(IN_MOVELEFT);
	auto holding_s = HACKS->cmd->buttons.has(IN_BACK);
	auto holding_d = HACKS->cmd->buttons.has(IN_MOVERIGHT);

	auto moving = holding_w || holding_a || holding_s || holding_d;

	auto choke = FAKE_LAG->get_choke_amount();
	float max_roll = ensure_lean ? 50.f : 47.f;
	if (g_cfg.antihit.distortion_pitch > 0 && choke >= 14)
		max_roll = 94.f;

	float roll_angle = max_roll * ((float)g_cfg.antihit.distortion_range / 100.f) * (-fake_side);

	if (g_cfg.binds[ens_lean_b].toggled || !moving)
		HACKS->cmd->viewangles.z = roll_angle;
}

void c_anti_aim::manual_yaw()
{
	if (g_cfg.binds[left_b].toggled)
		best_yaw -= 90.f;

	if (g_cfg.binds[right_b].toggled)
		best_yaw += 90.f;
}

void c_anti_aim::automatic_edge()
{
	edging = false;

	if (g_cfg.binds[freestand_b].toggled || !g_cfg.binds[edge_b].toggled || !MOVEMENT->on_ground() || g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled)
		return;

	float best_dist = FLT_MAX;
	float best_edge_yaw = FLT_MAX;
	vec3_t best_pos = {};

	vec3_t start = HACKS->local->get_abs_origin() + vec3_t(0.f, 0.f, HACKS->local->view_offset().z / 2.f);

	for (float step = 0.f; step <= 2.f * M_PI; step += DEG2RAD(18.f))
	{
		float x = 40.f * std::cos(step);
		float y = 40.f * std::sin(step);

		vec3_t end = vec3_t(start.x + x, start.y + y, start.z);

		c_trace_filter filter;
		filter.skip = HACKS->local;

		c_game_trace trace = {};
		HACKS->engine_trace->trace_ray(ray_t(start, end), CONTENTS_SOLID | CONTENTS_GRATE, &filter, &trace);

		if (trace.entity && (trace.entity->is_player() || trace.entity->is_weapon()))
			continue;

		if (trace.fraction < 1.f)
		{
			float dist = start.dist_to(trace.end);
			if (best_dist > dist)
			{
				best_edge_yaw = RAD2DEG(step);
				best_pos = trace.end;
				best_dist = dist;
			}
		}
	}

	if (best_edge_yaw == FLT_MAX)
		return;

	if (g_cfg.antihit.yaw > 0)
		best_edge_yaw -= 180.f;

	edging = true;
	best_yaw = math::normalize_yaw(best_edge_yaw);
}

void c_anti_aim::freestanding()
{
	if (!g_cfg.binds[freestand_b].toggled || !MOVEMENT->on_ground() || g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled)
		return;

	auto player = get_closest_player();
	if (!player)
		return;

	auto weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(player->active_weapon()));
	if (!weapon)
		return;

	auto weapon_info = HACKS->weapon_system->get_weapon_data(weapon->item_definition_index());
	if (!weapon_info)
		return;

	static float auto_dir{}, auto_dist{};

	auto update_dir = [&]()
	{
		constexpr float STEP{ 4.f };
		constexpr float RANGE{ 20.f };

		auto anim = ANIMFIX->get_local_anims();

		std::vector< c_adaptive_angle > angles{ };
		angles.emplace_back(best_yaw - 180.f);
		angles.emplace_back(best_yaw - 90.f);
		angles.emplace_back(best_yaw + 90.f);

		vec3_t start = player->get_eye_position();
		bool valid{ false };

		for (auto it = angles.begin(); it != angles.end(); ++it) 
		{
			vec3_t end { 
				anim->eye_pos.x + std::cos(DEG2RAD(it->yaw)) * RANGE,
				anim->eye_pos.y + std::sin(DEG2RAD(it->yaw)) * RANGE,
				anim->eye_pos.z 
			};

			vec3_t dir = end - start;
			float len = dir.normalized_float();

			if (len <= 0.f)
				continue;

			for (float i{ 0.f }; i < len; i += STEP) 
			{
				vec3_t point = start + (dir * i);
				int contents = HACKS->engine_trace->get_point_contents(point, MASK_SHOT_HULL);
				if (!(contents & MASK_SHOT_HULL))
					continue;

				float mult = 1.f;
				if (i > (len * 0.5f))
					mult = 1.25f;

				if (i > (len * 0.75f))
					mult = 1.25f;

				if (i > (len * 0.9f))
					mult = 2.f;

				it->distance += (STEP * mult);

				valid = true;
			}
		}

		if (!valid) 
		{
			auto_dir = math::normalize_yaw(best_yaw - 180.f);
			auto_dist = -1.f;
			return;
		}

		std::sort(angles.begin(), angles.end(),
			[](const c_adaptive_angle& a, const c_adaptive_angle& b) {
				return a.distance > b.distance;
			});

		c_adaptive_angle* best = &angles.front();

		if (best->distance != auto_dist) 
		{
			auto_dir = math::normalize_yaw(best->yaw);
			auto_dist = best->distance;
		}
	};

	update_dir();

	best_yaw = math::normalize_yaw(auto_dir - 180.f);
}

void c_anti_aim::at_targets()
{
	if (!g_cfg.antihit.at_targets || g_cfg.binds[edge_b].toggled || g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled)
		return;

	auto player = get_closest_player(false, true);
	if (!player)
		return;

	best_yaw = math::normalize_yaw(math::calc_angle(HACKS->local->get_abs_origin(), player->get_abs_origin()).y);
}

void c_anti_aim::fake()
{
	auto state = HACKS->local->animstate();
	if (!state)
		return;

	if (!g_cfg.antihit.desync || cmd_shift::shifting || EXPLOITS->cl_move.trigger && EXPLOITS->cl_move.shifting)
		return;

	auto vars = ANIMFIX->get_local_anims();

	float angle = 0.f;

	if (g_cfg.antihit.random_amount)
	{
		math::random_seed(HACKS->global_vars->tickcount);
		angle = fake_side == 1 ? 58 - math::random_int(0, 58) : math::random_int(0, 58);
	}
	else
		angle = fake_side == 1 ? g_cfg.antihit.desync_left : g_cfg.antihit.desync_right;

	float desync_range = angle * vars->aim_matrix_width_range;
	float desync_max = vars->max_desync_range;

	auto choke = FAKE_LAG->get_choke_amount();
	if (!*HACKS->send_packet)
		best_yaw += desync_range * 2.f * fake_side;
}

c_cs_player* c_anti_aim::get_closest_player(bool skip, bool local_distance)
{
	c_cs_player* best = nullptr;
	float best_dist = FLT_MAX;

	auto center = vec2_t(RENDER->screen.x * 0.5f, RENDER->screen.y * 0.5f);

	vec3_t view_angles{};
	HACKS->engine->get_view_angles(view_angles);

	auto local_anim = ANIMFIX->get_local_anims();

	vec3_t local_eye_pos = local_anim->eye_pos;

	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		if (!player->is_alive() || player->has_gun_game_immunity())
			return;
		
		auto esp = ESP->get_esp_player(player->index());
		if (skip)
		{
			if (player->dormant())
				return;
		}
		else
		{
			if (!esp->valid)
				return;
		}

		auto valid_dormant = player->dormant() && (std::abs(esp->dormant.time - HACKS->global_vars->curtime) < 5.f);

		auto base_origin = valid_dormant && esp->dormant.origin.valid() ? esp->dormant.origin : player->get_abs_origin();
		base_origin += vec3_t(0.f, 0.f, player->view_offset().z / 2.f);

		vec2_t origin = {};
		RENDER->world_to_screen(base_origin, origin);

		auto angle = math::calc_angle(local_eye_pos, base_origin);

		float dist = local_distance ? math::get_fov(view_angles, angle) : center.dist_to(origin);
		if (dist < best_dist)
		{
			best = player;
			best_dist = dist;
		}
	});

	return best;
}

vec3_t get_predicted_pos()
{
	auto updated_vars = ANIMFIX->get_local_anims();
	auto unpred_vars = ENGINE_PREDICTION->get_unpredicted_vars();

	const int max_ticks = 17;
	const auto& velocity = unpred_vars->velocity;

	auto max_speed = HACKS->local->is_scoped() ? HACKS->weapon_info->max_speed_alt : HACKS->weapon_info->max_speed;

	float speed = std::max< float >(velocity.length_2d(), 1.f);
	int max_stop_ticks = std::max< int >(((speed / max_speed) * 5.f) - 1, 0);
	int max_predict_ticks = std::clamp(max_ticks - max_stop_ticks, 0, max_ticks);
	if (max_predict_ticks == 0)
		return {};

	vec3_t last_predicted_velocity = unpred_vars->velocity;
	for (int i = 0; i < max_predict_ticks; ++i)
	{
		auto pred_velocity = unpred_vars->velocity * TICKS_TO_TIME(i + 1);

		vec3_t local_origin = updated_vars->eye_pos + pred_velocity;
		auto flags = HACKS->local->flags();

		game_movement::extrapolate(HACKS->local, local_origin, pred_velocity, flags, flags.has(FL_ONGROUND));

		last_predicted_velocity = pred_velocity;
	}

	return last_predicted_velocity;
}

bool c_anti_aim::is_peeking()
{
	auto updated_vars = ANIMFIX->get_local_anims();

	if (!updated_vars || !HACKS->local || !HACKS->weapon || !HACKS->weapon_info 
		|| !updated_vars->foot_yaw || !HACKS->in_game || HACKS->client_state->delta_tick == -1)
		return false;

	if (!RAGEBOT->can_fire() || RAGEBOT->is_shooting())
		return false;

	auto& local_cache = HACKS->local->bone_cache();
	if (!local_cache.base() || !local_cache.count())
		return false;

	auto player = get_closest_player(false, true);
	if (!player)
		return false;

	auto predicted_velocity = get_predicted_pos();
	auto predicted_eye_pos = updated_vars->eye_pos + predicted_velocity;

	bool can_peek = false;
	auto esp = ESP->get_esp_player(player->index());
	if (!predicted_eye_pos.valid() || !esp->valid)
		return false;

	auto valid_dormant = player->dormant() && (std::abs(esp->dormant.time - HACKS->global_vars->curtime) <= 5.f);
	auto origin = valid_dormant && esp->dormant.origin.valid() ? esp->dormant.origin : player->get_abs_origin();

	static matrix3x4_t predicted_matrix[128]{};
	std::memcpy(predicted_matrix, local_cache.base(), sizeof(predicted_matrix));

	if (player->dormant())
	{
		vec3_t poses[3]{ origin, origin + player->view_offset(), origin + vec3_t(0.f, 0.f, player->view_offset().z / 2.f) };

		for (int i = 0; i < 3; ++i)
		{
			c_trace_filter filter{};
			filter.skip = HACKS->local;

			c_game_trace out{};
			HACKS->engine_trace->trace_ray(ray_t(predicted_eye_pos, poses[i]), MASK_SHOT | CONTENTS_HITBOX, &filter, &out);

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
		auto anims = ANIMFIX->get_anims(player->index());
		if (!anims)
			return false;

		auto weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(player->active_weapon()));
		if (!weapon)
			return false;

		auto weapon_info = HACKS->weapon_system->get_weapon_data(weapon->item_definition_index());
		if (!weapon_info)
			return false;

		// detect if you can get dmg by enemy
		auto predicted_origin = HACKS->local->origin() + predicted_velocity;
		math::change_bones_position(predicted_matrix, 128, HACKS->local->origin(), predicted_origin);
		{
			auto head_pos = HACKS->local->get_hitbox_position(0, predicted_matrix);

			auto old_abs_origin = HACKS->local->get_abs_origin();
			static matrix3x4_t old_cache[128]{};
			HACKS->local->store_bone_cache(old_cache);
			{
				HACKS->local->set_abs_origin(predicted_origin);
				HACKS->local->set_bone_cache(predicted_matrix);

				HACKS->local->set_abs_origin(predicted_eye_pos);
				auto eyepos_awall = penetration::simulate(player, HACKS->local, player->get_eye_position(), predicted_eye_pos, false, true);
			//	HACKS->debug_overlay->add_text_overlay(predicted_eye_pos, 0.1f, "%d", eyepos_awall);

				for (auto& i : hitbox_list)
				{
					auto hitbox_position = HACKS->local->get_hitbox_position(i, local_cache.base());
					HACKS->local->set_abs_origin(hitbox_position);
					auto awall = penetration::simulate(player, HACKS->local, player->get_eye_position(), hitbox_position, false, true);

				//	HACKS->debug_overlay->add_text_overlay(hitbox_position, 0.1f, "! %d", awall.damage);
					if (eyepos_awall.damage >= 1 || awall.damage >= 1)
					{
						can_peek = true;
						break;
					}
				}
			}
			HACKS->local->set_abs_origin(old_abs_origin);
			HACKS->local->set_bone_cache(old_cache);
		}
		math::change_bones_position(predicted_matrix, 128, predicted_origin, HACKS->local->origin());

		if (!anims->records.empty())
		{
			auto first_find = std::find_if(anims->records.begin(), anims->records.end(), [&](anim_record_t& record) {
				return record.valid_lc;
				});

			anim_record_t* first = nullptr;
			if (first_find != anims->records.end())
				first = &*first_find;

			restore_record_t restore{};
			for (const auto& i : hitbox_list)
			{
				auto hitbox_pos = player->get_hitbox_position(i, first ? first->matrix_orig.matrix : nullptr);

				if (first)
				{
					restore.store(player);
					LAGCOMP->set_record(player, first, first->matrix_orig.matrix);
				}

				auto awall = penetration::simulate(HACKS->local, player, updated_vars->eye_pos, hitbox_pos, false, true);
				bool can_hit_point = awall.damage >= 1;

				if (first)
					restore.restore(player);

				if (can_hit_point)
				{
					can_peek = true;
					break;
				}
			}
		}

		if (can_peek)
			return true;
	}

	return can_peek;
}

bool c_anti_aim::is_fake_ducking()
{
#ifndef LEGACY
	return fake_ducking;
#else
	return false;
#endif
}

void c_anti_aim::run_movement()
{
	if (HACKS->local->move_type() == MOVETYPE_LADDER || HACKS->local->move_type() == MOVETYPE_NOCLIP)
		return;

	if (HACKS->game_rules->is_freeze_time() || HACKS->local->flags().has(FL_FROZEN) || HACKS->local->has_gun_game_immunity())
		return;

#ifndef LEGACY
	force_move();
#endif

	fake_duck();
	slow_walk();
}

void c_anti_aim::run()
{
	auto update_tickbase_state = [&]()
	{
#ifndef LEGACY
		static int old_tickbase = 0;

		if (!EXPLOITS->enabled() || (EXPLOITS->get_exploit_mode() != EXPLOITS_DT) || EXPLOITS->cl_move.trigger && EXPLOITS->cl_move.shifting || cmd_shift::shifting)
		{
			//	g_ctx.cmd->viewangles.x = g_ctx.orig_angle.x;

			defensive_aa = false;
			old_tickbase = 0;
			return;
		}

		auto tickbase_diff = HACKS->local->tickbase() - old_tickbase;

		switch (g_cfg.antihit.def_aa_mode)
		{
		case 0:
			defensive_aa = tickbase_diff < 0 || tickbase_diff > 1;
			break;
		case 1:
			defensive_aa = EXPLOITS->defensive.tickbase_choke != 100
				&& EXPLOITS->defensive.tickbase_choke > 0 && HACKS->client_state->choked_commands;
			break;
		}

		old_tickbase = HACKS->local->tickbase();
#endif
	};

	update_tickbase_state();

	if (!HACKS->weapon || !HACKS->weapon_info)
		return;

	if (HACKS->local->move_type() == MOVETYPE_LADDER || HACKS->local->move_type() == MOVETYPE_NOCLIP)
		return;

	if (!g_cfg.antihit.enable)
		return;

	if (HACKS->game_rules->is_freeze_time() || HACKS->local->flags().has(FL_FROZEN))
		return;

	auto state = HACKS->local->animstate();
	if (!state)
		return;

	static int duration{}, dsy_duration{};
	static bool random_flipper{}, random_dsy_flipper{};

	int choke_amount = FAKE_LAG->get_choke_amount();

	if (RAGEBOT->is_shooting())
		shot_cmd = HACKS->cmd->command_number;

	if (HACKS->cmd->buttons.has(IN_USE))
		return;

	if (shot_cmd == HACKS->cmd->command_number)
		return;

	auto holding_w = HACKS->cmd->buttons.has(IN_FORWARD);
	auto holding_a = HACKS->cmd->buttons.has(IN_MOVELEFT);
	auto holding_s = HACKS->cmd->buttons.has(IN_BACK);
	auto holding_d = HACKS->cmd->buttons.has(IN_MOVERIGHT);

	auto moving = holding_w || holding_a || holding_s || holding_d;
	float add_roll = g_cfg.antihit.distortion_pitch;

	switch (g_cfg.antihit.pitch)
	{
	case 1:
	{
		if (g_cfg.antihit.def_pitch && defensive_aa)
		{
			math::random_seed(HACKS->global_vars->tickcount);
			HACKS->cmd->viewangles.x = g_cfg.antihit.def_aa_mode == 1 ? 0.f : -89.f;
		}
		else
		{
#ifndef LEGACY
			if (g_cfg.antihit.distortion_pitch > 0.f && choke_amount >= 14)
			{
				const auto choke = HACKS->client_state->choked_commands + 1;
				for (int i = 1; i <= choke; i++)
				{
					auto cmds = HACKS->input->get_user_cmd(HACKS->cmd->command_number - choke + i);

					cmds->viewangles.x = 89.f + add_roll;
				}
			}
			else
#endif
				HACKS->cmd->viewangles.x = 89.f;
		}
	}
	break;
	case 2:
		HACKS->cmd->viewangles.x = -89.f;
		break;
	}

	start_yaw = HACKS->cmd->viewangles.y;
	best_yaw = HACKS->cmd->viewangles.y;

#ifdef LEGACY
	if (g_cfg.antihit.desync_mode)
		fake_side = flip_side ? 1 : -1;
	else
		fake_side = g_cfg.binds[inv_b].toggled ? -1 : 1;

	static int tick = 0;

	auto do_real = [&]()
	{
		at_targets();
		automatic_edge();
		freestanding();
		manual_yaw();

		switch (g_cfg.antihit.yaw)
		{
		case 1:
			best_yaw += 180.f;
			break;
		case 2:
			best_yaw += 360.f + 90.f + std::fmod(HACKS->global_vars->curtime * 360.f, 180.f);
			break;
		}

		float range = g_cfg.antihit.jitter_range * 0.5f;

		switch (g_cfg.antihit.jitter_mode)
		{
		case 1:
			best_yaw += flip_jitter ? range : -range;
			break;
		case 2:
			if (!flip_jitter)
				best_yaw += g_cfg.antihit.jitter_range;
			break;
		case 3:
			best_yaw += math::random_float(-g_cfg.antihit.jitter_range, g_cfg.antihit.jitter_range);
			break;
		case 4:
		{
			switch (tick)
			{
			case 0:
				best_yaw -= g_cfg.antihit.jitter_range;
				break;
			case 2:
				best_yaw += g_cfg.antihit.jitter_range;
				break;
			}
		}break;
		}

		if (!(g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled))
			best_yaw += g_cfg.antihit.yaw_add;

		HACKS->cmd->viewangles.y = math::normalize_yaw(best_yaw);
	};

	auto stand = g_cfg.binds[sw_b].toggled || HACKS->local->velocity().length_2d() < 10.f;
	auto anim = ANIMFIX->get_local_anims();

	if (g_cfg.antihit.desync && EXPLOITS->defensive.tickbase_choke > 2)
	{
		if (!*HACKS->send_packet)
		{
			do_real();

			if (stand && !HACKS->client_state->choked_commands && HACKS->global_vars->curtime >= anim->last_lby_time)
			{
				fake_side = g_cfg.binds[inv_b].toggled ? -1 : 1;

				*HACKS->send_packet = true;

				float lby_delta = g_cfg.antihit.desync_left * fake_side;
				HACKS->cmd->viewangles.y += lby_delta;

				flip_side = !flip_side;
			}
		}
		else
		{
			HACKS->cmd->viewangles.y = math::normalize_yaw(math::random_float(-180.f, 180.f));
		}
	}
	else
		do_real();

	if (*HACKS->send_packet)
	{
		flip_side = !flip_side;
		flip_jitter = !flip_jitter;

		++tick;
		tick %= 3;
	}

#else
	auto dsy_flipper = g_cfg.antihit.random_dsy ? random_dsy_flipper : flip_side;

	if (g_cfg.antihit.desync_mode)
		fake_side = dsy_flipper ? 1 : -1;
	else
		fake_side = g_cfg.binds[inv_b].toggled ? -1 : 1;

	at_targets();
	automatic_edge();
	freestanding();
	extended_fake();
	manual_yaw();
	fake();

	switch (g_cfg.antihit.yaw)
	{
	case 1:
		best_yaw += 180.f;
		break;
	case 2:
		best_yaw += 360.f + 90.f + std::fmod(HACKS->global_vars->curtime * 360.f, 180.f);
		break;
	}

	static int tick = 0;

	float range = g_cfg.antihit.jitter_range * 0.5f;

	auto jitter_flipper = g_cfg.antihit.random_jitter ? random_flipper : flip_jitter;

	switch (g_cfg.antihit.jitter_mode)
	{
	case 1:
		best_yaw += jitter_flipper ? range : -range;
		break;
	case 2:
		if (!jitter_flipper)
			best_yaw += g_cfg.antihit.jitter_range;
		break;
	case 3:
		best_yaw += math::random_float(-g_cfg.antihit.jitter_range, g_cfg.antihit.jitter_range);
		break;
	case 4:
	{
		switch (tick)
		{
		case 0:
			best_yaw -= g_cfg.antihit.jitter_range;
			break;
		case 2:
			best_yaw += g_cfg.antihit.jitter_range;
			break;
		}
	}break;
	}

	if (*HACKS->send_packet)
	{
		math::random_seed(HACKS->global_vars->tickcount);

		auto make_random_timer = [&](bool& flipper, int& timer, int min, int max)
		{
			if (std::abs(HACKS->global_vars->tickcount - timer) > math::random_int(min, max))
			{
				timer = HACKS->global_vars->tickcount;
				flipper = !flipper;
			}
		};

		make_random_timer(random_dsy_flipper, dsy_duration, 1, 4);
		make_random_timer(random_flipper, duration, 2, 5);

		flip_side = !flip_side;
		flip_jitter = !flip_jitter;

		++tick;
		tick %= 3;
	}

	if (!(g_cfg.binds[left_b].toggled || g_cfg.binds[right_b].toggled || g_cfg.binds[back_b].toggled))
		best_yaw += g_cfg.antihit.yaw_add;

	if (g_cfg.antihit.def_yaw && defensive_aa)
	{
		math::random_seed(HACKS->global_vars->tickcount);
		best_yaw += HACKS->global_vars->tickcount % 16 * (360 / 16) - 180;
	}

	HACKS->cmd->viewangles.y = math::normalize_yaw(best_yaw);
#endif
}

void c_anti_aim::cleanup()
{
	HACKS->cmd->viewangles = HACKS->cmd->viewangles.normalized_angle();
	HACKS->cmd->sidemove = std::clamp(HACKS->cmd->sidemove, -450.f, 450.f);
	HACKS->cmd->forwardmove = std::clamp(HACKS->cmd->forwardmove, -450.f, 450.f);
	HACKS->cmd->upmove = std::clamp(HACKS->cmd->upmove, -320.f, 320.f);

#ifdef LEGACY
	auto local_anim = ANIMFIX->get_local_anims();
	if (HACKS->client_state->choked_commands < 1) {
		local_anim->sent_eye_pos = HACKS->cmd->viewangles;
		return;
	}

	if (!*HACKS->send_packet)
		local_anim->sent_eye_pos = HACKS->cmd->viewangles;
#endif
}