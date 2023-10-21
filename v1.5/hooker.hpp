#pragma once
#include "hooker_interface.hpp"
#include "detour_hooks.hpp"
#include "vmt_hooks.hpp"

namespace hooker
{
	INLINE void add_detour(const std::uint64_t target, void* callback)
	{
		DETOUR_HOOKS->add_hook(target, callback);
	}

	INLINE void add_vmt(const std::uint64_t target, PLH::VFuncMap& swap)
	{
		VMT_HOOKS->add_hook(target, swap);
	}

	INLINE void hook()
	{
		DETOUR_HOOKS->hook_all();
		VMT_HOOKS->hook_all();
	}

	INLINE void unhook()
	{
		DETOUR_HOOKS->unhook_all();
		VMT_HOOKS->unhook_all();
	}

	template <typename T = void*>
	INLINE T get_original(T callback)
	{
		return DETOUR_HOOKS->get_original<T>(callback);
	}

	template <typename T = void*>
	INLINE T get_original(void* target, const int& index, T hook)
	{
		return VMT_HOOKS->get_original<T>(target, index, hook);
	}
}