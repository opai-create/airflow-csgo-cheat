#include "ragebot.h"
#include "../features.h"

#include "../../base/tools/threads.h"

#ifdef _DEBUG
#define DEBUG_LC 0
#define DEBUG_SP 0
void draw_hitbox( c_csplayer* player, matrix3x4_t* bones, int idx, int idx2, bool dur = false )
{
  studio_hdr_t* studio_model = interfaces::model_info->get_studio_model( player->get_model( ) );
  if( !studio_model )
    return;

  mstudio_hitbox_set_t* hitbox_set = studio_model->get_hitbox_set( 0 );
  if( !hitbox_set )
    return;

  for( int i = 0; i < hitbox_set->hitboxes; i++ )
  {
    mstudio_bbox_t* hitbox = hitbox_set->get_hitbox( i );
    if( !hitbox )
      continue;

    vector3d vMin, vMax;
    math::vector_transform( hitbox->bbmin, bones [ hitbox->bone ], vMin );
    math::vector_transform( hitbox->bbmax, bones [ hitbox->bone ], vMax );

    if( hitbox->radius != -1.f )
      interfaces::debug_overlay->add_capsule_overlay( vMin, vMax, hitbox->radius, 255, 255 * idx, 255 * idx2, 150, dur ? interfaces::global_vars->interval_per_tick * 2 : 5.f, 0, 1 );
  }
}
#endif

void c_rage_bot::store( c_csplayer* player )
{
  auto& backup = this->backup [ player->index( ) ];
  backup.duck = player->duck_amount( );
  backup.lby = player->lby( );
  backup.angles = player->eye_angles( );
  backup.origin = player->origin( );
  backup.absorigin = player->get_abs_origin( );
  backup.bbmin = player->bb_mins( );
  backup.bbmax = player->bb_maxs( );
  backup.velocity = player->velocity( );
  player->store_bone_cache( backup.bonecache );
  player->store_poses( backup.poses );

  backup.filled = true;
}

void c_rage_bot::set_record( c_csplayer* player, records_t* record, matrix3x4_t* matrix )
{
  player->invalidate_bone_cache( );

  player->eye_angles( ) = record->eye_angles;

  auto collideable = player->get_collideable( );
  func_ptrs::set_collision_bounds( collideable, &record->mins, &record->maxs );

  player->velocity( ) = record->anim_velocity;

  player->set_poses( record->poses );

  player->origin( ) = record->origin;
  player->set_abs_origin( record->origin );

  player->set_bone_cache( matrix ? matrix : record->sim_orig.bone );
}

void c_rage_bot::restore( c_csplayer* player )
{
  auto& backup = this->backup [ player->index( ) ];
  if( !backup.filled )
    return;

  player->invalidate_bone_cache( );

  player->eye_angles( ) = backup.angles;

  auto collideable = player->get_collideable( );
  func_ptrs::set_collision_bounds( collideable, &backup.bbmin, &backup.bbmax );

  player->set_poses( backup.poses );

  player->velocity( ) = backup.velocity;

  player->origin( ) = backup.origin;
  player->set_abs_origin( backup.origin );

  player->set_bone_cache( backup.bonecache );
}

std::vector< int > c_rage_bot::get_hitboxes( )
{
  std::vector< int > hitboxes{ };

  if( g_ctx.weapon->is_taser( ) )
  {
    hitboxes.emplace_back( ( int )hitbox_stomach );
    hitboxes.emplace_back( ( int )hitbox_pelvis );
    return hitboxes;
  }

  if( cheat_tools::get_weapon_config( ).hitboxes & head )
    hitboxes.emplace_back( ( int )hitbox_head );

  if( cheat_tools::get_weapon_config( ).hitboxes & chest )
  {
    hitboxes.emplace_back( ( int )hitbox_chest );
    hitboxes.emplace_back( ( int )hitbox_lower_chest );
  }

  if( cheat_tools::get_weapon_config( ).hitboxes & stomach )
    hitboxes.emplace_back( ( int )hitbox_stomach );

  if( cheat_tools::get_weapon_config( ).hitboxes & pelvis )
    hitboxes.emplace_back( ( int )hitbox_pelvis );

  if( cheat_tools::get_weapon_config( ).hitboxes & arms_ )
  {
    hitboxes.emplace_back( ( int )hitbox_left_upper_arm );
    hitboxes.emplace_back( ( int )hitbox_right_upper_arm );
  }

  if( cheat_tools::get_weapon_config( ).hitboxes & legs )
  {
    hitboxes.emplace_back( ( int )hitbox_left_thigh );
    hitboxes.emplace_back( ( int )hitbox_right_thigh );

    hitboxes.emplace_back( ( int )hitbox_left_calf );
    hitboxes.emplace_back( ( int )hitbox_right_calf );

    hitboxes.emplace_back( ( int )hitbox_left_foot );
    hitboxes.emplace_back( ( int )hitbox_right_foot );
  }

  return hitboxes;
}

