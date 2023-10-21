#include "animfix.h"
#include "../features.h"
#include "../../base/sdk/c_csplayerresource.h"
#include "../../base/tools/threads.h"

void c_animation_fix::anim_player_t::adjust_roll_angle(records_t* record)
{
	auto& info = resolver_info[ptr->index()];

	if (g_cfg.binds[force_roll_b].toggled)
	{
		if (info.side != side_zero && info.side != side_original)
			record->eye_angles.z = g_cfg.rage.roll_amt * -info.side;
		else
			record->eye_angles.z = g_cfg.rage.roll_amt;

		if (!record->shooting)
		{
			record->eye_angles.x += g_cfg.rage.roll_amt_pitch;
			record->eye_angles.x = std::min(120.f, record->eye_angles.x);
		}
	}

#if ALPHA || _DEBUG || BETA
	if (last_record && g_cfg.rage.jitterfix)
	{
		if (record->fix_jitter_angle)
		{
			switch (g_cfg.rage.jitterfix_method)
			{
			case 0:
				record->eye_angles.y = math::normalize(last_record->eye_angles.y);
				break;
			case 1:
			{
				if (std::fabsf(record->last_eyeang_diff_time - record->sim_time) < math::ticks_to_time(record->choke))
					record->eye_angles.y += math::normalize(record->last_eyeang_diff);
			}
			break;
			}

			record->fix_jitter_angle = false;
		}
	}

#endif
}

inline simulated_data_t* get_data_by_index(records_t* record, int side)
{
	switch (side)
	{
	case side_left:
		return &record->sim_left;
	case side_right:
		return &record->sim_right;
	case side_zero:
		return &record->sim_zero;
	case side_original:
		return &record->sim_orig;
	}
}

