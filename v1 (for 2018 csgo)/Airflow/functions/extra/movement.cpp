#include "movement.h"

#include "../config_vars.h"
#include "../features.h"
#include "../anti hit/exploits.h"

#include "../ragebot/engine_prediction.h"
#include "../ragebot/ragebot.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/tools/math.h"

#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

enum strafe_dir_t
{
  strafe_forwards = 0,
  strafe_backwards = 180,
  strafe_left = 90,
  strafe_right = -90,
  strafe_back_left = 135,
  strafe_back_right = -135
};

void c_movement::force_stop( )
{
  vector3d angle;
  math::vector_to_angles( g_ctx.local->velocity( ), angle );

  float speed = g_ctx.local->velocity( ).length( false );

  angle.y = g_ctx.orig_angle.y - angle.y;

  vector3d direction;
  math::angle_to_vectors( angle, direction );

  vector3d stop = direction * -speed;

  if( speed > 13.f )
  {
    g_ctx.cmd->forwardmove = stop.x;
    g_ctx.cmd->sidemove = stop.y;
  }
  else
  {
    g_ctx.cmd->forwardmove = 0.f;
    g_ctx.cmd->sidemove = 0.f;
  }
}

void c_movement::fast_stop( )
{
  if( !g_cfg.misc.fast_stop )
    return;

  if( peek_move )
    return;

  if( !g_utils->on_ground( ) || g_rage_bot->stopping || g_exploits->stop_movement )
    return;

  vector3d velocity = g_ctx.local->velocity( );
  float speed = velocity.length( true );

  if( speed < 5.f )
    return;

  bool pressing_move_keys = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_back || g_ctx.cmd->buttons & in_forward;

  if( pressing_move_keys )
    return;

  this->force_stop( );
}

void c_movement::auto_peek( )
{
  static bool old_move = false;

  bool moving = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_forward || g_ctx.cmd->buttons & in_back;

  vector3d origin = g_ctx.local->origin( );

  auto should_peek = [ & ]( )
  {
    if( g_ctx.weapon->is_misc_weapon( ) )
      return false;

    if( !g_cfg.binds [ ap_b ].toggled )
      return false;

    if( !peek_pos.valid( ) )
      peek_pos = origin;

    peek_start = true;

    if( g_utils->on_ground( ) )
    {
      if( ( g_utils->is_firing( ) || g_rage_bot->firing ) || ( g_cfg.misc.retrack_peek && !moving ) )
        peek_move = true;

      if( g_cfg.misc.retrack_peek && moving && !old_move )
        peek_move = false;
    }

    vector3d origin_delta = peek_pos - origin;
    float distance = origin_delta.length( true );

    if( peek_move )
    {
      if( g_exploits->cl_move.shifting )
        this->force_stop( );

      if( distance > 10.f )
      {
        vector3d return_position = math::angle_from_vectors( origin, peek_pos );
        g_ctx.base_angle.y = return_position.y;

        g_ctx.cmd->forwardmove = cvars::cl_forwardspeed->get_float( );
        g_ctx.cmd->sidemove = 0.f;
      }
      else
      {
        this->force_stop( );
        peek_move = false;
      }

      g_rage_bot->firing = false;
    }

    old_move = moving;
    return true;
  };

  if( !should_peek( ) )
  {
    peek_pos.reset( );
    peek_start = false;
    peek_move = false;
  }
}

void c_movement::jitter_move( )
{
  if( !g_cfg.antihit.enable )
    return;

  if( g_cfg.binds [ ap_b ].toggled || g_cfg.binds [ sw_b ].toggled || g_rage_bot->stopping )
    return;

  if( !g_utils->on_ground( ) )
    return;

  float tickrate_rate = 5.f / g_ctx.tick_rate;

  float rate = ( ( g_ctx.cmd->command_number % g_ctx.tick_rate ) * tickrate_rate ) + 95.f;
  float strength = std::clamp( rate, 95.f, 100.f );

  float max_speed = ( strength / 100.0f ) * this->max_speed;
  this->force_speed( max_speed );
}

