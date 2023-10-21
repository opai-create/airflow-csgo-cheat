#include "anti_aim.h"
#include "exploits.h"

#include "../config_vars.h"

#include "../features.h"

#include "../../base/tools/render.h"
#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

anti_aim_angles_t* c_anti_aim::get_config( )
{
  bool ground = g_utils->on_ground( );

  bool stand = ground && ( g_cfg.binds [ sw_b ].toggled || g_rage_bot->should_slide || g_ctx.local->velocity( ).length( true ) < 10.f );

  if( stand )
    return &g_cfg.antihit.angles [ 0 ];
  else if( g_ctx.local->velocity( ).length( true ) > 10.f && ground )
    return &g_cfg.antihit.angles [ 1 ];
  else if( !ground )
    return &g_cfg.antihit.angles [ 2 ];
}

c_csplayer* c_anti_aim::get_closest_player( bool skip, bool local_distance )
{
  auto& player_array = g_listener_entity->get_entity( ent_player );
  if( player_array.empty( ) )
    return nullptr;

  c_csplayer* best = nullptr;
  float best_dist = FLT_MAX;

  vector2d center = vector2d( g_render->screen_size.w * 0.5f, g_render->screen_size.h * 0.5f );

  vector3d view_angles{ };
  interfaces::engine->get_view_angles( view_angles );

  for( const auto& player_info : player_array )
  {
    auto player = ( c_csplayer* )player_info.m_entity;
    if( !player )
      continue;

    if( !player->is_alive( ) || player->gun_game_immunity( ) )
      continue;

    if( player == g_ctx.local || player->team( ) == g_ctx.local->team( ) )
      continue;

    auto& esp_info = g_esp_store->playerinfo [ player->index( ) ];
    if( skip )
    {
      if( player->dormant( ) )
        continue;
    }

    auto base_origin = player->get_abs_origin( );
    base_origin += vector3d( 0.f, 0.f, player->view_offset( ).z / 2.f );

    vector2d origin = { };
    g_render->world_to_screen( base_origin, origin );

    auto angle = math::angle_from_vectors( g_ctx.eye_position, base_origin );

    float dist = local_distance ? math::get_fov( view_angles, angle ) : center.dist_to( origin );
    if( dist < best_dist )
    {
      best = player;
      best_dist = dist;
    }
  }

  return best;
}

vector3d get_predicted_pos( )
{
  float speed = std::max< float >( g_engine_prediction->unprediced_velocity.length( true ), 1.f );

  int max_stop_ticks = std::max< int >( ( ( speed / g_movement->get_max_speed( ) ) * 5.f ) - 1, 0 );
  int max_predict_ticks = std::clamp( 17 - max_stop_ticks, 0, 17 );
  if( max_predict_ticks == 0 )
    return { };

  vector3d last_predicted_velocity = g_engine_prediction->unprediced_velocity;
  for( int i = 0; i < max_predict_ticks; ++i )
  {
    auto pred_velocity = g_engine_prediction->unprediced_velocity * math::ticks_to_time( i + 1 );

    vector3d local_origin = g_ctx.eye_position + pred_velocity;
    int flags = g_ctx.local->flags( );

    g_utils->extrapolate( g_ctx.local, local_origin, pred_velocity, flags, flags & fl_onground );

    last_predicted_velocity = pred_velocity;
  }

  auto predicted_eye_pos = g_ctx.eye_position + last_predicted_velocity;
  return predicted_eye_pos;
}

