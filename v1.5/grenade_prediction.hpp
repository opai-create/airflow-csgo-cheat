#pragma once

// mess...
struct nade_path_t
{
	bool is_detonated{};
	bool offscreen{};
	c_cs_player* nade_owner{};
	vec3_t nade_origin{}, nade_velocity{};
	c_client_entity* last_hit_entity{};
	collision_group_t cur_collision_group{};
	float nade_detonate_time{}, nade_expire_time{};
	int nade_idx{}, nade_tick_count{}, next_think_tick{}, last_update_tick{}, bounces_count{};
	std::vector<std::pair<vec3_t, bool>> path{};
	std::string preview_icon{}, preview_name{};
	float nade_damage{};
	ImVec2 last_path_pos{};

	INLINE nade_path_t() = default;

	INLINE nade_path_t(c_cs_player* owner, int index, const vec3_t& origin, const vec3_t& velocity, float throw_time, int offset) : nade_path_t()
	{
		nade_owner = owner;
		nade_idx = index;

		this->predict_nade(origin, velocity, throw_time, offset);
	}

	INLINE bool physics_simulate()
	{
		if (is_detonated)
			return true;

		const auto new_velocity_z = nade_velocity.z - (HACKS->convars.sv_gravity->get_float() * 0.4f) * HACKS->global_vars->interval_per_tick;
		const auto move = vec3_t(
			nade_velocity.x * HACKS->global_vars->interval_per_tick, 
			nade_velocity.y * HACKS->global_vars->interval_per_tick, 
			(nade_velocity.z + new_velocity_z) / 2.f * HACKS->global_vars->interval_per_tick);

		nade_velocity.z = new_velocity_z;

		auto trace = c_game_trace();

		this->physics_push_entity(move, trace);

		if (is_detonated)
			return true;

		if (trace.fraction != 1.f)
		{
			this->update_path< true >();
			this->perform_fly_collision_resolution(trace);
		}

		return false;
	}

	INLINE void physics_trace_entity(const vec3_t& src, const vec3_t& dst, std::uint32_t mask, c_game_trace& trace)
	{
		HACKS->engine_trace->trace_hull(src, dst, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }, mask, nade_owner, cur_collision_group, &trace);

