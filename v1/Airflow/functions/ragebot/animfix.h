#pragma once
#include <memory>
#include <deque>
#include <mutex>
#include <unordered_map>

#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

#include "../../base/tools/math.h"
#include "../../base/tools/memory/displacement.h"

#include "../../base/other/game_functions.h"

#include "setup_bones_manager.h"

enum simulate_side_t
{
	side_right = -1,
	side_zero,
	side_left,
	side_original,
};

struct simulated_data_t
{
	c_bone_builder builder{};
	matrix3x4_t bone[128]{};
	matrix3x4_t roll_bone[128]{};
	c_animation_layers layers[13]{};

	__forceinline void reset()
	{
		std::memset(layers, 0, sizeof(layers));
		std::memset(bone, 0, sizeof(bone));
		std::memset(roll_bone, 0, sizeof(roll_bone));

		builder = {};
	}
};

struct records_t
{
	bool valid{};
	bool valid_tick{};
	bool breaking_lc{};
	bool shifting{};
	bool dormant = true;
	bool walking{};
	bool strafing{};
	c_csplayer* ptr{};
	int flags{};
	int eflags{};
	int choke{};
	int resolver_side{};
	float duck_amount{};
	float lby{};
	float thirdperson_recoil{};
	float sim_time{};
	float update_time{};
	float interp_time{};
	float old_sim_time{};
	float land_time{};
	float anim_time{};
	float anim_speed{};
	float last_shot_time{};
	float last_eyeang_diff{};
	float last_eyeang_diff_time{};
	float last_eyeang{};
	float old_diff{};
	float collision_change_time{};
	float fixed_diff{};
	bool landing{};
	bool landed{};
	bool on_ground{};
	bool shooting{};
	bool crouch_jumping{};
	bool fix_jitter_angle{};
	vector3d abs_origin{};
	vector3d origin{};
	vector3d velocity{};
	vector3d velocity_for_animfix{};
	vector3d old_velocity{};
	vector3d mins{};
	vector3d maxs{};
	vector3d eye_angles{};
	vector3d abs_angles{};
	vector3d collision_change_origin{};
	simulated_data_t sim_orig{};
	simulated_data_t sim_left{};
	simulated_data_t sim_right{};
	simulated_data_t sim_zero{};
	matrix3x4_t render_bones[128]{};
	matrix3x4_t* bones_to_aim{};
	std::array< float, 24 > poses{};

	uint32_t ground_entity{};

	__forceinline void update_record(c_csplayer* player)
	{
		valid = true;
		ptr = player;
		flags = player->flags();
		eflags = player->e_flags();
		duck_amount = player->duck_amount();
		lby = player->lby();
		thirdperson_recoil = player->thirdperson_recoil();
		sim_time = player->simulation_time();
		old_sim_time = player->old_simtime();
		abs_origin = player->get_abs_origin();
		origin = player->origin();
		velocity = player->velocity();
		mins = player->bb_mins();
		maxs = player->bb_maxs();
		eye_angles = player->eye_angles();
		abs_angles = player->get_abs_angles();
		collision_change_time = player->collision_change_time();
		collision_change_origin = player->collision_change_origin();
		ground_entity = player->ground_entity();

		choke = std::clamp(math::time_to_ticks(sim_time - old_sim_time), 0, 64);

		land_time = 0.f;
		anim_speed = 0.f;
		landed = false;
		shifting = false;

		walking = player->is_walking();
		strafing = player->strafing();

		anim_time = interfaces::global_vars->cur_time;

		player->store_layer(sim_orig.layers);

		on_ground = flags & fl_onground;
	}

	__forceinline void update_shot(records_t* last)
	{
		auto weapon = ptr->get_active_weapon();
		if (last && weapon)
		{
			last_shot_time = weapon->last_shot_time();
			shooting = sim_time >= last_shot_time && last_shot_time > last->sim_time;
		}
	}

	__forceinline void update_dormant(int dormant_ticks)
	{
		dormant = false;
	}

	__forceinline void restore(c_csplayer* player)
	{
		player->velocity() = velocity;
		player->flags() = flags;
		player->duck_amount() = duck_amount;

		player->set_layer(sim_orig.layers);

		player->lby() = lby;
		player->thirdperson_recoil() = thirdperson_recoil;

		player->origin() = origin;
		player->set_abs_origin(abs_origin);
	}

