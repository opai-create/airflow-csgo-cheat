#include "globals.hpp"
#include "engine_prediction.hpp"
#include "movement.hpp"
#include "anti_aim.hpp"
#include "cmd_shift.hpp"
#include "fake_lag.hpp"
#include "animations.hpp"

// pasted from gamesense dump
// works better than CL_Move \ WriteUserCmd DT
// cuz you control all cmds that you shift by manual

namespace cmd_shift
{
	void shift_silent(c_user_cmd* current_cmd, c_user_cmd* first_cmd, int amount)
	{
		if (!*HACKS->send_packet)
			return;

		// alloc empty cmds
		std::vector<c_user_cmd> fake_cmds{};
		fake_cmds.resize(amount);

		// create fake commands & simulate their movement
		for (int i = 0; i < amount; ++i)
		{
			auto cmd = &fake_cmds[i];
			if (cmd != first_cmd)
				std::memcpy(cmd, first_cmd, sizeof(c_user_cmd));

			// disable in-game simulation for this cmd
			cmd->has_been_predicted = true;

			// don't add cmd to prediction & simulation record
			cmd->tickcount = INT_MAX;
		}

		// shift cmds
		auto net_chan = HACKS->client_state->net_channel;
		if (!net_chan)
			return;

		auto net_channel_info = HACKS->engine->get_net_channel();

		auto command_number = current_cmd->command_number + 1;
		auto add_command_number = current_cmd->command_number + 1;
		for (int i = 0; i < fake_cmds.size(); ++i)
		{
			auto fake_cmd = &fake_cmds[i];
			auto new_cmd = HACKS->input->get_user_cmd(command_number);

			if (new_cmd != fake_cmd)
				memcpy(new_cmd, fake_cmd, sizeof(c_user_cmd));

			// don't add cmd to prediction & simulation record
			new_cmd->tickcount = INT_MAX;

			new_cmd->command_number = command_number;
			new_cmd->has_been_predicted = true;

			auto verified_cmd = HACKS->input->get_verified_user_cmd(command_number);
			auto verfied_cmd_ptr = &verified_cmd->cmd;

			if (verfied_cmd_ptr != new_cmd)
				memcpy(verified_cmd, new_cmd, sizeof(c_user_cmd));

			verified_cmd->crc = new_cmd->get_checksum();

			++HACKS->client_state->choked_commands;

			command_number = add_command_number + 1;
			++add_command_number;
		}

		fake_cmds.clear();
	}

	void shift_predicted(c_user_cmd* current_cmd, c_user_cmd* first_cmd, int amount)
	{
		// alloc empty cmds
		std::vector<c_user_cmd> fake_cmds{};
		fake_cmds.resize(amount);

		RESTORE(HACKS->local->tickbase());
		{
			HACKS->local->tickbase() -= amount;

			HACKS->move_helper->set_host(HACKS->local);

			c_engine_prediction_restore restore{};

#ifdef LEGACY
			* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3314)) = current_cmd;
			*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x326C)) = *current_cmd;
#else
			* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3348)) = current_cmd;
			*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x3298)) = *current_cmd;
#endif

			* ENGINE_PREDICTION->prediction_random_seed = MD5_PseudoRandom(current_cmd->command_number) & 0x7FFFFFFF;
			*ENGINE_PREDICTION->prediction_player = (int)HACKS->local;

			*(bool*)((std::uintptr_t)HACKS->prediction + 0x8C) = true;
			*(int*)((std::uintptr_t)HACKS->prediction + 0x9C) = 0;

			HACKS->prediction->in_prediction = true;
			HACKS->prediction->is_first_time_predicted = false;

			current_cmd->buttons.force(HACKS->local->button_forced());

#ifndef LEGACY
			current_cmd->buttons.remove(HACKS->local->button_disabled());
