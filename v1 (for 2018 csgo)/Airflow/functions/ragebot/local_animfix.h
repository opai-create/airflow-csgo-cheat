#pragma once
#include <memory>
#include <deque>
#include <unordered_map>

#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

#include "../../base/tools/math.h"
#include "../../base/tools/memory/displacement.h"

#include "../../base/other/game_functions.h"

#include "setup_bones_manager.h"

class c_local_animation_fix
{
private:
  c_animstate* state{ };
  bool reset{ };
  float old_spawn{ };
  uint32_t old_handle{ };

  void update_fake( );
  void update_strafe_state( );
  void update_viewmodel( );
  void rebuild_animations( c_animation_layers* layer );
public:
  struct local_data_t
  {
    bool on_ground{ };
    float last_lby_time{ };
    float last_lby_tick{ };
    float lby_angle{ };

    int real_type{ };
    int real_flags{ };

    c_bone_builder realbuild{ };
    c_bone_builder fakebuild{ };

    c_animation_layers real_layers [ 13 ]{ };

    alignas( 16 ) matrix3x4_t matrix [ 128 ]{ };
    alignas( 16 ) matrix3x4_t matrix_fake [ 128 ]{ };

    std::array< float, 24 > render_poses{ };
    c_animation_layers render_layers [ 13 ];

    __forceinline void reset( )
    {
      std::memset( matrix, 0, sizeof( matrix ) );
      std::memset( matrix_fake, 0, sizeof( matrix_fake ) );
      std::memset( render_layers, 0, sizeof( render_layers ) );
      std::memset( real_layers, 0, sizeof( real_layers ) );

      last_lby_time = 0.f;
      last_lby_tick = 0.f;
      lby_angle = 0.f;
      on_ground = false;

      real_type = 0;
      real_flags = 0;

      render_poses = { };
    }
  } local_info;

  void on_predict_end( );

  void on_render_start_after( int stage );

  void on_game_events( c_game_event* event );
  void on_local_death( );
};