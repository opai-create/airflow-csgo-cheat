#include "globals.hpp"
#include "features.hpp"

namespace hooks::vmt
{
	enum indices_t : int
	{
		// client hooks
		FSN_IDX = LEGACY_DESYNC_VFUNC(36, 37),
		CL_CREATE_MOVE_IDX = LEGACY_DESYNC_VFUNC(21, 22),
		LEVEL_INIT_PRE_ENTITY_IDX = 5,
		LEVEL_INIT_POST_ENTITY_IDX = 6,
		LEVEL_INIT_SHUTDOWN_IDX = 7,
		WRITE_USER_CMD_DELTA_TO_BUFFER_IDX = LEGACY_DESYNC_VFUNC(23, 24),
		DISPATCH_UER_MESSAGE_IDX = 38,

		// d3d hooks
		ENDSCENE_IDX = 42,
		RESET_IDX = 16,

		// vgui panel hooks
		PAINT_TRAVERSE_IDX = 41,

		// client mode hooks
		OVERRIDE_VIEW_IDX = 18,
		CREATE_MOVE_IDX = 24,
		DO_POST_SCREEN_EFFECTS_IDX = 44,
		GET_VIEWMODEL_FOV_IDX = 35,

		// cvar hooks
		GET_INT_IDX = 13,
		GET_FLOAT_IDX = 12,

		// key values system hook
		ALLOC_KEY_VALUES_IDX = 2,

		// engine hooks
		IS_HLTV_IDX = 93,
		IS_CONNECTED_IDX = 27,
		GET_SCREEN_ASPECT_RATIO_IDX = 101,

		// file system hooks
		GET_UNVERIFIED_FILE_HASHES = 101,
		CAN_LOAD_THIRDPARTY_FILES = 128,

		// game movement hooks
		PROCESS_MOVEMENT_IDX = 1,

		// view render hooks
		RENDER_2D_EFFECTS_POST_HUD = 39,
		RENDER_SMOKE_OVERLAY = 40,

		// material system hook
		OVERRIDE_CONFIG = 21,

		// model render hook
		DRAW_MODEL_EXECUTE = 21,

		// bsp query hooks
		LIST_LEAVES_IN_BOX_IDX = 6,

		// client state hooks
		PACKET_START_IDX = 5,
		PACKET_END_IDX = 6,

		// prediction hooks
		RUN_COMMAND_IDX = 19,

		// sound hooks
		EMIT_SOUND_IDX = 5,
	};

	void __stdcall frame_stage_notify(int stage)
	{
		static auto original = hooker::get_original(&frame_stage_notify);

#ifdef _DEBUG
		if (HACKS->unload)
			return original(stage);
#else
		//  CHECKMEM;
#endif

		HACKS->init_local_player();

		if (stage == XORN(FRAME_RENDER_START))
		{
			RENDER->update_animation_speed();

			HACKS->in_game = HACKS->engine->is_connected() && HACKS->engine->is_in_game() && HACKS->client_state->signon_state == SIGNONSTATE_FULL;

			// LEGACY CODE FROM OLD SOURCE
			if (HACKS->in_game)
			{
				HACKS->valve_ds = HACKS->game_rules->is_valve_ds();
				HACKS->tick_rate = static_cast<int>(1.f / HACKS->global_vars->interval_per_tick);

				if (HACKS->convars.sv_maxusrcmdprocessticks)
					HACKS->max_choke = std::max(0, HACKS->convars.sv_maxusrcmdprocessticks->get_int() - 2);
				else
					HACKS->max_choke = 14;

				if (HACKS->valve_ds)
					HACKS->max_choke = std::clamp(HACKS->max_choke, 0, 6);

				HACKS->cl_lagcomp0 = HACKS->convars.cl_lagcompensation->get_int() == 0;
				HACKS->original_frame_time = HACKS->global_vars->frametime;
				HACKS->lerp_time = LAGCOMP->get_lerp_time();
				HACKS->max_unlag = HACKS->convars.sv_maxunlag->get_float();

				if (!g_cfg.misc.menu)
				{
					auto legit_tab = main_utils::get_legit_tab();

					g_cfg.legit.group_type = std::clamp(legit_tab, 0, (int)weapon_cfg_revolver);
					g_cfg.skins.group_type = legit_tab;
				}
			}
			// LEGACY END

			FOV_AND_VIEW->change_zoom_sensitivity();
			BULLET_TRACERS->render_tracers();
			HUD_HACKS->preverse_killfeed();
			SPOOFERS->unlock_hidden_cvars();
			POST_PROCESSING->remove();
			REMOVE_PROJECTILES->remove_smoke();
			WORLD_MODULATION->override_shadows();
			WORLD_MODULATION->override_sky_convar();
			WORLD_MODULATION->override_prop_color();
			GRENADE_PREDICTION->calc_local_nade_path();
			GRENADE_PREDICTION->calc_world_path();
			EVENT_LOGS->filter_console();
		}

		ENGINE_PREDICTION->fix_viewmodel(stage);

		// LEGACY CODE FROM OLD SOURCE
		skin_changer::on_postdataupdate_start(stage);
		skin_changer::on_frame_render_end(stage);
		// LEGACY END

		original(stage);

		if (HACKS->local && HACKS->tick_rate > 0 && HACKS->in_game && HACKS->client_state->delta_tick != -1)
		{
			// do animfix & store lagcomp records
			if (stage == XORN(FRAME_NET_UPDATE_END))
			{
				RAGEBOT->proceed_misses();

				ANIMFIX->update_enemies();
			}
		}

		if (stage == XORN(FRAME_RENDER_END))
		{
			RENDER->update_screen();

			if (RENDER->done)
			{
				// LEGACY CODE FROM OLD SOURCE
				g_menu.store_bomb();
				g_menu.store_spectators();
				// LEGACY END

				RENDER->list_start();
				{
					if (HACKS->in_game)
					{
						MOVEMENT->render_peek_position();
						ESP->render();
						GRENADE_PREDICTION->draw_local_path();
						GRENADE_PREDICTION->draw_world_path();
						BULLET_TRACERS->render_hitmarkers();
						ESP->render_local();
					}

					EVENT_LOGS->render_logs();
				}
				RENDER->list_end();
			}
		}
	}

