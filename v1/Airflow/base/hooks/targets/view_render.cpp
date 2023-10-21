#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"
#include "../../../functions/config_vars.h"
#include "../../../functions/extra/world_modulation.h"

#include "../../../functions/anti hit/exploits.h"

#include "../../../functions/ragebot/engine_prediction.h"
#include "../../../functions/ragebot/ragebot.h"

#include "../../../functions/visuals/visuals.h"
#include "../../../functions/visuals/event/event_visuals.h"

#include "../../../functions/ragebot/animfix.h"

#include <string>

namespace tr::view_render
{
	void __fastcall on_render_start(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_view_render_].original<decltype(&on_render_start)>(xor_int(4));
		original(ecx, edx);

		float newfov = 90.f + g_cfg.misc.fovs[world];

		auto ptr = (c_view_render*)ecx;

		if (!g_ctx.local || !g_ctx.local->is_alive())
		{
			ptr->view.fov = newfov;
			return;
		}

		ptr->view.fov_viewmodel = cvars::viewmodel_fov->get_float() + g_cfg.misc.fovs[arms] + g_cfg.misc.fovs[world];
	}

	void __fastcall render_2d_effects_post_hud(void* ecx, void* edx, const c_view_setup& setup)
	{
		static auto original = vtables[vmt_view_render_].original<decltype(&render_2d_effects_post_hud)>(xor_int(39));

		if (g_cfg.misc.removals & flash)
			return;

		original(ecx, edx, setup);
	}

	void __fastcall render_smoke_overlay(void* ecx, void* edx, bool unk)
	{
		static auto original = vtables[vmt_view_render_].original<decltype(&render_smoke_overlay)>(xor_int(40));

		if (g_cfg.misc.removals & flash)
			return;

		original(ecx, edx, unk);
	}
}