int c_rage_bot::get_min_damage( c_csplayer* player )
{
  const auto& weapon_cfg = cheat_tools::get_weapon_config( );

  int health = player->health( );

  int menu_damage = g_cfg.binds [ override_dmg_b ].toggled ? weapon_cfg.damage_override : weapon_cfg.mindamage;

  // hp + 1 slider
  if( menu_damage >= 100 )
    return health + ( menu_damage - 100 );

  return menu_damage;
}

bool c_rage_bot::should_stop( bool shoot_check )
{
  if( !( cheat_tools::get_weapon_config( ).quick_stop ) )
    return false;

  if( g_ctx.weapon->is_misc_weapon( ) )
    return false;

  if( !g_utils->on_ground( ) )
    return false;

  if( g_ctx.local->velocity( ).length( true ) < 1.f )
    return false;

  if( g_cfg.binds [ sw_b ].toggled )
    return false;

  bool able_to_shoot = g_utils->is_able_to_shoot( true );

  if( shoot_check )
  {
    bool between_shots_ = cheat_tools::get_weapon_config( ).quick_stop_options & between_shots;
    if( !able_to_shoot )
      return between_shots_;
  }

  return able_to_shoot;
}

void c_rage_bot::start_stop( )
{
  if( !g_ctx.weapon || !stopping )
    return;

  bool weapon_for_tp = g_ctx.weapon->is_pistols( ) && !g_ctx.weapon->is_heavy_pistols( ) && g_ctx.weapon->item_definition_index( ) == weapon_awp || g_ctx.weapon->item_definition_index( ) == weapon_ssg08;

  if( weapon_for_tp && g_exploits->cl_move.shift && !g_utils->is_able_to_shoot( true ) )
    return;

  if( this->should_stop( ) )
    force_accuracy = this->auto_stop( );
}

bool c_rage_bot::auto_stop( )
{
  auto velocity = g_ctx.local->velocity( );
  float raw_speed = velocity.length( true );

  int max_speed = ( int )g_movement->get_max_speed( ) * 0.34f;
  int speed = ( int )raw_speed;

  if( speed <= max_speed )
  {
    g_movement->force_speed( g_movement->get_max_speed( ) * 0.34f );
    return true;
  }

  g_movement->force_stop( );

  return !( cheat_tools::get_weapon_config( ).quick_stop_options & 4 );
}

bool c_rage_bot::knife_is_behind( records_t* record )
{
  vector3d delta{ record->origin - g_ctx.eye_position };
  delta.z = 0.f;
  delta = delta.normalized( );

  vector3d target;
  math::angle_to_vectors( record->abs_angles, target );
  target.z = 0.f;

  return delta.dot( target ) > 0.475f;
}

