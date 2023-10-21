#include "visuals.h"

#include "../menu/menu.h"

#include "../config_vars.h"
#include "../features.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/tools/render.h"
#include "../../base/tools/key_states.h"

#include "../../base/other/game_functions.h"

#include "../../base/sdk/entity.h"

void c_visuals_wrapper::on_paint_traverse()
{
	if (g_ctx.in_game)
	{
		g_local_visuals->on_paint_traverse();
		g_grenade_warning->on_paint_traverse();
		g_esp_store->on_paint_traverse();
	}
}

void c_visuals_wrapper::on_directx()
{
	if (g_ctx.in_game)
	{
		g_weapon_esp->on_directx();
		g_player_esp->on_directx();
		g_event_visuals->on_directx();
		g_local_visuals->on_directx();
		g_grenade_warning->on_directx();
		g_event_logger->on_directx();
		g_legit_bot->on_directx();
	}
}
