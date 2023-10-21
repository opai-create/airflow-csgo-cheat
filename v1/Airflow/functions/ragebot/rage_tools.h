#pragma once
#include <array>
#include <vector>
#include <algorithm>

#include "../../base/sdk/entity.h"
#include "../../base/global_context.h"

#include "../ragebot/animfix.h"
#include "../config_vars.h"

struct point_t;

namespace rage_tools
{
	inline bool debug_hitchance = false;
	inline vector2d spread_point{};
	inline float current_spread{};
	inline std::vector< vector2d > spread_points{};

	skin_weapon_t get_skin_weapon_config();
	legit_weapon_t get_legit_weapon_config();
	rage_weapon_t get_weapon_config();

	std::string hitbox_to_string(int id);
	std::string hitgroup_to_string(int hitgroup);
	int hitbox_to_hitgroup(int hitbox);

	int get_legit_tab(c_basecombatweapon* temp_weapon = nullptr);
	bool can_hit_hitbox(const vector3d& start, const vector3d& end, c_csplayer* player, int hitbox, records_t* record, matrix3x4_t* matrix = nullptr);
	bool is_accuracy_valid(c_csplayer* player, point_t& point, float amount, float* out_chance);

	std::vector< std::pair< vector3d, bool > > get_multipoints(c_csplayer* player, int hitbox, matrix3x4_t* matrix);
}