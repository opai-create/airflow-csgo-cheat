#include "engine_prediction.h"

#include "../features.h"

#include <thread>

void c_engine_prediction::on_render_start( int stage, bool after )
{
  if( stage != frame_render_start )
    return;

  if( !g_ctx.is_alive )
    return;

  if( !after )
  {
    interp_amount = interfaces::global_vars->interpolation_amount;

    if( g_exploits->recharge && !g_exploits->recharge_finish )
      interfaces::global_vars->interpolation_amount = 0.f;
  }
  else
    interfaces::global_vars->interpolation_amount = interp_amount;
}

void c_engine_prediction::net_compress_store( int tick )
{
  if( !g_ctx.local || !g_ctx.local->is_alive( ) )
  {
    this->reset( );
    return;
  }

  auto& data = net_data [ tick % 150 ];
  data.cmd_number = tick;
  data.vel_modifier = g_ctx.local->velocity_modifier( );
  data.fall_velocity = g_ctx.local->fall_velocity( );
  data.duck_amt = g_ctx.local->duck_amount( );
  data.duck_speed = g_ctx.local->duck_speed( );
  data.thirdperson_recoil = g_ctx.local->thirdperson_recoil( );
  data.punch = g_ctx.local->aim_punch_angle( );
  data.punch_vel = g_ctx.local->aim_punch_angle_vel( );
  data.view_offset = g_ctx.local->view_offset( );
  data.view_punch = g_ctx.local->view_punch_angle( );
  data.velocity = g_ctx.local->velocity( );
  data.filled = true;

  reset_net_data = true;
}

void c_engine_prediction::net_compress_apply( int tick )
{
  if( !g_ctx.local || !g_ctx.local->is_alive( ) )
  {
    this->reset( );
    return;
  }

  auto& data = net_data [ tick % 150 ];
  if( data.cmd_number != tick || !data.filled )
    return;

  auto aim_punch_vel_diff = data.punch_vel - g_ctx.local->aim_punch_angle_vel( );
  auto aim_punch_diff = data.punch - g_ctx.local->aim_punch_angle( );
  auto viewoffset_diff = data.view_offset - g_ctx.local->view_offset( );
  auto velocity_diff = data.velocity - g_ctx.local->velocity( );

  if( std::abs( aim_punch_diff.x ) < 0.03125f && std::abs( aim_punch_diff.y ) < 0.03125f && std::abs( aim_punch_diff.z ) < 0.03125f )
    g_ctx.local->aim_punch_angle( ) = data.punch;

  if( std::abs( aim_punch_vel_diff.x ) < 0.03125f && std::abs( aim_punch_vel_diff.y ) < 0.03125f && std::abs( aim_punch_vel_diff.z ) < 0.03125f )
    g_ctx.local->aim_punch_angle_vel( ) = data.punch_vel;

  if( std::abs( viewoffset_diff.x ) < 0.03125f && std::abs( viewoffset_diff.y ) < 0.03125f && std::abs( viewoffset_diff.z ) < 0.03125f )
    g_ctx.local->view_offset( ) = data.view_offset;

  reset_net_data = true;
}

void c_engine_prediction::init( )
{
  prediction_random_seed = *patterns::prediction_random_seed.as< int** >( );
  prediction_player = *patterns::prediction_player.as< int** >( );
}

void c_engine_prediction::update( )
{
  interfaces::prediction->update(
    interfaces::client_state->delta_tick, interfaces::client_state->delta_tick > 0, interfaces::client_state->last_command_ack, interfaces::client_state->last_outgoing_command + interfaces::client_state->choked_commands );
}

void c_engine_prediction::start( c_csplayer* local, c_usercmd* cmd )
{
  if( !local )
    return;

  auto weapon = local->get_active_weapon( );
  if( !weapon )
    return;

  unpred_vars [ cmd->command_number % 150 ].fill( );

  unprediced_velocity = local->velocity( );
  unpredicted_flags = local->flags( );

  old_cur_time = interfaces::global_vars->cur_time;
  old_frame_time = interfaces::global_vars->frame_time;
  old_tick_count = interfaces::global_vars->tick_count;

  old_in_prediction = interfaces::prediction->in_prediction;
  old_first_time_predicted = interfaces::prediction->is_first_time_predicted;

  old_seed = *prediction_random_seed;

  old_recoil_index = weapon->recoil_index( );
  old_accuracy_penalty = weapon->accuracy_penalty( );

  interfaces::global_vars->cur_time = math::ticks_to_time( g_ctx.tick_base );
  interfaces::global_vars->frame_time = interfaces::global_vars->interval_per_tick;

  *( c_usercmd** )( ( uintptr_t )local + 0x3314 ) = cmd;
  *( c_usercmd* )( ( uintptr_t )local + 0x326C ) = *cmd;

  *prediction_random_seed = MD5_PseudoRandom( cmd->command_number ) & 0x7FFFFFFF;
  *prediction_player = ( int )local;

  interfaces::prediction->in_prediction = true;
  interfaces::prediction->is_first_time_predicted = false;

  this->repredict( local, cmd, true );
}

