#pragma once
#include <array>
#include <string>
#include <mutex>
#include <Windows.h>
#include <corecrt_math_defines.h>

#include "tools/math.h"
#include "hooks/hooks.h"

class c_csplayer;
class c_basecombatweapon;

class c_base_client_dll;
class c_global_vars_base;
class c_surface;
class c_panel;
class c_engine;
class c_entity_list;
class c_client_mode;
class c_debug_overlay;
class c_input_system;
class c_input;
class c_engine_trace;
class c_model_render;
class c_material_system;
class c_studio_render;
class c_localize;
class c_convar;
class c_prediction;
class c_movehelper;
class c_game_movement;
class c_game_event_manager2;
class c_view_render_beams;
class c_player_resource;
class c_memory_allocate;
class c_clientstate;
class c_model_info;
class c_phys_surface_props;
class c_view_render;
class c_glow_object_manager;
class c_engine_sound;
class c_mdl_cache;
class c_render_view;
class c_game_rules;
class c_cvar;
class c_key_values_system;
class c_network_string_table_container;
class c_item_schema;
class c_static_prop_manager;

class c_usercmd;

class vector3d;

struct matrix3x4_t;
struct weapon_info_t;

namespace interfaces
{
  extern c_base_client_dll* client;
  extern c_global_vars_base* global_vars;
  extern c_surface* surface;
  extern c_panel* panel;
  extern c_engine* engine;
  extern c_entity_list* entity_list;
  extern c_debug_overlay* debug_overlay;
  extern c_input_system* input_system;
  extern c_input* input;
  extern c_engine_trace* engine_trace;
  extern c_model_render* model_render;
  extern c_material_system* material_system;
  extern c_studio_render* studio_render;
  extern c_localize* localize;
  extern c_prediction* prediction;
  extern c_convar* convar;
  extern c_client_mode* client_mode;
  extern c_movehelper* move_helper;
  extern c_game_movement* game_movement;
  extern c_game_event_manager2* game_event_manager;
  extern c_view_render_beams* beam;
  extern c_player_resource* player_resource;
  extern c_memory_allocate* memory;
  extern c_clientstate* client_state;
  extern c_model_info* model_info;
  extern c_phys_surface_props* phys_surface_props;
  extern c_view_render* view_render;
  extern c_glow_object_manager* glow_object_manager;
  extern c_engine_sound* engine_sound;
  extern c_mdl_cache* model_cache;
  extern c_render_view* render_view;
  extern c_game_rules* game_rules;
  extern c_key_values_system* key_values_system;
  extern c_network_string_table_container* network_string_table_container;
  extern c_item_schema* item_schema;
  extern c_static_prop_manager* static_prop_manager;

  extern void init( );
  extern void init_dynamic_interfaces( );
}

