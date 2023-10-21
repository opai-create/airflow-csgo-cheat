#pragma once

class c_engine_prediction_restore
{
private:
	bool in_prediciton{};
	bool first_time_predicted{};

	int random_seed{};
	int player{};

	vec3_t old_origin{};
	c_move_data movedata{};

	c_user_cmd* cmd_ptr{};
	c_user_cmd pred_cmd{};

public:
	INLINE c_engine_prediction_restore()
	{
		in_prediciton = HACKS->prediction->in_prediction;
		first_time_predicted = HACKS->prediction->is_first_time_predicted;

		random_seed = **offsets::prediction_random_seed.cast<int**>();
		player = **offsets::prediction_player.cast<int**>();

		movedata = **(c_move_data**)((std::uintptr_t)HACKS->game_movement + 8);

#ifdef LEGACY
		cmd_ptr = *(c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3314));
		pred_cmd = *(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x326C));
#else
		cmd_ptr = *(c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3348));
		pred_cmd = *(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x3298));
#endif

		old_origin = *(vec3_t*)((std::uintptr_t)HACKS->local + XORN(0x3AC));
	}

	INLINE ~c_engine_prediction_restore()
	{
		HACKS->prediction->in_prediction = std::move(in_prediciton);
		HACKS->prediction->is_first_time_predicted = std::move(first_time_predicted);

		**offsets::prediction_random_seed.cast<int**>() = std::move(random_seed);
		**offsets::prediction_player.cast<int**>() = std::move(player);

		**(c_move_data**)((std::uintptr_t)HACKS->game_movement + 8) = std::move(movedata);

#ifdef LEGACY
		* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3314)) = cmd_ptr;
		*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x326C)) = std::move(pred_cmd);
#else
		* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3348)) = cmd_ptr;
		*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x3298)) = std::move(pred_cmd);
#endif

		* (vec3_t*)((std::uintptr_t)HACKS->local + XORN(0x3AC)) = std::move(old_origin);
	}
};

namespace cmd_shift
{
	inline bool shifting{};

	extern void shift_silent(c_user_cmd* current_cmd, c_user_cmd* first_cmd, int amount);
	extern void shift_predicted(c_user_cmd* current_cmd, c_user_cmd* first_cmd, int amount);
}