#pragma once
#include <atomic>
#include <wtypes.h>
#include <functional>

#include "utils_macro.h"

class c_key_states
{
protected:
	std::atomic< short > array_states[256] = {};

public:
	c_key_states();

	struct key_info_t
	{
		WPARAM key{};
		bool state{};
	};

	__forceinline bool at(int index) const
	{
		return array_states[index];
	}

	bool proc_key(int idx, int key, bool state);
	key_info_t get_key_state(UINT uMsg, WPARAM wParam);
	bool key_updated(int key, UINT uMsg, WPARAM wParam);
	void update(UINT uMsg, WPARAM wParam);
};

declare_feature_ptr(key_states);