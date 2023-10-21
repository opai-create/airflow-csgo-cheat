#pragma once

#define OFFSET(name, type, offset) \
INLINE type name() { \
	return *(std::remove_reference_t<type>*)((std::uintptr_t)this + XORN(offset)); \
}

#define OFFSET_PTR(name, type, offset) \
INLINE type* name() { \
	return (type*)((std::uintptr_t)this + XORN(offset)); \
}

#define NETVAR(name, type, table, prop) \
INLINE type name() { \
	static std::uintptr_t offset = netvars::get_offset(HASH(table), HASH(prop)); \
	return *(std::remove_reference_t<type>*)((std::uintptr_t)this + offset); \
}

#define NETVAR_PTR(name, type, table, prop) \
INLINE type* name() { \
	static std::uintptr_t offset = netvars::get_offset(HASH(table), HASH(prop)); \
	return (type*)((std::uintptr_t)this + offset); \
}

#define NETVAR_OFFSET(name, type, table, prop, offset) \
INLINE type name() { \
	static std::uintptr_t _offset = netvars::get_offset(HASH(table), HASH(prop)) + XORN(offset); \
	return *(std::remove_reference_t<type>*)((std::uintptr_t)this + _offset); \
}

#define NETVAR_OFFSET_PTR(name, type, table, prop, offset) \
INLINE type* name() { \
	static std::uintptr_t _offset = netvars::get_offset(HASH(table), HASH(prop)) + XORN(offset); \
	return (std::remove_reference_t<type>*)((std::uintptr_t)this + _offset); \
}

#define DATAMAP(func_name, type, name) \
INLINE type func_name() { \
	static std::uintptr_t offset = offsets::find_in_datamap(this->get_pred_desc_map(), HASH(name)); \
	return *(std::remove_reference_t<type>*)((std::uintptr_t)this + offset); \
}

#define DATAMAP_PTR(func_name, type, name) \
INLINE type* func_name() { \
	static std::uintptr_t offset = offsets::find_in_datamap(this->get_pred_desc_map(), HASH(name)); \
	return (type*)((std::uintptr_t)this + offset); \
}

namespace offsets
{
	std::uint32_t find_in_datamap(datamap_t* map, std::uint32_t hash);

