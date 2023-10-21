#include "movement.h"

#include "../config_vars.h"
#include "../features.h"
#include "../anti hit/exploits.h"

#include "../ragebot/engine_prediction.h"
#include "../ragebot/ragebot.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/tools/math.h"

#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

#include "cmd_shift.h"

void c_movement::force_stop()
{
	vector3d angle;
	math::vector_to_angles(g_ctx.local->velocity(), angle);

	float speed = g_ctx.local->velocity().length(false);

	angle.y = g_ctx.orig_angle.y - angle.y;

	vector3d direction;
	math::angle_to_vectors(angle, direction);

	vector3d stop = direction * -speed;

	if (speed > 13.f)
	{
		g_ctx.cmd->forwardmove = stop.x;
		g_ctx.cmd->sidemove = stop.y;
	}
	else
	{
		g_ctx.cmd->forwardmove = 0.f;
		g_ctx.cmd->sidemove = 0.f;
	}
}

void c_movement::fast_stop()
{
	if (!g_cfg.misc.fast_stop && !g_cfg.binds[ap_b].toggled)
		return;

	if (peek_move)
		return;

	if (!g_utils->on_ground())
		return;

	vector3d velocity = g_ctx.local->velocity();
	float speed = velocity.length(true);

	if (speed < 5.f)
		return;

	bool pressing_move_keys = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_back || g_ctx.cmd->buttons & in_forward;

	if (pressing_move_keys)
		return;

	this->force_stop();
}

void c_movement::rotate(c_usercmd& cmd, const vector3d& wish_angles, const int& flags, const int& move_type)
{
	if (cmd.viewangles.z != 0.f && !(flags & fl_onground))
		cmd.sidemove = 0.f;

	auto move_2d = vector2d(cmd.forwardmove, cmd.sidemove);

	if (const auto speed_2d = move_2d.length()) 
	{
		const auto delta = cmd.viewangles.y - wish_angles.y;

		vector2d v1{};

		auto angle =  math::deg_to_rad(std::remainder(math::rad_to_deg(std::atan2(move_2d.y / speed_2d, move_2d.x / speed_2d)) + delta, 360.f));

		v1.x = std::sinf(angle);
		v1.y = std::cosf(angle);

		const auto cos_x = std::cos(math::deg_to_rad(std::remainder(math::rad_to_deg(std::atan2(0.f, speed_2d)), 360.f)));

		move_2d.x = cos_x * v1.y * speed_2d;
		move_2d.y = cos_x * v1.x * speed_2d;

		if (move_type == movetype_ladder) {
			if (wish_angles.x < 45.f
				&& std::abs(delta) <= 65.f
				&& cmd.viewangles.x >= 45.f) {
				move_2d.x *= -1.f;
			}
		}
		else if (std::abs(cmd.viewangles.x) > 90.f)
			move_2d.x *= -1.f;
	}

	cmd.forwardmove = move_2d.x;
	cmd.sidemove = move_2d.y;
}

void c_movement::auto_peek()
{
	static bool old_move = false;

	bool moving = g_ctx.cmd->buttons & in_moveleft || g_ctx.cmd->buttons & in_moveright || g_ctx.cmd->buttons & in_forward || g_ctx.cmd->buttons & in_back;

	vector3d origin = g_ctx.local->origin();

	auto should_peek = [&]()
	{
		if (!g_cfg.binds[ap_b].toggled)
			return false;

		if (!peek_pos.valid())
		{
			if (!(g_ctx.local->flags() & fl_onground))
			{
				c_game_trace trace{};
				c_trace_filter_world_and_props_only filter{};
				interfaces::engine_trace->trace_ray(ray_t{ origin, vector3d{origin.x, origin.y, origin.z * g_ctx.weapon_info->range } }, mask_solid, &filter, &trace);

				peek_pos = trace.end;
			}
			else
				peek_pos = origin;
		}

		peek_start = true;

		if (!g_ctx.weapon->is_misc_weapon() && g_utils->on_ground())
		{
			bool cmd_attack = g_ctx.cmd->buttons & in_attack;
			bool is_firing = g_ctx.weapon->item_definition_index() == weapon_revolver ? cmd_attack && g_utils->revolver_fire : cmd_attack;

			if (is_firing || g_rage_bot->firing || (g_cfg.misc.retrack_peek && !moving))
				peek_move = true;

			if (g_cfg.misc.retrack_peek && moving && !old_move)
				peek_move = false;
		}

		vector3d origin_delta = peek_pos - origin;
		float distance = origin_delta.length(true);

		if (peek_move)
		{
			auto return_position = math::angle_from_vectors(origin, peek_pos);

			if (distance > 10.f)
			{
				g_ctx.base_angle.y = math::normalize(return_position.y);

				g_ctx.cmd->forwardmove = cvars::cl_forwardspeed->get_float();
				g_ctx.cmd->sidemove = 0.f;
			}
			else
			{
				bool shot_finish = false;
				if (g_ctx.weapon->item_definition_index() == weapon_ssg08 || g_ctx.weapon->item_definition_index() == weapon_awp)
				{
					float shot_diff = std::abs(g_ctx.weapon->last_shot_time() - g_ctx.predicted_curtime);

					shot_finish = shot_diff >= 0.5f;
				}
				else
				{
					float old_time = g_ctx.predicted_curtime;

					g_ctx.predicted_curtime = math::ticks_to_time(g_ctx.local->tickbase() - ((g_exploits->limits.double_tap / 2) + 1));
					shot_finish = g_utils->is_able_to_shoot(true);
					g_ctx.predicted_curtime = old_time;
				}

				this->force_stop();

				if (shot_finish)
					peek_move = false;

				auto shifting = cmd_shift::shifting || g_exploits->cl_move.trigger && g_exploits->cl_move.shifting;
				if (shifting)
					g_engine_prediction->repredict(g_ctx.local, g_ctx.cmd);
			}
		}

		old_move = moving;
		return true;
	};

	if (!should_peek())
	{
		peek_pos.reset();
		peek_start = false;
		peek_move = false;
	}
}

