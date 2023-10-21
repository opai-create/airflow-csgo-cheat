#pragma once
#include "../../includes.h"
#include "../../base/tools/math.h"

#include "../interfaces/client.h"

#include "wrapper.h"
#include <unordered_map>

class c_view_setup;
class c_usercmd;
class c_studiohdr;
class c_studio_render;
class vector3d;
class vector4d;
class c_panel;
class c_material;
class c_movehelper;
class c_csplayer;
class c_collideable;
class c_game_trace;
class i_trace_filter;
class c_animstate;
class c_netchan;
class c_net_message;
class i_net_channel_info;
class c_movedata;
class c_key_values_system;
class c_glow_object_manager;
class c_renderable;
class c_clientstate;
class c_cvar;
class c_baseentity;

struct draw_model_state_t;
struct model_render_info_t;
struct draw_model_info_t;
struct matrix3x4_t;
struct studio_model_array_info2_t;
struct studio_array_data_t;
struct model_list_by_type_t;
struct ray_t;

#pragma region vmt_function_pointers
// client
using level_init_pre_entity_fn = void( __thiscall* )( void*, const char* );
using level_init_post_entity_fn = void( __thiscall* )( void* );
using write_usercmd_fn = bool( __thiscall* )( void*, int, void*, int, int, bool );
using create_move_client_fn = void( __fastcall* )( void*, void*, int, float, bool );

// view render
using on_render_start_fn = void( __thiscall* )( void* );
using render_2d_effects_post_hud_fn = void( __thiscall* )( void*, const c_view_setup& );
using render_smoke_overlay_fn = void( __thiscall* )( void*, bool );

// panel
using paint_traverse_fn = void( __thiscall* )( void*, unsigned int, bool, bool );

// surface
using on_screen_size_changed_fn = void( __thiscall* )( void*, int, int );

// studio render
using draw_model_fn = void( __thiscall* )( c_studio_render*, void*, const draw_model_info_t&, matrix3x4_t*, float*, float*, const vector3d&, int );

using draw_model_array_fn = void( __thiscall* )( void*, const studio_model_array_info2_t&, int, studio_array_data_t*, int, int );

// model render
using draw_model_execute_fn = void( __thiscall* )( void*, void*, const draw_model_state_t&, const model_render_info_t&, matrix3x4_t* );

// client mode
using override_view_fn = void( __thiscall* )( void*, c_view_setup* );
using get_viewmodel_fov_fn = float( __thiscall* )( void* );
using create_move_fn = bool( __thiscall* )( void*, float, c_usercmd* );
using do_post_screen_effects_fn = bool( __thiscall* )( void*, c_view_setup* );

// prediction
using in_prediction_fn = bool( __thiscall* )( void* );
using run_command_fn = void( __thiscall* )( void*, c_csplayer*, c_usercmd*, c_movehelper* );

// game movement
using process_movement_fn = void( __thiscall* )( void*, c_csplayer*, c_movedata* );

// engine bsp tree query
using list_leaves_in_box_fn = int( __thiscall* )( void*, const vector3d&, const vector3d&, unsigned short*, int );

// engine
using is_connected_fn = bool( __thiscall* )( void* );
using is_paused_fn = bool( __thiscall* )( void* );
using is_hltv_fn = bool( __thiscall* )( void* );
using get_screen_aspect_ratio_fn = float( __thiscall* )( void*, int, int );

// cl_brushfastpath
using cl_brushfastpath_get_bool_fn = bool( __thiscall* )( void* );

// get_cvars_fn
using get_int_fn = int( __thiscall* )( void* );
using get_bool_fn = bool( __thiscall* )( void* );
using get_float_fn = float( __thiscall* )( void* );

// engine trace
using clip_ray_to_collideable_fn = void( __thiscall* )( void*, const ray_t&, unsigned int, c_collideable*, c_game_trace* );
using trace_ray_fn = void( __thiscall* )( void*, const ray_t&, unsigned int, i_trace_filter*, c_game_trace* );

// player
using standard_blending_rules_fn = void( __thiscall* )( c_csplayer*, c_studiohdr*, vector3d*, vector4d*, float, int );
using build_transformations_fn = void( __thiscall* )( void*, c_studiohdr*, int, int, int, int, int );
using is_player_fn = bool( __thiscall* )( void* );
using do_extra_bone_processing_fn = void( __thiscall* )( void*, c_studiohdr*, vector3d*, vector4d*, const matrix3x4_t&, uint8_t*, void* );
using update_clientside_animation_fn = void( __thiscall* )( void* );
using setup_bones_fn = bool( __thiscall* )( void*, matrix3x4_t*, int, int, float );

// client state
using packet_start_fn = void( __thiscall* )( void*, int, int );
using packet_end_fn = void( __thiscall* )( void* );

// direct device
using end_scene_fn = HRESULT( __stdcall* )( IDirect3DDevice9* );
using present_fn = HRESULT( __stdcall* )( IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA* );
using reset_fn = HRESULT( __stdcall* )( IDirect3DDevice9*, D3DPRESENT_PARAMETERS* );