bool c_anti_aim::is_peeking( )
{
  auto player = this->get_closest_player( );
  if( !player )
    return false;

  auto predicted_eye_pos = get_predicted_pos( );

  bool can_peek = false;
  auto& esp_info = g_esp_store->playerinfo [ player->index( ) ];
  if( !esp_info.valid )
    return false;

  auto origin = player->dormant( ) ? esp_info.dormant_origin : player->get_abs_origin( );

  if( player->dormant( ) )
  {
    vector3d poses [ 3 ]{ origin, origin + player->view_offset( ), origin + vector3d( 0.f, 0.f, player->view_offset( ).z / 2.f ) };

    for( int i = 0; i < 3; ++i )
    {
      c_trace_filter filter{ };
      filter.skip = g_ctx.local;

      c_game_trace out{ };
      interfaces::engine_trace->trace_ray( ray_t( predicted_eye_pos, poses [ i ] ), mask_shot_hull | contents_hitbox, &filter, &out );

      if( out.fraction >= 0.97f )
      {
        can_peek = true;
        break;
      }
      else
        continue;
    }

    if( can_peek )
      return true;
  }
  else
  {
    auto valid_dmg_on_record = [ & ]( records_t* record )
    {
      for( auto& hitbox : hitbox_list )
      {
        auto pts = cheat_tools::get_multipoints( player, hitbox, record->sim_orig.bone );

        for( auto& p : pts )
        {
          g_rage_bot->store( player );
          g_rage_bot->set_record( player, record );

          bool ret = g_auto_wall->can_hit_point( player, p.first, predicted_eye_pos, 0 );

          g_rage_bot->restore( player );

          if( ret )
            return true;
          else
            continue;
        }

        return false;
      }

      return false;
    };

    auto old = g_animation_fix->get_oldest_record( player );

    bool can_peek_to_track = false;
    if( old )
      can_peek_to_track = valid_dmg_on_record( old );

    if( can_peek_to_track )
    {
      can_peek = true;
      return true;
    }

    auto last = g_animation_fix->get_latest_record( player );

    if( last )
    {
      if( valid_dmg_on_record( last ) )
      {
        can_peek = true;
        return true;
      }
    }

    for( auto& hitbox : hitbox_list )
    {
      auto pts = cheat_tools::get_multipoints( player, hitbox, player->bone_cache( ).base( ) );

      for( auto& p : pts )
      {
        bool ret = g_auto_wall->can_hit_point( player, p.first, predicted_eye_pos, 0 );

        if( ret )
        {
          can_peek = true;
          break;
        }
        else
          continue;
      }
    }
  }

  return can_peek;
}

bool c_anti_aim::is_fake_ducking( )
{
  return fake_ducking;
}

void c_anti_aim::fake_duck( )
{
  auto state = g_ctx.local->animstate( );
  if( !state )
    return;

  g_ctx.cmd->buttons |= in_bullrush;

  if( !g_cfg.binds [ fd_b ].toggled )
  {
    fake_ducking = false;
    return;
  }

  if( !g_utils->on_ground( ) || g_ctx.cmd->buttons & in_jump )
  {
    fake_ducking = false;
    return;
  }

  fake_ducking = true;

  if( interfaces::client_state->choked_commands <= 6 )
    g_ctx.cmd->buttons &= ~in_duck;
  else
    g_ctx.cmd->buttons |= in_duck;
}

int c_anti_aim::get_ticks_to_stop( )
{
  static auto predict_velocity = []( vector3d* velocity )
  {
    float speed = velocity->length( true );
    if( speed >= 1.f )
    {
      float friction = cvars::sv_friction->get_float( );
      float stop_speed = std::max< float >( speed, cvars::sv_stopspeed->get_float( ) );
      float time = std::max< float >( interfaces::global_vars->interval_per_tick, interfaces::global_vars->frame_time );
      *velocity *= std::max< float >( 0.f, speed - friction * stop_speed * time / speed );
    }
  };
  auto vel = g_ctx.local->velocity( );
  int ticks_to_stop = 0;
  for( ;; )
  {
    if( vel.length( true ) < 1.f )
      break;
    predict_velocity( &vel );
    ticks_to_stop++;
  }
  return ticks_to_stop;
}

void c_anti_aim::slow_walk( )
{
  if( !g_utils->on_ground( ) )
    return;

  if( g_rage_bot->should_slide )
  {
    g_movement->force_speed( g_movement->get_max_speed( ) * 0.33f );
    g_rage_bot->should_slide = false;
    return;
  }

  if( g_cfg.binds [ sw_b ].toggled )
  {
    g_ctx.cmd->buttons &= ~in_speed;

    int lby_ticks = math::time_to_ticks( g_local_animation_fix->local_info.last_lby_time - interfaces::global_vars->cur_time );
    int ticks = this->get_ticks_to_stop( );

    if( ticks > ( ( g_cfg.antihit.fakewalk_speed - 1 ) - interfaces::client_state->choked_commands ) || !interfaces::client_state->choked_commands )
      g_movement->force_stop( );
  }
}

void c_anti_aim::fake( )
{
  c_animstate* state = g_ctx.local->animstate( );
  if( !state )
    return;

  int choke = g_fake_lag->get_choke_amount( );
  if( choke < 8 )
  {
    static bool flip = false;

    float speed = ( 45.f * 3.f ) * 0.01f;
    float angle = std::sinf( ( g_ctx.system_time( ) * speed ) * 3.14f ) * 180.f;

    g_ctx.cmd->viewangles.y += flip ? angle : -angle;

    flip = !flip;

    return;
  }

  g_ctx.cmd->viewangles.y += this->last_real_angle + 180.f - math::random_float( 10.f, 30.f );
}

