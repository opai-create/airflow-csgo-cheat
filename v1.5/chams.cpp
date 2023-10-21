#include "globals.hpp"
#include "chams.hpp"
#include "animations.hpp"

constexpr auto MODEL_BLEND_FACTOR = 0.5f;

INLINE i_material* create_material(const char* material_name, const char* material_data, const char* shader_type)
{
	c_key_values* key_values = new c_key_values;

#ifdef LEGACY
	offsets::init_key_values.cast<void(__thiscall*)(void*, const char*)>()(key_values, shader_type);
	offsets::load_from_buffer.cast<void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*)>()(key_values, material_name, material_data, NULL, NULL, NULL);
#else
	offsets::init_key_values.cast<void(__thiscall*)(void*, const char*, int, int)>()(key_values, shader_type, NULL, NULL);
	offsets::load_from_buffer.cast<void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*, void*)>()(key_values, material_name, material_data, NULL, NULL, NULL, NULL);
#endif
	auto material = HACKS->material_system->create_material(material_name, key_values);
	material->increment_reference_count();

	return material;
}

void c_chams::init_materials()
{
	materials[0] = create_material(CXOR("csgo_soft"), CXOR(R"#("VertexLitGeneric"
	{
		"$basetexture" "vgui/white"
		"$nofog" "1"
		"$model" "1"
		"$nocull" "0"
		"$selfillum" "1"
		"$halflambert" "1"
		"$znearer" "0"
		"$flat" "1"
		"$ingorez" "1"
	}
	)#"),
	CXOR("VertexLitGeneric"));

	materials[1] = create_material(CXOR("csgo_shaded"), CXOR(R"#("VertexLitGeneric"
	{
		"$basetexture" "vgui/white"
		"$phong" "1"
		"$phongexponent" "0"
		"$phongboost" "0"
		"$rimlight" "1"
		"$rimlightexponent" "0"
		"$rimlightboost" "0"
		"$model" "1"
		"$nocull" "0"
		"$nofog" "0"
		"$ingorez" "1"
		"$halflambert" "0"
		"$basemapalphaphongmask" "1"
	}
	)#"),
	CXOR("VertexLitGeneric"));

	materials[2] = create_material(CXOR("csgo_flat"), CXOR(R"#("UnlitGeneric"
	{
		"$basetexture" "vgui/white"
		"$ignorez" "0"
		"$envmap" ""
		"$nofog" "1"
		"$model" "1"
		"$nocull" "0"
		"$selfillum" "1"
		"$halflambert" "1"
		"$znearer" "0"
		"$flat" "1"
	}
	)#"),
	CXOR("UnlitGeneric"));

	materials[3] = create_material(CXOR("csgo_shiny"), CXOR(R"#("VertexLitGeneric"
	{
		"$basetexture" "vgui/white"
		"$phong" "1"
		"$BasemapAlphaPhongMask" "1"
		"$phongexponent" "15"
		"$normalmapalphaenvmask" "1"
		"$envmap" "env_cubemap"
		"$envmaptint" "[0.6 0.6 0.6]"
		"$phongboost" "[0.6 0.6 0.6]"
		"$phongfresnelranges" "[0.5 0.5 1.0]"
		"$nofog" "1"
		"$ingorez" "1"
		"$model" "1"
		"$nocull" "0"
		"$selfillum" "1"
		"$halflambert" "1"
		"$znearer" "0"
		"$flat" "1"
	}
	)#"),
	CXOR("VertexLitGeneric"));

	materials[4] = create_material(CXOR("csgo_glow"), CXOR(R"#("VertexLitGeneric"
	{
		"$additive" "1"
		"$envmap" "models/effects/cube_white"
		"$envmaptint" "[1 1 1]"
		"$envmapfresnel" "1"
		"$envmapfresnelminmaxexp" "[0 1 2]"
		"$alpha" "1"
		"$nofog" "1 
		"$ingorez" "1"
	}
	)#"),
	CXOR("VertexLitGeneric"));

	materials[5] = create_material(CXOR("csgo_bubble"), CXOR(R"#("VertexLitGeneric"
	{
		"$basetexture" "vgui/white"
		"$additive" "1"
		"$nofog" "1"
		"$ingorez" "1"

		"$basemapalphaphongmask" "1"

		"$envmap" "env_cubemap"
		"$envmapfresnel" "1"
		"$envmaptint" "[.2 .2 .2]"
	}
	)#"),
	CXOR("VertexLitGeneric"));

	materials[6] = create_material(CXOR("csgo_world"), CXOR(R"#("VertexLitGeneric"
	{
		"$baseTexture"            "models\weapons\customization\paints\custom\money"
		"$basemapalphaphongmask"  "1"

		"$envmap"                 "env_cubemap"
		"$envmapfresnel"          "1"
		"$envmaptint" 	          "[.2 .2 .2]"
	}
	)#"),
	CXOR("VertexLitGeneric"));

	materials[7] = create_material(CXOR("csgo_fadeup"), CXOR(R"#("VertexLitGeneric"
	{
		"$baseTexture"            "vgui/white"
		"$basemapalphaphongmask"  "1"

		"$envmap"                 "env_cubemap"
		"$envmapfresnel"          "1"
		"$envmaptint" 	          "[.2 .2 .2]"
	}
	)#"),
	CXOR("VertexLitGeneric"));

	tye_dye = create_material(CXOR("csgo_tye_dye"), CXOR(R"#("VertexLitGeneric"
      {
        "$basetexture" "vgui/white"
        "$ignorez" "1"
        "$model" "1"
        "$linearwrite" "1"
        "$envmap" "models/effects/cube_white"
        "$envmaptint" "[1 1 1]"
        "$envmapfresnel" "1"
        "$envmapfresnelminmaxexp"    "[0 1 2]"
        "$alpha" "1"
        "$nofog" "1"
      }
    )#"), CXOR("VertexLitGeneric"));
}

bool c_chams::draw_model(chams_t& chams, matrix3x4_t* matrix, float alpha, bool xqz)
{
	if (!chams.enable)
		return false;

	auto& main_color = chams.main_color;
	auto& glow_color = chams.glow_color;

	float old_blend = HACKS->render_view->get_blend();
	float old_colors[3] = {};
	HACKS->render_view->get_color_modulation(old_colors);

	auto draw_model = [&](c_float_color& clr, i_material* mat, matrix3x4_t* matrix = nullptr)
	{
		mat->increment_reference_count();

		mat->set_material_var_flag(MATERIAL_VAR_TRANSLUCENT, true);
		mat->set_material_var_flag(MATERIAL_VAR_NOFOG, true);
		mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, xqz);

		float colors[3]{ clr[0], clr[1], clr[2] };

		HACKS->render_view->set_blend(clr[3] * alpha);
		HACKS->render_view->set_color_modulation(colors);

		HACKS->model_render->forced_material_override(mat);
		hook_data.call_original(matrix);
		HACKS->model_render->forced_material_override(nullptr);

		HACKS->render_view->set_blend(old_blend);
		HACKS->render_view->set_color_modulation(old_colors);
	};

	if (chams.material == 4)
	{
		{
			auto flat_chams = materials[2];
			draw_model(main_color, flat_chams, matrix);
		}
		{
			float fill_amount = std::clamp((chams.glow_fill / 100.f) - 0.15f, 0.f, 1.f);

			auto glow = materials[4];

			auto var = glow->find_var(CXOR("$envmaptint"), nullptr);
			if (var)
				var->set_vec_value({ glow_color[0], glow_color[1], glow_color[2] });

			auto var2 = glow->find_var(CXOR("$envmapfresnelminmaxexp"), nullptr);
			if (var2)
				var2->set_vec_value({ 0.f, 1.f, 5.f - (5.f * fill_amount) });

			draw_model(glow_color, glow, matrix);
		}
		return true;
	}
	else
	{
		auto material = materials[chams.material];

		if (chams.material == 7)
		{
			auto temp_material = materials[7];

			auto var1 = temp_material->find_var(CXOR("$envmaptint"), nullptr);
			if (var1)
				var1->set_vec_value({ main_color[0], main_color[1], main_color[2] });

			temp_material->color_modulate(0.f, 0.f, 0.f);

			draw_model(main_color, temp_material, matrix);
		}
		else if (chams.material == 6 && std::strlen(chams.chams_sprite) > 0)
		{
			static auto temp_material = materials[6];

			auto var = temp_material->find_var(CXOR("$basetexture"), nullptr);
			if (var && old_base_texture != chams.chams_sprite)
			{
				std::ostringstream text_stream;
				text_stream << XOR(R"("VertexLitGeneric" 
				    {
						"$baseTexture")")
					<< chams.chams_sprite
					<< XOR(R"(
						"$basemapalphaphongmask"  "1"

						"$envmap"                 "env_cubemap"
						"$envmapfresnel"          "1"
						"$envmaptint" 	          "[.2 .2 .2]"
					}
				)");

				temp_material = create_material(CXOR("csgo_sprite_material"), text_stream.str().c_str(),
					CXOR("VertexLitGeneric"));

				var->set_vec_value(chams.chams_sprite);
				old_base_texture = chams.chams_sprite;
			}

			auto var1 = temp_material->find_var(CXOR("$envmaptint"), nullptr);
			if (var1)
				var1->set_vec_value({ main_color[0], main_color[1], main_color[2] });

			draw_model(main_color, temp_material, matrix);
		}
		else
		{
			auto var = material->find_var(CXOR("$envmaptint"), nullptr);
			if (var)
				var->set_vec_value({ main_color[0], main_color[1], main_color[2] });

			draw_model(main_color, material, matrix);
		}
		return true;
	}

	return false;
}

