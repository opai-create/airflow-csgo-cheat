#pragma once
#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../ragebot/autowall.h"

class c_game_trace;
class ray_t;

struct pen_data_t;

struct head_pos_t
{
	float angle{};
	float fraction{};
	float damage{};
	vector3d position{};
	vector3d end_position{};

	c_game_trace trace{};
	pen_data_t pen{};
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
	float fake_angle{};
	float start_yaw{};
	float best_yaw{};

	int aa_shot_cmd{};

	std::vector< int > hitbox_list = {
	  hitbox_head,
	  hitbox_chest,
	  hitbox_stomach,
	  hitbox_pelvis,
	  hitbox_left_foot,
	  hitbox_right_foot,
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
	void fake();

	c_csplayer* get_closest_player(bool skip = false, bool local_distance = false);
	bool is_peeking();
	bool is_fake_ducking();

	void on_pre_predict();
	void on_predict_start();
	void on_predict_end();
};