#pragma once

constexpr int MAX_HOOKS = 150;

template <typename T = void*>
class c_hooker
{
public:
	virtual void add_hook(const std::uint64_t target, T swap) = 0;
	virtual void hook_all() = 0;
	virtual void unhook_all() = 0;
};