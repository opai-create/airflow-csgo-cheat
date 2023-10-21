#include "rage_tools.h"
#include "autowall.h"
#include "engine_prediction.h"

#include "../features.h"
#include "resolver.h"

namespace resolver
{
	inline void prepare_jitter(c_csplayer* player, resolver_info_t& resolver_info)
	{
		auto& jitter = resolver_info.jitter;
		jitter.yaw_cache[jitter.yaw_cache_offset % yaw_cache_size] = player->eye_angles().y;

		if (jitter.yaw_cache_offset >= yaw_cache_size + 1)
			jitter.yaw_cache_offset = 0;
		else
			jitter.yaw_cache_offset++;

		for (int i = 0; i < yaw_cache_size - 1; ++i)
		{
			float diff = std::fabsf(jitter.yaw_cache[i] - jitter.yaw_cache[i + 1]);
			if (diff <= 0.f)
			{
				if (jitter.static_ticks < 3)
					jitter.static_ticks++;
				else
					jitter.jitter_ticks = 0;
			}
			else if (diff >= 10.f)
			{
				if (jitter.jitter_ticks < 3)
					jitter.jitter_ticks++;
				else
					jitter.static_ticks = 0;
			}
		}

		jitter.is_jitter = jitter.jitter_ticks > jitter.static_ticks;
	}

	inline vector3d get_point_direction(c_csplayer* player)
	{
		vector3d fw{};

		float at_target_yaw = math::angle_from_vectors(g_ctx.local->origin(), player->origin()).y;
		math::angle_to_vectors(vector3d(0, at_target_yaw - 90.f, 0), fw);

		return fw;
	}

	inline void prepare_freestanding(c_csplayer* player)
	{
		auto& info = resolver_info[player->index()];
		auto& jitter = info.jitter;
		auto& freestanding = info.freestanding;

		auto layers = player->anim_overlay();

		if (!layers || !g_ctx.weapon_info || !g_ctx.local || !g_ctx.local->is_alive() || player->is_bot() || !g_cfg.rage.resolver)
		{
			if (freestanding.updated)
				freestanding.reset();

			return;
		}

		auto weight = layers[6].weight;
		if (jitter.is_jitter || weight > 0.75f)
		{
			if (freestanding.updated)
				freestanding.reset();

			return;
		}

		auto& cache = player->bone_cache();
		if (!cache.count() || !cache.base())
			return;

		freestanding.updated = true;

		bool inverse_side{};
		{
			float at_target = math::normalize(math::angle_from_vectors(g_ctx.local->origin(), player->get_abs_origin()).y);
			float angle = math::normalize(player->eye_angles().y);

			const bool sideways_left = std::abs(math::normalize(angle - math::normalize(at_target - 90.f))) < 45.f;
			const bool sideways_right = std::abs(math::normalize(angle - math::normalize(at_target + 90.f))) < 45.f;

			bool forward = std::abs(math::normalize(angle - math::normalize(at_target + 180.f))) < 45.f;
			inverse_side = forward && !(sideways_left || sideways_right);
		}

		auto direction = get_point_direction(player);

		static matrix3x4_t predicted_matrix[128]{};
		std::memcpy(predicted_matrix, cache.base(), sizeof(predicted_matrix));

		auto store_changed_matrix_data = [&](const vector3d& new_position, pen_data_t& out)
		{
			auto old_abs_origin = player->get_abs_origin();

			math::change_matrix_position(predicted_matrix, 128, player->origin(), new_position);
			{
				static matrix3x4_t old_cache[128]{};
				player->store_bone_cache(old_cache);
				{
					player->set_abs_origin(new_position);
					player->set_bone_cache(predicted_matrix);

					auto head_pos = cache.base()[8].get_origin();
					out = g_auto_wall->fire_bullet(g_ctx.local, player, g_ctx.weapon_info, false, g_ctx.eye_position, head_pos);

				//	interfaces::debug_overlay->add_text_overlay(head_pos, 0.1f, "%d", out.dmg);
				}
				player->set_bone_cache(old_cache);
			}
			math::change_matrix_position(predicted_matrix, 128, new_position, player->origin());
		};

		{
		
			pen_data_t left{}, right{};

			auto left_dir = inverse_side ? (player->origin() + direction * 40.f) : (player->origin() - direction * 40.f);
			store_changed_matrix_data(left_dir, left);

			auto right_dir = inverse_side ? (player->origin() - direction * 40.f) : (player->origin() + direction * 40.f);
			store_changed_matrix_data(right_dir, right);

			if (left.dmg > right.dmg)
				freestanding.side = side_right;
			else if (left.dmg < right.dmg)
				freestanding.side = side_left;
			else
				freestanding.side = side_zero;
		}
	}

	inline void prepare_side(c_csplayer* player, records_t* current)
	{
		auto& info = resolver_info[player->index()];
		if (!g_ctx.weapon_info || !g_ctx.local || !g_ctx.local->is_alive() || current->choke < 2 || player->is_bot() || !g_cfg.rage.resolver)
		{
			if (info.resolved)
				info.reset();

			return;
		}

		auto state = player->animstate();
		if (!state)
		{
			if (info.resolved)
				info.reset();

			return;
		}

		prepare_jitter(player, info);
		prepare_freestanding(player);

		auto& freestanding = info.freestanding;
		auto& jitter = info.jitter;
		if (jitter.is_jitter)
		{
			float first_angle = math::normalize(jitter.yaw_cache[yaw_cache_size - 1]);
			float second_angle = math::normalize(jitter.yaw_cache[yaw_cache_size - 2]);

			float _first_angle = std::sin(math::deg_to_rad(first_angle));
			float _second_angle = std::sin(math::deg_to_rad(second_angle));

			float __first_angle = std::cos(math::deg_to_rad(first_angle));
			float __second_angle = std::cos(math::deg_to_rad(second_angle));

			float avg_yaw = math::normalize(math::rad_to_deg(std::atan2f((_first_angle + _second_angle) / 2.f, (__first_angle + __second_angle) / 2.f)));
			float diff = math::normalize(current->eye_angles.y - avg_yaw);
			
			info.side = diff > 0.f ? side_right : side_left;
			info.resolved = true;
			info.mode = xor_c("jitter");
		}
		else
		{
			if (freestanding.updated)
			{
				info.side = freestanding.side;
				info.mode = xor_c("freestanding");
			}
			else
			{
				info.side = side_zero;
				info.mode = xor_c("static");
			}

			info.resolved = true;
		}
	}

	inline void apply_side(c_csplayer* player, const int& choke)
	{
		auto& info = resolver_info[player->index()];
		if (!g_ctx.weapon_info || !g_ctx.local || !g_ctx.local->is_alive() || !info.resolved || info.side == side_original)
			return;

		auto state = player->animstate();
		if (!state)
			return;

		float desync_angle = choke > 3 ? 120.f : state->get_max_rotation();
		state->abs_yaw = math::normalize(player->eye_angles().y + desync_angle * info.side);
	}
}