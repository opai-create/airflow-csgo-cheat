#include "globals.hpp"
#include "features.hpp"

#define ADD_DETOUR(name) hooker::add_detour(offsets::##name.cast<std::uint64_t>(), ##name)

namespace hooks::detour
{
	void __fastcall get_exposure_range(float* min, float* max)
	{
		static auto original = hooker::get_original(&get_exposure_range);

		// adjust exposure for every map 
		*min = 1.f;
		*max = 1.f;

		original(min, max);
	}

	void __fastcall calc_view_model_bob(void* ecx, void* edx, vec3_t& position)
	{
	//	CHECKMEM;

		static auto original = hooker::get_original(&calc_view_model_bob);
		original(ecx, edx, position);
	}

	void __fastcall physics_simulate(c_cs_player* ecx, void* edx)
	{
		static auto original = hooker::get_original(&physics_simulate);
		int& simulation_tick = *(int*)((std::uintptr_t)ecx + 0x2AC);

		if (!ecx || !ecx->is_alive()
			|| HACKS->global_vars->tickcount == simulation_tick
			|| ecx != HACKS->local
			|| HACKS->engine->is_hltv()
			|| ecx->flags().has(FL_ATCONTROLS))
		{
			original(ecx, edx);
			return;
		}

		auto& ctx = ecx->cmd_context();
		if (ctx.user_cmd.tickcount == INT_MAX)
		{
			auto old_tick = ctx.user_cmd.command_number - 1;
			auto vars = PREDFIX->get_compressed_netvars(old_tick);

			if (vars->tickbase == HACKS->local->tickbase())
				PREDFIX->fix_netvars(old_tick);

			PREDFIX->store(ctx.user_cmd.command_number);

			ctx.needs_processing = false;
			HACKS->global_vars->tickcount = simulation_tick;
			return;
		}

		if (ctx.user_cmd.buttons.has(IN_ATTACK | IN_ATTACK2))
			HACKS->local->looking_at_weapon() = false;

		if (ctx.user_cmd.command_number == HACKS->client_state->last_outgoing_command + HACKS->client_state->choked_commands + 1)
			HACKS->local->velocity_modifier() = PREDFIX->velocity_modifier;

		original(ecx, edx);
	}

	void __fastcall update_postscreen_effects(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&update_postscreen_effects);

		if (!HACKS->local || !HACKS->local->is_alive())
			return original(ecx, edx);

		if (HACKS->weapon && HACKS->weapon->is_sniper() && (g_cfg.misc.removals & scope))
		{
			// remove shaders
			RESTORE(HACKS->local->is_scoped());

			HACKS->local->is_scoped() = false;

			original(ecx, edx);
			return;
		}