void c_rage_bot::knife_bot( )
{
  if( !g_cfg.rage.enable )
    return;

  if( !g_ctx.weapon->is_knife( ) )
    return;

  if( g_ctx.predicted_curtime < g_ctx.weapon->next_primary_attack( ) || g_ctx.predicted_curtime < g_ctx.weapon->next_secondary_attack( ) )
    return;

  auto nearest_target = g_anti_aim->get_closest_player( true );
  if( !nearest_target )
    return;

  auto last = g_animation_fix->get_latest_record( nearest_target );
  if( !last )
    return;

  auto position = nearest_target->get_hitbox_position( hitbox_stomach );
  if( g_ctx.eye_position.dist_to( position ) > 65.f )
    return;

  knife_point_t best{ };

  auto get_record_damage = [ & ]( records_t* record )
  {
    this->store( nearest_target );
    this->set_record( nearest_target, record );

    auto awall = g_auto_wall->fire_bullet( g_ctx.local, nearest_target, g_ctx.weapon_info, g_ctx.weapon->is_taser( ), g_ctx.eye_position, nearest_target->get_hitbox_position( hitbox_stomach ) );

    this->restore( nearest_target );

    if( awall.remaining_pen < 4 )
      return 0;

    return awall.dmg;
  };

  auto last_damage = get_record_damage( last );
  best.point = nearest_target->get_hitbox_position( hitbox_stomach );
  best.record = last;
  best.damage = last_damage;

  if( g_ctx.lagcomp )
  {
    auto old = g_animation_fix->get_oldest_record( nearest_target );
    if( old && old != last )
    {
      auto old_damage = get_record_damage( old );
      if( old_damage > last_damage )
      {
        best.point = nearest_target->get_hitbox_position( hitbox_stomach );
        best.record = old;
        best.damage = old_damage;
      }
    }
  }

  if( best.record )
  {
    bool stab = false;
    bool armor = best.record->ptr->armor_value( ) > 0;
    bool first = g_ctx.weapon->next_primary_attack( ) + 0.4f < g_ctx.predicted_curtime;
    bool back = this->knife_is_behind( best.record );

    int stab_dmg = knife_dmg.stab [ armor ][ back ];
    int slash_dmg = knife_dmg.swing [ first ][ armor ][ back ];
    int swing_dmg = knife_dmg.swing [ false ][ armor ][ back ];

    int health = best.record->ptr->health( );
    if( health <= slash_dmg )
      stab = false;
    else if( health <= stab_dmg )
      stab = true;
    else if( health > ( slash_dmg + swing_dmg + stab_dmg ) )
      stab = true;
    else
      stab = false;

    g_ctx.cmd->viewangles = math::normalize( math::angle_from_vectors( g_ctx.eye_position, best.point ), true );

    if( g_ctx.lagcomp )
      g_ctx.cmd->tickcount = math::time_to_ticks( best.record->sim_time + g_ctx.lerp_time );

    g_ctx.cmd->buttons |= stab ? in_attack2 : in_attack;
  }
}

std::vector< int > backtrack_hitboxes{
  hitbox_head,
  hitbox_chest,
  hitbox_pelvis,
  hitbox_stomach,
  hitbox_right_foot,
  hitbox_left_foot,
};

int get_record_damage( c_csplayer* player, records_t* record )
{
  int total_dmg = 0;

  g_rage_bot->store( player );
  g_rage_bot->set_record( player, record );

  for( auto& hitbox : backtrack_hitboxes )
  {
    auto position = player->get_hitbox_position( hitbox, record->sim_orig.bone );

    auto awall = g_auto_wall->fire_bullet( g_ctx.local, player, g_ctx.weapon_info, g_ctx.weapon->is_taser( ), g_ctx.eye_position, position );

    total_dmg += awall.dmg;
  }

  g_rage_bot->restore( player );

  return total_dmg;
}

records_t* get_best_record( c_csplayer* player )
{
  auto last = g_animation_fix->get_latest_record( player );
  auto old = g_animation_fix->get_oldest_record( player );

  if( g_ctx.lagcomp )
  {
    int damage_last = -1;
    if( last )
      damage_last = get_record_damage( player, last );

    int damage_old = -1;
    if( old )
      damage_old = get_record_damage( player, old );

    if( damage_old > damage_last )
      return old;

    return last;
  }

  return last;
}

