#include "local_player_visuals.h"

#include "../config_vars.h"
#include "../../base/sdk.h"

#include "../../base/other/game_functions.h"
#include "../../base/sdk/entity.h"

#include "../features.h"

#include <algorithm>

#ifdef _DEBUG
void draw_debug_angle( )
{
  if( !g_ctx.cmd )
    return;

  auto angle_line = [ & ]( float ang, int idx = 0 )
  {
    vector3d src = g_ctx.local->get_render_origin( );
    vector3d forward = { };
    vector2d sc1, sc2;

    math::angle_to_vectors( vector3d( 0, ang, 0 ), forward );
    interfaces::debug_overlay->add_line_overlay( src, src + ( forward * 40.f ), 255, 255 * idx, 0, false, 0.1f );
  };

  angle_line( g_ctx.base_angle.y );
  // DrawAngle(g_ctx.cmd->viewangles.y);
  // DrawAngle(g_local_animation_fix->local_info.m_abs, 1);
}
#endif

void c_local_visuals::thirdperson( )
{
  if( !g_ctx.in_game )
    return;

  g_ctx.alpha_amt = 1.f;
  bool alive = g_ctx.local && g_ctx.local->is_alive( );

  if( g_cfg.binds [ tp_b ].toggled )
  {
    if( alive )
    {
      if( !interfaces::input->camera_in_third_person )
        interfaces::input->camera_in_third_person = true;
    }
    else
    {
      if( interfaces::input->camera_in_third_person )
      {
        interfaces::input->camera_in_third_person = false;
        interfaces::input->camera_offset.z = 0.f;
      }

      if( g_cfg.misc.thirdperson_dead && g_ctx.local->observer_mode( ) == 4 )
        g_ctx.local->observer_mode( ) = 5;
    }

    thirdperson_enabled = true;
  }
  else if( interfaces::input->camera_in_third_person && thirdperson_enabled )
  {
    interfaces::input->camera_in_third_person = false;
    interfaces::input->camera_offset.z = 0.f;
    thirdperson_enabled = false;
  }

  if( interfaces::input->camera_in_third_person )
  {
    vector3d offset{ };
    interfaces::engine->get_view_angles( offset );

    vector3d forward;
    math::angle_to_vectors( offset, forward );

    offset.z = g_cfg.misc.thirdperson_dist;

    bool fd = std::abs( interfaces::global_vars->cur_time - last_duck_time ) <= 0.2f;

    auto origin = fd ? g_ctx.local->get_render_origin( ) + vector3d( 0.0f, 0.0f, interfaces::game_movement->get_player_view_offset( false ).z + 0.064f ) 
      : g_ctx.local->get_render_origin( ) + g_ctx.local->view_offset( );

    c_trace_filter_world_and_props_only filter{ };
    c_game_trace tr{ };

    interfaces::engine_trace->trace_ray( ray_t( origin, origin - ( forward * offset.z ), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f } ), mask_npcworldstatic, ( i_trace_filter* )&filter, &tr );

    if( tr.fraction <= 0.5f )
      g_ctx.alpha_amt *= tr.fraction;

    offset.z *= tr.fraction;

    interfaces::input->camera_offset = { offset.x, offset.y, offset.z };
  }
}

