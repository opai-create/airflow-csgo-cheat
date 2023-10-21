#include "globals.hpp"
#include "threaded_animstate.hpp"

constexpr auto CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX = 0.8f;
constexpr auto CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX_SCOPED = 4.2f;
constexpr auto CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MAX = 58.0f;
constexpr auto CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MIN = -58.0f;
constexpr auto CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX = 90.0f;
constexpr auto CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN = -90.0f;

using ANIMSTATE_FUNC_FN = void(__thiscall*)(c_animation_state*);

float get_sequence_animtag(c_studio_hdr* hdr, int sequence, int tag) {
	auto studiohdr = *reinterpret_cast<studiohdr_t**>(hdr);
	if (!studiohdr)
		return 0.f;

	if (!studiohdr || sequence >= studiohdr->local_seq)
		return 0.f;

	auto& sequence_desc = hdr->get_sequence_desc(sequence);

	int anim_tags = sequence_desc.num_animtags;
	if (anim_tags == 0)
		return 0.f;

	for (int i = 0; i < anim_tags; ++i) {
		auto array_index = 12 * i;

		auto anim_tag_ptr = sequence_desc.anim_tag(i);
		auto anim_tag = *reinterpret_cast<int*>(anim_tag_ptr);

		if (anim_tag == -1)
			continue;

		if (anim_tag == 0) {
			auto anim_tag_name = reinterpret_cast<const char*>(anim_tag_ptr + *reinterpret_cast<int*>(anim_tag_ptr + 8));
			auto func = offsets::index_from_anim_tag_name.cast<int(__thiscall*)(const char*)>();
			anim_tag = func(anim_tag_name);
		}

		if (anim_tag == tag) {
			auto cycle = *reinterpret_cast<float*>(anim_tag_ptr + 4);

			if (cycle >= 0.f && cycle < 1.f)
				return cycle;
		}
	}

	return 0.f;
}

float get_any_sequence_animtag(c_studio_hdr* hdr, int sequence, int tag, float def_value) {
	auto studiohdr = *reinterpret_cast<studiohdr_t**>(hdr);
	if (!studiohdr)
		return 0.f;

	if (!studiohdr || sequence >= studiohdr->local_seq)
		return def_value;

	auto& sequence_desc = hdr->get_sequence_desc(sequence);

	int anim_tags = sequence_desc.num_animtags;
	if (anim_tags == 0)
		return def_value;

	for (int i = 0; i < anim_tags; ++i) {
		auto array_index = 12 * i;

		auto anim_tag_ptr = sequence_desc.anim_tag(i);
		auto anim_tag = *reinterpret_cast<int*>(anim_tag_ptr);

		if (anim_tag == -1)
			continue;

		if (anim_tag == 0) {
			auto anim_tag_name = reinterpret_cast<const char*>(anim_tag_ptr + *reinterpret_cast<int*>(anim_tag_ptr + 8));
			auto func = offsets::index_from_anim_tag_name.cast<int(__thiscall*)(const char*)>();
			anim_tag = func(anim_tag_name);
		}

		if (anim_tag == tag) {
			auto cycle = *reinterpret_cast<float*>(anim_tag_ptr + 4);
			return cycle;
		}
	}

	return def_value;
}


