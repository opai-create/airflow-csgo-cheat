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

#define backup_globals( name ) float prev_##name = interfaces::global_vars->##name
#define restore_globals( name ) interfaces::global_vars->##name = prev_##name

struct records_t
{
  struct simulated_data_t
  {
    c_bone_builder builder{ };
    matrix3x4_t bone [ 128 ]{ };
    c_animation_layers layers [ 13 ]{ };

    __forceinline void reset( )
    {
      std::memset( layers, 0, sizeof( layers ) );
      std::memset( bone, 0, sizeof( bone ) );
    }
  };

  bool valid{ };
  bool dormant{ };
  bool lby_update{ };
  c_csplayer* ptr{ };
  int flags{ };
  int eflags{ };
  int choke{ };
  int last_lerp_update{ };
  int lerp{ };
  float duck_amount{ };
  float lby{ };
  float thirdperson_recoil{ };
  float interp_time{ };
  float sim_time{ };
  float old_sim_time{ };
  float anim_time{ };
  float anim_speed{ };
  bool on_ground{ };
  bool real_on_ground{ };
  bool shooting{ };
  bool fake_walking{ };
  vector3d abs_origin{ };
  vector3d origin{ };
  vector3d velocity{ };
  vector3d anim_velocity{ };
  vector3d mins{ };
  vector3d maxs{ };
  vector3d eye_angles{ };
  vector3d abs_angles{ };
  simulated_data_t sim_orig{ };
  matrix3x4_t render_bones [ 128 ]{ };
  std::array< float, 24 > poses{ };

  __forceinline void update_record( c_csplayer* player, bool after_lagcomp = false )
  {
    valid = true;
    ptr = player;
    lby = player->lby( );
    thirdperson_recoil = player->thirdperson_recoil( );
    sim_time = player->simulation_time( );
    old_sim_time = player->old_simtime( );
    abs_origin = player->get_abs_origin( );
    origin = player->origin( );

    mins = player->bb_mins( );
    maxs = player->bb_maxs( );
    eye_angles = player->eye_angles( );
    abs_angles = player->get_abs_angles( );

    if( !after_lagcomp )
    {
      flags = player->flags( );
      eflags = player->e_flags( );
      duck_amount = player->duck_amount( );

      velocity = player->velocity( );

      float time_delta = std::max< float >( interfaces::global_vars->interval_per_tick, sim_time - old_sim_time );
      choke = std::clamp( math::time_to_ticks( time_delta ), 0, 64 );

      anim_speed = 0.f;
      fake_walking = false;
      lby_update = false;

      anim_time = interfaces::global_vars->cur_time;
    }

    if( !after_lagcomp )
      player->store_layer( sim_orig.layers );

    on_ground = flags & fl_onground;
  }

  __forceinline void update_shot( records_t* last )
  {
    auto weapon = ptr->get_active_weapon( );
    if( last && weapon )
    {
      float last_shot_time = weapon->last_shot_time( );
      shooting = sim_time >= last_shot_time && last_shot_time > last->sim_time;
    }
  }

  __forceinline void update_dormant( int dormant_ticks )
  {
    dormant = dormant_ticks < 1;
  }

  __forceinline void restore( c_csplayer* player )
  {
    player->velocity( ) = velocity;
    player->flags( ) = flags;
    player->duck_amount( ) = duck_amount;

    player->set_layer( sim_orig.layers );

    player->lby( ) = lby;
    player->thirdperson_recoil( ) = thirdperson_recoil;

    player->set_abs_origin( origin );
  }

  bool is_valid( );

  __forceinline void reset( )
  {
    valid = false;
    dormant = false;
    ptr = nullptr;
    flags = 0;
    eflags = 0;
    choke = 0;
    duck_amount = 0.f;
    lby = 0.f;
    thirdperson_recoil = 0.f;
    sim_time = 0.f;
    old_sim_time = 0.f;
    anim_time = 0.f;
    anim_speed = 0.f;
    interp_time = 0.f;

    on_ground = false;
    shooting = false;
    fake_walking = false;
    real_on_ground = false;
    lby_update = false;

    origin.reset( );
    velocity.reset( );
    mins.reset( );
    maxs.reset( );
    abs_origin.reset( );
    eye_angles.reset( );
    abs_angles.reset( );
    anim_velocity.reset( );

    sim_orig.reset( );

    std::memset( render_bones, 0, sizeof( render_bones ) );
  }
};

