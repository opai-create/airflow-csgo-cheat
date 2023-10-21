#include "../features.h"
#include "../../includes.h"
#include "server_animations.h"

// proceed server-sided animation logic on client
// allows to play animations without delays
// and make local animations a WAAY better
// credits:
// UC (activity modifiers)
// infirms (researching)

namespace server_animations
{
	// https://www.unknowncheats.me/forum/3146777-post4.html
#pragma runtime_checks("", off)
	__forceinline float get_sequence_dist(int sequence)
	{
		auto hdr = g_ctx.local->get_studio_hdr();
		if (!hdr)
			return -1.f;

		static auto get_sequence_linear_motion = patterns::get_sequence_linear_motion.as<void(__fastcall*)(void*, int, float*, vector3d*)>();

		auto poses = g_ctx.local->pose_parameter();

		vector3d retn;
		get_sequence_linear_motion(hdr, sequence, poses.data(), &retn);
		return retn.length(false);
	}
#pragma runtime_checks("", restore)

	__forceinline void set_sequence(c_animstate* state, c_animation_layers* layer, const int& activity)
	{
		state->set_layer_sequence(layer, state->select_sequence_from_activity_modifier(activity));
	}

	void play_landing_animations(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers)
	{
		auto rebuilt_state = &local_anim->rebuilt_state;

		auto land_or_climb = &layers[animation_layer_movement_land_or_climb];
		auto jump_or_fall = &layers[animation_layer_movement_jump_or_fall];

		auto ground_entity = (c_baseentity*)interfaces::entity_list->get_entity_handle(g_ctx.local->ground_entity());

		auto& flags = g_ctx.local->flags();

		bool landed = (flags & fl_onground) && !(local_anim->old_flags & fl_onground);
		bool jumped = !(flags & fl_onground) && (local_anim->old_flags & fl_onground);

		local_anim->old_flags = flags;

		float distance_fell = 0;
		if (jumped)
			rebuilt_state->left_ground_height = rebuilt_state->position_current.z;

		if (landed)
		{
			distance_fell = std::abs(rebuilt_state->left_ground_height - rebuilt_state->position_current.z);
			float distance_bias_range = math::bias(math::reval_map_clamped(rebuilt_state->next_twitch_time, 12.0f, 72.0f, 0.0f, 1.0f), 0.4f);
			rebuilt_state->land_anim_multiplier = std::clamp(math::bias(state->duration_in_air, 0.3f), 0.1f, 1.0f);
			rebuilt_state->duck_additional = std::max<float>(rebuilt_state->land_anim_multiplier, distance_bias_range);
		}
		else
		{
			rebuilt_state->duck_additional = math::approach(0, rebuilt_state->duck_additional, state->last_update_increment * 2);
		}

		bool old_on_ladder = local_anim->old_movetype == movetype_ladder;
		bool on_ladder = g_ctx.local->move_type() == movetype_ladder;

		if (!old_on_ladder && on_ladder)
			set_sequence(state, land_or_climb, act_csgo_climb_ladder);
		else if (old_on_ladder && !on_ladder)
			set_sequence(state, land_or_climb, act_csgo_fall);
		else
		{
			if ((flags & fl_onground))
			{
				if (!local_anim->landing && landed)
				{
					int activity = state->duration_in_air > 1.f ? act_csgo_land_heavy : act_csgo_land_light;
					set_sequence(state, land_or_climb, activity);

					local_anim->landing = true;
				}
			}
			else
				local_anim->landing = false;

			if ((g_ctx.jump_buttons & in_jump) && !ground_entity)
				set_sequence(state, jump_or_fall, act_csgo_jump);
		}

		local_anim->old_movetype = g_ctx.local->move_type();
	}