void c_movement::auto_strafe( )
{
  if( !g_cfg.misc.auto_strafe )
    return;

  if( g_ctx.local->move_type( ) == movetype_ladder || g_ctx.local->move_type( ) == movetype_noclip )
    return;

  if( g_utils->on_ground( ) || g_exploits->stop_movement )
    return;

  if( g_ctx.local->velocity( ).length( true ) < 10.f )
    return;

  if( g_ctx.cmd->buttons & in_speed )
    return;

  bool holding_w = g_ctx.cmd->buttons & in_forward;
  bool holding_a = g_ctx.cmd->buttons & in_moveleft;
  bool holding_s = g_ctx.cmd->buttons & in_back;
  bool holding_d = g_ctx.cmd->buttons & in_moveright;

  bool m_pressing_move = holding_w || holding_a || holding_s || holding_d;

  static auto switch_key = 1.f;
  static auto circle_yaw = 0.f;
  static auto old_yaw = 0.f;

  auto velocity = g_ctx.local->velocity( );
  velocity.z = 0.f;

  auto speed = velocity.length( true );

  auto ideal_strafe = ( speed > 5.f ) ? math::rad_to_deg( std::asin( 15.f / speed ) ) : 90.f;
  ideal_strafe *= 1.f - ( g_cfg.misc.strafe_smooth * 0.01f );

  ideal_strafe = min( 90.f, ideal_strafe );

  switch_key *= -1.f;

  if( m_pressing_move )
  {
    float wish_dir{ };

    if( holding_w )
    {
      if( holding_a )
        wish_dir += ( strafe_left / 2 );
      else if( holding_d )
        wish_dir += ( strafe_right / 2 );
      else
        wish_dir += strafe_forwards;
    }
    else if( holding_s )
    {
      if( holding_a )
        wish_dir += strafe_back_left;
      else if( holding_d )
        wish_dir += strafe_back_right;
      else
        wish_dir += strafe_backwards;

      g_ctx.cmd->forwardmove = 0.f;
    }
    else if( holding_a )
      wish_dir += strafe_left;
    else if( holding_d )
      wish_dir += strafe_right;

    g_ctx.base_angle.y += math::normalize( wish_dir );
  }

  float smooth = ( 1.f - ( 0.15f * ( 1.f - g_cfg.misc.strafe_smooth * 0.01f ) ) );

  if( speed <= 0.5f )
  {
    g_ctx.cmd->forwardmove = 450.f;
    return;
  }

  const auto diff = math::normalize( g_ctx.base_angle.y - math::rad_to_deg( std::atan2f( velocity.y, velocity.x ) ) );

  g_ctx.cmd->forwardmove = std::clamp( ( 5850.f / speed ), -450.f, 450.f );
  g_ctx.cmd->sidemove = ( diff > 0.f ) ? -450.f : 450.f;

  g_ctx.base_angle.y = math::normalize( g_ctx.base_angle.y - diff * smooth );
}

void c_movement::auto_jump( )
{
  if( !g_cfg.misc.auto_jump )
    return;

  static bool last_jump = false;
  static bool fake_jump = false;

  if( !last_jump && fake_jump )
  {
    fake_jump = false;
    g_ctx.cmd->buttons |= in_jump;
  }
  else if( g_ctx.cmd->buttons & in_jump )
  {
    if( g_ctx.local->flags( ) & fl_onground )
      fake_jump = last_jump = true;
    else
    {
      g_ctx.cmd->buttons &= ~in_jump;
      last_jump = false;
    }
  }
  else
    fake_jump = last_jump = false;
}

void c_movement::force_speed( float max_speed )
{
  // as did in game movement of CSGO:
  // reference: CCSGameMovement::CheckParameters

  if( g_ctx.local->move_type( ) == movetype_noclip || g_ctx.local->move_type( ) == movetype_isometric || g_ctx.local->move_type( ) == movetype_observer )
    return;

  float sidemove = g_ctx.cmd->sidemove;
  float forwardmove = g_ctx.cmd->forwardmove;
  float upmove = g_ctx.cmd->upmove;

  float move_speed = std::sqrt( std::pow( sidemove, 2 ) + std::pow( forwardmove, 2 ) + std::pow( upmove, 2 ) );

  if( move_speed <= max_speed )
    return;

  float kys = max_speed / move_speed;

  g_ctx.cmd->forwardmove *= kys;
  g_ctx.cmd->sidemove *= kys;
  g_ctx.cmd->upmove *= kys;
}

float c_movement::get_max_speed( )
{
  return max_speed;
}

void c_movement::fix_movement( c_usercmd* cmd, vector3d& ang )
{
  vector3d move( cmd->forwardmove, cmd->sidemove, cmd->upmove );
  float speed = sqrt( move.x * move.x + move.y * move.y ), yaw;

  vector3d move_vec;
  math::vector_to_angles( move, move_vec );

  move_vec = math::normalize( move_vec, true );

  yaw = math::deg_to_rad( cmd->viewangles.y - ang.y + move_vec.y );
  cmd->forwardmove = cos( yaw ) * speed;
  cmd->sidemove = sin( yaw ) * speed;

  if( 90.f < abs( cmd->viewangles.x ) && 180.f > abs( cmd->viewangles.x ) )
    cmd->forwardmove *= -1.f;

  auto roll_move_angle = [ & ]( float base_angle )
  {
    float angle = std::cos( math::deg_to_rad( base_angle ) ) * -cmd->forwardmove + ( std::sin( math::deg_to_rad( base_angle ) ) ) * cmd->sidemove;
    return angle;
  };

  if( cmd->viewangles.z != 0.f )
  {
    float negative = roll_move_angle( 180.f - cmd->viewangles.z );
    float positive = roll_move_angle( cmd->viewangles.z + 180.f );

    float step = cmd->viewangles.x / 89.f;
    cmd->forwardmove = std::lerp( cmd->forwardmove, step <= 0.f ? positive : negative, step );
  }

  if( !g_cfg.misc.slide_walk )
    cmd->buttons &= ~( in_forward | in_back | in_moveright | in_moveleft );

  cmd->forwardmove = std::clamp( cmd->forwardmove, -450.f, 450.f );
  cmd->sidemove = std::clamp( cmd->sidemove, -450.f, 450.f );
  cmd->upmove = std::clamp( cmd->upmove, -320.f, 320.f );
}

