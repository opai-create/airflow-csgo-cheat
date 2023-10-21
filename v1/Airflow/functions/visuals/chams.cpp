#include "chams.h"

#include "../features.h"
#include "../ragebot/local_animfix.h"

#include <string>
#include <iostream>
#include <sstream>

__forceinline c_material* create_material(const char* material_name, const char* material_data, const char* shader_type)
{
	c_key_values* key_values = new c_key_values;
	func_ptrs::init_key_values(key_values, shader_type, NULL, NULL);
	func_ptrs::load_from_buffer(key_values, material_name, material_data, NULL, NULL, NULL, NULL);

	c_material* material = interfaces::material_system->create_material(material_name, key_values);
	material->increment_reference_count();

	return material;
}

void c_chams::init_materials()
{
	static bool init = false;
	if (init)
		return;

	materials_list[0] = create_material(xor_c("csgo_soft"), xor_c(R"#("VertexLitGeneric"
			{
				"$basetexture"			"vgui/white"
				"$nofog"						"1"
				"$model"						"1"
				"$nocull"						"0"
				"$selfillum"				"1"
				"$halflambert"			"1"
				"$znearer"					"0"
				"$flat"							"1"
				"$ingorez"					"1"
			}
		)#"),
		xor_c("VertexLitGeneric"));

	materials_list[1] = create_material(xor_c("csgo_shaded"), xor_c(R"#("VertexLitGeneric"
			{
				"$basetexture"			        "vgui/white"
				"$phong"						        "1"
				"$phongexponent"		        "0"
				"$phongboost"					      "0"
				"$rimlight"					        "1"
				"$rimlightexponent"		      "0"
				"$rimlightboost"		        "0"
				"$model"						        "1"
				"$nocull"						        "0"
				"$nofog"						        "0"
				"$ingorez"						      "1"
				"$halflambert"				      "0"
				"$basemapalphaphongmask"		"1"
			}
		)#"),
		xor_c("VertexLitGeneric"));

	materials_list[2] = create_material(xor_c("csgo_flat"), xor_c(R"#("UnlitGeneric"
			{
					  "$basetexture"          "vgui/white"
					  "$ignorez"               "0"
					  "$envmap"                ""
					  "$nofog"                 "1"
					  "$model"                 "1"
					  "$nocull"                "0"
					  "$selfillum"             "1"
					  "$halflambert"           "1"
					  "$znearer"               "0"
					  "$flat"                  "1"
			}
		)#"),
		xor_c("UnlitGeneric"));

	materials_list[3] = create_material(xor_c("csgo_shiny"), xor_c(R"#("VertexLitGeneric"
			{
				"$basetexture"					          "vgui/white"
				"$phong"						              "1"
				"$BasemapAlphaPhongMask"          "1"
				"$phongexponent"				          "15"
				"$normalmapalphaenvmask"		      "1"
				"$envmap"						              "env_cubemap"
				"$envmaptint"					            "[0.6 0.6 0.6]"
				"$phongboost"					            "[0.6 0.6 0.6]"
				"$phongfresnelranges"			        "[0.5 0.5 1.0]"
				"$nofog"						              "1"
				"$ingorez"						            "1"
				"$model"						              "1"
				"$nocull"						              "0"
				"$selfillum"					            "1"
				"$halflambert"					          "1"
				"$znearer"						            "0"
				"$flat"							              "1"
			}
		)#"),
		xor_c("VertexLitGeneric"));

	materials_list[4] = create_material(xor_c("csgo_glow"), xor_c(R"#("VertexLitGeneric"
			{
				"$additive"						        "1"
				"$envmap"						          "models/effects/cube_white"
				"$envmaptint"					        "[1 1 1]"
				"$envmapfresnel"				      "1"
				"$envmapfresnelminmaxexp"		  "[0 1 2]"
				"$alpha"						          "1"
				"$nofog"						          "1"
				"$ingorez"						        "1"
			}
		)#"),
		xor_c("VertexLitGeneric"));

	materials_list[5] = create_material(xor_c("csgo_bubble"), xor_c(R"#("VertexLitGeneric"
			{
				"$basetexture" 			    "vgui/white"
				"$additive"				    "1"
				"$nofog"				    "1"
				"$ingorez"				    "1"

				"$basemapalphaphongmask"  "1"

				"$envmap"                 "env_cubemap"
				"$envmapfresnel"          "1"
				"$envmaptint" 	          "[.2 .2 .2]"
			}
		)#"),
		xor_c("VertexLitGeneric"));

	custom_world = create_material(xor_c("csgo_world"), xor_c(R"#("VertexLitGeneric"
			{
				"$baseTexture"            "models\weapons\customization\paints\custom\money"
				"$basemapalphaphongmask"  "1"

				"$envmap"                 "env_cubemap"
				"$envmapfresnel"          "1"
				"$envmaptint" 	          "[.2 .2 .2]"
			}
		)#"),
		xor_c("VertexLitGeneric"));

	materials_list[7] = create_material(xor_c("csgo_fadeup"), xor_c(R"#("VertexLitGeneric"
			{
				"$baseTexture"            "vgui/white"
				"$basemapalphaphongmask"  "1"

				"$envmap"                 "env_cubemap"
				"$envmapfresnel"          "1"
				"$envmaptint" 	          "[.2 .2 .2]"
			}
		)#"),
		xor_c("VertexLitGeneric"));

	materials_list[6] = custom_world;

	should_init = true;
}