	void __stdcall cl_create_move(int sequence_number, float input_sample_frametime, bool active, bool& send_packet)
	{
		static auto original = hooker::get_original(&cl_create_move_proxy);

		//  CHECKMEM;
		HACKS->init_local_player();

		HACKS->send_packet = &send_packet;
		original(HACKS->client, 0, sequence_number, input_sample_frametime, active);

		auto shifting = EXPLOITS->cl_move.trigger && EXPLOITS->cl_move.shifting;
		if (HACKS->client_state && HACKS->cmd && !shifting)
		{
			auto net_channel = HACKS->client_state->net_channel;
			if (net_channel)
			{
				if (!*HACKS->send_packet && net_channel->choked_packets > 0)
				{
					HACKS->fake_datagram = true;

					RESTORE(net_channel->choked_packets);

					net_channel->choked_packets = 0;
					net_channel->send_datagram();
					--net_channel->out_sequence_nr;
				}
				else
					HACKS->last_outgoing_commands.emplace_back(HACKS->cmd->command_number);

				HACKS->fake_datagram = false;
			}
		}
	}

	__declspec(naked) void __fastcall cl_create_move_proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
	{
		__asm
		{
			push ebp
			mov ebp, esp
			push ebx
			push esp
			push dword ptr[active]
			push dword ptr[input_sample_frametime]
			push dword ptr[sequence_number]
			call cl_create_move
			pop ebx
			pop ebp
			retn 0Ch
		}
	}

	void __fastcall level_init_pre_entity(void* ecx, void* edx, const char* map)
	{
		static auto original = hooker::get_original(&level_init_pre_entity);
		original(ecx, edx, map);
	}

	void __fastcall level_init_post_entity(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&level_init_post_entity);

		//  THREAD_POOL->wait_all();
		//  THREAD_POOL->wait_all();

		HACKS->reset_ctx_values();
		g_menu.reset_game_info();
		MOVEMENT->reset();
		BULLET_TRACERS->reset();
		// EVENT_LOGS->reset();
		BUY_BOT->reset();
		HUD_HACKS->reset();
		CLANTAG->reset();
		SPOOFERS->reset();
		THIRDPSERON->reset();
		FOV_AND_VIEW->reset();
		POST_PROCESSING->reset();
		REMOVE_PROJECTILES->reset();
		WORLD_MODULATION->reset(true);
		ESP->reset();
		GRENADE_PREDICTION->reset();
		CHAMS->reset();
		HACKS->update_dynamic_interfaces();
		ENGINE_PREDICTION->reset();
		PREDFIX->reset();
		ANIMFIX->reset();
		RAGEBOT->reset();
		THREADED_STATE->reset();
		PING_SPIKE->reset();
		EXPLOITS->reset();
		TICKBASE->reset();
		resolver::reset();