void c_animation_fix::anim_player_t::simulate_animation_side(records_t* record, int side)
{
	auto state = ptr->animstate();
	if (!state)
		return;

	ptr->update_weapon_dispatch_layers();

	backup_globals(cur_time);
	backup_globals(frame_time);

	if (!last_record || record->dormant)
	{
		auto& layers = record->sim_orig.layers;
		
		auto last_update_time = state->last_update_time;

		// this happens only when first record arrived or when enemy is out from dormant
		// so we should set latest data as soon as possible
		state->primary_cycle = layers[6].cycle;
		state->move_weight = layers[6].weight;
		state->acceleration_weight = layers[12].weight;

		// fixes goalfeetyaw on spawn
		state->last_update_time = (record->sim_time - interfaces::global_vars->interval_per_tick);

		if (backup_record.flags & fl_onground)
		{
			state->on_ground = true;
			state->landing = false;
		}

		if (ptr->get_sequence_activity(record->sim_orig.layers[4].sequence) == act_csgo_jump)
		{
			auto land_time = ptr->simulation_time() - (record->sim_orig.layers[4].cycle / record->sim_orig.layers[4].playback_rate);

			if (land_time > last_update_time)
			{
				state->on_ground = true;
				state->landing = false;
				state->last_update_time = land_time;

				ptr->pose_parameter()[6] = 0.0f;
			}
		}
	}
	else
	{
		auto& layers = last_record->sim_orig.layers;
		state->primary_cycle = layers[6].cycle;
		state->move_weight = layers[6].weight;
		state->acceleration_weight = layers[12].weight;
	}

	if (!last_record || record->choke < 2)
	{
		ptr->abs_velocity() = ptr->velocity() = record->velocity_for_animfix;

		if (!teammate)
		{
			if (side != side_original)
				state->abs_yaw = math::normalize(ptr->eye_angles().y + (state->get_max_rotation() * side));

#if ALPHA || _DEBUG || BETA
			if (last_record)
			{
				float diff = math::normalize(last_record->eye_angles.y - record->eye_angles.y);

				record->fix_jitter_angle = std::fabsf(diff) > 10.f;
				record->last_eyeang = last_record->eye_angles.y;
				record->last_eyeang_diff = diff;

				if (record->old_diff != record->last_eyeang_diff)
				{
					record->old_diff = record->last_eyeang_diff;
					record->last_eyeang_diff_time = ptr->simulation_time();
				}
			}
#endif
		}

		this->force_update();
	}
	else
	{
		ptr->lby() = last_record->lby;
		ptr->thirdperson_recoil() = last_record->thirdperson_recoil;

		for (int i = 0; i < record->choke; ++i)
		{
			float new_simtime = record->old_sim_time + math::ticks_to_time(i + 1);
			auto sim_tick = math::time_to_ticks(new_simtime);

			ptr->abs_velocity() = ptr->velocity() = record->velocity;

			if (record->on_ground)
				ptr->flags() |= fl_onground;
			else
				ptr->flags() &= ~fl_onground;

			if (!teammate)
			{
				if (side != side_original)
					state->abs_yaw = math::normalize(ptr->eye_angles().y + (state->get_max_rotation() * side));
				else
					resolver::apply_side(ptr, record->choke);

#if ALPHA || _DEBUG || BETA
				if (last_record)
				{
					float diff = math::normalize(last_record->eye_angles.y - record->eye_angles.y);

					record->fix_jitter_angle = std::fabsf(diff) > 10.f;
					record->last_eyeang = math::normalize(record->eye_angles.y);
					record->last_eyeang_diff = diff;

					if (record->old_diff != record->last_eyeang_diff)
					{
						record->old_diff = record->last_eyeang_diff;
						record->last_eyeang_diff_time = ptr->simulation_time();
					}
				}
#endif
			}

			if (record->shooting)
			{
				if (record->last_shot_time <= new_simtime)
				{
					ptr->lby() = record->lby;
					ptr->thirdperson_recoil() = record->thirdperson_recoil;
				}
			}

			float old_simtime = ptr->simulation_time();
			ptr->simulation_time() = new_simtime;
			this->force_update();
			ptr->simulation_time() = old_simtime;
		}
	}

	if (this->last_record)
	{
		record->sim_orig.layers[12].weight = this->last_record->sim_orig.layers[12].weight;
		record->sim_orig.layers[12].cycle = this->last_record->sim_orig.layers[12].cycle;
	}
	else
	{
		record->sim_orig.layers[12].weight = 0.f;
		record->sim_orig.layers[12].cycle = 0.f;
	}

	const auto backup_origin = ptr->get_abs_origin();
	ptr->set_abs_origin(ptr->origin());
	{
		auto current_side = get_data_by_index(record, side);

		ptr->store_layer(current_side->layers);
		ptr->set_layer(record->sim_orig.layers);

		ptr->setup_uninterpolated_bones(current_side->bone);

		if (side != side_original && side != side_zero)
		{
			auto old_angles = ptr->eye_angles().z;

			ptr->eye_angles().z = 50.f * -side;
			memcpy_fast(current_side->roll_bone, current_side->bone, sizeof(current_side->bone));

			g_ctx.modify_body[ptr->index()] = true;
			func_ptrs::modify_body_yaw(ptr, current_side->bone, 0x7FF00);
			g_ctx.modify_body[ptr->index()] = false;

			ptr->eye_angles().z = old_angles;
		}
		else
		{
			if (side == side_original)
			{
				auto old_angle = ptr->eye_angles();

				adjust_roll_angle(record);

				ptr->eye_angles() = record->eye_angles;

				g_ctx.modify_body[ptr->index()] = true;
				func_ptrs::modify_body_yaw(ptr, current_side->bone, 0x7FF00);
				g_ctx.modify_body[ptr->index()] = false;

				ptr->eye_angles() = old_angle;
			}
		}
	}
	ptr->set_abs_origin(backup_origin);

	ptr->store_poses(record->poses);

	restore_globals(cur_time);
	restore_globals(frame_time);
}

void c_animation_fix::anim_player_t::build_bones(records_t* record, simulated_data_t* sim)
{
	ptr->setup_uninterpolated_bones(sim->bone, { 0.f, 9999.f, 0.f });
}