void c_threaded_animstate::setup_velocity(c_animation_state* state, float curtime) {
	auto player = reinterpret_cast<c_cs_player*>(state->player);
	auto hdr = player->get_studio_hdr();
	if (!hdr)
		return;

	auto weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(player->active_weapon()));
	if (!weapon)
		return;

	auto weapon_info = HACKS->weapon_system->get_weapon_data(weapon->item_definition_index());
	auto max_speed = weapon && weapon_info ?
		std::max<float>((player->is_scoped() ? weapon_info->max_speed_alt : weapon_info->max_speed), 0.001f)
		: CS_PLAYER_SPEED_RUN;

	auto abs_velocity = player->velocity();

	state->velocity_length_z = abs_velocity.z;
	abs_velocity.z = 0.f;

	state->player_is_accelerating = (state->velocity_last.length_sqr() < abs_velocity.length_sqr());

	state->velocity = math::approach(abs_velocity, state->velocity, state->last_update_increment * 2000);
	state->velocity_normalized = state->velocity.normalized();

	state->velocity_length_xy = std::min<float>(state->velocity.length(), CS_PLAYER_SPEED_RUN);

	if (state->velocity_length_xy > 0)
		state->velocity_normalized_non_zero = state->velocity_normalized;

	state->speed_as_portion_of_run_top_speed = std::clamp<float>(state->velocity_length_xy / max_speed, 0, 1);
	state->speed_as_portion_of_walk_top_speed = state->velocity_length_xy / (max_speed * CS_PLAYER_SPEED_WALK_MODIFIER);
	state->speed_as_portion_of_crouch_top_speed = state->velocity_length_xy / (max_speed * CS_PLAYER_SPEED_DUCK_MODIFIER);

	if (state->speed_as_portion_of_walk_top_speed >= 1)
		state->static_approach_speed = state->velocity_length_xy;
	else if (state->speed_as_portion_of_walk_top_speed < 0.5f)
		state->static_approach_speed = math::approach(80, state->static_approach_speed, state->last_update_increment * 60);

	bool started_moving_this_frame = false;
	bool stopped_moving_this_frame = false;

	if (state->velocity_length_xy > 0) {
		started_moving_this_frame = (state->duration_moving <= 0);
		state->duration_still = 0;
		state->duration_moving += state->last_update_increment;
	}
	else {
		stopped_moving_this_frame = (state->duration_still <= 0);
		state->duration_moving = 0;
		state->duration_still += state->last_update_increment;
	}

	auto adjust = &player->animlayers()[ANIMATION_LAYER_ADJUST];

	if (!state->adjust_started && stopped_moving_this_frame && state->on_ground && !state->on_ladder && !state->landing && state->stutter_step < 50.f) {
		state->set_layer_sequence(adjust, state->select_sequence_from_activity_modifier(ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING));
		state->adjust_started = true;
	}

	int layer_activity = player->get_sequence_activity(adjust->sequence);
	if (layer_activity == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING || layer_activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST) {
		if (state->adjust_started && state->speed_as_portion_of_crouch_top_speed <= 0.25f) {
			float previous_weight = adjust->weight;
			state->increment_layer_cycle(adjust, false);
			state->set_layer_weight(adjust, state->get_layer_ideal_weight_from_seq_cycle(adjust, ACT_CSGO_IDLE_TURN_BALANCEADJUST));
			state->set_layer_weight_rate(adjust, previous_weight);

			state->adjust_started = !(state->is_layer_sequence_finished(adjust, state->last_update_increment));
		}
		else {
			state->adjust_started = false;

			float previous_weight = adjust->weight;
			state->set_layer_weight(adjust, math::approach(0, previous_weight, state->last_update_increment * 5.f));
			state->set_layer_weight_rate(adjust, previous_weight);
		}
	}

	state->abs_yaw_last = state->abs_yaw;
	state->abs_yaw = std::clamp<float>(state->abs_yaw, -360, 360);

	auto eye_delta = math::angle_diff(state->eye_yaw, state->abs_yaw);

	auto speed_portion_walk = state->speed_as_portion_of_walk_top_speed;
	auto speed_portion_duck = state->speed_as_portion_of_crouch_top_speed;
	auto transition = state->walk_run_transition;
	auto duck_amount = state->anim_duck_amount;

	auto aim_matrix_width_range = math::lerp(std::clamp(speed_portion_walk, 0.f, 1.f), 1.f,
		math::lerp(transition, 0.8f, 0.5f));

	if (duck_amount > 0)
		aim_matrix_width_range = math::lerp(duck_amount * std::clamp(speed_portion_duck, 0.f, 1.f),
			aim_matrix_width_range, 0.5f);

	auto yaw_max = state->aim_yaw_max * aim_matrix_width_range;
	auto yaw_min = state->aim_yaw_min * aim_matrix_width_range;

	if (eye_delta > yaw_max)
	{
		state->abs_yaw = state->eye_yaw - abs(yaw_max);
	}
	else if (eye_delta < yaw_min)
	{
		state->abs_yaw = state->eye_yaw + abs(yaw_min);
	}
	state->abs_yaw = math::normalize_yaw(state->abs_yaw);

	auto& lower_body_realign_timer = this->lower_body_realign_timer[player->index()];
	if (state->on_ground) {
		if (state->velocity_length_xy > 0.1f) {
			state->abs_yaw = math::approach_angle(state->eye_yaw, state->abs_yaw, state->last_update_increment * (30.0f + 20.0f * state->walk_run_transition));
		}
		else {
			state->abs_yaw = math::approach_angle(player->lower_body_yaw(), state->abs_yaw, state->last_update_increment * 100.f);
		}
	}

	if (state->velocity_length_xy <= 1.f && state->on_ground && !state->on_ladder && !state->landing
		&& state->last_update_increment > 0 && (std::abs(math::angle_diff(state->abs_yaw_last, state->abs_yaw)) / state->last_update_increment) > 120.f) {
		state->set_layer_sequence(adjust, state->select_sequence_from_activity_modifier(ACT_CSGO_IDLE_TURN_BALANCEADJUST));
		state->adjust_started = true;
	}

	if (adjust->weight > 0) {
		state->increment_layer_cycle(adjust, false);
		state->increment_layer_weight(adjust);
	}

	if (state->velocity_length_xy > 0 && state->on_ground) {
		float raw_yaw_ideal = (std::atan2(-state->velocity[1], -state->velocity[0]) * 180 / M_PI);
		if (raw_yaw_ideal < 0)
			raw_yaw_ideal += 360;

		state->move_yaw_ideal = math::normalize_yaw(math::angle_diff(raw_yaw_ideal, state->abs_yaw));
	}

	state->move_yaw_current_to_ideal = math::normalize_yaw(math::angle_diff(state->move_yaw_ideal, state->move_yaw));

	auto move = &player->animlayers()[ANIMATION_LAYER_MOVEMENT_MOVE];
	auto strafe_change = &player->animlayers()[ANIMATION_LAYER_MOVEMENT_STRAFECHANGE];

	if (started_moving_this_frame && state->move_weight <= 0.f) {
		state->move_yaw = state->move_yaw_ideal;

		auto move_sequence = move->sequence;
		if (move_sequence != -1) {
			auto& sequence_desc = hdr->get_sequence_desc(move->sequence);
			auto anim_tags = sequence_desc.num_animtags;

			if (anim_tags > 0) {
				if (std::abs(math::angle_diff(state->move_yaw, 180)) <= EIGHT_WAY_WIDTH) //N
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_N);
				else if (std::abs(math::angle_diff(state->move_yaw, 135)) <= EIGHT_WAY_WIDTH) //NE
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_NE);
				else if (std::abs(math::angle_diff(state->move_yaw, 90)) <= EIGHT_WAY_WIDTH) //E
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_E);
				else if (std::abs(math::angle_diff(state->move_yaw, 45)) <= EIGHT_WAY_WIDTH) //SE
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_SE);
				else if (std::abs(math::angle_diff(state->move_yaw, 0)) <= EIGHT_WAY_WIDTH) //S
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_S);
				else if (std::abs(math::angle_diff(state->move_yaw, -45)) <= EIGHT_WAY_WIDTH) //SW
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_SW);
				else if (std::abs(math::angle_diff(state->move_yaw, -90)) <= EIGHT_WAY_WIDTH) //W
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_W);
				else if (std::abs(math::angle_diff(state->move_yaw, -135)) <= EIGHT_WAY_WIDTH) //NW
					state->primary_cycle = get_sequence_animtag(hdr, move_sequence, ANIMTAG_STARTCYCLE_NW);
			}
		}
	}
	else {
		if (strafe_change->weight >= 1)
			state->move_yaw = state->move_yaw_ideal;
		else {
			float move_weight = math::lerp(state->anim_duck_amount,
				std::clamp(state->speed_as_portion_of_walk_top_speed, 0.f, 1.f),
				std::clamp(state->speed_as_portion_of_crouch_top_speed, 0.f, 1.f));

			float ratio = state->bias(move_weight, 0.18f) + 0.1f;

			state->move_yaw = math::normalize_yaw(state->move_yaw + (state->move_yaw_current_to_ideal * ratio));
		}
	}

	state->pose_param_mappings[PLAYER_POSE_PARAM_MOVE_YAW].set_value(player, state->move_yaw);

	float aim_yaw = math::angle_diff(state->eye_yaw, state->abs_yaw);
	if (aim_yaw >= 0 && state->aim_yaw_max != 0)
		aim_yaw = (aim_yaw / state->aim_yaw_max) * 60.0f;
	else if (state->aim_yaw_min != 0)
		aim_yaw = (aim_yaw / state->aim_yaw_min) * -60.0f;

	state->pose_param_mappings[PLAYER_POSE_PARAM_BODY_YAW].set_value(player, aim_yaw);

	// we need non-symmetrical arbitrary min/max bounds for vertical aim (pitch) too
	float pitch = math::angle_diff(state->eye_pitch, 0);
	if (pitch > 0)
		pitch = (pitch / state->aim_pitch_max) * 90.f;
	else
		pitch = (pitch / state->aim_pitch_min) * -90.f;

	state->pose_param_mappings[PLAYER_POSE_PARAM_BODY_PITCH].set_value(player, pitch);
	state->pose_param_mappings[PLAYER_POSE_PARAM_SPEED].set_value(player, state->speed_as_portion_of_walk_top_speed);
	state->pose_param_mappings[PLAYER_POSE_PARAM_STAND].set_value(player, 1.0f - (state->anim_duck_amount * state->in_air_smooth_value));
}

