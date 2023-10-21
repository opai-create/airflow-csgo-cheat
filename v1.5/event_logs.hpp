#pragma once

struct event_log_t
{
	float life_time{};
	c_color clr{};
	std::string message{};
};

class c_event_logs 
{
private:
	bool log_value = true;
	bool set_console = true;

	std::vector<event_log_t> event_logs{};

	void on_item_purchase(c_game_event* event);
	void on_bomb_plant(c_game_event* event);
	void on_player_hurt(c_game_event* event);

public:
	INLINE void push_message(const std::string& message, const c_color& color = { 255, 255, 255, 255 }, bool debug = false)
	{
		auto clr = g_cfg.misc.ui_color.base();
		HACKS->cvar->print_console_color(clr, CXOR("[AIRFLOW] "));

		if (!debug)
		{
			HACKS->cvar->print_console_color(color, "%s \n", message.c_str());
			event_logs.emplace_back(event_log_t{ HACKS->system_time(), color, message });
		}
		else
			HACKS->cvar->print_console_color(c_color{ 150, 150, 150 }, "%s \n", message.c_str());
	}

	INLINE void reset()
	{
		log_value = true;
		set_console = true;

		event_logs.clear();
	}

	void on_game_events(c_game_event* event);
	void filter_console();
	void render_logs();
};

#ifdef _DEBUG
inline auto EVENT_LOGS = std::make_unique<c_event_logs>();
#else
CREATE_DUMMY_PTR(c_event_logs);
DECLARE_XORED_PTR(c_event_logs, GET_XOR_KEYUI32);

#define EVENT_LOGS XORED_PTR(c_event_logs)
#endif