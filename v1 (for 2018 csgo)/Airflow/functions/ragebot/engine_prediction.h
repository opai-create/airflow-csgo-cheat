#pragma once
#include "../../base/sdk.h"

#include "../../base/tools/math.h"

#include "../../base/other/game_functions.h"

#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/entity.h"

#include <memory>
#include <optional>
#undef local

class c_engine_prediction
{
private:
  struct net_data_t
  {
    int cmd_number{ };

    float vel_modifier{ };
    float fall_velocity{ };
    float duck_amt{ };
    float duck_speed{ };
    float thirdperson_recoil{ };

    vector3d punch{ };
    vector3d punch_vel{ };
    vector3d view_offset{ };
    vector3d view_punch{ };
    vector3d velocity{ };

    bool filled{ };

    __forceinline void reset( )
    {
      cmd_number = 0;

      vel_modifier = 0.f;
      fall_velocity = 0.f;
      duck_amt = 0.f;
      duck_speed = 0.f;
      thirdperson_recoil = 0.f;

      punch.reset( );
      punch_vel.reset( );
      view_offset.reset( );
      view_punch.reset( );
      velocity.reset( );

      filled = false;
    }
  };

  struct netvars_t
  {
    bool done = false;

    float recoil_index;
    float acc_penalty;

    vector3d origin;
    vector3d abs_origin;
    vector3d viewoffset;
    vector3d aimpunch;
    vector3d aimpunch_vel;
    vector3d viewpunch;

    __forceinline void fill( )
    {
      recoil_index = g_ctx.weapon->recoil_index( );
      acc_penalty = g_ctx.weapon->accuracy_penalty( );

      origin = g_ctx.local->origin( );
      abs_origin = g_ctx.local->get_abs_origin( );
      viewoffset = g_ctx.local->view_offset( );
      aimpunch = g_ctx.local->aim_punch_angle( );
      aimpunch_vel = g_ctx.local->aim_punch_angle_vel( );
      viewpunch = g_ctx.local->view_punch_angle( );

      done = true;
    }

    __forceinline void set( )
    {
      if( !done )
        return;

      g_ctx.weapon->recoil_index( ) = recoil_index;
      g_ctx.weapon->accuracy_penalty( ) = acc_penalty;

      g_ctx.local->origin( ) = origin;
      g_ctx.local->set_abs_origin( abs_origin );
      g_ctx.local->view_offset( ) = viewoffset;
      g_ctx.local->aim_punch_angle( ) = aimpunch;
      g_ctx.local->aim_punch_angle_vel( ) = aimpunch_vel;
      g_ctx.local->view_punch_angle( ) = viewpunch;
    }
  };

  netvars_t unpred_vars [ 150 ];

  bool reset_net_data{ };
  bool old_in_prediction{ };
  bool old_first_time_predicted{ };

  int old_tick_base{ };
  int old_tick_count{ };

  float old_cur_time{ };
  float old_frame_time{ };

  float old_recoil_index{ };
  float old_accuracy_penalty{ };

  uint32_t old_seed{ };

  c_usercmd* old_cmd{ };

  int* prediction_player{ };
  int* prediction_random_seed{ };

  c_movedata move_data{ };

  std::array< net_data_t, 150 > net_data{ };

  __forceinline void reset( )
  {
    if( !reset_net_data )
      return;

    for( auto& d : net_data )
      d.reset( );

    reset_net_data = false;
  }

public:
  vector3d unprediced_velocity{ };
  int unpredicted_flags{ };
  int predicted_buttons{ };

  float predicted_inaccuracy{ };
  float predicted_spread{ };

  float interp_amount{ };

  void on_render_start( int stage, bool after );

  void net_compress_store( int tick );
  void net_compress_apply( int tick );

  void init( );
  void update( );

  void start( c_csplayer* local, c_usercmd* cmd );
  void force_update_eyepos( const float& pitch );
  void repredict( c_csplayer* local, c_usercmd* cmd, bool real_cmd = false );
  void finish( c_csplayer* local );
};