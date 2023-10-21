#pragma once
#include "../../base/sdk.h"
#include "../../base/global_context.h"

class c_game_trace;
class ray_t;

struct anti_aim_angles_t;

class c_anti_aim
{
private:
  bool fake_ducking{ };

  bool flip_side{ };
  bool flip_jitter{ };
  bool flip_move{ };

  int fake_side{ };
  float fake_angle{ };
  float best_dist{ };

  float last_real_angle = -1;
  float old_speed = -1;
  float random_dist_speed = 0.f;

  int aa_shot_cmd{ };

  std::vector< int > hitbox_list = { hitbox_head, hitbox_chest, hitbox_stomach, hitbox_pelvis };

  void fake_duck( );
  void slow_walk( );
  void fake( );
  void manual_yaw( );

  void automatic_edge( );
  void at_targets( );

public:
  int get_ticks_to_stop( );

  anti_aim_angles_t* get_config( );

  c_csplayer* get_closest_player( bool skip = false, bool local_distance = false );
  bool is_peeking( );
  bool is_fake_ducking( );

  void on_pre_predict( );
  void on_predict_start( );
  void on_predict_end( );
};