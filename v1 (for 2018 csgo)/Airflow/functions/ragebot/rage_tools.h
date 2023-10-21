#pragma once
#include <array>
#include <vector>
#include <algorithm>

#include "../../base/sdk/entity.h"
#include "../../base/global_context.h"

#include "../ragebot/animfix.h"
#include "../config_vars.h"

struct point_t;

namespace cheat_tools
{
  inline bool debug_hitchance = false;
  inline vector2d spread_point{ };
  inline float current_spread{ };
  inline std::vector< vector2d > spread_points{ };

  int get_legit_tab( c_basecombatweapon* temp_weapon = nullptr );
  skin_weapon_t get_skin_weapon_config( );
  weapon_config_t get_weapon_config( );

  std::string hitbox_to_string( int id );
  std::string hitgroup_to_string( int hitgroup );
  int hitbox_to_hitgroup( int hitbox );

  bool can_hit_hitbox( const vector3d& start, const vector3d& end, c_csplayer* player, int hitbox, records_t* record, matrix3x4_t* matrix = nullptr );
  bool is_accuracy_valid( c_csplayer* player, point_t& point, float amount, float* out_chance );

  std::vector< std::pair< vector3d, bool > > get_multipoints( c_csplayer* player, int hitbox, matrix3x4_t* matrix );
}

struct resolver_info_t
{
  bool valid{ };

  // current resolver mode:
  std::string mode{ };

  float last_moving_lby{ };
  float last_stand_angle{ };

  struct
  {
    bool available = false;

    float left_fraction = 0.f;
    float right_fraction = 0.f;
    float yaw = 0.f;

    int left_damage = 0;
    int right_damage = 0;

    __forceinline void reset( )
    {
      available = false;

      left_fraction = right_fraction = 0.f;
      yaw = 0.f;
      left_damage = right_damage = 0;
    }
  } freestanding;

  struct
  {
    float lby_time{ };
    float old_lby{ };

    __forceinline void reset( )
    {
      lby_time = 0.f;
      old_lby = 0.f;
    }
  } lby;

  __forceinline void reset( )
  {
    valid = false;
    mode = "";

    last_moving_lby = 0.f;
    last_stand_angle = 0.f;

    freestanding.reset( );
  }
};

namespace resolver
{
  inline std::array< resolver_info_t, 65 > info{ };

  __forceinline void reset_info( c_csplayer* player )
  {
    auto& i = info [ player->index( ) ];
    i.reset( );
  }

  void start( c_csplayer* player, records_t* current );
  void on_fsn( );
}