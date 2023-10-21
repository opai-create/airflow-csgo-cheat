#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../functions/config_vars.h"

#include "../../../base/sdk/entity.h"

#include "../../../functions/listeners/listener_entity.h"

#include "../../../functions/features.h"

namespace tr::prediction
{
	bool __fastcall in_prediction(void* ecx, void* edx)
	{
		static auto original = vtables[vmt_prediction].original<decltype(&in_prediction)>(xor_int(14));

		if (!g_ctx.in_game)
			return original(ecx, edx);

		if (!g_ctx.local || !g_ctx.local->is_alive())
			return original(ecx, edx);

		if ((g_cfg.misc.removals & vis_recoil) && (uintptr_t)_ReturnAddress() == patterns::return_addr_drift_pitch.as< uintptr_t >())
			return true;

		return original(ecx, edx);
	}

	void __fastcall run_command(void* ecx, void* edx, c_csplayer* player, c_usercmd* cmd, c_movehelper* move_helper)
	{
		static auto original = vtables[vmt_prediction].original<decltype(&run_command)>(xor_int(19));

		if (!g_ctx.local || !player || player != g_ctx.local)
			return;

		if (cmd->tickcount == INT_MAX)
		{
			player->tickbase()++;
			return;
		}		

		g_tickbase->fix(cmd->command_number, player->tickbase());
		original(ecx, edx, player, cmd, move_helper);
	}

	void __fastcall process_movement(void* ecx, void* edx, c_csplayer* player, c_movedata* data)
	{
		static auto original = vtables[vmt_game_movement].original<decltype(&process_movement)>(xor_int(1));

		// fix prediction error in air (by stop calculating some vars in movement)
		data->game_code_moved_player = false;
		original(ecx, edx, player, data);
	}
}