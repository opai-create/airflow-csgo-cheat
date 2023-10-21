#include "esp_player.h"
#include "../ragebot/ragebot.h"
#include "../ragebot/rage_tools.h"
#include "../features.h"

void render_main_esp(c_esp_store::player_info_t* info)
{
	auto& enemy_esp = g_cfg.visuals.esp[esp_enemy];

	int bar_offsets[4]{};
	float string_offsets[4]{};

	int iter = 0;

	auto& box = info->box;

	auto esp_step = 1.f - (std::clamp(info->health, 0.f, 100.f) / 100.f);

	auto base_color = enemy_esp.colors.health.base();
	auto health_color = base_color.multiply(color(255, 87, 87), esp_step).new_alpha(base_color.a());

	if (enemy_esp.elements & 1)
	{
		auto clr = enemy_esp.colors.box.base();

		g_render->rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, color(0, 0, 0, clr.a() * 0.6f * info->dormant_alpha));
		g_render->rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, color(0, 0, 0, clr.a() * 0.6f * info->dormant_alpha));
		g_render->rect(box.x, box.y, box.w, box.h, clr.new_alpha(clr.a() * info->dormant_alpha));
	}

#ifdef _DEBUG
	if (rage_tools::debug_hitchance)
	{
		if (rage_tools::current_spread > 0.f)
		{
			g_render->circle(rage_tools::spread_point.x, rage_tools::spread_point.y, rage_tools::current_spread, color(255, 0, 255), 100);
		}

		for (auto& p : rage_tools::spread_points)
		{
			g_render->filled_circle(p.x, p.y, 4.f, color(0, 0, 0), 100);
			g_render->filled_circle(p.x, p.y, 3.f, color(255, 255, 255), 100);
		}
	}
#endif

	if (enemy_esp.elements & 2)
		info->objs[iter++] = esp_objects_t{ true, esp_up, enemy_esp.colors.name.base(), 0.f, 0.f, info->name, font_default, true, info->dormant_alpha };

	if (enemy_esp.elements & 4)
		info->objs[iter++] = esp_objects_t{ true, esp_left, health_color, info->health, 100.f, "", 0, false, info->dormant_alpha };

	if (enemy_esp.elements & 8)
		info->objs[iter++] = esp_objects_t{ true, esp_down, enemy_esp.colors.weapon.base(), 0.f, 0.f, info->weaponicon, font_icon, true, info->dormant_alpha };

	if (enemy_esp.elements & 16)
		info->objs[iter++] = esp_objects_t{ true, esp_down, enemy_esp.colors.weapon.base(), 0.f, 0.f, info->weaponname, font_pixel, false, info->dormant_alpha };

	if (enemy_esp.elements & 32 && !info->miscweapon)
		info->objs[iter++] = esp_objects_t{ true, esp_down, enemy_esp.colors.ammo_bar.base(), info->ammo, info->maxammo, "", 0, false, info->dormant_alpha };

	if (enemy_esp.elements & 64)
	{
		auto& flags = info->flags;
		if (info->cheat_id > 0)
		{
			switch (info->cheat_id)
			{
			case 1:
				info->objs[iter++] = esp_objects_t{ true, esp_right, color(254, 103, 49), 0.f, 0.f, xor_str("WEAVE"), font_pixel, false, info->dormant_alpha };
				break;
			case 2:
				info->objs[iter++] = esp_objects_t{ true, esp_right, color(150, 113, 220), 0.f, 0.f, xor_str("AIRFLOW"), font_pixel, false, info->dormant_alpha };
				break;
			case 3:
				info->objs[iter++] = esp_objects_t{ true, esp_right, color(0, 255, 163), 0.f, 0.f, xor_str("BOSS"), font_pixel, false, info->dormant_alpha };
				break;
			case 4:
				info->objs[iter++] = esp_objects_t{ true, esp_right, color(171, 102, 79), 0.f, 0.f, xor_str("CRACKER"), font_pixel, false, info->dormant_alpha };
				break;
			case 5:
				info->objs[iter++] = esp_objects_t{ true, esp_right, color(0, 255, 163), 0.f, 0.f, xor_str("WEAVE BOSS"), font_pixel, false, info->dormant_alpha };
				break;
			}
		}
		if (flags.armorvalue)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(255, 255, 255), 0.f, 0.f, flags.hashelmet ? xor_str("HK") : xor_str("K"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.zoom)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(48, 196, 255), 0.f, 0.f, xor_str("ZOOM"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.aimtarget)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(255, 255, 255), 0.f, 0.f, xor_str("HIT"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.fakeduck)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(255, 69, 74), 0.f, 0.f, xor_str("FD"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.deffensive)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(255, 255, 255), 0.f, 0.f, xor_str("EX"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.reloading)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(184, 199, 255), 0.f, 0.f, xor_str("RELOAD"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.havebomb)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(255, 147, 128), 0.f, 0.f, xor_str("C4"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.havekit)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(125, 212, 255), 0.f, 0.f, xor_str("KIT"), font_pixel, false, info->dormant_alpha };
		}
		if (flags.defusing)
		{
			info->objs[iter++] = esp_objects_t{ true, esp_right, color(201, 125, 255), 0.f, 0.f, xor_str("DEF"), font_pixel, false, info->dormant_alpha };
		}
	}

	if (info->dormant_alpha >= 0.9f && (enemy_esp.elements & 512))
	{
		auto clr = enemy_esp.colors.skeleton.base();

		for (int i = 0; i < 128; i++)
		{
			if (info->bone_pos_parent[i].length() > 0.f && info->bone_pos_child[i].length() > 0.f)
			{
				g_render->line(info->bone_pos_parent[i].x, info->bone_pos_parent[i].y,
					info->bone_pos_child[i].x, info->bone_pos_child[i].y,
					clr.new_alpha(clr.a() * info->dormant_alpha), 1.f);
			}
		}
	}

	for (int i = 0; i < max_objs_count; ++i)
	{
		if (!info->objs[i].filled)
			continue;

		switch (info->objs[i].pos_type)
		{
		case esp_left:
			esp_renderer::esp_bars(box, info->objs[i], bar_offsets[0]);
			break;
		case esp_up:
			esp_renderer::esp_bars(box, info->objs[i], bar_offsets[1]);
			break;
		case esp_right:
			esp_renderer::esp_bars(box, info->objs[i], bar_offsets[2]);
			break;
		case esp_down:
			esp_renderer::esp_bars(box, info->objs[i], bar_offsets[3]);
			break;
		}
	}

	// ghetto method: grab latest bar offset and apply it to next esp strings
	for (int i = 0; i < max_objs_count; ++i)
	{
		if (!info->objs[i].filled)
			continue;

		switch (info->objs[i].pos_type)
		{
		case esp_left:
			esp_renderer::esp_strings(box, info->objs[i], bar_offsets[0], string_offsets[0]);
			break;
		case esp_up:
			esp_renderer::esp_strings(box, info->objs[i], bar_offsets[1], string_offsets[1]);
			break;
		case esp_right:
			esp_renderer::esp_strings(box, info->objs[i], bar_offsets[2], string_offsets[2]);
			break;
		case esp_down:
			esp_renderer::esp_strings(box, info->objs[i], bar_offsets[3], string_offsets[3]);
			break;
		}
	}
}

