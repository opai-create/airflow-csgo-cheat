#include <algorithm>

#include "../tools/memory/memory.h"

#include "../other/game_functions.h"

#include "c_animstate.h"
#include "entity.h"

void c_animstate::create( c_csplayer* player )
{
  func_ptrs::create_animstate( this, player );
}

void c_animstate::update( const vector3d& angle )
{
  func_ptrs::update_animstate( this, NULL, NULL, angle.y, angle.x, NULL );
}

void c_animstate::reset( )
{
  func_ptrs::reset_animstate( this );
}

float c_animstate::get_min_rotation( )
{
  float speed_walk = std::max( 0.f, std::min( this->speed_as_portion_of_walk_top_speed, 1.f ) );
  float speed_duck = std::max( 0.f, std::min( this->speed_as_portion_of_crouch_top_speed, 1.f ) );

  float modifier = ( ( this->walk_run_transition * -0.30000001f ) - 0.19999999f ) * speed_walk + 1.f;

  if( this->anim_duck_amount > 0.0f )
    modifier += ( ( this->anim_duck_amount * speed_duck ) * ( 0.5f - modifier ) );

  return this->aim_yaw_min * modifier;
}

float c_animstate::get_max_rotation( )
{
  float speed_walk = std::max( 0.f, std::min( this->speed_as_portion_of_walk_top_speed, 1.f ) );
  float speed_duck = std::max( 0.f, std::min( this->speed_as_portion_of_crouch_top_speed, 1.f ) );

  float modifier = ( ( this->walk_run_transition * -0.30000001f ) - 0.19999999f ) * speed_walk + 1.f;

  if( this->anim_duck_amount > 0.0f )
    modifier += ( ( this->anim_duck_amount * speed_duck ) * ( 0.5f - modifier ) );

  return this->aim_yaw_max * modifier;
}

void c_animstate::increment_layer_cycle( c_animation_layers* layer, bool loop )
{
  float new_cycle = ( layer->playback_rate * this->last_update_increment ) + layer->cycle;
  if( !loop && new_cycle >= 1.0f )
    new_cycle = 0.999f;

  new_cycle -= ( int32_t )( new_cycle );
  if( new_cycle < 0.0f )
    new_cycle += 1.0f;

  if( new_cycle > 1.0f )
    new_cycle -= 1.0f;

  layer->cycle = new_cycle;
}

bool c_animstate::is_layer_sequence_finished( c_animation_layers* layer, float time )
{
  return ( layer->playback_rate * time ) + layer->cycle >= 1.0f;
}

void c_animstate::set_layer_cycle( c_animation_layers* layer, float_t cycle )
{
  if( layer )
    layer->cycle = cycle;
}

void c_animstate::set_layer_rate( c_animation_layers* layer, float rate )
{
  if( layer )
    layer->playback_rate = rate;
}

void c_animstate::set_layer_weight( c_animation_layers* layer, float weight )
{
  if( layer )
    layer->weight = weight;
}

void c_animstate::set_layer_weight_rate( c_animation_layers* layer, float prev )
{
  if( layer )
    layer->weight_delta_rate = ( layer->weight - prev ) / this->last_update_increment;
}

void c_animstate::set_layer_sequence( c_animation_layers* layer, int sequence )
{
  if( sequence <= 1 )
    return;

  layer->cycle = 0.0f;
  layer->weight = 0.0f;
  layer->sequence = sequence;
  layer->playback_rate = ( ( c_csplayer* )this->player )->get_layer_sequence_cycle_rate( layer, sequence );
}

int c_animstate::select_sequence_from_activity_modifier( int iActivity )
{
  bool ducking = this->anim_duck_amount > 0.55f;
  bool running = this->speed_as_portion_of_walk_top_speed > 0.25f;

  int current_sequence = 0;
  switch( iActivity )
  {
  case act_csgo_jump:
  {
    current_sequence = 15 + static_cast< int32_t >( running );
    if( ducking )
      current_sequence = 17 + static_cast< int32_t >( running );
  }
  break;

  case act_csgo_alive_loop:
  {
    current_sequence = 8;
    if( this->weapon_last != this->weapon )
      current_sequence = 9;
  }
  break;

  case act_csgo_idle_adjust_stoppedmoving:
  {
    current_sequence = 6;
  }
  break;

  case act_csgo_fall:
  {
    current_sequence = 14;
  }
  break;

  case act_csgo_idle_turn_balanceadjust:
  {
    current_sequence = 4;
  }
  break;

  case act_csgo_land_light:
  {
    current_sequence = 20;
    if( running )
      current_sequence = 22;

    if( ducking )
    {
      current_sequence = 21;
      if( running )
        current_sequence = 19;
    }
  }
  break;

  case act_csgo_land_heavy:
  {
    current_sequence = 23;
    if( running )
      current_sequence = 24;
  }
  break;

  case act_csgo_climb_ladder:
  {
    current_sequence = 13;
  }
  break;

  default:
    break;
  }

  return current_sequence;
}