void c_local_visuals::modulate_bloom( )
{
  auto& tonemap_array = g_listener_entity->get_entity( ent_tonemap );
  if( !tonemap_array.empty( ) )
  {
    for( const auto& tonemap : tonemap_array )
    {
      auto entity = tonemap.m_entity;
      if( entity == nullptr )
        continue;

      bool& custom_bloom_scale = *( bool* )( ( uintptr_t )entity + offsets::m_bUseCustomBloomScale );
      bool& custom_expo_min = *( bool* )( ( uintptr_t )entity + offsets::m_bUseCustomAutoExposureMin );
      bool& custom_expo_max = *( bool* )( ( uintptr_t )entity + offsets::m_bUseCustomAutoExposureMax );

      custom_bloom_scale = true;
      custom_expo_min = true;
      custom_expo_max = true;

      if( g_cfg.misc.custom_bloom )
      {
        // allow user to set up brightness
        *( float* )( ( uintptr_t )entity + offsets::m_flCustomBloomScale ) = g_cfg.misc.bloom_scale;
        *( float* )( ( uintptr_t )entity + offsets::m_flCustomAutoExposureMin ) = g_cfg.misc.exposure_min;
        *( float* )( ( uintptr_t )entity + offsets::m_flCustomAutoExposureMax ) = g_cfg.misc.exposure_max;
      }
      else
      {
        // make game bright and sweet :)
        *( float* )( ( uintptr_t )entity + offsets::m_flCustomAutoExposureMin ) = 2.f + 1.f * ( int )( g_cfg.misc.world_modulation & 1 );
        *( float* )( ( uintptr_t )entity + offsets::m_flCustomAutoExposureMax ) = 2.f + 1.f * ( int )( g_cfg.misc.world_modulation & 1 );
        *( float* )( ( uintptr_t )entity + offsets::m_flCustomBloomScale ) = 0.f;
      }
    }
  }
}

void c_local_visuals::remove_post_processing( )
{
  static bool should_override = false;

  auto get_postprocess = [ & ]( )
  {
    if( g_cfg.misc.custom_bloom )
      return false;

    if( g_cfg.misc.removals & post_process )
      return true;

    return false;
  };

  bool enabled_postprocessing = get_postprocess( );

  if( !g_ctx.in_game )
  {
    should_override = !enabled_postprocessing;
    return;
  }

  if( should_override != enabled_postprocessing )
  {
    cvars::mat_postprocess_enable->set_value( ( int )( !enabled_postprocessing ) );
    should_override = enabled_postprocessing;
  }
}

void c_local_visuals::remove_smoke( )
{
  // hard code but works very good (was made by myself in weave)
  static auto mat1 = interfaces::material_system->find_material( xor_c( "particle/vistasmokev1/vistasmokev1_smokegrenade" ), xor_c( "Other textures" ) );
  static auto mat2 = interfaces::material_system->find_material( xor_c( "particle/vistasmokev1/vistasmokev1_emods" ), xor_c( "Other textures" ) );
  static auto mat3 = interfaces::material_system->find_material( xor_c( "particle/vistasmokev1/vistasmokev1_emods_impactdust" ), xor_c( "Other textures" ) );
  static auto mat4 = interfaces::material_system->find_material( xor_c( "particle/vistasmokev1/vistasmokev1_fire" ), xor_c( "Other textures" ) );

  bool state = ( g_cfg.misc.removals & smoke );
  mat1->set_material_var_flag( material_var_no_draw, state );
  mat2->set_material_var_flag( material_var_no_draw, state );
  mat3->set_material_var_flag( material_var_no_draw, state );
  mat4->set_material_var_flag( material_var_no_draw, state );

  if( g_cfg.misc.removals & smoke )
    *( int* )func_ptrs::smoke_count = 0;
}

void c_local_visuals::remove_viewmodel_sway( )
{
  static bool state = false;
  bool active = ( g_cfg.misc.removals & viewmodel_move );

  if( !g_ctx.in_game )
  {
    state = !active;
    return;
  }

  if( state != active )
  {
    cvars::cl_wpn_sway_interp->set_value( active ? 0.f : 0.1f );
    state = active;
  }
}

