#pragma once
#include "../../base/tools/math.h"

class c_usercmd;

class c_movement
{
private:
  float max_speed{ };

  void fast_stop( );
  void auto_jump( );
  void auto_peek( );

public:
  vector3d peek_pos{ };
  bool peek_start{ };
  bool peek_move{ };

  float jitter_move_speed{ };

  float get_max_speed( );
  void fix_movement( c_usercmd* cmd, vector3d& ang );

  void force_speed( float max_speed );
  void force_stop( );

  void on_pre_predict( );
  void auto_strafe( );
  void jitter_move( );
  void on_predict_start( );
};