bool c_chams::draw_model(chams_t& chams, matrix3x4_t* matrix, float alpha, bool xqz)
{
	if (!chams.enable)
		return false;

	auto& main_color = chams.main_color;
	auto& glow_color = chams.glow_color;

	float old_blend = interfaces::render_view->get_blend();
	float old_colors[3] = {};
	interfaces::render_view->get_color_modulation(old_colors);

	auto draw_model = [&](c_float_color& clr, c_material* mat, matrix3x4_t* matrix = nullptr)
	{
		mat->increment_reference_count();

		mat->set_material_var_flag(material_var_translucent, true);
		mat->set_material_var_flag(material_var_nofog, true);
		mat->set_material_var_flag(material_var_ignorez, xqz);

		float colors[3]{ clr[0], clr[1], clr[2] };

		interfaces::render_view->set_blend(clr[3] * alpha);
		interfaces::render_view->set_color_modulation(colors);

		interfaces::model_render->forced_material_override(mat);
		hook_data.call_original(matrix);
		interfaces::model_render->forced_material_override(nullptr);

		interfaces::render_view->set_blend(old_blend);
		interfaces::render_view->set_color_modulation(old_colors);
	};

	if (chams.material == 4)
	{
		{
			auto flat_chams = materials_list[2];
			draw_model(main_color, flat_chams, matrix);
		}
		{
			float fill_amount = std::clamp((chams.glow_fill / 100.f) - 0.15f, 0.f, 1.f);

			auto glow = materials_list[4];

			auto var = glow->find_var(xor_c("$envmaptint"), nullptr);
			if (var)
				var->set_vec_value(glow_color[0], glow_color[1], glow_color[2]);

			auto var2 = glow->find_var(xor_c("$envmapfresnelminmaxexp"), nullptr);
			if (var2)
				var2->set_vec_value(0.f, 1.f, 5.f - (5.f * fill_amount));

			draw_model(glow_color, glow, matrix);
		}
		return true;
	}
	else
	{
		auto material = materials_list[chams.material];

		if (chams.material == 7)
		{
			auto temp_material = materials_list[7];

			auto var1 = temp_material->find_var(xor_c("$envmaptint"), nullptr);
			if (var1)
				var1->set_vec_value(main_color[0], main_color[1], main_color[2]);

			temp_material->color_modulate(0.f, 0.f, 0.f);

			draw_model(main_color, temp_material, matrix);
		}
		else if (chams.material == 6 && std::strlen(chams.chams_sprite) > 0)
		{
			static auto temp_material = materials_list[6];

			static std::string old_base_texture = "";

			auto var = temp_material->find_var(xor_c("$basetexture"), nullptr);
			if (var && old_base_texture != chams.chams_sprite)
			{
				std::ostringstream text_stream;
				text_stream << R"("VertexLitGeneric" 
				    {
						"$baseTexture")"
										<< chams.chams_sprite
										<< R"(
						"$basemapalphaphongmask"  "1"

						"$envmap"                 "env_cubemap"
						"$envmapfresnel"          "1"
						"$envmaptint" 	          "[.2 .2 .2]"
					}
				)";

				temp_material = create_material(xor_c("csgo_sprite_material"), text_stream.str().c_str(),
				xor_c("VertexLitGeneric"));

				var->set_vec_value(chams.chams_sprite);
				old_base_texture = chams.chams_sprite;
			}

			auto var1 = temp_material->find_var(xor_c("$envmaptint"), nullptr);
			if (var1)
				var1->set_vec_value(main_color[0], main_color[1], main_color[2]);

			draw_model(main_color, temp_material, matrix);
		}
		else
		{
			auto var = material->find_var(xor_c("$envmaptint"), nullptr);
			if (var)
				var->set_vec_value(main_color[0], main_color[1], main_color[2]);

			draw_model(main_color, material, matrix);
		}
		return true;
	}

	return false;
}