void c_local_visuals::filter_console( )
{
  static auto set_console = true;

  if( set_console )
  {
    set_console = false;

    interfaces::convar->find_convar( xor_c( "developer" ) )->set_value( 0 );
    interfaces::convar->find_convar( xor_c( "con_filter_enable" ) )->set_value( 1 );
    interfaces::convar->find_convar( xor_c( "con_filter_text" ) )->set_value( xor_c( "" ) );
  }

  static auto log_value = true;

  if( log_value != g_cfg.visuals.eventlog.filter_console )
  {
    log_value = g_cfg.visuals.eventlog.filter_console;

    if( !log_value )
      interfaces::convar->find_convar( xor_c( "con_filter_text" ) )->set_value( xor_c( "" ) );
    else
      interfaces::convar->find_convar( xor_c( "con_filter_text" ) )->set_value( xor_c( "IrWL5106TZZKNFPz4P4Gl3pSN?J370f5hi373ZjPg%VOVh6lN" ) );
  }
}

void c_local_visuals::fullbright( )
{
  static bool old_full_bright = false;

  bool fullbright = g_cfg.misc.world_modulation & 4;

  if( !g_ctx.in_game )
  {
    // force update fullbright on map change
    old_full_bright = !fullbright;
    return;
  }

  if( old_full_bright != fullbright )
  {
    cvars::mat_fullbright->callbacks.m_size = 0;
    cvars::mat_fullbright->set_value( ( int )fullbright );

    old_full_bright = fullbright;
  }
}

void c_local_visuals::spoof_cvars( )
{
  static bool old_unlock = false;
  static bool old_cvars = false;

  if( g_cfg.misc.unlock_hidden_cvars )
  {
    if( !old_unlock )
    {
      if( interfaces::convar )
      {
        auto it = **( c_con_cmd_base*** )( interfaces::convar + 0x34 );
        for( auto cv = it->next; cv != nullptr; cv = cv->next )
        {
          c_con_cmd_base* cmd = cv;

          cmd->flags &= ~( 1 << 1 );
          cmd->flags &= ~( 1 << 4 );
        }
      }
      old_unlock = true;
    }
  }

  if( !g_ctx.in_game )
  {
    // force spoof cvar on map change
    old_cvars = !g_cfg.misc.spoof_sv_cheats;
    return;
  }

  if( interfaces::convar )
  {
    if( g_cfg.misc.spoof_sv_cheats )
    {
      if( !old_cvars )
      {
        cvars::sv_cheats->callbacks.m_size = 0;
        cvars::sv_cheats->set_value( 1 );
        old_cvars = true;
      }
    }
    else
    {
      if( old_cvars )
      {
        cvars::sv_cheats->callbacks.m_size = 0;
        cvars::sv_cheats->set_value( 0 );
        old_cvars = false;
      }
    }
  }
}

void c_local_visuals::preverse_killfeed( )
{
  auto death_notice = ( kill_feed_t* )func_ptrs::find_hud_element( *patterns::get_hud_ptr.as< uintptr_t** >( ), xor_str( "SFHudDeathNoticeAndBotStatus" ).c_str( ) );
  if( !death_notice )
    return;

  static auto clear_notices = patterns::clear_killfeed.as< void( __thiscall* )( kill_feed_t* ) >( );
  static bool reset_killfeed = false;

  if( g_ctx.round_start || !g_cfg.misc.preverse_killfeed )
  {
    if( reset_killfeed )
    {
      clear_notices( death_notice );
      reset_killfeed = false;
    }

    g_ctx.round_start = false;
    return;
  }

  reset_killfeed = true;

  int size = death_notice->notices.count( );
  if( !size )
    return;

  for( int i = 0; i < size; ++i )
  {
    auto notice = &death_notice->notices [ i ];

    if( notice->fade == 1.5f )
      notice->fade = FLT_MAX;
  }
}

