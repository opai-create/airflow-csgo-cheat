#include "hooks.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"
#include "../../base/tools/memory/displacement.h"

#include "../../functions/listeners/listener_entity.h"

std::array< hooks::vmt::c_hooks, vmt_max > vtables = { };
hooks::detour::c_hooks hooker = { };

namespace hooks
{
  __forceinline void hook_vmt( )
  {
    vtables [ vtables_t::client ].setup( interfaces::client );
    vtables [ vtables_t::panel ].setup( interfaces::panel );
    vtables [ vtables_t::surface ].setup( interfaces::surface );
    vtables [ vtables_t::studio_render ].setup( interfaces::studio_render );
    vtables [ vtables_t::client_mode ].setup( interfaces::client_mode );
    vtables [ vtables_t::prediction ].setup( interfaces::prediction );
    vtables [ vtables_t::engine_bsp ].setup( interfaces::engine->get_bsp_tree_query( ) );
    vtables [ vtables_t::engine_ ].setup( interfaces::engine );
    vtables [ vtables_t::debug_show_spread ].setup( cvars::weapon_debug_spread_show );
    vtables [ vtables_t::cl_foot_contact_shadows ].setup( cvars::cl_foot_contact_shadows );
    vtables [ vtables_t::cl_brushfastpath ].setup( cvars::cl_brushfastpath );
    vtables [ vtables_t::cl_csm_shadows ].setup( cvars::cl_csm_shadows );
    vtables [ vtables_t::sv_cheats ].setup( cvars::sv_cheats );
    vtables [ vtables_t::trace ].setup( interfaces::engine_trace );
    vtables [ vtables_t::client_state_ ].setup( ( c_clientstate* )( ( uint32_t )interfaces::client_state + 0x8 ) );
    vtables [ vtables_t::view_render_ ].setup( interfaces::view_render );
    vtables [ vtables_t::game_movement ].setup( interfaces::game_movement );
    vtables [ vtables_t::model_render ].setup( interfaces::model_render );
    vtables [ vtables_t::key_values_system ].setup( interfaces::key_values_system );
    vtables [ vtables_t::cl_clock_correction ].setup( cvars::cl_clock_correction );
    vtables [ vtables_t::net_showfragments ].setup( cvars::net_showfragments );
    vtables [ vtables_t::material_system ].setup( interfaces::material_system );

    vtables [ vtables_t::client ].hook( xor_int( 5 ), tr::client::level_init_pre_entity );
    vtables [ vtables_t::client ].hook( xor_int( 6 ), tr::client::level_init_post_entity );
    vtables [ vtables_t::client ].hook( xor_int( 23 ), tr::client::write_usercmd_to_delta_buffer );
    vtables [ vtables_t::client ].hook( xor_int( 21 ), tr::client::create_move_wrapper );
    vtables [ vtables_t::client ].hook( xor_int( 38 ), tr::client::dispatch_user_message );
    vtables [ vtables_t::client ].hook( xor_int( 36 ), tr::client::frame_stage_notify );

    vtables [ vtables_t::panel ].hook( xor_int( 41 ), tr::panel::paint_traverse );

    vtables [ vtables_t::surface ].hook( xor_int( 116 ), tr::surface::on_screen_size_changed );
    vtables [ vtables_t::studio_render ].hook( xor_int( 29 ), tr::studio_render::draw_model );
    vtables [ vtables_t::studio_render ].hook( xor_int( 48 ), tr::studio_render::draw_model_array );

    vtables [ vtables_t::trace ].hook( xor_int( 4 ), tr::trace::clip_ray_to_collideable );

    vtables [ vtables_t::client_mode ].hook( xor_int( 18 ), tr::client_mode::override_view );
    vtables [ vtables_t::client_mode ].hook( xor_int( 24 ), tr::client_mode::create_move );
    vtables [ vtables_t::client_mode ].hook( xor_int( 44 ), tr::client_mode::do_post_screen_effects );

    vtables [ vtables_t::prediction ].hook( xor_int( 14 ), tr::prediction::in_prediction );
    vtables [ vtables_t::prediction ].hook( xor_int( 19 ), tr::prediction::run_command );

    vtables [ vtables_t::game_movement ].hook( xor_int( 1 ), tr::prediction::process_movement );

    vtables [ vtables_t::engine_bsp ].hook( xor_int( 6 ), tr::engine::list_leaves_in_box );

    vtables [ vtables_t::engine_ ].hook( xor_int( 27 ), tr::engine::is_connected );
    vtables [ vtables_t::engine_ ].hook( xor_int( 90 ), tr::engine::is_paused );
    vtables [ vtables_t::engine_ ].hook( xor_int( 93 ), tr::engine::is_hltv );
    vtables [ vtables_t::engine_ ].hook( xor_int( 101 ), tr::engine::get_screen_aspect_ratio );

    vtables [ vtables_t::debug_show_spread ].hook( xor_int( 13 ), tr::convars::debug_show_spread_get_int );
    vtables [ vtables_t::cl_foot_contact_shadows ].hook( xor_int( 13 ), tr::convars::cl_foot_contact_shadows_get_int );
    vtables [ vtables_t::cl_csm_shadows ].hook( xor_int( 13 ), tr::convars::cl_csm_shadows_get_int );
    vtables [ vtables_t::cl_brushfastpath ].hook( xor_int( 13 ), tr::convars::cl_brushfastpath_get_int );
    vtables [ vtables_t::sv_cheats ].hook( xor_int( 13 ), tr::convars::sv_cheats_get_int );
    vtables [ vtables_t::cl_clock_correction ].hook( xor_int( 13 ), tr::convars::cl_clock_correction_get_int );
    vtables [ vtables_t::net_showfragments ].hook( xor_int( 13 ), tr::convars::net_showfragments_get_bool );

    vtables [ vtables_t::client_state_ ].hook( xor_int( 5 ), tr::client_state::packet_start );
    vtables [ vtables_t::client_state_ ].hook( xor_int( 6 ), tr::client_state::packet_end );

    vtables [ vtables_t::view_render_ ].hook( xor_int( 4 ), tr::view_render::on_render_start );
    vtables [ vtables_t::view_render_ ].hook( xor_int( 39 ), tr::view_render::render_2d_effects_post_hud );
    vtables [ vtables_t::view_render_ ].hook( xor_int( 40 ), tr::view_render::render_smoke_overlay );

    vtables [ vtables_t::model_render ].hook( xor_int( 21 ), tr::model_render::draw_model_execute );

    vtables [ vtables_t::material_system ].hook( xor_int( 84 ), tr::material_system::find_material );

    original_present = **reinterpret_cast< decltype( &original_present )* >( patterns::direct_present.as< uintptr_t >( ) );
    **reinterpret_cast< void*** >( patterns::direct_present.as< uintptr_t >( ) ) = reinterpret_cast< void* >( &tr::direct::present );

    original_reset = **reinterpret_cast< decltype( &original_reset )* >( patterns::direct_reset.as< uintptr_t >( ) );
    **reinterpret_cast< void*** >( patterns::direct_reset.as< uintptr_t >( ) ) = reinterpret_cast< void* >( &tr::direct::reset );

    g_netvar_manager->hook_prop( __fnva1( "DT_BaseViewModel" ), __fnva1( "m_nSequence" ), ( recv_var_proxy_fn )tr::netvar_proxies::viewmodel_sequence, original_sequence, true );
  }

