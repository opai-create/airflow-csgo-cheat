#pragma once

class c_prediction_fix
{
private:
	struct compressed_netvars_t
	{
		bool filled{};
		int cmd_number{};
		int tickbase{};

		float fall_velocity{};
		float velocity_modifier{};
		float stamina{};

		vec3_t aimpunch{};
		vec3_t viewpunch{};
		vec3_t aimpunch_vel{};
		vec3_t viewoffset{};
		vec3_t origin{};
		vec3_t base_velocity{};
		vec3_t velocity{};
		vec3_t network_origin{};

		INLINE void store(int tick)
		{
			tickbase = HACKS->local->tickbase();
			fall_velocity = HACKS->local->fall_velocity();
			velocity_modifier = HACKS->local->velocity_modifier();
			aimpunch = HACKS->local->aim_punch_angle();
			viewpunch = HACKS->local->view_punch_angle();
			aimpunch_vel = HACKS->local->aim_punch_angle_vel();
			viewoffset = HACKS->local->view_offset();
			origin = HACKS->local->origin();
			base_velocity = HACKS->local->base_velocity();
			velocity = HACKS->local->velocity();
			network_origin = HACKS->local->network_origin();
			stamina = HACKS->local->stamina();
			cmd_number = tick;

			filled = true;
		}

		INLINE void reset()
		{
			filled = false;
			cmd_number = 0;
			tickbase = 0;

			fall_velocity = 0.f;
			velocity_modifier = 0.f;
			stamina = 0.f;

			aimpunch.reset();
			viewpunch.reset();
			aimpunch_vel.reset();
			viewoffset.reset();
			origin.reset();
			base_velocity.reset();
			velocity.reset();
			network_origin.reset();
		}
	};

	struct ping_backup_t
	{
		int cs_tickcount{};
		int tickcount{};
		float curtime{};
		float frametime{};

		INLINE void store()
		{
			if (!HACKS->client_state)
				return;

			curtime = HACKS->global_vars->curtime;
			frametime = HACKS->global_vars->frametime;
			tickcount = HACKS->global_vars->tickcount;
			cs_tickcount = HACKS->client_state->old_tickcount;
		}

		INLINE void restore()
		{
			if (!HACKS->client_state)
				return;

			HACKS->global_vars->curtime = curtime;
			HACKS->global_vars->frametime = frametime;
			HACKS->global_vars->tickcount = tickcount;
			HACKS->client_state->old_tickcount = cs_tickcount;
		}

		INLINE void reset()
		{
			cs_tickcount = 0;
			tickcount = 0;

			curtime = 0.f;
			frametime = 0.f;
		}
	};

	compressed_netvars_t compressed_netvars[150]{};
public:
	bool pred_error_occured{};
	float velocity_modifier{};
	ping_backup_t ping_backup{};

	INLINE void reset()
	{
		pred_error_occured = false;
		velocity_modifier = 0.f;
		ping_backup.reset();

		for (auto& i : compressed_netvars)
			i.reset();
	}

	INLINE bool available()
	{
		if (!HACKS->engine->is_connected() || !HACKS->engine->is_in_game())
			return false;

		auto net_chan = HACKS->engine->get_net_channel();
		if (!net_chan)
			return false;

		if (net_chan->is_loopback())
			return false;

		return true;
	}

	INLINE bool should_reduce_ping()
	{
		if (!available())
			return false;

		ping_backup.restore();
		return true;
	}

	INLINE void update_ping_values(bool final_tick)
	{
		if (!available())
			return;

		static auto original_packet = hooker::get_original(&hooks::detour::read_packets);

		ping_backup_t backup;
		backup.store();

		original_packet(final_tick);
		ping_backup.store();

		backup.restore();
	}

	INLINE compressed_netvars_t* get_compressed_netvars(int cmd)
	{
		return &compressed_netvars[cmd % 150];
	}

	void fix_netvars(int tick);
	void store(int tick);
};

#ifdef _DEBUG
inline auto PREDFIX = std::make_unique<c_prediction_fix>();
#else
CREATE_DUMMY_PTR(c_prediction_fix);
DECLARE_XORED_PTR(c_prediction_fix, GET_XOR_KEYUI32);

#define PREDFIX XORED_PTR(c_prediction_fix)
#endif