	bool is_valid(bool deadtime = true);

	__forceinline void reset()
	{
		valid = false;
		valid_tick = false;
		dormant = false;
		walking = false;
		strafing = false;
		ptr = nullptr;
		flags = 0;
		eflags = 0;
		choke = 0;
		resolver_side = 0;
		duck_amount = 0.f;
		lby = 0.f;
		thirdperson_recoil = 0.f;
		sim_time = 0.f;
		old_sim_time = 0.f;
		land_time = 0.f;
		anim_time = 0.f;
		anim_speed = 0.f;
		interp_time = 0.f;
		last_shot_time = 0.f;
		collision_change_time = 0.f;
		last_eyeang_diff = 0.f;
		old_diff = 0.f;
		last_eyeang = 0.f;
		last_eyeang_diff_time = 0.f;

		ground_entity = 0;

		landing = false;
		landed = false;
		on_ground = false;
		shooting = false;
		crouch_jumping = false;
		fix_jitter_angle = false;

		origin.reset();
		velocity.reset();
		mins.reset();
		maxs.reset();
		old_velocity.reset();
		abs_origin.reset();
		eye_angles.reset();
		abs_angles.reset();
		collision_change_origin.reset();

		sim_orig.reset();
		sim_left.reset();
		sim_right.reset();
		sim_zero.reset();

		bones_to_aim = nullptr;

		poses = {};
	}
};

class c_animation_fix
{
public:
	struct anim_player_t
	{
		bool teammate{};
		int dormant_ticks{};
		float old_spawn_time{};
		float old_aliveloop_cycle{};
		float old_simulation_time{};
		c_csplayer* ptr{};
		records_t backup_record{};
		records_t* last_record{};
		records_t* old_record{};
		std::deque< records_t > records{};

		__forceinline void reset_data()
		{
			teammate = false;
			dormant_ticks = 0;

			old_spawn_time = 0.f;
			old_aliveloop_cycle = 0.f;
			old_simulation_time = 0.f;

			if (ptr)
				ptr = nullptr;
			last_record = nullptr;
			old_record = nullptr;

			records.clear();
			backup_record.reset();
		}

		__forceinline void fix_land(records_t* record)
		{
			if (last_record)
			{
				auto& real_layers = record->sim_orig.layers;

				if (real_layers[4].cycle < 0.5f)
				{
					if (!(record->flags & fl_onground) || !(last_record->flags & fl_onground))
					{
						record->land_time = record->sim_time - (real_layers[4].cycle / real_layers[4].playback_rate);
						record->landing = record->land_time >= record->old_sim_time;
					}
				}

				if (record->landing && !record->landed)
				{
					if (record->land_time >= record->old_sim_time)
					{
						record->landed = true;
						record->on_ground = true;
					}
					else
						record->on_ground = last_record->flags & fl_onground;
				}
			}
		}

