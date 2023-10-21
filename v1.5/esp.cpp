#include "globals.hpp"
#include "entlistener.hpp"
#include "animations.hpp"
#include "engine_prediction.hpp"
#include "esp_object_render.hpp"
#include "render.hpp"
#include "penetration.hpp"
#include "esp.hpp"
#include "resolver.hpp"
#include "legacy ui/menu/menu.h"

constexpr auto MOLOTOV_ICON = (u8"\uE02E");
constexpr auto SMOKE_ICON = (u8"\uE02D");
constexpr auto MAX_ESP_DISTANCE = 1500.f;
const auto TIMER_SIZE = vec2_t{ 60.f, 2.f };

#define EMPLACE_OBJECT(...) esp.objects[iter++] = esp_object_t{ __VA_ARGS__ }
#define LERP_ALPHA(amount) esp.alpha = std::clamp(std::lerp(esp.alpha, amount, RENDER->get_animation_speed() * 5.f), 0.f, 1.f);

INLINE box_t get_entity_bounding_box(c_base_entity* entity)
{
	box_t out{};

	vec3_t min = entity->bb_mins();
	vec3_t max = entity->bb_maxs();

	vec3_t points[8] = {
		{ min.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, max.z },
		{ min.x, max.y, max.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z }
	};

	int valid_bounds = 0;
	vec2_t points_to_screen[8] = {};
	for (int i = 0; i < 8; i++)
	{
		if (!RENDER->world_to_screen(math::get_vector_transform(points[i], entity->coordinate_frame()), points_to_screen[i]))
			continue;

		valid_bounds++;
	}

	out.offscreen = valid_bounds == 0;
	if (out.offscreen)
		return out;

	float left = points_to_screen[3].x;
	float top = points_to_screen[3].y;
	float right = points_to_screen[3].x;
	float bottom = points_to_screen[3].y;

	for (auto i = 1; i < 8; i++)
	{
		if (left > points_to_screen[i].x)
			left = points_to_screen[i].x;
		if (top < points_to_screen[i].y)
			top = points_to_screen[i].y;
		if (right < points_to_screen[i].x)
			right = points_to_screen[i].x;
		if (bottom > points_to_screen[i].y)
			bottom = points_to_screen[i].y;
	}

	out.x = left;
	out.y = bottom;
	out.w = right - left;
	out.h = top - bottom;
	return out;
}

INLINE box_t get_player_bounding_box(c_cs_player* player, esp_player_t& esp)
{
	box_t out{};

	auto abs_origin = esp.origin;
	auto center = abs_origin + math::reversed_lerp(0.5f, esp.mins, esp.maxs);

	float stand_pose = esp.poses[1];
	float body_pitch = esp.poses[12];

	float offset_top = stand_pose * 18.f + 58.f - body_pitch * 6.f;
	float offset_base = esp.maxs.z * -0.6f;

	auto top = vec3_t{ center.x, center.y, abs_origin.z + offset_top };
	auto base = vec3_t{ center.x, center.y, center.z + offset_base };

	vec2_t screen[2]{ };
	vec3_t world[2]{ top, base };

	int valid_bounds = 0;
	for (int i = 0; i < 2; i++)
	{
		if (!RENDER->world_to_screen(world[i], screen[i]))
			continue;

		++valid_bounds;
	}

	out.offscreen = valid_bounds == 0;
	if (out.offscreen)
		return out;

	float y2 = screen[0].y + std::abs(screen[0].y - screen[1].y);
	float height = y2 - screen[0].y;

	float cur_w = screen[1].x - screen[0].x;
	float center_x = screen[0].x + cur_w / 2.f;

	float w = height / 2.f;
	float x1 = center_x - w / 2.f;
	float x2 = center_x + w / 2.f;

	out.x = x1;
	out.y = screen[0].y;
	out.w = x2 - x1;
	out.h = y2 - screen[0].y;

	return out;
}