void c_local_visuals::force_ragdoll_gravity( )
{
  constexpr float ragdoll_force_amt = 800000.f;

  auto& ragdolls = g_listener_entity->get_entity( ent_ragdoll );
  if( ragdolls.empty( ) )
    return;

  for( auto& ragdoll : ragdolls )
  {
    auto entity = ragdoll.m_entity;
    if( !entity || entity->dormant( ) )
      continue;

    auto player = entity->get_ragdoll_player( );
    if( !player || player == g_ctx.local )
      continue;

    if( g_cfg.misc.ragdoll_gravity > 0 )
    {
      if( g_cfg.misc.ragdoll_gravity == 1 )
      {
        entity->vec_force( ).x = ragdoll_force_amt;
        entity->vec_force( ).y = ragdoll_force_amt;
      }
      entity->vec_force( ).z = ragdoll_force_amt;
      entity->ragdoll_velocity( ) *= 10000000.f;
    }
  }
}

// do smooth fov changer
void c_local_visuals::on_calc_view( )
{
  if( !g_ctx.local || !g_ctx.weapon || !g_ctx.weapon_info )
    return;

  float newfov = 90.f + g_cfg.misc.fovs [ world ];

  if( !g_ctx.local->is_alive( ) )
  {
    g_ctx.local->default_fov( ) = g_ctx.local->fov_start( ) = g_ctx.local->fov( ) = newfov;
    return;
  }

  bool invalid_wpn = !g_ctx.weapon->is_sniper( ) || g_ctx.weapon->item_definition_index( ) == weapon_sg556 || g_ctx.weapon->item_definition_index( ) == weapon_aug;

  int m_zoomLevel = g_ctx.weapon->zoom_level( );
  if( m_zoomLevel > 1 && g_cfg.misc.skip_second_zoom )
    m_zoomLevel = 1;

  // get fov that should be added to real
  float zoom_fov = g_ctx.weapon->get_zoom_fov( m_zoomLevel ) + g_cfg.misc.fovs [ world ];
  float fov_delta = newfov - zoom_fov;
  float total_fov = fov_delta * ( 1.f - g_cfg.misc.fovs [ zoom ] * 0.01f );

  g_ctx.local->default_fov( ) = g_ctx.local->fov_start( ) = g_ctx.local->fov( ) = newfov;

  static int last_fov = 0;

  // do anim for snipers zoom
  if( !invalid_wpn )
  {
    float out = zoom_fov + total_fov;

    // smooth zoom in
    if( g_ctx.local->is_scoped( ) )
    {
      if( m_zoomLevel > 1 )
        g_ctx.local->fov_start( ) = out / ( m_zoomLevel - 1 );

      g_ctx.local->fov( ) = out / m_zoomLevel;
      last_fov = g_ctx.local->fov( );

      g_ctx.local->fov_rate( ) = g_ctx.weapon->get_zoom_time( m_zoomLevel );
    }
    else
    {
      // smooth zoom out
      if( last_fov && g_ctx.local->fov( ) == g_ctx.local->fov_start( ) )
      {
        g_ctx.local->fov_start( ) = last_fov;
        g_ctx.local->fov( ) = newfov;

        g_ctx.local->fov_rate( ) = 0.05f;
      }
      else
      {
        g_ctx.local->fov_start( ) = newfov;
        last_fov = 0;
      }
    }
  }
  else
    last_fov = 0;
}

