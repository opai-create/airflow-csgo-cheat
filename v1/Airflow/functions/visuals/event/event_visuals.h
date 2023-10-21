#pragma once
#include "../../../base/tools/math.h"

#include <vector>
#include <array>

class c_game_event;
class vector3d;
class color;
class c_csplayer;

struct client_verify_t
{
	vector3d pos{};
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

	vector3d pos{};
	vector2d peek_w2s{};
};

class c_event_visuals
{
private:
	int last_impact_size{};

	struct bullet_tracer_t
	{
		vector3d eye_position{};
		vector3d impact_position{};
	};

	std::array< std::vector< bullet_tracer_t >, 65 > bullets{};

	void on_item_purchase(c_game_event* event);
	void on_bomb_plant(c_game_event* event);
	void on_player_hurt(c_game_event* event);
	void on_bullet_impact(c_game_event* event);

	void draw_hitmarkers();

public:
	std::vector< hitmarker_t > impacts{};
	std::vector< hitmarker_t > hitmarkers{};

	void on_render_start(int stage);
	void on_game_events(c_game_event* event);
	void on_directx();
};