#endif

			const int buttons = current_cmd->buttons.bits;
			const int local_buttons = *HACKS->local->buttons();
			const int buttons_changed = buttons ^ local_buttons;

			HACKS->local->button_last() = local_buttons;
			*HACKS->local->buttons() = buttons;
			HACKS->local->button_pressed() = buttons_changed & buttons;
			HACKS->local->button_released() = buttons_changed & (~buttons);

			HACKS->local->run_post_think();

			++HACKS->local->tickbase();

			HACKS->move_helper->set_host(nullptr);
		}

		// create fake commands & simulate their movement
		for (int i = 0; i < amount; ++i)
		{
			auto cmd = &fake_cmds[i];
			if (cmd != first_cmd)
				std::memcpy(cmd, first_cmd, sizeof(c_user_cmd));

			// disable in-game simulation for this cmd
			cmd->has_been_predicted = true;

			cmd->buttons.remove(IN_ATTACK | IN_ATTACK2);
			cmd->buttons.force(first_cmd->buttons.bits & (IN_BULLRUSH | IN_SPEED | IN_DUCK));

			{
				c_engine_prediction_restore restore{};

				auto old_cmd = HACKS->cmd;
				HACKS->cmd = cmd;

#ifdef LEGACY
				* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3314)) = cmd;
				*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x326C)) = *cmd;
#else
				* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3348)) = cmd;
				*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x3298)) = *cmd;
#endif

				*ENGINE_PREDICTION->prediction_random_seed = MD5_PseudoRandom(HACKS->cmd->command_number) & 0x7FFFFFFF;
				*ENGINE_PREDICTION->prediction_player = (int)HACKS->local;

				*(bool*)((std::uintptr_t)HACKS->prediction + 0x8C) = true;
				*(int*)((std::uintptr_t)HACKS->prediction + 0x9C) = 0;

				HACKS->prediction->in_prediction = true;
				HACKS->prediction->is_first_time_predicted = false;

				MOVEMENT->run();
				ANTI_AIM->run_movement();

				//	RAGEBOT->run_stop();

				HACKS->predicted_time = TICKS_TO_TIME(HACKS->tickbase);

				ANTI_AIM->run();
				FAKE_LAG->update_shot_cmd();
				MOVEMENT->run_predicted();

				ANTI_AIM->cleanup();
				MOVEMENT->rotate_movement(cmd, MOVEMENT->get_base_angle());

				HACKS->move_helper->set_host(HACKS->local);
				HACKS->prediction->run_command(HACKS->local, cmd, HACKS->move_helper);
				HACKS->move_helper->set_host(nullptr);

				--HACKS->local->tickbase();
				ANIMFIX->update_local();
				++HACKS->local->tickbase();

				HACKS->cmd = old_cmd;
			}
		}

		// shift cmds
		auto net_chan = HACKS->client_state->net_channel;
		if (!net_chan)
			return;

		auto net_channel_info = HACKS->engine->get_net_channel();

		auto command_number = current_cmd->command_number + 1;
		auto add_command_number = current_cmd->command_number + 1;
		for (int i = 0; i < fake_cmds.size(); ++i)
		{
			shifting = true;

			auto fake_cmd = &fake_cmds[i];
			auto new_cmd = HACKS->input->get_user_cmd(command_number);

			if (new_cmd != fake_cmd)
				memcpy(new_cmd, fake_cmd, sizeof(c_user_cmd));

			new_cmd->command_number = command_number;
			new_cmd->has_been_predicted = true;

			auto verified_cmd = HACKS->input->get_verified_user_cmd(command_number);
			auto verfied_cmd_ptr = &verified_cmd->cmd;

			if (verfied_cmd_ptr != new_cmd)
				memcpy(verified_cmd, new_cmd, sizeof(c_user_cmd));

			verified_cmd->crc = new_cmd->get_checksum();

			++net_chan->choked_packets;
			++net_chan->out_sequence_nr;
			++HACKS->client_state->choked_commands;

			command_number = add_command_number + 1;
			++add_command_number;
		}
		

		shifting = false;
		HACKS->prediction->prev_ack_had_errors = true;
		HACKS->prediction->commands_predicted = 0;

		*HACKS->send_packet = true;

		fake_cmds.clear();
	}
}