#include "hooks.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"
#include "../../base/tools/memory/displacement.h"

#include "../../functions/listeners/listener_entity.h"
#include "../../functions/config_vars.h"

namespace hooks
{
	void hook_vmt() //note from @cacamelio : ez fix XDDD
	{
		const auto d3d_device = **patterns::direct_device.as<IDirect3DDevice9***>(); //we add this 
		vtables[vmt_client].setup(interfaces::client);
		vtables[vmt_panel].setup(interfaces::panel);
		vtables[vmt_surface].setup(interfaces::surface);
		vtables[vmt_studio_render].setup(interfaces::studio_render);
		vtables[vmt_client_mode].setup(interfaces::client_mode);
		vtables[vmt_prediction].setup(interfaces::prediction);
		vtables[vmt_engine_bsp].setup(interfaces::engine->get_bsp_tree_query());
		vtables[vmt_engine_].setup(interfaces::engine);
		vtables[vmt_debug_show_spread].setup(cvars::weapon_debug_spread_show);
		vtables[vmt_cl_foot_contact_shadows].setup(cvars::cl_foot_contact_shadows);
		vtables[vmt_cl_brushfastpath].setup(cvars::cl_brushfastpath);
		vtables[vmt_cl_csm_shadows].setup(cvars::cl_csm_shadows);
		vtables[vmt_sv_cheats].setup(cvars::sv_cheats);
		vtables[vmt_trace].setup(interfaces::engine_trace);
		vtables[vmt_client_state_].setup((c_clientstate*)((uint32_t)interfaces::client_state + 0x8));
		vtables[vmt_view_render_].setup(interfaces::view_render);
		vtables[vmt_game_movement].setup(interfaces::game_movement);
		vtables[vmt_model_render].setup(interfaces::model_render);
		vtables[vmt_key_values_system].setup(interfaces::key_values_system);
		vtables[vmt_material_system].setup(interfaces::material_system);
		vtables[vmt_file_system].setup(interfaces::file_system);
		vtables[vmt_engine_sound].setup(interfaces::engine_sound);
		vtables[vmt_direct].setup(d3d_device); //we add this 

		vtables[vmt_direct].hook(xor_int(42), tr::direct::end_scene); //we add this
		vtables[vmt_direct].hook(xor_int(16), tr::direct::reset); //and this 

		vtables[vmt_client].hook(xor_int(5), tr::client::level_init_pre_entity);
		vtables[vmt_client].hook(xor_int(6), tr::client::level_init_post_entity);
		vtables[vmt_client].hook(xor_int(7), tr::client::level_shutdown);
		vtables[vmt_client].hook(xor_int(24), tr::client::write_usercmd_to_delta_buffer);
		vtables[vmt_client].hook(xor_int(22), tr::client::create_move_wrapper);
		vtables[vmt_client].hook(xor_int(38), tr::client::dispatch_user_message);

		vtables[vmt_panel].hook(xor_int(41), tr::panel::paint_traverse);
		vtables[vmt_trace].hook(xor_int(4), tr::trace::clip_ray_to_collideable);

		vtables[vmt_surface].hook(xor_int(116), tr::surface::on_screen_size_changed);
		vtables[vmt_studio_render].hook(xor_int(48), tr::studio_render::draw_model_array);

		vtables[vmt_client_mode].hook(xor_int(18), tr::client_mode::override_view);
		vtables[vmt_client_mode].hook(xor_int(24), tr::client_mode::create_move);
		vtables[vmt_client_mode].hook(xor_int(44), tr::client_mode::do_post_screen_effects);

		vtables[vmt_prediction].hook(xor_int(14), tr::prediction::in_prediction);
		vtables[vmt_prediction].hook(xor_int(19), tr::prediction::run_command);

		vtables[vmt_game_movement].hook(xor_int(1), tr::prediction::process_movement);

		vtables[vmt_engine_bsp].hook(xor_int(6), tr::engine::list_leaves_in_box);
		vtables[vmt_engine_sound].hook(xor_int(5), tr::engine::emit_sound);

		vtables[vmt_engine_].hook(xor_int(27), tr::engine::is_connected);
		vtables[vmt_engine_].hook(xor_int(90), tr::engine::is_paused);
		vtables[vmt_engine_].hook(xor_int(93), tr::engine::is_hltv);
		vtables[vmt_engine_].hook(xor_int(101), tr::engine::get_screen_aspect_ratio);

		vtables[vmt_debug_show_spread].hook(xor_int(13), tr::convars::debug_show_spread_get_int);
		vtables[vmt_cl_foot_contact_shadows].hook(xor_int(13), tr::convars::cl_foot_contact_shadows_get_int);
		vtables[vmt_cl_csm_shadows].hook(xor_int(13), tr::convars::cl_csm_shadows_get_int);
		vtables[vmt_cl_brushfastpath].hook(xor_int(13), tr::convars::cl_brushfastpath_get_int);
		vtables[vmt_sv_cheats].hook(xor_int(13), tr::convars::sv_cheats_get_int);

		vtables[vmt_client_state_].hook(xor_int(5), tr::client_state::packet_start);
		vtables[vmt_client_state_].hook(xor_int(6), tr::client_state::packet_end);

		vtables[vmt_view_render_].hook(xor_int(4), tr::view_render::on_render_start);
		vtables[vmt_view_render_].hook(xor_int(39), tr::view_render::render_2d_effects_post_hud);
		vtables[vmt_view_render_].hook(xor_int(40), tr::view_render::render_smoke_overlay);

		vtables[vmt_model_render].hook(xor_int(21), tr::model_render::draw_model_execute);
		vtables[vmt_key_values_system].hook(xor_int(2), tr::key_values::alloc_key_values_memory);

		vtables[vmt_material_system].hook(xor_int(84), tr::material_system::find_material);

		vtables[vmt_file_system].hook(xor_int(128), tr::engine::can_load_third_party_files);
		vtables[vmt_file_system].hook(xor_int(101), tr::engine::get_unverified_file_hashes);

		//original_present = **reinterpret_cast<decltype(&original_present)*>(patterns::direct_present.as< uintptr_t >());  //and we remove these 
		//**reinterpret_cast<void***>(patterns::direct_present.as< uintptr_t >()) = reinterpret_cast<void*>(&tr::direct::present); //and we remove these 

		//original_reset = **reinterpret_cast<decltype(&original_reset)*>(patterns::direct_reset.as< uintptr_t >()); //and we remove these  
		//**reinterpret_cast<void***>(patterns::direct_reset.as< uintptr_t >()) = reinterpret_cast<void*>(&tr::direct::reset); //and we remove these 

		g_netvar_manager->hook_prop(__fnva1("DT_BaseViewModel"), __fnva1("m_nSequence"), (recv_var_proxy_fn)tr::netvar_proxies::viewmodel_sequence, original_sequence, true);
		g_netvar_manager->hook_prop(__fnva1("DT_CSPlayer"), __fnva1("m_flSimulationTime"), (recv_var_proxy_fn)tr::netvar_proxies::simulation_time, original_simulation_time, true);
	}