INLINE std::vector<ImVec2> get_world_position(const vec3_t& pos, float radius)
{
	constexpr float step = M_PI * 2.0f / 60.f;

	std::vector<ImVec2> points{};
	for (float lat = 0.f; lat <= M_PI * 2.f; lat += step)
	{
		const auto& point3d = vec3_t(std::sin(lat), std::cos(lat), 0.f) * radius;

		vec2_t point2d{};
		if (RENDER->world_to_screen(pos + point3d, point2d, true))
			points.emplace_back(ImVec2(point2d.x, point2d.y));
	}

	return points;
}

void c_esp::draw_smoke_range(float& weapon_alpha, weapon_esp_t& esp, c_base_entity* entity)
{
	auto& esp_config = g_cfg.visuals.esp[esp_weapon];
	auto smoke_color = esp_config.colors.smoke_range.base();

	auto smoke_timer = esp.smoke_time / 18.f;
	g_menu.create_animation(esp.range_lerp, smoke_timer > 0.1f, 0.3f, lerp_animation);

	auto base_origin = entity->get_abs_origin();
	auto world_position = get_world_position(base_origin, 160.5f * esp.range_lerp);

	vec2_t origin{};
	if (!RENDER->world_to_screen(base_origin, origin))
		return;

	auto base_alpha = weapon_alpha * esp.range_lerp;
	auto total_pos = vec2_t{ origin.x - TIMER_SIZE.x / 2, origin.y - TIMER_SIZE.y };
	auto ease_text = std::sin(HACKS->system_time() * 2.f * M_PI) * 5.f;

	if (weapon_alpha > 0.75f)
	{
		auto list = RENDER->get_draw_list();

		RESTORE(list->Flags);
		list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
		list->AddPolyline(world_position.data(), world_position.size(), smoke_color.new_alpha((int)(smoke_color.a() * base_alpha)).as_imcolor(), false, 2.f);
	}

	RENDER->filled_rect(total_pos.x - 1, total_pos.y - 1 - ease_text, TIMER_SIZE.x + 2, TIMER_SIZE.y + 2, c_color(0, 0, 0, (int)(255 * 0.6f * base_alpha)));
	RENDER->filled_rect(total_pos.x, total_pos.y - ease_text, (TIMER_SIZE.x * esp.smoke_time) / 18.f, TIMER_SIZE.y, smoke_color.new_alpha((int)(255 * base_alpha)));

	RENDER->text(origin.x, origin.y - 30.f - ease_text, { 255, 255, 255, (int)(255 * base_alpha) }, FONT_CENTERED_X | FONT_DROPSHADOW | FONT_LIGHT_BACK,
		&RENDER->fonts.weapon_icons_large, (const char*)SMOKE_ICON);
}

void c_esp::draw_molotov_range(float& weapon_alpha, weapon_esp_t& esp, c_base_entity* entity)
{
	auto& esp_config = g_cfg.visuals.esp[esp_weapon];
	auto fire_color = esp_config.colors.molotov_range.base();

	vec3_t base_origin{};
	bool* m_bFireIsBurning = entity->m_bFireIsBurning();
	int* m_fireXDelta = entity->fire_x_delta();
	int* m_fireYDelta = entity->fire_y_delta();
	int* m_fireZDelta = entity->fire_z_delta();
	int m_fireCount = entity->fire_coint();

	float inferno_range = -1.f;
	auto average_vector = vec3_t{ 0, 0, 0 };
	for (int i = 0; i <= m_fireCount; i++)
	{
		if (!m_bFireIsBurning[i])
			continue;

		auto fire_origin = vec3_t( m_fireXDelta[i], m_fireYDelta[i], m_fireZDelta[i] );
		float delta = fire_origin.length_2d() + 14.4f;
		if (delta > inferno_range)
			inferno_range = delta;

		average_vector += fire_origin;
	}

	if (m_fireCount <= 1)
		base_origin = entity->get_abs_origin();
	else
		base_origin = (average_vector / m_fireCount) + entity->get_abs_origin();

	auto world_position = get_world_position(base_origin, std::max(100.f, inferno_range) * esp.range_lerp);

	constexpr auto fire_time = 7.03125f;

	auto fire_timer = esp.inferno_time / fire_time;
	g_menu.create_animation(esp.range_lerp, fire_timer > 0.1f, 0.3f, lerp_animation);

	vec2_t origin{};
	if (!RENDER->world_to_screen(base_origin, origin))
		return;
	
	auto base_alpha = weapon_alpha * esp.range_lerp;
	auto total_pos = vec2_t{ origin.x - TIMER_SIZE.x / 2, origin.y - TIMER_SIZE.y };
	auto ease_text = std::sin(HACKS->system_time() * 2.f * M_PI) * 5.f;

	if (weapon_alpha > 0.75f)
	{
		auto list = RENDER->get_draw_list();

		RESTORE(list->Flags);
		list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
		list->AddPolyline(world_position.data(), world_position.size(), fire_color.new_alpha((int)(fire_color.a() * base_alpha)).as_imcolor(), false, 2.f);
	}

	RENDER->filled_rect(total_pos.x - 1, total_pos.y - 1 - ease_text, TIMER_SIZE.x + 2, TIMER_SIZE.y + 2, c_color(0, 0, 0, (int)(255 * 0.6f * base_alpha)));
	RENDER->filled_rect(total_pos.x, total_pos.y - ease_text, (TIMER_SIZE.x * esp.inferno_time) / fire_time, TIMER_SIZE.y, fire_color.new_alpha((int)(255 * base_alpha)));

	RENDER->text(origin.x, origin.y - 30.f - ease_text, { 255, 255, 255, (int)(255 * base_alpha) }, FONT_CENTERED_X | FONT_DROPSHADOW | FONT_LIGHT_BACK,
		&RENDER->fonts.weapon_icons_large, (const char*)MOLOTOV_ICON);
}

