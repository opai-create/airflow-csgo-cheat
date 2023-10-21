#include "../features.h"
#include "../../includes.h"
#include "game_movement.h"

#undef min
#undef max

namespace game_movement
{
	inline void accelerate(c_usercmd& user_cmd, const vector3d& wishdir, const float wishspeed, vector3d& velocity, float acceleration)
	{
		const auto cur_speed = velocity.dot(wishdir);

		const auto add_speed = wishspeed - cur_speed;
		if (add_speed <= 0.f)
			return;

		const auto v57 = std::max<float>(cur_speed, 0.f);
		const auto ducking = user_cmd.buttons & in_duck || g_ctx.local->ducking() || (g_ctx.local->flags() & fl_ducking);

		auto v20 = true;
		if (ducking || !(user_cmd.buttons & in_speed))
			v20 = false;

		auto finalwishspeed = std::max<float>(wishspeed, 250.f);
		auto abs_finalwishspeed = finalwishspeed;

		auto max_speed = g_ctx.local->is_scoped() ? g_ctx.weapon_info->max_speed_alt : g_ctx.weapon_info->max_speed;

		bool slow_down{};
		if (g_ctx.weapon && cvars::sv_accelerate_use_weapon_speed->get_bool())
		{
			const auto item_index = static_cast<std::uint16_t>(g_ctx.weapon->item_definition_index());
			if (g_ctx.weapon->zoom_level() > 0 && (item_index == 11 || item_index == 38 || item_index == 9 || item_index == 8 || item_index == 39 || item_index == 40))
				slow_down = (max_speed * 0.52f) < 110.f;

			const auto modifier = std::min<float>(1.f, max_speed / 250.f);
			abs_finalwishspeed *= modifier;
			if ((!ducking && !v20) || slow_down)
				finalwishspeed *= modifier;
		}

		if (ducking)
		{
			if (!slow_down)
				finalwishspeed *= 0.34f;

			abs_finalwishspeed *= 0.34f;
		}

		if (v20)
		{
			if (!slow_down)
				finalwishspeed *= 0.52f;

			abs_finalwishspeed *= 0.52f;

			const auto abs_finalwishspeed_minus5 = abs_finalwishspeed - 5.f;
			if (v57 < abs_finalwishspeed_minus5)
			{
				const auto v30 =
					std::max(v57 - abs_finalwishspeed_minus5, 0.f)
					/ std::max(abs_finalwishspeed - abs_finalwishspeed_minus5, 0.f);

				const auto v27 = 1.f - v30;
				if (v27 >= 0.f)
					acceleration = std::min(v27, 1.f) * acceleration;
				else
					acceleration = 0.f;
			}
		}

		const auto v33 = std::min(
			add_speed,
			((interfaces::global_vars->interval_per_tick * acceleration) * finalwishspeed)
			* g_ctx.local->surface_friction()
		);

		velocity += wishdir * v33;

		const auto len = velocity.length(false);
		if (len && len > max_speed)
			velocity *= max_speed / len;
	}

	inline void walk_move(c_usercmd& user_cmd, vector3d& move, vector3d& fwd, vector3d& right, vector3d& velocity)
	{
		if (fwd.z != 0.f)
			fwd = fwd.normalized();

		if (right.z != 0.f)
			right = right.normalized();

		auto max_speed = g_ctx.local->is_scoped() ? g_ctx.weapon_info->max_speed_alt : g_ctx.weapon_info->max_speed;

		vector3d wishvel{ fwd.x * move.x + right.x * move.y,fwd.y * move.x + right.y * move.y, 0.f };

		auto wishdir = wishvel;
		auto wishspeed = wishdir.normalized_float();
		if (wishspeed && wishspeed > max_speed)
		{
			wishvel *= max_speed / wishspeed;
			wishspeed = max_speed;
		}

		velocity.z = 0.f;
		accelerate(user_cmd, wishdir, wishspeed, velocity, cvars::sv_accelerate->get_float());
		velocity.z = 0.f;

		const auto speed_sqr = velocity.length_sqr();
		if (speed_sqr > (max_speed * max_speed))
			velocity *= max_speed / std::sqrt(speed_sqr);

		if (velocity.length(false) < 1.f)
			velocity = {};
	}

	inline void friction(vector3d& velocity)
	{
		const auto speed = velocity.length(false);
		if (speed >= 0.1f)
		{
			const auto friction = cvars::sv_friction->get_float() * g_ctx.local->surface_friction();
			const auto stop_speed = cvars::sv_stopspeed->get_float();
			const auto control = speed < stop_speed ? stop_speed : speed;

			const auto new_speed = std::max(0.f, speed - ((control * friction) * interfaces::global_vars->interval_per_tick));
			if (speed != new_speed)
				velocity *= new_speed / speed;
		}
	}

	inline void full_walk_move(c_usercmd& user_cmd, vector3d& move, vector3d& fwd, vector3d& right, vector3d& velocity)
	{
		auto unpredicted_vars = g_engine_prediction->get_unpredicted_vars(g_ctx.cmd->command_number);
		if (!unpredicted_vars)
			return;

		if (unpredicted_vars->ground_entity != INT_MAX)
		{
			velocity.z = 0.f;

			friction(velocity);
			walk_move(user_cmd, move, fwd, right, velocity);

			velocity.z = 0.f;
		}

		auto max_velocity = cvars::sv_maxvelocity->get_float();
		for (int i = 0; i < 3; ++i)
		{
			auto& element = velocity[i];
			if (element > max_velocity)
				element = max_velocity;
			else if (element < -max_velocity)
				element = -max_velocity;
		}
	}

	inline void modify_move(c_usercmd& user_cmd, vector3d& velocity, float max_speed)
	{
		vector3d fwd{}, right{}, up{};
		math::angle_to_vectors(g_ctx.orig_angle, fwd, right, up);

		auto cmd_movement = vector3d{ user_cmd.forwardmove, user_cmd.sidemove, user_cmd.upmove };

		const auto speed_sqr = cmd_movement.length_sqr();
		if (speed_sqr > (max_speed * max_speed))
			cmd_movement *= max_speed / std::sqrt(speed_sqr);

		full_walk_move(user_cmd, cmd_movement, fwd, right, velocity);

		user_cmd.forwardmove = cmd_movement.x;
		user_cmd.sidemove = cmd_movement.y;
		user_cmd.upmove = cmd_movement.z;
	}

	inline void force_stop()
	{
		auto velocity = g_ctx.local->velocity();
		velocity.z = 0.f;

		friction(velocity);

		vector3d angle;
		math::vector_to_angles(velocity, angle);

		float stop_speed = velocity.length(false);

		angle.y = g_ctx.orig_angle.y - angle.y;

		vector3d direction;
		math::angle_to_vectors(angle, direction);

		vector3d stop = direction * -stop_speed;

		g_ctx.cmd->forwardmove = stop.x;
		g_ctx.cmd->sidemove = stop.y;
	}

	inline unsigned int get_jump_buttons()
	{
		unsigned int bits = g_ctx.cmd->buttons;

		static bool last_jump = false;
		static bool fake_jump = false;

		if (!last_jump && fake_jump)
		{
			fake_jump = false;
			bits |= in_jump;
		}
		else if ((bits & in_jump))
		{
			if (g_ctx.local->flags() & fl_onground)
				fake_jump = last_jump = true;
			else
			{
				bits &= ~in_jump;
				last_jump = false;
			}
		}
		else
			fake_jump = last_jump = false;

		return bits;
	}
}