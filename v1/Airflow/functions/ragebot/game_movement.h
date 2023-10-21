#pragma once

namespace game_movement
{
	extern void accelerate(c_usercmd& user_cmd, const vector3d& wishdir, const float wishspeed, vector3d& velocity, float acceleration);
	extern void friction(vector3d& velocity);
	extern void force_stop();
	extern void modify_move(c_usercmd& user_cmd, vector3d& velocity, float max_speed);
	extern unsigned int get_jump_buttons();
}