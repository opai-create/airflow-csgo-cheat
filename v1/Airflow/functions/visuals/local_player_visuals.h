#pragma once
#include "../../base/tools/math.h"
#include "../../base/tools/render.h"

#include <vector>

class c_view_setup;

class c_local_visuals
{
private:
	void thirdperson();
	void modulate_bloom();
	void remove_post_processing();
	void remove_smoke();
	void remove_viewmodel_sway();
	void filter_console();
	void fullbright();
	void spoof_cvars();
	void preverse_killfeed();
	void force_ragdoll_gravity();

	bool thirdperson_enabled{};

	vector2d peek_w2s{};
	std::vector< ImVec2 > peek_positions{};

	float old_zoom_sensitivity{};

public:
	void on_paint_traverse();
	void on_directx();
	void on_calc_view();

	void on_render_start(int stage);
	void on_render_start_after(int stage);

	void on_render_view(c_view_setup* setup);
	void on_render_view_after(c_view_setup* setup);

	float last_duck_time{};
};