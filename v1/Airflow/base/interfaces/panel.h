#pragma once
#include "../tools/memory/memory.h"

class c_panel
{
public:
	const char* get_name(int iIndex)
	{
		using fn = const char* (__thiscall*)(void*, int);
		return g_memory->getvfunc< fn >(this, 36)(this, iIndex);
	}

	void set_mouse_input_enabled(unsigned int panel, bool state)
	{
		using fn = void(__thiscall*)(PVOID, int, bool);
		return g_memory->getvfunc< fn >(this, 32)(this, panel, state);
	}
};

class c_ui_panel
{
public:
	int get_child_count()
	{
		using fn = int(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 48)(this);
	}

	c_ui_panel* get_child(int n)
	{
		using fn = c_ui_panel * (__thiscall*)(void*, int);
		return g_memory->getvfunc< fn >(this, 49)(this, n);
	}

	bool has_class(const char* name)
	{
		using fn = bool(__thiscall*)(void*, const char*);
		return g_memory->getvfunc< fn >(this, 139)(this, name);
	}

	void set_attribute_float(const char* name, float value)
	{
		using fn = void(__thiscall*)(void*, const char*, float);
		return g_memory->getvfunc< fn >(this, 288)(this, name, value);
	}
};