  __forceinline void hook_detour( )
  {
    //void* fsn_vfunc = g_memory->getvfunc( interfaces::client, xor_int( 36 ) );
    //hooker.create_hook( tr::client::frame_stage_notify, fsn_vfunc );
    hooker.create_hook( tr::client::physics_simulate, patterns::physics_simulate.as< void* >( ) );
 //   hooker.create_hook( tr::client::setup_clr_modulation, patterns::setup_clr_modulation.as< void* >( ) );
    hooker.create_hook( tr::client::draw_models, patterns::draw_models.as< void* >( ) );
    hooker.create_hook( tr::client::perform_screen_overlay, patterns::perform_screen_overlay.as< void* >( ) );
    hooker.create_hook( tr::client::render_glow_boxes, patterns::render_glow_boxes.as< void* >( ) );
    hooker.create_hook( tr::client::update_postscreen_effects, patterns::update_postscreen_effects.as< void* >( ) );
    hooker.create_hook( tr::client::get_exposure_range, patterns::get_exposure_range.as< void* >( ) );
    hooker.create_hook( tr::client::calc_view_model_bob, patterns::calc_view_model_bob.as< void* >( ) );

    hooker.create_hook( tr::client_mode::set_view_model_offsets, patterns::set_view_model_offsets.as< void* >( ) );
    hooker.create_hook( tr::client_mode::draw_fog, patterns::remove_fog.as< void* >( ) );

    hooker.create_hook( tr::engine::using_static_props_debug, patterns::using_static_prop_debug.as< void* >( ) );
    hooker.create_hook( tr::engine::send_net_msg, patterns::send_net_msg.as< void* >( ) );
    hooker.create_hook( tr::engine::check_file_crc_with_server, patterns::check_file_crc_with_server.as< void* >( ) );
    hooker.create_hook( tr::engine::cl_move, patterns::cl_move.as< void* >( ) );
    hooker.create_hook( tr::engine::process_packet, patterns::process_packet.as< void* >( ) );
    hooker.create_hook( tr::engine::read_packets, patterns::read_packets.as< void* >( ) );
    hooker.create_hook( tr::engine::temp_entities, patterns::temp_entities.as< void* >( ) );
    hooker.create_hook( tr::engine::send_datagram, patterns::send_datagram.as< void* >( ) );

    hooker.create_hook( tr::material_system::get_color_modulation, patterns::get_color_modulation.as< void* >( ) );

    hooker.create_hook( tr::player::should_skip_anim_frame, patterns::should_skip_anim_frame.as< void* >( ) );
    hooker.create_hook( tr::player::modify_eye_position, patterns::modify_eye_position.as< void* >( ) );
    hooker.create_hook( tr::player::setup_bones, patterns::setup_bones.as< void* >( ) );
    hooker.create_hook( tr::player::build_transformations, patterns::build_transformations.as< void* >( ) );
    hooker.create_hook( tr::player::standard_blending_rules, patterns::standard_blending_rules.as< void* >( ) );
    hooker.create_hook( tr::player::do_extra_bone_processing, patterns::do_extra_bone_processing.as< void* >( ) );
    hooker.create_hook( tr::player::update_clientside_animation, patterns::update_clientside_animation.as< void* >( ) );
    hooker.create_hook( tr::player::model_renderable_animating, patterns::model_renderable_animating.as< void* >( ) );
    hooker.create_hook( tr::player::calc_viewmodel_view, patterns::calc_viewmodel_view.as< void* >( ) );
    hooker.create_hook( tr::player::calc_view, patterns::calc_view.as< void* >( ) );
    hooker.create_hook( tr::player::process_interpolated_list, patterns::process_interpolated_list.as< void* >( ) );
    hooker.create_hook( tr::player::add_view_model_bob, patterns::add_view_model_bob.as< void* >( ) );
    hooker.create_hook( tr::player::want_reticle_shown, patterns::want_reticle_shown.as< void* >( ) );
    // hooker.create_hook(tr::player::interpolate, patterns::viewmodel_interpolate.as<void*>());
    hooker.create_hook( tr::player::add_renderable, patterns::add_renderable.as< void* >( ) );

    hooker.enable( );
  }

  __forceinline void hook_winapi( )
  {
    if( g_ctx.window )
      g_ctx.backup_window = ( WNDPROC )SetWindowLongA( g_ctx.window, GWL_WNDPROC, ( LONG_PTR )tr::wnd_proc );
  }

  __forceinline void init( )
  {
    hook_vmt( );
    hook_detour( );
    hook_winapi( );
  }

  __forceinline void unhook( )
  {
#ifdef _DEBUG
    SetWindowLongPtr( g_ctx.window, GWL_WNDPROC, ( LONG_PTR )g_ctx.backup_window );

    for( auto& v : vtables )
      v.unhook_all( );

    hooker.restore( );

    **reinterpret_cast< void*** >( patterns::direct_present.as< uintptr_t >( ) ) = reinterpret_cast< void* >( original_present );
    **reinterpret_cast< void*** >( patterns::direct_reset.as< uintptr_t >( ) ) = reinterpret_cast< void* >( original_reset );

    g_netvar_manager->hook_prop( __fnva1( "DT_BaseViewModel" ), __fnva1( "m_nSequence" ), original_sequence, original_sequence, false );
#endif
  }
}