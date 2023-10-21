#pragma once

// Security
#include "lazy_importer.h"
#include "xorstr.h"

// Hooking
#include "PolyHook/polyhook2/IHook.hpp"
#include "PolyHook/polyhook2/Detour/x86Detour.hpp"
#include "PolyHook/polyhook2/Virtuals/VFuncSwapHook.hpp"
#include "PolyHook/polyhook2/Virtuals/VTableSwapHook.hpp"

// Utils for cheat base
#include "tinyformat.h"

#ifdef DEBUG
#define WINCALL(func) func
#else
#define WINCALL(func) LI_FN(func).cached()
#endif