#pragma once

namespace game_movement
{
	extern void friction(vec3_t& velocity);
	extern void modify_move(c_user_cmd& user_cmd, vec3_t& velocity, float max_speed);
	extern void force_stop();
	extern void extrapolate(c_cs_player* player, vec3_t& origin, vec3_t& velocity, memory::bits_t& flags, bool on_ground);
	extern memory::bits_t get_fake_jump_buttons();
	extern unsigned int physics_solid_mask_for_entity(c_cs_player* entity);
}