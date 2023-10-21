#pragma once

class c_usercmd;

namespace cmd_shift
{
	inline bool shifting{};

	extern void shift_silent(c_usercmd* current_cmd, c_usercmd* first_cmd, int amount);
	extern void shift_predicted(c_usercmd* current_cmd, c_usercmd* first_cmd, int amount);
}