void c_esp::draw_weapon_esp()
{
	auto& esp_config = g_cfg.visuals.esp[esp_weapon];
	if (!esp_config.enable)
		return;

	vec3_t local_origin = HACKS->local->get_abs_origin();

	auto box_color = esp_config.colors.box.base();
	auto name_color = esp_config.colors.name.base();
	auto ammo_color = esp_config.colors.ammo_bar.base();

	LISTENER_ENTITY->for_each_entity([&](c_base_entity* entity)
	{
		auto client_class = entity->get_client_class();
		if (!client_class)
			return;

		auto class_id = client_class->class_id;

		auto base_combat_weapon = (c_base_combat_weapon*)entity;
		auto misc_weapon = entity->is_grenade(class_id);
		auto projectile = entity->is_projectile(class_id) || class_id == CDecoyProjectile;

		auto& esp = weapon_esp[base_combat_weapon->index()];
		if (base_combat_weapon->owner() != -1 && class_id != CPlantedC4 && !projectile && class_id != CInferno)
		{
			esp.reset();
			return;
		}

		vec3_t start_entity_origin{};
		if (HACKS->local->is_alive())
			start_entity_origin = local_origin;
		else
		{
			auto observer_target = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(HACKS->local->observer_target()));
			if (observer_target && observer_target->is_player())
				start_entity_origin = observer_target->get_abs_origin();
		}

		auto weapon_alpha = start_entity_origin.valid() ? 1.f - std::clamp(start_entity_origin.dist_to(base_combat_weapon->get_abs_origin()) / MAX_ESP_DISTANCE, 0.f, 1.f) : 1.f;
		if (weapon_alpha <= 0.f)
		{
			esp.reset();
			return;
		}

		if (misc_weapon)
		{
			bool render_range_only = false;

			switch (class_id)
			{
			case CSmokeGrenadeProjectile:
			{
				if (entity->did_smoke_effect())
				{
					render_range_only = true;
					esp.did_smoke = true;

					float tick_time = (entity->smoke_effect_tick_begin() * HACKS->global_vars->interval_per_tick) + 17.5f;
					esp.smoke_time = tick_time - HACKS->global_vars->curtime;
					esp.inferno_time = 0.f;
				}
			}
			break;
			case CInferno:
			{
				render_range_only = true;
				esp.did_smoke = false;
				esp.smoke_time = 0.f;
				esp.inferno_time = (((*(float*)((std::uintptr_t)entity + 0x20)) + 7.03125f) - HACKS->global_vars->curtime);
			}
			break;
			}

#ifndef LEGACY
			if (entity->nade_exploded() && !esp.did_smoke && class_id != CInferno)
			{
				esp.reset();
				return;
			}
#endif

			if ((esp_config.elements & 16) && class_id == CInferno)
				draw_molotov_range(weapon_alpha, esp, entity);

			if ((esp_config.elements & 32) && esp.did_smoke)
				draw_smoke_range(weapon_alpha, esp, entity);

			if (render_range_only)
				return;
		}

		box_t box = get_entity_bounding_box(base_combat_weapon);
		if (box.offscreen)
		{
			esp.reset();
			return;
		}

		auto model = entity->get_model();
		if (!model)
		{
			esp.reset();
			return;
		}

		for (int i = 0; i < MAX_ESP_OBJECTS; ++i)
			esp.objects[i].reset();

		if (esp_config.elements & 1)
		{
			auto new_color = box_color.new_alpha((int)(box_color.a() * weapon_alpha));

			RENDER->rect(box.x - 1, box.y - 1, box.w + 2, box.h + 2, c_color{ 0, 0, 0, (int)(new_color.a() * 0.8f) });
			RENDER->rect(box.x + 1, box.y + 1, box.w - 2, box.h - 2, c_color{ 0, 0, 0, (int)(new_color.a() * 0.8f) });
			RENDER->rect(box.x, box.y, box.w, box.h, new_color);
		}

		int iter = 0;

		if (!misc_weapon)
		{
			auto weapon_info = HACKS->weapon_system->get_weapon_data(base_combat_weapon->item_definition_index());
			if (!weapon_info)
			{
				esp.reset();
				return;
			}

			if (esp_config.elements & 8)
				EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_DOWN, (float)base_combat_weapon->clip1(), (float)weapon_info->max_ammo_1,
					weapon_alpha, ammo_color, { 0, 0, 0, 255 });
		}

		if (esp_config.elements & 4)
		{
			std::string weapon_name{};
			if (class_id == CPlantedC4)
				weapon_name = XOR("C4");
			else
				weapon_name = misc_weapon ? base_combat_weapon->get_grenade_name(class_id) : base_combat_weapon->get_weapon_name();

			EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_DOWN, 0.f, 0.f, weapon_alpha, name_color, { 0, 0, 0, 255 }, weapon_name);
		}

		if ((esp_config.elements & 2) && class_id != CPlantedC4)
		{
			auto weapon_icon = misc_weapon ? base_combat_weapon->get_projectile_icon(model, class_id) : base_combat_weapon->get_weapon_icon();
			EMPLACE_OBJECT(true, FONT_DROPSHADOW | FONT_LIGHT_BACK, FONT_ICON, ESP_POS_DOWN, 0.f, 0.f, weapon_alpha, name_color, { 0, 0, 0, 255 }, (const char*)weapon_icon);
		}

		{
			int bar_offsets[4]{ };
			float string_offsets[4]{ };

			for (int i = 0; i < MAX_ESP_OBJECTS; ++i)
			{
				if (!esp.objects[i].valid)
					continue;

				auto& position = esp.objects[i].position;
				if (position == -1)
					continue;

				esp_objects::render_bars(box, esp.objects[i], bar_offsets[position]);
			}

			// ghetto method: grab latest bar offset and apply it to next esp strings
			for (int i = 0; i < MAX_ESP_OBJECTS; ++i)
			{
				if (!esp.objects[i].valid)
					continue;

				auto& position = esp.objects[i].position;
				if (position == -1)
					continue;

				esp_objects::render_strings(box, esp.objects[i], bar_offsets[position], string_offsets[position]);
			}
		}

	}, ENT_WEAPON);
}

