#pragma once

class c_ping_spike
{
private:
	bool flipped_state{};

public:
	INLINE void reset()
	{
		flipped_state = false;
	}

	void on_procces_packet();
	void on_net_chan(c_net_channel* netchan, float latency);
};

class c_fake_lag
{
private:
	int get_max_choke();
	void bypass_choke_limit();

public:
	int get_choke_amount();
	void update_shot_cmd();
	void run();

	INLINE void reset()
	{

	}
};

#ifdef _DEBUG
inline auto PING_SPIKE = std::make_unique<c_ping_spike>();
#else
CREATE_DUMMY_PTR(c_ping_spike);
DECLARE_XORED_PTR(c_ping_spike, GET_XOR_KEYUI32);

#define PING_SPIKE XORED_PTR(c_ping_spike)
#endif

#ifdef _DEBUG
inline auto FAKE_LAG = std::make_unique<c_fake_lag>();
#else
CREATE_DUMMY_PTR(c_fake_lag);
DECLARE_XORED_PTR(c_fake_lag, GET_XOR_KEYUI32);

#define FAKE_LAG XORED_PTR(c_fake_lag)
#endif