bool is_point_predictive( c_csplayer* player, point_t& point )
{
  if( !g_rage_bot->should_stop( ) || !( cheat_tools::get_weapon_config( ).quick_stop_options & early ) )
    return false;

  int dmg = g_rage_bot->get_min_damage( player );
  auto& esp_info = g_esp_store->playerinfo [ player->index( ) ];
  if( !esp_info.valid )
    return false;

  float speed = std::max< float >( g_engine_prediction->unprediced_velocity.length( true ), 1.f );

  int max_stop_ticks = std::max< int >( ( ( speed / g_movement->get_max_speed( ) ) * 7.f ) - 1, 0 );
  if( max_stop_ticks == 0 )
    return false;

  vector3d last_predicted_velocity = g_engine_prediction->unprediced_velocity;
  for( int i = 0; i < max_stop_ticks; ++i )
  {
    auto pred_velocity = g_engine_prediction->unprediced_velocity * math::ticks_to_time( i + 1 );

    vector3d origin = g_ctx.eye_position + pred_velocity;
    int flags = g_ctx.local->flags( );

    g_utils->extrapolate( g_ctx.local, origin, pred_velocity, flags, flags & fl_onground );

    last_predicted_velocity = pred_velocity;
  }

  auto predicted_eye_pos = g_ctx.eye_position + last_predicted_velocity;

  if( player->dormant( ) )
  {
    vector3d poses [ 3 ]{ player->get_abs_origin( ), player->get_abs_origin( ) + player->view_offset( ), player->get_abs_origin( ) + vector3d( 0.f, 0.f, player->view_offset( ).z / 2.f ) };

    for( int i = 0; i < 3; ++i )
    {
      c_trace_filter filter{ };
      filter.skip = g_ctx.local;

      c_game_trace out{ };
      interfaces::engine_trace->trace_ray( ray_t( predicted_eye_pos, poses [ i ] ), mask_shot_hull | contents_hitbox, &filter, &out );

      if( out.fraction >= 0.97f )
        return true;
      else
        continue;
    }
  }
  else
    return g_auto_wall->can_hit_point( player, point.position, predicted_eye_pos, dmg );

  return false;
}

void force_scope( )
{
  bool able_to_zoom = g_ctx.predicted_curtime >= g_ctx.weapon->next_secondary_attack( );

  if( able_to_zoom && cheat_tools::get_weapon_config( ).auto_scope && g_ctx.weapon->zoom_level( ) < 1 && g_utils->on_ground( ) && g_ctx.weapon->is_sniper( ) )
    g_ctx.cmd->buttons |= in_attack2;
}

void thread_build_points( aim_cache_t* aim_cache )
{
  aim_cache->points.clear( );

  if( !aim_cache->player )
    return;

  auto best_record = get_best_record( aim_cache->player );
  if( !best_record )
    return;

  g_rage_bot->store( aim_cache->player );
  g_rage_bot->set_record( aim_cache->player, best_record );

  for( auto& hitbox : g_rage_bot->get_hitboxes( ) )
  {
    const auto& pts = cheat_tools::get_multipoints( aim_cache->player, hitbox, best_record->sim_orig.bone );
    for( auto& p : pts )
    {
      auto awall = g_auto_wall->fire_bullet( g_ctx.local, aim_cache->player, g_ctx.weapon_info, g_ctx.weapon->is_taser( ), g_ctx.eye_position, p.first );

      // interfaces::debug_overlay->add_text_overlay(p.first, interfaces::global_vars->interval_per_tick * 2.f, "%d", awall.dmg);

      auto new_point = point_t( hitbox, p.second, awall.dmg, best_record, p.first );

      if( p.second )
        new_point.predictive = is_point_predictive( aim_cache->player, new_point );

#ifdef _DEBUG
      if( g_rage_bot->debug_aimbot )
      {
        interfaces::debug_overlay->add_box_overlay(
          new_point.position, vector3d( -1, -1, -1 ), vector3d( 1, 1, 1 ), { }, 255, new_point.center ? 255 : 0, new_point.center ? 255 : 0, 200, interfaces::global_vars->interval_per_tick * 2.f );

        interfaces::debug_overlay->add_text_overlay( new_point.position, interfaces::global_vars->interval_per_tick * 2.f, "%d", new_point.damage );
      }
#endif

      aim_cache->points.emplace_back( new_point );
    }
  }

  g_rage_bot->restore( aim_cache->player );
}