void c_anti_aim::manual_yaw( )
{
  if( g_cfg.binds [ left_b ].toggled )
    g_ctx.cmd->viewangles.y -= 90.f;

  if( g_cfg.binds [ right_b ].toggled )
    g_ctx.cmd->viewangles.y += 90.f;
}

void c_anti_aim::automatic_edge( )
{
  if( !g_cfg.binds [ edge_b ].toggled || !g_utils->on_ground( ) || g_cfg.binds [ left_b ].toggled || g_cfg.binds [ right_b ].toggled || g_cfg.binds [ back_b ].toggled )
    return;

  auto player = this->get_closest_player( false, true );
  if( !player )
    return;

  auto weapon = player->get_active_weapon( );
  if( !weapon )
    return;

  auto weapon_info = weapon->get_weapon_info( );
  if( !weapon_info )
    return;

  float at_target = math::normalize( math::angle_from_vectors( player->origin( ), g_ctx.local->origin( ) ).y );
  auto shoot_pos = player->get_eye_position( );

  const float height = 64.f;

  vector3d direction_1, direction_2;
  math::angle_to_vectors( vector3d( 0.f, at_target - 90.f, 0.f ), direction_1 );
  math::angle_to_vectors( vector3d( 0.f, at_target + 90.f, 0.f ), direction_2 );

  const auto left_eye_pos = g_ctx.local->origin( ) + vector3d( 0, 0, height ) + ( direction_1 * 16.f );
  const auto right_eye_pos = g_ctx.local->origin( ) + vector3d( 0, 0, height ) + ( direction_2 * 16.f );

  float left_damage = g_auto_wall->fire_bullet( player, g_ctx.local, weapon_info, weapon->is_taser( ), shoot_pos, left_eye_pos ).dmg;

  float right_damage = g_auto_wall->fire_bullet( player, g_ctx.local, weapon_info, weapon->is_taser( ), shoot_pos, right_eye_pos ).dmg;

  c_game_trace trace = { };
  c_trace_filter_world_only filter = { };

  interfaces::engine_trace->trace_ray( ray_t( left_eye_pos, shoot_pos ), mask_all, &filter, &trace );
  float left_fraction = trace.fraction;

  interfaces::engine_trace->trace_ray( ray_t( right_eye_pos, shoot_pos ), mask_all, &filter, &trace );
  float right_fraction = trace.fraction;

  if( left_damage <= 0 && right_damage <= 0 )
  {
    if( right_fraction < left_fraction )
      g_ctx.cmd->viewangles.y += 90.f;
    else if( right_fraction > left_fraction )
      g_ctx.cmd->viewangles.y -= 90.f;
  }
  else
  {
    if( left_damage > right_damage )
      g_ctx.cmd->viewangles.y -= 90.f;
    else if( left_damage < right_damage )
      g_ctx.cmd->viewangles.y += 90.f;
  }
}

void c_anti_aim::at_targets( )
{
  auto cfg = this->get_config( );

  if( !cfg )
    return;

  if( !cfg->at_targets || g_cfg.binds [ edge_b ].toggled || g_cfg.binds [ left_b ].toggled || g_cfg.binds [ right_b ].toggled || g_cfg.binds [ back_b ].toggled )
    return;

  auto player = this->get_closest_player( false, true );
  if( !player )
    return;

  g_ctx.cmd->viewangles.y = math::normalize( math::angle_from_vectors( g_ctx.local->get_abs_origin( ), player->get_abs_origin( ) ).y );
}

void c_anti_aim::on_pre_predict( )
{
  if( g_ctx.local->move_type( ) == movetype_noclip || g_ctx.local->move_type( ) == movetype_ladder )
    return;

  if( interfaces::game_rules->is_freeze_time( ) || g_ctx.local->flags( ) & fl_frozen || g_ctx.local->gun_game_immunity( ) )
    return;

  this->fake_duck( );
  this->slow_walk( );
}

