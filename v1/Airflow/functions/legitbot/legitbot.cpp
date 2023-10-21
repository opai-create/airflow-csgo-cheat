#include "legitbot.h"

#include "../ragebot/rage_tools.h"
#include "../features.h"

constexpr auto fix_min_fov_head = 50;
constexpr auto fov_base_distance = 200;

std::vector< int > c_legit_bot::get_hitboxes()
{
	std::vector< int > hitboxes{};
	auto weapon_config = rage_tools::get_legit_weapon_config();

	if (weapon_config.hitboxes & head)
		hitboxes.emplace_back((int)hitbox_head);

	if (weapon_config.hitboxes & chest)
	{
		hitboxes.emplace_back((int)hitbox_chest);
		hitboxes.emplace_back((int)hitbox_lower_chest);
	}

	if (weapon_config.hitboxes & stomach)
		hitboxes.emplace_back((int)hitbox_stomach);

	if (weapon_config.hitboxes & pelvis)
		hitboxes.emplace_back((int)hitbox_pelvis);

	return hitboxes;
}

int c_legit_bot::get_fov()
{
	auto weapon_config = rage_tools::get_legit_weapon_config();

	int fov_valie = weapon_config.fov;
	int base_fov = pow(fov_valie + fix_min_fov_head, 2) * 90;

	return (int)(base_fov / (fov_base_distance * g_ctx.current_fov));
}

bool c_legit_bot::is_target_in_fov(const vector2d& center, const vector2d& screen)
{
	int fov_x = (int)center.x - (int)screen.x;
	int fov_y = (int)center.y - (int)screen.y;

	int fov_amount = this->get_fov();
	if (fov_x < fov_amount && fov_x > -fov_amount && fov_y < fov_amount && fov_y - fov_amount)
		return true;

	return false;
}

__forceinline void smooth_angles(vector3d MyViewAngles, vector3d AimAngles, vector3d& OutAngles, float Smoothing)
{
	OutAngles = AimAngles - MyViewAngles;

	OutAngles = math::normalize(OutAngles, true);

	OutAngles.x = OutAngles.x / Smoothing + MyViewAngles.x;
	OutAngles.y = OutAngles.y / Smoothing + MyViewAngles.y;

	OutAngles = math::normalize(OutAngles, true);
}

int c_legit_bot::get_min_damage(c_csplayer* player)
{
	int health = player->health();

	int menu_damage = g_cfg.legit.min_damage;
	int maximum_damage = 0;

	// hp + 1 slider
	if (menu_damage >= 100)
		maximum_damage = health + (menu_damage - 100);
	else
		maximum_damage = menu_damage;

	return maximum_damage;
}

bool c_legit_bot::is_visible(c_csplayer* player, const vector3d& pos)
{
	auto eye_pos = g_ctx.eye_position;

	c_trace_filter filter{};
	filter.skip = g_ctx.local;

	c_game_trace trace{};
	interfaces::engine_trace->trace_ray(ray_t(eye_pos, pos), mask_shot_hull | contents_hitbox, &filter, &trace);

	// interfaces::debug_overlay->add_line_overlay(eye_pos, pos, 255, 255, 255, true, interfaces::global_vars->interval_per_tick * 2.f);
	return trace.entity == player || trace.fraction >= 0.97f;
}

bool line_goes_through_smoke(const vector3d& start, const vector3d& end)
{
	return func_ptrs::line_goes_through_smoke(start, end);
}

float calc_fov_value(const vector3d& view_angles, const vector3d& start, const vector3d& end)
{
	vector3d dir, fw;

	dir = (end - start).normalized();

	math::angle_to_vectors(view_angles, fw);

	return std::max< float >(math::rad_to_deg(std::acos(fw.dot(dir))), 0.f);
}