void c_engine_prediction::force_update_eyepos( const float& pitch )
{
  auto state = g_ctx.local->animstate( );
  if( !state )
    return;

  const auto old_abs_angles = g_ctx.local->get_abs_angles( );
  const auto old_poses = *( float* )( ( uintptr_t )g_ctx.local + ( offsets::m_flPoseParameter + 48 ) );

  g_ctx.local->set_abs_angles( { 0.f, state->abs_yaw, 0.f } );

  auto eye_pitch = math::normalize( pitch );

  if( eye_pitch > 180.f )
    eye_pitch = eye_pitch - 360.f;

  eye_pitch = std::clamp( eye_pitch, -90.f, 90.f );
  *( float* )( ( uintptr_t )g_ctx.local + ( offsets::m_flPoseParameter + 48 ) ) = std::clamp( ( eye_pitch + 90.f ) / 180.f, 0.0f, 1.0f );

  g_ctx.local->invalidate_bone_cache( );

  const auto old_abs_origin = g_ctx.local->get_abs_origin( );
  g_ctx.local->set_abs_origin( g_ctx.local->origin( ) );

  g_ctx.setup_bones [ g_ctx.local->index( ) ] = true;
  g_ctx.local->setup_bones( nullptr, -1, 0x100, g_ctx.local->simulation_time( ) );
  g_ctx.setup_bones [ g_ctx.local->index( ) ] = false;

  g_ctx.local->set_abs_origin( old_abs_origin );

  g_ctx.eye_position = g_ctx.local->get_eye_position( );

  *( float* )( ( uintptr_t )g_ctx.local + ( offsets::m_flPoseParameter + 48 ) ) = old_poses;
  g_ctx.local->set_abs_angles( old_abs_angles );
}

void c_engine_prediction::repredict( c_csplayer* local, c_usercmd* cmd, bool real_cmd )
{
  if( !local )
    return;

  auto state = local->animstate( );
  if( !state )
    return;

  auto weapon = local->get_active_weapon( );
  if( !weapon )
    return;

  auto old_tickbase = local->tickbase( );

  if( real_cmd )
  {
    cmd->buttons |= *reinterpret_cast< uint8_t* >( uintptr_t( local ) + 0x3310 );

    if( cmd->impulse )
      *reinterpret_cast< uint8_t* >( uintptr_t( local ) + 0x31EC ) = cmd->impulse;
  }

  interfaces::game_movement->start_track_prediction_errors( local );

  if( real_cmd )
  {
    int buttonsChanged = cmd->buttons ^ *reinterpret_cast< int* >( uintptr_t( local ) + 0x31E8 );
    *reinterpret_cast< int* >( uintptr_t( local ) + 0x31DC ) = ( uintptr_t( local ) + 0x31E8 );
    *reinterpret_cast< int* >( uintptr_t( local ) + 0x31E8 ) = cmd->buttons;
    *reinterpret_cast< int* >( uintptr_t( local ) + 0x31E0 ) = cmd->buttons & buttonsChanged;  // m_afButtonPressed ~ The changed ones still down are "pressed"
    *reinterpret_cast< int* >( uintptr_t( local ) + 0x31E4 ) = buttonsChanged & ~cmd->buttons; // m_afButtonReleased ~ The ones not down are "released"
  }

  memset( &move_data, 0, sizeof( c_movedata ) );

  interfaces::prediction->check_moving_ground( local, interfaces::global_vars->frame_time );
  interfaces::prediction->set_local_view_angles( cmd->viewangles );

  local->run_pre_think( );
  local->run_think( );

  interfaces::move_helper->set_host( local );
  interfaces::prediction->setup_move( local, cmd, interfaces::move_helper, &move_data );
  interfaces::game_movement->process_movement( local, &move_data );
  interfaces::prediction->finish_move( local, cmd, &move_data );

  interfaces::move_helper->process_impacts( );

  local->post_think( );

  local->tickbase( ) = old_tickbase;

  interfaces::game_movement->finish_track_prediction_errors( local );
  interfaces::move_helper->set_host( nullptr );

  if( weapon && !weapon->is_misc_weapon( ) )
  {
    weapon->update_accuracy_penalty( );

    predicted_inaccuracy = weapon->get_inaccuracy( );
    predicted_spread = weapon->get_spread( );
  }
}

void c_engine_prediction::finish( c_csplayer* local )
{
  if( !local )
    return;

  auto weapon = local->get_active_weapon( );
  if( !weapon )
    return;

  *( c_usercmd** )( ( uintptr_t )local + 0x3314 ) = 0;
  *prediction_random_seed = old_seed;
  *prediction_player = 0;

  interfaces::global_vars->cur_time = old_cur_time;
  interfaces::global_vars->frame_time = old_frame_time;

  if( weapon && !weapon->is_misc_weapon( ) )
  {
    weapon->recoil_index( ) = old_recoil_index;
    weapon->accuracy_penalty( ) = old_accuracy_penalty;
  }

  interfaces::game_movement->reset( );

  interfaces::prediction->in_prediction = old_in_prediction;
  interfaces::prediction->is_first_time_predicted = old_first_time_predicted;
}