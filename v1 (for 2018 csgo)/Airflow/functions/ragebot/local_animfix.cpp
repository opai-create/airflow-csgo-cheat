#include "local_animfix.h"

#include "../features.h"

#include "../config_vars.h"

class activity_modifiers_wrapper
{
private:
  uint32_t gap [ 0x4D ]{ 0 };
  c_utlvector< uint16_t > modifiers{ };

public:
  activity_modifiers_wrapper( ) = default;

  explicit activity_modifiers_wrapper( c_utlvector< uint16_t > current_modifiers )
  {
    modifiers.remove_all( );
    modifiers.grow_vector( current_modifiers.count( ) );

    for( auto i = 0; i < current_modifiers.count( ); i++ )
      modifiers [ i ] = current_modifiers [ i ];
  }

  void add_modifier( const char* name )
  {
    using add_activity_modifier_fn_t = void( __thiscall* )( void*, const char* );
    static const auto add_activity_modifier = g_memory->find_pattern( modules::server, "55 8B EC 8B 55 ? 83 EC ? 56" )
        .as< add_activity_modifier_fn_t >( );

    if( add_activity_modifier && name )
      add_activity_modifier( this, name );
  };

  c_utlvector< uint16_t > get( ) const
  {
    return modifiers;
  }
};

void c_local_animation_fix::rebuild_animations( c_animation_layers* layer )
{
  auto state = g_ctx.local->animstate( );
  if( !state )
    return;

  if( interfaces::game_rules->is_freeze_time( ) || ( g_ctx.local->flags( ) & fl_frozen ) )
  {
    local_info.real_type = movetype_none;
    local_info.real_flags = fl_onground;

    return;
  }

  static auto get_weapon_prefix = patterns::get_weapon_prefix.as< const char*( __thiscall* )( void* ) >( );

  activity_modifiers_wrapper modifier_wrapper{ };

  modifier_wrapper.add_modifier( get_weapon_prefix( state ) );

  if( state->speed_as_portion_of_run_top_speed > .25f )
    modifier_wrapper.add_modifier( "moving" );

  if( state->anim_duck_amount > .55f )
    modifier_wrapper.add_modifier( "crouch" );

  auto mod = modifier_wrapper.get( );

  static auto find_mapping = patterns::find_mapping.as< void*( __thiscall* )( void* ) >( );
  static auto select_sequence_from_mods = patterns::select_sequence_from_mods.as< int( __thiscall* )( void*, void*, int, uint16_t*, int ) >( );

  auto hdr = g_ctx.local->get_studio_hdr( );
  auto mapping = find_mapping( g_ctx.local->get_studio_hdr( ) );

  if( !hdr && !mapping )
    return;

  auto adjust = &layer [ animation_layer_adjust ];
  if( !adjust )
    return;

  bool bStartedMovingThisFrame = false;
  bool bStoppedMovingThisFrame = false;

  if( state->velocity_length_xy > 0 )
  {
    bStartedMovingThisFrame = ( state->duration_moving <= 0 );
  }
  else
  {
    bStoppedMovingThisFrame = ( state->duration_still <= 0 );
  }

  static bool landing = false;

  if( !state->adjust_started && bStoppedMovingThisFrame && state->on_ground && !state->on_ladder && !landing && state->stutter_step < 50 )
  {
    int activity = select_sequence_from_mods( mapping, hdr, act_csgo_idle_adjust_stoppedmoving, mod.base( ), mod.count( ) );
    state->set_layer_sequence( adjust, activity );
    state->adjust_started = true;
  }

  auto alive_loop = &layer [ animation_layer_aliveloop ];
  if( !alive_loop )
    return;

  if( g_ctx.local->get_sequence_activity( alive_loop->sequence ) != act_csgo_alive_loop )
  {
    int activity = select_sequence_from_mods( mapping, hdr, act_csgo_alive_loop, mod.base( ), mod.count( ) );

    state->set_layer_sequence( alive_loop, activity );
    state->set_layer_cycle( alive_loop, math::random_float( 0.0f, 1.0f ) );
    state->set_layer_rate( alive_loop, g_ctx.local->get_layer_sequence_cycle_rate( alive_loop, alive_loop->sequence ) * math::random_float( 0.8, 1.1f ) );
  }
  else
  {
    float retain_cycle = alive_loop->cycle;
    if( state->weapon != state->weapon_last )
    {
      int activity = select_sequence_from_mods( mapping, hdr, act_csgo_alive_loop, mod.base( ), mod.count( ) );

      state->set_layer_sequence( alive_loop, activity );
      state->set_layer_cycle( alive_loop, retain_cycle );
    }
    else if( state->is_layer_sequence_finished( alive_loop, state->last_update_increment ) )
      state->set_layer_rate( alive_loop, g_ctx.local->get_layer_sequence_cycle_rate( alive_loop, alive_loop->sequence ) * math::random_float( 0.8, 1.1f ) );
    else
      state->set_layer_weight( alive_loop, math::remap_val_clamped( state->speed_as_portion_of_run_top_speed, 0.55f, 0.9f, 1.0f, 0.0f ) );
  }

  state->increment_layer_cycle( alive_loop, true );

  auto land_or_climb = &layer [ animation_layer_movement_land_or_climb ];
  if( !land_or_climb )
    return;

  auto jump_or_fall = &layer [ animation_layer_movement_jump_or_fall ];
  if( !jump_or_fall )
    return;

  auto ground_entity = interfaces::entity_list->get_entity_handle( g_ctx.local->ground_entity( ) );

  auto current_flags = g_ctx.local->flags( );

  bool landed = ( current_flags & fl_onground ) && !( local_info.real_flags & fl_onground );
  bool jumped = !( current_flags & fl_onground ) && ( local_info.real_flags & fl_onground );

  local_info.real_flags = current_flags;

  if( local_info.real_type != movetype_ladder && g_ctx.local->move_type( ) == movetype_ladder )
  {
    int activity = select_sequence_from_mods( mapping, hdr, act_csgo_climb_ladder, mod.base( ), mod.count( ) );
    state->set_layer_sequence( land_or_climb, activity );
  }
  else if( local_info.real_type == movetype_ladder && g_ctx.local->move_type( ) != movetype_ladder )
  {
    int activity = select_sequence_from_mods( mapping, hdr, act_csgo_climb_ladder, mod.base( ), mod.count( ) );
    state->set_layer_sequence( land_or_climb, activity );
  }
  else
  {
    if( current_flags & fl_onground )
    {
      if( !landing && landed )
      {
        int current_sequence = state->duration_in_air > 1.f ? act_csgo_land_heavy : act_csgo_land_light;

        int activity = select_sequence_from_mods( mapping, hdr, current_sequence, mod.base( ), mod.count( ) );

        state->set_layer_sequence( land_or_climb, activity );

        landing = true;
      }
    }
    else if( g_ctx.local->move_type( ) != movetype_ladder )
    {
      auto current_flags = g_ctx.local->flags( );

      if( ( g_ctx.cmd->buttons & in_jump ) && !ground_entity )
      {
        int activity = select_sequence_from_mods( mapping, hdr, act_csgo_jump, mod.base( ), mod.count( ) );

        state->set_layer_sequence( jump_or_fall, activity );
      }
    }
  }

  local_info.real_type = g_ctx.local->move_type( );
}

