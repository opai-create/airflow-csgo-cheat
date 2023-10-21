#pragma once

class c_hud_hacks
{
private:
	bool round_started{};
	bool reset_killfeed{};
	float next_update{};

public:
	INLINE void reset()
	{
		round_started = false;
		reset_killfeed = false;

		next_update = 0.f;
	}

	void on_game_events(c_game_event* event);
;	void preverse_killfeed();
};

#ifdef _DEBUG
inline auto HUD_HACKS = std::make_unique<c_hud_hacks>();
#else
CREATE_DUMMY_PTR(c_hud_hacks);
DECLARE_XORED_PTR(c_hud_hacks, GET_XOR_KEYUI32);

#define HUD_HACKS XORED_PTR(c_hud_hacks)
#endif