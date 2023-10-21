#include "../features.h"
#include "cmd_shift.h"

namespace cmd_shift
{
	void shift_silent(c_usercmd* current_cmd, c_usercmd* first_cmd, int amount)
	{
		// alloc empty cmds
		std::vector<c_usercmd> fake_cmds{};
		fake_cmds.resize(amount);

		// create fake commands & simulate their movement
		for (int i = 0; i < amount; ++i)
		{
			auto cmd = &fake_cmds[i];
			if (cmd != first_cmd)
				std::memcpy(cmd, first_cmd, sizeof(c_usercmd));

			// disable in-game simulation for this cmd
			cmd->predicted = true;

			// don't add cmd to prediction & simulation record
			cmd->tickcount = INT_MAX;
		}

		// shift cmds
		auto net_chan = interfaces::client_state->net_channel_ptr;
		if (!net_chan)
			return;

		auto command_number = current_cmd->command_number + 1;
		auto add_command_number = current_cmd->command_number + 1;
		for (int i = 0; i < fake_cmds.size(); ++i)
		{
			auto fake_cmd = &fake_cmds[i];
			auto new_cmd = interfaces::input->get_user_cmd(command_number);

			if (new_cmd != fake_cmd)
				memcpy(new_cmd, fake_cmd, sizeof(c_usercmd));

			// don't add cmd to prediction & simulation record
			new_cmd->tickcount = INT_MAX;

			new_cmd->command_number = command_number;
			new_cmd->predicted = true;

			auto verified_cmd = interfaces::input->get_verified_user_cmd(command_number);
			auto verified_cmd_ptr = &verified_cmd->cmd;

			if (verified_cmd_ptr != new_cmd)
				memcpy(verified_cmd, new_cmd, sizeof(c_usercmd));

			verified_cmd->crc = new_cmd->get_check_sum();

			++interfaces::client_state->choked_commands;

			command_number = add_command_number + 1;
			++add_command_number;
		}

		fake_cmds.clear();
	}