		original(ecx, edx);
	}

	bool __fastcall draw_fog(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&draw_fog);

		if ((g_cfg.misc.removals & fog_) && !g_cfg.misc.custom_fog)
			return false;

		return original(ecx, edx);
	}

	int __fastcall send_datagram(c_net_channel* ecx, void* edx, void* datagram)
	{
		static auto original = hooker::get_original(&send_datagram);
#ifdef _DEBUG
		if (HACKS->unload)
			return original(ecx, edx, datagram);
#endif

		if (ecx != HACKS->client_state->net_channel || !HACKS->in_game || !HACKS->local || !HACKS->local->is_alive() || !g_cfg.binds[spike_b].toggled)
			return original(ecx, edx, datagram);

		auto netchannel_info = HACKS->engine->get_net_channel();

		float spike_amount = (g_cfg.rage.spike_amt / 1000.f);
		spike_amount -= netchannel_info->get_latency(FLOW_OUTGOING);
		spike_amount -= HACKS->lerp_time;

		float correct = std::clamp(spike_amount, 0.f, HACKS->max_unlag);

		RESTORE(ecx->in_sequence_nr);
		RESTORE(ecx->in_reliable_state);

		PING_SPIKE->on_net_chan(ecx, correct);

		return original(ecx, edx, datagram);
	}

	bool __fastcall temp_entities(c_client_state* ecx, void* edx, void* msg)
	{
		static auto original = hooker::get_original(&temp_entities);
		return original(ecx, edx, msg);
	}

	bool __fastcall send_net_msg(i_net_channel_info* ecx, void* edx, c_net_message& msg, bool force_reliable, bool voice)
	{
		static auto original = hooker::get_original(&send_net_msg);

		if (ecx != HACKS->engine->get_net_channel())
			return original(ecx, edx, msg, force_reliable, voice);

		if (g_cfg.misc.bypass_sv_pure && msg.get_type() == 14)
			return false;

		return original(ecx, edx, msg, force_reliable, voice);
	}

	bool __fastcall using_static_prop_debug(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&using_static_prop_debug);
		return original(ecx, edx);
	}

	bool __fastcall msg_voice_data(void* ecx, void* edx, c_svc_msg_voice_data* message)
	{
		static auto original = hooker::get_original(&msg_voice_data);
		return original(ecx, edx, message);
	}

	void __vectorcall read_packets(bool final_tick)
	{
		static auto original = hooker::get_original(&read_packets);
	//	CHECKMEM;

		if (!PREDFIX->should_reduce_ping())
			original(final_tick);
	}

	void __vectorcall cl_move(float accumulated_extra_samples, bool final_tick)
	{
		static auto original = hooker::get_original(&cl_move);
		PREDFIX->update_ping_values(final_tick);

		if (!HACKS->client_state || !HACKS->client_state->net_channel || !HACKS->local || !HACKS->local->is_alive())
		{
			if (!HACKS->last_outgoing_commands.empty())
				HACKS->last_outgoing_commands.clear();

			original(accumulated_extra_samples, final_tick);
			return;
		}

		if (HACKS->client_state && EXPLOITS->should_recharge())
			return;

		ENGINE_PREDICTION->update();

		original(accumulated_extra_samples, final_tick);
		EXPLOITS->shift_clmove(accumulated_extra_samples, final_tick);
	}

	void __fastcall process_packet(c_net_channel* ecx, void* edx, void* packet, bool header)
	{
		static auto original = hooker::get_original(&process_packet);
		original(ecx, edx, packet, header);
		
		// remove event delay and execute it after it recieved
		// prevent delays or other issues

		// TO-DO: remove hardcoded offset
#ifdef LEGACY
		auto events = *reinterpret_cast<c_event_info**>(reinterpret_cast<std::uintptr_t>(HACKS->client_state) + XORN(0x4DEC));
#else
		auto events = *reinterpret_cast<c_event_info**>(reinterpret_cast<std::uintptr_t>(HACKS->client_state) + XORN(0x4E6C));
#endif
		if (events) 
		{
			auto iter = events;
			while (iter) 
			{
				if (iter->class_id)
					iter->delay = 0.f;

				iter = iter->next;
			}
		}

		HACKS->engine->fire_events();
		PING_SPIKE->on_procces_packet();
	}

	void __fastcall get_color_modulation(i_material* ecx, void* edx, float* r, float* g, float* b)
	{
		static auto original = hooker::get_original(&get_color_modulation);
		original(ecx, edx, r, g, b);

		WORLD_MODULATION->override_world_color(ecx, r, g, b);
	}

	int process_interpolated_list()
	{
		static auto original = hooker::get_original(&process_interpolated_list);

		static auto allow_extrapolation = *offsets::allow_extrapolation.add(XORN(1)).cast< bool** >();

		// turn off extrapolation against shifting players
		if (allow_extrapolation)
			*allow_extrapolation = false;

		return original();
	}

	void __fastcall add_view_model_bob(void* ecx, void* edx, c_base_entity* model, vec3_t& pos, vec3_t& angles)
	{
		static auto original = hooker::get_original(&add_view_model_bob);
		original(ecx, edx, model, pos, angles);
	}

	void __fastcall calc_viewmodel_view(void* ecx, void* edx, c_cs_player* owner, vec3_t& eye_pos, vec3_t& eye_angles)
	{
		static auto original = hooker::get_original(&calc_viewmodel_view);

		if (!owner || !owner->is_player() || !owner->is_alive() || !HACKS->local || owner != HACKS->local)
			return original(ecx, edx, owner, eye_pos, eye_angles);

		vec3_t ang = eye_angles;
		vec3_t fwd, rt, up;

		math::angle_vectors(ang, &fwd, &rt, &up);

		if (ANTI_AIM->is_fake_ducking())
			eye_pos = HACKS->local->get_render_origin() + vec3_t{ 0.f, 0.f, HACKS->game_movement->get_player_view_offset(false).z + 0.064f };

		eye_pos += (fwd * g_cfg.misc.viewmodel_pos[1]) + (rt * g_cfg.misc.viewmodel_pos[0]) + (up * g_cfg.misc.viewmodel_pos[2]);

		if (g_cfg.misc.removals & vis_recoil)
			eye_angles -= HACKS->local->aim_punch_angle() * 0.9f + HACKS->local->view_punch_angle();

		original(ecx, edx, owner, eye_pos, eye_angles);
	}

	void* __fastcall model_renderable_animating(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&model_renderable_animating);

