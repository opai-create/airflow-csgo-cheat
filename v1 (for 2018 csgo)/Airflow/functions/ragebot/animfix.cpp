#include "animfix.h"
#include "../features.h"
#include "../../base/tools/threads.h"

void c_animation_fix::anim_player_t::simulate_animation_side( records_t* record )
{
  auto state = ptr->animstate( );
  if( !state )
    return;

  if( !last_record )
    state->last_update_time = ( record->sim_time - interfaces::global_vars->interval_per_tick );

  restore_anims_t restore{ };
  restore.store( ptr );

  if( !teammate )
    resolver::start( ptr, record );

  if( last_record && record->choke >= 2 )
  {
    if( record->on_ground )
      ptr->flags( ) |= fl_onground;
    else
      ptr->flags( ) &= ~fl_onground;

    if( state->on_ground && state->velocity_length_xy <= 0.1f && !state->landing && state->last_update_increment > 0.f )
    {
      float delta = math::normalize( std::abs( state->abs_yaw - state->abs_yaw_last ) );
      if( ( delta / state->last_update_increment ) > 120.f )
      {
        record->sim_orig.layers [ 3 ].cycle = record->sim_orig.layers [ 3 ].weight = 0.f;
        record->sim_orig.layers [ 3 ].sequence = ptr->get_sequence_activity( 979 );
      }
    }
  }

  this->force_update( );

  ptr->store_poses( record->poses );

  auto old = ptr->get_abs_origin( );
  ptr->set_abs_origin( record->origin );

  // 100iq lean fix
  if( last_record )
    record->sim_orig.layers [ 12 ].weight = last_record->sim_orig.layers [ 12 ].weight;
  else
    record->sim_orig.layers [ 12 ].weight = 0.f;

  ptr->set_layer( record->sim_orig.layers );
  this->build_bones( record, &record->sim_orig );

  ptr->set_abs_origin( old );

  restore.restore( ptr );
}

bool records_t::is_valid( )
{
  auto netchan = interfaces::engine->get_net_channel_info( );
  if( !netchan )
    return false;

  if( !g_ctx.lagcomp )
    return true;

  if( !valid )
    return false;

  float time = g_ctx.local->is_alive( ) ? g_ctx.predicted_curtime : interfaces::global_vars->cur_time;

  float correct = 0.f;

  correct += netchan->get_latency( flow_outgoing );
  correct += netchan->get_latency( flow_incoming );
  correct += g_ctx.lerp_time;

  correct = std::clamp< float >( correct, 0.0f, cvars::sv_maxunlag->get_float( ) );

  float deltaTime = correct - ( time - this->sim_time );

  return std::fabsf( deltaTime ) < 0.2f;
}

void c_animation_fix::anim_player_t::build_bones( records_t* record, records_t::simulated_data_t* sim )
{
  ptr->setup_uninterpolated_bones( sim->bone );
}

void c_animation_fix::anim_player_t::update_animations( )
{
  backup_record.update_record( ptr );

  if( records.size( ) > 0 )
  {
    last_record = &records.front( );

    if( records.size( ) >= 3 )
      old_record = &records [ 2 ];
  }

  auto& record = records.emplace_front( );
  record.update_record( ptr );
  record.update_dormant( dormant_ticks );
  record.update_shot( last_record );

  if( dormant_ticks < 1 )
    dormant_ticks++;

  this->update_land( &record );
  this->update_velocity( &record );
  this->simulate_animation_side( &record );

  backup_record.restore( ptr );

  if( g_ctx.lagcomp && last_record )
  {
    if( last_record->sim_time > record.sim_time )
    {
      next_update_time = record.sim_time + std::abs( last_record->sim_time - record.sim_time ) + math::ticks_to_time( 1 );
      record.valid = false;
    }
    else
    {
      if( math::time_to_ticks( std::abs( next_update_time - record.sim_time ) ) > 17 )
        next_update_time = -1.f;

      if( next_update_time > record.sim_time )
        record.valid = false;
    }
  }

  const auto records_size = teammate ? 3 : g_ctx.tick_rate;
  while( records.size( ) > records_size )
    records.pop_back( );
}

void thread_anim_update( c_animation_fix::anim_player_t* player )
{
  player->update_animations( );
}

void c_animation_fix::on_net_update_and_render_after( int stage )
{
  if( !g_ctx.in_game || !g_ctx.local || g_ctx.uninject )
    return;

  const std::unique_lock< std::mutex > lock( mutexes::animfix );

  if( !g_ctx.in_game || !g_ctx.local || g_ctx.uninject )
    return;

  auto& players = g_listener_entity->get_entity( ent_player );
  if( players.empty( ) )
    return;

  switch( stage )
  {
  case frame_net_update_postdataupdate_end:
  {
    g_rage_bot->on_pre_predict( );

    for( auto& player : players )
    {
      auto ptr = ( c_csplayer* )player.m_entity;
      if( !ptr )
        continue;

      if( ptr == g_ctx.local )
        continue;

      auto anim_player = this->get_animation_player( ptr->index( ) );
      if( anim_player->ptr != ptr )
      {
        anim_player->reset_data( );
        anim_player->ptr = ptr;
        continue;
      }

      if( !ptr->is_alive( ) )
      {
        if( !anim_player->teammate )
        {
          resolver::reset_info( ptr );
          g_rage_bot->missed_shots [ ptr->index( ) ] = 0;
        }

        anim_player->ptr = nullptr;
        continue;
      }

      if( g_cfg.misc.force_radar && ptr->team( ) != g_ctx.local->team( ) )
        ptr->target_spotted( ) = true;

      if( ptr->dormant( ) )
      {
        anim_player->dormant_ticks = 0;

        if( !anim_player->teammate )
          resolver::reset_info( ptr );
        continue;
      }

      if( ptr->simulation_time( ) == ptr->old_simtime( ) )
        continue;

      auto& layer = ptr->anim_overlay( ) [ 11 ];
      if( layer.cycle == anim_player->old_aliveloop_cycle )
        continue;

      anim_player->old_aliveloop_cycle = layer.cycle;

      auto state = ptr->animstate( );
      if( !state )
        continue;

      if( anim_player->old_spawn_time != ptr->spawn_time( ) )
      {
        state->player = ptr;
        state->reset( );

        anim_player->old_spawn_time = ptr->spawn_time( );
        continue;
      }

#ifdef _DEBUG
      anim_player->update_animations( );
#else
      g_thread_pool->enqueue( thread_anim_update, anim_player );
#endif
    }

#ifndef _DEBUG
    g_thread_pool->wait( );
#endif
  }
  break;
  case frame_render_start:
  {
    for( auto& player : players )
    {
      auto entity = ( c_csplayer* )player.m_entity;
      if( !entity || !entity->is_alive( ) || entity == g_ctx.local )
        continue;

      auto animation_player = this->get_animation_player( entity->index( ) );
      if( !animation_player || animation_player->records.empty( ) || animation_player->dormant_ticks < 1 )
      {
        g_ctx.setup_bones [ entity->index( ) ] = true;
        continue;
      }

      auto first_record = &animation_player->records.front( );
      if( !first_record || !first_record->sim_orig.bone )
        continue;

      std::memcpy( first_record->render_bones, first_record->sim_orig.bone, sizeof( first_record->render_bones ) );
      math::change_matrix_position( first_record->render_bones, 128, first_record->origin, entity->get_render_origin( ) );

      entity->interpolate_moveparent_pos( );

      entity->set_bone_cache( first_record->render_bones );
      entity->attachments_helper( );
    }
  }
  break;
  }
}