		original(ecx, edx);
	}

	void __fastcall level_shutdown(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&level_shutdown);

		//  THREAD_POOL->wait_all();

		HACKS->reset_ctx_values();
		g_menu.reset_game_info();
		MOVEMENT->reset();
		BULLET_TRACERS->reset();
		//   EVENT_LOGS->reset();
		BUY_BOT->reset();
		HUD_HACKS->reset();
		CLANTAG->reset();
		SPOOFERS->reset();
		THIRDPSERON->reset();
		FOV_AND_VIEW->reset();
		POST_PROCESSING->reset();
		REMOVE_PROJECTILES->reset();
		CHAMS->reset();
		WORLD_MODULATION->reset(false);
		ESP->reset();
		GRENADE_PREDICTION->reset();
		ENGINE_PREDICTION->reset();
		ANIMFIX->reset();
		RAGEBOT->reset();
		THREADED_STATE->reset();
		ANTI_AIM->reset();
		FAKE_LAG->reset();
		PING_SPIKE->reset();
		EXPLOITS->reset();
		TICKBASE->reset();
		resolver::reset();

		original(ecx, edx);
	}

	bool __fastcall write_usercmd_to_delta_buffer(void* ecx, void* edx, int slot, void* buf, int from, int to, bool isnewcommand)
	{
		static auto original = hooker::get_original(&write_usercmd_to_delta_buffer);

		// CHECKMEM;

		if (!HACKS->local || !HACKS->local->is_alive())
			return original(ecx, edx, slot, buf, from, to, isnewcommand);

		if (!EXPLOITS->enabled() || !EXPLOITS->tick_to_shift)
			return original(ecx, edx, slot, buf, from, to, isnewcommand);

		if (from != -1)
			return true;

		uintptr_t frame_ptr{};
		__asm mov frame_ptr, ebp;

		int* backup_commands = (int*)(frame_ptr + 0xFD8);
		int* new_commands = (int*)(frame_ptr + 0xFDC);

		//printf("breaking lc: %d \n", EXPLOITS->tick_to_shift);

		return EXPLOITS->should_shift_cmd(new_commands, backup_commands, ecx, edx, slot, buf, from, to);
	}

	bool __fastcall dispatch_user_message(void* _this, void* edx, int msg_type, int arg, int arg1, void* data)
	{
		static auto original = hooker::get_original(&dispatch_user_message);

		if (HACKS->game_rules && !HACKS->game_rules->is_valve_ds())
		{
			if (g_cfg.misc.remove_ads && (msg_type == 7 || msg_type == 8 || msg_type == 5))
				return true;
		}

		return original(_this, edx, msg_type, arg, arg1, data);
	}

	HRESULT __stdcall end_scene(c_d3d_device* device)
	{
		static auto original = hooker::get_original(&end_scene);

#ifdef _DEBUG
		if (HACKS->unload)
			return original(device);
#endif

		IDirect3DStateBlock9* d3d9_state_block = nullptr;
		if (device->CreateStateBlock(D3DSBT_PIXELSTATE, &d3d9_state_block) < 0)
			return original(device);

		RENDER->set_device(device);
		if (!RENDER->init())
			return original(device);

		DWORD colorwrite{}, srgbwrite{};
		IDirect3DVertexDeclaration9* vert_dec = nullptr;
		IDirect3DVertexShader9* vert_shader = nullptr;
		DWORD dwOld_D3DRS_COLORWRITEENABLE = NULL;
		device->GetRenderState(D3DRS_COLORWRITEENABLE, &colorwrite);
		device->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgbwrite);

		DWORD multisample{}, antialias{};
		device->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &multisample);
		device->GetRenderState(D3DRS_ANTIALIASEDLINEENABLE, &antialias);

		device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
		device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);

		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

		device->GetRenderState(D3DRS_COLORWRITEENABLE, &dwOld_D3DRS_COLORWRITEENABLE);
		device->GetVertexDeclaration(&vert_dec);
		device->GetVertexShader(&vert_shader);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		d3d9_state_block->Capture();

		RENDER->begin();

		if (HACKS->cheat_init && !g_cfg.misc.menu)
		{
			g_cfg.misc.menu = true;
			HACKS->cheat_init = false;
			HACKS->cheat_init2 = true;
		}

		imgui_blur::set_device(device);
		imgui_blur::new_frame();

		g_menu.draw();

		if (g_cfg.legit.enable)
		{
			if (g_cfg.rage.enable)
				g_cfg.rage.enable = false;
		}

		if (g_cfg.rage.enable)
		{
			if (g_cfg.legit.enable)
				g_cfg.legit.enable = false;
		}

		RENDER->end();

		d3d9_state_block->Apply();
		d3d9_state_block->Release();

		device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, multisample);
		device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, antialias);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, colorwrite);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, srgbwrite);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, dwOld_D3DRS_COLORWRITEENABLE);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, true);
		device->SetVertexDeclaration(vert_dec);
		device->SetVertexShader(vert_shader);

		return original(device);
	}

	HRESULT __stdcall reset(c_d3d_device* device, D3DPRESENT_PARAMETERS* params)
	{
		static auto original = hooker::get_original(&reset);

#ifdef _DEBUG
		if (HACKS->unload)
			return original(device, params);
#endif

		RENDER->set_device(device);
		imgui_blur::on_device_reset();

		ImGui_ImplDX9_InvalidateDeviceObjects();

		auto hr = original(device, params);

		if (SUCCEEDED(hr))
			ImGui_ImplDX9_CreateDeviceObjects();

		return hr;
	}

	void __fastcall paint_traverse(c_panel* ecx, void* edx, unsigned int panel, bool repaint, bool force)
	{
		static auto original = hooker::get_original(&paint_traverse);

		static c_thread_loop loop{__COUNTER__};
		VGUI_PANEL->update_panels(panel);

		// CHECKMEM;

#ifdef _DEBUG
		if (HACKS->unload)
		{
			original(ecx, edx, panel, repaint, force);

			if (VGUI_PANEL->is_valid_panel(FOCUS_OVERLAY_PANEL, panel))
				ecx->set_mouse_input_enabled(panel, false);

			return;
		}
#endif

		if (HACKS->local && HACKS->local->is_alive() && HACKS->weapon && HACKS->weapon->is_sniper()
			&& VGUI_PANEL->is_valid_panel(HUD_ZOOM, panel) && (g_cfg.misc.removals & scope))
			return;

		original(ecx, edx, panel, repaint, force);

		if (VGUI_PANEL->is_valid_panel(FOCUS_OVERLAY_PANEL, panel))
		{
			// fixes some memes with d3dx render
			HACKS->surface->draw_set_color(c_color{ 0, 0, 0, 255 });
			HACKS->surface->draw_filled_rect(0, 0, 1, 1);

			ecx->set_mouse_input_enabled(panel, g_cfg.misc.menu);
		}
	}

	void __fastcall override_view(void* ecx, void* edx, c_view_setup* setup)
	{
		static auto original = hooker::get_original(&override_view);

		THIRDPSERON->run_alive();
		FOV_AND_VIEW->change_fov_dead_and_remove_recoil(setup);
		original(ecx, edx, setup);
		THIRDPSERON->fix_camera_on_fakeduck(setup);
	}

	float __fastcall get_viewmodel_fov(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&get_viewmodel_fov);

		return original(ecx, edx) + g_cfg.misc.fovs[arms];
	}

	bool __fastcall do_post_screen_effects(void* ecx, void* edx, c_view_setup* setup)
	{
		static auto original = hooker::get_original(&do_post_screen_effects);

		GLOW->run();
		CHAMS->draw_shot_records();

		return original(ecx, edx, setup);
	}

	bool __fastcall create_move(void* ecx, void* edx, float input_sample_time, c_user_cmd* cmd)
	{
		static auto original = hooker::get_original(&create_move);

#ifdef _DEBUG
		if (HACKS->unload)
			return original(ecx, edx, input_sample_time, cmd);
#endif

		if (!cmd || !cmd->command_number || !HACKS->local || !HACKS->in_game)
			return original(ecx, edx, input_sample_time, cmd);

		if (original(ecx, edx, input_sample_time, cmd)) {
			HACKS->engine->set_view_angles(cmd->viewangles);
			HACKS->prediction->set_local_view_angles(cmd->viewangles);
		}

		auto netchannel = HACKS->engine->get_net_channel();
		if (netchannel)
		{
			HACKS->outgoing = netchannel->get_latency(FLOW_OUTGOING);

			if (!netchannel->is_loopback())
				HACKS->real_ping = HACKS->outgoing;
			else
				HACKS->real_ping = -1.f;

			HACKS->ping = netchannel->get_latency(FLOW_INCOMING) + HACKS->outgoing;
			HACKS->arrival_tick = TIME_TO_TICKS(HACKS->outgoing) + HACKS->client_state->clock_drift_mgr.server_tick;
		}

		if (cmd)
			std::memcpy(&EXPLOITS->first_cmd, cmd, sizeof(c_user_cmd));

		HACKS->cmd = cmd;
		{
			CLANTAG->run();

			static bool should_reset = false;
			if (HACKS->local->is_alive())
			{
				should_reset = true;

				if (HACKS->client_state->choked_commands >= HACKS->max_choke)
					*HACKS->send_packet = true;

				if (EXPLOITS->cl_move.trigger && EXPLOITS->cl_move.shifting)
				{
					*HACKS->send_packet = HACKS->client_state->choked_commands >= EXPLOITS->limits.double_tap;

				//	HACKS->cmd->buttons.remove(IN_ATTACK | IN_ATTACK2);
				}

				HACKS->tickbase = [&]()
				{
					static int tickbase = 0;

					if (cmd != nullptr)
					{
						static c_user_cmd* last_cmd = nullptr;

						// if command was not predicted - increment tickbase
						if (last_cmd == nullptr || last_cmd->has_been_predicted)
							tickbase = HACKS->local->tickbase();
						else
							tickbase++;

						last_cmd = cmd;
					}

					return tickbase;
				}();

				RAGEBOT->auto_revolver();
				EXPLOITS->update_instance();

				MOVEMENT->run();
				ANTI_AIM->run_movement();

				RAGEBOT->run_stop();
				ENGINE_PREDICTION->start();
				{
					if (HACKS->weapon)
					{
						BUY_BOT->run();

						if (HACKS->weapon->is_grenade())
							HACKS->predicted_time = TICKS_TO_TIME(HACKS->tickbase);
						else
							HACKS->predicted_time = TICKS_TO_TIME(HACKS->tickbase - EXPLOITS->tickbase_offset());
					}

					auto local_anims = ANIMFIX->get_local_anims();
					local_anims->eye_pos = HACKS->local->origin() + HACKS->local->view_offset();

					LAGCOMP->update_tick_validation();

					auto old_abs = HACKS->local->get_abs_origin();
					RAGEBOT->run();
					HACKS->local->set_abs_origin(old_abs);

					MOVEMENT->run_predicted();
					FAKE_LAG->run();

					EXPLOITS->run();

					ANTI_AIM->run();
					FAKE_LAG->update_shot_cmd();
				}
				ENGINE_PREDICTION->end();
				ANTI_AIM->cleanup();
				MOVEMENT->rotate_movement(HACKS->cmd, MOVEMENT->get_base_angle());
				ANIMFIX->update_local();
			}
			else
			{
				LAGCOMP->update_tick_validation();

				if (should_reset)
				{
					ENGINE_PREDICTION->reset();
					MOVEMENT->reset();
					RAGEBOT->reset();
					ANTI_AIM->reset();
					FAKE_LAG->reset();
					EXPLOITS->reset();
					TICKBASE->reset();

					auto anim = ANIMFIX->get_anims(HACKS->local->index());
					if (anim)
					{
						auto local = ANIMFIX->get_local_anims();
						if (local)
							local->reset();

						anim->reset();
					}

					PREDFIX->reset();

					should_reset = false;
				}
			}
		}

		return false;
	}

	bool __fastcall is_hltv(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&is_hltv);

		const auto& return_address = (std::uintptr_t)_ReturnAddress();

		if (return_address == offsets::return_addr_setup_velocity.pointer
			|| return_address == offsets::return_addr_accumulate_layers.pointer
			|| return_address == offsets::return_addr_reevaluate_anim_lod.pointer
			|| g_cfg.misc.force_radar && return_address == offsets::return_addr_show_radar.pointer)
			return true;

		return original(ecx, edx);
	}

	bool __fastcall is_connected(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&is_connected);

		const auto& return_address = (std::uintptr_t)_ReturnAddress();
		if (return_address == offsets::return_addr_loadout_allowed.pointer)
			return false;

		return original(ecx, edx);
	}

	int __fastcall can_load_thirdparty_files(void* ecx, void* edx)
	{
		static auto original = hooker::get_original(&can_load_thirdparty_files);

		if (ecx != HACKS->file_system)
			return original(ecx, edx);

		if (g_cfg.misc.bypass_sv_pure)
			return 1;

		return original(ecx, edx);
	}

	void __fastcall process_movement(void* ecx, void* edx, c_cs_player* player, c_move_data* data)
	{
		static auto original = hooker::get_original(&process_movement);

		// fix prediction error in air (by stop calculating some vars in movement)
		data->game_code_moved_player = false;

		original(ecx, edx, player, data);
	}

	int __fastcall sv_cheats_get_int(c_convar* ecx, void* edx)
	{
		static auto original = hooker::get_original(&sv_cheats_get_int);
		if (ecx != HACKS->convars.sv_cheats)
			return original(ecx, edx);

		const auto return_address = (std::uintptr_t)_ReturnAddress();
		if (return_address == offsets::return_addr_cam_think.pointer)
			return 1;

		return original(ecx, edx);
	}

	int __fastcall weapon_debug_spread_show_get_int(c_convar* ecx, void* edx)
	{
		static auto original = hooker::get_original(&weapon_debug_spread_show_get_int);

#ifdef _DEBUG
		if (HACKS->unload)
			return original(ecx, edx);
#endif

		if (ecx != HACKS->convars.weapon_debug_spread_show)
			return original(ecx, edx);

		if (!HACKS->local || !HACKS->local->is_alive())
			return original(ecx, edx);

		if (!HACKS->weapon)
			return original(ecx, edx);

		if (!g_cfg.misc.snip_crosshair || HACKS->local->is_scoped())
			return original(ecx, edx);

		if (!HACKS->weapon->is_scoping_weapon())
			return original(ecx, edx);

		return 3;
	}

	float __fastcall get_screen_aspect_ratio(void* ecx, void* edx, int width, int height)
	{
		static auto original = hooker::get_original(&get_screen_aspect_ratio);
		return g_cfg.misc.aspect_ratio > 0 ? g_cfg.misc.aspect_ratio / 100.f : original(ecx, edx, width, height);
	}

	void __fastcall render_2d_effects_post_hud(void* ecx, void* edx, const c_view_setup& setup)
	{
		static auto original = hooker::get_original(&render_2d_effects_post_hud);

		if (g_cfg.misc.removals & flash)
			return;

		original(ecx, edx, setup);
	}

	void __fastcall render_smoke_overlay(void* ecx, void* edx, bool unk)
	{
		static auto original = hooker::get_original(&render_smoke_overlay);

		if (g_cfg.misc.removals & flash)
			return;

		original(ecx, edx, unk);
	}

	bool __fastcall override_config(void* ecx, void* edx, material_system_config_t* config, bool update)
	{
		static auto original = hooker::get_original(&override_config);
		if (g_cfg.misc.world_modulation & 4)
			config->m_nFullbright = true;

		return original(ecx, edx, config, update);
	}

	void __fastcall draw_model_execute(void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world)
	{
		static auto original = hooker::get_original(&draw_model_execute);
		if (HACKS->studio_render->is_forced_material_override())
			return original(ecx, edx, ctx, state, info, bone_to_world);

		auto model = info.model;
		if (model)
		{
			if (std::strstr(model->name, CXOR("player/contactshadow")))
				return;
		}

		CHAMS->on_draw_model_execute(original, ecx, edx, ctx, state, info, bone_to_world);

		HACKS->model_render->forced_material_override(nullptr);
		HACKS->render_view->set_blend(1.f);
	}

	constexpr auto MAX_COORD_FLOAT = 16384.f;
	constexpr auto MIN_COORD_FLOAT = -MAX_COORD_FLOAT;

	const vec3_t map_min = vec3_t{ MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT };
	const vec3_t map_max = vec3_t{ MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT };

	int __fastcall list_leaves_in_box(void* ecx, void* edx, const vec3_t& mins, const vec3_t& maxs, unsigned short* list, int list_max)
	{
		static auto original = hooker::get_original(&list_leaves_in_box);

		const auto& return_address = (std::uintptr_t)_ReturnAddress();
		if (return_address != offsets::list_leaves_in_box.pointer)
			return original(ecx, edx, mins, maxs, list, list_max);

		auto info = *(renderable_info_t**)((std::uintptr_t)_AddressOfReturnAddress() + 0x14);
		if (!info || !info->renderable)
			return original(ecx, edx, mins, maxs, list, list_max);

		auto player = (c_cs_player*)info->renderable->get_i_unknown_entity()->get_base_entity();
		if (!player || !HACKS->local || !player->is_player() || !player->is_alive() || player == HACKS->local)
			return original(ecx, edx, mins, maxs, list, list_max);

		// fuck struct
		// all my homies do hardcode
		// tbh it works better than doing changes in struct vars
		// todo: reclass this bullshit
		*(std::uint16_t*)((std::uintptr_t)info + 0x0016) &= ~0x100;
		*(std::uint16_t*)((std::uintptr_t)info + 0x0018) |= 0xC0;

		return original(ecx, edx, map_min, map_max, list, list_max);
	}

	void __fastcall packet_start(c_client_state* ecx, void* edx, int incoming, int outgoing)
	{
		static auto original = hooker::get_original(&packet_start);

		auto shifting = EXPLOITS->cl_move.trigger && EXPLOITS->cl_move.shifting;
		if (!HACKS->local || !HACKS->local->is_alive() || shifting)
			return original(ecx, edx, incoming, outgoing);

		if (HACKS->fake_datagram)
			outgoing = HACKS->client_state->last_command_ack;

		if (std::find(HACKS->last_outgoing_commands.begin(), HACKS->last_outgoing_commands.end(), outgoing) != HACKS->last_outgoing_commands.end())
			original(ecx, edx, incoming, outgoing);

		std::erase_if(HACKS->last_outgoing_commands, [&](int command)
			{
				return std::abs(command - outgoing) >= 150 || command < outgoing;
			});
	}

	void __fastcall packet_end(c_client_state* ecx, void* edx)
	{
		static auto original = hooker::get_original(&packet_end);

		if (HACKS->local && HACKS->local->is_alive())
		{
			if (HACKS->client_state->clock_drift_mgr.server_tick == HACKS->client_state->delta_tick)
				PREDFIX->fix_netvars(ecx->last_command_ack);
		}

		original(ecx, edx);
	}

	void __fastcall run_command(void* ecx, void* edx, c_cs_player* player, c_user_cmd* cmd, c_move_helper* move_helper)
	{
		static auto original = hooker::get_original(&run_command);

		if (ecx != HACKS->prediction || !cmd || !HACKS->local || !player || player != HACKS->local || !player->is_alive())
			return original(ecx, edx, player, cmd, move_helper);

		if (cmd->tickcount == INT_MAX)
		{
			player->tickbase()++;
			cmd->has_been_predicted = true;
			return;
		}

		TICKBASE->fix(cmd->command_number, player->tickbase());

		original(ecx, edx, player, cmd, move_helper);
		ENGINE_PREDICTION->update_viewmodel_info(cmd);
		PREDFIX->store(cmd->command_number);

#ifdef LEGACY
		static auto collision_state = netvars::get_offset(HASH("DT_CSPlayer"), HASH("m_vphysicsCollisionState"));
		*(int*)((std::uintptr_t)player + collision_state) = 0;
#endif
	}

	void __fastcall emit_sound(void* thisptr, void* edx, void* filter, int ent_index, int channel, const char* sound_entry, unsigned int sound_entry_hash,
		const char* sample, float volume, float attenuation, int seed, memory::bits_t flags, int pitch, const vec3_t* origin, const vec3_t* direction,
		void* vec_origins, bool update_positions, float sound_time, int speaker_entity, int test)
	{
		static auto original = hooker::get_original(&emit_sound);

		auto call_original = [&]()
		{
			original(thisptr, edx, filter, ent_index, channel, sound_entry, sound_entry_hash, sample, volume, attenuation, seed, flags,
				pitch, origin, direction, vec_origins, update_positions, sound_time, speaker_entity, test);
		};

		// don't replay same sound for weapon
		// TO-DO: figure out why events still plays even when you fixed data
		if (ent_index == HACKS->engine->get_local_player())
		{
			auto shifting = cmd_shift::shifting || EXPLOITS->cl_move.trigger && EXPLOITS->cl_move.shifting;

			if (std::strstr(sound_entry, CXOR("Draw"))
				|| std::strstr(sound_entry, CXOR("Deploy"))
				|| std::strstr(sound_entry, CXOR("Weapon")))
			{
				auto& last_sound_name = ENGINE_PREDICTION->get_last_sound_name();
				if (!shifting && !last_sound_name.empty() && std::strstr(sound_entry, last_sound_name.c_str()))
				{
					flags.force(1 << 2);
					return;
				}

				call_original();

				last_sound_name = sound_entry;
				return;
			}
		}

		// anti client lagger
		if (std::strstr(sound_entry, CXOR("null")))
			flags.force((1 << 2) | (1 << 5));

		// don't play double jump sounds
		if (ENGINE_PREDICTION->in_prediction)
		{
			flags.force(1 << 2);
			return;
		}

		call_original();
	}