void test()
{
	if (g_cfg.binds[ap_b].toggled || g_cfg.binds[sw_b].toggled || g_rage_bot->stopping || !g_utils->on_ground())
		return;

	static int old_buttons = 0;

	if (!(g_ctx.cmd->buttons & in_jump))
	{
		int unk = g_ctx.cmd->buttons & (in_moveright | in_moveleft | in_back | in_forward) & (g_ctx.cmd->buttons & (in_moveright | in_moveleft | in_back | in_forward) ^ (int)g_cfg.rage.enable);

		int cur_buttons = old_buttons;

		if ((unk & in_moveleft) != 0)
		{
			cur_buttons = cur_buttons & -(in_moveright | in_moveleft | in_attack) | in_moveright;
		}
		else if ((g_ctx.cmd->buttons & in_moveleft) == 0)
		{
			cur_buttons &= ~in_moveright;
		}
		if ((unk & in_moveright) != 0)
		{
			cur_buttons = cur_buttons & ~(in_moveright | in_moveleft) | in_moveleft;
		}
		else if ((g_ctx.cmd->buttons & in_moveright) == 0)
		{
			cur_buttons &= ~in_moveleft;
		}
		if ((unk & in_forward) != 0)
		{
			cur_buttons = cur_buttons & -(in_back | in_forward | in_attack) | in_back;
		}
		else if ((g_ctx.cmd->buttons & in_forward) == 0)
		{
			cur_buttons &= -(in_back | in_attack);
		}
		if ((unk & in_back) != 0)
		{
			cur_buttons = cur_buttons & -(in_back | in_forward | in_attack) | in_forward;
		}
		else if ((g_ctx.cmd->buttons & in_back) == 0)
		{
			cur_buttons &= ~in_forward;
		}

		old_buttons = cur_buttons;
		g_ctx.cmd->buttons &= ~old_buttons;

		auto new_buttons = g_ctx.cmd->buttons;

		if ((new_buttons & (in_moveright | in_moveleft)) != (in_moveright | in_moveleft))
		{
			if ((new_buttons & 0x200) != 0)
			{
				g_ctx.cmd->sidemove = -450.f;
			}
			else if ((new_buttons & 0x400) != 0)
			{
				g_ctx.cmd->sidemove = 450.f;
			}
		}
		if ((new_buttons & 0x18) != 24)
		{
			if ((new_buttons & 8) != 0)
			{
				g_ctx.cmd->forwardmove = 450.f;
			}
			else if ((new_buttons & 0x10) != 0)
			{
				g_ctx.cmd->forwardmove = -450.f;
			}
		}
	}
	else
		old_buttons = 0;
}

void c_movement::jitter_move()
{
	if (!g_cfg.antihit.enable || !g_cfg.antihit.jitter_move)
		return;

	if (g_cfg.binds[ap_b].toggled || g_cfg.binds[sw_b].toggled || g_rage_bot->stopping)
		return;

	if (!g_utils->on_ground())
		return;

	test();

	float tickrate_rate = 5.f / g_ctx.tick_rate;

	float rate = ((g_ctx.cmd->command_number % g_ctx.tick_rate) * tickrate_rate) + 95.f;
	float strength = std::clamp(rate, 95.f, 100.f);

	float max_speed = (strength / 100.0f) * this->max_speed;
	this->force_speed(max_speed);
}

