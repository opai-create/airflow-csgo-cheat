#pragma once

constexpr int cache_size = 2;
constexpr int yaw_cache_size = 8;
constexpr int weight_cache_size = 10;

struct resolver_info_t
{
	bool resolved{};
	int side{};

	std::string mode{};

	struct jitter_info_t
	{
		bool is_jitter{};

		float delta_cache[cache_size]{};
		int cache_offset{};

		float yaw_cache[yaw_cache_size]{};
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

	inline void reset()
	{
		resolved = false;
		side = 0;

		mode = "";

		freestanding.reset();
		jitter.reset();
	}
};

inline resolver_info_t resolver_info[65]{};

namespace resolver
{
	extern vector3d get_point_direction(c_csplayer* player);
	extern void prepare_angle(c_csplayer* player);
	extern void prepare_side(c_csplayer* player, records_t* current);
	extern void apply_side(c_csplayer* player, const int& choke);
}