void c_animation_fix::anim_player_t::update_animations()
{
	const auto records_size = teammate ? 3 : g_ctx.tick_rate;
	while (records.size() > records_size)
		records.pop_back();

	if (records.size() > 0)
	{
		last_record = &records.front();

		if (records.size() >= 3)
			old_record = &records[2];
	}

	backup_record.update_record(ptr);

	auto& record = records.emplace_front();
	record.update_record(ptr);
	record.update_dormant(dormant_ticks);
	record.update_shot(last_record);

	if (dormant_ticks < 1)
		dormant_ticks++;

	this->fix_land(&record);
	this->fix_velocity(&record);

	if (!teammate)
	{
		resolver::prepare_side(ptr, &record);

		c_animstate old_state{};
		old_state = *ptr->animstate();

		this->simulate_animation_side(&record, side_right);
		*ptr->animstate() = old_state;

		this->simulate_animation_side(&record, side_left);
		*ptr->animstate() = old_state;

		this->simulate_animation_side(&record, side_zero);
		*ptr->animstate() = old_state;
	}

	this->simulate_animation_side(&record);

	if (g_ctx.lagcomp)
	{
		if (old_simulation_time > record.sim_time)
		{
			record.shooting = false;
			record.last_shot_time = 0.f;
			record.shifting = true;
		}
		else
			old_simulation_time = record.sim_time;

		if (!record.shifting && last_record)
			record.breaking_lc = record.choke > 3 && (record.origin - last_record->origin).length_sqr() > 4096.f;
	}
	else
		record.shifting = record.breaking_lc = false;

	last_record = nullptr;
	old_record = nullptr;
	backup_record.restore(ptr);
}

void thread_anim_update(c_animation_fix::anim_player_t* player)
{
	player->update_animations();
}

void c_animation_fix::force_data_for_render()
{
	if (!g_ctx.in_game || !g_ctx.local || g_ctx.uninject)
		return;

	auto& players = g_listener_entity->get_entity(ent_player);
	if (players.empty())
		return;

	for (auto& player : players)
	{
		auto entity = (c_csplayer*)player.entity;
		if (!entity || entity == g_ctx.local)
			continue;

		if (!entity->is_alive())
		{
			g_ctx.setup_bones[entity->index()] = g_ctx.modify_body[entity->index()] = true;
			continue;
		}

		auto animation_player = this->get_animation_player(entity->index());
		if (!animation_player || animation_player->records.empty() || animation_player->dormant_ticks < 1)
		{
			g_ctx.setup_bones[entity->index()] = g_ctx.modify_body[entity->index()] = true;
			continue;
		}

		auto first_record = &animation_player->records.front();
		if (!first_record || !first_record->sim_orig.bone)
		{
			g_ctx.setup_bones[entity->index()] = g_ctx.modify_body[entity->index()] = true;
			continue;
		}

		/*	main_utils::draw_hitbox(entity, first_record->sim_left.bone, 0, 0, true);
			main_utils::draw_hitbox(entity, first_record->sim_right.bone, 1, 0, true);
			main_utils::draw_hitbox(entity, first_record->sim_zero.bone, 1, 1, true);
			main_utils::draw_hitbox(entity, first_record->sim_orig.bone, 0, 1, true);*/

		memcpy_fast(first_record->render_bones, first_record->sim_orig.bone, sizeof(first_record->render_bones));

		math::change_matrix_position(first_record->render_bones, 128, first_record->origin, entity->get_render_origin());

		entity->interpolate_moveparent_pos();
		entity->set_bone_cache(first_record->render_bones);
		entity->attachments_helper();
	}
}