void c_local_visuals::on_paint_traverse( )
{
  const std::unique_lock< std::mutex > lock( mutexes::local );

  if( !g_ctx.in_game || !g_ctx.is_alive )
  {
    g_ctx.scoped = false;
    g_ctx.is_sniper = false;
    g_ctx.is_nade = false;
    g_ctx.is_misc = false;
    g_ctx.can_penetrate = false;
    return;
  }

  if( g_ctx.in_game )
  {
    g_ctx.valve_ds = interfaces::game_rules->is_valve_ds( );
    g_ctx.tick_rate = 1.f / interfaces::global_vars->interval_per_tick;

    static auto sv_maxusrcmdprocessticks = interfaces::convar->find_convar( xor_c( "sv_maxusrcmdprocessticks" ) );

    if( sv_maxusrcmdprocessticks )
      g_ctx.max_choke = std::max( 0, sv_maxusrcmdprocessticks->get_int( ) - 2 );
    else
      g_ctx.max_choke = 14;

    if( g_ctx.valve_ds )
      g_ctx.max_choke = std::clamp( g_ctx.max_choke, 0, 6 );

    if( g_ctx.weapon )
    {
      g_ctx.scoped = g_ctx.local->is_scoped( );
      g_ctx.is_sniper = g_ctx.weapon->is_sniper( );
      g_ctx.is_nade = g_ctx.weapon->is_grenade( );
      g_ctx.is_misc = g_ctx.weapon->is_misc_weapon( );

      if( !g_cfg.misc.menu )
        g_cfg.skins.group_type = cheat_tools::get_legit_tab( );

      if( !g_ctx.is_misc && g_cfg.misc.pen_xhair )
      {
        vector3d dir{ };
        math::angle_to_vectors( g_ctx.orig_angle, dir );

        auto start = g_ctx.local->get_eye_position( );
        auto awall = g_auto_wall->fire_bullet( g_ctx.local, nullptr, g_ctx.weapon_info, g_ctx.weapon->is_taser( ), start, start + ( dir * g_ctx.weapon_info->range ), true );

        g_ctx.can_penetrate = awall.dmg > 0;
      }
      else
        g_ctx.can_penetrate = false;
    }
  }
  else
    g_menu->bomb.reset( );

  if( g_ctx.weapon )
  {
    if( ( g_cfg.misc.removals & scope ) && g_ctx.scoped && g_ctx.is_sniper )
    {
      interfaces::surface->draw_set_color( color( 0, 0, 0, 255 ) );
      interfaces::surface->draw_line( 0, g_render->screen_size.h / 2, g_render->screen_size.w, g_render->screen_size.h / 2 );

      interfaces::surface->draw_set_color( color( 0, 0, 0, 255 ) );
      interfaces::surface->draw_line( g_render->screen_size.w / 2, 0, g_render->screen_size.w / 2, g_render->screen_size.h );
    }
  }

  peek_positions.clear( );

  if( g_movement->peek_start )
  {
    static constexpr float Step = M_PI * 2.f / 60.f;
    if( g_render->world_to_screen( g_movement->peek_pos, peek_w2s, true ) )
    {
      for( float lat = 0.f; lat <= M_PI * 2.f; lat += Step )
      {
        const auto& point3d = vector3d( std::sin( lat ), std::cos( lat ), 0.f ) * 15.f;

        vector2d point2d{ };
        if( g_render->world_to_screen( g_movement->peek_pos + point3d, point2d, true ) )
          peek_positions.push_back( ImVec2( point2d.x, point2d.y ) );
      }
    }
  }
}

