#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"
#include "../../../functions/config_vars.h"

#include "../../../base/sdk/entity.h"

namespace tr::convars
{
	int __fastcall cl_foot_contact_shadows_get_int(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_cl_foot_contact_shadows].original<decltype(&cl_foot_contact_shadows_get_int)>(xor_int(13));
		if (g_ctx.uninject)
			return original(ecx, edx);

		return 0;
	}

	int __fastcall sv_cheats_get_int(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_sv_cheats].original<decltype(&sv_cheats_get_int)>(xor_int(13));
		if (g_ctx.uninject)
			return original(ecx, edx);

		if ((uintptr_t)_ReturnAddress() == patterns::return_addr_cam_think.as<uintptr_t>())
			return 1;

		return original(ecx, edx);
	}

	int __fastcall cl_csm_shadows_get_int(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_cl_csm_shadows].original<decltype(&cl_csm_shadows_get_int)>(xor_int(13));
		if (g_ctx.uninject || g_cfg.misc.world_modulation & 2)
			return original(ecx, edx);

		return !(g_cfg.misc.removals & shadow);
	}

	int __fastcall cl_brushfastpath_get_int(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_cl_brushfastpath].original<decltype(&cl_brushfastpath_get_int)>(xor_int(13));
		return original(ecx, edx);
	}

	int __fastcall debug_show_spread_get_int(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_debug_show_spread].original<decltype(&debug_show_spread_get_int)>(xor_int(13));

		if (g_ctx.uninject || !g_ctx.local || !g_ctx.local->is_alive())
			return original(ecx, edx);

		c_basecombatweapon* weapon = g_ctx.weapon;
		if (!weapon)
			return original(ecx, edx);

		if (!g_cfg.misc.snip_crosshair || g_ctx.local->is_scoped())
			return original(ecx, edx);

		if (!weapon || !weapon->is_scoping_weapon())
			return original(ecx, edx);

		return 3;
	}

	int __fastcall cl_clock_correction_get_int(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_cl_clock_correction].original<decltype(&cl_clock_correction_get_int)>(xor_int(13));
		return original(ecx, edx);
	}
}