void c_esp::render_offscreen_esp(esp_player_t& esp)
{
	if (esp.alpha <= 0.5f)
		return;

	if (!HACKS->local || !HACKS->local->is_alive())
		return;

	auto& enemy_esp = g_cfg.visuals.esp[esp_enemy];
	if (!(enemy_esp.elements & 128))
		return;

	vec3_t angles{};
	HACKS->engine->get_view_angles(angles);

	auto clr = enemy_esp.colors.offscreen_arrow.base();
	auto outline_clr = enemy_esp.colors.offscreen_arrow_outline.base();

	auto local_origin = HACKS->local->get_abs_origin();

	const auto& origin = esp.origin;
	int arrow_size = enemy_esp.offscreen_size + 5.f;
	float arrow_distance = enemy_esp.offscreen_dist + 10.f;

	float rotation = DEG2RAD(math::normalize_yaw(angles.y - math::calc_angle(local_origin, origin).y - 90.f));

	vec2_t center = vec2_t(RENDER->screen.x / 2, RENDER->screen.y / 2);

	vec2_t position =
	{
		center.x + arrow_distance * std::cos(rotation),
		center.y + arrow_distance * std::sin(rotation)
	};

	vec2_t points[3] =
	{
		vec2_t(position.x - arrow_size, position.y - arrow_size),
		vec2_t(position.x + arrow_size, position.y),
		vec2_t(position.x - arrow_size, position.y + arrow_size)
	};

	math::rotate_triangle_points(points, rotation);

	auto alpha = esp.alpha;

	RENDER->triangle_filled(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, clr.new_alpha((int)(clr.a() * alpha)));
	RENDER->triangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, outline_clr.new_alpha((int)(outline_clr.a() * alpha)));
}