#ifdef _DEBUG
		if (HACKS->unload)
			return original(ecx, edx);
#endif

		auto player = (c_cs_player*)((std::uintptr_t)ecx - 4);
		if (!player || player->get_client_class()->class_id != CCSRagdoll)
			return original(ecx, edx);

		// remove ragdolls from fast path list (allows to make chams in DME)
		return nullptr;
	}

	void __fastcall build_transformations(c_cs_player* ecx, void* edx, c_studio_hdr* hdr, int a3, int a4, int a5, int a6, int a7)
	{
		static auto original = hooker::get_original(&build_transformations);

#ifdef _DEBUG
		if (HACKS->unload)
			return original(ecx, edx, hdr, a3, a4, a5, a6, a7);
#endif

		if (!ecx || !ecx->is_player() || !ecx->is_alive())
			return original(ecx, edx, hdr, a3, a4, a5, a6, a7);

		RESTORE(ecx->jiggle_bones_enabled());
		RESTORE(ecx->use_new_animstate());

#ifndef LEGACY
		RESTORE(*(int*)((uintptr_t)ecx + 0x26B0)); // mask ptr

		// don't call clamp bones in bbox when it's not needed
		*(int*)((uintptr_t)ecx + 0x26B0) = 0;
#endif

		// don't allow game to procees attachments shake (jiggle)
		ecx->jiggle_bones_enabled() = false;

		// don't allow game to lerp bones through body snapshots
		ecx->use_new_animstate() = false;

		original(ecx, edx, hdr, a3, a4, a5, a6, a7);
	}

	void __fastcall do_extra_bone_processing(c_cs_player* ecx, void* edx, c_studio_hdr* hdr, vec3_t* pos, vec3_t* q, const matrix3x4_t& mat, std::uint8_t* bone_computed, void* context)
	{
		return;
	}

	bool __fastcall should_skip_anim_frame(c_cs_player* ecx, void* edx)
	{
		return false;
	}

	void __fastcall add_renderable(void* ecx, void* edx, c_renderable* renderable, bool render_with_viewmodels, int type, int model_type, int split_screen_enables)
	{
		static auto original = hooker::get_original(&add_renderable);

		auto renderable_addr = (std::uintptr_t)renderable;
		if (!renderable_addr || renderable_addr == 0x4)
			return original(ecx, edx, renderable, render_with_viewmodels, type, model_type, split_screen_enables);

		auto entity = (c_base_entity*)(renderable_addr - 0x4);
		int index = *(int*)((std::uintptr_t)entity + 0x64);

		if (index < 1 || index > 64)
			return original(ecx, edx, renderable, render_with_viewmodels, type, model_type, split_screen_enables);

		// set advanced transparency type for fixing z order (chams renders behind props)
		if (index == HACKS->engine->get_local_player())
			type = 1;
		else
			type = 2;

		original(ecx, edx, renderable, render_with_viewmodels, type, model_type, split_screen_enables);
	}

	void __fastcall update_client_side_animation(c_cs_player* ecx, void* edx)
	{
		static auto original = hooker::get_original(&update_client_side_animation);

		if (HACKS->client_state->delta_tick != -1 && ecx && ecx->is_alive() && !ecx->is_teammate(false))
		{
			auto anim = ANIMFIX->get_anims(ecx->index());
			if (anim && anim->update_anims)
				return original(ecx, edx);

			ANIMFIX->render_matrices(ecx);
		}
		else
			return original(ecx, edx);
	}

	void __fastcall calc_view(c_cs_player* ecx, void* edx, vec3_t& eye_origin, vec3_t& eye_angles, float& near_, float& far_, float& fov)
	{
		static auto original = hooker::get_original(&calc_view);

		if (!HACKS->local || !HACKS->weapon || !HACKS->weapon_info)
			return original(ecx, edx, eye_origin, eye_angles, near_, far_, fov);

		if (ecx == HACKS->local && ecx->is_alive())
		{
			RESTORE(ecx->default_fov());
			RESTORE(ecx->fov());
			RESTORE(ecx->fov_rate());
			RESTORE(ecx->fov_start());
			RESTORE(ecx->use_new_animstate());

			ecx->use_new_animstate() = false;
			FOV_AND_VIEW->change_fov();

			original(ecx, edx, eye_origin, eye_angles, near_, far_, fov);
			return;
		}

		original(ecx, edx, eye_origin, eye_angles, near_, far_, fov);
	}

	void __fastcall modify_eye_position(c_animation_state* ecx, void* edx, vec3_t& pos)
	{
		static auto original = hooker::get_original(&modify_eye_position);

		auto player = (c_cs_player*)ecx->player;

		// do rebuilt version of this func
		if (player == HACKS->local)
		{
			constexpr auto FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MIN = 4.0f;
			constexpr auto FIRSTPERSON_TO_THIRDPERSON_VERTICAL_TOLERANCE_MAX = 10.0f;

			auto anim = ANIMFIX->get_anims(player->index());
			if (!anim->ptr || anim->ptr != player || !anim->modify_eye_pos)
				return;

			if (!ecx->landing && ecx->anim_duck_amount == 0.f)
				return;

			auto head_bone = player->bone_cache().base()[8].get_origin();
			const auto bone_z = head_bone.z + 1.7f;
			if (pos.z > bone_z)
			{
				const auto view_modifier = std::clamp((fabsf(pos.z - bone_z) - 4.f) * 0.16666667f, 0.f, 1.f);
				const auto view_modifier_sqr = view_modifier * view_modifier;
				pos.z += (bone_z - pos.z) * (3.f * view_modifier_sqr - 2.f * view_modifier_sqr * view_modifier);
			}
		}
		else
			original(ecx, edx, pos);
	}

	bool __fastcall setup_bones(void* ecx, void* edx, matrix3x4_t* bone_to_world, int max_bones, int mask, float time)
	{
		static auto original = hooker::get_original(&setup_bones);

		auto player = (c_cs_player*)((std::uintptr_t)ecx - 4);

		if (HACKS->client_state->delta_tick == -1 || !player || !player->is_player() || !player->is_alive() || player->is_teammate(false))
			return original(ecx, edx, bone_to_world, max_bones, mask, time);

		auto anim = ANIMFIX->get_anims(player->index());
		if (anim && anim->setup_bones)
			return original(ecx, edx, bone_to_world, max_bones, mask, time);
		else
		{
			if (bone_to_world)
				math::memcpy_sse(bone_to_world, player->bone_cache().base(), sizeof(matrix3x4_t) * player->bone_cache().count());

			return true;
		}
	}

	bool __fastcall want_reticle_shown(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&want_reticle_shown);

		if (!HACKS->weapon)
			return original(ecx, edx);

		if (!g_cfg.misc.snip_crosshair || HACKS->local->is_scoped())
			return original(ecx, edx);

		if (!HACKS->weapon->is_scoping_weapon())
			return original(ecx, edx);

		if ((uintptr_t)_ReturnAddress() != offsets::return_addr_process_input.pointer)
			return original(ecx, edx);

		return true;
	}

	void __fastcall engine_paint(void* ecx, void* edx, const int mode)
	{
		static auto orig = hooker::get_original(&engine_paint);

		orig(ecx, edx, mode);
	}

	void __fastcall perform_screen_overlay(void* _this, void* edx, int x, int y, int w, int h)
	{
		if (g_cfg.misc.remove_ads)
			return;

		static auto original = hooker::get_original(&perform_screen_overlay);
		return original(_this, edx, x, y, w, h);
	}

	INLINE vec3_t get_chase_cam_offset(c_cs_player* player) 
	{
		auto view_vectors = HACKS->game_rules->get_view_vectors();

		if (player && player->is_alive()) 
		{
			if (player->flags().has(FL_DUCKING))
				return view_vectors->duck_view;

			return view_vectors->view;
		}

		return view_vectors->dead_view_height;
	}

	void __fastcall calc_chase_cam_view(c_cs_player* ecx, void* edx, vec3_t& eye_origin, vec3_t& eye_angles, float& fov) 
	{
		static auto original = hooker::get_original(&calc_chase_cam_view);

		if (ecx != HACKS->local || !g_cfg.misc.thirdperson_dead || ecx->is_alive())
			return original(ecx, edx, eye_origin, eye_angles, fov);

		auto target = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(ecx->observer_target()));
		if (!target) 
		{
			eye_origin = ecx->origin() + ecx->view_offset();
			eye_angles = ecx->eye_angles();
			return;
		}

		auto animating = reinterpret_cast<c_base_entity*>(reinterpret_cast<std::uintptr_t>(target) + 4);
		if (!animating || !target->get_model()) 
		{
			offsets::calc_roaming_view.cast<void(__thiscall*)(void*, vec3_t&, vec3_t&, float&)>()(ecx, eye_origin, eye_angles, fov);
			return;
		}

		vec3_t forward{}, view_point{};
		auto origin = target->get_render_origin() + get_chase_cam_offset(target);

		vec3_t view_angles{};
		if (ecx->observer_mode() == OBS_MODE_IN_EYE)
			view_angles = eye_angles;
		else if (ecx == HACKS->local)
			HACKS->engine->get_view_angles(view_angles);
		else
			view_angles = ecx->eye_angles();

		view_angles.z = 0.f;

		// change it later, okay?
		ecx->observer_chase_distance() = g_cfg.misc.thirdperson_dist;

		math::angle_vectors(view_angles, forward);
		forward = forward.normalized();

		view_point = origin + forward * (-ecx->observer_chase_distance());

		c_game_trace trace{};
		c_trace_filter_no_npcs_or_player filter{};
		filter.skip = target;

		HACKS->engine_trace->trace_ray({ origin, view_point, {-6.f, -6.f, -6.f}, {6.f, 6.f, 6.f} }, MASK_SOLID, &filter, &trace);

		if (trace.fraction < 1.f) 
		{
			view_point = trace.end;
			ecx->observer_chase_distance() = (origin - eye_origin).length();
		}

		eye_angles = view_angles;
		eye_origin = view_point;

		//CHECKMEM;
	}

	int __fastcall start_sound_immediate(start_sound_params_t* ecx, void* edx)
	{
		static auto original = hooker::get_original(&start_sound_immediate);

		if (!HACKS->in_game || !HACKS->local || !ecx || !ecx->sfx)
			return original(ecx, edx);

		char sound_name[260]{};
		ecx->sfx->get_sound_name(sound_name, 260);

		static auto semiauto_switch = XOR("auto_semiauto_switch");
		static auto weapons = XOR("weapons");
		static auto player_land = XOR("player\\land");
		static auto player_footsteps = XOR("player\\footsteps");

		const auto is_valid_sound = [sound_name]() -> bool {
			return strstr(sound_name, semiauto_switch.c_str())
				|| strstr(sound_name, weapons.c_str())
				|| strstr(sound_name, player_land.c_str())
				|| strstr(sound_name, player_footsteps.c_str());
		};

		auto entity = (c_base_entity*)HACKS->entity_list->get_client_entity(ecx->sound_source);
		if (!entity || !is_valid_sound())
			return original(ecx, edx);

		auto player = (c_cs_player*)entity;
		if (player && !player->is_player())
		{
			if (entity->is_weapon())
				player = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(entity->owner()));
		}

		auto origin = ecx->origin;
		if (std::strstr(sound_name, weapons.c_str()))
			origin.z -= std::strstr(sound_name, semiauto_switch.c_str()) ? 32.f : 64.f;

		if (player && player->is_player() && !player->is_teammate())
		{
			auto esp = ESP->get_esp_player(player->index());
			if (esp) 
			{
				esp->valid = true;

				if (!esp->health)
					esp->health = 100;

				esp->dormant.update(player);
				esp->dormant.origin = origin;
				esp->dormant.mins = vec3_t(- 16.f, -16.f, 0.f );
				esp->dormant.maxs = vec3_t( 16.f, 16.f, 72.f );
				esp->dormant.duck_amount = 0.f;
			}
		}

		return original(ecx, edx);
	}

	// force game to use a default m_vecVelocity instead of m_vecAbsVelocity
