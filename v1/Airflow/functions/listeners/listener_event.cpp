#include "listener_event.h"

#include "../features.h"

void c_event_listener::fire_game_event(c_game_event* event)
{
	g_event_visuals->on_game_events(event);

	//g_local_animation_fix->on_game_events(event);
	g_rage_bot->on_game_events(event);

	g_utils->on_game_events(event);

	g_esp_store->on_game_events(event);

	skin_changer::on_game_events(event);
}

void c_event_listener::init_events()
{
	for (const auto& event : event_list)
		interfaces::game_event_manager->add_listener(this, event.c_str(), false);
}

void c_event_listener::remove_events()
{
	interfaces::game_event_manager->remove_listener(this);

	event_list.clear();
}