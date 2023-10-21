#pragma once

struct head_position_t
{
	float angle;
	float fraction;
	vec3_t position;
	vec3_t end_position;
};

class c_adaptive_angle
{
public:
	float yaw{};
	float distance{};

public:
	INLINE c_adaptive_angle(float yaw, float penalty = 0.f) : yaw(math::normalize_yaw(yaw))
	{
		distance = 0.f;
		distance -= penalty;
	}
};

class c_anti_aim
{
private:
	bool fake_ducking{};
	bool defensive_aa{};
	bool can_duck{};
	bool edging{};

	bool flip_side{};
	bool flip_jitter{};
	bool flip_move{};

	int fake_side{};
	int shot_cmd{};
	float fake_angle{};
	float start_yaw{};
	float best_yaw{};

	std::vector<int> hitbox_list = {
		HITBOX_HEAD,
		HITBOX_CHEST,
		HITBOX_STOMACH,
		HITBOX_PELVIS,
		HITBOX_LEFT_FOOT,
		HITBOX_RIGHT_FOOT,
	};

	void fake_duck();
	void slow_walk();
	void force_move();
	void extended_fake();
	void manual_yaw();
	void automatic_edge();
	void freestanding();
	void at_targets();

public:
	INLINE void reset()
	{
		fake_ducking = false;
		defensive_aa = false;
		can_duck = false;
		edging = false;
		flip_side = false;
		flip_jitter = false;
		flip_move = false;

		fake_side = 0;
		shot_cmd = 0;

		fake_angle = 0.f;
		start_yaw = 0.f;
		best_yaw = 0.f;
	}

	void fake();

	c_cs_player* get_closest_player(bool skip = false, bool local_distance = false);
	bool is_peeking();
	bool is_fake_ducking();

	void run_movement();
	void run();
	void cleanup();
};

#ifdef _DEBUG
inline auto ANTI_AIM = std::make_unique<c_anti_aim>();
#else
CREATE_DUMMY_PTR(c_anti_aim);
DECLARE_XORED_PTR(c_anti_aim, GET_XOR_KEYUI32);

#define ANTI_AIM XORED_PTR(c_anti_aim)
#endif