bool c_chams::should_draw()
{
	if (!HACKS->in_game || !HACKS->local)
		return false;

	auto renderable = (c_renderable*)hook_data.state.renderable;
	if (!renderable)
		return false;

	auto unknown = renderable->get_i_unknown_entity();
	if (!unknown)
		return false;

	auto entity = (c_base_entity*)unknown->get_client_entity();
	if (!entity)
		return false;

	auto model = hook_data.info.model;
	if (!model)
		return false;

	auto client_class = entity->get_client_class();
	if (!client_class)
		return false;

	if (std::strstr(model->name, CXOR("models/player")))
	{
		auto player = (c_cs_player*)entity;
		if (!player)
			return false;

		if (client_class->class_id == CCSRagdoll && !player->is_teammate(false))
			return this->draw_model(g_cfg.visuals.chams[c_ragdolls], nullptr, 1.f, true);

		if (!player->is_player())
			return false;

		if (!player->is_alive() || player->has_gun_game_immunity() || player->dormant())
			return false;

		if (player == HACKS->local)
		{
			if (!HACKS->input->camera_in_third_person)
				return false;

			auto alpha = 1.f;
			if (HACKS->local->is_scoped() || HACKS->weapon && HACKS->weapon->is_grenade())
				alpha *= MODEL_BLEND_FACTOR;

			HACKS->render_view->set_blend(alpha);
			return this->draw_model(g_cfg.visuals.chams[c_local], nullptr, alpha);
		}
		else
		{
			if (player->is_teammate(false))
				return false;

			if (HACKS->local == player)
				return false;

			if (HACKS->local->is_alive() && !HACKS->cl_lagcomp0 && g_cfg.visuals.chams[c_history].enable) 
			{
				auto anims = ANIMFIX->get_anims(player->index());
				if (anims)
				{
					auto& records = anims->records;
					if (!records.empty())
					{
						if (g_cfg.visuals.show_all_history)
						{
							for (int i = 1; i <= records.size(); ++i)
							{
								auto record = &records[i - 1];
								if (!record || !record->valid_lc)
									continue;

								float alpha = (float)i / (float)records.size();
								this->draw_model(g_cfg.visuals.chams[c_history], record->matrix_orig.matrix, std::lerp(1.f, 0.f, alpha), true);
							}
						}
						else
						{
							auto last_find = std::find_if(records.rbegin(), records.rend(), [&](anim_record_t& record) {
								return record.valid_lc;
								});

							anim_record_t* last = nullptr;
							if (last_find != records.rend())
								last = &*last_find;

							if (last)
							{
								auto dist = std::clamp(last->origin.dist_to(player->origin()) / 30.f, 0.f, 1.f);
								this->draw_model(g_cfg.visuals.chams[c_history], last->matrix_orig.matrix, dist, true);
							}
						}
					}
				}
			}

			bool xqz = this->draw_model(g_cfg.visuals.chams[c_xqz], nullptr, 1.f, true);
			bool vis = this->draw_model(g_cfg.visuals.chams[c_vis]);
			return vis || xqz;
		}
	}
	else if (client_class->class_id == CPredictedViewModel && HACKS->local->is_alive())
		return this->draw_model(g_cfg.visuals.chams[c_wpn]);
	else if (std::strstr(model->name, CXOR("arms")) && HACKS->local->is_alive())
		return this->draw_model(g_cfg.visuals.chams[c_viewmodel]);
	else if (HACKS->local->is_alive())
	{
		auto entity_move_parent = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(entity->move_parent()));
		if (entity_move_parent != HACKS->local)
			return false;

		if (!HACKS->input->camera_in_third_person)
			return false;

		float additional_amt = g_cfg.misc.attachments_amt * 0.01f;

		auto alpha = 1.f * additional_amt;
		if (HACKS->local->is_scoped() || HACKS->weapon && HACKS->weapon->is_grenade())
			alpha *= MODEL_BLEND_FACTOR;

		HACKS->render_view->set_blend(alpha);
		return this->draw_model(g_cfg.visuals.chams[c_attachments], nullptr, alpha);
	}

	return false;
}