void c_animation_fix::on_net_update_and_render_after(int stage)
{
	if (!g_ctx.in_game || !g_ctx.local || g_ctx.uninject)
		return;

	auto& players = g_listener_entity->get_entity(ent_player);
	if (players.empty())
		return;

	switch (stage)
	{
	case frame_net_update_end:
	{
		patterns::cl_fireevents.as<void(*)()>()();

#ifdef _DEBUG
		g_ctx.local->personal_data_public_level() = 14880;
#else
		if (g_ctx.is_boss)
			g_ctx.local->personal_data_public_level() = 14880;
		else
			g_ctx.local->personal_data_public_level() = 25120;
#endif

		g_rage_bot->on_pre_predict();

		std::vector<std::uint64_t> tasks{};

		for (auto& player : players)
		{
			auto ptr = (c_csplayer*)player.entity;
			if (!ptr)
				continue;

			if (ptr == g_ctx.local)
			{
				if (ptr->is_alive())
					esp_share::share_player_position(ptr);

				continue;
			}

			auto anim_player = this->get_animation_player(ptr->index());
			if (anim_player->ptr != ptr)
			{
				anim_player->reset_data();
				anim_player->ptr = ptr;
				continue;
			}

			if (!ptr->is_alive())
			{
				if (!anim_player->teammate)
				{
					//	resolver::reset_info(ptr);
					g_rage_bot->missed_shots[ptr->index()] = 0;
				}

				anim_player->ptr = nullptr;
				continue;
			}

			if (g_cfg.misc.force_radar && ptr->team() != g_ctx.local->team())
				ptr->target_spotted() = true;

			if (ptr->dormant())
			{
				anim_player->dormant_ticks = 0;

				//	if (!anim_player->teammate)
				//		resolver::reset_info(ptr);

				anim_player->records.clear();
				anim_player->last_record = nullptr;
				anim_player->old_record = nullptr;

				continue;
			}

			auto& layer = ptr->anim_overlay()[11];
			if (layer.cycle == anim_player->old_aliveloop_cycle)
			{
				ptr->simulation_time() = ptr->old_simtime();
				continue;
			}

			anim_player->old_aliveloop_cycle = layer.cycle;

			if (ptr->simulation_time() <= ptr->old_simtime())
				continue;

			esp_share::share_player_position(ptr);

			auto state = ptr->animstate();
			if (!state)
				continue;

			if (anim_player->old_spawn_time != ptr->spawn_time())
			{
				state->player = ptr;
				state->reset();

				anim_player->old_spawn_time = ptr->spawn_time();
			}

			anim_player->teammate = g_ctx.local && g_ctx.local->team() == ptr->team();

			tasks.emplace_back(g_thread_pool->add_task(thread_anim_update, anim_player));
		}

		for (auto& task : tasks)
			g_thread_pool->wait(task);

		for (auto& player : players)
		{
			auto ptr = (c_csplayer*)player.entity;
			if (!ptr)
				continue;

			if (ptr == g_ctx.local)
				continue;

			auto anim_player = this->get_animation_player(ptr->index());
			if (anim_player->ptr != ptr)
				continue;

			if (!ptr->is_alive())
				continue;

			if (ptr->dormant())
				continue;

			if (ptr->simulation_time() <= ptr->old_simtime())
				continue;

			if (*(int*)((std::uintptr_t)ptr + 0x28C0) != -1)
			{
				using fn = void(__thiscall*)(void*, int);
				g_memory->getvfunc<fn>(ptr, 108)(ptr, 1);
			}
		}
	}
	break;
	}
}

void c_animation_fix::update_valid_ticks()
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

		auto anim_player = this->get_animation_player(entity->index());
		if (anim_player->records.empty())
			continue;

		for (auto& i : anim_player->records)
			i.valid_tick = g_ctx.is_alive ? i.is_valid() : true;
	}
}

bool records_t::is_valid(bool deadtime)
{
	auto netchan = interfaces::engine->get_net_channel_info();
	if (!netchan)
		return false;

	if (!g_ctx.lagcomp)
		return true;

	if (this->shifting)
		return false;

	float time = g_ctx.is_alive ? g_ctx.predicted_curtime : interfaces::global_vars->cur_time;
	float outgoing = g_ctx.real_ping;

	float correct = 0.f;

	correct += g_ctx.ping;
	correct += g_ctx.lerp_time;
	correct = std::clamp<float>(correct, 0.0f, cvars::sv_maxunlag->get_float());

	float delta_time = correct - (time - this->sim_time);

	if (std::fabsf(delta_time) > 0.2f)
		return false;

	auto extra_choke = 0;

	if (g_anti_aim->is_fake_ducking())
		extra_choke = 14 - interfaces::client_state->choked_commands;

	auto server_tickcount = interfaces::global_vars->tick_count + g_ctx.ping + extra_choke;
	auto dead_time = (int)(float)((float)((int)((float)((float)server_tickcount 
		* interfaces::global_vars->interval_per_tick) - 0.2f) / interfaces::global_vars->interval_per_tick) + 0.5f);

	if (math::time_to_ticks(this->sim_time + g_ctx.lerp_time) < dead_time)
		return false;

	return true;
}