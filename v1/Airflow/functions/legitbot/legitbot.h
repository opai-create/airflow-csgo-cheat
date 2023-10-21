#pragma once
#include <vector>
#include <array>
#include <memory>

#include "../config_vars.h"
#include "../../base/sdk.h"
#include "../../base/global_context.h"

class c_legit_bot
{
private:
	std::vector< int > get_hitboxes();
	int get_fov();
	bool is_target_in_fov(const vector2d& center, const vector2d& screen);
	int get_min_damage(c_csplayer* player);
	bool is_visible(c_csplayer* player, const vector3d& pos);
	void do_aimbot();

public:
	void on_predict_start();
	void on_directx();
};