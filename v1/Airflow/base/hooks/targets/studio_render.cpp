#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../functions/config_vars.h"
#include "../../../functions/features.h"

#include "../../../base/sdk/entity.h"

#include <string>

namespace tr
{
	namespace studio_render
	{
		void __fastcall draw_model(c_studio_render* ecx, void* edx, void* results, const draw_model_info_t& info, matrix3x4_t* bone_to_world, float* flex_weights, float* flex_delayed_weights, const vector3d& model_origin, int flags)
		{
			static auto original = vtables[vmt_studio_render].original<decltype(&draw_model)>(xor_int(29));
			original(ecx, edx, results, info, bone_to_world, flex_weights, flex_delayed_weights, model_origin, flags);
		}

		void __fastcall draw_model_array(void* ecx, void* edx, const studio_model_array_info2_t& info, int count, studio_array_data_t* array_data, int stride, int flags)
		{
			static auto original = vtables[vmt_studio_render].original<decltype(&draw_model_array)>(xor_int(48));

			// force all layers flag (opaque & translucent)
			original(ecx, edx, info, count, array_data, stride, flags);
		}
	}

	namespace model_render
	{
		void __fastcall draw_model_execute(void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world)
		{
			static auto original = vtables[vmt_model_render].original<decltype(&draw_model_execute)>(xor_int(21));
			if (interfaces::studio_render->is_forced_material_override())
				return original(ecx, edx, ctx, state, info, bone_to_world);

			g_chams->on_draw_model_execute(original, ecx, edx, ctx, state, info, bone_to_world);

			interfaces::model_render->forced_material_override(nullptr);
			interfaces::render_view->set_blend(1.f);
		}
	}
}