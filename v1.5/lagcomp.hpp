#pragma once
#include "animations.hpp"

struct restore_record_t
{
	vec3_t origin{};
	vec3_t abs_origin{};
	vec3_t eye_angles{};

	vec3_t mins{};
	vec3_t maxs{};

	matrix3x4_t bone_cache[128]{};

	INLINE void store(c_cs_player* player)
	{
		origin = player->origin();
		abs_origin = player->get_abs_origin();
		eye_angles = player->eye_angles();

		mins = player->bb_mins();
		maxs = player->bb_maxs();

		player->store_bone_cache(bone_cache);
	}

	INLINE void restore(c_cs_player* player)
	{
		player->origin() = origin;
		player->set_abs_origin(abs_origin);
		player->eye_angles() = eye_angles;

		player->bb_maxs() = maxs;
		player->bb_mins() = mins;

		player->invalidate_bone_cache();
		player->set_bone_cache(bone_cache);
	}

	INLINE void reset()
	{
		origin.reset();
		abs_origin.reset();

		mins.reset();
		maxs.reset();

		std::memset(bone_cache, 0, sizeof(bone_cache));
	}
};

class c_lag_comp
{
private:

public:
	INLINE bool is_tick_valid(bool shifting, bool break_lc, float sim_time)
	{
		if (shifting || break_lc)
			return false;

		auto netchan = HACKS->engine->get_net_channel();
		if (!netchan)
			return false;

		if (HACKS->cl_lagcomp0)
			return true;

		float correct = std::clamp(HACKS->lerp_time + HACKS->ping, 0.f, HACKS->max_unlag);

		const auto server_tickcount = HACKS->global_vars->tickcount + TIME_TO_TICKS(HACKS->ping);
		const auto dead_time = static_cast<int>(TICKS_TO_TIME(server_tickcount) - HACKS->max_unlag);

		float delta_time = std::fabs(correct - (HACKS->predicted_time - sim_time));

#ifdef LEGACY
		return delta_time < 0.19f;
#else
		return delta_time < 0.2f && (sim_time >= static_cast<float>(dead_time));
#endif
	}

	INLINE float get_lerp_time()
	{
		return std::max(HACKS->convars.cl_interp->get_float(), HACKS->convars.cl_interp_ratio->get_float() / HACKS->convars.cl_updaterate->get_float());
	}

	INLINE void set_record(c_cs_player* player, anim_record_t* record, matrix3x4_t* matrix)
	{
		auto anim = ANIMFIX->get_anims(player->index());
		if (!anim)
			return;

		auto origin = record->extrapolated ? record->prediction.origin : record->origin;
		player->origin() = origin;
		player->set_abs_origin(origin);

		player->bb_maxs() = record->maxs;
		player->bb_mins() = record->mins;

		player->invalidate_bone_cache();
		player->set_bone_cache(matrix);
	}

	void build_roll_matrix(c_cs_player* player, matrix_t* side, int side_index, float& fresh_tick, vec3_t& fresh_angles, clamp_bones_info_t& clamp_info);
	void clamp_matrix(c_cs_player* player, matrix_t* side, float& fresh_tick, vec3_t& fresh_angles, clamp_bones_info_t& clamp_info);
	void update_tick_validation();
};

#ifdef _DEBUG
inline auto LAGCOMP = std::make_unique<c_lag_comp>();
#else
CREATE_DUMMY_PTR(c_lag_comp);
DECLARE_XORED_PTR(c_lag_comp, GET_XOR_KEYUI32);

#define LAGCOMP XORED_PTR(c_lag_comp)
#endif