void c_chams::draw_shot_records()
{
	if (shots.empty() || !HACKS->in_game || !HACKS->local)
		return;

	auto ctx = HACKS->material_system->get_render_context();
	if (!ctx)
		return;

	auto& on_shot = g_cfg.visuals.chams[c_onshot];
	if (!on_shot.enable)
		return;

	auto& main_color = on_shot.main_color;
	auto& glow_color = on_shot.glow_color;

	auto& data = shots.front();

	auto ent = HACKS->entity_list->get_client_entity(data.info.entity_index);
	if (!ent)
	{
		shots.pop_front();
		return;
	}

	float diff = std::clamp(HACKS->global_vars->realtime - data.time, 0.f, (float)on_shot.shot_duration);
	if (diff >= (float)on_shot.shot_duration)
	{
		data.alpha -= RENDER->get_animation_speed() * 1.5f;

		if (data.alpha <= 0.f)
		{
			shots.pop_front();
			return;
		}
	}

	float old_blend = HACKS->render_view->get_blend();
	float old_colors[3] = {};
	HACKS->render_view->get_color_modulation(old_colors);

	auto draw_model = [&](c_float_color& clr, i_material* mat)
	{
		mat->increment_reference_count();

		mat->set_material_var_flag(MATERIAL_VAR_TRANSLUCENT, true);
		mat->set_material_var_flag(MATERIAL_VAR_NOFOG, true);
		mat->set_material_var_flag(MATERIAL_VAR_IGNOREZ, true);

		float colors[3]{ clr[0], clr[1], clr[2] };

		HACKS->render_view->set_blend(clr[3] * data.alpha);
		HACKS->render_view->set_color_modulation(colors);

		HACKS->model_render->forced_material_override(mat);
		hook_data.original(HACKS->model_render, 0, ctx, data.state, data.info, data.bones);
		HACKS->model_render->forced_material_override(nullptr);

		HACKS->render_view->set_blend(old_blend);
		HACKS->render_view->set_color_modulation(old_colors);
	};

	if (on_shot.material == 4)
	{
		{
			auto flat_chams = materials[2];
			draw_model(main_color, flat_chams);
		}
		{
			float fill_amount = std::clamp((on_shot.glow_fill / 100.f) - 0.15f, 0.f, 1.f);

			auto glow = materials[4];

			auto var = glow->find_var(CXOR("$envmaptint"), nullptr);
			if (var)
				var->set_vec_value({ glow_color[0], glow_color[1], glow_color[2] });

			auto var2 = glow->find_var(CXOR("$envmapfresnelminmaxexp"), nullptr);
			if (var2)
				var2->set_vec_value({ 0.f, 1.f, 5.f - (5.f * fill_amount) });

			draw_model(glow_color, glow);
		}
	}
	else
	{
		auto material = materials[on_shot.material];

		if (on_shot.material == 7)
		{
			auto temp_material = materials[7];

			auto var1 = temp_material->find_var(CXOR("$envmaptint"), nullptr);
			if (var1)
				var1->set_vec_value({ main_color[0], main_color[1], main_color[2] });

			temp_material->color_modulate(0.f, 0.f, 0.f);

			draw_model(main_color, temp_material);
		}
		else if (on_shot.material == 6 && std::strlen(on_shot.chams_sprite) > 0)
		{
			static auto temp_material = materials[6];

			auto var = temp_material->find_var(CXOR("$basetexture"), nullptr);
			if (var && old_base_texture != on_shot.chams_sprite)
			{
				std::ostringstream text_stream;
				text_stream << XOR(R"("VertexLitGeneric" 
				    {
						"$baseTexture")")
					<< on_shot.chams_sprite
					<< XOR(R"(
						"$basemapalphaphongmask"  "1"

						"$envmap"                 "env_cubemap"
						"$envmapfresnel"          "1"
						"$envmaptint" 	          "[.2 .2 .2]"
					})
				)");

				temp_material = create_material(CXOR("csgo_sprite_material"), text_stream.str().c_str(),
					CXOR("VertexLitGeneric"));

				var->set_vec_value(on_shot.chams_sprite);
				old_base_texture = on_shot.chams_sprite;
			}

			auto var1 = temp_material->find_var(CXOR("$envmaptint"), nullptr);
			if (var1)
				var1->set_vec_value({ main_color[0], main_color[1], main_color[2] });

			draw_model(main_color, temp_material);
		}
		else
		{
			auto var = material->find_var(CXOR("$envmaptint"), nullptr);
			if (var)
				var->set_vec_value({ main_color[0], main_color[1], main_color[2] });

			draw_model(main_color, material);
		}
	}
}

