#pragma once
class c_color;
class c_base_entity;
class c_cs_player;

class c_hacks;

#define INLINE __forceinline

#ifdef _DEBUG
#define DEBUG_LOG(str, ...) printf(str, __VA_ARGS__)
#else
#define DEBUG_LOG(str, ...)
#endif

// STL
#include <vector>
#include <string>
#include <array>
#include <deque>
#include <algorithm>
#include <thread>
#include <mutex>
#include <iostream>

// LIBS
#include <windows.h>
#include <libs.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <math.h>
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_freetype.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

// SECURITY
#include "seeds.hpp"
#include "xorint.hpp"
#include "xorstr.hpp"
#include "xorpointer.hpp"
#include "hash.hpp"

// SDK
#include "bytesarray.hpp"
#include "color.hpp"
#include "memory.hpp"
#include "math.hpp"
#include "hooks.hpp"
#include "defines.hpp"
#include "sdk_misc.hpp"
#include "misc_structs.hpp"
#include "structs.hpp"
#include "interfaces.hpp"
#include "hacks.hpp"
#include "vgui_panel.hpp"
#include "render.hpp"
#include "netvars.hpp"
#include "offsets.hpp"
#include "entities.hpp"
#include "main_utils.hpp"
#include "legacy ui/config_system.h"
#include "legacy ui/config_vars.h"
#include "key_states.hpp"
#include "threads.hpp"

#undef min
#undef max

#define TIME_TO_TICKS(t) ((int)(0.5f + (float)(t) / HACKS->global_vars->interval_per_tick))
#define TICKS_TO_TIME(t) (HACKS->global_vars->interval_per_tick * (t))

class c_thread_loop
{
public:
    c_thread_loop(int counter)
    {
    }
};


#ifdef _DEBUG
#define CHECKMEM
#else
#define CHECKMEM \
static c_thread_loop thread_loop{__COUNTER__}; 
#endif