#ifndef LEGACY
	void* __fastcall alloc_key_values_memory(c_key_values_system* ecx, int edx, int size)
	{
		static auto original = hooker::get_original(&alloc_key_values_memory);

		const auto& return_address = (std::uintptr_t)_ReturnAddress();
		if (return_address == offsets::alloc_key_values_engine.pointer || return_address == offsets::alloc_key_values_client.pointer)
			return nullptr;

		return original(ecx, edx, size);
	}
#endif

	INLINE void init()
	{
		hooker::add_detour(memory::get_virtual(HACKS->client, XORN(FSN_IDX)).cast<std::uint64_t>(), frame_stage_notify);
		hooker::add_detour(memory::get_virtual(HACKS->client, XORN(CL_CREATE_MOVE_IDX)).cast<std::uint64_t>(), cl_create_move_proxy);
		hooker::add_detour(memory::get_virtual(HACKS->client, XORN(LEVEL_INIT_PRE_ENTITY_IDX)).cast<std::uint64_t>(), level_init_pre_entity);
		hooker::add_detour(memory::get_virtual(HACKS->client, XORN(LEVEL_INIT_POST_ENTITY_IDX)).cast<std::uint64_t>(), level_init_post_entity);
		hooker::add_detour(memory::get_virtual(HACKS->client, XORN(LEVEL_INIT_SHUTDOWN_IDX)).cast<std::uint64_t>(), level_shutdown);
		hooker::add_detour(memory::get_virtual(HACKS->client, XORN(WRITE_USER_CMD_DELTA_TO_BUFFER_IDX)).cast<std::uint64_t>(), write_usercmd_to_delta_buffer);
		hooker::add_detour(memory::get_virtual(HACKS->client, XORN(DISPATCH_UER_MESSAGE_IDX)).cast<std::uint64_t>(), dispatch_user_message);

		hooker::add_detour(memory::get_virtual(HACKS->d3d_device, XORN(ENDSCENE_IDX)).cast<std::uint64_t>(), end_scene);
		hooker::add_detour(memory::get_virtual(HACKS->d3d_device, XORN(RESET_IDX)).cast<std::uint64_t>(), reset);

		hooker::add_detour(memory::get_virtual(HACKS->panel, XORN(PAINT_TRAVERSE_IDX)).cast<std::uint64_t>(), paint_traverse);

		hooker::add_detour(memory::get_virtual(HACKS->client_mode, XORN(OVERRIDE_VIEW_IDX)).cast<std::uint64_t>(), override_view);
		hooker::add_detour(memory::get_virtual(HACKS->client_mode, XORN(CREATE_MOVE_IDX)).cast<std::uint64_t>(), create_move);
		hooker::add_detour(memory::get_virtual(HACKS->client_mode, XORN(DO_POST_SCREEN_EFFECTS_IDX)).cast<std::uint64_t>(), do_post_screen_effects);
		hooker::add_detour(memory::get_virtual(HACKS->client_mode, XORN(GET_VIEWMODEL_FOV_IDX)).cast<std::uint64_t>(), get_viewmodel_fov);

		hooker::add_detour(memory::get_virtual(HACKS->engine, XORN(IS_HLTV_IDX)).cast<std::uint64_t>(), is_hltv);
		hooker::add_detour(memory::get_virtual(HACKS->engine, XORN(IS_CONNECTED_IDX)).cast<std::uint64_t>(), is_connected);
		hooker::add_detour(memory::get_virtual(HACKS->engine, XORN(GET_SCREEN_ASPECT_RATIO_IDX)).cast<std::uint64_t>(), get_screen_aspect_ratio);

		hooker::add_detour(memory::get_virtual(HACKS->file_system, XORN(CAN_LOAD_THIRDPARTY_FILES)).cast<std::uint64_t>(), can_load_thirdparty_files);

		hooker::add_detour(memory::get_virtual(HACKS->game_movement, XORN(PROCESS_MOVEMENT_IDX)).cast<std::uint64_t>(), process_movement);
		hooker::add_detour(memory::get_virtual(HACKS->convars.sv_cheats, XORN(GET_INT_IDX)).cast<std::uint64_t>(), sv_cheats_get_int);
		hooker::add_detour(memory::get_virtual(HACKS->convars.weapon_debug_spread_show, XORN(GET_INT_IDX)).cast<std::uint64_t>(), weapon_debug_spread_show_get_int);

		hooker::add_detour(memory::get_virtual(HACKS->view_render, XORN(RENDER_2D_EFFECTS_POST_HUD)).cast<std::uint64_t>(), render_2d_effects_post_hud);
		hooker::add_detour(memory::get_virtual(HACKS->view_render, XORN(RENDER_SMOKE_OVERLAY)).cast<std::uint64_t>(), render_smoke_overlay);

		hooker::add_detour(memory::get_virtual(HACKS->material_system, XORN(OVERRIDE_CONFIG)).cast<std::uint64_t>(), override_config);
		hooker::add_detour(memory::get_virtual(HACKS->model_render, XORN(DRAW_MODEL_EXECUTE)).cast<std::uint64_t>(), draw_model_execute);
		//   hooker::add_detour(memory::get_virtual(HACKS->engine_sound, XORN(EMIT_SOUND_IDX)).cast<std::uint64_t>(), emit_sound);

		auto client_state_vmt = (void*)((std::uintptr_t)HACKS->client_state + XORN(0x8));
		hooker::add_detour(memory::get_virtual(client_state_vmt, XORN(PACKET_START_IDX)).cast<std::uint64_t>(), packet_start);
		hooker::add_detour(memory::get_virtual(client_state_vmt, XORN(PACKET_END_IDX)).cast<std::uint64_t>(), packet_end);

		auto bsp_query = HACKS->engine->get_bsp_tree_query();
		hooker::add_detour(memory::get_virtual(bsp_query, XORN(LIST_LEAVES_IN_BOX_IDX)).cast<std::uint64_t>(), list_leaves_in_box);

		hooker::add_detour(memory::get_virtual(HACKS->prediction, XORN(RUN_COMMAND_IDX)).cast<std::uint64_t>(), run_command);

#ifndef LEGACY
		hooker::add_detour(memory::get_virtual(HACKS->key_values_system, XORN(ALLOC_KEY_VALUES_IDX)).cast<std::uint64_t>(), alloc_key_values_memory);
#endif
	}
}