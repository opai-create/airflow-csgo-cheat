#include "../features.h"
#include "../config_vars.h"

#include "grenade_warning.h"

class c_handle_entity;

void render_weapon_offscreen_esp(c_grenade_warning::nade_path_t* info, const float& mod, const float& duration)
{
	if (!g_ctx.is_alive)
		return;

	auto& enemy_esp = g_cfg.visuals.esp[esp_weapon];
	if (!(enemy_esp.nade_offscreen))
		return;

	auto clr = enemy_esp.colors.offscreen_arrow.base();
	auto outline_clr = enemy_esp.colors.offscreen_arrow_outline.base();

	vector3d local_origin = g_ctx.abs_origin;

	const vector3d& origin = info->path.back().first;
	int arrow_size = enemy_esp.offscreen_size + 5.f;
	float arrow_distance = enemy_esp.offscreen_dist + 10.f;

	float rotation = math::normalize(math::deg_to_rad(g_ctx.orig_angle.y - math::angle_from_vectors(local_origin, origin).y - 90.f));

	auto flags_backup = g_render->draw_list->Flags;
	g_render->draw_list->Flags |= ImDrawListFlags_AntiAliasedLines | ImDrawListFlags_AntiAliasedFill;
	{
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

		auto step = duration > 0.f ? 1.f - duration : 0.f;
		auto alpha = step > 0.f ? mod * step : mod;

		float additional_pixels = 0.f;
		if (std::fabsf(rotation) >= 90.f && std::fabsf(rotation) <= 100.f)
			additional_pixels -= 2.f;

		g_render->filled_triangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, clr.new_alpha((int)(clr.a() * alpha)));
		g_render->triangle(points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y, outline_clr.new_alpha((int)(outline_clr.a() * alpha)));

		auto points_center = (points[0] + points[1] + points[2]) / vector2d(3.f, 3.f);
		g_render->string(points_center.x, points_center.y + additional_pixels, color{ 255, 255, 255, (int)(255 * alpha) }, centered_x | centered_y | outline_light, g_fonts.weapon_icons_large, info->preview_icon);
	}

	g_render->draw_list->Flags = flags_backup;
}

void c_grenade_warning::nade_path_t::perform_fly_collision_resolution(trace_t& trace)
{
	auto surface_elasticity = 1.f;

	if (trace.entity)
	{
		if (g_auto_wall->is_breakable_entity((c_csplayer*)trace.entity))
		{
		BREAKTROUGH:
			nade_velocity *= 0.4f;

			return;
		}

		const auto is_player = ((c_csplayer*)trace.entity)->is_player();
		if (is_player)
		{
			surface_elasticity = 0.3f;
		}

		if (trace.entity->index())
		{
			if (is_player && last_hit_entity == trace.entity)
			{
				cur_collision_group = collision_group_debris;

				return;
			}

			last_hit_entity = trace.entity;
		}
	}

	auto velocity = vector3d();

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
			nade_velocity = vector3d(0, 0, 0);
		}
		else
		{
			nade_velocity = velocity;

			this->physics_push_entity(velocity * ((1.f - trace.fraction) * interfaces::global_vars->interval_per_tick), trace);
		}
	}
	else
	{
		nade_velocity = velocity;

		this->physics_push_entity(velocity * ((1.f - trace.fraction) * interfaces::global_vars->interval_per_tick), trace);
	}

	if (bounces_count > 20)
		return detonate< false >();

	++bounces_count;
}

bool c_grenade_warning::nade_path_t::should_draw()
{
	if (path.size() <= 1u || interfaces::global_vars->cur_time >= nade_expire_time)
		return false;

	last_path_pos = {};

	vector2d out{};
	offscreen = !g_render->world_to_screen(path.back().first, out);

	if (!offscreen)
		last_path_pos = ImVec2(out.x, out.y);

	return true;
}

