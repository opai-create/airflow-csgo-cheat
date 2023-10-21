#pragma once
#include "../tools/math.h"
#include "../tools/memory/displacement.h"

class c_animstate;
class c_studiohdr;
class c_ikcontext;
class c_csplayer;
class c_baseentity;
class c_basecombatweapon;
class c_usercmd;

struct mstudio_pose_param_desc_t;
struct bone_setup_t;

namespace func_ptrs
{
  using get_name_fn = const char*( __thiscall* )( void* );
  extern get_name_fn get_name;

  using init_key_values_fn = void( __thiscall* )( void*, const char* );
  extern init_key_values_fn init_key_values;

  using load_from_buffer_fn = void( __thiscall* )( void*, const char*, const char*, void*, const char*, void* );
  extern load_from_buffer_fn load_from_buffer;

  using weapon_shootpos_fn = float*( __thiscall* )( void*, vector3d* );
  extern weapon_shootpos_fn weapon_shootpos;

  using show_and_update_selection_fn = void( __thiscall* )( void*, int, c_basecombatweapon*, bool );
  extern show_and_update_selection_fn show_and_update_selection;

  using calc_shotgun_spread_fn = void( __stdcall* )( short, int, int, float*, float* );
  extern calc_shotgun_spread_fn calc_shotgun_spread;

  using load_named_sky_fn = void( __fastcall* )( const char* );
  extern load_named_sky_fn load_named_sky;

  using physics_run_think_fn = bool( __thiscall* )( void*, int );
  extern physics_run_think_fn physics_run_think;

  using think_fn = void( __thiscall* )( void*, int );
  extern think_fn think;

  using create_animstate_fn = void( __thiscall* )( c_animstate*, c_csplayer* );
  extern create_animstate_fn create_animstate;

  using reset_animstate_fn = void( __thiscall* )( c_animstate* );
  extern reset_animstate_fn reset_animstate;

  using update_animstate_fn = void( __vectorcall* )( c_animstate*, void*, float, float, float, void* );
  extern update_animstate_fn update_animstate;

  using set_abs_angles_fn = void( __thiscall* )( void*, const vector3d& );
  extern set_abs_angles_fn set_abs_angles;

  using set_abs_origin_fn = void( __thiscall* )( void*, const vector3d& );
  extern set_abs_origin_fn set_abs_origin;

  using get_pose_parameter_fn = mstudio_pose_param_desc_t*( __thiscall* )( c_studiohdr*, int );
  extern get_pose_parameter_fn get_pose_parameter;

  using update_merge_cache_fn = void( __thiscall* )( uintptr_t );
  extern update_merge_cache_fn update_merge_cache;

  using add_dependencies_fn = void( __thiscall* )( c_ikcontext*, float, int, int, const float [], float );
  extern add_dependencies_fn add_dependencies;

  using attachments_helper_fn = void( __thiscall* )( c_csplayer*, c_studiohdr* );
  extern attachments_helper_fn attachments_helper;

  using copy_to_follow_fn = void( __thiscall* )( uintptr_t, vector3d*, quaternion*, int, vector3d*, quaternion* );
  extern copy_to_follow_fn copy_to_follow;

  using copy_from_follow_fn = void( __thiscall* )( uintptr_t, vector3d*, quaternion*, int, vector3d*, quaternion* );
  extern copy_from_follow_fn copy_from_follow;

  using accumulate_pose_fn = void( __thiscall* )( bone_setup_t*, vector3d*, quaternion*, int, float, float, float, c_ikcontext* );
  extern accumulate_pose_fn accumulate_pose;

  using invalidate_physics_recursive_fn = void( __thiscall* )( void*, int );
  extern invalidate_physics_recursive_fn invalidate_physics_recursive;

  using lookup_bone_fn = int( __thiscall* )( void*, const char* );
  extern lookup_bone_fn lookup_bone;

  using write_user_cmd_fn = void( __fastcall* )( void*, c_usercmd*, c_usercmd* );
  extern write_user_cmd_fn write_user_cmd;

  using post_think_physics_fn = bool( __thiscall* )( c_csplayer* );
  extern post_think_physics_fn post_think_physics;

  using simulate_player_simulated_entities_fn = void( __thiscall* )( c_csplayer* );
  extern simulate_player_simulated_entities_fn simulate_player_simulated_entities;

  using set_collision_bounds_fn = void( __thiscall* )( void*, vector3d*, vector3d* );
  extern set_collision_bounds_fn set_collision_bounds;

  using find_hud_element_fn = DWORD( __thiscall* )( void*, const char* );
  extern find_hud_element_fn find_hud_element;

  using compute_hitbox_surrounding_box_fn = bool( __thiscall* )( void*, vector3d*, vector3d* );
  extern compute_hitbox_surrounding_box_fn compute_hitbox_surrounding_box;

  using is_breakable_entity_fn = bool( __thiscall* )( void* );
  extern is_breakable_entity_fn is_breakable_entity;

  using update_all_viewmodel_addons_fn = int( __fastcall* )( void* );
  extern update_all_viewmodel_addons_fn update_all_viewmodel_addons;

  using get_viewmodel_fn = void*( __thiscall* )( void*, int );
  extern get_viewmodel_fn get_viewmodel;

  using calc_absolute_position_fn = void( __thiscall* )( void* );
  extern calc_absolute_position_fn calc_absolute_position;

  extern bool* override_processing;
  extern int smoke_count;

  extern void init( );
}