void c_threaded_animstate::setup_lean(c_animation_state* state, float curtime) {
	auto player = reinterpret_cast<c_cs_player*>(state->player);
	auto lean = &player->animlayers()[ANIMATION_LAYER_LEAN];
	lean->weight = lean->cycle = 0.f;
}

void c_threaded_animstate::setup_aim_matrix(c_animation_state* state, float curtime)
{

	auto player = reinterpret_cast<c_cs_player*>(state->player);
	auto weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(player->active_weapon()));

	if (state->anim_duck_amount <= 0 || state->anim_duck_amount >= 1) // only transition aim pose when fully ducked or fully standing
	{
		bool bPlayerIsWalking = (player && player->is_walking());
		bool bPlayerIsScoped = (player && player->is_scoped());

		float flTransitionSpeed = state->last_update_increment * (bPlayerIsScoped ? CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX_SCOPED : CSGO_ANIM_SPEED_TO_CHANGE_AIM_MATRIX);

		if (bPlayerIsScoped) // hacky: just tell all the transitions they've been invalid too long so all transitions clear as soon as the player starts scoping
		{
			state->m_tStandWalkAim.m_flDurationStateHasBeenInValid = state->m_tStandWalkAim.m_flHowLongToWaitUntilTransitionCanBlendOut;
			state->m_tStandRunAim.m_flDurationStateHasBeenInValid = state->m_tStandRunAim.m_flHowLongToWaitUntilTransitionCanBlendOut;
			state->m_tCrouchWalkAim.m_flDurationStateHasBeenInValid = state->m_tCrouchWalkAim.m_flHowLongToWaitUntilTransitionCanBlendOut;
		}

		state->m_tStandWalkAim.UpdateTransitionState(bPlayerIsWalking && !bPlayerIsScoped && state->speed_as_portion_of_walk_top_speed > 0.7f && state->speed_as_portion_of_run_top_speed < 0.7,
			state->last_update_increment, flTransitionSpeed);

		state->m_tStandRunAim.UpdateTransitionState(!bPlayerIsScoped && state->speed_as_portion_of_run_top_speed >= 0.7,
			state->last_update_increment, flTransitionSpeed);

		state->m_tCrouchWalkAim.UpdateTransitionState(!bPlayerIsScoped && state->speed_as_portion_of_crouch_top_speed >= 0.5,
			state->last_update_increment, flTransitionSpeed);
	}

	// Set aims to zero weight if they're underneath aims with 100% weight, for animation perf optimization.
	// Also set aims to full weight if their overlapping aims aren't enough to cover them, because cross-fades don't sum to 100% weight.

	float flStandIdleWeight = 1;
	float flStandWalkWeight = state->m_tStandWalkAim.m_flBlendValue;
	float flStandRunWeight = state->m_tStandRunAim.m_flBlendValue;
	float flCrouchIdleWeight = 1;
	float flCrouchWalkWeight = state->m_tCrouchWalkAim.m_flBlendValue;

	if (flStandWalkWeight >= 1)
		flStandIdleWeight = 0;

	if (flStandRunWeight >= 1)
	{
		flStandIdleWeight = 0;
		flStandWalkWeight = 0;
	}

	if (flCrouchWalkWeight >= 1)
		flCrouchIdleWeight = 0;

	if (state->anim_duck_amount >= 1)
	{
		flStandIdleWeight = 0;
		flStandWalkWeight = 0;
		flStandRunWeight = 0;
	}
	else if (state->anim_duck_amount <= 0)
	{
		flCrouchIdleWeight = 0;
		flCrouchWalkWeight = 0;
	}

	float flOneMinusDuckAmount = 1.0f - state->anim_duck_amount;

	flCrouchIdleWeight *= state->anim_duck_amount;
	flCrouchWalkWeight *= state->anim_duck_amount;
	flStandWalkWeight *= flOneMinusDuckAmount;
	flStandRunWeight *= flOneMinusDuckAmount;

	// make sure idle is present underneath cross-fades
	if (flCrouchIdleWeight < 1 && flCrouchWalkWeight < 1 && flStandWalkWeight < 1 && flStandRunWeight < 1)
		flStandIdleWeight = 1;

	state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_STAND_IDLE].set_value(player, flStandIdleWeight);
	state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_STAND_WALK].set_value(player, flStandWalkWeight);
	state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_STAND_RUN].set_value(player, flStandRunWeight);
	state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_CROUCH_IDLE].set_value(player, flCrouchIdleWeight);
	state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_CROUCH_WALK].set_value(player, flCrouchWalkWeight);

	static auto get_weapon_prefix = offsets::get_weapon_prefix.cast<const char* (__thiscall*)(void*)>();

	char szTransitionStandAimMatrix[64]{};
	if (get_weapon_prefix(state))
		sprintf_s(szTransitionStandAimMatrix, CXOR("%s_aim"), get_weapon_prefix(state));

	int nSeqStand = player->lookup_sequence(szTransitionStandAimMatrix);

	auto layer = &player->animlayers()[ANIMATION_LAYER_AIMMATRIX];
	{
		// use data-driven aim matrix limits
		if (layer && weapon)
		{
			auto aim_matrix_holder = player;
			int nSeq = nSeqStand;

			auto pWeaponWorldModel = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(weapon->weapon_world_model()));
			if (pWeaponWorldModel && layer->dispatch_sequence != ACT_INVALID)
			{
				aim_matrix_holder = pWeaponWorldModel;
				nSeq = layer->dispatch_sequence;
			}

			if (nSeq > 0)
			{
				auto hdr = aim_matrix_holder->get_studio_hdr();
				if (hdr)
				{
					float flYawIdleMin = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMIN_IDLE, CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MIN);
					float flYawIdleMax = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMAX_IDLE, CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MAX);
					float flYawWalkMin = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMIN_WALK, flYawIdleMin);
					float flYawWalkMax = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMAX_WALK, flYawIdleMax);
					float flYawRunMin = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_YAWMIN_RUN, flYawWalkMin);
					float flYawRunMax = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_YAWMAX_RUN, flYawWalkMax);
					float flYawCrouchIdleMin = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMIN_CROUCHIDLE, CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MIN);
					float flYawCrouchIdleMax = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMAX_CROUCHIDLE, CSGO_ANIM_AIMMATRIX_DEFAULT_YAW_MAX);
					float flYawCrouchWalkMin = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMIN_CROUCHWALK, flYawCrouchIdleMin);
					float flYawCrouchWalkMax = get_any_sequence_animtag(hdr, nSeq, ANIMTAG_AIMLIMIT_YAWMAX_CROUCHWALK, flYawCrouchIdleMax);

					float flWalkAmt = state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_STAND_WALK].get_value(player);
					float flRunAmt = state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_STAND_RUN].get_value(player);
					float flCrouchWalkAmt = state->pose_param_mappings[PLAYER_POSE_PARAM_AIM_BLEND_CROUCH_WALK].get_value(player);

					state->aim_yaw_min = math::lerp(state->anim_duck_amount,
						math::lerp(flRunAmt, math::lerp(flWalkAmt, flYawIdleMin, flYawWalkMin), flYawRunMin),
						math::lerp(flCrouchWalkAmt, flYawCrouchIdleMin, flYawCrouchWalkMin));
					state->aim_yaw_max = math::lerp(state->anim_duck_amount,
						math::lerp(flRunAmt, math::lerp(flWalkAmt, flYawIdleMax, flYawWalkMax), flYawRunMax),
						math::lerp(flCrouchWalkAmt, flYawCrouchIdleMax, flYawCrouchWalkMax));

					float flPitchIdleMin = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMIN_IDLE, CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN);
					float flPitchIdleMax = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMAX_IDLE, CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX);
					float flPitchWalkRunMin = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMIN_WALKRUN, flPitchIdleMin);
					float flPitchWalkRunMax = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMAX_WALKRUN, flPitchIdleMax);
					float flPitchCrouchMin = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMIN_CROUCH, CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN);
					float flPitchCrouchMax = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMAX_CROUCH, CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX);
					float flPitchCrouchWalkMin = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMIN_CROUCHWALK, flPitchCrouchMin);
					float flPitchCrouchWalkMax = get_any_sequence_animtag(hdr,nSeq, ANIMTAG_AIMLIMIT_PITCHMAX_CROUCHWALK, flPitchCrouchMax);

					state->aim_pitch_min = math::lerp(state->anim_duck_amount, math::lerp(flWalkAmt, flPitchIdleMin, flPitchWalkRunMin), math::lerp(flCrouchWalkAmt, flPitchCrouchMin, flPitchCrouchWalkMin));
					state->aim_pitch_max = math::lerp(state->anim_duck_amount, math::lerp(flWalkAmt, flPitchIdleMax, flPitchWalkRunMax), math::lerp(flCrouchWalkAmt, flPitchCrouchMax, flPitchCrouchWalkMax));
				}
			}
		}
	}

	state->update_layer(layer, nSeqStand, 0.f, 0.f, 1.f, ANIMATION_LAYER_AIMMATRIX);
}