bool c_chams::get_backtrack_matrix(matrix3x4_t* bones, c_csplayer* player, float& alpha)
{
	if (!g_ctx.local->is_alive())
		return false;

	alpha = 1.f;

	if (g_cfg.rage.enable)
	{
		auto record = g_animation_fix->get_oldest_record(player);
		if (!record)
			return false;

		memcpy_fast(bones, record->sim_orig.bone, sizeof(matrix3x4_t) * 128);

		float distance = record->origin.dist_to(player->get_abs_origin());
		if (distance <= 25.f)
			alpha = std::clamp(distance / 25.f, 0.f, 1.f);

		return alpha > 0.f;
	}

	return false;
}

void c_chams::add_shot_record(c_csplayer* player, matrix3x4_t* matrix)
{
	shot_record_t data;
	data.time = g_ctx.system_time();
	data.alpha = 1.f;
	data.origin = player->get_abs_origin();
	memcpy_fast(data.bones, matrix, sizeof(matrix3x4_t) * 128);

	auto renderable = player->get_renderable_entity();
	if (!renderable)
		return;

	auto model = player->get_model();

	if (!model)
		return;

	auto hdr = interfaces::model_info->get_studio_model(model);
	if (hdr)
	{
		static int m_nSkin = find_in_datamap(player->get_pred_desc_map(), xor_c("m_nSkin"));
		static int m_nBody = find_in_datamap(player->get_pred_desc_map(), xor_c("m_nBody"));

		data.info.origin = player->get_abs_origin();
		data.info.angles = player->get_abs_angles();

		data.state.studio_hdr = hdr;
		data.state.studio_hdr_data = interfaces::model_cache->get_hardware_data(model->studio);
		data.state.renderable = renderable;
		data.state.draw_flags = 0;

		data.info.renderable = renderable;
		data.info.model = model;

		data.info.lightning_offset = nullptr;
		data.info.lightning_origin = nullptr;

		data.info.hitboxset = player->hitbox_set();

		data.info.skin = (int)((uintptr_t)player + m_nSkin);
		data.info.body = (int)((uintptr_t)player + m_nBody);

		data.info.entity_index = player->index();

		data.info.instance = g_memory->getvfunc< unsigned short(__thiscall*)(void*) >(renderable, 30)(renderable);

		data.info.flags = 0x1;

		data.state.decals = 0;
		data.state.lod = 0;

		data.info.model_to_world = &data.current_bones;
		data.state.model_to_world = &data.current_bones;
		data.current_bones.angle_matrix(data.info.angles);
		data.current_bones.set_origin(data.info.origin);
	}

	shot_records.emplace_back(data);
}