void c_local_animation_fix::update_fake( )
{
  // don't need to create state and other shit when chams disabled
  // affects on performance
  if( !g_cfg.visuals.chams [ c_fake ].enable )
  {
    if( state )
    {
      interfaces::memory->free( state );
      state = nullptr;
    }
    reset = false;
    return;
  }

  // force recreate animstate when player spawned
  if( old_spawn != g_ctx.local->spawn_time( ) )
  {
    interfaces::memory->free( state );
    state = nullptr;

    old_spawn = g_ctx.local->spawn_time( );
    reset = false;
    return;
  }

  // force recreate animstate when handle changed (disconnect / map change)
  if( old_handle != g_ctx.local->get_ref_handle( ) )
  {
    interfaces::memory->free( state );
    state = nullptr;

    old_handle = g_ctx.local->get_ref_handle( );
    reset = false;
    return;
  }

  // don't do anything when game is frozen
  if( interfaces::client_state->delta_tick == -1 )
    return;

  if( !reset )
  {
    if( !state )
      state = ( c_animstate* )interfaces::memory->alloc( sizeof( c_animstate ) );

    state->create( g_ctx.local );
    reset = true;
  }

  if( !reset || !state )
    return;

  auto& info = this->local_info;

  auto old_angle = g_ctx.local->render_angles( );

  g_ctx.local->render_angles( ) = g_ctx.cur_angle;

  state->update( g_ctx.fake_angle );

  g_ctx.local->set_abs_angles( { 0.f, state->abs_yaw, 0.f } );
  g_ctx.local->setup_uninterpolated_bones( info.matrix_fake );

  vector3d render_origin = g_ctx.local->get_render_origin( );
  math::change_matrix_position( info.matrix_fake, 128, render_origin, vector3d( ) );

  g_ctx.local->render_angles( ) = old_angle;
}

void c_local_animation_fix::update_strafe_state( )
{
  auto state = g_ctx.local->animstate( );
  if( !state )
    return;

  int buttons = g_engine_prediction->predicted_buttons;

  vector3d forward;
  vector3d right;
  vector3d up;

  math::angle_to_vectors( vector3d( 0, state->abs_yaw, 0 ), forward, right, up );
  right = right.normalized( );

  auto velocity = state->velocity_normalized_non_zero;
  auto speed = state->speed_as_portion_of_walk_top_speed;

  float vel_to_right_dot = velocity.dot( right );
  float vel_to_foward_dot = velocity.dot( forward );

  bool move_right = ( buttons & ( in_moveright ) ) != 0;
  bool move_left = ( buttons & ( in_moveleft ) ) != 0;
  bool move_forward = ( buttons & ( in_forward ) ) != 0;
  bool move_backward = ( buttons & ( in_back ) ) != 0;

  bool strafe_right = ( speed >= 0.73f && move_right && !move_left && vel_to_right_dot < -0.63f );
  bool strafe_left = ( speed >= 0.73f && move_left && !move_right && vel_to_right_dot > 0.63f );
  bool strafe_forward = ( speed >= 0.65f && move_forward && !move_backward && vel_to_foward_dot < -0.55f );
  bool strafe_backward = ( speed >= 0.65f && move_backward && !move_forward && vel_to_foward_dot > 0.55f );

  g_ctx.local->strafing( ) = ( strafe_right || strafe_left || strafe_forward || strafe_backward );
}