void c_esp::draw_player_esp()
{
	auto& entity_visuals = g_cfg.visuals.esp[esp_enemy];
	auto box_color = entity_visuals.colors.box.base();
	auto name_color = entity_visuals.colors.name.base();
	auto weapon_color = entity_visuals.colors.weapon.base();
	auto ammo_color = entity_visuals.colors.ammo_bar.base();
	auto health_color = entity_visuals.colors.health.base();
	auto skeleton_color = entity_visuals.colors.skeleton.base();

	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
		auto deref = *(std::uintptr_t*)player;
		if (deref == NULL || deref == 0x01000100)
			return;

		auto index = player->index();
		auto& esp = player_esp[index];
		
		if (!HACKS->local->is_alive())
		{
			auto observer_target = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(HACKS->local->observer_target()));
			if (observer_target && observer_target == player && HACKS->local->observer_mode() == OBS_MODE_IN_EYE)
			{
				// don't draw esp in first person of spectator
				esp.reset();
				return;
			}
		}

		auto esp_index = 0;
		if (!entity_visuals.enable)
		{
			esp.reset();
			return;
		}

		if (player->observer_mode() == OBS_MODE_IN_EYE || player->observer_mode() == OBS_MODE_CHASE)
		{
			auto observer_target = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(player->observer_target()));
			if (observer_target && observer_target->is_alive())
			{
				auto& observer_esp = player_esp[observer_target->index()];
				observer_esp.valid = true;

				auto& observer_dormant = observer_esp.dormant;
				observer_dormant.update(observer_target);
			}
		}

		if (!player->is_alive())
		{
			if (esp.alpha > 0.f)
				LERP_ALPHA(0.f);
		}
		else
		{
			if (player->dormant())
			{
				auto& dormant = esp.dormant;

				if (std::abs(dormant.time - HACKS->global_vars->curtime) > 5.f)
				{
					if (esp.alpha > 0.f)
						LERP_ALPHA(0.f);
				}
				else
					LERP_ALPHA(0.45f);

				esp.origin = dormant.origin;

				if (dormant.maxs.valid())
					esp.maxs = dormant.maxs;
				else
					esp.maxs = vec3_t{ 16.f, 16.f, 72.f };

				if (dormant.maxs.valid())
					esp.mins = dormant.mins;
				else
					esp.mins = vec3_t{ -16.f, -16.f, 0.f };

				if (dormant.duck_amount != -1.f)
					esp.duck_amount = dormant.duck_amount;
			}
			else
			{
				if (esp.alpha < 1.f)
					LERP_ALPHA(1.f);

				esp.dormant.update(player);
				esp.update(player);
			}
		}

		if (!esp.origin.valid())
		{
			esp.reset();
			return;
		}

		esp.box = get_player_bounding_box(player, esp);
		if (esp.box.offscreen)
		{
			render_offscreen_esp(esp);
			return;
		}

		if (esp.alpha <= 0.f)
		{
			esp.reset();
			return;
		}

		esp.valid = true;

		auto& bbox = esp.box;
		if (entity_visuals.elements & 1)
		{
			RENDER->rect(bbox.x - 1, bbox.y - 1, bbox.w + 2, bbox.h + 2, { 0, 0, 0, (int)(150 * esp.alpha) }, 1.f);
			RENDER->rect(bbox.x + 1, bbox.y + 1, bbox.w - 2, bbox.h - 2, { 0, 0, 0, (int)(150 * esp.alpha) }, 1.f);
			RENDER->rect(bbox.x, bbox.y, bbox.w, bbox.h, box_color.new_alpha((int)(box_color.a() * esp.alpha)), 1.f);
		}

		for (int i = 0; i < MAX_ESP_OBJECTS; ++i)
			esp.objects[i].reset();

		// store esp objects 
		{
			int iter = 0;
			
			if (entity_visuals.elements & 2)
				EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_UP, 0.f, 0.f, esp.alpha, name_color, { 0, 0, 0, 255 }, player->get_name());

			if (entity_visuals.elements & 4)
			{
				auto esp_step = 1.f - (std::clamp((float)esp.health, 0.f, 100.f) / 100.f);
				auto lerp_color = health_color.multiply({ 255, 33, 33 }, esp_step).new_alpha((int)(health_color.a() * esp.alpha));

				EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_LEFT, (float)esp.health, 100.f, (float)((health_color.a() * esp.alpha) / 255.f), lerp_color, {0, 0, 0, 255}, "");
			}

			if (entity_visuals.elements & 16)
				EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_DOWN, 0.f, 0.f, esp.alpha, weapon_color, { 0, 0, 0, 255 }, esp.weapon_name);

			if (entity_visuals.elements & 8)
				EMPLACE_OBJECT(true, FONT_DROPSHADOW | FONT_LIGHT_BACK, FONT_ICON, ESP_POS_DOWN, 0.f, 0.f, esp.alpha, weapon_color, { 0, 0, 0, 255 }, esp.weapon_icon);

			if (entity_visuals.elements & 32)
				EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_DOWN, (float)esp.ammo, (float)esp.max_ammo, (float)((ammo_color.a() * esp.alpha) / 255.f), ammo_color, { 0, 0, 0, 255 }, "");

			{
				if (esp.alpha > 0.5f && esp.health > 0)
				{
					/*if (entity_visuals.elements & 512)
					{
						auto studio_hdr = HACKS->model_info->get_studio_model(player->get_model());
						if (studio_hdr)
						{
							auto bone_cache = player->bone_cache().base();
							for (int j = 0; j < studio_hdr->bones; j++)
							{
								auto bone = studio_hdr->bone(j);
								if (!bone)
									continue;

								auto name = bone->get_name();
								if (bone->flags > 0 && (bone->flags & 0x100) && bone->parent != -1)
								{
									vec2_t parent{}, child{};
									if (RENDER->world_to_screen(bone_cache[j].get_origin(), child, true)
										&& RENDER->world_to_screen(bone_cache[bone->parent].get_origin(), parent, true))
									{
										RENDER->line(parent.x, parent.y, child.x, child.y, skeleton_color.new_alpha((int)(skeleton_color.a() * esp.alpha)));
									}
								}
							}
						}
					}*/

					if (entity_visuals.elements & 64)
					{
						if (player->armor_value() > 0)
							EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_RIGHT, 0.f, 0.f, esp.alpha, { 255, 255, 255, 255 }, { 0, 0, 0, 255 }, player->has_helmet() ? XOR("HK") : XOR("K"));

						if (player->is_scoped())
							EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_RIGHT, 0.f, 0.f, esp.alpha, { 255, 255, 255, 255 }, { 0, 0, 0, 255 }, XOR("ZOOM"));

						if (player->has_defuser())
							EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_RIGHT, 0.f, 0.f, esp.alpha, { 255, 255, 255, 255 }, { 0, 0, 0, 255 }, XOR("KIT"));

						if (HACKS->player_resource && HACKS->player_resource->player_c4_index() == player->index())
							EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_RIGHT, 0.f, 0.f, esp.alpha, { 255, 255, 255, 255 }, { 0, 0, 0, 255 }, XOR("C4"));

						if (esp.planting)
							EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_RIGHT, 0.f, 0.f, esp.alpha, { 255, 255, 255, 255 }, { 0, 0, 0, 255 }, XOR("PLANTING"));
					}