// that also corrected in animatino fix
	void __fastcall estimate_abs_velocity(c_base_entity* ecx, void* edx, vec3_t& vel) {
		static auto original = hooker::get_original(&estimate_abs_velocity);

		if (ecx && ecx->is_player()) {
			auto player = reinterpret_cast<c_cs_player*>(ecx);
			if (player->is_teammate())
				return original(ecx, edx, vel);

			vel = player->velocity();
		}
		else
			return original(ecx, edx, vel);
	}

	// server simulation time recieves wrong and not rounded to 100 ticks (as it should)
	// the best fix will be compare if networked animations were updated, wrong simtime recieved & you try to update at the same server tick
	void __fastcall post_data_update(c_networkable* ecx, void* edx, int update_type) {
		static auto original = hooker::get_original(&post_data_update);

		if (!ecx)
			return original(ecx, edx, update_type);

		auto unknown_entity = ecx->get_n_unknown_entity();
		if (!unknown_entity)
			return original(ecx, edx, update_type);

		auto entity = unknown_entity->get_base_entity();
		if (!entity || !entity->is_player())
			return original(ecx, edx, update_type);

		auto player = reinterpret_cast<c_cs_player*>(entity);
		if (!player || !player->is_alive() || player->dormant() || player->is_teammate())
			return original(ecx, edx, update_type);

		auto index = ecx->index();
		auto animation_info = ANIMFIX->get_anims(index);

		if (animation_info) {
			auto& alive_loop = player->animlayers()[ANIMATION_LAYER_ALIVELOOP];

			if (player->sim_time() == animation_info->old_simulation_time2) {
				if (alive_loop.cycle == animation_info->old_aliveloop_cycle)
					player->sim_time() = player->old_sim_time();
			}
			else {
				auto server_tick = HACKS->client_state->clock_drift_mgr.server_tick;
				auto network_time_base = HACKS->global_vars->get_network_base(server_tick, index);
				auto old_network_time_base = HACKS->global_vars->get_network_base(animation_info->server_tick, index);

				auto save_server_tick = (TIME_TO_TICKS(player->sim_time()) % 100)
					|| alive_loop.cycle != animation_info->old_aliveloop_cycle
					|| network_time_base == old_network_time_base;

				if (save_server_tick)
					animation_info->server_tick = server_tick;
				else {
					animation_info->old_simulation_time2 = player->sim_time();
					player->sim_time() = player->old_sim_time();
				}
			}

			original(ecx, edx, update_type);
		}
		else
			return original(ecx, edx, update_type);
	}

	// stop to update animstate move weight & cycle
	// required for better animation sync with custom fixed velocity values
	void __fastcall notify_on_layer_change_weight(c_cs_player* ecx, void* edx, int layer, int weight) {
		static auto original = hooker::get_original(&notify_on_layer_change_weight);
		if (ecx && ecx->is_teammate())
			return original(ecx, edx, layer, weight);

		return;
	}

	void __fastcall notify_on_layer_change_cycle(c_cs_player* ecx, void* edx, int layer, int cycle) {
		static auto original = hooker::get_original(&notify_on_layer_change_cycle);
		if (ecx && ecx->is_teammate())
			return original(ecx, edx, layer, cycle);

		return;
	}

	// prevent bone snapshots from update when player change weapon
	void __fastcall notify_on_layer_change_sequence(c_cs_player* ecx, void* edx, int layer, int sequence) {
		static auto original = hooker::get_original(&notify_on_layer_change_sequence);
		if (ecx && ecx->is_teammate())
			return original(ecx, edx, layer, sequence);

		return;
	}

	void __fastcall accumulate_layers(c_cs_player* ecx, void* edx, bone_setup_t& bone_setup, vec3_t pos[], quaternion_t q[], float curtime) {
		static auto original = hooker::get_original(&accumulate_layers);

		RESTORE(ecx->lod_flags());

		ecx->lod_flags().remove(0xA);
		original(ecx, edx, bone_setup, pos, q, curtime);
	}

	void __fastcall on_latch_interpolated_variables(c_cs_player* ecx, void* edx, int type) {
		static auto original = hooker::get_original(&on_latch_interpolated_variables);

		original(ecx, edx, type);
	}

	INLINE bool have_recharge_on_command()
	{
		auto& recharge = EXPLOITS->recharge;
		auto& limits = EXPLOITS->limits;
		return recharge.start && !recharge.finish;
	}

	bool __fastcall interpolate(void* ecx, void* edx, float time)
	{
		static auto original = hooker::get_original(&interpolate);

		auto base_entity = (c_base_entity*)ecx;

		auto owner = (c_cs_player*)HACKS->entity_list->get_client_entity_handle(base_entity->viewmodel_owner());
		if (!owner || owner->index() != HACKS->local->index())
			return original(ecx, edx, time);

		if (have_recharge_on_command())
		{
			RESTORE(owner->final_predicted_tick());
			RESTORE(HACKS->global_vars->interpolation_amount);

			owner->final_predicted_tick() = owner->tickbase();
			HACKS->global_vars->interpolation_amount = 0.f;

			return original(ecx, edx, time);
		}

		return original(ecx, edx, time);
	}

	bool __fastcall interpolate_player(void* ecx, void* edx, float time)
	{
		static auto original = hooker::get_original(&interpolate_player);

		auto base_entity = (c_cs_player*)ecx;
		if (!base_entity || !base_entity->is_player() || !HACKS->local || HACKS->local != base_entity)
			return original(ecx, edx, time);

		if (have_recharge_on_command())
		{
			RESTORE(base_entity->final_predicted_tick());
			RESTORE(HACKS->global_vars->interpolation_amount);

			base_entity->final_predicted_tick() = base_entity->tickbase();
			HACKS->global_vars->interpolation_amount = 0.f;

			return original(ecx, edx, time);
		}

		return original(ecx, edx, time);
	}

	void __fastcall reset_latched(c_cs_player* ecx, void* edx)
	{
		static auto original = hooker::get_original(&reset_latched);

		if (!HACKS->local || ecx != HACKS->local)
			return original(ecx, edx);

		if (PREDFIX->pred_error_occured)
			return original(ecx, edx);

		return;
	}

