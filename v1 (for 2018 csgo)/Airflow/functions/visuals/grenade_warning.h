#pragma once
#include "../../base/sdk/entity.h"
#include "../../base/sdk.h"

#include <unordered_map>
#include <vector>

class c_csplayer;
class vector3d;
class c_game_trace;
class c_auto_wall;
typedef c_game_trace trace_t;

// yeah... it's pasted
// but anyway it works good
// TO-DO: rework data stroring ang calculations
class c_grenade_warning
{
public:
  struct nade_path_t
  {
    __forceinline nade_path_t( ) = default;

    __forceinline nade_path_t( c_csplayer* owner, int index, const vector3d& origin, const vector3d& velocity, float throw_time, int offset ): nade_path_t( )
    {
      nade_owner = owner;
      nade_idx = index;

      this->predict_nade( origin, velocity, throw_time, offset );
    }

    __forceinline bool physics_simulate( )
    {
      if( is_detonated )
        return true;

      const auto new_velocity_z = nade_velocity.z - ( cvars::sv_gravity->get_float( ) * 0.4f ) * interfaces::global_vars->interval_per_tick;

      const auto move = vector3d(
        nade_velocity.x * interfaces::global_vars->interval_per_tick, nade_velocity.y * interfaces::global_vars->interval_per_tick, ( nade_velocity.z + new_velocity_z ) / 2.f * interfaces::global_vars->interval_per_tick );

      nade_velocity.z = new_velocity_z;

      auto trace = trace_t( );

      this->physics_push_entity( move, trace );

      if( is_detonated )
        return true;

      if( trace.fraction != 1.f )
      {
        this->update_path< true >( );

        this->perform_fly_collision_resolution( trace );
      }

      return false;
    }

    __forceinline void physics_trace_entity( const vector3d& src, const vector3d& dst, std::uint32_t mask, trace_t& trace )
    {
      interfaces::engine_trace->trace_hull( src, dst, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }, mask, nade_owner, cur_collision_group, &trace );

      if( trace.start_solid && ( trace.contents & contents_current_90 ) )
      {
        trace.clear( );

        interfaces::engine_trace->trace_hull( src, dst, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }, mask & ~contents_current_90, nade_owner, cur_collision_group, &trace );
      }

      if( !trace.did_hit( ) || !trace.entity || !( ( c_csplayer* )trace.entity )->is_player( ) )
        return;

      trace.clear( );

      interfaces::engine_trace->trace_line( src, dst, mask, nade_owner, cur_collision_group, &trace );
    }

    __forceinline void physics_push_entity( const vector3d& push, trace_t& trace )
    {
      this->physics_trace_entity( nade_origin, nade_origin + push,
        cur_collision_group == collision_group_debris ? ( mask_solid | contents_current_90 ) & ~contents_monster : mask_solid | contents_opaque | contents_ignore_nodraw_opaque | contents_current_90 | contents_hitbox, trace );

      if( trace.start_solid )
      {
        cur_collision_group = collision_group_interactive_debris;

        interfaces::engine_trace->trace_line( nade_origin - push, nade_origin + push, ( mask_solid | contents_current_90 ) & ~contents_monster, nade_owner, cur_collision_group, &trace );
      }

      if( trace.fraction )
      {
        nade_origin = trace.end;
      }

      if( !trace.entity )
        return;

      if( ( ( c_csplayer* )trace.entity )->is_player( ) || nade_idx != weapon_tagrenade && nade_idx != weapon_molotov && nade_idx != weapon_incgrenade )
        return;

      if( nade_idx != weapon_tagrenade && trace.plane.normal.z < std::cos( math::deg_to_rad( cvars::weapon_molotov_maxdetonateslope->get_float( ) ) ) )
        return;

      this->detonade< true >( );
    }

    void perform_fly_collision_resolution( trace_t& trace );

    __forceinline void think( )
    {
      switch( nade_idx )
      {
      case weapon_smokegrenade:
        if( nade_velocity.length_sqr( ) <= 0.01f )
        {
          this->detonade< false >( );
        }

        break;
      case weapon_decoy:
        if( nade_velocity.length_sqr( ) <= 0.04f )
        {
          this->detonade< false >( );
        }

        break;
      case weapon_flashbang:
      case weapon_hegrenade:
      case weapon_molotov:
      case weapon_incgrenade:
        if( math::ticks_to_time( nade_tick_count ) > nade_detonate_time )
        {
          this->detonade< false >( );
        }

        break;
      }

      next_think_tick = nade_tick_count + math::time_to_ticks( 0.2f );
    }

    template < bool _bounced >
    __forceinline void detonade( )
    {
      is_detonated = true;

      update_path< _bounced >( );
    }

    template < bool _bounced >
    __forceinline void update_path( )
    {
      last_update_tick = nade_tick_count;

      path.emplace_back( nade_origin, _bounced );
    }

    __forceinline void predict_nade( const vector3d& origin, const vector3d& velocity, float throw_time, int offset )
    {
      nade_origin = origin;
      nade_velocity = velocity;
      cur_collision_group = collision_group_projectile;

      const auto tick = math::time_to_ticks( 1.f / 30.f );

      last_update_tick = -tick;

      switch( nade_idx )
      {
      case weapon_smokegrenade:
        next_think_tick = math::time_to_ticks( 1.5f );
        break;
      case weapon_decoy:
        next_think_tick = math::time_to_ticks( 2.f );
        break;
      case weapon_flashbang:
      case weapon_hegrenade:
        nade_detonate_time = 1.5f;
        next_think_tick = math::time_to_ticks( 0.02f );

        break;
      case weapon_molotov:
      case weapon_incgrenade:
        nade_detonate_time = cvars::molotov_throw_detonate_time->get_float( );
        next_think_tick = math::time_to_ticks( 0.02f );

        break;
      }

      for( ; nade_tick_count < math::time_to_ticks( 60.f ); ++nade_tick_count )
      {
        if( next_think_tick <= nade_tick_count )
        {
          think( );
        }

        if( nade_tick_count < offset )
          continue;

        if( this->physics_simulate( ) )
          break;

        if( last_update_tick + tick > nade_tick_count )
          continue;

        this->update_path< false >( );
      }

      if( last_update_tick + tick <= nade_tick_count )
      {
        this->update_path< false >( );
      }

      nade_expire_time = throw_time + math::ticks_to_time( nade_tick_count );
    }

    bool should_draw( );

    bool is_detonated{ };
    c_csplayer* nade_owner{ };
    vector3d nade_origin{ }, nade_velocity{ };
    c_client_entity* last_hit_entity{ };
    collision_group_t cur_collision_group{ };
    float nade_detonate_time{ }, nade_expire_time{ };
    int nade_idx{ }, nade_tick_count{ }, next_think_tick{ }, last_update_tick{ }, bounces_count{ };

    std::vector< std::pair< vector3d, bool > > path{ };

    std::string preview_icon{ }, preview_name{ };
    ImVec2 last_path_pos{ };
  };

  std::unordered_map< unsigned long, float > nade_alpha{ };
  std::unordered_map< unsigned long, nade_path_t > list{ };

  nade_path_t local_path{ };

public:
  __forceinline c_grenade_warning( ) = default;

  __forceinline std::unordered_map< unsigned long, nade_path_t >& get_nade_list( )
  {
    return list;
  }

  __forceinline void erase_handle( unsigned long handle )
  {
    list.erase( handle );
  }

  void calc_nade_path( c_csplayer* entity );
  void calc_local_nade_path( );
  void draw_local_path( );
  void on_render_start( int stage );
  void on_paint_traverse( );
  void on_directx( );
};