#pragma once
#include <atomic>
#include <wtypes.h>
#include <functional>

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

#ifdef _DEBUG
inline auto KEY_STATES = std::make_unique<c_key_states>();
#else
CREATE_DUMMY_PTR(c_key_states);
DECLARE_XORED_PTR(c_key_states, GET_XOR_KEYUI32);

#define KEY_STATES XORED_PTR(c_key_states)
#endif