void c_grenade_warning::calc_nade_path(c_csplayer* entity)
{
	auto& predicted_nades = this->get_nade_list();

	static auto last_server_tick = interfaces::client_state->clock_drift_mgr.server_tick;
	if (last_server_tick != interfaces::client_state->clock_drift_mgr.server_tick)
	{
		predicted_nades.clear();

		last_server_tick = interfaces::client_state->clock_drift_mgr.server_tick;
	}

	c_client_class* client_class = entity->get_client_class();
	if (client_class == nullptr)
		return;

	const auto handle = entity->get_ref_handle();

	if (entity->explode_effect_tick_begin())
	{
		predicted_nades.erase(handle);

		return;
	}

	auto get_weapon_index = [&](int class_id)
	{
		switch (class_id)
		{
		case(int)CBaseCSGrenadeProjectile:
			return weapon_hegrenade;
			break;
		case(int)CDecoyProjectile:
			return weapon_decoy;
			break;
		case(int)CMolotovProjectile:
			return weapon_molotov;
			break;
		case(int)CSmokeGrenadeProjectile:
			return weapon_smokegrenade;
			break;
		default:
			return weapon_hegrenade;
			break;
		}
	};

	if (predicted_nades.find(handle) == predicted_nades.end())
	{
		int index = get_weapon_index(client_class->class_id);

		predicted_nades.emplace(std::piecewise_construct, std::forward_as_tuple(handle),
			std::forward_as_tuple(((c_basecombatweapon*)entity)->thrower(), index, entity->origin(), ((c_csplayer*)entity)->velocity(), entity->grenade_spawn_time(),
				math::time_to_ticks(((c_csplayer*)entity)->simulation_time() - entity->grenade_spawn_time())));

		predicted_nades.at(handle).preview_icon = (const char*)main_utils::get_projectile_icon(entity->get_model(), client_class->class_id);
		predicted_nades.at(handle).preview_name = main_utils::get_projectile_name(entity->get_model(), client_class->class_id);
	}

	if (predicted_nades.at(handle).should_draw())
		return;

	predicted_nades.erase(handle);
}

bool is_projectile(int class_id)
{
	switch (class_id)
	{
	case(int)CBaseCSGrenadeProjectile:
	case(int)CDecoyProjectile:
	case(int)CMolotovProjectile:
	case(int)CSmokeGrenadeProjectile:
		return true;
		break;
	}
	return false;
}

void c_grenade_warning::calc_local_nade_path()
{
	local_path = {};

	if (!g_cfg.visuals.grenade_predict)
		return;

	if (!g_ctx.is_alive || !g_ctx.weapon)
		return;

	if (!g_ctx.weapon->is_grenade())
		return;

	if (!g_ctx.weapon->pin_pulled() && g_ctx.weapon->throw_time() == 0.f)
		return;

	if (!g_ctx.weapon_info)
		return;

	local_path.nade_owner = g_ctx.local;
	local_path.nade_idx = g_ctx.weapon->item_definition_index();

	vector3d view_angles{};
	interfaces::engine->get_view_angles(view_angles);

	if (view_angles.x < -90.f)
	{
		view_angles.x += 360.f;
	}
	else if (view_angles.x > 90.f)
	{
		view_angles.x -= 360.f;
	}

	view_angles.x -= (90.f - std::fabsf(view_angles.x)) * 10.f / 90.f;

	vector3d direction{};
	math::angle_to_vectors(view_angles, direction);

	const auto throw_strength = std::clamp< float >(g_ctx.weapon->throw_strength(), 0.f, 1.f);
	const auto eye_pos = g_ctx.eye_position;
	const auto src = vector3d(eye_pos.x, eye_pos.y, eye_pos.z + (throw_strength * 12.f - 12.f));

	c_game_trace trace{};
	interfaces::engine_trace->trace_hull(src, src + direction * 22.f, { -2.f, -2.f, -2.f }, { 2.f, 2.f, 2.f }, mask_solid | contents_current_90, g_ctx.local, collision_group_none, &trace);

	vector3d velocity = g_ctx.local->abs_velocity();
	if (g_ctx.local->abs_velocity().length(true) < 10.f)
		velocity = {};

	local_path.predict_nade(
		trace.end - direction * 6.f, direction * (std::clamp< float >(g_ctx.weapon_info->throw_velocity * 0.9f, 15.f, 750.f) * (throw_strength * 0.7f + 0.3f)) + velocity * 1.25f, interfaces::global_vars->cur_time, 0);
}

void c_grenade_warning::draw_local_path()
{
	if (!g_cfg.visuals.grenade_predict)
		return;

	if (!local_path.should_draw())
		return;

	if (local_path.path.size() < 1)
		return;

	auto& first_pos = local_path.path.front();
	vector2d nade_start, nade_end;
	vector3d prev = first_pos.first;

	auto clr = g_cfg.visuals.predict_clr.base();

	g_render->enable_aa();
	for (const auto& it : local_path.path)
	{
		if (g_render->world_to_screen(prev, nade_start) && g_render->world_to_screen(it.first, nade_end))
		{
			g_render->line(nade_start.x, nade_start.y, nade_end.x, nade_end.y, clr, 1.5f);

			if (it.second)
			{
				g_render->filled_circle(nade_end.x, nade_end.y, 4.f, color(30, 30, 30, 200), 20);
				g_render->filled_circle(nade_end.x, nade_end.y, 3.f, color(255, 255, 255, 255), 20);
			}
		}

		prev = it.first;
	}
	g_render->disable_aa();
}

void c_grenade_warning::on_render_start(int stage)
{
	const std::unique_lock< std::mutex > lock(mutexes::warning);

	if (stage != frame_render_start)
		return;

	if (!g_ctx.in_game)
		return;

	this->calc_local_nade_path();
}

