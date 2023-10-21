#pragma once

class c_detour_hooks : public c_hooker<void*>
{
private:
	struct hooks_t
	{
		std::unique_ptr<PLH::IHook> hook;
		std::uint64_t original;
		std::uint64_t target;
		std::uint64_t callback;
	};

	std::vector<hooks_t> hooks{};
public:
	c_detour_hooks()
	{
		hooks.reserve(MAX_HOOKS);
	}

	INLINE void add_hook(const std::uint64_t target, void* swap) override
	{
		auto& hook		= hooks.emplace_back();
		hook.target		= target;
		hook.callback	= (std::uint64_t)swap;
		hook.hook		= std::make_unique<PLH::x86Detour>(hook.target, hook.callback, &hook.original);
	}

	INLINE void hook_all() override
	{
		for (const auto& i : hooks)
		{
			if (i.hook)
			{
				if (i.hook->isHooked())
					continue;

				i.hook->hook();
			}
		}
	}

	INLINE void unhook_all() override
	{
		for (const auto& i : hooks)
		{
			if (i.hook)
			{
				if (!i.hook->isHooked())
					continue;

				i.hook->unHook();
			}
		}
	}

	template <typename T = void*>
	INLINE T get_original(T callback)
	{
		auto iter = std::find_if(hooks.begin(), hooks.end(), [&](const hooks_t& hook)
			{
				return (void*)hook.callback == (void*)callback;
			});

		if (iter == hooks.end() || iter->original == 0)
			return 0;

		return PLH::FnCast(iter->original, callback);
	}
};

#ifdef _DEBUG
inline auto DETOUR_HOOKS = std::make_unique<c_detour_hooks>();
#else
CREATE_DUMMY_PTR(c_detour_hooks);
DECLARE_XORED_PTR(c_detour_hooks, GET_XOR_KEYUI32);

#define DETOUR_HOOKS XORED_PTR(c_detour_hooks)
#endif