void c_movement::auto_strafe()
{
	if (!g_cfg.misc.auto_strafe)
		return;

	if (g_ctx.local->move_type() == movetype_ladder || g_ctx.local->move_type() == movetype_noclip)
		return;

	if (g_utils->on_ground())
		return;

	if (g_ctx.cmd->buttons & in_speed)
		return;

	if (g_rage_bot->stopping && (g_rage_bot->weapon_config.quick_stop_options & in_air))
		return;

	bool holding_w = g_ctx.cmd->buttons & in_forward;
	bool holding_a = g_ctx.cmd->buttons & in_moveleft;
	bool holding_s = g_ctx.cmd->buttons & in_back;
	bool holding_d = g_ctx.cmd->buttons & in_moveright;

	bool m_pressing_move = holding_w || holding_a || holding_s || holding_d;

	static auto switch_key = 1.f;
	static auto circle_yaw = 0.f;
	static auto old_yaw = 0.f;

	auto velocity = g_ctx.local->velocity();
	velocity.z = 0.f;

	auto speed = velocity.length(true);

	auto ideal_strafe = (speed > 5.f) ? math::rad_to_deg(std::asin(15.f / speed)) : 90.f;
	ideal_strafe *= 1.f - (g_cfg.misc.strafe_smooth * 0.01f);

	ideal_strafe = min(90.f, ideal_strafe);

	switch_key *= -1.f;

	if (m_pressing_move)
	{
		float wish_dir{};

		if (holding_w)
		{
			if (holding_a)
				wish_dir += (strafe_left / 2);
			else if (holding_d)
				wish_dir += (strafe_right / 2);
			else
				wish_dir += strafe_forwards;
		}
		else if (holding_s)
		{
			if (holding_a)
				wish_dir += strafe_back_left;
			else if (holding_d)
				wish_dir += strafe_back_right;
			else
				wish_dir += strafe_backwards;

			g_ctx.cmd->forwardmove = 0.f;
		}
		else if (holding_a)
			wish_dir += strafe_left;
		else if (holding_d)
			wish_dir += strafe_right;

		g_ctx.base_angle.y += math::normalize(wish_dir);
	}

	float smooth = (1.f - (0.15f * (1.f - g_cfg.misc.strafe_smooth * 0.01f)));

	if (speed <= 0.5f)
	{
		g_ctx.cmd->forwardmove = 450.f;
		return;
	}

	/*if (g_ctx.cmd->forwardmove > 0.f)
		g_ctx.cmd->forwardmove = 0.f;

	const auto yaw_delta = math::normalize(g_ctx.base_angle.y - old_yaw);
	const auto absolute_yaw_delta = std::abs(yaw_delta);

	circle_yaw = old_yaw = g_ctx.base_angle.y;

	if (yaw_delta > 0.f)
		g_ctx.cmd->sidemove = -cvars::cl_sidespeed->get_float();
	else if (yaw_delta < 0.f)
		g_ctx.cmd->sidemove = cvars::cl_sidespeed->get_float();

	if (absolute_yaw_delta <= ideal_strafe || absolute_yaw_delta >= 30.f) {
		vector3d velocity_angles;
		math::vector_to_angles(velocity, velocity_angles);

		const auto velocity_delta = math::normalize(g_ctx.base_angle.y - velocity_angles.y);
		const auto retrack = ideal_strafe * 2.f;

		if (velocity_delta <= retrack || speed <= 15.f) {
			if (-retrack <= velocity_delta || speed <= 15.f) {
				g_ctx.base_angle.y += ideal_strafe * switch_key;
				g_ctx.cmd->sidemove = switch_key * cvars::cl_sidespeed->get_float();
			}
			else {
				g_ctx.base_angle.y = velocity_angles.y - retrack;
				g_ctx.cmd->sidemove = cvars::cl_sidespeed->get_float();
			}
		}
		else {
			g_ctx.base_angle.y = velocity_angles.y + retrack;
			g_ctx.cmd->sidemove = -cvars::cl_sidespeed->get_float();
		}
	}*/

	const auto diff = math::normalize(g_ctx.base_angle.y - math::rad_to_deg(std::atan2f(velocity.y, velocity.x)));

	g_ctx.cmd->forwardmove = std::clamp((5850.f / speed), -450.f, 450.f);
	g_ctx.cmd->sidemove = (diff > 0.f) ? -450.f : 450.f;

	g_ctx.base_angle.y = math::normalize(g_ctx.base_angle.y - diff * smooth);
}