void c_chams::add_shot_record(c_cs_player* player, matrix3x4_t* matrix)
{
	auto model = player->get_model();
	if (!model)
		return;

	auto renderable = player->get_renderable_entity();
	if (!renderable)
		return;

	auto hdr = HACKS->model_info->get_studio_model(player->get_model());
	if (!hdr)
		return;

	auto& shot = shots.emplace_front();
	shot.time = HACKS->global_vars->realtime;
	shot.alpha = 1.f;
	shot.origin = player->origin();
	math::memcpy_sse(shot.bones, matrix, sizeof(shot.bones));

	shot.info.origin = player->get_abs_origin();
	shot.info.angles = player->get_abs_angles();

	shot.state.studio_hdr = hdr;
	shot.state.studio_hdr_data = HACKS->model_cache->get_hardware_data(model->studio);
	shot.state.renderable = renderable;
	shot.state.draw_flags = 0;

	shot.info.renderable = renderable;
	shot.info.model = model;
	shot.info.lightning_offset = nullptr;
	shot.info.lightning_origin = nullptr;
	shot.info.hitboxset = player->hitbox_set();
	shot.info.skin = player->skin();
	shot.info.body = player->body();
	shot.info.entity_index = player->index();
	shot.info.instance = memory::get_virtual(renderable, 30).cast<unsigned short(__thiscall*)(void*)>()(renderable);
	shot.info.flags = 0x1;
	shot.state.decals = 0;
	shot.state.lod = 0;
	shot.info.model_to_world = &shot.current_bones;
	shot.state.model_to_world = &shot.current_bones;
	shot.current_bones.angle_matrix(shot.info.angles);
	shot.current_bones.set_origin(shot.info.origin);

	if (shots.size() > 1)
		shots.erase(shots.end() - 1);
}

void c_chams::on_draw_model_execute(dme_original original, void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world)
{
	hook_data.init(original, ecx, edx, ctx, state, info, bone_to_world);

	if (HACKS->client_state->delta_tick == -1 || !this->should_draw())
		original(ecx, edx, ctx, state, info, bone_to_world);
}