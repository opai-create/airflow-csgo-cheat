#include "globals.hpp"

namespace offsets
{
	std::uint32_t find_in_datamap(datamap_t* map, std::uint32_t hash)
	{
		while (map != nullptr)
		{
			for (int i = 0; i < map->size; i++)
			{
				auto& data_desc = map->desc[i];
				if (data_desc.name == NULL)
					continue;

				if (CONST_HASH(data_desc.name) == hash)
					return data_desc.offset[TD_OFFSET_NORMAL];

				if (data_desc.type == 10)
				{
					if (data_desc.td)
					{
						std::uint32_t offset;
						if ((offset = find_in_datamap(data_desc.td, hash)) != 0)
							return offset;
					}
				}
			}

			map = map->base;
		}
		return 0;
	}

	INLINE void init()
	{
		auto client_dll = HACKS->modules.client;
		auto materialsystem_dll = HACKS->modules.materialsystem;
		auto engine_dll = HACKS->modules.engine;
		auto server_dll = HACKS->modules.server;
		auto gameoverlayrenderer_dll = HACKS->modules.gameoverlayrenderer;
		auto shaderapidx9_dll = HACKS->modules.shaderapidx9;
		auto tier0_dll = HACKS->modules.tier0;
		auto datacache_dll = HACKS->modules.datacache;
		auto filesystem_stdio_dll = HACKS->modules.filesystem_stdio;

		d3d_device = memory::get_pattern(shaderapidx9_dll, CXOR("A1 ? ? ? ? 50 8B 08 FF 51 0C")).add(XORN(1));
		studio_hdr = memory::get_pattern(client_dll, CXOR("8B B6 ? ? ? ? 85 F6 74 05 83 3E 00 75 02 33 F6 F3 0F 10 44 24")).add(XORN(2));

		return_addr_maintain_seq_transitions = memory::get_pattern(client_dll, CXOR("84 C0 74 ? 8B 87 ? ? ? ? 89 87"));
		update_client_side_animation = memory::get_pattern(client_dll, CXOR("55 8B EC 51 56 8B F1 80 BE ?? ?? 00 00 00 74 36 8B 06 FF 90 ?? ?? 00 00"));
		world_to_screen_matrix = memory::get_pattern(client_dll, CXOR("0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9"));

		get_name = memory::get_pattern(client_dll, CXOR("56 8B 35 ? ? ? ? 85 F6 74 21 8B 41"));
		load_from_buffer = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89"));
		get_color_modulation = memory::get_pattern(materialsystem_dll, CXOR("55 8B EC 83 EC ? 56 8B F1 8A 46"));

		draw_models = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 51 8B 45 18"));
		load_named_sky = memory::get_pattern(engine_dll, CXOR("55 8B EC 81 EC ? ? ? ? 56 57 8B F9 C7 45"));
		disable_post_process = memory::get_pattern(client_dll, CXOR("80 3D ? ? ? ? ? 53 56 57 0F 85")).add(XORN(2));

		post_process_data = memory::get_pattern(client_dll, CXOR("0F 11 05 ? ? ? ? 0F 10 87")).add(XORN(3));

		return_addr_drift_pitch = memory::get_pattern(client_dll, CXOR("84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C"));
		return_addr_apply_shake = memory::get_pattern(client_dll, CXOR("84 C0 75 24 A1 ? ? ? ? B9 ? ? ? ? FF 50 1C A1 ? ? ? ? 51 C7 04 24 ? ? 80 3F B9 ? ? ? ? 53 57 FF 50 20"));

		remove_smoke = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0")).add(XORN(8));
		prediction_random_seed = memory::get_pattern(client_dll, CXOR("A3 ? ? ? ? 66 0F 6E 86")).add(XORN(1));
		prediction_player = memory::get_pattern(client_dll, CXOR("89 35 ? ? ? ? F3 0F 10 48")).add(XORN(2));
		add_view_model_bob = memory::get_pattern(client_dll, CXOR("55 8B EC A1 ? ? ? ? 83 EC 20 8B 40 34"));
		calc_view_model_bob = memory::get_pattern(client_dll, CXOR("55 8B EC A1 ? ? ? ? 83 EC 10 56 8B F1 B9"));

		create_animstate = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B F1 B9 ? ? ? ? C7 46"));
		reset_animstate = memory::get_pattern(client_dll, CXOR("56 6A 01 68 ? ? ? ? 8B F1"));
		update_animstate = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3 0F 11 54 24"));

		set_abs_angles = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8"));
		set_abs_origin = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8"));

		should_skip_anim_frame = memory::get_pattern(client_dll, CXOR("57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02"));
		setup_bones = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F0 B8 D8"));
		standard_blending_rules = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6"));
		do_extra_bone_processing = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C"));

		return_addr_setup_velocity = memory::get_pattern(client_dll, CXOR("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80"));
		return_addr_accumulate_layers = memory::get_pattern(client_dll, CXOR("84 C0 75 0D F6 87"));
		return_addr_reevaluate_anim_lod = memory::get_pattern(client_dll, CXOR("84 C0 0F 85 ? ? ? ? A1 ? ? ? ? 8B B7"));
		return_addr_extrapolation = memory::get_pattern(client_dll, CXOR("FF D0 A1 ? ? ? ? B9 ? ? ? ? D9 1D ? ? ? ? FF 50 34 85 C0 74 22 8B 0D ? ? ? ?")).add(XORN(0x29));
		return_addr_setup_bones = memory::get_pattern(client_dll, CXOR("84 C0 0F 84 ? ? ? ? 8B 44 24 24"));

		invalidate_bone_cache = memory::get_pattern(client_dll, CXOR("80 3D ? ? ? ? ? 74 16 A1 ? ? ? ? 48 C7 81"));
		attachments_helper = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4"));

		studio_hdr_ptr = memory::get_pattern(client_dll, CXOR("8B B7 ? ? ? ? 89 74 24 20"));
		invalidate_physics_recursive = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56 83 E0 04"));
		trace_filter = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F0 83 EC 7C 56 52"));
		trace_filter_skip_entities = memory::get_pattern(client_dll, CXOR("C7 45 ? ? ? ? ? 89 45 E4 8B 01"));

		write_user_cmd = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 51 53 56 8B D9"));
		lookup_bone = memory::get_pattern(client_dll, CXOR("55 8B EC 53 56 8B F1 57 83 BE ? ? ? ? ? 75"));
		glow_object = memory::get_pattern(client_dll, CXOR("0F 11 05 ? ? ? ? 83 C8 01 C7 05 ? ? ? ? ? ? ? ?")).add(XORN(3));
		draw_hitbox = memory::get_pattern(server_dll, CXOR("55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE"));
		server_edict = memory::get_pattern(server_dll, CXOR("8B 15 ? ? ? ? 33 C9 83 7A 18 01"));

		model_renderable_animating = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 85 C0 75 04 33 C0 5E C3 83")).relative(XORN(1));
		setup_clr_modulation = memory::get_pattern(client_dll, CXOR("55 8B EC 83 7D 08 ? 7E"));
		set_collision_bounds = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 0F BF 87")).relative(XORN(1));
		find_hud_element = memory::get_pattern(client_dll, CXOR("55 8B EC 53 8B 5D 08 56 57 8B F9 33 F6 39 77 28"));

		process_packet = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 C0 81 EC ? ? ? ? 53 56 57 8B 7D 08 8B D9"));

		direct_present = memory::get_pattern(gameoverlayrenderer_dll, CXOR("FF 15 ? ? ? ? 8B F0 85 FF")).add(XORN(2));
		direct_reset = memory::get_pattern(gameoverlayrenderer_dll, CXOR("C7 45 ? ? ? ? ? FF 15 ? ? ? ? 8B D8")).add(XORN(9));
		direct_device = memory::get_pattern(shaderapidx9_dll, CXOR("A1 ? ? ? ? 50 8B 08 FF 51 0C")).add(XORN(1));

		local = memory::get_pattern(client_dll, CXOR("8B 0D ? ? ? ? 83 FF FF 74 07")).add(XORN(2));
		screen_matrix = memory::get_pattern(client_dll, CXOR("0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9"));
		read_packets = memory::get_pattern(engine_dll, CXOR("53 8A D9 8B 0D ? ? ? ? 56 57 8B B9"));

		add_renderable = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B 75 08 57 FF 75 18"));
		perform_screen_overlay = memory::get_pattern(client_dll, CXOR("55 8B EC 51 A1 ? ? ? ? 53 56 8B D9"));

		render_glow_boxes = memory::get_pattern(client_dll, CXOR("53 8B DC 83 EC ? 83 E4 ? 83 C4 ? 55 8B 6B ? 89 6C ? ? 8B EC 83 EC ? 56 57 8B F9 89 7D ? 8B 4F"));

		clip_ray_to_hitbox = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 F3 0F 10 42"));

		read_packets_return_addr = memory::get_pattern(engine_dll, CXOR("85 C0 0F 95 C0 84 C0 75 0C"));
		compute_hitbox_surrounding_box = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 81 EC 24 04 00 00 ? ? ? ? ? ?"));

		clantag = memory::get_pattern(engine_dll, CXOR("53 56 57 8B DA 8B F9 FF 15"));

		is_breakable_entity = memory::get_pattern(client_dll, CXOR("55 8B EC 51 56 8B F1 85 F6 74 68"));
		process_interpolated_list = memory::get_pattern(client_dll, CXOR("53 0F B7 1D ? ? ? ? 56"));

		allow_extrapolation = memory::get_pattern(client_dll, CXOR("A2 ? ? ? ? 8B 45 E8"));
		get_exposure_range = memory::get_pattern(client_dll, CXOR("55 8B EC 51 80 3D ? ? ? ? ? 0F 57"));
		temp_entities = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 F8 83 EC 4C A1 ? ? ? ? 80"));
		calc_shotgun_spread = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? EB 38 83 EC 08")).relative(XORN(1));
		line_goes_through_smoke = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 08 8B 15 ? ? ? ? 0F 57 C0"));

		file_system_ptr = memory::get_pattern(engine_dll, CXOR("8B 0D ? ? ? ? 83 C1 04 8B 01 FF 37 FF 50 1C 89 47 10")).add(XORN(2));
		item_system = memory::get_pattern(client_dll, CXOR("A1 ? ? ? ? 85 C0 75 53"));
		force_update = memory::get_pattern(engine_dll, CXOR("A1 ? ? ? ? B9 ? ? ? ? 56 FF 50 14 8B 34 85"));

		update_visibility = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 24 53 56 57 8B F9 8D B7"));

		thread_id_allocated = memory::get_pattern(tier0_dll, CXOR("C6 86 ? ? ? ? ? 83 05 ? ? ? ? ? 5E 75 04 33 C0 87 07")).add(XORN(2));
		weapon_shootpos = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B 75 ? 57 8B F9 56 8B 07 FF 90"));

		construct_voice_data_message = memory::get_pattern(engine_dll, CXOR("56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7"));
		put_attachments = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 ? 8B D1 83 F8 ? 0F 8C"));
		calc_absolute_position = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 80 3D ? ? ? ? ? 56 57 8B F9"));
		eye_angles = memory::get_pattern(client_dll, CXOR("56 8B F1 85 F6 74 ? 8B 06 8B 80 ? ? ? ? FF D0 84 C0 74 ? 8A 86 ? ? ? ? 84 C0 74 ? 83 3D"));

		input = memory::get_pattern(client_dll, CXOR("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10")).add(XORN(1));
		client_state = memory::get_pattern(engine_dll, CXOR("A1 ? ? ? ? 8B 80 ? ? ? ? C3")).add(XORN(1));
		move_helper = memory::get_pattern(client_dll, CXOR("8B 0D ? ? ? ? 8B 46 08 68")).add(XORN(2));
		global_vars = memory::get_pattern(client_dll, CXOR("A1 ? ? ? ? F3 0F 10 40 ? 0F 5A C0 F2 0F 11 04")).add(XORN(1));
		weapon_system = memory::get_pattern(client_dll, CXOR("8B 35 ? ? ? ? FF 10 0F B7 C0")).add(XORN(2));
		engine_paint = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 EC 40 53 8B D9 8B 0D ? ? ? ? 89 5D F8"));
		client_side_animation_list = memory::get_pattern(client_dll, CXOR("A1 ? ? ? ? F6 44")).add(XORN(1));
		interpolate_server_entities = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56"));
		get_pose_parameter = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15 8B"));
		get_bone_merge = memory::get_pattern(client_dll, CXOR("89 86 ? ? ? ? E8 ? ? ? ? FF 75 08")).add(XORN(2));
		update_merge_cache = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 83 7E 10 ? 74 64")).relative(XORN(1));
		copy_to_follow = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 8B 87 ? ? ? ? 8D 8C 24 ? ? ? ? 8B 7C 24 18")).relative(XORN(1));
		copy_from_follow = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? F3 0F 10 45 ? 8D 84 24 ? ? ? ?")).relative(XORN(1));
		add_dependencies = memory::get_pattern(client_dll, CXOR("55 8B EC 81 EC BC ? ? ? 53 56 57"));
		send_move_addr = memory::get_pattern(engine_dll, CXOR("B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC"));
		get_unverified_file_hashes = memory::get_pattern(filesystem_stdio_dll, CXOR("55 8B EC 81 C1 ? ? ? ? 5D E9 ? ? ? ? CC 55 8B EC 8B 45 ? 89 41"));
		calc_chase_cam_view = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? 56 8B F1 57 8B 06"));
		start_sound_immediate = memory::get_pattern(engine_dll, CXOR("E8 ? ? ? ? 3B F8 0F 4F C7")).relative(XORN(1));
		index_from_anim_tag_name = memory::get_pattern(client_dll, CXOR("56 57 8B F9 BE ? ? ? ? 0F 1F 80"));
		setup_aim_matrix = memory::get_pattern(client_dll, CXOR("55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D"));
		setup_movement = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? 56 57 8B 3D ? ? ? ? 8B F1"));
		post_data_update = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 56 57 83 CB"));
		estimate_abs_velocity = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 56 8B F1 85 F6 74 ? 8B 06 8B 80 ? ? ? ? FF D0 84 C0 74 ? 8A 86"));
		notify_on_layer_change_weight = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 ? 85 C0 74 ? 80 B9 ? ? ? ? ? 74 ? 56 8B B1 ? ? ? ? 85 F6 74 ? 8D 4D ? 51 50 8B CE E8 ? ? ? ? 84 C0 74 ? 83 7D ? ? 75 ? F3 0F 10 45 ? F3 0F 11 86 ? ? ? ? 5E 5D C2 ? ? CC CC CC CC CC CC CC CC CC CC 55 8B EC 8B 45"));
		notify_on_layer_change_sequence = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 ? 85 C0 74 ? 80 B9 ? ? ? ? ? 74 ? 8B 89"));
		accumulate_layers = memory::get_pattern(client_dll, CXOR("55 8B EC 57 8B F9 8B 0D ? ? ? ? 8B 01 8B 80"));
		on_latch_interpolated_variables = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 53 56 8B F1 57 80 BE ? ? ? ? ? 75 ? 8B 06"));
		interpolate = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 56 8B F1 57 83 BE ? ? ? ? ? 75 ? 8B 46 ? 8D 4E ? FF 50 ? 85 C0 74 ? 8B CE E8 ? ? ? ? 8B 9E"));
		interpolate_player = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 56 8B F1 83 BE ? ? ? ? ? 0F 85"));
		reset_latched = memory::get_pattern(client_dll, CXOR("56 8B F1 57 8B BE ? ? ? ? 85 FF 74 ? 8B CF E8 ? ? ? ? 68"));

#ifdef LEGACY
		view_render = memory::get_pattern(client_dll, CXOR("8B 0D ? ? ? ? 8B 01 FF 50 4C 8B 06")).add(XORN(2));
		glow_object_manager = memory::get_pattern(client_dll, CXOR("A1 ? ? ? ? A8 01 75 4B"));

		return_addr_loadout_allowed = memory::get_pattern(client_dll, CXOR("84 C0 75 04 B0 01 5F"));
		using_static_prop_debug = memory::get_pattern(engine_dll, CXOR("8B 0D ? ? ? ? 81 F9 ? ? ? ? 75 ? A1 ? ? ? ? 35 ? ? ? ? EB ? 8B 01 FF 50 ? 83 F8 ? 0F 85 ? ? ? ? 8B 0D"));
		build_transformations = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B 75 ? 57 FF 75 ? 8B F9 56"));
		update_postscreen_effects = memory::get_pattern(client_dll, CXOR("55 8B EC 51 53 56 8B 35 ? ? ? ? 57 8B F9 85 F6 74"));
		get_sequence_activity = memory::get_pattern(client_dll, CXOR("55 8B EC 83 7D 08 FF 56 8B F1 74 3D"));
		modify_eye_position = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60"));
		init_key_values = memory::get_pattern(client_dll, CXOR("8B 0E 33 4D FC 81 E1 ? ? ? ? 31 0E 88 46 03 C1 F8 08 66 89 46 12 8B C6")).sub(XORN(0x45));
		want_reticle_shown = memory::get_pattern(client_dll, CXOR("53 56 57 8B 3D ? ? ? ? 8B F1 85 FF"));
		update_all_viewmodel_addons = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 8B D9 56 57 8B 03 FF 90 ? ? ? ? 8B F8 89 7C 24 ? 85 FF 0F 84 ? ? ? ? 8B 17 8B CF"));

		get_viewmodel = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 8B D9 56 57 8B 03 FF 90 ? ? ? ? 8B F8 89 7C 24 ? 85 FF 0F 84 ? ? ? ? 8B 17 8B CF"));
		calc_view = memory::get_pattern(client_dll, CXOR("55 8B EC 53 8B 5D ? 56 57 FF 75 ? 8B F1"));
		get_hud_ptr = memory::get_pattern(client_dll, CXOR("B9 ? ? ? ? 0F 94 C0 0F B6 C0 50 68")).add(XORN(1));
		draw_fog = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 0D ? ? ? ? 83 EC 0C 8B 01 53"));
		cl_move = memory::get_pattern(engine_dll, CXOR("55 8B EC 81 EC ? ? ? ? 53 56 57 8B 3D ? ? ? ? 8A"));
		clear_killfeed = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 68 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 8B F0 85 F6 74 19")).relative(XORN(1));
		physics_simulate = memory::get_pattern(client_dll, CXOR("56 8B F1 8B 8E ? ? ? ? 83 F9 ? 74 ? 0F B7 C1 C1 E0 ? 05 ? ? ? ? C1 E9 ? 39 48 ? 75 ? 8B 08 85 C9 74 ? 8B 01 FF 90 ? ? ? ? A1"));
		send_datagram = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 89 7C 24 18"));
		list_leaves_in_box = memory::get_pattern(client_dll, CXOR("FF 52 18 8B 7D 08 8B")).add(XORN(3));
		calc_viewmodel_view = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 64 56 57"));
		return_addr_cam_think = memory::get_pattern(client_dll, CXOR("85 C0 75 30 38 86"));
		send_net_msg = memory::get_pattern(engine_dll, CXOR("55 8B EC 56 8B F1 8B 86 ? ? ? ? 85 C0 74 ? 48 83 F8 ? 77 ? 83 BE ? ? ? ? ? 8D 8E ? ? ? ? 74 ? 32 C0 84 C0 EB ? E8 ? ? ? ? 84 C0 EB ? 83 BE ? ? ? ? ? 0F 94 C0 84 C0 74 ? B0 ? 5E 5D C2 ? ? 53"));
		return_addr_process_input = memory::get_pattern(client_dll, CXOR("84 C0 74 ? 68 ? ? ? ? 8D 8C 24"));
		host_shutdown = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? A0"));
		destruct_voice_data_message = memory::get_pattern(engine_dll, CXOR("E8 ? ? ? ? 5E 8B E5 5D C3 CC CC CC CC CC CC CC CC CC CC CC CC 55 8B EC 83 E4 ? 51")).relative(XORN(1));
		msg_voice_data = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 ? A1 ? ? ? ? 81 EC ? ? ? ? 53 56 8B F1 B9 ? ? ? ? 57 FF 50 ? 8B 7D"));
		physics_run_think = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 53 56 57 8B F9 8B 87 ? ? ? ? C1 E8"));
		think = memory::get_pattern(client_dll, CXOR("55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B C6"));
		post_think_physics = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 8B D9 56 57 83 BB"));
		simulate_player_simulated_entities = memory::get_pattern(client_dll, CXOR("56 8B F1 57 8B BE ? ? ? ? 83 EF ? 78 ? 90"));
		game_rules = memory::get_pattern(client_dll, CXOR("A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 B8 ? ? ? ? ? 0F 84 ? ? ? ? 0F 10 05")).add(XORN(1));

		add_activity_modifier = memory::get_pattern(server_dll, CXOR("55 8B EC 8B 55 08 83 EC 30 56 8B F1 85 D2 0F 84 ? ? ? ? 8D 45 D0 8B C8 2B D1"));
		get_weapon_prefix = memory::get_pattern(client_dll, CXOR("53 56 8B F1 57 33 FF 8B 4E ? 8B 01"));
		find_mapping = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8B 17"));
		select_sequence_from_mods = memory::get_pattern(server_dll, CXOR("55 8B EC 83 EC ? 53 56 8B 75 ? 8B D9 57 89 5D ? 8B 16"));
		get_sequence_desc = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B 75 ? 57 8B F9 85 F6 78 ? 8B 47"));
		lookup_sequence = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B F1 83 BE ? ? ? ? ? 75 ? 8B 46 ? 8D 4E ? FF 50 ? 85 C0 74 ? 8B CE E8 ? ? ? ? 8B B6 ? ? ? ? 85 F6 74 ? 83 3E ? 74 ? 8B CE E8 ? ? ? ? 84 C0 74 ? FF 75"));
		get_sequence_linear_motion = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 56 8B F1 57 8B FA 85 F6 75 ? 68"));
		update_layer_order_preset = memory::get_pattern(client_dll, CXOR("55 8B EC 51 53 56 57 8B F9 83 7F ? ? 0F 84"));

		ik_ctx_construct = memory::get_pattern(client_dll, CXOR("56 8B F1 6A 00 6A 00 C7"));
		ik_ctx_destruct = memory::get_pattern(client_dll, CXOR("56 8B F1 57 8D 8E ? ? ? ? E8 ? ? ? ? 8D 8E ? ? ? ? E8 ? ? ? ? 83 BE ? ? ? ? ?"));
		ik_ctx_init = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F"));
		ik_ctx_update_targets = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 8B 47 FC 8D 4F FC F3 0F 10 44 24")).relative(XORN(1));
		ik_ctx_solve_dependencies = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 8B 44 24 40 8B 4D 0C")).relative(XORN(1));
		bone_setup_init_pose = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 10 53 8B D9 89 55 F8 56 57 89 5D F4 8B 0B 89 4D F0"));
		accumulate_pose = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1"));
		bone_setup_calc_autoplay_sequences = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 10 53 56 57 8B 7D 10 8B D9 F3 0F 11 5D ?"));
		bone_setup_calc_bone_adjust = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89 54 24 04 89 44 24 2C 56 57 8B ?"));

		show_and_update_selection = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? A1 ? ? ? ? F3 0F 10 40 ? C6 83")).relative(XORN(1));

		view_render_beams = memory::get_pattern(client_dll, CXOR("B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9")).add(XORN(1));
		return_addr_show_radar = memory::get_pattern(client_dll, CXOR("84 C0 75 ? 8B CB E8 ? ? ? ? 84 C0 75 ? 83 BF"));
		calc_roaming_view = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8D 4C 24"));
		return_addr_post_process = memory::get_pattern(client_dll, CXOR("85 C0 75 ? E8 ? ? ? ? 8B C8 E8 ? ? ? ? 32 DB"));
		cache_sequences = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 56 8B F1 57 8B 46 ? 85 C0 75"));
		setup_weapon_action = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 56 8B F1 57 8B 46 ? 85 C0 75"));
		notify_on_layer_change_cycle = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 ? 85 C0 74 ? 80 B9 ? ? ? ? ? 74 ? 56 8B B1 ? ? ? ? 85 F6 74 ? 8D 4D ? 51 50 8B CE E8 ? ? ? ? 84 C0 74 ? 83 7D ? ? 75 ? F3 0F 10 45 ? F3 0F 11 86 ? ? ? ? 5E 5D C2 ? ? CC CC CC CC CC CC CC CC CC CC 55 8B EC A1"));

#else
		view_render = memory::get_pattern(client_dll, CXOR("FF 50 14 E8 ? ? ? ? 5F")).sub(XORN(7));
		glow_object_manager = memory::get_pattern(client_dll, CXOR("0F 11 05 ? ? ? ? 83 C8 01")).add(XORN(3));

		return_addr_loadout_allowed = memory::get_pattern(client_dll, CXOR("84 C0 75 05 B0"));
		using_static_prop_debug = memory::get_pattern(engine_dll, CXOR("8B 0D ? ? ? ? 81 F9 ? ? ? ? 75 ? F3 0F 10 05 ? ? ? ? 0F 2E 05 ? ? ? ? 8B 15 ? ? ? ? 9F F6 C4 ? 7A ? 39 15 ? ? ? ? 75 ? A1 ? ? ? ? 33 05 ? ? ? ? A9 ? ? ? ? 74 ? 8B 0D ? ? ? ? 85 C9 74 ? 8B 01 68 ? ? ? ? FF 90 ? ? ? ? 8B 15 ? ? ? ? 8B 0D ? ? ? ? 81 F2 ? ? ? ? EB ? 8B 01 FF 50 ? 8B 0D ? ? ? ? 8B D0 83 FA ? 0F 85"));
		build_transformations = memory::get_pattern(client_dll, CXOR("55 8B EC 53 56 57 FF 75 ? 8B 7D"));
		update_postscreen_effects = memory::get_pattern(client_dll, CXOR("55 8B EC 51 53 56 57 8B F9 8B 4D 04 E8 ? ? ? ? 8B"));
		get_sequence_activity = memory::get_pattern(client_dll, CXOR("55 8B EC 53 8B 5D 08 56 8B F1 83"));
		modify_eye_position = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 70 56 57 8B F9 89 7C 24 14 83 7F 60"));
		init_key_values = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 8B F0 EB 02 33 F6 8B 45 08 8B 78")).relative(XORN(1));
		alloc_key_values_engine = memory::get_pattern(engine_dll, CXOR("85 C0 74 0F 51 6A 00 56 8B C8 E8 ? ? ? ? 8B F0"));
		alloc_key_values_client = memory::get_pattern(client_dll, CXOR("85 C0 74 10 6A 00 6A 00 56 8B C8 E8 ? ? ? ? 8B F0"));
		on_bbox_change_callback = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 10 F3 0F 10 81"));
		want_reticle_shown = memory::get_pattern(client_dll, CXOR("55 8B EC 53 56 8B F1 8B 4D ? 57 E8"));
		update_all_viewmodel_addons = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 83 EC 2C 53 8B D9 56 57 8B"));

		get_viewmodel = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 08 53 8B D9 56 8B 84 83 ? ? ? ? 83 F8 FF 74 1A 0F B7 F0"));
		calc_view = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 14 53 56 57 FF 75 18"));
		get_hud_ptr = memory::get_pattern(client_dll, CXOR("B9 ? ? ? ? E8 ? ? ? ? 8B 5D 08")).add(XORN(1));
		draw_fog = memory::get_pattern(client_dll, CXOR("53 56 8B F1 8A DA 8B 0D"));
		cl_move = memory::get_pattern(engine_dll, CXOR("55 8B EC 81 EC 64 01 00 00 53 56 8A F9"));
		clear_killfeed = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 0C 53 56 8B 71 58"));
		physics_simulate = memory::get_pattern(client_dll, CXOR("56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 23"));
		clamp_bones_in_bbox = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 80 BE ? ? ? ? ? 0F 84 ? ? ? ? 83 BE ? ? ? ? ? 0F 84 ? ? ? ? B8")).relative(XORN(1));
		clear_hud_weapons = memory::get_pattern(client_dll, CXOR("55 8B EC 51 53 56 8B 75 08 8B D9 57 6B FE 34"));
		send_datagram = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 57 8B F9 89 7C 24 14"));
		list_leaves_in_box = memory::get_pattern(client_dll, CXOR("FF 50 18 89 44 24 14 EB")).add(XORN(3));
		calc_viewmodel_view = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 58 56 57"));
		return_addr_cam_think = memory::get_pattern(client_dll, CXOR("85 C0 75 30 38 87"));
		send_net_msg = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 EC 08 56 8B F1 8B 4D 04"));
		return_addr_process_input = memory::get_pattern(client_dll, CXOR("84 C0 74 ? 8D 8E ? ? ? ? E8 ? ? ? ? B9"));
		viewmodel_arm_cfg = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 89 87 ? ? ? ? 6A")).relative(XORN(1));
		trace_filter_to_head_collision = memory::get_pattern(client_dll, CXOR("55 8B EC 83 B9 ? ? ? ? ? 75 0F"));
		mask_ptr = memory::get_pattern(client_dll, CXOR("FF 35 ? ? ? ? FF 90 ? ? ? ? 8B 8F")).add(XORN(2));
		update_addon_models = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 53 8B D9 8D 45 ? 8B 08"));
		host_shutdown = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 ? 56 8B 35"));
		destruct_voice_data_message = memory::get_pattern(engine_dll, CXOR("E8 ? ? ? ? 5E 8B E5 5D C3 CC CC CC CC CC CC CC CC CC CC CC CC 51")).relative(XORN(1));
		msg_voice_data = memory::get_pattern(engine_dll, CXOR("55 8B EC 83 E4 F8 A1 ? ? ? ? 81 EC 84 01 00"));
		physics_run_think = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 10 53 56 57 8B F9 8B 87"));
		think = memory::get_pattern(client_dll, CXOR("55 8B EC 56 57 8B F9 8B B7 ? ? ? ? 8B"));
		post_think_physics = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB ? ? ? ? ? 0F 84"));
		simulate_player_simulated_entities = memory::get_pattern(client_dll, CXOR("56 8B F1 57 8B BE ? ? ? ? 83 EF 01 78 74"));
		game_rules = memory::get_pattern(client_dll, CXOR("A1 ? ? ? ? 85 C0 0F 84 ? ? ? ? 80 B8 ? ? ? ? ? 74 7A")).add(XORN(1));
		return_addr_send_datagram_cl_move = memory::get_pattern(engine_dll, CXOR("6A 00 8B 01 FF 90 B8 00 00 00 83 BF 08 01")).add(XORN(0xA));
		fire_bullet = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? F3 0F 7E 45"));

		add_activity_modifier = memory::get_pattern(server_dll, CXOR("55 8B EC 8B 55 08 83 EC 30 56 8B F1 85 D2 0F 84 ? ? ? ? 8D 45 D0 8B C8 2B D1"));
		get_weapon_prefix = memory::get_pattern(client_dll, CXOR("53 56 57 8B F9 33 F6 8B 4F ? 8B 01 FF 90 ? ? ? ? 89 47"));
		find_mapping = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8B 17"));
		select_sequence_from_mods = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 56 8B 75 ? 8B D9 57 89 5C 24 ? 8B 16"));
		get_sequence_desc = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B 75 ? 57 8B F9 85 F6 78 ? 8B 47"));
		lookup_sequence = memory::get_pattern(client_dll, CXOR("55 8B EC 56 8B F1 83 BE ? ? ? ? ? 75 ? 8B 46 ? 8D 4E ? FF 50 ? 85 C0 74 ? 8B CE E8 ? ? ? ? 8B B6 ? ? ? ? 85 F6 74 ? 83 3E ? 74 ? 8B CE E8 ? ? ? ? 84 C0 74 ? FF 75"));
		get_sequence_linear_motion = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 56 8B F1 57 8B FA 85 F6 75 ? 68"));
		update_layer_order_preset = memory::get_pattern(client_dll, CXOR("55 8B EC 51 53 56 57 8B F9 83 7F ? ? 0F 84"));

		ik_ctx_construct = memory::get_pattern(client_dll, CXOR("53 8B D9 F6 C3 03 74 0B FF 15 ? ? ? ? 84 C0 74 01 ? C7 83 ? ? ? ? ? ? ? ? 8B CB"));
		ik_ctx_destruct = memory::get_pattern(client_dll, CXOR("56 8B F1 57 8D 8E ? ? ? ? E8 ? ? ? ? 8D 8E ? ? ? ? E8 ? ? ? ? 83 BE ? ? ? ? ?"));
		ik_ctx_init = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D 8F"));
		ik_ctx_update_targets = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 8B 47 FC 8D 4F FC F3 0F 10 44 24")).relative(XORN(1));
		ik_ctx_solve_dependencies = memory::get_pattern(client_dll, CXOR("E8 ? ? ? ? 8B 44 24 40 8B 4D 0C")).relative(XORN(1));
		bone_setup_init_pose = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 10 53 8B D9 89 55 F8 56 57 89 5D F4 8B 0B 89 4D F0"));
		accumulate_pose = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1"));
		bone_setup_calc_autoplay_sequences = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC 10 53 56 57 8B 7D 10 8B D9 F3 0F 11 5D"));
		bone_setup_calc_bone_adjust = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89 54 24 04 89 44 24 2C 56 57 8B"));
		note_prediction_error = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 56 8B F1 8B 06 8B 80 ? ? ? ? FF D0 84 C0 75"));
		check_moving_ground = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 56 8B 75 ? F6 86"));
	
		view_render_beams = memory::get_pattern(client_dll, CXOR("B9 ? ? ? ? A1 ? ? ? ? FF 10 A1 ? ? ? ? B9")).add(XORN(1));
		return_addr_show_radar = memory::get_pattern(client_dll, CXOR("84 C0 0F 85 ? ? ? ? 8B CB E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 83 BF"));
		calc_roaming_view = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 8B D9 8D 4C 24"));
		return_addr_post_process = memory::get_pattern(client_dll, CXOR("85 D2 75 ? E8 ? ? ? ? 8B C8"));

		notify_on_layer_change_cycle = memory::get_pattern(client_dll, CXOR("55 8B EC 8B 45 ? 85 C0 74 ? 80 B9 ? ? ? ? ? 74 ? 56 8B B1 ? ? ? ? 85 F6 74 ? 8D 4D ? 51 50 8B CE E8 ? ? ? ? 84 C0 74 ? 83 7D ? ? 75 ? F3 0F 10 45 ? F3 0F 11 86 ? ? ? ? 5E 5D C2 ? ? CC CC CC CC CC CC CC CC CC CC 55 8B EC 53"));

		setup_weapon_action = memory::get_pattern(client_dll, CXOR("55 8B EC 51 53 56 57 8B F9 8B 77 ? 83 BE"));
		setup_whole_body_action = memory::get_pattern(client_dll, CXOR("55 8B EC 83 EC ? 56 57 8B F9 8B 77"));
		setup_flinch = memory::get_pattern(client_dll, CXOR("55 8B EC 51 56 8B 71 ? 83 BE ? ? ? ? ? 0F 84 ? ? ? ? 8B B6 ? ? ? ? 81 C6 ? ? ? ? 0F 84 ? ? ? ? F3 0F 10 56 ? 0F 28 C2 E8 ? ? ? ? 0F 57 DB 0F 2F D8 73 ? F3 0F 10 49 ? F3 0F 10 66 ? F3 0F 59 CA F3 0F 10 15"));
		cache_sequences = memory::get_pattern(client_dll, CXOR("55 8B EC 83 E4 ? 83 EC ? 53 56 8B F1 57 8B 46"));

#endif
	}
}