void c_movement::auto_jump()
{
	if (!g_cfg.misc.auto_jump)
		return;

	if (g_ctx.jump_buttons & in_jump)
		g_ctx.cmd->buttons |= in_jump;
	else
		g_ctx.cmd->buttons &= ~in_jump;
}

void c_movement::force_speed(float max_speed)
{
	// as did in game movement of CSGO:
	// reference: CCSGameMovement::CheckParameters

	if (g_ctx.local->move_type() == movetype_noclip || g_ctx.local->move_type() == movetype_isometric || g_ctx.local->move_type() == movetype_observer)
		return;

	auto velocity = g_ctx.local->velocity();
	game_movement::modify_move(*g_ctx.cmd, velocity, max_speed);
}

float c_movement::get_max_speed()
{
	return max_speed;
}

void c_movement::fix_movement(c_usercmd* cmd, vector3d& ang)
{
	if (g_ctx.local->move_type() == movetype_ladder || g_ctx.local->move_type() == movetype_noclip)
		return;

	vector3d direction;
	vector3d move_angle;

	auto move = vector3d{ cmd->forwardmove, cmd->sidemove, 0 };
	auto length = move.normalized_float();
	if (length == 0.f)
		return;

	math::vector_to_angles(move, move_angle);

	auto delta = (cmd->viewangles.y - ang.y);

	move_angle.y += delta;

	math::angle_to_vectors(move_angle, direction);

	direction *= length;

	if (cmd->viewangles.x < -90 || cmd->viewangles.x > 90)
		direction.x = -direction.x;

	cmd->forwardmove = direction.x;
	cmd->sidemove = direction.y;

	auto calc_move_angle = [&](float base_angle) 
	{
		float angle = std::cos(math::deg_to_rad(base_angle)) * -cmd->forwardmove
			+ (std::sin(math::deg_to_rad(base_angle))) * cmd->sidemove;
		return angle;
	};

	float negative = calc_move_angle(180.f - cmd->viewangles.z);
	float positive = calc_move_angle(cmd->viewangles.z + 180.f);

	float step = cmd->viewangles.x / 89.f;

	if (std::fabsf(cmd->viewangles.z) > 0.f)
		cmd->forwardmove = std::lerp(cmd->forwardmove, step <= 0.f ? positive : negative, step);

	if (!g_cfg.misc.slide_walk)
		cmd->buttons &= ~(in_forward | in_back | in_moveright | in_moveleft);
	else
	{
		cmd->buttons &= ~(in_forward | in_back | in_moveright | in_moveleft);

		if (!*g_ctx.send_packet)
		{
			if (cmd->sidemove < -5.f)
				cmd->buttons |= in_moveright;
			else if (cmd->sidemove > 5.f)
				cmd->buttons |= in_moveleft;

			if (cmd->forwardmove < -5.f)
				cmd->buttons |= in_forward;
			else if (cmd->forwardmove > 5.f)
				cmd->buttons |= in_back;
		}
		else
		{
			if (cmd->sidemove > 5.f)
				cmd->buttons |= in_moveright;
			else if (cmd->sidemove < -5.f)
				cmd->buttons |= in_moveleft;

			if (cmd->forwardmove > 5.f)
				cmd->buttons |= in_forward;
			else if (cmd->forwardmove < -5.f)
				cmd->buttons |= in_back;
		}
	}
}

void c_movement::on_pre_predict()
{
	if (!g_ctx.in_game)
		return;

	if (!g_ctx.local->is_alive() || !g_ctx.weapon_info)
		return;

	if (g_ctx.local->move_type() == movetype_ladder || g_ctx.local->move_type() == movetype_noclip)
		return;

	if (interfaces::game_rules->is_freeze_time() || g_ctx.local->flags() & fl_frozen)
		return;

	max_speed = g_ctx.local->is_scoped() ? g_ctx.weapon_info->max_speed_alt : g_ctx.weapon_info->max_speed;

	this->auto_jump();
}

void c_movement::on_predict_start()
{
	if (!g_ctx.weapon)
		return;

	if (g_ctx.local->move_type() == movetype_ladder || g_ctx.local->move_type() == movetype_noclip)
		return;

	if (interfaces::game_rules->is_freeze_time() || g_ctx.local->flags() & fl_frozen)
		return;

	this->auto_peek();
	this->fast_stop();
}