	void play_idle_animations(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers)
	{
		auto rebuilt_state = &local_anim->rebuilt_state;

		auto adjust = &layers[animation_layer_adjust];
		auto move = &layers[animation_layer_movement_move];
		auto strafe_change = &layers[animation_layer_movement_strafechange];

		bool started_moving_this_frame = false;
		bool stopped_moving_this_frame = false;

		if (rebuilt_state->velocity_length_xy > 0.f)
		{
			stopped_moving_this_frame = false;

			started_moving_this_frame = (rebuilt_state->duration_moving <= 0);
			rebuilt_state->duration_still = 0;
			rebuilt_state->duration_moving += state->last_update_increment;
		}
		else
		{
			started_moving_this_frame = false;

			stopped_moving_this_frame = (rebuilt_state->duration_still <= 0);
			rebuilt_state->duration_moving = 0;
			rebuilt_state->duration_still += state->last_update_increment;
		}

		if (!rebuilt_state->adjust_started && stopped_moving_this_frame && rebuilt_state->on_ground && !rebuilt_state->on_ladder && local_anim->landing && rebuilt_state->stutter_step < 50.f)
		{
			set_sequence(state, adjust, act_csgo_idle_adjust_stoppedmoving);
			rebuilt_state->adjust_started = true;
		}

		int layer_activity = g_ctx.local->get_sequence_activity(adjust->sequence);

		if (layer_activity == act_csgo_idle_adjust_stoppedmoving || layer_activity == act_csgo_idle_turn_balanceadjust)
		{
			if (rebuilt_state->adjust_started && rebuilt_state->speed_as_portion_of_crouch_top_speed <= 0.25f)
			{
				float previous_weight = adjust->weight;
				state->increment_layer_cycle(adjust, false);
				state->set_layer_weight(adjust, state->get_layer_ideal_weight_from_seq_cycle(adjust));
				state->set_layer_weight_rate(adjust, previous_weight);

				rebuilt_state->adjust_started = !(state->is_layer_sequence_finished(adjust, state->last_update_increment));
			}
			else
			{
				rebuilt_state->adjust_started = false;

				float previous_weight = adjust->weight;
				state->set_layer_weight(adjust, math::approach(0, previous_weight, state->last_update_increment * 5.f));
				state->set_layer_weight_rate(adjust, previous_weight);
			}
		}

		if (rebuilt_state->velocity_length_xy <= 1.f && rebuilt_state->on_ground && !rebuilt_state->on_ladder && !local_anim->landing
			&& state->last_update_increment > 0 && std::abs(math::angle_diff(state->abs_yaw_last, state->abs_yaw) / state->last_update_increment > 120.f))
		{
			set_sequence(state, adjust, act_csgo_idle_turn_balanceadjust);
			rebuilt_state->adjust_started = true;
		}

		if (rebuilt_state->velocity_length_xy > 0 && rebuilt_state->on_ground)
		{
			float raw_yaw_ideal = (atan2(-rebuilt_state->velocity[1], -rebuilt_state->velocity[0]) * 180 / M_PI);
			if (raw_yaw_ideal < 0)
				raw_yaw_ideal += 360;

			rebuilt_state->move_yaw_ideal = math::normalize(math::angle_diff(raw_yaw_ideal, rebuilt_state->abs_yaw));
		}

		rebuilt_state->move_yaw_current_to_ideal = math::normalize(math::angle_diff(rebuilt_state->move_yaw_ideal, rebuilt_state->move_yaw));

		if (started_moving_this_frame && rebuilt_state->move_weight <= 0.f)
		{
			rebuilt_state->move_yaw = rebuilt_state->move_yaw_ideal;

			int move_sequence = move->sequence;
			if (move_sequence != -1)
			{
				auto hdr = g_ctx.local->get_studio_hdr();
				if (hdr)
				{
					auto sequence_desc = hdr->get_sequence_desc(move->sequence);
					if (sequence_desc)
					{
						int anim_tags = *(int*)((std::uintptr_t)sequence_desc + 0xC4);
						if (anim_tags > 0)
						{
							// fuck it.
							// assign leg dir on start of running
							// anyway it gets the same angle as server
							// for moving animations weight should be rebuilt btw
							rebuilt_state->primary_cycle = state->primary_cycle;
						}
					}
				}
			}
		}
		else
		{
			if (strafe_change->weight >= 1)
				rebuilt_state->move_yaw = rebuilt_state->move_yaw_ideal;
			else
			{
				float move_weight = math::interpolate_inversed(rebuilt_state->anim_duck_amount,
					std::clamp(rebuilt_state->speed_as_portion_of_walk_top_speed, 0.f, 1.f),
					std::clamp(rebuilt_state->speed_as_portion_of_crouch_top_speed, 0.f, 1.f));

				float ratio = math::bias(move_weight, 0.18f) + 0.1f;

				rebuilt_state->move_yaw = math::normalize(rebuilt_state->move_yaw + (rebuilt_state->move_yaw_current_to_ideal * ratio));
			}
		}
	}