void c_threaded_animstate::update(c_cs_player* player, c_animation_state* state, float yaw, float pitch, float curtime, int framecount) {
	state->player = player;

	if (!offsets::cache_sequences.cast<bool(__thiscall*)(c_animation_state*)>()(state))
		return;

	pitch = math::normalize_yaw(pitch + player->thirdperson_recoil());

	if (state->last_update_frame == framecount || state->last_update_time == curtime)
		return;

	state->last_update_increment = std::max<float>(0.f, curtime - state->last_update_time);

	state->eye_yaw = math::normalize_yaw(yaw);
	state->eye_pitch = math::normalize_yaw(pitch);
	state->position_current = player->origin();
	state->weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(player->active_weapon()));

	if (state->weapon != state->weapon_last || state->first_run_since_init) {
		for (int i = 0; i < 13; ++i) {
			auto layer = &player->animlayers()[i];
			if (layer) {
				layer->dispatch_sequence = -1;
				layer->second_dispatch_sequence = -1;
			}
		}
	}

	state->anim_duck_amount = std::clamp<float>(math::approach(std::clamp<float>(player->duck_amount() + state->duck_additional, 0, 1),
		state->anim_duck_amount, state->last_update_increment * 6.0f), 0, 1);

//	memory::get_virtual(HACKS->model_cache, XORN(MDL_CACHE_LOCK_VFUNC)).cast<void(__thiscall*)(void*)>()(HACKS->model_cache);
	{
		auto& new_seq = player->sequence();
		if (new_seq != 0)
		{
			new_seq = 0;
			//player->invalidate_physics_recursive(48);
		}

#ifdef LEGACY
		*reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(player) + XORN(0xA18)) = 0.f;
