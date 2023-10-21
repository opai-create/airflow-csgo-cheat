#include "globals.hpp"
#include "buy_bot.hpp"

void c_buy_bot::on_game_events(c_game_event* event)
{
	if (std::strcmp(event->get_name(), CXOR("round_start")))
		return;

	should_buy = true;
}

void c_buy_bot::run()
{
	if (!g_cfg.misc.buybot.enable)
		return;

	if (should_buy)
	{
		std::string buy_str{ };

		switch (g_cfg.misc.buybot.main_weapon)
		{
		case 1:
			buy_str += (CXOR("buy scar20; "));
			buy_str += (CXOR("buy g3sg1; "));
			break;
		case 2:
			buy_str += (CXOR("buy ssg08; "));
			break;
		case 3:
			buy_str += (CXOR("buy awp; "));
			break;
		case 4:
			buy_str += (CXOR("buy negev; "));
			break;
		case 5:
			buy_str += (CXOR("buy m249; "));
			break;
		case 6:
			buy_str += (CXOR("buy ak47; "));
			buy_str += (CXOR("buy m4a1; "));
			buy_str += (CXOR("buy m4a1_silencer; "));
			break;
		case 7:
			buy_str += (CXOR("buy aug; "));
			buy_str += (CXOR("buy sg556; "));
			break;
		}

		switch (g_cfg.misc.buybot.second_weapon)
		{
		case 1:
			buy_str += (CXOR("buy elite; "));
			break;
		case 2:
			buy_str += (CXOR("buy p250; "));
			break;
		case 3:
			buy_str += (CXOR("buy tec9; "));
			buy_str += (CXOR("buy fn57; "));
			break;
		case 4:
			buy_str += (CXOR("buy deagle; "));
			buy_str += (CXOR("buy revolver; "));
			break;
		}

		if (g_cfg.misc.buybot.other_items & 1)
			buy_str += (CXOR("buy vesthelm; "));
		if (g_cfg.misc.buybot.other_items & 2)
			buy_str += (CXOR("buy vest; "));

		if (g_cfg.misc.buybot.other_items & 4)
			buy_str += (CXOR("buy hegrenade; "));

		if (g_cfg.misc.buybot.other_items & 8)
		{
			buy_str += (CXOR("buy molotov; "));
			buy_str += (CXOR("buy incgrenade; "));
		}

		if (g_cfg.misc.buybot.other_items & 16)
			buy_str += (CXOR("buy smokegrenade; "));
		if (g_cfg.misc.buybot.other_items & 32)
			buy_str += (CXOR("buy taser; "));
		if (g_cfg.misc.buybot.other_items & 64)
			buy_str += (CXOR("buy defuser; "));

		HACKS->engine->execute_client_cmd(buy_str.c_str());
	}

	should_buy = false;
}