void c_local_visuals::on_directx( )
{
  const std::unique_lock< std::mutex > lock( mutexes::local );

  if( !g_ctx.in_game || !g_ctx.is_alive )
    return;

#ifdef _DEBUG
    // if (g_ctx.ideal_spread > 0.f)
    // g_render->circle(g_render->screen_size.w / 2.f, g_render->screen_size.h / 2.f, g_ctx.ideal_spread, color(255, 0, 0), 100);

    // if (g_ctx.spread > 0.f)
    // g_render->circle(g_render->screen_size.w / 2.f, g_render->screen_size.h / 2.f, g_ctx.spread, color(0, 255, 0), 100);
#endif

  if( !peek_positions.empty( ) )
  {
    auto flags_backup = g_render->draw_list->Flags;
    g_render->draw_list->Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;

    static float animation = 1.f;
    g_menu->create_animation( animation, !g_movement->peek_move, 0.5f, lerp_animation );

    auto first_clr = g_cfg.misc.autopeek_clr.base( );
    auto second_clr = g_cfg.misc.autopeek_clr_back.base( );

    color clr = first_clr.multiply( second_clr, 1.f - animation );

    float new_alpha = animation * 150.f;
    for( int i = 0; i < peek_positions.size( ) - 1; ++i )
    {
      g_render->filled_triangle_gradient(
        peek_positions [ i ].x, peek_positions [ i ].y, 
        peek_positions [ i + 1 ].x, peek_positions [ i + 1 ].y, 
        peek_w2s.x, peek_w2s.y, clr.new_alpha( new_alpha ), 
        clr.new_alpha( new_alpha ), clr.new_alpha( 150.f - new_alpha ) );
    }

    g_render->draw_list->Flags = flags_backup;
  }

  if( !g_ctx.is_misc && g_cfg.misc.pen_xhair )
  {
    auto center = vector2d( g_render->screen_size.w / 2, g_render->screen_size.h / 2 );

    auto clr = color( 255, 0, 0 ).multiply( color( 0, 255, 0 ), 1.f * g_ctx.can_penetrate );

    g_render->filled_rect( center.x - 1.f, center.y - 1.f, 3.f, 3.f, color( 20, 20, 20, 100 ) );

    g_render->line( center.x - 1.f, center.y, center.x + 2.f, center.y, clr );
    g_render->line( center.x, center.y - 1.f, center.x, center.y + 2.f, clr );
  }
}

void c_local_visuals::on_render_start( int stage )
{
  if( stage != frame_render_start )
    return;

  this->spoof_cvars( );
  this->filter_console( );
  this->modulate_bloom( );
  this->remove_post_processing( );
  this->remove_smoke( );
  this->force_ragdoll_gravity( );
  this->fullbright( );
  this->remove_viewmodel_sway( );

  if( !g_ctx.in_game )
    return;

  if( !g_ctx.local )
    return;

#ifdef _DEBUG
    // g_ctx.local->draw_server_hitbox();
    // draw_debug_angle();
#endif

  this->preverse_killfeed( );

  old_zoom_sensitivity = cvars::zoom_sensitivity_ratio_mouse->get_float( );

  if( g_cfg.misc.fix_sensitivity && old_zoom_sensitivity > 0.f )
    cvars::zoom_sensitivity_ratio_mouse->set_value( 0.f );
}

void c_local_visuals::on_render_start_after( int stage )
{
  if( !g_ctx.in_game )
    return;

  if( !g_ctx.local )
    return;

  if( stage != frame_render_start )
    return;

  cvars::zoom_sensitivity_ratio_mouse->set_value( old_zoom_sensitivity );
}

void c_local_visuals::on_render_view( c_view_setup* setup )
{
  if( !g_ctx.in_game )
    return;

  if( !g_ctx.local )
    return;

  this->thirdperson( );

  if( g_ctx.is_alive && ( g_cfg.misc.removals & vis_recoil ) )
    setup->angles -= g_ctx.local->aim_punch_angle( ) * 0.9f + g_ctx.local->view_punch_angle( );
}

void c_local_visuals::on_render_view_after( c_view_setup* setup )
{
  if( !g_ctx.in_game )
    return;

  if( !g_ctx.local )
    return;

  if( !g_ctx.local->is_alive( ) )
    return;

  if( g_anti_aim->is_fake_ducking( ) )
    last_duck_time = interfaces::global_vars->cur_time;

  if( std::abs( interfaces::global_vars->cur_time - last_duck_time ) <= 0.2f )
  {
    setup->origin = g_ctx.local->get_render_origin( ) + vector3d( 0.f, 0.f, interfaces::game_movement->get_player_view_offset( false ).z + 0.064f );

    if( interfaces::input->camera_in_third_person )
    {
      vector3d angles = vector3d( interfaces::input->camera_offset.x, interfaces::input->camera_offset.y, 0.f );

      vector3d forward = { };
      math::angle_to_vectors( angles, forward );

      math::vector_multiply( setup->origin, -interfaces::input->camera_offset.z, forward, setup->origin );
    }
  }
}
