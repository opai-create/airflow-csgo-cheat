#pragma once
#include "../../base/tools/math.h"
#include "../../base/interfaces/engine_trace.h"

#include <vector>

class c_basecombatweapon;
class c_csplayer;
class c_game_trace;
class c_trace_filter;
struct weapon_info_t;
class vector3d;

using should_hit_fn_t = bool(__cdecl*)(c_baseentity* const, const int);

struct pen_data_t
{
	c_csplayer* hit_player{};
	int dmg{}, hitbox{}, hitgroup{}, remaining_pen{};
};

class c_auto_wall
{
private:
	bool trace_to_exit(const vector3d& src, const vector3d& dir, const c_game_trace& enter_trace, c_game_trace& exit_trace, c_csplayer* shooter);
	void clip_trace_to_player(const vector3d& src, const vector3d& dst, c_game_trace& trace, c_csplayer* const player, const should_hit_fn_t& should_hit_fn);

	bool handle_bullet_penetration(c_csplayer* const shooter, const weapon_info_t* const wpn_data, const c_game_trace& enter_trace, vector3d& src, const vector3d& dir, int& pen_count, float& cur_dmg, const float pen_modifier);

public:
	bool is_breakable_entity(c_baseentity* entity);
	bool can_hit_point(c_csplayer* entity, c_csplayer* starter, const vector3d& point, const vector3d& source, int min_damage);

	void scale_dmg(c_csplayer* const player, float& dmg, const float weapon_armor_ratio, const float headshot_mult, const int hitgroup);

	pen_data_t fire_bullet(c_csplayer* const shooter, c_csplayer* const target, const weapon_info_t* const wpn_data, const bool is_taser, vector3d src, const vector3d& dst, bool ignore_target = false, c_game_trace* tr = nullptr);
};