	void play_move_animations(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers)
	{
		auto rebuilt_state = &local_anim->rebuilt_state;

		auto hdr = g_ctx.local->get_studio_hdr();
		if (!hdr)
			return;

		static auto get_weapon_prefix = patterns::get_weapon_prefix.as<const char* (__thiscall*)(void*)>();

		auto move = &layers[animation_layer_movement_move];
		auto land_or_climb = &layers[animation_layer_movement_land_or_climb];

		char weapon_move_sequence[64]{};
		sprintf_s(weapon_move_sequence, xor_c("move_%s"), get_weapon_prefix(state));

		int weapon_move_seq = func_ptrs::lookup_sequence(g_ctx.local, weapon_move_sequence);
		if (weapon_move_seq == -1)
			weapon_move_seq = func_ptrs::lookup_sequence(g_ctx.local, xor_c("move"));

		// rebuilt formulas again
		{
			float move_weight = math::interpolate_inversed(rebuilt_state->anim_duck_amount, std::clamp(rebuilt_state->speed_as_portion_of_walk_top_speed, 0.f, 1.f),
				std::clamp(rebuilt_state->speed_as_portion_of_crouch_top_speed, 0.f, 1.f));

			if (rebuilt_state->move_weight <= move_weight)
				rebuilt_state->move_weight = move_weight;
			else
			{
				rebuilt_state->move_weight = math::approach(move_weight, rebuilt_state->move_weight, state->last_update_increment
					* math::reval_map_clamped(rebuilt_state->stutter_step, 0.0f, 100.0f, 2, 20));
			}

			vector3d move_yaw_direction;
			math::angle_to_vectors({ 0, math::normalize(rebuilt_state->abs_yaw + rebuilt_state->move_yaw + 180.f), 0 }, move_yaw_direction);
			float yaw_delta_abs = abs(rebuilt_state->velocity_normalized_non_zero.dot(move_yaw_direction));
			rebuilt_state->move_weight *= math::bias(yaw_delta_abs, 0.2f);
		}

		float move_weight_with_air_smooth = rebuilt_state->move_weight * rebuilt_state->in_air_smooth_value;
		move_weight_with_air_smooth *= std::max<float>((1.0f - land_or_climb->weight), 0.55f);

		float move_cycle_rate = 0;
		if (rebuilt_state->velocity_length_xy > 0)
		{
			move_cycle_rate = g_ctx.local->get_sequence_cycle_rate(hdr, weapon_move_seq);

			float dist = get_sequence_dist(weapon_move_seq);

			float sequence_ground_speed = std::max<float>(dist / (1.0f / move_cycle_rate), 0.001f);

			move_cycle_rate *= rebuilt_state->velocity_length_xy / sequence_ground_speed;
			move_cycle_rate *= math::interpolate_inversed(rebuilt_state->walk_run_transition, 1.0f, 0.85f);
		}

		float local_cycle_increment = (move_cycle_rate * state->last_update_increment);
		rebuilt_state->primary_cycle = math::clamp_cycle(rebuilt_state->primary_cycle + local_cycle_increment);
		/*
			printf("\n weapon_move_seq-> %d\n local_cycle_increment-> %f\n rebuilt_state->primary_cycle-> %f\n move_weight_with_air_smooth-> %f\n",
				weapon_move_seq, local_cycle_increment, rebuilt_state->primary_cycle, move_weight_with_air_smooth);*/

		rebuilt_state->update_layer(move, weapon_move_seq, local_cycle_increment, rebuilt_state->primary_cycle, move_weight_with_air_smooth, animation_layer_movement_move);
	}

	void update_strafe_state(c_animstate* state, local_anims_t* local_anim)
	{
		auto rebuilt_state = &local_anim->rebuilt_state;

		auto buttons = g_ctx.cmd->buttons;

		vector3d forward;
		vector3d right;
		vector3d up;

		math::angle_to_vectors(vector3d(0, state->abs_yaw, 0), forward, right, up);
		right = right.normalized();

		auto velocity = rebuilt_state->velocity_normalized_non_zero;
		auto speed = rebuilt_state->speed_as_portion_of_walk_top_speed;

		float vel_to_right_dot = velocity.dot(right);
		float vel_to_foward_dot = velocity.dot(forward);

		bool move_right = (buttons & (in_moveright)) != 0;
		bool move_left = (buttons & (in_moveleft)) != 0;
		bool move_forward = (buttons & (in_forward)) != 0;
		bool move_backward = (buttons & (in_back)) != 0;

		bool strafe_right = (speed >= 0.73f && move_right && !move_left && vel_to_right_dot < -0.63f);
		bool strafe_left = (speed >= 0.73f && move_left && !move_right && vel_to_right_dot > 0.63f);
		bool strafe_forward = (speed >= 0.65f && move_forward && !move_backward && vel_to_foward_dot < -0.55f);
		bool strafe_backward = (speed >= 0.65f && move_backward && !move_forward && vel_to_foward_dot > 0.55f);

		g_ctx.local->strafing() = (strafe_right || strafe_left || strafe_forward || strafe_backward);
	}

