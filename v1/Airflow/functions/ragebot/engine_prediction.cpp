#include "engine_prediction.h"

#include "../features.h"

#include <thread>

void c_engine_prediction::net_compress_store(int tick)
{
	if (!g_ctx.local || !g_ctx.local->is_alive())
	{
		this->reset();
		return;
	}

	if (g_ctx.local->view_offset().z <= 46.05f)
		g_ctx.local->view_offset().z = 46.0f;
	else if (g_ctx.local->view_offset().z > 64.0f)
		g_ctx.local->view_offset().z = 64.0f;

	auto& data = net_data[tick % 150];
	data.cmd_number = tick;
	data.vel_modifier = g_ctx.local->velocity_modifier();
	data.fall_velocity = g_ctx.local->fall_velocity();
	data.duck_amt = g_ctx.local->duck_amount();
	data.duck_speed = g_ctx.local->duck_speed();
	data.thirdperson_recoil = g_ctx.local->thirdperson_recoil();
	data.punch = g_ctx.local->aim_punch_angle();
	data.punch_vel = g_ctx.local->aim_punch_angle_vel();
	data.view_offset = g_ctx.local->view_offset();
	data.view_punch = g_ctx.local->view_punch_angle();
	data.velocity = g_ctx.local->velocity();
	data.network_origin = g_ctx.local->network_origin();
	data.tickbase = g_ctx.local->tickbase();
	data.filled = true;

	reset_net_data = true;
}

void c_engine_prediction::net_compress_apply(int tick)
{
	g_ctx.pred_error_occured = false;

	if (!g_ctx.local || !g_ctx.local->is_alive())
	{
		this->reset();
		return;
	}

	auto& data = net_data[tick % 150];
	if (data.cmd_number != tick || !data.filled)
		return;

	auto tickbase_diff = data.tickbase - g_ctx.local->tickbase();
	auto aim_punch_vel_diff = data.punch_vel - g_ctx.local->aim_punch_angle_vel();
	auto aim_punch_diff = data.punch - g_ctx.local->aim_punch_angle();
	auto viewoffset_diff = data.view_offset - g_ctx.local->view_offset();
	auto velocity_diff = data.velocity - g_ctx.local->velocity();
	auto fall_velocity_diff = data.fall_velocity - g_ctx.local->fall_velocity();
	auto network_origin_diff = data.network_origin - g_ctx.local->network_origin();

	if (std::abs(aim_punch_diff.x) <= 0.03125f && std::abs(aim_punch_diff.y) <= 0.03125f && std::abs(aim_punch_diff.z) <= 0.03125f)
		g_ctx.local->aim_punch_angle() = data.punch;
	else
		g_ctx.pred_error_occured = true;

	if (std::abs(aim_punch_vel_diff.x) <= 0.03125f && std::abs(aim_punch_vel_diff.y) <= 0.03125f && std::abs(aim_punch_vel_diff.z) <= 0.03125f)
		g_ctx.local->aim_punch_angle_vel() = data.punch_vel;
	else
		g_ctx.pred_error_occured = true;

	if (std::abs(viewoffset_diff.z) <= 0.065f)
		g_ctx.local->view_offset() = data.view_offset;
	else
		g_ctx.pred_error_occured = true;

	if (std::abs(fall_velocity_diff) <= 0.5f)
		g_ctx.local->fall_velocity() = data.fall_velocity;
	else
		g_ctx.pred_error_occured = true;

	if (std::abs(velocity_diff.x) > 0.5f || std::abs(velocity_diff.y) > 0.5f || std::abs(velocity_diff.z) > 0.5f)
		g_ctx.pred_error_occured = true;

	if (std::abs(network_origin_diff.x) > 0.0625f || std::abs(network_origin_diff.y) > 0.0625f || std::abs(network_origin_diff.z) > 0.0625f)
		g_ctx.pred_error_occured = true;

	//if (std::abs(tickbase_diff) > 0)
	//{
	//	interfaces::prediction->prev_ack_had_errors = true;
	//	interfaces::prediction->commands_predicted = 0;

	//	//g_ctx.pred_error_occured = true;
	//}

	reset_net_data = true;
}

void c_engine_prediction::init()
{
	prediction_random_seed = *patterns::prediction_random_seed.as< int** >();
	prediction_player = *patterns::prediction_player.as< int** >();
}

void c_engine_prediction::update()
{
	interfaces::prediction->update(
		interfaces::client_state->delta_tick, interfaces::client_state->delta_tick > 0, interfaces::client_state->last_command_ack, interfaces::client_state->last_outgoing_command + interfaces::client_state->choked_commands);
}