void c_grenade_warning::on_paint_traverse()
{
	const std::unique_lock< std::mutex > lock(mutexes::warning);

	if (!g_ctx.in_game)
	{
		if (!list.empty())
			list.clear();

		return;
	}

	if (!g_cfg.visuals.grenade_warning)
	{
		if (!list.empty())
			list.clear();

		return;
	}

	auto& weapon_array = g_listener_entity->get_entity(ent_weapon);
	if (weapon_array.empty())
	{
		if (!list.empty())
			list.clear();

		return;
	}

	list.clear();
	for (auto& weapon : weapon_array)
	{
		auto entity = weapon.entity;
		if (!entity)
			continue;

		auto class_id = entity->get_client_class()->class_id;
		if (!is_projectile(class_id))
			continue;

		this->calc_nade_path((c_csplayer*)entity);
	}
}

void c_grenade_warning::on_directx()
{
	const std::unique_lock< std::mutex > lock(mutexes::warning);

	if (!g_ctx.in_game)
	{
		if (!list.empty())
			list.clear();

		return;
	}

	this->draw_local_path();

	if (!g_cfg.visuals.grenade_warning)
	{
		if (!list.empty())
			list.clear();

		return;
	}

	g_render->enable_aa();

	auto& list = this->get_nade_list();
	if (list.empty())
	{
		g_render->disable_aa();
		return;
	}

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
			grenade_duration = std::clamp(std::abs(data.nade_expire_time - interfaces::global_vars->cur_time) / data.nade_detonate_time, 0.f, 1.f);

		g_menu->create_animation(mod, data.path.size() > 1 && (data.nade_detonate_time ? grenade_duration >= 0.01f : data.is_detonated), 0.3f, lerp_animation);
		g_menu->create_animation(oof_nade_alpha[l.first], l.second.offscreen && mod > 0.f, 0.3f, lerp_animation);

		if (mod <= 0.f)
			continue;

		if (l.second.offscreen && oof_nade_alpha[l.first] > 0.f)
		{
			render_weapon_offscreen_esp(&l.second, oof_nade_alpha[l.first], grenade_duration);
			continue;
		}

		auto& first_pos = data.path.front();

		auto base_clr = g_cfg.visuals.warning_clr.base();
		auto clr = base_clr.new_alpha(base_clr.a() * mod);

		// grenade path
		if (data.path.size() > 1 && g_cfg.visuals.grenade_warning_line)
		{
			vector2d nade_start, nade_end;
			vector3d prev = first_pos.first;

			for (const auto& it : data.path)
			{
				if (g_render->world_to_screen(prev, nade_start) && g_render->world_to_screen(it.first, nade_end))
					g_render->line(nade_start.x, nade_start.y, nade_end.x, nade_end.y, clr, 1.5f);

				prev = it.first;
			}
		}

		if (data.last_path_pos.x != 0.f && data.last_path_pos.y != 0.f)
		{
			auto rect_size = vector2d(50.f, 45.f);
			auto rect_start = vector2d(data.last_path_pos.x - (rect_size.x / 2.f), data.last_path_pos.y - (rect_size.y / 2.f));

			g_render->blur(rect_start.x, rect_start.y, rect_size.x, rect_size.y, color(200, 200, 200, 255 * mod), 4.f);
			g_render->rect(rect_start.x, rect_start.y, rect_size.x, rect_size.y, color(150, 150, 150, 12.75f * mod), 4.f, 2.f);

			ImGui::PushFont(g_fonts.weapon_icons);
			auto weapon_icon_size = ImGui::CalcTextSize(data.preview_icon.c_str());
			ImGui::PopFont();

			auto center_x = rect_start.x + (rect_size.x / 2.f);
			auto bot_y = (rect_start.y + rect_size.y) - 4.f;
			auto timer_width = (rect_size.x / 2.f) * (1.f - grenade_duration);

			auto weapon_icon_pos = ImVec2(rect_start.x + (rect_size.x / 2.f), rect_start.y + (rect_size.y / 2.f) - weapon_icon_size.y - 3.f);

			g_render->string(weapon_icon_pos.x, weapon_icon_pos.y, color(255, 255, 255, 255 * mod), centered_x | dropshadow_, g_fonts.weapon_icons_large, data.preview_icon.c_str());

			if (data.nade_detonate_time)
			{
				g_render->filled_rect(center_x, bot_y, timer_width, 4.f, clr, 4.f, ImDrawCornerFlags_BotRight);
				g_render->filled_rect(center_x - timer_width, bot_y, timer_width, 4.f, clr, 4.f, ImDrawCornerFlags_BotLeft);
			}
		}
	}

	g_render->disable_aa();
}