void c_anti_aim::on_predict_start( )
{
  if( !g_cfg.antihit.enable )
  {
    g_exploits->stop_movement = false;
    return;
  }

  if( interfaces::game_rules->is_freeze_time( ) || g_ctx.local->flags( ) & fl_frozen || g_ctx.local->gun_game_immunity( ) )
  {
    g_exploits->stop_movement = false;
    return;
  }

  auto state = g_ctx.local->animstate( );
  if( !state )
  {
    g_exploits->stop_movement = false;
    return;
  }

  g_exploits->stop_movement = false;

  if( g_utils->is_firing( ) )
    aa_shot_cmd = g_ctx.cmd->command_number;

  if( g_ctx.local->move_type( ) == movetype_ladder || g_ctx.local->move_type( ) == movetype_noclip )
    return;

  if( g_ctx.cmd->buttons & in_use )
    return;

  if( aa_shot_cmd == g_ctx.cmd->command_number )
    return;

  bool moving = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_forward || g_ctx.cmd->buttons & in_back;

  auto cfg = this->get_config( );

  if( !cfg )
    return;

  bool stand = g_cfg.binds [ sw_b ].toggled || g_rage_bot->should_slide || g_ctx.local->velocity( ).length( true ) < 10.f;

  switch( cfg->pitch )
  {
  case 1:
    g_ctx.cmd->viewangles.x = state->aim_pitch_max;
    break;
  case 2:
    g_ctx.cmd->viewangles.x = -state->aim_pitch_min;
    break;
  case 3:
  {
    if( stand )
      g_ctx.cmd->viewangles.x = state->aim_pitch_max - math::random_float( 0.f, 10.f );
    else
      g_ctx.cmd->viewangles.x = state->aim_pitch_max;
  }
  break;
  }

  auto real = [ & ]( )
  {
    this->automatic_edge( );
    this->at_targets( );

    switch( cfg->yaw )
    {
    case 1:
      g_ctx.cmd->viewangles.y += 180.f;
      break;
    case 2:
    {
      auto speed_preset = cfg->random_speed ? this->random_dist_speed : cfg->distortion_speed;
      float speed = ( speed_preset * 3.f ) * 0.01f;
      g_ctx.cmd->viewangles.y += 180.f + std::sinf( ( g_ctx.system_time( ) * speed ) * 3.14f ) * cfg->distortion_range;
    }
    break;
    case 3:
    {
      g_ctx.cmd->viewangles.y += g_ctx.local->lby( ) + cfg->crooked_offset;
    }
    break;
    }

    this->manual_yaw( );

    switch( cfg->jitter_mode )
    {
    case 1:
      g_ctx.cmd->viewangles.y += flip_jitter ? cfg->jitter_range : -cfg->jitter_range;
      break;
    case 2:
      if( flip_jitter )
        g_ctx.cmd->viewangles.y += cfg->jitter_range;
      break;
    case 3:
      g_ctx.cmd->viewangles.y += math::random_float( -cfg->jitter_range, cfg->jitter_range );
      break;
    }

    if( !( g_cfg.binds [ left_b ].toggled || g_cfg.binds [ right_b ].toggled || g_cfg.binds [ back_b ].toggled ) )
      g_ctx.cmd->viewangles.y += cfg->yaw_add;
  };

  if( *g_ctx.send_packet )
    flip_jitter = !flip_jitter;

  if( g_cfg.antihit.desync )
  {
    if( !*g_ctx.send_packet )
    {
      real( );

      bool air = !g_utils->on_ground( );
      bool landing = g_ctx.local->fall_velocity( ) > 0.f;

      bool lby_update = stand && !interfaces::client_state->choked_commands && interfaces::global_vars->cur_time >= g_local_animation_fix->local_info.last_lby_time;

      bool update_tick = false;

      static int tick_to_switch = 0;
      if( std::abs( tick_to_switch - interfaces::global_vars->tick_count ) > g_fake_lag->get_choke_amount( ) + 1 )
      {
        update_tick = true;
        tick_to_switch = interfaces::global_vars->tick_count;
      }

      if( landing || lby_update )
      {
        if( this->old_speed != this->random_dist_speed )
        {
          math::random_seed( interfaces::global_vars->tick_count );
          this->random_dist_speed = math::random_float( 1.f, 100.f );
          this->old_speed = this->random_dist_speed;
        }
      }
      else
        this->old_speed = -1.f;

      if( stand && !interfaces::client_state->choked_commands && interfaces::global_vars->cur_time >= g_local_animation_fix->local_info.last_lby_time )
      {
        if( g_cfg.antihit.desync_type == 2 )
          fake_side = flip_side ? 1 : -1;
        else
          fake_side = g_cfg.binds [ inv_b ].toggled ? -1 : 1;

        if( g_cfg.antihit.desync_type )
        {
          *g_ctx.send_packet = true;

          float lby_delta = g_cfg.antihit.desync_range * fake_side;
          g_ctx.cmd->viewangles.y += lby_delta;
        }

        flip_side = !flip_side;
      }
    }
    else
      this->fake( );
  }
  else
    real( );
}

void c_anti_aim::on_predict_end( )
{
  g_ctx.cmd->viewangles = math::normalize( g_ctx.cmd->viewangles, true );

  if( g_ctx.local->move_type( ) == movetype_ladder || g_ctx.local->move_type( ) == movetype_noclip )
    return;

  g_movement->fix_movement( g_ctx.cmd, g_ctx.base_angle );
}