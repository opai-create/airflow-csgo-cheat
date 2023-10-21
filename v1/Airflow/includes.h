#pragma once
#include "windows_includes.h"
#include "base/tools/protect.h"
#include "base/tools/memory/displacement.h"
#include "base/tools/utils_macro.h"
#include "base/tools/memcpy_fast.h"

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#define NtCurrentProcess (HANDLE)-1

// builds
#define ALPHA 0
#define BETA 1

#ifdef _DEBUG
#define debug_text(text, ...) printf(text, __VA_ARGS__)
#else
#define debug_text
#endif