#else
		*reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(player) + XORN(0x286)) = 0.f;
#endif

#ifdef LEGACY
		auto& cycle = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(player) + XORN(0xA14));
#else
		auto& cycle = *reinterpret_cast<float*>(reinterpret_cast<std::uintptr_t>(player) + XORN(0xA134));
#endif
		if (cycle != 0.f) 
		{
			cycle = 0;
		//	player->invalidate_physics_recursive(8);
		}
	}
//	memory::get_virtual(HACKS->model_cache, XORN(MDL_CACHE_UNLOCK_VFUNC)).cast<void(__thiscall*)(void*)>()(HACKS->model_cache);

	{
		setup_velocity(state, curtime);
		//offsets::setup_aim_matrix.cast<ANIMSTATE_FUNC_FN>()(state);
		setup_aim_matrix(state, curtime);
		offsets::setup_weapon_action.cast<ANIMSTATE_FUNC_FN>()(state);
		offsets::setup_movement.cast<ANIMSTATE_FUNC_FN>()(state);

		auto alive_loop = &player->animlayers()[ANIMATION_LAYER_ALIVELOOP];
		state->increment_layer_cycle(alive_loop, true);

		auto whole_body = &player->animlayers()[ANIMATION_LAYER_FLASHED];
		if (whole_body->weight > 0)
		{
			state->increment_layer_cycle(whole_body, false);
			state->increment_layer_weight(whole_body);
		}

		auto flashed = &player->animlayers()[ANIMATION_LAYER_FLASHED];
		if (flashed->weight > 0) {
			if (flashed->weight_delta_rate < 0.f)
				state->increment_layer_weight(flashed);
		}

		auto flinch = &player->animlayers()[ANIMATION_LAYER_FLINCH];
		state->increment_layer_cycle(flinch, false);

		setup_lean(state, curtime);
	}

	for (int i = 0; i < 13; ++i) {
		auto layer = &player->animlayers()[i];
		if (layer->sequence == 0) {
			//if (layer->owner && layer->weight != 0.f) {
				//if (layer->weight == 0.0f)
				//	player->invalidate_physics_recursive(16);
			//}

			layer->weight = 0.f;
		}
	}

	player->set_abs_angles({ 0.f, state->abs_yaw, 0.f });

	state->weapon_last = state->weapon;
	state->position_last = state->position_current;
	state->first_run_since_init = false;
	state->last_update_frame = framecount;
	state->last_update_time = curtime;
}