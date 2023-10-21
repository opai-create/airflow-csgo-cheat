#include "globals.hpp"
#include "grenade_prediction.hpp"
#include "entlistener.hpp"
#include "legacy ui/menu/menu.h"

void nade_path_t::perform_fly_collision_resolution(c_game_trace& trace)
{
	auto surface_elasticity = 1.f;

	if (trace.entity)
	{
		if (((c_cs_player*)trace.entity)->is_breakable())
		{
		BREAKTROUGH:
			nade_velocity *= 0.4f;

			return;
		}

		const auto is_player = ((c_cs_player*)trace.entity)->is_player();
		if (is_player)
		{
			surface_elasticity = 0.3f;
		}

		if (trace.entity->index())
		{
			if (is_player && last_hit_entity == trace.entity)
			{
				cur_collision_group = COLLISION_GROUP_DEBRIS;

				return;
			}

			last_hit_entity = trace.entity;
		}
	}

	vec3_t velocity{};

	const auto back_off = nade_velocity.dot(trace.plane.normal) * 2.f;

	for (auto i = 0u; i < 3u; i++)
	{
		const auto change = trace.plane.normal[i] * back_off;

		velocity[i] = nade_velocity[i] - change;

		if (std::fabs(velocity[i]) >= 1.f)
			continue;

		velocity[i] = 0.f;
	}

	velocity *= std::clamp< float >(surface_elasticity * 0.45f, 0.f, 0.9f);

	if (trace.plane.normal.z > 0.7f)
	{
		const auto speed_sqr = velocity.length_sqr();
		if (speed_sqr > 96000.f)
		{
			const auto l = velocity.normalized().dot(trace.plane.normal);
			if (l > 0.5f)
			{
				velocity *= 1.f - l + 0.5f;
			}
		}

		if (speed_sqr < 400.f)
		{
			nade_velocity = vec3_t(0, 0, 0);
		}
		else
		{
			nade_velocity = velocity;

			this->physics_push_entity(velocity * ((1.f - trace.fraction) * HACKS->global_vars->interval_per_tick), trace);
		}
	}
	else
	{
		nade_velocity = velocity;

		this->physics_push_entity(velocity * ((1.f - trace.fraction) * HACKS->global_vars->interval_per_tick), trace);
	}

	if (bounces_count > 20)
		return detonate< false >();

	++bounces_count;
}

bool nade_path_t::should_draw()
{
	if (path.size() <= 1u || HACKS->global_vars->curtime >= nade_expire_time)
		return false;

	last_path_pos = {};

	vec2_t out{};
	offscreen = !RENDER->world_to_screen(path.back().first, out);

	if (!offscreen)
		last_path_pos = ImVec2(out.x, out.y);

	return true;
}

void c_grenade_prediction::calc_local_nade_path()
{
	if (!HACKS->in_game || !HACKS->local || !HACKS->weapon || !HACKS->local->is_alive())
		return;

	local_path = {};

	if (!g_cfg.visuals.grenade_predict)
		return;

	if (!HACKS->weapon->is_grenade())
		return;

	if (!HACKS->weapon->pin_pulled() && HACKS->weapon->throw_time() == 0.f)
		return;

	if (!HACKS->weapon_info)
		return;

	local_path.nade_owner = HACKS->local;
	local_path.nade_idx = HACKS->weapon->item_definition_index();

	vec3_t view_angles{};
	HACKS->engine->get_view_angles(view_angles);

	if (view_angles.x < -90.f)
	{
		view_angles.x += 360.f;
	}
	else if (view_angles.x > 90.f)
	{
		view_angles.x -= 360.f;
	}

	view_angles.x -= (90.f - std::fabsf(view_angles.x)) * 10.f / 90.f;

	vec3_t direction{};
	math::angle_vectors(view_angles, direction);

	const auto throw_strength = std::clamp<float>(HACKS->weapon->throw_strength(), 0.f, 1.f);
	const auto eye_pos = HACKS->local->get_abs_origin() + HACKS->local->view_offset();
	const auto src = vec3_t(eye_pos.x, eye_pos.y, eye_pos.z + (throw_strength * 12.f - 12.f));

	c_game_trace trace{};
	HACKS->engine_trace->trace_hull(src, src + direction * 22.f, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }, MASK_SOLID | CONTENTS_CURRENT_90, HACKS->local, COLLISION_GROUP_NONE, &trace);

	vec3_t velocity = HACKS->local->abs_velocity();
	if (HACKS->local->abs_velocity().length_2d() < 10.f)
		velocity = {};

	local_path.predict_nade(
		trace.end - direction * 6.f, direction *
		(std::clamp<float>(HACKS->weapon_info->throw_velocity * 0.9f, 15.f, 750.f) *
			(throw_strength * 0.7f + 0.3f)) + velocity * 1.25f, HACKS->global_vars->curtime, 0);
}

