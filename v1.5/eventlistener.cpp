#include "globals.hpp"
#include "eventlistener.hpp"
#include "features.hpp"

void c_event_listener::fire_game_event(c_game_event* event)
{
	skin_changer::on_game_events(event);

	BULLET_TRACERS->on_game_events(event);
	EVENT_LOGS->on_game_events(event);
	BUY_BOT->on_game_events(event);
	HUD_HACKS->on_game_events(event);
	RAGEBOT->on_game_events(event);

	if (!std::strcmp(event->get_name(), CXOR("round_start")))
	{
        g_menu.reset_game_info();
        MOVEMENT->reset();
        BULLET_TRACERS->reset();
        ESP->reset();
        GRENADE_PREDICTION->reset();
        ENGINE_PREDICTION->reset();
        ANIMFIX->reset();
        RAGEBOT->reset();
        THREADED_STATE->reset();
        ANTI_AIM->reset();
        FAKE_LAG->reset();
        PING_SPIKE->reset();
        EXPLOITS->reset();
        TICKBASE->reset();

		for (int i = 0; i < 65; ++i)
		{
			auto player = ESP->get_esp_player(i);
			player->dormant.reset();
		}
	}
}