#if _DEBUG || ALPHA || BETA
					if (entity_visuals.elements & 512)
					{
						auto& info = resolver_info[player->index()];
						EMPLACE_OBJECT(true, FONT_OUTLINE | FONT_LIGHT_BACK, FONT_PIXEL, ESP_POS_RIGHT, 0.f, 0.f, esp.alpha, { 255, 255, 255, 255 }, { 0, 0, 0, 255 }, info.mode);
					}
#endif
				}
			}
		}

		// render esp objects on screen
		{
			int bar_offsets[4]{ };
			float string_offsets[4]{ };

			for (int i = 0; i < MAX_ESP_OBJECTS; ++i)
			{
				if (!esp.objects[i].valid)
					continue;

				auto& position = esp.objects[i].position;
				if (position == -1)
					continue;

				esp_objects::render_bars(bbox, esp.objects[i], bar_offsets[position]);
			}

			// ghetto method: grab latest bar offset and apply it to next esp strings
			for (int i = 0; i < MAX_ESP_OBJECTS; ++i)
			{
				if (!esp.objects[i].valid)
					continue;

				auto& position = esp.objects[i].position;
				if (position == -1)
					continue;

				esp_objects::render_strings(bbox, esp.objects[i], bar_offsets[position], string_offsets[position]);
			}
		}
	}, true);
}