#ifndef LEGACY
	vec3_t* __fastcall eye_angles(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&eye_angles);
		return original(ecx, edx);
	}

	void __fastcall on_bbox_change_callback(c_cs_player* ecx, void* edx, vec3_t* old_mins, vec3_t* new_mins, vec3_t* old_maxs, vec3_t* new_maxs)
	{
		auto fresh_tick = HACKS->local && HACKS->local->is_alive() ? HACKS->predicted_time : HACKS->global_vars->curtime;
		auto& base_origin = HACKS->local && ecx == HACKS->local ? ecx->get_abs_origin() : ecx->origin();

		ecx->collision_change_origin() = base_origin.z + old_maxs->z;
		ecx->collision_change_time() = fresh_tick;
	}

	void __fastcall clamp_bones_in_bbox(c_cs_player* ecx, void* edx, matrix3x4_t* bones, int mask)
	{
		return;
	}

	bool __fastcall trace_filter_to_head_collision(void* ecx, void* edx, int collision_group, int contents_mask)
	{
		static auto original = hooker::get_original(&trace_filter_to_head_collision);
		static auto player_movement = XORN(COLLISION_GROUP_PLAYER_MOVEMENT);

		if (collision_group == player_movement)
		{
			memory::bits_t player_bits = game_movement::physics_solid_mask_for_entity((c_cs_player*)ecx);
			memory::bits_t contents_bits = contents_mask;

			auto team_mask = player_bits.bits & CONTENTS_TEAM1 | CONTENTS_TEAM2;
			auto other_team_mask = contents_bits.bits & CONTENTS_TEAM1 | CONTENTS_TEAM2;

			if (team_mask && team_mask == other_team_mask)
				return false;
		}

		return original(ecx, edx, collision_group, contents_mask);
	}
