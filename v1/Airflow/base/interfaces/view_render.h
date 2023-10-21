#pragma once
#include "client.h"

class c_view_render
{
public:
	char pad[4];
	c_view_setup view;
};

class c_render_view
{
public:
	void set_blend(float value)
	{
		using fn = void(__thiscall*)(void*, float);
		return g_memory->getvfunc< fn >(this, 4)(this, value);
	}

	float get_blend()
	{
		using fn = float(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 5)(this);
	}

	void set_color_modulation(float* value)
	{
		using fn = void(__thiscall*)(void*, float*);
		return g_memory->getvfunc< fn >(this, 6)(this, value);
	}

	void get_color_modulation(float* value)
	{
		using fn = void(__thiscall*)(void*, float*);
		return g_memory->getvfunc< fn >(this, 7)(this, value);
	}
};