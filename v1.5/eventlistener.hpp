#pragma once

constexpr int EVENT_DEBUG_ID_INIT = 42;
constexpr int EVENT_DEBUG_ID_SHUTDOWN = 13;

class c_event_listener : public c_game_event_listener2
{
private:
	bool registered_events{};
	int debug_id{};

public:
	INLINE c_event_listener() : registered_events(false), debug_id(EVENT_DEBUG_ID_INIT) {}
	INLINE ~c_event_listener()
	{
		debug_id = EVENT_DEBUG_ID_SHUTDOWN;
	}

	INLINE void init()
	{
		HACKS->game_event_manager->add_listener_global(this, false);
	}

	INLINE void remove()
	{
		HACKS->game_event_manager->remove_listener(this);
	}

	virtual void fire_game_event(c_game_event* event);

	virtual int get_event_debug_id()
	{
		return debug_id;
	}
};

#ifdef _DEBUG
inline auto LISTENER_EVENT = std::make_unique<c_event_listener>();
#else
CREATE_DUMMY_PTR(c_event_listener);
DECLARE_XORED_PTR(c_event_listener, GET_XOR_KEYUI32);

#define LISTENER_EVENT XORED_PTR(c_event_listener)
#endif