#pragma once

class c_usercmd;
class vector3d;
class c_csplayer;
class c_game_event;

class c_utils
{
private:
  int ground_ticks{ };

public:
  bool revolver_fire{ };
  bool start_buybot{ };
  bool hold_fire_animation{ };

  void update_ground_ticks( );
  void update_shot_cmd( );
  void update_shot_time( );
  void update_viewangles( );

  void extrapolate( c_csplayer* player, vector3d& origin, vector3d& velocity, int& flags, bool on_ground );

  bool on_ground( );
  bool chat_opened( );

  void clantag( );
  void buybot( );

  void update_mouse_delta( );
  void auto_revolver( );

  void auto_pistol( );
  bool is_able_to_shoot( bool revolver = false );
  bool is_firing( );

  void update_viewmodel_sequence( c_usercmd* cmd, bool restore );

  void on_postdata_update_start( int stage );
  void on_net_update_end_after( int stage );
  void on_game_events( c_game_event* event );
  void on_pre_predict( );
  void on_predict_start( );
  void on_predict_end( );
  void on_changed_map( );
};