	inline memory::address_t d3d_device;
	inline memory::address_t studio_hdr;
	inline memory::address_t world_to_screen_matrix;
	inline memory::address_t return_addr_maintain_seq_transitions;
	inline memory::address_t update_client_side_animation;
	inline memory::address_t get_name;
	inline memory::address_t load_from_buffer;
	inline memory::address_t get_color_modulation;
	inline memory::address_t draw_models;
	inline memory::address_t load_named_sky;
	inline memory::address_t disable_post_process;
	inline memory::address_t post_process_data;
	inline memory::address_t return_addr_drift_pitch;
	inline memory::address_t return_addr_apply_shake;
	inline memory::address_t remove_smoke;
	inline memory::address_t prediction_random_seed;
	inline memory::address_t prediction_player;
	inline memory::address_t add_view_model_bob;
	inline memory::address_t calc_view_model_bob;
	inline memory::address_t create_animstate;
	inline memory::address_t reset_animstate;
	inline memory::address_t update_animstate;
	inline memory::address_t set_abs_angles;
	inline memory::address_t set_abs_origin;
	inline memory::address_t get_exposure_range;
	inline memory::address_t should_skip_anim_frame;
	inline memory::address_t setup_bones;
	inline memory::address_t standard_blending_rules;
	inline memory::address_t do_extra_bone_processing;
	inline memory::address_t return_addr_setup_velocity;
	inline memory::address_t return_addr_accumulate_layers;
	inline memory::address_t return_addr_reevaluate_anim_lod;
	inline memory::address_t return_addr_extrapolation;
	inline memory::address_t return_addr_setup_bones;
	inline memory::address_t invalidate_bone_cache;
	inline memory::address_t attachments_helper;
	inline memory::address_t studio_hdr_ptr;
	inline memory::address_t invalidate_physics_recursive;
	inline memory::address_t trace_filter;
	inline memory::address_t trace_filter_skip_entities;
	inline memory::address_t write_user_cmd;
	inline memory::address_t lookup_bone;
	inline memory::address_t glow_object;
	inline memory::address_t draw_hitbox;
	inline memory::address_t server_edict;
	inline memory::address_t model_renderable_animating;
	inline memory::address_t setup_clr_modulation;
	inline memory::address_t set_collision_bounds;
	inline memory::address_t find_hud_element;
	inline memory::address_t process_packet;
	inline memory::address_t direct_present;
	inline memory::address_t direct_reset;
	inline memory::address_t direct_device;
	inline memory::address_t local;
	inline memory::address_t screen_matrix;
	inline memory::address_t read_packets;
	inline memory::address_t add_renderable;
	inline memory::address_t perform_screen_overlay;
	inline memory::address_t render_glow_boxes;
	inline memory::address_t clip_ray_to_hitbox;
	inline memory::address_t read_packets_return_addr;
	inline memory::address_t compute_hitbox_surrounding_box;
	inline memory::address_t clantag;
	inline memory::address_t is_breakable_entity;
	inline memory::address_t process_interpolated_list;
	inline memory::address_t allow_extrapolation;
	inline memory::address_t temp_entities;
	inline memory::address_t calc_shotgun_spread;
	inline memory::address_t line_goes_through_smoke;
	inline memory::address_t file_system_ptr;
	inline memory::address_t item_system;
	inline memory::address_t force_update;
	inline memory::address_t update_visibility;
	inline memory::address_t thread_id_allocated;
	inline memory::address_t weapon_shootpos;
	inline memory::address_t construct_voice_data_message;
	inline memory::address_t put_attachments;
	inline memory::address_t calc_absolute_position;
	inline memory::address_t eye_angles;
	inline memory::address_t input;
	inline memory::address_t client_state;
	inline memory::address_t move_helper;
	inline memory::address_t global_vars;
	inline memory::address_t weapon_system;
	inline memory::address_t engine_paint;
	inline memory::address_t client_side_animation_list;
	inline memory::address_t interpolate_server_entities;
	inline memory::address_t get_pose_parameter;
	inline memory::address_t get_bone_merge;
	inline memory::address_t update_merge_cache;
	inline memory::address_t copy_to_follow;
	inline memory::address_t copy_from_follow;
	inline memory::address_t add_dependencies;
	inline memory::address_t send_move_addr;
	inline memory::address_t get_unverified_file_hashes;
	inline memory::address_t calc_chase_cam_view;
	inline memory::address_t start_sound_immediate;
	inline memory::address_t index_from_anim_tag_name;
	inline memory::address_t setup_aim_matrix;
	inline memory::address_t setup_movement;
	inline memory::address_t estimate_abs_velocity;
	inline memory::address_t post_data_update;
	inline memory::address_t notify_on_layer_change_weight;
	inline memory::address_t notify_on_layer_change_sequence;
	inline memory::address_t accumulate_layers;
	inline memory::address_t on_latch_interpolated_variables;
	inline memory::address_t interpolate;
	inline memory::address_t interpolate_player;
	inline memory::address_t reset_latched;

#ifdef LEGACY
	inline memory::address_t view_render;
	inline memory::address_t glow_object_manager;
	inline memory::address_t return_addr_loadout_allowed;
	inline memory::address_t using_static_prop_debug;
	inline memory::address_t build_transformations;
	inline memory::address_t update_postscreen_effects;
	inline memory::address_t get_sequence_activity;
	inline memory::address_t modify_eye_position;
	inline memory::address_t init_key_values;
	inline memory::address_t want_reticle_shown;
	inline memory::address_t update_all_viewmodel_addons;
	inline memory::address_t get_viewmodel;
	inline memory::address_t calc_view;
	inline memory::address_t get_hud_ptr;
	inline memory::address_t draw_fog;
	inline memory::address_t cl_move;
	inline memory::address_t clear_killfeed;
	inline memory::address_t physics_simulate;
	inline memory::address_t send_datagram;
	inline memory::address_t list_leaves_in_box;
	inline memory::address_t calc_viewmodel_view;
	inline memory::address_t return_addr_cam_think;
	inline memory::address_t send_net_msg;
	inline memory::address_t return_addr_process_input;
	inline memory::address_t host_shutdown;
	inline memory::address_t destruct_voice_data_message;
	inline memory::address_t msg_voice_data;
	inline memory::address_t physics_run_think;
	inline memory::address_t think;
	inline memory::address_t post_think_physics;
	inline memory::address_t simulate_player_simulated_entities;
	inline memory::address_t game_rules;
	inline memory::address_t return_addr_send_datagram_cl_move;
	inline memory::address_t add_activity_modifier;
	inline memory::address_t get_weapon_prefix;
	inline memory::address_t find_mapping;
	inline memory::address_t select_sequence_from_mods;
	inline memory::address_t get_sequence_desc;
	inline memory::address_t lookup_sequence;
	inline memory::address_t get_sequence_linear_motion;
	inline memory::address_t update_layer_order_preset;

