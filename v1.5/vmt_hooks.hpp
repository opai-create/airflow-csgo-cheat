#pragma once

class c_vmt_hooks : public c_hooker<PLH::VFuncMap&>
{
private:
	struct hooks_t
	{
		std::unique_ptr<PLH::IHook> hook;
		std::uint64_t callback;
		PLH::VFuncMap orig_map;
		PLH::VFuncMap new_map;
	};

	std::vector<hooks_t> hooks{};
public:
	c_vmt_hooks()
	{
		hooks.reserve(MAX_HOOKS);
	}

	INLINE void add_hook(const std::uint64_t target, PLH::VFuncMap& swap) override
	{
		auto& hook		= hooks.emplace_back();
		hook.new_map	= std::move(swap);
		hook.callback	= target;
		hook.hook		= std::make_unique<PLH::VFuncSwapHook>(hook.callback, hook.new_map, &hook.orig_map);
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
	INLINE T get_original(void* target, const int& index, T hook)
	{
		auto iter = std::find_if(hooks.begin(), hooks.end(), [&](const hooks_t& hook)
			{
				return (void*)hook.callback == target;
			});

		if (iter == hooks.end() || iter->orig_map.empty())
			return 0;

		return (T)iter->orig_map[index];
	}
};

#ifdef _DEBUG
inline auto VMT_HOOKS = std::make_unique<c_vmt_hooks>();
#else
CREATE_DUMMY_PTR(c_vmt_hooks);
DECLARE_XORED_PTR(c_vmt_hooks, GET_XOR_KEYUI32);

#define VMT_HOOKS XORED_PTR(c_vmt_hooks)
#endif