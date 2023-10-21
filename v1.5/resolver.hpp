#pragma once
#include "animations.hpp"

constexpr int CACHE_SIZE = 2;
constexpr int YAW_CACHE_SIZE = 8;
constexpr auto MAX_TICKS = 3;

struct resolver_info_t
{
	bool resolved{};
	int side{};

	int legit_ticks{};
	int fake_ticks{};

	INLINE void add_legit_ticks()
	{
		if (legit_ticks < MAX_TICKS)
			++legit_ticks;
		else
			fake_ticks = 0;
	}

	INLINE void add_fake_ticks()
	{
		if (fake_ticks < MAX_TICKS)
			++fake_ticks;
		else
			legit_ticks = 0;
	}

	INLINE bool is_legit()
	{
		return legit_ticks > fake_ticks;
	}

	std::string mode{};
	c_animation_layers initial_layers[13]{};

	struct jitter_info_t
	{
		bool is_jitter{};

		float delta_cache[CACHE_SIZE]{};
		int cache_offset{};

		float yaw_cache[YAW_CACHE_SIZE]{};
		int yaw_cache_offset{};

		int jitter_ticks{};
		int static_ticks{};

		int jitter_tick{};

		__forceinline void reset()
		{
			is_jitter = false;

			cache_offset = 0;
			yaw_cache_offset = 0;

			jitter_ticks = 0;
			static_ticks = 0;

			jitter_tick = 0;

			std::memset(delta_cache, 0, sizeof(delta_cache));
			std::memset(yaw_cache, 0, sizeof(yaw_cache));
		}
	} jitter;

	struct freestanding_t
	{
		bool updated{};
		int side{};
		float update_time{};

		inline void reset()
		{
			updated = false;
			side = 0;
			update_time = 0.f;
		}
	} freestanding{};

#ifdef LEGACY
	int lby_breaker{};
	int lby_update{};

	struct move_t
	{
		float time{};
		float lby{};

		inline void reset()
		{
			time = 0.f;
			lby = 0.f;
		}
	} move{};

	struct lby_flicks_t
	{
		bool lby_breaker_failed = false;

		float last_lby_value = 0.0f;
		float next_lby_update = 0.0f;

		int logged_lby_delta_score = 0;
		float logged_lby_delta = 0.0f;

		c_animation_layers old_layers[13]{};

		inline void reset()
		{
			lby_breaker_failed = false;
			last_lby_value = 0.f;
			next_lby_update = 0.f;

			logged_lby_delta_score = 0;
			logged_lby_delta = 0.f;

			for (auto& i : old_layers)
				i = {};
		}
	} lby{};
#endif

	anim_record_t record{};

	inline void reset()
	{
		resolved = false;
		side = 0;
		legit_ticks = 0;
		fake_ticks = 0;

		mode = "";

		freestanding.reset();
		jitter.reset();

#ifdef LEGACY
		lby_breaker = 0;
		lby_update = 0;
		lby.reset();
		move.reset();
		record.reset();
#endif

		for (auto& i : initial_layers)
			i = {};
	}
};

inline resolver_info_t resolver_info[65]{};

namespace resolver
{
	INLINE void reset()
	{
		for (auto& i : resolver_info)
			i.reset();
	}

	extern void prepare_side(c_cs_player* player, anim_record_t* current, anim_record_t* last);
	extern void apply_side(c_cs_player* player, anim_record_t* current, int choke);
}