	inline memory::address_t ik_ctx_construct;
	inline memory::address_t ik_ctx_destruct;
	inline memory::address_t ik_ctx_init;
	inline memory::address_t ik_ctx_update_targets;
	inline memory::address_t ik_ctx_solve_dependencies;
	inline memory::address_t bone_setup_init_pose;
	inline memory::address_t accumulate_pose;
	inline memory::address_t bone_setup_calc_autoplay_sequences;
	inline memory::address_t bone_setup_calc_bone_adjust;

	inline memory::address_t show_and_update_selection;
	inline memory::address_t view_render_beams;
	inline memory::address_t return_addr_show_radar;
	inline memory::address_t calc_roaming_view;
	inline memory::address_t return_addr_post_process;
	inline memory::address_t cache_sequences;
	inline memory::address_t setup_weapon_action;
	inline memory::address_t notify_on_layer_change_cycle;

#else
	inline memory::address_t view_render;
	inline memory::address_t glow_object_manager;
	inline memory::address_t return_addr_loadout_allowed;
	inline memory::address_t using_static_prop_debug;
	inline memory::address_t build_transformations;
	inline memory::address_t update_postscreen_effects;
	inline memory::address_t get_sequence_activity;
	inline memory::address_t modify_eye_position;
	inline memory::address_t init_key_values;
	inline memory::address_t alloc_key_values_engine;
	inline memory::address_t alloc_key_values_client;
	inline memory::address_t on_bbox_change_callback;
	inline memory::address_t want_reticle_shown;
	inline memory::address_t update_all_viewmodel_addons;
	inline memory::address_t get_viewmodel;
	inline memory::address_t calc_view;
	inline memory::address_t get_hud_ptr;
	inline memory::address_t draw_fog;
	inline memory::address_t cl_move;
	inline memory::address_t clear_killfeed;
	inline memory::address_t physics_simulate;
	inline memory::address_t clamp_bones_in_bbox;
	inline memory::address_t clear_hud_weapons;
	inline memory::address_t send_datagram;
	inline memory::address_t list_leaves_in_box;
	inline memory::address_t calc_viewmodel_view;
	inline memory::address_t return_addr_cam_think;
	inline memory::address_t send_net_msg;
	inline memory::address_t return_addr_process_input;
	inline memory::address_t viewmodel_arm_cfg;
	inline memory::address_t trace_filter_to_head_collision;
	inline memory::address_t mask_ptr;
	inline memory::address_t update_addon_models;
	inline memory::address_t host_shutdown;
	inline memory::address_t destruct_voice_data_message;
	inline memory::address_t msg_voice_data;
	inline memory::address_t physics_run_think;
	inline memory::address_t think;
	inline memory::address_t post_think_physics;
	inline memory::address_t simulate_player_simulated_entities;
	inline memory::address_t game_rules;
	inline memory::address_t return_addr_send_datagram_cl_move;
	inline memory::address_t fire_bullet;
	inline memory::address_t add_activity_modifier;
	inline memory::address_t get_weapon_prefix;
	inline memory::address_t find_mapping;
	inline memory::address_t select_sequence_from_mods;
	inline memory::address_t get_sequence_desc;
	inline memory::address_t lookup_sequence;
	inline memory::address_t get_sequence_linear_motion;
	inline memory::address_t update_layer_order_preset;

	inline memory::address_t ik_ctx_construct;
	inline memory::address_t ik_ctx_destruct;
	inline memory::address_t ik_ctx_init;
	inline memory::address_t ik_ctx_update_targets;
	inline memory::address_t ik_ctx_solve_dependencies;
	inline memory::address_t bone_setup_init_pose;
	inline memory::address_t accumulate_pose;
	inline memory::address_t bone_setup_calc_autoplay_sequences;
	inline memory::address_t bone_setup_calc_bone_adjust;

	inline memory::address_t note_prediction_error;
	inline memory::address_t check_moving_ground;
	inline memory::address_t view_render_beams;
	inline memory::address_t return_addr_show_radar;
	inline memory::address_t calc_roaming_view;
	inline memory::address_t return_addr_post_process;

	inline memory::address_t notify_on_layer_change_cycle;
	inline memory::address_t setup_weapon_action;
	inline memory::address_t setup_whole_body_action;
	inline memory::address_t setup_flinch;
	inline memory::address_t cache_sequences;
#endif

	extern void init();
}