void c_chams::on_post_screen_effects()
{
	if (!g_ctx.in_game || !g_ctx.local || !g_ctx.local->is_alive())
	{
		if (shot_records.size() > 0)
			shot_records.clear();
		return;
	}

	static auto original = vtables[vmt_model_render].original<dme_original>(xor_int(21));

	auto chams = g_cfg.visuals.chams[c_onshot];
	if (!chams.enable)
		return;

	auto& main_color = chams.main_color;
	auto& glow_color = chams.glow_color;

	auto ctx = interfaces::material_system->get_render_context();
	if (!ctx)
		return;

	for (int i = 0; i < shot_records.size(); ++i)
	{
		auto& data = shot_records[i];

		auto ent = interfaces::entity_list->get_entity(data.info.entity_index);
		if (!ent)
		{
			shot_records.erase(shot_records.begin() + i);
			continue;
		}

		float diff = std::clamp(g_ctx.system_time() - data.time, 0.f, (float)chams.shot_duration);
		if (diff >= chams.shot_duration)
		{
			data.alpha -= g_ctx.animation_speed * 1.5f;

			if (data.alpha <= 0.f)
			{
				shot_records.erase(shot_records.begin() + i);
				continue;
			}
		}

		float old_blend = interfaces::render_view->get_blend();
		float old_colors[3] = {};
		interfaces::render_view->get_color_modulation(old_colors);

		auto draw_model = [&](c_float_color& clr, c_material* mat)
		{
			mat->increment_reference_count();

			mat->set_material_var_flag(material_var_translucent, true);
			mat->set_material_var_flag(material_var_nofog, true);
			mat->set_material_var_flag(material_var_ignorez, true);

			float colors[3]{ clr[0], clr[1], clr[2] };

			interfaces::render_view->set_blend(clr[3] * data.alpha);
			interfaces::render_view->set_color_modulation(colors);

			interfaces::model_render->forced_material_override(mat);
			original(interfaces::model_render, 0, ctx, data.state, data.info, data.bones);
			interfaces::model_render->forced_material_override(nullptr);

			interfaces::render_view->set_blend(old_blend);
			interfaces::render_view->set_color_modulation(old_colors);
		};

		auto material = materials_list[chams.material];
		if (chams.material == 4)
		{
			{
				auto flat_chams = materials_list[2];
				draw_model(main_color, flat_chams);
			}
			{
				float fill_amount = std::clamp((chams.glow_fill / 100.f) - 0.15f, 0.f, 1.f);

				auto glow = materials_list[4];

				auto var = glow->find_var(xor_c("$envmaptint"), nullptr);
				if (var)
					var->set_vec_value(glow_color[0], glow_color[1], glow_color[2]);

				auto var2 = glow->find_var(xor_c("$envmapfresnelminmaxexp"), nullptr);
				if (var2)
					var2->set_vec_value(0.f, 1.f, 5.f - (5.f * fill_amount));

				draw_model(glow_color, glow);
			}
		}
		else
		{
			auto material = materials_list[chams.material];

			auto var = material->find_var(xor_c("$envmaptint"), nullptr);
			if (var)
				var->set_vec_value(main_color[0], main_color[1], main_color[2]);

			draw_model(main_color, material);
		}
	}
}

