#pragma once

class c_buy_bot
{
private:
	bool should_buy{};

public:
	INLINE void reset()
	{
		should_buy = false;
	}

	void on_game_events(c_game_event* event);
	void run();
};

#ifdef _DEBUG
inline auto BUY_BOT = std::make_unique<c_buy_bot>();
#else
CREATE_DUMMY_PTR(c_buy_bot);
DECLARE_XORED_PTR(c_buy_bot, GET_XOR_KEYUI32);

#define BUY_BOT XORED_PTR(c_buy_bot)
#endif