namespace cvars
{
  extern c_cvar* weapon_debug_spread_show;
  extern c_cvar* cl_foot_contact_shadows;
  extern c_cvar* cl_brushfastpath;
  extern c_cvar* cl_csm_shadows;
  extern c_cvar* sv_cheats;
  extern c_cvar* sv_skyname;
  extern c_cvar* viewmodel_fov;
  extern c_cvar* weapon_accuracy_nospread;
  extern c_cvar* weapon_recoil_scale;
  extern c_cvar* cl_lagcompensation;
  extern c_cvar* cl_sidespeed;
  extern c_cvar* m_pitch;
  extern c_cvar* m_yaw;
  extern c_cvar* sensitivity;
  extern c_cvar* cl_updaterate;
  extern c_cvar* cl_interp;
  extern c_cvar* cl_interp_ratio;
  extern c_cvar* zoom_sensitivity_ratio_mouse;
  extern c_cvar* mat_fullbright;
  extern c_cvar* mat_postprocess_enable;
  extern c_cvar* fog_override;
  extern c_cvar* fog_start;
  extern c_cvar* fog_end;
  extern c_cvar* fog_maxdensity;
  extern c_cvar* fog_color;
  extern c_cvar* sv_gravity;
  extern c_cvar* sv_maxunlag;
  extern c_cvar* cl_csm_rot_override;
  extern c_cvar* cl_csm_rot_x;
  extern c_cvar* cl_csm_rot_y;
  extern c_cvar* cl_csm_rot_z;
  extern c_cvar* sv_footsteps;
  extern c_cvar* cl_clock_correction;
  extern c_cvar* sv_friction;
  extern c_cvar* sv_stopspeed;
  extern c_cvar* sv_accelerate;
  extern c_cvar* sv_jump_impulse;
  extern c_cvar* weapon_molotov_maxdetonateslope;
  extern c_cvar* molotov_throw_detonate_time;
  extern c_cvar* mp_damage_scale_ct_head;
  extern c_cvar* mp_damage_scale_ct_body;
  extern c_cvar* mp_damage_scale_t_head;
  extern c_cvar* mp_damage_scale_t_body;
  extern c_cvar* ff_damage_reduction_bullets;
  extern c_cvar* ff_damage_bullet_penetration;
  extern c_cvar* net_showfragments;
  extern c_cvar* cl_wpn_sway_interp;
  extern c_cvar* weapon_accuracy_shotgun_spread_patterns;
  extern c_cvar* cl_forwardspeed;
  extern c_cvar* sv_minupdaterate;
  extern c_cvar* sv_maxupdaterate;
  extern c_cvar* sv_client_min_interp_ratio;
  extern c_cvar* sv_client_max_interp_ratio;
  extern c_cvar* cl_interpolate;
  extern c_cvar* r_DrawSpecificStaticProp;

  extern void init( );
}

namespace mutexes
{
  inline std::mutex players{ };
  inline std::mutex weapons{ };
  inline std::mutex local{ };
  inline std::mutex bomb{ };
  inline std::mutex warning{ };
  inline std::mutex rage{ };
  inline std::mutex animfix{ };
}

class c_global_context
{
public:
  bool uninject{ };
  bool render_init{ };
  bool cheat_init{ };
  bool cheat_init2{ };

  bool in_game{ };
  bool is_alive{ };
  bool open_console{ };
  bool scoped{ };
  bool is_sniper{ };
  bool is_nade{ };
  bool is_misc{ };
  bool can_penetrate{ };
  bool loading_config{ };

  HWND window{ };
  WNDPROC backup_window{ };

  c_usercmd* cmd{ };
  bool* send_packet{ };

  vector3d orig_angle{ };
  vector3d base_angle{ };
  vector3d fake_angle{ };
  vector3d cur_angle{ };
  vector3d last_shoot_position{ };
  vector3d abs_origin{ };
  vector3d eye_position{ };

  int tick_rate{ };
  int tick_base{ };
  int shot_cmd{ };
  int fsn_stage{ };
  int sent_tick_count{ };
  int max_choke = 16;

  std::string sky_name{ };
  std::string username{ };

  bool round_start{ };

  bool valve_ds{ };
  bool shooting{ };
  bool cycle_changed{ };
  bool fix_cycle{ };
  bool lagcomp{ };
  bool modify_eye_pos{ };

  float velocity_modifier{ };

  float animation_speed{ };
  float last_time_updated{ };
  float lerp_time{ };
  float ping{ };
  float predicted_curtime{ };
  float real_ping{ };

  float forwardmove{ };
  float sidemove{ };
  float ideal_spread{ };
  float spread{ };
  float current_fov{ };
  float max_unlag = 1.f;

  float alpha_amt = 1.f;

  int health{ };

  std::array< bool, 65 > setup_bones{ };
  std::array< bool, 65 > update{ };

  std::string avatar{ };
  std::string session_key{ };

  c_csplayer* local{ };
  c_basecombatweapon* weapon{ };
  weapon_info_t* weapon_info{ };

  float system_time( )
  {
    return ( float )( clock( ) / ( float )1000.f );
  }

  void update_animations( );
  void update_local_player( );
};

inline c_global_context g_ctx{ };