#pragma once
#include "../tools/utils_macro.h"

struct verified_cmd_t
{
	c_usercmd cmd{};
	crc32_t crc{};
};

class c_input
{
public:
	padding(0xC);

	bool trackir_available{};
	bool mouse_initialized{};
	bool mouse_active{};

	padding(0x9A);

	bool camera_in_third_person{};

	padding(0x2);

	vector3d camera_offset{};

	padding(0x38);

	c_usercmd* commands{};
	verified_cmd_t* verified_commands{};

	c_usercmd* get_user_cmd(int seq)
	{
		return &commands[seq % 150];
	}

	c_usercmd* get_user_cmd(int slot, int seq)
	{
		return g_memory->getvfunc< c_usercmd* (__thiscall*)(void*, int, int) >(this, 8)(this, slot, seq);
	}

	verified_cmd_t* get_verified_user_cmd(int sequence_number)
	{
		return &verified_commands[sequence_number % 150];
	}

	bool write_user_cmd_delta_to_buffer(int slot, void* buf, int from, int to, bool is_new_cmd)
	{
		using fn = bool(__thiscall*)(c_input*, int, void*, int, int, bool);
		return g_memory->getvfunc< fn >(this, 5)(this, slot, buf, from, to, is_new_cmd);
	}
};