#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../functions/features.h"

#include "../../../functions/config_vars.h"

#include <string>

namespace tr
{
	namespace material_system
	{
		std::string world_tex = xor_str("World textures");
		std::string prop_tex = xor_str("StaticProp textures");
		std::string light_map = xor_str("LightmappedGeneric");

		c_material* __fastcall find_material(void* _this, void* edx, const char* mat_name, const char* tex_name, bool complain, const char* comp_prefix)
		{
			static auto original = vtables[vmt_material_system].original<decltype(&find_material)>(84);

			if (strstr(mat_name, xor_c("engine")))
				return original(_this, edx, mat_name, tex_name, complain, comp_prefix);

			bool use_phong = g_cfg.misc.world_material_options & 1;
			bool use_env = g_cfg.misc.world_material_options & 2;
			bool use_money = g_cfg.misc.world_material_options & 4;

			auto material = original(_this, edx, mat_name, tex_name, complain, comp_prefix);
			if (material->is_error_material() || !material || tex_name == nullptr || mat_name == nullptr)
			{
				return material;
			}
			else
			{
				if (!std::strcmp(material->get_texture_group_name(), world_tex.c_str()) || !std::strcmp(material->get_texture_group_name(), prop_tex.c_str()))
				{
					auto material_to_set = use_money ? g_chams->custom_world : material;

					auto var1 = material_to_set->find_var(xor_c("$phong"), nullptr);
					if (var1)
						var1->set_vec_value(use_phong ? "1" : "0");

					auto var2 = material_to_set->find_var(xor_c("$phongboost"), nullptr);
					if (var2)
						var2->set_vec_value(1.f, 1.f, 1.f);

					auto var3 = material_to_set->find_var(xor_c("$basemapalphaphongmask"), nullptr);
					if (var3)
						var3->set_vec_value("1");

					auto var4 = material_to_set->find_var(xor_c("$normalmapalphaenvmask"), nullptr);
					if (var4)
						var4->set_vec_value("1");

					auto var5 = material_to_set->find_var(xor_c("$envmap"), nullptr);
					if (var5)
						var5->set_vec_value(xor_c("env_cubemap"));

					float value = 1.f * (int)use_env;

					auto var6 = material_to_set->find_var(xor_c("$envmaptint"), nullptr);
					if (var6)
						var6->set_vec_value(value, value, value);

					return material_to_set;
				}

				return material;
			}
			return nullptr;
		}

		void __fastcall get_color_modulation(c_material* ecx, void* edx, float* r, float* g, float* b)
		{
			static auto original = hooker.original(&get_color_modulation);
			original(ecx, edx, r, g, b);

			if (!ecx || ecx->is_error_material())
				return;

			const char* name = ecx->get_texture_group_name();

			static auto world_tex = xor_str("World");
			static auto sky_tex = xor_str("SkyBox");
			static auto prop_tex = xor_str("StaticProp");

			if (!(g_cfg.misc.world_modulation & 1))
				return;

			static auto smoke = xor_str("smoke");

			if (std::strstr(name, world_tex.c_str()))
			{
				*r *= g_cfg.misc.world_clr[walls][0];
				*g *= g_cfg.misc.world_clr[walls][1];
				*b *= g_cfg.misc.world_clr[walls][2];
			}
			else if (std::strstr(name, sky_tex.c_str()))
			{
				*r *= g_cfg.misc.world_clr[sky][0];
				*g *= g_cfg.misc.world_clr[sky][1];
				*b *= g_cfg.misc.world_clr[sky][2];
			}
			else if (std::strstr(name, prop_tex.c_str()))
			{
				*r *= g_cfg.misc.world_clr[props][0];
				*g *= g_cfg.misc.world_clr[props][1];
				*b *= g_cfg.misc.world_clr[props][2];
			}
		}
	}

	// credits: https://github.com/rollraw/qo0-base/blob/2de7c14bf1941ad2ad77f30783f9e06fc646a153/base/core/hooks.cpp#L197 & osiris
	namespace key_values
	{
		void* __fastcall alloc_key_values_memory(c_key_values_system* thisptr, int edx, int size)
		{
			static auto original = vtables[vmt_key_values_system].original<decltype(&alloc_key_values_memory)>(xor_int(2));

			const uintptr_t return_address = (uintptr_t)_ReturnAddress();
			if (return_address == patterns::alloc_key_values_engine.as< uintptr_t >() || return_address == patterns::alloc_key_values_client.as< uintptr_t >())
				return nullptr;

			return original(thisptr, edx, size);
		}
	}
}