void thread_get_best_point( aim_cache_t* aim_cache )
{
  if( !aim_cache->player )
    return;

  int health = aim_cache->player->health( );
  int lethal_dmg = g_rage_bot->can_dt( ) ? health / 2 : health;

  aim_cache->best_point.reset( );

  if( aim_cache->points.empty( ) )
    return;

  int dmg = g_rage_bot->get_min_damage( aim_cache->player );

  // prepare points in right order
  // because we need to prefer the best points to tap enemy
  std::sort( aim_cache->points.begin( ), aim_cache->points.end( ), [ & ]( point_t& a, point_t& b ) { return a.center > b.center; } );
  std::sort( aim_cache->points.begin( ), aim_cache->points.end( ), [ & ]( point_t& a, point_t& b ) { return a.damage > b.damage; } );

  auto select_prefered_point = [ & ]( ) -> point_t
  {
    point_t best{ };
    for( auto& point : aim_cache->points )
    {
      // force stop before peek
      if( point.predictive )
      {
        force_scope( );

        if( g_rage_bot->should_stop( ) )
        {
          g_rage_bot->auto_stop( );
        }

        point.predictive = false;
      }

      if( point.damage < dmg || !point.body && g_cfg.binds [ force_body_b ].toggled )
        continue;

      // prefer lethal enemies
      else if( point.body && point.center && point.damage >= lethal_dmg )
        return point;

      // choose by best dmg
      else
      {
        if( point.damage > best.damage )
          best = point;
      }
    }

    return best;
  };

  aim_cache->best_point = select_prefered_point( );
}

