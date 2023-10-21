#pragma once

class c_cs_player;
struct bullet_t
{
	int damage{};
	int penetration_count{};
	int hitbox{};
	int hitgroup{};

	c_cs_player* traced_target{};
};

namespace penetration
{
	extern bool test_hitboxes(c_cs_player* player, c_game_trace* trace, const ray_t& ray, matrix3x4_t* matrix = nullptr);
	extern bullet_t simulate(c_cs_player* shooter, c_cs_player* target, vec3_t source, const vec3_t& dest, bool ignore_damage = false, bool simple = false);
}