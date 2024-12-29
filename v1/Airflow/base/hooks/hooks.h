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
class c_baseentity;
class c_recv_proxy_data;
class c_svc_msg_voice_data;
class c_engine_sound;
class c_cvar;

struct draw_model_state_t;
struct model_render_info_t;
struct draw_model_info_t;
struct matrix3x4_t;
struct studio_model_array_info2_t;
struct studio_array_data_t;
struct model_list_by_type_t;
struct model_render_system_data_t;
struct brush_array_instance_data_t;
struct ray_t;
struct start_sound_params_t;

enum vtables_t
{
	vmt_client = 0,
	vmt_panel,
	vmt_surface,
	vmt_studio_render,
	vmt_client_mode,
	vmt_prediction,
	vmt_engine_bsp,
	vmt_engine_,
	vmt_debug_show_spread,
	vmt_cl_foot_contact_shadows,
	vmt_zoom_sensitivity_ratio_mouse,
	vmt_cl_csm_shadows,
	vmt_cl_brushfastpath,
	vmt_trace,
	vmt_client_state_,
	vmt_view_render_,
	vmt_game_movement,
	vmt_model_render,
	vmt_sv_cheats,
	vmt_key_values_system,
	vmt_cl_clock_correction,
	vmt_material_system,
	vmt_file_system,
	vmt_engine_sound,
	vmt_direct, //we also add this
	vmt_max
};

namespace tr
{
	namespace client
	{
		void __fastcall get_exposure_range(float* min, float* max);
		void __fastcall perform_screen_overlay(void* _this, void* edx, int x, int y, int w, int h);
		bool __fastcall dispatch_user_message(void* _this, void* edx, int msg_type, int arg, int arg1, void* data);
		void __fastcall frame_stage_notify(void* ecx, void* edx, int stage);
		void __fastcall create_move_wrapper(void* _this, int, int sequence_number, float input_sample_frametime, bool active);
		void __fastcall setup_clr_modulation_brushes(void* ecx, void* edx, int cnt, model_render_system_data_t* models, brush_array_instance_data_t* data, int render_mode);
		void __fastcall setup_clr_modulation(void* ecx, void* edx, int cnt, model_list_by_type_t* list);
		void __fastcall draw_models(void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, char a7);
		void __fastcall add_view_model_bob(void* ecx, void* edx, void* model, vector3d& origin, vector3d& angles);
		void __fastcall calc_view_model_bob(void* ecx, void* edx, vector3d& position);
		void __fastcall level_init_pre_entity(void* ecx, void* edx, const char* map);
		void __fastcall level_init_post_entity(void* ecx, void* edx);
		void __fastcall level_shutdown(void* ecx, void* edx);
		void __fastcall physics_simulate(c_csplayer* ecx, void* edx);
		bool __fastcall write_usercmd_to_delta_buffer(void* ecx, void* edx, int slot, void* buf, int from, int to, bool isnewcommand);
		void __fastcall render_glow_boxes(c_glow_object_manager* ecx, void* edx, int pass, void* ctx);
		void __fastcall update_postscreen_effects(void* ecx, void* edx);
		bool __fastcall process_spotted_entity_update(void* ecx, uint32_t edx, c_spotted_entity_update_message* message);
	}

	namespace client_mode
	{
		void __fastcall override_view(void* ecx, void* edx, c_view_setup* setup);
		int __fastcall set_view_model_offsets(void* ecx, void* edx, int something, float x, float y, float z);
		bool __fastcall draw_fog(void* ecx, void* edx);
		bool __fastcall do_post_screen_effects(void* ecx, void* edx, c_view_setup* setup);
		bool __stdcall create_move(float a, c_usercmd* cmd);
	}

	namespace prediction
	{
		void __fastcall run_command(void* ecx, void* edx, c_csplayer* player, c_usercmd* cmd, c_movehelper* move_helper);
		bool __fastcall in_prediction(void* ecx, void* edx);
		void __fastcall process_movement(void* ecx, void* edx, c_csplayer* player, c_movedata* data);
	}

	namespace client_state
	{
		void __fastcall packet_start(void* ecx, void* edx, int incoming, int outgoing);
		void __fastcall packet_end(void* ecx, void* edx);
	}