	void __fastcall reset_latched(c_csplayer* ecx, void* edx)
	{
		static auto original = hooker.original(&reset_latched);

		if (!g_ctx.local || ecx != g_ctx.local)
			return original(ecx, edx);

		if (g_ctx.pred_error_occured)
			return original(ecx, edx);

		return;
	}

	void hook_detour()
	{
		void* fsn_vfunc = g_memory->getvfunc(interfaces::client, xor_int(37));
		hooker.create_hook(reset_latched, patterns::reset_latched);
		hooker.create_hook(tr::client::frame_stage_notify, fsn_vfunc);
		hooker.create_hook(tr::client::physics_simulate, patterns::physics_simulate.as< void* >());
	 //	hooker.create_hook(tr::client::setup_clr_modulation, patterns::setup_clr_modulation.as< void* >());
		hooker.create_hook(tr::client::perform_screen_overlay, patterns::perform_screen_overlay.as< void* >());
		hooker.create_hook(tr::client::update_postscreen_effects, patterns::update_postscreen_effects.as< void* >());
		hooker.create_hook(tr::client::get_exposure_range, patterns::get_exposure_range.as< void* >());
		hooker.create_hook(tr::client::calc_view_model_bob, patterns::calc_view_model_bob.as< void* >());
		hooker.create_hook(tr::client::process_spotted_entity_update, patterns::process_spotted_entity_update.as< void* >());

		hooker.create_hook(tr::client_mode::set_view_model_offsets, patterns::set_view_model_offsets.as< void* >());
		hooker.create_hook(tr::client_mode::draw_fog, patterns::remove_fog.as< void* >());

		hooker.create_hook(tr::engine::send_net_msg, patterns::send_net_msg.as< void* >());
		hooker.create_hook(tr::engine::check_file_crc_with_server, patterns::check_file_crc_with_server.as< void* >());
		hooker.create_hook(tr::engine::cl_move, patterns::cl_move.as< void* >());
		hooker.create_hook(tr::engine::process_packet, patterns::process_packet.as< void* >());
		hooker.create_hook(tr::engine::read_packets, patterns::read_packets.as< void* >());
		hooker.create_hook(tr::engine::temp_entities, patterns::temp_entities.as< void* >());
		hooker.create_hook(tr::engine::send_datagram, patterns::send_datagram.as< void* >());
		hooker.create_hook(tr::engine::host_shutdown, patterns::host_shutdown.as< void* >());
		hooker.create_hook(tr::engine::msg_voice_data, patterns::msg_voice_data.as< void* >());
		hooker.create_hook(tr::engine::start_sound_immediate, patterns::start_sound_immediate.as< void* >());

		hooker.create_hook(tr::material_system::get_color_modulation, patterns::get_color_modulation.as< void* >());

		hooker.create_hook(tr::trace::trace_filter_for_head_collision, patterns::trace_filter_to_head_collision.as< void* >());

		hooker.create_hook(tr::player::should_skip_anim_frame, patterns::should_skip_anim_frame.as< void* >());
		hooker.create_hook(tr::player::modify_eye_position, patterns::modify_eye_position.as< void* >());
		hooker.create_hook(tr::player::setup_bones, patterns::setup_bones.as< void* >());
		hooker.create_hook(tr::player::build_transformations, patterns::build_transformations.as< void* >());
		hooker.create_hook(tr::player::standard_blending_rules, patterns::standard_blending_rules.as< void* >());
		hooker.create_hook(tr::player::do_extra_bone_processing, patterns::do_extra_bone_processing.as< void* >());
		hooker.create_hook(tr::player::update_clientside_animation, patterns::update_clientside_animation.as< void* >());
		hooker.create_hook(tr::player::model_renderable_animating, patterns::model_renderable_animating.as< void* >());
		hooker.create_hook(tr::player::calc_viewmodel_view, patterns::calc_viewmodel_view.as< void* >());
		hooker.create_hook(tr::player::calc_view, patterns::calc_view.as< void* >());
		hooker.create_hook(tr::player::process_interpolated_list, patterns::process_interpolated_list.as< void* >());
		hooker.create_hook(tr::player::add_view_model_bob, patterns::add_view_model_bob.as< void* >());
		hooker.create_hook(tr::player::want_reticle_shown, patterns::want_reticle_shown.as< void* >());
		hooker.create_hook(tr::player::add_renderable, patterns::add_renderable.as< void* >());
		hooker.create_hook(tr::player::interpolate_server_entities, patterns::interpolate_server_entities.as< void* >());
		hooker.create_hook(tr::player::interpolate, patterns::viewmodel_interpolate.as< void* >());
		hooker.create_hook(tr::player::interpolate_player, patterns::interpolate_player.as< void* >());
		hooker.create_hook(tr::player::eye_angles, patterns::eye_angles.as< void* >());
		hooker.create_hook(tr::player::modify_body_yaw, patterns::modify_body_yaw.as< void* >());
		hooker.create_hook(tr::player::on_bbox_change_callback, patterns::on_bbox_change_callback.as< void* >());
	//	hooker.create_hook(tr::player::weapon_shootpos, patterns::weapon_shootpos.as< void* >());

		hooker.enable();
	}

	void hook_winapi()
	{
		if (g_ctx.window)
			g_ctx.backup_window = (WNDPROC)SetWindowLongA(g_ctx.window, GWL_WNDPROC, (LONG_PTR)tr::wnd_proc);
	}

	void init()
	{
		hook_vmt();
		hook_detour();
		hook_winapi();
	}

	void unhook()
	{
#ifdef _DEBUG
		SetWindowLongPtr(g_ctx.window, GWL_WNDPROC, (LONG_PTR)g_ctx.backup_window);

		for (auto& v : vtables)
			v.unhook_all();

		hooker.restore();

		//**reinterpret_cast<void***>(patterns::direct_present.as< uintptr_t >()) = reinterpret_cast<void*>(original_present); //and these
		//**reinterpret_cast<void***>(patterns::direct_reset.as< uintptr_t >()) = reinterpret_cast<void*>(original_reset);  //and these

		g_netvar_manager->hook_prop(__fnva1("DT_CSPlayer"), __fnva1("m_flSimulationTime"), original_simulation_time, original_simulation_time, true);
		g_netvar_manager->hook_prop(__fnva1("DT_BaseViewModel"), __fnva1("m_nSequence"), original_sequence, original_sequence, false);
#endif
	}
}