bool c_chams::should_draw()
{
	if (!g_ctx.in_game || !g_ctx.local)
		return false;

	auto renderable = (c_renderable*)hook_data.state.renderable;
	if (!renderable)
		return false;

	auto unknown = renderable->get_i_unknown_entity();
	if (!unknown)
		return false;

	auto entity = (c_baseentity*)unknown->get_client_entity();
	if (!entity)
		return false;

	auto model = hook_data.info.model;
	if (!model)
		return false;

	auto client_class = entity->get_client_class();
	if (!client_class)
		return false;

	static auto models_player = xor_str("models/player");
	static auto arms = xor_str("arms");

	if (std::strstr(model->name, models_player.c_str()))
	{
		auto player = (c_csplayer*)entity;
		if (!player)
			return false;

		if (client_class->class_id == CCSRagdoll && player != g_ctx.local && player->team() != g_ctx.local->team())
			return this->draw_model(g_cfg.visuals.chams[c_ragdolls], nullptr, 1.f, true);

		if (!player->is_player())
			return false;

		if (!player->is_alive() || player->gun_game_immunity() || player->dormant())
			return false;

		if (player == g_ctx.local)
		{
			if (!interfaces::input->camera_in_third_person)
				return false;

			float alpha = g_ctx.alpha_amt;
			if (g_ctx.local->is_scoped() && g_cfg.misc.blend_scope)
				alpha *= g_cfg.misc.scope_amt / 100.f;

			auto vars = g_local_animation_fix->get_updated_netvars();
			if (vars)
			{
				const auto& render_origin = g_ctx.local->get_render_origin();

				math::change_matrix_position(vars->fake_bones, 256, {}, render_origin);
				this->draw_model(g_cfg.visuals.chams[c_fake], vars->fake_bones, alpha);
				math::change_matrix_position(vars->fake_bones, 256, render_origin, {});
			}

			// TO-DO: proper interpolated fakelag chams
			// i did this in 2020 but it works wrong and looks like shit
			// maybe i will push this..............

			interfaces::render_view->set_blend(alpha);
			return this->draw_model(g_cfg.visuals.chams[c_local], nullptr, alpha);
		}
		else
		{
			if (g_ctx.local->team() == player->team())
				return false;

			if (g_ctx.local == player)
				return false;

			if (g_ctx.is_alive && g_ctx.lagcomp && g_cfg.visuals.chams[c_history].enable) {
				if (g_cfg.visuals.show_all_history) {
					auto records = g_animation_fix->get_all_records(player);
					if (!records.empty()) {
						for (int i = 1; i <= records.size(); ++i) {
							auto& record = records[i - 1];
							if (!record)
								continue;

							float alpha = (float)i / (float)records.size();
							this->draw_model(g_cfg.visuals.chams[c_history], record->sim_orig.bone, std::lerp(1.f, 0.f, alpha), true);
						}
					}
				}
				else {
					static matrix3x4_t matrix[128]{};
					float alpha = 1.f;
					if (this->get_backtrack_matrix(matrix, player, alpha))
						this->draw_model(g_cfg.visuals.chams[c_history], matrix, alpha, true);
				}
			}

			bool xqz = this->draw_model(g_cfg.visuals.chams[c_xqz], nullptr, 1.f, true);
			bool vis = this->draw_model(g_cfg.visuals.chams[c_vis]);
			return vis || xqz;
		}
	}
	else if (client_class->class_id == CPredictedViewModel && g_ctx.local->is_alive())
		return this->draw_model(g_cfg.visuals.chams[c_wpn]);
	else if (std::strstr(model->name, arms.c_str()) && g_ctx.local->is_alive())
		return this->draw_model(g_cfg.visuals.chams[c_viewmodel]);
	else if (entity->get_move_parent() && entity->get_move_parent() == g_ctx.local && g_ctx.local->is_alive())
	{
		if (!interfaces::input->camera_in_third_person)
			return false;

		float additional_amt = g_cfg.misc.attachments_amt * 0.01f;

		float alpha = g_ctx.alpha_amt;
		if (g_ctx.local->is_scoped() && g_cfg.misc.blend_scope)
			alpha *= g_cfg.misc.scope_amt * 0.01f;

		alpha *= additional_amt;

		interfaces::render_view->set_blend(alpha);
		return this->draw_model(g_cfg.visuals.chams[c_attachments], nullptr, alpha);
	}

	return false;
}

void c_chams::on_draw_model_execute(dme_original original, void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world)
{
	hook_data.init(original, ecx, edx, ctx, state, info, bone_to_world);

	if (interfaces::client_state->delta_tick == -1 || !this->should_draw())
		original(ecx, edx, ctx, state, info, bone_to_world);
}