	namespace engine
	{
		int __fastcall send_datagram(void* netchan, void* edx, void* datagram);
		bool __fastcall temp_entities(c_clientstate* ecx, void* edx, void* msg);
		int __fastcall can_load_third_party_files(void* _this);
		int __stdcall get_unverified_file_hashes(void* _this, void* someclass, int nMaxFiles);
		bool __fastcall send_net_msg(i_net_channel_info* ecx, void* edx, c_net_message& msg, bool force_reliable, bool voice);
		int __fastcall list_leaves_in_box(void* ecx, void* edx, const vector3d& mins, const vector3d& maxs, unsigned short* list, int list_max);
		void __fastcall emit_sound(c_engine_sound* thisptr, uint32_t edx, void* filter, int ent_index, int channel, const char* sound_entry, unsigned int sound_entry_hash,
			const char* sample, float volume, float attenuation, int seed, int flags, int pitch, const vector3d* origin, const vector3d* direction,
			void* vec_origins, bool update_positions, float sound_time, int speaker_entity, int test);
		bool __fastcall using_static_props_debug(void* ecx, void* edx);
		float __fastcall get_screen_aspect_ratio(void* ecx, void* edx, int width, int height);
		void __fastcall check_file_crc_with_server(void* ecx, void* edx);
		bool __fastcall is_connected(void* ecx, void* edx);
		bool __fastcall is_paused(void* ecx, void* edx);
		bool __fastcall msg_voice_data(void* ecx, void* edx, c_svc_msg_voice_data* message);
		bool __fastcall is_hltv(void* ecx, void* edx);
		void __vectorcall read_packets(bool final_tick);
		void __vectorcall cl_move(float accumulated_extra_samples, bool final_tick);
		void __fastcall process_packet(c_netchan* ecx, void* edx, void* packet, bool header);
		void __stdcall host_shutdown();
		int __fastcall start_sound_immediate(start_sound_params_t* ecx, void* edx);
	}

	namespace panel
	{
		void __fastcall paint_traverse(c_panel* ecx, void* edx, unsigned int panel, bool a, bool b);
	}

	namespace surface
	{
		void __fastcall on_screen_size_changed(void* ecx, void* edx, int old_w, int old_h);
	}

	namespace studio_render
	{
		void __fastcall draw_model(c_studio_render* ecx, void* edx, void* results, const draw_model_info_t& info, matrix3x4_t* bone_to_world, float* flex_weights, float* flex_delayed_weights, const vector3d& model_origin, int flags);
		void __fastcall draw_model_array(void* ecx, void* edx, const studio_model_array_info2_t& info, int count, studio_array_data_t* array_data, int stride, int flags);
	}

	namespace model_render
	{
		void __fastcall draw_model_execute(void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world);
	}

	namespace material_system
	{
		c_material* __fastcall find_material(void* _this, void* ebp, const char* mat_name, const char* tex_name = nullptr, bool complain = true, const char* comp_prefix = NULL);
		void __fastcall get_color_modulation(c_material* ecx, void* edx, float* r, float* g, float* b);
	}

	namespace key_values
	{
		void* __fastcall alloc_key_values_memory(c_key_values_system* thisptr, int edx, int size);
	}

	namespace convars
	{
		int __fastcall sv_cheats_get_int(void* ecx, void* edx);
		int __fastcall cl_foot_contact_shadows_get_int(void* ecx, void* edx);
		int __fastcall cl_brushfastpath_get_int(void* ecx, void* edx);
		int __fastcall cl_csm_shadows_get_int(void* ecx, void* edx);
		int __fastcall debug_show_spread_get_int(void* ecx, void* edx);
		int __fastcall cl_clock_correction_get_int(void* ecx, void* edx);
	}