struct restore_anims_t
{
  float feet_cycle{ };
  float feet_weight{ };

  __forceinline void store( c_csplayer* player )
  {
    auto state = player->animstate( );
    if( !state )
      return;

    feet_cycle = state->primary_cycle;
    feet_weight = state->move_weight;
  }

  __forceinline void restore( c_csplayer* player )
  {
    auto state = player->animstate( );
    if( !state )
      return;

    state->primary_cycle = feet_cycle;
    state->move_weight = feet_weight;
  }
};

class c_animation_fix
{
public:
  struct anim_player_t
  {
    bool teammate{ };
    int dormant_ticks{ };

    float old_spawn_time{ };
    float next_update_time{ };
    float old_aliveloop_cycle{ };

    c_csplayer* ptr{ };

    records_t backup_record{ };
    records_t* last_record{ };
    records_t* old_record{ };

    std::deque< records_t > records{ };

    __forceinline void reset_data( )
    {
      teammate = false;
      dormant_ticks = 0;

      old_spawn_time = 0.f;
      next_update_time = 0.f;
      old_aliveloop_cycle = 0.f;

      ptr = nullptr;
      last_record = nullptr;
      old_record = nullptr;

      records.clear( );
      backup_record.reset( );
    }

    // pasted from supremacy and onetap
    // too lazy to work at it
    __forceinline void update_land( records_t* record )
    {
      if( !last_record )
        return;

      bool on_ground = ptr->flags( ) & fl_onground;
      record->on_ground = false;
      record->real_on_ground = on_ground;

      if( on_ground && last_record->real_on_ground )
      {
        record->on_ground = true;
      }
      else
      {
        if( record->sim_orig.layers [ 4 ].weight != 1.f && record->sim_orig.layers [ 4 ].weight == 1.f && record->sim_orig.layers [ 5 ].weight != 0.f )
        {
          record->on_ground = true;
        }

        if( on_ground )
        {
          bool ground = record->on_ground;
          if( !last_record->real_on_ground )
            ground = false;
          record->on_ground = ground;
        }
      }
    }

    __forceinline void update_velocity( records_t* record )
    {
      if( !last_record )
        return;
      
      auto weapon = ptr->get_active_weapon( );
      if( !weapon )
        return;

      if( record->choke > 0 && record->choke < 16 && last_record && !last_record->dormant )
        record->velocity = ( record->origin - last_record->origin ) * ( 1.f / math::ticks_to_time( record->choke ) );

      record->anim_velocity = record->velocity;

      if( record->flags & fl_onground && record->velocity.length( false ) > 0.1f && record->sim_orig.layers [ 12 ].weight == 0.f && record->sim_orig.layers [ 6 ].weight < 0.1f )
        record->fake_walking = true;

      if( ptr->flags( ) & fl_onground && record->sim_orig.layers [ 11 ].weight > 0.f && record->sim_orig.layers [ 11 ].weight < 1.f && record->sim_orig.layers [ 11 ].cycle > last_record->sim_orig.layers [ 11 ].cycle )
      {
        if( weapon )
        {
          float max_speed = 260.f;
          auto weapon_info = weapon->get_weapon_info( );

          if( weapon_info )
            max_speed = ptr->is_scoped( ) ? weapon_info->max_speed_alt : weapon_info->max_speed;

          float modifier = 0.35f * ( 1.f - record->sim_orig.layers [ 11 ].weight );

          if( modifier > 0.f && modifier < 1.f )
            record->anim_speed = max_speed * ( modifier + 0.55f );
        }
      }

      if( record->anim_speed > 0.f )
      {
        record->anim_speed /= record->velocity.length( true );

        record->anim_velocity.x *= record->anim_speed;
        record->anim_velocity.y *= record->anim_speed;
      }

      if( record->fake_walking )
        record->anim_velocity = { 0.f, 0.f, 0.f };
    }

    __forceinline void force_update( )
    {
      backup_globals( cur_time );
      backup_globals( frame_time );

      interfaces::global_vars->cur_time = ptr->simulation_time( );
      interfaces::global_vars->frame_time = interfaces::global_vars->interval_per_tick;

      ptr->force_update( );

      restore_globals( cur_time );
      restore_globals( frame_time );
    }

