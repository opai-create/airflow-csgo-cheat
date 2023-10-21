#pragma once
#include "../tools/memory/memory.h"

class c_memory_allocate
{
public:
	void* alloc(int nSize)
	{
		using fn = void* (__thiscall*)(void*, int);
		return g_memory->getvfunc< fn >(this, 1)(this, nSize);
	}

	void* realloc(void* pMem, int nSize)
	{
		using fn = void* (__thiscall*)(void*, void*, int);
		return g_memory->getvfunc< fn >(this, 3)(this, pMem, nSize);
	}

	void free(void* pMem)
	{
		using fn = void(__thiscall*)(void*, void*);
		return g_memory->getvfunc< fn >(this, 5)(this, pMem);
	}
};