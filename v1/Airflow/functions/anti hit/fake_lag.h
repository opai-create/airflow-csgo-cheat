#pragma once
#include <deque>
#include <vector>

struct commands_t
{
	int prev_cmd{};
	int cmd{};
	bool used{};
	bool outgoing{};
};

struct choked_ticks_t
{
	int tickcount{};
	int choke{};
	int cmd{};
};

class c_ping_spike
{
private:
	bool flipped_state{};

public:
	void on_procces_packet();
	void on_net_chan(c_netchan* netchan, float latency);
};

class c_fake_lag
{
private:
	int get_max_choke();
	void bypass_choke_limit();
	vector3d last_velocity{};

public:
	int get_choke_amount();
	void on_predict_start();
	void on_predict_end();
	void on_local_death();

	std::vector< int > commands{};
	std::deque< choked_ticks_t > choked_ticks{};
	std::deque< int > choked_commands{};
};