void c_engine_prediction::start(c_csplayer* local, c_usercmd* cmd)
{
	if (!local)
		return;

	auto weapon = local->get_active_weapon();
	if (!weapon)
		return;

	g_ctx.in_prediction = true;

	unpred_vars[cmd->command_number % 150].fill();

	unprediced_velocity = local->velocity();
	unpredicted_flags = local->flags();

	old_cur_time = interfaces::global_vars->cur_time;
	old_frame_time = interfaces::global_vars->frame_time;
	old_tick_count = interfaces::global_vars->tick_count;

	old_in_prediction = interfaces::prediction->in_prediction;
	old_first_time_predicted = interfaces::prediction->is_first_time_predicted;

	//old_seed = *prediction_random_seed;

	old_recoil_index = weapon->recoil_index();
	old_accuracy_penalty = weapon->accuracy_penalty();

	predicted_inaccuracy = weapon->get_inaccuracy();
	predicted_spread = weapon->get_spread();

	float cone = g_ctx.weapon->get_inaccuracy() + g_ctx.weapon->get_spread();
	cone *= g_render->screen_size.h * 0.7f;

	g_ctx.spread = cone;

	float zoom_substract = 0.f;
	if ((g_ctx.weapon->item_definition_index() == weapon_ssg08 || g_ctx.weapon->item_definition_index() == weapon_awp) && !g_ctx.local->is_scoped())
		zoom_substract = 25.f;
	else
		zoom_substract = 24.f;

	g_ctx.ideal_spread = std::clamp(26.f - zoom_substract, 0.f, 26.f);

	interfaces::global_vars->cur_time = math::ticks_to_time(g_ctx.tick_base);
	interfaces::global_vars->frame_time = interfaces::global_vars->interval_per_tick;

	*(c_usercmd**)((uintptr_t)local + 0x3348) = cmd;
	*(c_usercmd*)((uintptr_t)local + 0x3298) = *cmd;

	*prediction_random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;
	*prediction_player = (int)local;

	*(bool*)((uintptr_t)interfaces::prediction + 0x8C) = true;
	*(int*)((uintptr_t)interfaces::prediction + 0x9C) = 0;

	interfaces::prediction->in_prediction = true;
	interfaces::prediction->is_first_time_predicted = false;

	if (!local)
		return;

	auto state = local->animstate();
	if (!state)
		return;

	cmd->buttons |= local->button_forced();
	cmd->buttons &= ~(local->button_disabled());

	interfaces::game_movement->start_track_prediction_errors(local);

	const int buttons = cmd->buttons;
	const int local_buttons = *local->buttons();
	const int buttons_changed = buttons ^ local_buttons;

	local->button_last() = local_buttons;
	*local->buttons() = buttons;
	local->button_pressed() = buttons_changed & buttons;
	local->button_released() = buttons_changed & (~buttons);

	memset(&move_data, 0, sizeof(c_movedata));

	//move_data.buttons = cmd->buttons;
	//move_data.old_buttons = cmd->buttons;

	auto old_tickbase = g_ctx.local->tickbase();

	interfaces::prediction->check_moving_ground(local, interfaces::global_vars->frame_time);
	interfaces::prediction->set_local_view_angles(cmd->viewangles);

	local->run_pre_think();
	local->run_think();

	interfaces::move_helper->set_host(local);
	interfaces::prediction->setup_move(local, cmd, interfaces::move_helper, &move_data);
	interfaces::game_movement->process_movement(local, &move_data);
	interfaces::prediction->finish_move(local, cmd, &move_data);

	interfaces::move_helper->process_impacts();

	local->post_think();
	local->tickbase() = old_tickbase;

	++g_ctx.sim_tick_base;

	interfaces::game_movement->finish_track_prediction_errors(local);
	interfaces::move_helper->set_host(nullptr);

	if (weapon)
		weapon->update_accuracy_penalty();

	predicted_vars[cmd->command_number % 150].fill();
}

__forceinline void modify_eye_pos(vector3d& pos, matrix3x4_t* matrix)
{
	auto state = g_ctx.local->animstate();
	if (!state)
		return;

	if (state->landing || state->anim_duck_amount != 0.f || g_ctx.local->ground_entity() == 0) 
	{
		auto bone_pos = matrix[8].get_origin();
		const auto bone_z = bone_pos.z + 1.7f;
		if (pos.z > bone_z) 
		{
			const auto view_modifier = std::clamp((fabsf(pos.z - bone_z) - 4.f) * 0.16666667f, 0.f, 1.f);
			const auto view_modifier_sqr = view_modifier * view_modifier;
			pos.z += (bone_z - pos.z) * (3.f * view_modifier_sqr - 2.f * view_modifier_sqr * view_modifier);
		}
	}
}