		__forceinline void fix_velocity(records_t* record)
		{
			auto weapon = ptr->get_active_weapon();
			if (!weapon)
				return;

			record->velocity.reset();
			record->velocity_for_animfix.reset();

			auto layers = record->sim_orig.layers;
			auto anim_velocity = ptr->velocity();
			anim_velocity -= ptr->base_velocity();

			if (!last_record)
			{
				if (layers[6].playback_rate > 0.0f && anim_velocity.length(false) > 0.f)
				{
					if (layers[6].weight > 0.0f)
					{
						auto max_speed = ptr->max_speed();

						if (record->flags & 6)
							max_speed *= 0.34f;
						else if (ptr->is_walking())
							max_speed *= 0.52f;

						auto anim_speed = layers[6].weight * max_speed;
						anim_velocity *= anim_speed / anim_velocity.length(false);
					}

					if (record->flags & fl_onground)
						anim_velocity.z = 0.f;
				}
				else
					anim_velocity.reset();

				record->velocity = anim_velocity;
				record->velocity_for_animfix = anim_velocity;
				return;
			}

			bool on_ground = (record->flags & fl_onground) && (last_record->flags & fl_onground);

			auto origin_diff = record->origin - last_record->origin;
			if (on_ground && layers[6].playback_rate <= 0.0f)
				anim_velocity.reset();

			if ((ptr->effects() & 8) != 0 || ptr->no_interp_perty() != ptr->old_no_interp_perty())
			{
				record->velocity.reset();
				record->velocity_for_animfix.reset();
				return;
			}

			if (record->choke < 2)
			{
				record->velocity = anim_velocity;
				record->velocity_for_animfix = anim_velocity;
				return;
			}

			auto choke_time = math::ticks_to_time(record->choke);
			auto origin_delta_length = origin_diff.length(false);

			if (choke_time > 0.0f && choke_time < 1.0f && origin_delta_length >= 1.f && origin_delta_length <= 1000000.0f)
			{
				anim_velocity = origin_diff / choke_time;
				anim_velocity.reset_invalid();

				record->velocity = anim_velocity;
				if (!on_ground)
				{
					auto currently_ducking = record->flags & 2;
					if ((last_record->flags & 2) != currently_ducking)
					{
						float duck_modifier;
						if (currently_ducking)
							duck_modifier = 9.f;
						else
							duck_modifier = -9.f;

						anim_velocity.z += duck_modifier;
					}
				}
			}

			float anim_speed = 0.f;

			auto speed_2d = anim_velocity.length(true);
			auto alive_loop_weight = layers[11].weight;
			if (speed_2d > 0.f && on_ground && alive_loop_weight > 0.0f && alive_loop_weight < 1.0f && layers[11].playback_rate == last_record->sim_orig.layers[11].playback_rate)
			{
				auto weigth_calc = (1.f - alive_loop_weight) * 0.35f;
				if (weigth_calc > 0.f && weigth_calc < 1.f)
					anim_speed = (weigth_calc * 260.f) + 143.f;
			}

			if (anim_speed > 0.0f)
			{
				anim_speed /= anim_velocity.length(true);
				anim_velocity.x *= anim_speed;
				anim_velocity.y *= anim_speed;
			}

			if (old_record && anim_velocity.length(false) >= 400.f)
			{
				auto prev_velocity = (last_record->origin - old_record->origin) / math::ticks_to_time(last_record->choke);
				if (prev_velocity.valid() && !on_ground)
				{
					auto current_dir = math::normalize(math::rad_to_deg(std::atan2(anim_velocity.y, anim_velocity.x)));
					auto old_dir = math::normalize(math::rad_to_deg(std::atan2(prev_velocity.y, prev_velocity.x)));

					auto avg_dir = current_dir - old_dir;
					avg_dir = math::deg_to_rad(math::normalize(current_dir + avg_dir * 0.5f));

					auto dir_cos = std::cos(avg_dir);
					auto dir_sin = std::sin(avg_dir);

					anim_velocity.x = dir_cos * anim_velocity.length(true);
					anim_velocity.y = dir_sin * anim_velocity.length(true);
				}
			}

			if (!(record->flags & fl_onground))
			{
				float max_speed = ptr->max_speed();

				// fix velocity values in this fucking broken game
				// why the fuck it gets too high before jump?
				// reduce speed when enemy going to jump
				// @opai
				if (record->sim_orig.layers[4].weight <= 0.1f)
					anim_velocity.z *= 0.1f;

				// clamp this piece of shit
				auto length = anim_velocity.length(false);
				if (length > max_speed)
				{
					float clamp_velocity = ((anim_velocity / length) * (max_speed / length)).z;
					anim_velocity.z = clamp_velocity;
				}
			}

			// detect fakewalking players
			if (anim_velocity.valid() && anim_velocity.length(false) >= 0.1f)
			{
				if (record->sim_orig.layers[4].playback_rate == 0.0f
					&& record->sim_orig.layers[5].playback_rate == 0.0f
					&& record->sim_orig.layers[6].playback_rate == 0.0f
					&& record->flags & fl_onground)
				{
					record->velocity.reset();
					anim_velocity.reset();
				}
			}

			anim_velocity.reset_invalid();
			record->velocity_for_animfix = anim_velocity;

			record->velocity.reset_invalid();
			record->velocity_for_animfix.reset_invalid();
		}