// material system
using find_material_fn = c_material*( __thiscall* )( void*, const char*, const char*, bool, const char* );

inline present_fn original_present{ };
inline reset_fn original_reset{ };

inline recv_var_proxy_fn original_sequence{ };
#pragma endregion

enum vtables_t
{
  client = 0,
  panel,
  surface,
  studio_render,
  client_mode,
  prediction,
  engine_bsp,
  engine_,
  debug_show_spread,
  cl_foot_contact_shadows,
  zoom_sensitivity_ratio_mouse,
  cl_csm_shadows,
  cl_brushfastpath,
  trace,
  client_state_,
  view_render_,
  game_movement,
  model_render,
  sv_cheats,
  key_values_system,
  cl_clock_correction,
  net_showfragments,
  material_system,
  vmt_max
};

extern std::array< hooks::vmt::c_hooks, vmt_max > vtables;

extern hooks::detour::c_hooks hooker;

namespace tr
{
  namespace client
  {
    extern void __fastcall get_exposure_range( float* min, float* max );
    extern void __fastcall perform_screen_overlay( void* _this, void* edx, int x, int y, int w, int h );
    extern bool __fastcall dispatch_user_message( void* _this, void* edx, int msg_type, int arg, int arg1, void* data );
    extern void __fastcall frame_stage_notify( void* ecx, void* edx, int stage );
    extern void __fastcall create_move_wrapper( void* _this, int, int sequence_number, float input_sample_frametime, bool active );
    extern void __fastcall setup_clr_modulation( void* ecx, void* edx, int cnt, model_list_by_type_t* list );
    extern void __fastcall draw_models( void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, char a7 );
    extern void __fastcall add_view_model_bob( void* ecx, void* edx, void* model, vector3d& origin, vector3d& angles );
    extern void __fastcall calc_view_model_bob( void* ecx, void* edx, vector3d& position );
    extern void __fastcall level_init_pre_entity( void* ecx, void* edx, const char* map );
    extern void __fastcall level_init_post_entity( void* ecx, void* edx );
    extern void __fastcall physics_simulate( c_csplayer* ecx, void* edx );
    extern bool __fastcall write_usercmd_to_delta_buffer( void* ecx, void* edx, int slot, void* buf, int from, int to, bool isnewcommand );
    extern void __fastcall render_glow_boxes( c_glow_object_manager* ecx, void* edx, int pass, void* ctx );
    extern void __fastcall update_postscreen_effects( void* ecx, void* edx );
  }

  namespace client_mode
  {
    extern void __fastcall override_view( void* ecx, void* edx, c_view_setup* setup );
    extern int __fastcall set_view_model_offsets( void* ecx, void* edx, int something, float x, float y, float z );
    extern bool __fastcall draw_fog( void* ecx, void* edx );
    extern bool __fastcall do_post_screen_effects( void* ecx, void* edx, c_view_setup* setup );
    extern bool __stdcall create_move( float a, c_usercmd* cmd );
  }

  namespace prediction
  {
    extern void __fastcall run_command( void* ecx, void* edx, c_csplayer* player, c_usercmd* cmd, c_movehelper* move_helper );
    extern bool __fastcall in_prediction( void* ecx, void* edx );
    extern void __fastcall process_movement( void* ecx, void* edx, c_csplayer* player, c_movedata* data );
  }

  namespace client_state
  {
    extern void __fastcall packet_start( void* ecx, void* edx, int incoming, int outgoing );
    extern void __fastcall packet_end( void* ecx, void* edx );
  }

  namespace engine
  {
    extern bool __fastcall temp_entities( c_clientstate* ecx, void* edx, void* msg );
    extern bool __fastcall send_net_msg( i_net_channel_info* ecx, void* edx, c_net_message& msg, bool force_reliable, bool voice );
    extern int __fastcall list_leaves_in_box( void* ecx, void* edx, const vector3d& mins, const vector3d& maxs, unsigned short* list, int list_max );
    extern bool __fastcall using_static_props_debug( void* ecx, void* edx );
    extern float __fastcall get_screen_aspect_ratio( void* ecx, void* edx, int width, int height );
    extern void __fastcall check_file_crc_with_server( void* ecx, void* edx );
    extern bool __fastcall is_connected( void* ecx, void* edx );
    extern bool __fastcall is_paused( void* ecx, void* edx );
    extern bool __fastcall is_hltv( void* ecx, void* edx );
    extern void __vectorcall read_packets( bool final_tick );
    extern void __vectorcall cl_move( float accumulated_extra_samples, bool final_tick );
    extern void __fastcall process_packet( c_netchan* ecx, void* edx, void* packet, bool header );
    extern int __fastcall send_datagram( void* netchan, void* edx, void* datagram );
  }

  namespace panel
  {
    extern void __fastcall paint_traverse( c_panel* ecx, void* edx, unsigned int panel, bool a, bool b );
  }