#endif

	INLINE void init() 
	{
		ADD_DETOUR(get_exposure_range);
		ADD_DETOUR(calc_view_model_bob);
		ADD_DETOUR(physics_simulate);
		ADD_DETOUR(update_postscreen_effects);
		ADD_DETOUR(draw_fog);
		ADD_DETOUR(send_datagram);
		ADD_DETOUR(temp_entities);
		ADD_DETOUR(send_net_msg);
		ADD_DETOUR(using_static_prop_debug);
		ADD_DETOUR(msg_voice_data);
		ADD_DETOUR(read_packets);
		ADD_DETOUR(cl_move);
		ADD_DETOUR(process_packet);
		ADD_DETOUR(get_color_modulation);
		ADD_DETOUR(process_interpolated_list);
		ADD_DETOUR(add_view_model_bob);
		ADD_DETOUR(calc_viewmodel_view);
		ADD_DETOUR(model_renderable_animating);
		ADD_DETOUR(build_transformations);
		ADD_DETOUR(do_extra_bone_processing);
		ADD_DETOUR(should_skip_anim_frame);
		ADD_DETOUR(add_renderable);
		ADD_DETOUR(update_client_side_animation);
		ADD_DETOUR(calc_view);
		ADD_DETOUR(modify_eye_position);
		ADD_DETOUR(setup_bones);
		ADD_DETOUR(want_reticle_shown);
		ADD_DETOUR(engine_paint);
		ADD_DETOUR(perform_screen_overlay);
		ADD_DETOUR(calc_chase_cam_view);
		ADD_DETOUR(start_sound_immediate);
		ADD_DETOUR(estimate_abs_velocity);
		ADD_DETOUR(post_data_update);
		ADD_DETOUR(notify_on_layer_change_weight);
		ADD_DETOUR(notify_on_layer_change_cycle);
		ADD_DETOUR(notify_on_layer_change_sequence);
		ADD_DETOUR(accumulate_layers);
		ADD_DETOUR(on_latch_interpolated_variables);
		ADD_DETOUR(interpolate);
		ADD_DETOUR(interpolate_player);
		//ADD_DETOUR(reset_latched);

#ifndef LEGACY
		//	ADD_DETOUR(trace_filter_to_head_collision);
		ADD_DETOUR(eye_angles);
		ADD_DETOUR(on_bbox_change_callback);
		ADD_DETOUR(clamp_bones_in_bbox);
#endif
	}
}