bullet_t bullet_for_indicator()
{
	vec3_t angles{};
	HACKS->engine->get_view_angles(angles);

	vec3_t direction{};
	math::angle_vectors(angles, direction);

	auto eye_pos = HACKS->local->origin() + HACKS->local->view_offset();
	return penetration::simulate(HACKS->local, nullptr, eye_pos, eye_pos + (direction * HACKS->weapon_info->range), true, true);
}

void c_esp::render_local()
{
	if (!HACKS->in_game || !HACKS->local || !HACKS->weapon || !HACKS->local->is_alive())
		return;

	if (HACKS->local->is_scoped() && HACKS->weapon->is_sniper() && (g_cfg.misc.removals & scope))
	{
		RENDER->line(-1, RENDER->screen.y / 2, RENDER->screen.x, RENDER->screen.y / 2, { 0, 0, 0, 255 });
		RENDER->line(RENDER->screen.x / 2.f, -1, RENDER->screen.x / 2.f, RENDER->screen.y, { 0, 0, 0, 255 });
	}

	if (g_cfg.misc.pen_xhair && !HACKS->weapon->is_misc_weapon())
	{
		constexpr auto box_size = 3.f;

		auto thread_id = THREAD_POOL->add_task(bullet_for_indicator);
		auto bullet = std::any_cast<bullet_t>(THREAD_POOL->wait_result(thread_id));
		auto crosshair_color = bullet.damage > 0 ? c_color{0, 255, 0 } : c_color{ 255, 0, 0 };

		auto center = vec2_t{ RENDER->screen.x / 2, RENDER->screen.y / 2 };
		auto box_half = box_size * 0.5f;

		RENDER->filled_rect(center.x - 1.f, center.y - 1.f, 3.f, 3.f, c_color{ 20, 20, 20, 100 });

		RENDER->line(center.x - 1.f, center.y, center.x + 2.f, center.y, crosshair_color);
		RENDER->line(center.x, center.y - 1.f, center.x, center.y + 2.f, crosshair_color);
	}
}

void c_esp::render()
{
	if (HACKS->client_state->delta_tick == -1 || !HACKS->in_game || !HACKS->local)
		return;

	auto list = RENDER->get_draw_list();

	RESTORE(list->Flags);
	list->Flags = ImDrawListFlags_None;

	draw_weapon_esp();
	draw_player_esp();
}