	namespace player
	{
		int process_interpolated_list();
		void __fastcall interpolate_server_entities(void* ecx, void* edx);
		bool __fastcall interpolate(void* ecx, void* edx, float time);
		bool __fastcall interpolate_player(void* ecx, void* edx, float time);
		void __fastcall standard_blending_rules(c_csplayer* ecx, void* edx, c_studiohdr* hdr, vector3d* pos, vector4d* q, float cur_time, int bone_mask);
		void __fastcall add_view_model_bob(void* ecx, void* edx, c_baseentity* model, vector3d& pos, vector3d& angles);
		void __fastcall build_transformations(c_csplayer* ecx, void* edx, c_studiohdr* hdr, int a3, int a4, int a5, int a6, int a7);
		void __fastcall calc_viewmodel_view(void* ecx, void* edx, c_csplayer* owner, vector3d& eye_pos, vector3d& eye_angles);
		void* __fastcall model_renderable_animating(void* ecx, void* edx);
		bool __fastcall should_skip_anim_frame(c_csplayer* ecx, void* edx);
		void __fastcall on_bbox_change_callback(c_csplayer* ecx, void* edx, vector3d* old_mins, vector3d* new_mins, vector3d* old_maxs, vector3d* new_maxs);
		void __fastcall do_extra_bone_processing(c_csplayer* ecx, void* edx, c_studiohdr* hdr, vector3d* pos, vector4d* q, const matrix3x4_t& mat, uint8_t* bone_computed, void* context);
		void __fastcall modify_body_yaw(c_csplayer* ecx, void* edx, matrix3x4_t* bones, int mask);
		void __fastcall add_renderable(void* ecx, void* edx, c_renderable* pRenderable, bool bRenderWithViewModels, int nType, int nModelType, int nSplitscreenEnabled);
		void __fastcall update_clientside_animation(c_csplayer* ecx, void* edx);
		vector3d* __fastcall weapon_shootpos(c_csplayer* ecx, void* edx, vector3d& eye);
		void __fastcall modify_eye_position(c_animstate* ecx, void* edx, vector3d& pos);
		void __fastcall calc_view(c_csplayer* ecx, void* edx, vector3d& eye_origin, vector3d& eye_angles, float& near_, float& far_, float& fov);
		bool __fastcall want_reticle_shown(void* ecx, void* edx);
		vector3d* __fastcall eye_angles(void* ecx, void* edx);
		bool __fastcall setup_bones(void* ecx, void* edx, matrix3x4_t* bone_to_world, int max_bones, int mask, float time);
	}

	namespace trace
	{
		void __fastcall clip_ray_to_collideable(void* ecx, void* edx, const ray_t& ray, unsigned int mask, c_collideable* collide, c_game_trace* trace);
		void __fastcall trace_ray(void* ecx, void* edx, const ray_t& ray, unsigned int mask, i_trace_filter* filter, c_game_trace* trace);
		bool __fastcall trace_filter_for_head_collision(void* ecx, void* edx, c_csplayer* player, void* trace_params);
	}

	namespace view_render
	{
		void __fastcall on_render_start(void* ecx, void* edx);
		void __fastcall render_2d_effects_post_hud(void* ecx, void* edx, const c_view_setup& setup);
		void __fastcall render_smoke_overlay(void* ecx, void* edx, bool unk);
	}

	namespace direct
	{
		//HRESULT __stdcall present(IDirect3DDevice9* device, const RECT* src_rect, const RECT* dest_rect, HWND window_override, const RGNDATA* dirty_region); //we replace this 
		HRESULT __stdcall end_scene(IDirect3DDevice9* device);
		HRESULT __stdcall reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params);
	}

	namespace netvar_proxies
	{
		void __cdecl viewmodel_sequence(c_recv_proxy_data* data, void* entity, void* out);
		void __cdecl simulation_time(c_recv_proxy_data* data, void* entity, void* out);
		void __cdecl anim_time(c_recv_proxy_data* data, void* entity, void* out);
	}

	LRESULT __stdcall wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
}

namespace hooks
{
	void init();
	void unhook();
}

/*
using present_fn = HRESULT(__stdcall*)(IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
using reset_fn = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);

inline present_fn original_present{};
inline reset_fn original_reset{};
*/     //we remove this cause useless

inline recv_var_proxy_fn original_sequence{};
inline recv_var_proxy_fn original_simulation_time{};
inline recv_var_proxy_fn original_anim_time{};

inline hooks::vmt::c_hooks vtables[vmt_max]{};
inline hooks::detour::c_hooks hooker{};