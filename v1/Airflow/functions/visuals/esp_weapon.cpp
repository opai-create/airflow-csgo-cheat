#include "esp_weapon.h"
#include "../features.h"

const auto nade_bar_size = vector2d(60.f, 2.f);

constexpr auto molotov_icon = u8"\uE02E";
constexpr auto smoke_icon = u8"\uE02D";
constexpr float fire_duration = 7.03125f;
constexpr float smoke_duration = 18.f;

std::vector< ImVec2 > get_world_position(const vector3d& pos, float radius)
{
	constexpr float step = PI * 2.0f / 60.f;

	std::vector< ImVec2 > points{};

	for (float lat = 0.f; lat <= PI * 2.f; lat += step)
	{
		const auto& point3d = vector3d(std::sin(lat), std::cos(lat), 0.f) * radius;

		vector2d point2d{};
		if (g_render->world_to_screen(pos + point3d, point2d, true))
			points.emplace_back(ImVec2(point2d.x, point2d.y));
	}

	return points;
}

void render_weapon_esp(c_esp_store::world_info_t* info)
{

	int bar_offsets[4]{};
	float string_offsets[4]{};

	int iter = 0;

	auto& box = info->box;
	auto& weapon_esp = g_cfg.visuals.esp[esp_weapon];

	float distance_alpha = info->alpha / 255.f;
	if (weapon_esp.elements & 1)
	{
		auto base_clr = weapon_esp.colors.box.base();

		auto box_color = base_clr.new_alpha(base_clr.a() * distance_alpha);

		g_render->rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, color(0, 0, 0, box_color.a() * 0.8f));
		g_render->rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, color(0, 0, 0, box_color.a() * 0.8f));
		g_render->rect(box.x, box.y, box.w, box.h, box_color);
	}

	if (weapon_esp.elements & 2)
	{
		auto base_clr = weapon_esp.colors.name.base();

		auto name_color = base_clr.new_alpha(base_clr.a() * distance_alpha);
		info->objs[iter++] = esp_objects_t{ true, esp_down, name_color, 0.f, 0.f, info->icon, font_icon, true };
	}

	if (weapon_esp.elements & 4)
	{
		auto base_clr = weapon_esp.colors.name.base();

		auto name_color = base_clr.new_alpha(base_clr.a() * distance_alpha);
		info->objs[iter++] = esp_objects_t{ true, esp_down, name_color, 0.f, 0.f, info->name, font_pixel };
	}

	if (weapon_esp.elements & 8)
	{
		auto base_clr = weapon_esp.colors.ammo_bar.base();

		auto ammo_color = base_clr.new_alpha(base_clr.a() * distance_alpha);
		info->objs[iter++] = esp_objects_t{ true, esp_down, ammo_color, info->ammo, info->ammo_max };
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

void render_nade_range(float radius, float timer, const vector3d& origin, const color& clr, const char* icon, float anim, float anim2, float alpha)
{
	vector2d screen_origin{};
	if (!g_render->world_to_screen(origin, screen_origin, true))
		return;

	const auto& points = get_world_position(origin, radius);

	g_render->draw_list->AddConvexPolyFilled(points.data(), points.size(), clr.new_alpha(clr.a() * 0.8f * alpha).as_imcolor());

	g_render->draw_list->AddPolyline(points.data(), points.size(), clr.new_alpha(clr.a() * alpha).as_imcolor(), true, 2.f);

	g_render->string(screen_origin.x + 3.f, screen_origin.y - 40.f - anim, color(255, 255, 255, 255 * alpha), centered_x | dropshadow_, g_fonts.weapon_icons_large, icon);

	g_render->draw_list->PathArcTo(ImVec2(screen_origin.x + 15.f, screen_origin.y - 17.f - anim), 5.f, 0.f, 2.f * PI, 32);
	g_render->draw_list->PathStroke(color(40, 40, 40, 200 * alpha).as_imcolor(), false, 4.f);

	g_render->draw_list->PathArcTo(ImVec2(screen_origin.x + 15.f, screen_origin.y - 17.f - anim), 5.f, 0.f, 2.f * PI * timer, 32);
	g_render->draw_list->PathStroke(clr.new_alpha(255 * alpha).as_imcolor(), false, 2.f);

	vector2d positions[3]{
	  { screen_origin.x - 5.f, screen_origin.y - 10.f - anim2 },
	  { screen_origin.x + 5.f, screen_origin.y - 10.f - anim2 },
	  { screen_origin.x, screen_origin.y - anim2 },
	};

	g_render->filled_triangle(positions[0].x, positions[0].y, positions[1].x, positions[1].y, positions[2].x, positions[2].y, color(255, 255, 255, 255 * alpha));

	g_render->triangle(positions[0].x, positions[0].y, positions[1].x, positions[1].y, positions[2].x, positions[2].y, color(0, 0, 0, 100 * alpha), 2.f);
}

void c_weapon_esp::on_directx()
{
	const std::unique_lock< std::mutex > lock(mutexes::weapons);

	if (!g_cfg.visuals.esp[esp_weapon].enable)
		return;

	float ease_text = std::sin(g_ctx.system_time() * 2.f * M_PI) * 5.f;
	float ease_triangle = std::sin(g_ctx.system_time() * 2.f * M_PI) * 4.f;

	auto old_flags = g_render->draw_list->Flags;
	g_render->draw_list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	for (auto& weapon_info : g_esp_store->weaponinfo)
	{
		if (weapon_info.proj)
		{
			if (weapon_info.class_id == CInferno && g_cfg.visuals.esp[esp_weapon].elements & 16)
			{
				float timer = weapon_info.expire_inferno / fire_duration;
				g_menu->create_animation(weapon_info.ease_inferno, timer > 0.f, 0.3f, lerp_animation);

				float range = std::max(100.f, weapon_info.inferno_range) * weapon_info.ease_inferno;

				auto molotov_color = g_cfg.visuals.esp[esp_weapon].colors.molotov_range.base();
				render_nade_range(range, timer, weapon_info.origin, molotov_color, (const char*)molotov_icon, ease_text, ease_triangle, weapon_info.ease_inferno);
			}

			if (weapon_info.did_smoke && g_cfg.visuals.esp[esp_weapon].elements & 32)
			{
				float timer = weapon_info.expire_smoke / smoke_duration;
				g_menu->create_animation(weapon_info.ease_smoke, timer > 0.f, 0.3f, lerp_animation);

				float range = 160.5f * weapon_info.ease_smoke;

				auto smoke_color = g_cfg.visuals.esp[esp_weapon].colors.smoke_range.base();
				render_nade_range(range, timer, weapon_info.origin, smoke_color, (const char*)smoke_icon, ease_text, ease_triangle, weapon_info.ease_smoke);
			}
		}

		if (weapon_info.did_smoke || !weapon_info.valid)
			continue;

		for (int j = 0; j < max_objs_count; ++j)
			weapon_info.objs[j].reset();

		// box, bars, icons / flags, etc..
		render_weapon_esp(&weapon_info);
	}

	g_render->draw_list->Flags = old_flags;
}