  namespace surface
  {
    extern void __fastcall on_screen_size_changed( void* ecx, void* edx, int old_w, int old_h );
  }

  namespace netvar_proxies
  {
    extern void __cdecl viewmodel_sequence( c_recv_proxy_data* data, void* entity, void* out );
  }

  namespace studio_render
  {
    extern void __fastcall draw_model(
      c_studio_render* ecx, void* edx, void* results, const draw_model_info_t& info, matrix3x4_t* bone_to_world, float* flex_weights, float* flex_delayed_weights, const vector3d& model_origin, int flags );

    extern void __fastcall draw_model_array( void* ecx, void* edx, const studio_model_array_info2_t& info, int count, studio_array_data_t* array_data, int stride, int flags );
  }

  namespace model_render
  {
    extern void __fastcall draw_model_execute( void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world );
  }

  namespace material_system
  {
    extern c_material* __fastcall find_material( void* _this, void* ebp, const char* mat_name, const char* tex_name, bool complain, const char* comp_prefix );
    extern void __fastcall get_color_modulation( c_material* ecx, void* edx, float* r, float* g, float* b );
  }

  namespace convars
  {
    extern int __fastcall sv_cheats_get_int( void* ecx, void* edx );
    extern int __fastcall cl_foot_contact_shadows_get_int( void* ecx, void* edx );
    extern int __fastcall cl_brushfastpath_get_int( void* ecx, void* edx );
    extern int __fastcall cl_csm_shadows_get_int( void* ecx, void* edx );
    extern int __fastcall debug_show_spread_get_int( void* ecx, void* edx );
    extern int __fastcall cl_clock_correction_get_int( void* ecx, void* edx );
    extern bool __fastcall net_showfragments_get_bool( void* ecx, void* edx );
  }

  namespace player
  {
    extern bool __fastcall interpolate( void* ecx, void* edx, float time );
    extern bool __fastcall want_reticle_shown( void* ecx, void* edx );
    extern int process_interpolated_list( );
    extern void __fastcall add_view_model_bob( void* ecx, void* edx, c_baseentity* model, vector3d& pos, vector3d& angles );
    extern void __fastcall standard_blending_rules( c_csplayer* ecx, void* edx, c_studiohdr* hdr, vector3d* pos, vector4d* q, float cur_time, int bone_mask );
    extern void __fastcall build_transformations( c_csplayer* ecx, void* edx, c_studiohdr* hdr, int a3, int a4, int a5, int a6, int a7 );
    extern void __fastcall calc_viewmodel_view( void* ecx, void* edx, c_csplayer* owner, vector3d& eye_pos, vector3d& eye_angles );
    extern void* __fastcall model_renderable_animating( void* ecx, void* edx );
    extern bool __fastcall should_skip_anim_frame( c_csplayer* ecx, void* edx );
    extern void __fastcall do_extra_bone_processing( c_csplayer* ecx, void* edx, c_studiohdr* hdr, vector3d* pos, vector4d* q, const matrix3x4_t& mat, uint8_t* bone_computed, void* context );
    extern void __fastcall add_renderable( void* ecx, void* edx, c_renderable* pRenderable, bool bRenderWithViewModels, int nType, int nModelType, int nSplitscreenEnabled );
    extern void __fastcall update_clientside_animation( c_csplayer* ecx, void* edx );
    extern void __fastcall modify_eye_position( c_animstate* ecx, void* edx, vector3d& pos );
    extern void __fastcall calc_view( c_csplayer* ecx, void* edx, vector3d& eye_origin, vector3d& eye_angles, float& near_, float& far_, float& fov );
    extern bool __fastcall setup_bones( void* ecx, void* edx, matrix3x4_t* bone_to_world, int max_bones, int mask, float time );
  }

  namespace trace
  {
    extern bool __fastcall trace_filter_for_head_collision( void* ecx, void* edx, c_csplayer* player, void* trace_params );
    extern void __fastcall clip_ray_to_collideable( void* ecx, void* edx, const ray_t& ray, unsigned int mask, c_collideable* collide, c_game_trace* trace );
    extern void __fastcall trace_ray( void* ecx, void* edx, const ray_t& ray, unsigned int mask, i_trace_filter* filter, c_game_trace* trace );
  }

  namespace view_render
  {
    extern void __fastcall render_2d_effects_post_hud( void* ecx, void* edx, const c_view_setup& setup );
    extern void __fastcall render_smoke_overlay( void* ecx, void* edx, bool unk );
    extern void __fastcall on_render_start( void* ecx, void* edx );
  }

  namespace direct
  {
    extern HRESULT __stdcall present( IDirect3DDevice9* device, const RECT* src_rect, const RECT* dest_rect, HWND window_override, const RGNDATA* dirty_region );
    extern HRESULT __stdcall reset( IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params );
  }

  extern LRESULT __stdcall wnd_proc( HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam );
}

namespace hooks
{
  extern void init( );
  extern void unhook( );
}