void c_grenade_prediction::draw_local_path()
{
	if (!HACKS->in_game || !HACKS->local || !HACKS->weapon || !HACKS->local->is_alive())
		return;

	if (!g_cfg.visuals.grenade_predict)
		return;

	if (!local_path.should_draw())
		return;

	if (local_path.path.size() < 1)
		return;

	auto& first_pos = local_path.path.front();
	vec2_t nade_start{}, nade_end{};
	vec3_t prev = first_pos.first;

	auto clr = g_cfg.visuals.predict_clr.base();
	auto list = RENDER->get_draw_list();

	RESTORE(list->Flags);
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	for (const auto& it : local_path.path)
	{
		if (RENDER->world_to_screen(prev, nade_start) && RENDER->world_to_screen(it.first, nade_end))
		{
			if (it.second)
			{
				RENDER->circle_filled(nade_end.x, nade_end.y, 4.f, c_color(30, 30, 30, 200), 15);
				RENDER->circle_filled(nade_end.x, nade_end.y, 3.f, c_color(255, 255, 255, 255), 15);
			}

			RENDER->line(nade_start.x, nade_start.y, nade_end.x, nade_end.y, clr, 1.f);
		}

		prev = it.first;
	}
}

void c_grenade_prediction::calc_nade_path(c_base_combat_weapon* entity)
{
	auto client_class = entity->get_client_class();
	if (!client_class)
		return;

	if (last_server_tick != HACKS->client_state->clock_drift_mgr.server_tick)
	{
		list.clear();
		last_server_tick = HACKS->client_state->clock_drift_mgr.server_tick;
	}

	const auto handle = entity->get_ref_handle();
#ifndef LEGACY
	if (entity->explode_effect_tick_begin() || entity->nade_exploded() || client_class->class_id == CSmokeGrenadeProjectile && entity->smoke_effect_tick_begin())
	{
		list.erase(handle);
		return;
	}
#endif

	auto get_weapon_index = [&](int class_id)
	{
		switch (class_id)
		{
		case CBaseCSGrenadeProjectile:
			return WEAPON_HEGRENADE;
			break;
		case CMolotovProjectile:
			return WEAPON_MOLOTOV;
			break;
		case CSensorGrenadeProjectile:
			return WEAPON_INCGRENADE;
		case CSmokeGrenadeProjectile:
			return WEAPON_SMOKEGRENADE;
			break;
		default:
			return WEAPON_NONE;
			break;
		}
	};

	if (list.find(handle) == list.end())
	{
		int index = get_weapon_index(client_class->class_id);
		auto nade_owner = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(entity->owner()));

		list.emplace(std::piecewise_construct, std::forward_as_tuple(handle),
			std::forward_as_tuple(nade_owner, index, entity->origin(), ((c_cs_player*)entity)->velocity(), entity->grenade_spawn_time(),
				TIME_TO_TICKS(((c_cs_player*)entity)->sim_time() - entity->grenade_spawn_time())));

		list.at(handle).preview_icon = (const char*)entity->get_projectile_icon(entity->get_model(), client_class->class_id);
		list.at(handle).preview_name = entity->get_grenade_name(client_class->class_id);
	}

	if (list.at(handle).should_draw())
		return;

	list.erase(handle);
}

void c_grenade_prediction::calc_world_path()
{
	if (!HACKS->in_game)
		return;

	if (!g_cfg.visuals.grenade_warning)
	{
		if (!list.empty())
			list.clear();

		return;
	}

	list.clear();
	LISTENER_ENTITY->for_each_entity([&](c_base_entity* entity)
	{
		auto base_combat_weapon = (c_base_combat_weapon*)entity;
		auto client_class = base_combat_weapon->get_client_class();
		if (!client_class)
			return;

		auto class_id = client_class->class_id;
		if (!entity->is_projectile(class_id))
			return;

		calc_nade_path(base_combat_weapon);

	}, ENT_WEAPON);
}

