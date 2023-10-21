#pragma once
#include "xored_pointers.h"
#include <winnt.h>

#define MUTATION_START \
__asm jmp skip_instruction \
__asm __emit  0xBE \
__asm __emit  0xEF \
__asm __emit  0xAB \
__asm __emit  0xBA \
__asm __emit  0x10 \
__asm __emit  0x20 \
__asm __emit  0xFC \
__asm skip_instruction: \

#define MUTATION_END \
__asm jmp skip_instruction2 \
__asm __emit  0xBE \
__asm __emit  0xEF \
__asm __emit  0xBA \
__asm __emit  0xAB \
__asm __emit  0x20 \
__asm __emit  0x10 \
__asm __emit  0xFC \
__asm skip_instruction2: \
\

namespace cheat
{
	extern void initialize();
}

class c_main_cheat_info
{
public:
	std::atomic<bool> init_done{};

	std::string user_name{};
	std::string user_avatar{};
	std::string user_token{};

	LPVOID reserved{};
	HMODULE cheat_module{};

	void init();
};

create_xored_ptr(c_main_cheat_info)

#define g_cheat_info xored_ptr(c_main_cheat_info)