    void simulate_animation_side( records_t* record );
    void build_bones( records_t* record, records_t::simulated_data_t* sim );
    void update_animations( );
  };

  std::array< anim_player_t, 65 > players{ };

  __forceinline anim_player_t* get_animation_player( int idx )
  {
    return &players [ idx ];
  }

  __forceinline float get_lerp_time( )
  {
    float total_lerp = 0.f;

    float update_rate_value = cvars::cl_updaterate->get_float( );
    if( !interfaces::engine->is_hltv( ) )
    {
      if( cvars::sv_minupdaterate && cvars::sv_maxupdaterate )
        update_rate_value = std::clamp< float >( update_rate_value, cvars::sv_minupdaterate->get_float( ), cvars::sv_maxupdaterate->get_float( ) );
    }

    bool use_interp = cvars::cl_interpolate->get_int( );
    if( use_interp )
    {
      float lerp_ratio = cvars::cl_interp_ratio->get_float( );
      if( lerp_ratio == 0.f )
        lerp_ratio = 1.f;

      float lerp_amount = cvars::cl_interp->get_float( );

      if( cvars::sv_client_min_interp_ratio && cvars::sv_client_max_interp_ratio && cvars::sv_client_min_interp_ratio->get_float( ) != -1 )
      {
        lerp_ratio = std::clamp< float >( lerp_ratio, cvars::sv_client_min_interp_ratio->get_float( ), cvars::sv_client_max_interp_ratio->get_float( ) );
      }
      else
      {
        if( lerp_ratio == 0.f )
          lerp_ratio = 1.f;
      }

      total_lerp = std::max< float >( lerp_amount, lerp_ratio / update_rate_value );
    }
    else
    {
      total_lerp = 0.0f;
    }

    return total_lerp;
  }

  __forceinline records_t* get_latest_record( c_csplayer* player )
  {
    const std::unique_lock< std::mutex > lock( mutexes::animfix );

    auto anim_player = this->get_animation_player( player->index( ) );
    if( !anim_player || anim_player->records.empty( ) )
      return nullptr;

    auto record = std::find_if( anim_player->records.begin( ), anim_player->records.end( ), [ & ]( records_t& record ) { return record.is_valid( ); } );

    if( record != anim_player->records.end( ) )
      return &*record;

    return nullptr;
  }

  __forceinline records_t* get_oldest_record( c_csplayer* player )
  {
    const std::unique_lock< std::mutex > lock( mutexes::animfix );

    auto anim_player = this->get_animation_player( player->index( ) );
    if( !anim_player || anim_player->records.empty( ) || !g_ctx.lagcomp )
      return nullptr;

    auto record = std::find_if( anim_player->records.rbegin( ), anim_player->records.rend( ), [ & ]( records_t& record ) { return record.is_valid( ); } );

    if( record != anim_player->records.rend( ) )
      return &*record;

    return nullptr;
  }

  __forceinline std::vector< records_t* > get_all_records( c_csplayer* player )
  {
    const std::unique_lock< std::mutex > lock( mutexes::animfix );

    auto anim_player = this->get_animation_player( player->index( ) );
    if( !anim_player || anim_player->records.empty( ) )
      return { };

    std::vector< records_t* > out{ };
    for( auto it = anim_player->records.begin( ); it != anim_player->records.end( ); it = next( it ) )
      if( ( it )->is_valid( ) )
        out.emplace_back( &*it );

    return out;
  }

  __forceinline std::pair< records_t*, records_t* > get_interp_record( c_csplayer* player )
  {
    const std::unique_lock< std::mutex > lock( mutexes::animfix );

    auto anim_player = this->get_animation_player( player->index( ) );
    if( !anim_player || anim_player->records.empty( ) || anim_player->records.size( ) <= 1 )
      return std::make_pair( nullptr, nullptr );

    auto it = anim_player->records.begin( ), prev = it;

    for( ; it != anim_player->records.end( ); it = next( it ) )
    {
      if( prev->is_valid( ) && !it->is_valid( ) )
      {
        return std::make_pair( &*prev, &*it );
      }
      prev = it;
    }

    return std::make_pair( nullptr, nullptr );
  }

  void on_net_update_and_render_after( int stage );
};