void c_legit_bot::do_aimbot()
{
	static float last_time = 0.f;

	auto weapon_config = rage_tools::get_legit_weapon_config();
	if (!weapon_config.enable)
	{
		last_time = 0.f;
		return;
	}

	auto nearest_target = g_anti_aim->get_closest_player();
	if (!nearest_target)
	{
		last_time = 0.f;
		return;
	}

	float best_dist = FLT_MAX;
	vector3d nearest_hitbox{};
	vector2d center = vector2d(g_render->screen_size.w * 0.5f, g_render->screen_size.h * 0.5f);
	records_t* best_record{};
	int best_hitbox{};

	vector3d view_angles{};
	interfaces::engine->get_view_angles(view_angles);

	if (weapon_config.backtrack)
	{
		auto records = g_animation_fix->get_all_records(nearest_target);
		if (records.empty())
		{
			last_time = 0.f;
			return;
		}

		for (auto& record : records)
		{
			if (!record)
				continue;

			for (auto& hitbox : this->get_hitboxes())
			{
				for (auto& points : rage_tools::get_multipoints(nearest_target, hitbox, record->sim_orig.bone))
				{
					if (!points.second)
						continue;

					vector2d w2s{};
					if (!g_render->world_to_screen(points.first, w2s))
						continue;

					if (!this->is_target_in_fov(center, w2s))
						continue;

					float fov = calc_fov_value(view_angles, g_ctx.eye_position, points.first);
					if (fov < best_dist)
					{
						nearest_hitbox = points.first;
						best_dist = fov;
						best_record = record;
						best_hitbox = hitbox;
					}
				}
			}
		}
	}
	else
	{
		auto record = g_animation_fix->get_latest_record(nearest_target);
		if (!record)
			return;

		for (auto& hitbox : this->get_hitboxes())
		{
			for (auto& points : rage_tools::get_multipoints(nearest_target, hitbox, record->sim_orig.bone))
			{
				if (!points.second)
					continue;

				vector2d w2s{};
				if (!g_render->world_to_screen(points.first, w2s))
					continue;

				if (!this->is_target_in_fov(center, w2s))
					continue;

				float fov = calc_fov_value(view_angles, g_ctx.eye_position, points.first);
				if (fov < best_dist)
				{
					nearest_hitbox = points.first;
					best_dist = fov;
					best_record = record;
					best_hitbox = hitbox;
				}
			}
		}
	}

	if (!best_record || !nearest_hitbox.valid() || !g_cfg.binds[toggle_legit_b].toggled)
	{
		last_time = 0.f;
		return;
	}

	bool shoot = false;

	g_rage_bot->store(nearest_target);
	g_rage_bot->set_record(nearest_target, best_record);

	auto awall = g_auto_wall->fire_bullet(g_ctx.local, nearest_target, g_ctx.weapon_info, g_ctx.weapon->is_taser(), g_ctx.eye_position, nearest_hitbox);
	shoot = this->is_visible(nearest_target, nearest_hitbox) || g_cfg.legit.autowall && awall.dmg >= this->get_min_damage(nearest_target);

	if (g_cfg.legit.flash_check && g_ctx.local->is_flashed())
		shoot = false;

	bool smoke_check = line_goes_through_smoke(g_ctx.eye_position, nearest_hitbox);
	if (g_cfg.legit.smoke_check && smoke_check)
		shoot = false;

	g_rage_bot->restore(nearest_target);

	if (!shoot)
		return;

	if (weapon_config.quick_stop && g_ctx.local->velocity().length(true) >= (g_movement->get_max_speed() * 0.33f))
		g_movement->force_stop();

	vector3d smooth_angle{};
	auto angle = math::normalize(math::angle_from_vectors(g_ctx.eye_position, nearest_hitbox), true);

	if (g_cfg.legit.rcs.enable)
	{
		auto punch_angle = g_ctx.local->aim_punch_angle() * cvars::weapon_recoil_scale->get_float() * (g_cfg.legit.rcs.amount / 100.f);
		angle -= punch_angle;
	}

	float smooth = weapon_config.smooth;
	if (smooth < 10.f)
		smooth = 10.f;

	smooth = smooth / 10.f;

	if (weapon_config.aim_delay > 0)
	{
		if (!last_time)
		{
			last_time = interfaces::global_vars->cur_time;
			g_ctx.cmd->buttons &= ~(in_attack);
		}
		else
		{
			float delay_amt = (float)weapon_config.aim_delay / 1000.f;
			float diff = std::clamp((interfaces::global_vars->cur_time - last_time) / delay_amt, 0.f, 1.f);

			if (diff < 1.f)
				g_ctx.cmd->buttons &= ~(in_attack);
		}
	}

	smooth_angles(g_ctx.cmd->viewangles, angle, smooth_angle, smooth);

	g_ctx.cmd->viewangles = smooth > 0.f ? smooth_angle : angle;
	interfaces::engine->set_view_angles(smooth_angle);

	if (g_ctx.lagcomp && g_utils->is_firing())
		g_ctx.cmd->tickcount = math::time_to_ticks(best_record->sim_time + g_ctx.lerp_time);
}

void c_legit_bot::on_predict_start()
{
	if (g_cfg.rage.enable || !g_cfg.legit.enable || g_ctx.weapon->is_misc_weapon())
		return;

	this->do_aimbot();
}

void c_legit_bot::on_directx()
{
	if (!g_ctx.is_alive)
		return;

	if (!g_cfg.legit.enable)
		return;

	auto weapon_config = rage_tools::get_legit_weapon_config();
	if (!weapon_config.enable)
		return;

	if (!weapon_config.fov_cicle)
		return;

	auto clr = weapon_config.circle_color.base();
	g_render->circle(g_render->screen_size.w / 2, g_render->screen_size.h / 2, this->get_fov(), clr, 100);
}