		if (trace.start_solid && (trace.contents & CONTENTS_CURRENT_90))
		{
			trace.clear();
			HACKS->engine_trace->trace_hull(src, dst, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }, mask & ~CONTENTS_CURRENT_90, nade_owner, cur_collision_group, &trace);
		}

		if (!trace.did_hit() || !trace.entity || !((c_cs_player*)trace.entity)->is_player())
			return;

		trace.clear();

		HACKS->engine_trace->trace_line(src, dst, mask, nade_owner, cur_collision_group, &trace);
	}

	INLINE void physics_push_entity(const vec3_t& push, c_game_trace& trace)
	{
		this->physics_trace_entity(nade_origin, nade_origin + push,
			cur_collision_group == COLLISION_GROUP_DEBRIS ? (MASK_SOLID | CONTENTS_CURRENT_90) 
			& ~CONTENTS_MONSTER 
			: MASK_SOLID | CONTENTS_OPAQUE | CONTENTS_IGNORE_NODRAW_OPAQUE | CONTENTS_CURRENT_90 | CONTENTS_HITBOX, trace);

		if (trace.start_solid)
		{
			cur_collision_group = COLLISION_GROUP_INTERACTIVE_DEB;

			HACKS->engine_trace->trace_line(nade_origin - push, nade_origin + push, (MASK_SOLID | CONTENTS_CURRENT_90) & ~CONTENTS_MONSTER, nade_owner, cur_collision_group, &trace);
		}

		if (trace.fraction)
			nade_origin = trace.end;

		if (!trace.entity)
			return;

		if (((c_cs_player*)trace.entity)->is_player() || nade_idx != WEAPON_TAGRENADE && nade_idx != WEAPON_MOLOTOV && nade_idx != WEAPON_INCGRENADE)
			return;

		if (nade_idx != WEAPON_TAGRENADE && trace.plane.normal.z < std::cos(DEG2RAD(HACKS->convars.weapon_molotov_maxdetonateslope->get_float())))
			return;

		this->detonate< true >();
	}

	INLINE void think()
	{
		switch (nade_idx)
		{
		case WEAPON_SMOKEGRENADE:
			if (nade_velocity.length_sqr() <= 0.01f)
			{
				this->detonate< false >();
			}

			break;
		case WEAPON_DECOY:
			if (nade_velocity.length_sqr() <= 0.04f)
			{
				this->detonate< false >();
			}

			break;
		case WEAPON_FLASHBANG:
		case WEAPON_HEGRENADE:
		case WEAPON_MOLOTOV:
		case WEAPON_INCGRENADE:
			if (TICKS_TO_TIME(nade_tick_count) > nade_detonate_time)
			{
				this->detonate< false >();
			}

			break;
		}

		next_think_tick = nade_tick_count + TIME_TO_TICKS(0.2f);
	}

	template < bool _bounced >
	INLINE void detonate()
	{
		is_detonated = true;

		update_path< _bounced >();
	}

	template < bool _bounced >
	INLINE void update_path()
	{
		last_update_tick = nade_tick_count;

		path.emplace_back(nade_origin, _bounced);
	}

	INLINE void predict_nade(const vec3_t& origin, const vec3_t& velocity, float throw_time, int offset)
	{
		nade_origin = origin;
		nade_velocity = velocity;
		cur_collision_group = COLLISION_GROUP_PROJECTILE;

		const auto tick = TIME_TO_TICKS(1.f / 30.f);

		last_update_tick = -tick;

		switch (nade_idx)
		{
		case WEAPON_SMOKEGRENADE:
			next_think_tick = TIME_TO_TICKS(1.5f);
			break;
		case WEAPON_DECOY:
			next_think_tick = TIME_TO_TICKS(2.f);
			break;
		case WEAPON_FLASHBANG:
		case WEAPON_HEGRENADE:
			nade_detonate_time = 1.5f;
			next_think_tick = TIME_TO_TICKS(0.02f);

			break;
		case WEAPON_MOLOTOV:
		case WEAPON_INCGRENADE:
			nade_detonate_time = HACKS->convars.molotov_throw_detonate_time->get_float();
			next_think_tick = TIME_TO_TICKS(0.02f);

			break;
		}

		for (; nade_tick_count < TIME_TO_TICKS(60.f); ++nade_tick_count)
		{
			if (next_think_tick <= nade_tick_count)
			{
				think();
			}

			if (nade_tick_count < offset)
				continue;

			if (this->physics_simulate())
				break;

			if (last_update_tick + tick > nade_tick_count)
				continue;

			this->update_path< false >();
		}

		if (last_update_tick + tick <= nade_tick_count)
		{
			this->update_path< false >();
		}

		nade_expire_time = throw_time + TIME_TO_TICKS(nade_tick_count);
	}

	INLINE void reset()
	{
		is_detonated = false;
		offscreen = false;
		nade_owner = nullptr;
		last_hit_entity = nullptr;
		cur_collision_group = COLLISION_GROUP_NONE;
		nade_origin.reset();
		nade_velocity.reset();

		nade_detonate_time = 0.f;
		nade_expire_time = 0.f;

		nade_idx = 0;
		nade_tick_count = 0;
		next_think_tick = 0;
		last_update_tick = 0;
		bounces_count = 0;

		path.clear();

		preview_icon = "";
		preview_name = "";

		nade_damage = 0.f;

		last_path_pos = {};
	}

	void perform_fly_collision_resolution(c_game_trace& trace);
	bool should_draw();
};

class c_grenade_prediction
{
private:
	int last_server_tick{};

	std::unordered_map<unsigned long, float> nade_alpha{};
	std::unordered_map<unsigned long, float> oof_nade_alpha{};
	std::unordered_map<unsigned long, nade_path_t> list{};

	nade_path_t local_path{};

	void render_offscreen_esp(nade_path_t* info, const float& mod, const float& duration);

public:
	INLINE void reset()
	{
		last_server_tick = 0;

		nade_alpha.clear();
		oof_nade_alpha.clear();
		list.clear();

		local_path.reset();
	}

	INLINE std::unordered_map<unsigned long, nade_path_t>& get_nade_list()
	{
		return list;
	}

	INLINE void erase_handle(unsigned long handle)
	{
		list.erase(handle);
	}

	void calc_local_nade_path();
	void draw_local_path();

	void calc_nade_path(c_base_combat_weapon* entity);

	void calc_world_path();
	void draw_world_path();
};

#ifdef _DEBUG
inline auto GRENADE_PREDICTION = std::make_unique<c_grenade_prediction>();
#else
CREATE_DUMMY_PTR(c_grenade_prediction);
DECLARE_XORED_PTR(c_grenade_prediction, GET_XOR_KEYUI32);

#define GRENADE_PREDICTION XORED_PTR(c_grenade_prediction)
#endif