__forceinline float Studio_SetPoseParameter(c_studiohdr* hdr, int index, float flValue, float& ctlValue)
{
	if (index < 0 || index > 24)
		return 0.f;

	auto ptr = func_ptrs::get_pose_parameter(hdr, index);
	if (!ptr)
		return 0.f;

	auto PoseParam = *ptr;

	if (PoseParam.loop)
	{
		float wrap = (PoseParam.start + PoseParam.end) / 2.0 + PoseParam.loop / 2.0;
		float shift = PoseParam.loop - wrap;

		flValue = flValue - PoseParam.loop * floor((flValue + shift) / PoseParam.loop);
	}

	ctlValue = (flValue - PoseParam.start) / (PoseParam.end - PoseParam.start);

	if (ctlValue < 0) ctlValue = 0;
	if (ctlValue > 1) ctlValue = 1;

	return ctlValue * (PoseParam.end - PoseParam.start) + PoseParam.start;
}

vector3d c_engine_prediction::get_eye_pos(const vector3d& angle)
{
	auto state = g_ctx.local->animstate();
	if (!state)
		return {};

	auto hdr = g_ctx.local->get_studio_hdr();
	if (!hdr)
		return {};

	auto eye_pos = g_ctx.local->origin() + g_ctx.local->view_offset();
	auto old_poses = g_ctx.local->pose_parameter()[12];
	
	float new_angle = angle.x;

	if (new_angle > 180.f)
		new_angle = new_angle - 360.f;

	new_angle = std::clamp(new_angle, -90.f, 90.f);
	
	g_ctx.local->set_abs_angles({ 0.f, state->abs_yaw, 0.f });

	float pose = 0.f;;
	new_angle = Studio_SetPoseParameter(hdr, 12, new_angle, pose);
	g_ctx.local->pose_parameter()[12] = pose;

	g_ctx.local->setup_uninterpolated_bones(nullptr);

	modify_eye_pos(eye_pos, g_ctx.local->bone_cache().base());

	g_ctx.local->pose_parameter()[12] = old_poses;

	return eye_pos;
}
 
void c_engine_prediction::repredict(c_csplayer* local, c_usercmd* cmd, bool real_cmd)
{
	if (!local)
		return;

	auto state = local->animstate();
	if (!state)
		return;

	auto weapon = local->get_active_weapon();
	if (!weapon)
		return;

	//unpred_vars[cmd->command_number % 150].set();

	*(c_usercmd**)((uintptr_t)local + 0x3348) = cmd;
	*(c_usercmd*)((uintptr_t)local + 0x3298) = *cmd;

	move_data.forwardmove = cmd->forwardmove;
	move_data.sidemove = cmd->sidemove;
	move_data.upmove = cmd->upmove;
	move_data.buttons = cmd->buttons;
	move_data.old_buttons = cmd->buttons;
	move_data.view_angles = cmd->viewangles;
	move_data.angles = cmd->viewangles;

	interfaces::game_movement->start_track_prediction_errors(local);
	interfaces::move_helper->set_host(local);
	interfaces::prediction->setup_move(local, cmd, interfaces::move_helper, &move_data);
	interfaces::game_movement->process_movement(local, &move_data);
	interfaces::prediction->finish_move(local, cmd, &move_data);
	interfaces::game_movement->finish_track_prediction_errors(local);
	interfaces::move_helper->set_host(nullptr);

	if (weapon)
		weapon->update_accuracy_penalty();

	predicted_vars[cmd->command_number % 150].fill();

	//g_ctx.eye_position = this->get_eye_pos(g_ctx.orig_angle);
}

void c_engine_prediction::finish(c_csplayer* local)
{
	g_ctx.in_prediction = false;

	if (!local)
		return;

	auto weapon = local->get_active_weapon();
	if (!weapon)
		return;

	*(c_usercmd**)((uintptr_t)local + 0x3348) = 0;
	*prediction_random_seed = -1;
	*prediction_player = 0;

	interfaces::global_vars->cur_time = old_cur_time;
	interfaces::global_vars->frame_time = old_frame_time;

	if (weapon && !weapon->is_misc_weapon())
	{
		weapon->recoil_index() = old_recoil_index;
		weapon->accuracy_penalty() = old_accuracy_penalty;
	}

	interfaces::game_movement->reset();

	interfaces::prediction->in_prediction = old_in_prediction;
	interfaces::prediction->is_first_time_predicted = old_first_time_predicted;
}