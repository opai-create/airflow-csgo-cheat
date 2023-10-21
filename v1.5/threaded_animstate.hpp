#include "globals.hpp"

#pragma once

// to avoid changing global variables in game and avoid race condition
// the best solution would be rebuild parts when global vars were used 
// and then take custom variables instead

class c_threaded_animstate
{
private:
	void setup_velocity(c_animation_state* state, float curtime);
	void setup_lean(c_animation_state* state, float curtime);
	void setup_aim_matrix(c_animation_state* state, float curtime);

	// it gets bugged, idk why lol
	float last_velocity_test_time[65]{};
	float lower_body_realign_timer[65]{};
public:
	INLINE void reset_variables(int index) {
		last_velocity_test_time[index] = 0.f;
		lower_body_realign_timer[index] = 0.f;
	}

	INLINE void reset()
	{
		for (int i = 0; i < 65; ++i)
			reset_variables(i);
	}

	void update(c_cs_player* player, c_animation_state* state, float yaw, float pitch, float curtime, int framecount);
};

#ifdef _DEBUG
inline auto THREADED_STATE = std::make_unique<c_threaded_animstate>();
#else
CREATE_DUMMY_PTR(c_threaded_animstate);
DECLARE_XORED_PTR(c_threaded_animstate, GET_XOR_KEYUI32);

#define THREADED_STATE XORED_PTR(c_threaded_animstate)
#endif