		__forceinline void force_update()
		{
			interfaces::global_vars->cur_time = ptr->simulation_time();
			interfaces::global_vars->frame_time = interfaces::global_vars->interval_per_tick;

			ptr->force_update();
			ptr->invalidate_physics_recursive(0x1 | 0x2 | 0x4 | 0x8);
		}

		void adjust_roll_angle(records_t* record);

		void simulate_animation_side(records_t* record, int side = side_original);
		void build_bones(records_t* record, simulated_data_t* sim);
		void update_animations();
	};

private:
	std::array< anim_player_t, 65 > players{};

public:
	__forceinline anim_player_t* get_animation_player(int idx)
	{
		return &players[idx];
	}

	__forceinline float get_lerp_time()
	{
		float total_lerp = 0.f;

		float update_rate_value = cvars::cl_updaterate->get_float();
		if (!interfaces::engine->is_hltv())
		{
			if (cvars::sv_minupdaterate && cvars::sv_maxupdaterate)
				update_rate_value = std::clamp< float >(update_rate_value, cvars::sv_minupdaterate->get_float(), cvars::sv_maxupdaterate->get_float());
		}

		bool use_interp = cvars::cl_interpolate->get_int();
		if (use_interp)
		{
			float lerp_ratio = cvars::cl_interp_ratio->get_float();
			if (lerp_ratio == 0.f)
				lerp_ratio = 1.f;

			float lerp_amount = cvars::cl_interp->get_float();

			if (cvars::sv_client_min_interp_ratio && cvars::sv_client_max_interp_ratio && cvars::sv_client_min_interp_ratio->get_float() != -1)
			{
				lerp_ratio = std::clamp< float >(lerp_ratio, cvars::sv_client_min_interp_ratio->get_float(), cvars::sv_client_max_interp_ratio->get_float());
			}
			else
			{
				if (lerp_ratio == 0.f)
					lerp_ratio = 1.f;
			}

			total_lerp = std::max< float >(lerp_amount, lerp_ratio / update_rate_value);
		}
		else
		{
			total_lerp = 0.0f;
		}

		return total_lerp;
	}

	__forceinline records_t* get_latest_record(c_csplayer* player)
	{
		auto anim_player = this->get_animation_player(player->index());
		if (!anim_player || anim_player->records.empty())
			return nullptr;

		auto record = std::find_if(anim_player->records.begin(), anim_player->records.end(), [&](records_t& record) { return record.valid_tick; });

		if (record != anim_player->records.end())
			return &*record;

		return nullptr;
	}

	__forceinline records_t* get_oldest_record(c_csplayer* player)
	{
		auto anim_player = this->get_animation_player(player->index());
		if (!anim_player || anim_player->records.empty() || !g_ctx.lagcomp)
			return nullptr;

		auto record = std::find_if(anim_player->records.rbegin(), anim_player->records.rend(), [&](records_t& record) { return record.valid_tick; });

		if (record != anim_player->records.rend())
			return &*record;

		return nullptr;
	}

	__forceinline std::vector< records_t* > get_all_records(c_csplayer* player)
	{
		auto anim_player = this->get_animation_player(player->index());
		if (!anim_player || anim_player->records.empty())
			return {};

		std::vector< records_t* > out{};
		for (auto it = anim_player->records.begin(); it != anim_player->records.end(); it = next(it))
			if ((it)->valid_tick)
				out.emplace_back(&*it);

		return out;
	}

	__forceinline std::pair< records_t*, records_t* > get_interp_record(c_csplayer* player)
	{
		auto anim_player = this->get_animation_player(player->index());
		if (!anim_player || anim_player->records.empty() || anim_player->records.size() <= 1)
			return std::make_pair(nullptr, nullptr);

		auto it = anim_player->records.begin(), prev = it;

		for (; it != anim_player->records.end(); it = next(it))
		{
			if (prev->valid_tick && !it->valid_tick)
			{
				return std::make_pair(&*prev, &*it);
			}
			prev = it;
		}

		return std::make_pair(nullptr, nullptr);
	}

	void update_valid_ticks();
	void force_data_for_render();
	void on_net_update_and_render_after(int stage);
};