void c_local_animation_fix::update_viewmodel( )
{
  auto viewmodel = g_ctx.local->get_view_model( );
  if( viewmodel )
    func_ptrs::update_all_viewmodel_addons( viewmodel );
}

void c_local_animation_fix::on_predict_end( )
{
  if( !g_ctx.cmd || !g_ctx.local || !g_ctx.local->is_alive( ) )
    return;

  auto state = g_ctx.local->animstate( );
  if( !state )
    return;

  if( interfaces::client_state->delta_tick == -1 )
  {
    g_ctx.local->force_update( );
    g_ctx.setup_bones [ g_ctx.local->index( ) ] = true;
    return;
  }

  if( ( g_exploits->recharge && !g_exploits->recharge_finish ) || g_exploits->hs_works )
    return;

 // g_ctx.local->draw_server_hitbox( );

  auto& info = this->local_info;

  static float anim_time = 0.f;

  if( interfaces::client_state->choked_commands )
    return;

  this->update_fake( );

  anim_time = interfaces::global_vars->cur_time;

  auto old_angle = g_ctx.local->render_angles( );

  g_ctx.local->render_angles( ) = g_ctx.cur_angle;
  g_ctx.local->lby( ) = local_info.lby_angle;

  // force update
  this->update_viewmodel( );

  auto layers = g_ctx.local->anim_overlay( );

  g_ctx.update [ g_ctx.local->index( ) ] = true;
  state->update( g_ctx.cur_angle );
  g_ctx.update [ g_ctx.local->index( ) ] = false;

  g_ctx.local->store_poses( info.render_poses );
  g_ctx.local->store_layer( info.render_layers );

  if( g_cfg.misc.animation_changes & 1 && ( state->landing ) )
    info.render_poses [ 12 ] = 0.5f;

  if( g_cfg.misc.animation_changes & 2 )
    info.render_poses [ 0 ] = 1.f;

  if( !local_info.on_ground && state->on_ground )
  {
    local_info.lby_angle = g_ctx.cur_angle.y;
    local_info.last_lby_time = anim_time;
  }
  else if( state->velocity_length_xy > 0.1f )
  {
    if( state->on_ground )
      local_info.lby_angle = g_ctx.cur_angle.y;

    local_info.last_lby_time = anim_time + 0.22f;
  }
  else if( anim_time > local_info.last_lby_time )
  {
    local_info.lby_angle = g_ctx.cur_angle.y;
    local_info.last_lby_time = anim_time + 1.1f;
  }

  local_info.on_ground = state->on_ground;
  local_info.last_lby_tick = interfaces::global_vars->interval_per_tick;

  if( !( g_cfg.misc.animation_changes & 4 ) )
    layers [ animation_layer_lean ].weight = 0.f;

  std::array< float, 24 > old_render_poses{ };

  g_ctx.local->store_poses( old_render_poses );

  g_ctx.local->set_poses( info.render_poses );
  g_ctx.local->set_abs_angles( { 0.f, state->abs_yaw, 0.f } );

  g_ctx.local->setup_uninterpolated_bones( info.matrix );

  g_ctx.local->set_poses( old_render_poses );

  vector3d render_origin = g_ctx.local->get_render_origin( );
  math::change_matrix_position( info.matrix, 128, render_origin, vector3d( ) );

  g_ctx.local->render_angles( ) = old_angle;
}

void c_local_animation_fix::on_render_start_after( int stage )
{
  if( !g_ctx.in_game || !g_ctx.local || interfaces::client_state->delta_tick == -1 )
    return;

  if( stage != frame_render_start )
    return;

  if( !g_ctx.local->is_alive( ) )
    return;

  auto state = g_ctx.local->animstate( );
  if( !state )
    return;

  auto& info = this->local_info;

  vector3d render_origin = g_ctx.local->get_render_origin( );
  math::change_matrix_position( info.matrix, 128, vector3d( ), render_origin );

  g_ctx.local->interpolate_moveparent_pos( );
  g_ctx.local->set_bone_cache( info.matrix );
  g_ctx.local->attachments_helper( );

  math::change_matrix_position( info.matrix, 128, render_origin, vector3d( ) );
}

void c_local_animation_fix::on_game_events( c_game_event* event )
{
  if( std::strcmp( event->get_name( ), xor_c( "round_start" ) ) )
    return;

  local_info.reset( );
}

void c_local_animation_fix::on_local_death( )
{
  local_info.reset( );
}