void render_offscreen_esp(c_esp_store::player_info_t* info)
{
	if (!g_ctx.is_alive)
		return;

	auto& enemy_esp = g_cfg.visuals.esp[esp_enemy];
	if (!(enemy_esp.elements & 128))
		return;

	auto clr = enemy_esp.colors.offscreen_arrow.base();
	auto outline_clr = enemy_esp.colors.offscreen_arrow_outline.base();

	vector3d local_origin = g_ctx.abs_origin;

	const vector3d& origin = info->origin;
	int arrow_size = enemy_esp.offscreen_size;
	float arrow_distance = enemy_esp.offscreen_dist;

	const float rotation = math::deg_to_rad(g_ctx.orig_angle.y - math::angle_from_vectors(local_origin, origin).y - 90.f);

	vector2d center = vector2d(g_render->screen_size.w / 2, g_render->screen_size.h / 2);

	vector2d position = 
	{ 
		center.x + arrow_distance * std::cos(rotation), 
		center.y + arrow_distance * std::sin(rotation) 
	};

	vector2d points[3] = 
	{ 
		vector2d(position.x - arrow_size, position.y - arrow_size), 
		vector2d(position.x + arrow_size, position.y), 
		vector2d(position.x - arrow_size, position.y + arrow_size) 
	};

	math::rotate_triangle_points(points, rotation);

	auto flags_backup = g_render->draw_list->Flags;
	g_render->draw_list->Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;

	g_render->filled_triangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, clr.new_alpha((int)(clr.a() * info->dormant_alpha)));
	g_render->triangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, outline_clr.new_alpha((int)(outline_clr.a() * info->dormant_alpha)));

	g_render->draw_list->Flags = flags_backup;
}

void c_player_esp::on_directx()
{
	const std::unique_lock< std::mutex > lock(mutexes::players);

	if (!g_cfg.visuals.esp[esp_enemy].enable)
		return;

	for (int i = 0; i < 65; ++i)
	{
		auto player_info = g_esp_store->get_player_info(i);
		if (!player_info)
			continue;

		if (!player_info->valid)
			continue;

		for (int j = 0; j < max_objs_count; ++j)
			player_info->objs[j].reset();

		auto& box = player_info->box;
		if (box.offscreen)
		{
			// esp arrows
			render_offscreen_esp(player_info);
			continue;
		}

		// box, bars, icons / flags, etc..
		render_main_esp(player_info);
	}
}