	void shift_predicted(c_usercmd* current_cmd, c_usercmd* first_cmd, int amount)
	{
		// alloc empty cmds
		std::vector<c_usercmd> fake_cmds{};
		fake_cmds.resize(amount);

		auto old_tickbase = g_ctx.local->tickbase();
		{
			g_ctx.local->tickbase() -= amount;

			interfaces::move_helper->set_host(g_ctx.local);

			c_engine_prediction_restore restore{};

			*(c_usercmd**)((std::uintptr_t)g_ctx.local + xor_int(0x3348)) = current_cmd;
			*(c_usercmd*)((std::uintptr_t)g_ctx.local + xor_int(0x3298)) = *current_cmd;

			*g_engine_prediction->prediction_random_seed = MD5_PseudoRandom(current_cmd->command_number) & 0x7FFFFFFF;
			*g_engine_prediction->prediction_player = (int)g_ctx.local;

			*(bool*)((std::uintptr_t)interfaces::prediction + 0x8C) = true;
			*(int*)((std::uintptr_t)interfaces::prediction + 0x9C) = 0;

			interfaces::prediction->in_prediction = true;
			interfaces::prediction->is_first_time_predicted = false;

			current_cmd->buttons |= (g_ctx.local->button_forced());
			current_cmd->buttons &= ~(g_ctx.local->button_disabled());

			const int buttons = current_cmd->buttons;
			const int local_buttons = *g_ctx.local->buttons();
			const int buttons_changed = buttons ^ local_buttons;

			g_ctx.local->button_last() = local_buttons;
			*g_ctx.local->buttons() = buttons;
			g_ctx.local->button_pressed() = buttons_changed & buttons;
			g_ctx.local->button_released() = buttons_changed & (~buttons);

			g_ctx.local->post_think();

			++g_ctx.local->tickbase();

			interfaces::move_helper->set_host(nullptr);
		}

		// create fake commands & simulate their movement
		for (int i = 0; i < amount; ++i)
		{
			auto cmd = &fake_cmds[i];
			if (cmd != first_cmd)
				std::memcpy(cmd, first_cmd, sizeof(c_usercmd));

			// disable in-game simulation for this cmd
			cmd->predicted = true;

			cmd->buttons &= ~(in_attack | in_attack2);
			cmd->buttons |= (first_cmd->buttons & (in_bullrush | in_speed | in_duck));

			{
				c_engine_prediction_restore restore{};

				auto old_cmd = g_ctx.cmd;
				g_ctx.cmd = cmd;

				if (g_ctx.weapon->is_grenade())
					g_ctx.predicted_curtime = math::ticks_to_time(g_ctx.tick_base);
				else
					g_ctx.predicted_curtime = math::ticks_to_time(g_ctx.tick_base - g_exploits->tickbase_offset());

				*(c_usercmd**)((uintptr_t)g_ctx.local + 0x3348) = cmd;
				*(c_usercmd*)((uintptr_t)g_ctx.local + 0x3298) = *cmd;

				*g_engine_prediction->prediction_random_seed = MD5_PseudoRandom(cmd->command_number) & 0x7FFFFFFF;
				*g_engine_prediction->prediction_player = (int)g_ctx.local;

				*(bool*)((uintptr_t)interfaces::prediction + 0x8C) = true;
				*(int*)((uintptr_t)interfaces::prediction + 0x9C) = 0;

				interfaces::prediction->in_prediction = true;
				interfaces::prediction->is_first_time_predicted = false;

				interfaces::engine->get_view_angles(g_ctx.base_angle);

				g_ctx.jump_buttons = game_movement::get_jump_buttons();

				g_utils->on_pre_predict();
				g_movement->on_pre_predict();

				g_rage_bot->on_cm_start();

				g_animation_fix->update_valid_ticks();

				g_rage_bot->start_stop();
				g_movement->on_predict_start();

				g_movement->jitter_move();
				g_movement->auto_strafe();

				g_anti_aim->on_predict_start();
				g_anti_aim->on_predict_end();
				g_utils->on_predict_start();

				g_rage_bot->on_cm_end();

				interfaces::move_helper->set_host(g_ctx.local);
				interfaces::prediction->run_command(g_ctx.local, cmd, interfaces::move_helper);
				interfaces::move_helper->set_host(nullptr);

				g_ctx.cmd = old_cmd;
			}
		}

		// shift cmds
		auto net_chan = interfaces::client_state->net_channel_ptr;
		if (!net_chan)
			return;

		auto command_number = current_cmd->command_number + 1;
		auto add_command_number = current_cmd->command_number + 1;
		for (int i = 0; i < fake_cmds.size(); ++i)
		{
			shifting = true;

			auto fake_cmd = &fake_cmds[i];
			auto new_cmd = interfaces::input->get_user_cmd(command_number);

			if (new_cmd != fake_cmd)
				memcpy(new_cmd, fake_cmd, sizeof(c_usercmd));

			new_cmd->command_number = command_number;
			new_cmd->predicted = true;

			bool fix_pitch = true;
			if (!g_ctx.valve_ds && g_ctx.is_alive && g_cfg.antihit.distortion_pitch > 0)
				fix_pitch = false;

			new_cmd->viewangles = math::normalize(new_cmd->viewangles, fix_pitch);
			new_cmd->forwardmove = std::clamp(new_cmd->forwardmove, -450.f, 450.f);
			new_cmd->sidemove = std::clamp(new_cmd->sidemove, -450.f, 450.f);
			new_cmd->upmove = std::clamp(new_cmd->upmove, -320.f, 320.f);

			auto verified_cmd = interfaces::input->get_verified_user_cmd(command_number);
			auto verified_cmd_ptr = &verified_cmd->cmd;

			if (verified_cmd_ptr != new_cmd)
				memcpy(verified_cmd, new_cmd, sizeof(c_usercmd));

			verified_cmd->crc = new_cmd->get_check_sum();

			++net_chan->choked_packets;
			++net_chan->out_sequence_nr;
			++interfaces::client_state->choked_commands;

			command_number = add_command_number + 1;
			++add_command_number;
		}

		shifting = false;

		g_ctx.local->tickbase() = old_tickbase;

		interfaces::prediction->prev_ack_had_errors = true;
		interfaces::prediction->commands_predicted = 0;

		*g_ctx.send_packet = true;

		fake_cmds.clear();
	}
}