void test( )
{
  if( g_cfg.binds [ ap_b ].toggled || g_cfg.binds [ sw_b ].toggled || g_rage_bot->stopping || !g_utils->on_ground( ) )
    return;

  static int old_buttons = 0;

  if( !( g_ctx.cmd->buttons & in_jump ) )
  {
    int unk = g_ctx.cmd->buttons & ( in_moveright | in_moveleft | in_back | in_forward ) & ( g_ctx.cmd->buttons & ( in_moveright | in_moveleft | in_back | in_forward ) ^ ( int )g_cfg.rage.enable );

    int cur_buttons = old_buttons;

    if( ( unk & in_moveleft ) != 0 )
    {
      cur_buttons = cur_buttons & -( in_moveright | in_moveleft | in_attack ) | in_moveright;
    }
    else if( ( g_ctx.cmd->buttons & in_moveleft ) == 0 )
    {
      cur_buttons &= ~in_moveright;
    }
    if( ( unk & in_moveright ) != 0 )
    {
      cur_buttons = cur_buttons & ~( in_moveright | in_moveleft ) | in_moveleft;
    }
    else if( ( g_ctx.cmd->buttons & in_moveright ) == 0 )
    {
      cur_buttons &= ~in_moveleft;
    }
    if( ( unk & in_forward ) != 0 )
    {
      cur_buttons = cur_buttons & -( in_back | in_forward | in_attack ) | in_back;
    }
    else if( ( g_ctx.cmd->buttons & in_forward ) == 0 )
    {
      cur_buttons &= -( in_back | in_attack );
    }
    if( ( unk & in_back ) != 0 )
    {
      cur_buttons = cur_buttons & -( in_back | in_forward | in_attack ) | in_forward;
    }
    else if( ( g_ctx.cmd->buttons & in_back ) == 0 )
    {
      cur_buttons &= ~in_forward;
    }

    old_buttons = cur_buttons;
    g_ctx.cmd->buttons &= ~old_buttons;

    auto new_buttons = g_ctx.cmd->buttons;

    if( ( new_buttons & ( in_moveright | in_moveleft ) ) != ( in_moveright | in_moveleft ) )
    {
      if( ( new_buttons & 0x200 ) != 0 )
      {
        g_ctx.cmd->sidemove = -450.f;
      }
      else if( ( new_buttons & 0x400 ) != 0 )
      {
        g_ctx.cmd->sidemove = 450.f;
      }
    }
    if( ( new_buttons & 0x18 ) != 24 )
    {
      if( ( new_buttons & 8 ) != 0 )
      {
        g_ctx.cmd->forwardmove = 450.f;
      }
      else if( ( new_buttons & 0x10 ) != 0 )
      {
        g_ctx.cmd->forwardmove = -450.f;
      }
    }
  }
  else
    old_buttons = 0;
}

void c_movement::on_pre_predict( )
{
  if( !g_ctx.in_game )
    return;

  if( !g_ctx.local->is_alive( ) || !g_ctx.weapon_info )
    return;

  if( g_ctx.local->move_type( ) == movetype_ladder || g_ctx.local->move_type( ) == movetype_noclip )
    return;

  if( interfaces::game_rules->is_freeze_time( ) || g_ctx.local->flags( ) & fl_frozen )
    return;

  max_speed = g_ctx.local->is_scoped( ) ? g_ctx.weapon_info->max_speed_alt : g_ctx.weapon_info->max_speed;

  this->jitter_move( );
  test( );

  this->auto_jump( );
  // this->auto_strafe( );
}

void c_movement::on_predict_start( )
{
  if( !g_ctx.weapon )
    return;

  if( g_ctx.local->move_type( ) == movetype_ladder || g_ctx.local->move_type( ) == movetype_noclip )
    return;

  if( interfaces::game_rules->is_freeze_time( ) || g_ctx.local->flags( ) & fl_frozen )
    return;

  this->auto_peek( );
  this->fast_stop( );
}