	void update_rebuilt_state_vars(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers)
	{
		auto rebuilt_state = &local_anim->rebuilt_state;
		auto velocity = g_ctx.local->velocity();
		auto max_speed_run = g_ctx.weapon ?
			std::max<float>(g_ctx.local->is_scoped() ? g_ctx.weapon_info->max_speed_alt : g_ctx.weapon_info->max_speed, 0.001f)
			: 260.f;

		rebuilt_state->player = g_ctx.local;
		rebuilt_state->weapon = state->weapon;
		rebuilt_state->weapon_last = state->weapon_last;
		rebuilt_state->weapon_last_bone_setup = state->weapon_last_bone_setup;

		velocity.z = 0.f;

		// update server sided vars
		{
			rebuilt_state->on_ground = g_ctx.local->flags() & fl_onground;
			rebuilt_state->on_ladder = g_ctx.local->move_type() == movetype_ladder;
			rebuilt_state->position_current = g_ctx.local->origin();
			rebuilt_state->abs_yaw = state->abs_yaw;

			rebuilt_state->anim_duck_amount = std::clamp<float>(math::approach(std::clamp(g_ctx.local->duck_amount() + rebuilt_state->duck_additional, 0.f, 1.f),
				rebuilt_state->anim_duck_amount, state->last_update_increment * 6.0f), 0.f, 1.f);
		}

		// logic from CCSGOPlayerAnimState::SetUpVelocity
		{
			rebuilt_state->velocity = math::approach(velocity, rebuilt_state->velocity, state->last_update_increment * 2000.f);
			rebuilt_state->velocity_normalized = rebuilt_state->velocity.normalized();

			rebuilt_state->velocity_length_xy = std::min<float>(rebuilt_state->velocity.length(false), 260.f);

			if (rebuilt_state->velocity_length_xy > 0)
				rebuilt_state->velocity_normalized_non_zero = rebuilt_state->velocity_normalized;

			rebuilt_state->speed_as_portion_of_run_top_speed = std::clamp(rebuilt_state->velocity_length_xy / max_speed_run, 0.f, 1.f);
			rebuilt_state->speed_as_portion_of_walk_top_speed = rebuilt_state->velocity_length_xy / (max_speed_run * 0.52f);
			rebuilt_state->speed_as_portion_of_crouch_top_speed = rebuilt_state->velocity_length_xy / (max_speed_run * 0.34f);
		}

		// logic from CCSGOPlayerAnimState::SetUpMovement
		{
			if (rebuilt_state->walk_run_transition > 0 && rebuilt_state->walk_run_transition < 1)
			{
				if (rebuilt_state->walk_to_run_transition_state == 0)
					rebuilt_state->walk_run_transition += state->last_update_increment * 2.f;
				else
					rebuilt_state->walk_run_transition -= state->last_update_increment * 2.f;

				rebuilt_state->walk_run_transition = std::clamp(rebuilt_state->walk_run_transition, 0.f, 1.f);
			}

			if (rebuilt_state->velocity_length_xy > (260.f * 0.52f) && rebuilt_state->walk_to_run_transition_state == 1)
			{
				rebuilt_state->walk_to_run_transition_state = 0;
				rebuilt_state->walk_run_transition = std::max<float>(0.01f, rebuilt_state->walk_run_transition);
			}
			else if (rebuilt_state->velocity_length_xy < (260.f * 0.52f) && rebuilt_state->walk_to_run_transition_state == 0)
			{
				rebuilt_state->walk_to_run_transition_state = 1;
				rebuilt_state->walk_run_transition = std::min<float>(0.99f, rebuilt_state->walk_run_transition);
			}

			if (g_ctx.local->move_state() != rebuilt_state->previous_move_state)
				rebuilt_state->stutter_step += 10;

			rebuilt_state->previous_move_state = g_ctx.local->move_state();
			rebuilt_state->stutter_step = std::clamp(math::approach(0, rebuilt_state->stutter_step, state->last_update_increment * 40), 0.f, 100.f);

			rebuilt_state->in_air_smooth_value = math::approach(state->on_ground ? 1 : 0,
				rebuilt_state->in_air_smooth_value, math::interpolate_inversed(rebuilt_state->anim_duck_amount, 8.f, 16.f)
				* state->last_update_increment);

			rebuilt_state->in_air_smooth_value = std::clamp(rebuilt_state->in_air_smooth_value, 0.f, 1.f);
		}
	}

	void run(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers)
	{
		play_landing_animations(state, local_anim, layers);
	}

	// change already calculated values to proper one
	// and apply them on matrix layer
	void recalculate(c_animstate* state, local_anims_t* local_anim, c_animation_layers* layers)
	{
		update_rebuilt_state_vars(state, local_anim, layers);
		play_idle_animations(state, local_anim, layers);
		play_move_animations(state, local_anim, layers);
		update_strafe_state(state, local_anim);
	}
}