void c_grenade_prediction::render_offscreen_esp(nade_path_t* info, const float& mod, const float& duration)
{
	if (!HACKS->local || !HACKS->local->is_alive())
		return;

	auto& enemy_esp = g_cfg.visuals.esp[esp_weapon];
	if (!(enemy_esp.nade_offscreen))
		return;

	vec3_t angles{};
	HACKS->engine->get_view_angles(angles);

	auto clr = enemy_esp.colors.offscreen_arrow.base();
	auto outline_clr = enemy_esp.colors.offscreen_arrow_outline.base();

	auto local_origin = HACKS->local->get_abs_origin();

	const auto& origin = info->path.back().first;
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

	auto step = duration > 0.f ? 1.f - duration : 0.f;
	auto alpha = step > 0.f ? mod * step : mod;

	float additional_pixels = 0.f;
	if (std::fabsf(RAD2DEG(rotation)) >= 90.f && std::fabsf(RAD2DEG(rotation)) <= 100.f)
		additional_pixels -= 2.f;

	RENDER->triangle_filled(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, clr.new_alpha((int)(clr.a() * alpha)));
	RENDER->triangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, outline_clr.new_alpha((int)(outline_clr.a() * alpha)));

	auto points_center = (points[0] + points[1] + points[2]) / vec2_t(3.f, 3.f);
	RENDER->text(points_center.x, points_center.y + additional_pixels, c_color{ 255, 255, 255, (int)(255 * alpha) },
		FONT_CENTERED_X | FONT_CENTERED_Y | FONT_OUTLINE | FONT_LIGHT_BACK, &RENDER->fonts.weapon_icons_large, info->preview_icon);
}

void c_grenade_prediction::draw_world_path()
{
	if (!HACKS->in_game)
		return;

	this->draw_local_path();

	if (!g_cfg.visuals.grenade_warning)
	{
		if (!list.empty())
			list.clear();

		return;
	}

	auto draw_list = RENDER->get_draw_list();

	if (list.empty())
		return;

	vec3_t local_origin = HACKS->local->get_abs_origin();

	auto base_clr = g_cfg.visuals.warning_clr.base();
	for (auto& l : list)
	{
		if (!l.first)
			continue;

		auto& data = l.second;
		auto& mod = nade_alpha[l.first];

		float grenade_duration = 0.f;

		// we don't know when decoy or smoke will arrive
		// TO-DO: calculate it manually
		if (data.nade_detonate_time)
			grenade_duration = std::clamp(std::abs(data.nade_expire_time - HACKS->global_vars->curtime) / data.nade_detonate_time, 0.f, 1.f);

		g_menu.create_animation(mod, data.path.size() > 1 && data.is_detonated, 0.3f, lerp_animation);
		g_menu.create_animation(oof_nade_alpha[l.first], l.second.offscreen && mod > 0.f, 0.3f, lerp_animation);

		if (mod <= 0.f)
			continue;

		vec3_t start_entity_origin{};
		if (HACKS->local->is_alive())
			start_entity_origin = local_origin;
		else
		{
			auto observer_target = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(HACKS->local->observer_target()));
			if (observer_target && observer_target->is_player())
				start_entity_origin = observer_target->get_abs_origin();
		}

		auto is_local_owner = data.nade_owner == HACKS->local;

		auto weapon_alpha = !is_local_owner && start_entity_origin.valid() ? 1.f - std::clamp(start_entity_origin.dist_to(data.path.back().first) / 1500.f, 0.f, 1.f) : 1.f;
		if (weapon_alpha <= 0.f)
			continue;

		if (l.second.offscreen && oof_nade_alpha[l.first] > 0.f)
		{
			render_offscreen_esp(&l.second, oof_nade_alpha[l.first], grenade_duration);
			continue;
		}
	
		auto& first_pos = data.path.front();
		auto clr = base_clr.new_alpha(base_clr.a() * mod * weapon_alpha * 0.7f);

		RESTORE(draw_list->Flags);
		draw_list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

		// grenade path
		if (is_local_owner && data.path.size() > 1 && g_cfg.visuals.grenade_warning_line)
		{
			vec2_t nade_start, nade_end;
			vec3_t prev = first_pos.first;

			for (const auto& it : data.path)
			{
				if (RENDER->world_to_screen(prev, nade_start) && RENDER->world_to_screen(it.first, nade_end))
					RENDER->line(nade_start.x, nade_start.y, nade_end.x, nade_end.y, clr, 1.f);

				prev = it.first;
			}
		}

		if (data.path.size() > 1 && data.last_path_pos.x != 0.f && data.last_path_pos.y != 0.f)
		{
			constexpr auto thickness = 2.5f;
			constexpr auto radius = 20.f;
			constexpr auto radius_second = radius - thickness;

			RENDER->circle_filled(data.last_path_pos.x, data.last_path_pos.y, radius, { 0, 0, 0, (int)(150 * mod) }, 32);

			RENDER->circle(data.last_path_pos.x, data.last_path_pos.y, radius_second, clr, 32, thickness);

			RENDER->text(data.last_path_pos.x, data.last_path_pos.y,
				c_color{ 255, 255, 255, (int)(255 * mod) }, FONT_CENTERED_X | FONT_CENTERED_Y | FONT_DROPSHADOW | FONT_LIGHT_BACK,
				&RENDER->fonts.weapon_icons_large, data.preview_icon.c_str());
		}
	}
}