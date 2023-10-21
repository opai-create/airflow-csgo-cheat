#pragma once

struct bullet_tracer_t
{
	vec3_t eye_position{};
	vec3_t impact_position{};
};

struct client_verify_t
{
	vec3_t pos{};
	float time{};
	float expires{};
};

struct hitmarker_t
{
	int dmg{};
	int hp{};

	float time{};
	float dmg_time{};
	float alpha{};
	float impact_time{};

	vec3_t pos{};
};

class c_bullet_tracers 
{
private:
	int last_impact_size{};
	std::vector<bullet_tracer_t> bullets[65]{};

	void on_player_hurt(c_game_event* event);
	void on_bullet_impact(c_game_event* event);

public:
	std::vector<hitmarker_t> impacts{};
	std::vector<hitmarker_t> hitmarkers{};

	void on_game_events(c_game_event* event);

	void render_tracers();
	void render_hitmarkers();

	INLINE void reset()
	{
		last_impact_size = 0;

		impacts.clear();
		hitmarkers.clear();

		for (auto& i : bullets)
			i.clear();
	}
};

#ifdef _DEBUG
inline auto BULLET_TRACERS = std::make_unique<c_bullet_tracers>();
#else
CREATE_DUMMY_PTR(c_bullet_tracers);
DECLARE_XORED_PTR(c_bullet_tracers, GET_XOR_KEYUI32);

#define BULLET_TRACERS XORED_PTR(c_bullet_tracers)
#endif