void c_rage_bot::proceed_aimbot( )
{
  const std::unique_lock< std::mutex > lock( mutexes::rage );

#ifdef _DEBUG
  if( cheat_tools::debug_hitchance )
  {
    cheat_tools::spread_point.reset( );
    cheat_tools::current_spread = 0.f;
    cheat_tools::spread_points.clear( );
  }
#endif

  target = nullptr;
  working = false;
  stopping = false;
  reset_data = false;
  force_accuracy = true;

  if( !g_ctx.weapon || interfaces::game_rules->is_freeze_time( ) || g_ctx.local->flags( ) & fl_frozen || g_ctx.local->gun_game_immunity( ) )
  {
    return;
  }

  this->knife_bot( );

  bool invalid_weapon = g_ctx.weapon->is_misc_weapon( ) && !g_ctx.weapon->is_taser( );

  if( !g_cfg.rage.enable || invalid_weapon )
  {
    if( reset_scan_data )
    {
      for( auto& b : backup )
        b.reset( );

      target = nullptr;
      reset_scan_data = false;
    }

    return;
  }

  reset_scan_data = true;

  float hitchance = std::clamp( cheat_tools::get_weapon_config( ).hitchance / 100.f, 0.f, 1.f );

  auto& players = g_listener_entity->get_entity( ent_player );
  if( players.empty( ) )
    return;

  point_t best_point{ };

  int index_iter = 0;
  for( auto& player : players )
  {
    auto entity = ( c_csplayer* )player.m_entity;
    if( !entity )
      continue;

    if( entity == g_ctx.local || entity->team( ) == g_ctx.local->team( ) )
      continue;

    auto& cache = aim_cache [ entity->index( ) ];

    if( !entity->is_alive( ) || entity->dormant( ) || entity->gun_game_immunity( ) )
    {
      target = nullptr;

      if( !cache.points.empty( ) )
        cache.points.clear( );

      if( cache.best_point.filled )
        cache.best_point.reset( );

      if( cache.player )
        cache.player = nullptr;

      continue;
    }

    ++index_iter;

    cache.player = entity;

#ifdef _DEBUG
    thread_build_points( &cache );
#else
    g_thread_pool->enqueue( thread_build_points, &cache );
#endif
  }

  if( index_iter < 1 )
    return;

#ifndef _DEBUG
  g_thread_pool->wait( );
#endif

  for( auto& player : players )
  {
    auto entity = ( c_csplayer* )player.m_entity;
    if( !entity || entity == g_ctx.local )
      continue;

    if( entity->team( ) == g_ctx.local->team( ) || !entity->is_alive( ) || entity->dormant( ) || entity->gun_game_immunity( ) )
      continue;

    auto& cache = aim_cache [ entity->index( ) ];
    if( !cache.player || cache.player != entity )
      continue;

    thread_get_best_point( &cache );
  }

  int highest_damage = INT_MIN;

  should_slide = false;

  for( auto& player : players )
  {
    auto entity = ( c_csplayer* )player.m_entity;
    if( !entity || entity == g_ctx.local )
      continue;

    if( entity->team( ) == g_ctx.local->team( ) || !entity->is_alive( ) || entity->dormant( ) || entity->gun_game_immunity( ) )
      continue;

    auto cache = &aim_cache [ entity->index( ) ];
    if( !cache || !cache->player || cache->player != entity )
      continue;

    if( !cache->best_point.filled )
    {
      this->restore( entity );
      continue;
    }

    if( highest_damage < cache->best_point.damage )
    {
      target            = entity;
      highest_damage    = cache->best_point.damage;
    }
  }

  if( target )
    best_point = aim_cache [ target->index( ) ].best_point;

  if( best_point.filled )
  {
    working = true;
    stopping = true;

    force_scope( );

    bool shoot_on_unlag = true;
    if( !g_cfg.binds [ sw_b ].toggled && g_cfg.rage.delay_shot && interfaces::client_state->choked_commands > 2 )
      shoot_on_unlag = false;

    if( shoot_on_unlag && force_accuracy && cheat_tools::is_accuracy_valid( target, best_point, hitchance, &best_point.hitchance ) )
    {
      if( g_utils->is_able_to_shoot( true ) )
      {
        if( g_cfg.rage.auto_fire )
        {
          if( !g_anti_aim->is_fake_ducking( ) )
          {
            if( g_cfg.binds [ hs_b ].toggled )
              *g_ctx.send_packet = true;
            else
            {
              if( !interfaces::client_state->choked_commands )
                *g_ctx.send_packet = false;
            }
          }

          g_ctx.cmd->buttons |= in_attack;
        }

        // interfaces::engine->set_view_angles(g_ctx.cmd->viewangles);

        if( g_ctx.cmd->buttons & in_attack )
        {
          firing = true;

          if( g_ctx.lagcomp )
            g_ctx.cmd->tickcount = math::time_to_ticks( best_point.record->sim_time + g_ctx.lerp_time );

          g_ctx.cmd->viewangles = math::normalize( math::angle_from_vectors( g_ctx.eye_position, best_point.position ), true );
          g_ctx.cmd->viewangles -= g_ctx.local->aim_punch_angle( ) * cvars::weapon_recoil_scale->get_float( );

          g_ctx.cmd->viewangles = math::normalize( g_ctx.cmd->viewangles, true );

          g_ctx.shot_cmd = g_ctx.cmd->command_number;
          g_ctx.last_shoot_position = g_ctx.eye_position;

          if( g_cfg.visuals.chams [ c_onshot ].enable )
            g_chams->add_shot_record( target, best_point.record->sim_orig.bone );
#ifdef _DEBUG
#if DEBUG_LC
          draw_hitbox( target, best_point.record->sim_orig.bone, 0, 0, false );
#endif

#if DEBUG_SP
          draw_hitbox( target, best_point.record->sim_left.bone, 0, 0, false );
          draw_hitbox( target, best_point.record->sim_right.bone, 1, 0, false );
          draw_hitbox( target, best_point.record->sim_zero.bone, 0, 1, false );
#endif
#endif

          this->add_shot_record( target, best_point );

          this->restore( target );
        }
      }
    }
  }
}

void c_rage_bot::on_predict_start( )
{
  auto& players = g_listener_entity->get_entity( ent_player );
  if( players.empty( ) )
    return;

  for( auto& player : players )
  {
    auto entity = ( c_csplayer* )player.m_entity;
    if( !entity )
      continue;

    if( entity == g_ctx.local || entity->team( ) == g_ctx.local->team( ) )
      continue;

    if( !entity->is_alive( ) || entity->dormant( ) )
      continue;

    this->store( entity );
  }

  this->proceed_aimbot( );

  for( auto& player : players )
  {
    auto entity = ( c_csplayer* )player.m_entity;
    if( !entity )
      continue;

    if( entity == g_ctx.local || entity->team( ) == g_ctx.local->team( ) )
      continue;

    if( !entity->is_alive( ) || entity->dormant( ) )
      continue;

    this->restore( entity );
  }
}

void c_rage_bot::on_local_death( )
{
  if( reset_data )
    return;

  for( auto& m : missed_shots )
    m = 0;

  reset_data = true;
}

void c_rage_bot::on_changed_map( )
{
  for( auto& b : backup )
    b.reset( );
}