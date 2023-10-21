#pragma once
#include "../tools/math.h"
#include "../tools/netvar_parser.h"
#include "activity.h"

class c_animation_layers;
class c_csplayer;

enum animstate_pose_param_idx_t
{
  player_pose_param_first = 0,
  player_pose_param_lean_yaw = player_pose_param_first,
  player_pose_param_speed,
  player_pose_param_ladder_speed,
  player_pose_param_ladder_yaw,
  player_pose_param_move_yaw,
  player_pose_param_run,
  player_pose_param_body_yaw,
  player_pose_param_body_pitch,
  player_pose_param_death_yaw,
  player_pose_param_stand,
  player_pose_param_jump_fall,
  player_pose_param_aim_blend_stand_idle,
  player_pose_param_aim_blend_crouch_idle,
  player_pose_param_strafe_dir,
  player_pose_param_aim_blend_stand_walk,
  player_pose_param_aim_blend_stand_run,
  player_pose_param_aim_blend_crouch_walk,
  player_pose_param_move_blend_walk,
  player_pose_param_move_blend_run,
  player_pose_param_move_blend_crouch_walk,
  player_pose_param_count,
};

struct animstate_pose_param_cache_t
{
  bool initialized{ };
  int index{ };
  const char* name{ };

  animstate_pose_param_cache_t( )
  {
    initialized = false;
    index = -1;
    name = "";
  }
};

class c_animstate
{
public:
  c_animstate( ){ };
  c_animstate( const c_animstate& animstate )
  {
    std::memcpy( this, &animstate, sizeof( c_animstate ) );
  };

  const char* get_weapon_prefix( void );
  void release( void )
  {
    delete this;
  }

  padding( 0x1C );

  void* outer;

  padding( 0x40 );

  void* player = nullptr;
  void* weapon = nullptr;
  void* weapon_last = nullptr;

  float last_update_time{ };
  int last_update_frame{ };
  float last_update_increment{ };

  float eye_yaw{ };
  float eye_pitch{ };
  float abs_yaw{ };
  float abs_yaw_last{ };
  float move_yaw{ };
  float move_yaw_ideal{ };
  float move_yaw_current_to_ideal{ };

  padding( 4 );

  float primary_cycle{ };
  float move_weight{ };

  float move_weight_smoothed{ };
  float anim_duck_amount{ };
  float duck_additional{ };
  float recrouch_weight{ };
  ;

  vector3d position_current{ };
  vector3d position_last{ };

  vector3d velocity{ };
  vector3d velocity_normalized{ };
  vector3d velocity_normalized_non_zero{ };

  float velocity_length_xy{ };
  float velocity_length_z{ };

  float speed_as_portion_of_run_top_speed{ };
  float speed_as_portion_of_walk_top_speed{ };
  float speed_as_portion_of_crouch_top_speed{ };

  float duration_moving{ };
  float duration_still{ };

  bool on_ground{ };
  bool landing{ };

  float jump_to_fall{ };
  float duration_in_air{ };
  float left_ground_height{ };
  float land_anim_multiplier{ };
  float walk_run_transition{ };

  bool landed_on_ground_this_frame{ };
  bool left_the_ground_this_frame{ };

  float in_air_smooth_value{ };

  bool on_ladder{ };
  float ladder_weight{ };
  float ladder_speed{ };
  ;

  bool walk_to_run_transition_state{ };

  bool defuse_started{ };
  bool plant_anim_started{ };
  bool twitch_anim_started{ };
  ;
  bool adjust_started{ };

  char activity_modifiers_server [ 20 ]{ };

  float next_twitch_time{ };
  float time_of_last_known_injury{ };
  float last_velocity_test_time{ };

  vector3d velocity_last{ };
  vector3d target_acceleration{ };
  vector3d acceleration{ };

  float acceleration_weight{ };

  float aim_matrix_transition{ };
  float aim_matrix_transition_delay{ };

  bool flashed{ };

  float strafe_change_weight{ };
  float strafe_change_target_weight{ };
  float strafe_change_cycle{ };

  int strafe_sequence{ };

  bool strafe_changing{ };

  float duration_strafing{ };
  float foot_lerp{ };

  bool feet_crossed{ };
  bool player_is_accelerating{ };

  animstate_pose_param_cache_t pose_param_mappings [ player_pose_param_count ]{ };

  float duration_move_weight_is_too_high{ };
  float static_approach_speed{ };

  int previous_move_state{ };
  float stutter_step{ };

  float action_weight_bias_remainder{ };

  padding( 112 );

  float camera_smooth_height{ };
  bool smooth_height_valid{ };
  float last_time_velocity_over_ten{ };

  float aim_yaw_min{ };
  float aim_yaw_max{ };
  float aim_pitch_min{ };
  float aim_pitch_max{ };

  int animstate_model_version{ };

  void create( c_csplayer* player );
  void update( const vector3d& angle );
  void reset( );

  float get_min_rotation( );
  float get_max_rotation( );

  void increment_layer_cycle( c_animation_layers* layer, bool loop );
  bool is_layer_sequence_finished( c_animation_layers* layer, float time );
  void set_layer_cycle( c_animation_layers* layer, float_t cycle );
  void set_layer_rate( c_animation_layers* layer, float rate );
  void set_layer_weight( c_animation_layers* layer, float weight );
  void set_layer_weight_rate( c_animation_layers* layer, float prev );
  void set_layer_sequence( c_animation_layers* layer, int sequence );
  int select_sequence_from_activity_modifier( int iActivity );
};