#include "exploits.h"
#include "fake_lag.h"
#include "anti_aim.h"

#include "../config_vars.h"
#include "../features.h"
#include "../extra/movement.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

inline bool disable_custom_overrides()
{
	auto shifting = g_exploits->cl_move.trigger && g_exploits->cl_move.shifting;
	if (shifting)
		return true;

	if (g_anti_aim->is_fake_ducking())
		return true;

	if (g_exploits->enabled())
		return true;

	if (g_cfg.antihit.fakelag)
		return false;

	return true;
}

int c_fake_lag::get_max_choke()
{
	auto shifting = g_exploits->cl_move.trigger && g_exploits->cl_move.shifting;
	if (shifting)
		return 0;

	if (g_anti_aim->is_fake_ducking())
		return std::clamp(g_ctx.max_choke, 0, 14);

	if (g_exploits->enabled())
		return (int)g_cfg.antihit.desync;

	if (g_cfg.antihit.fakelag)
		return std::clamp(g_cfg.antihit.fakelag_limit, 0, g_ctx.max_choke);

	return (int)g_cfg.antihit.desync;
}

int c_fake_lag::get_choke_amount()
{
	if (disable_custom_overrides())
		return this->get_max_choke();

	auto state = g_ctx.local->animstate();

	int max_choke = this->get_max_choke();
	int temp_choke = (int)g_cfg.antihit.desync;

	auto velocity = g_ctx.local->velocity();
	bool standing = g_ctx.local->velocity().length(true) < 10.f;
	if ((g_cfg.antihit.fakelag_conditions & 1) && standing 
		|| (g_cfg.antihit.fakelag_conditions & 2) && !standing && g_utils->on_ground() 
		|| (g_cfg.antihit.fakelag_conditions & 4) && !g_utils->on_ground())
		temp_choke = max_choke;

	static int timer{};
	if (!(g_utils->on_ground()) && g_cfg.antihit.fluctiate_in_air)
	{
		if (std::abs(timer - interfaces::global_vars->tick_count) > max_choke + 1)
		{
			temp_choke = 1;
			timer = interfaces::global_vars->tick_count;
		}
		else
			temp_choke = max_choke;
	}

	return temp_choke;
}

void c_fake_lag::bypass_choke_limit()
{
	if (g_ctx.valve_ds)
		return;

	static int old_choke = 0;

	if (old_choke != g_ctx.max_choke)
	{
		auto address = patterns::send_move_addr.add(1).as< uint8_t* >();

		uint32_t choke_clamp = g_ctx.max_choke + 3;

		DWORD old_protect = 0;
		VirtualProtect((void*)address, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &old_protect);
		*(uint32_t*)address = choke_clamp;
		VirtualProtect((void*)address, sizeof(uint32_t), old_protect, &old_protect);

		old_choke = g_ctx.max_choke;
	}
}

void c_fake_lag::on_predict_start()
{
	this->bypass_choke_limit();

	if (interfaces::game_rules->is_freeze_time() || g_ctx.local->flags() & fl_frozen)
	{
		if (!*g_ctx.send_packet)
			*g_ctx.send_packet = true;
		return;
	}

	int choke_amount = this->get_choke_amount();

	auto shifting = g_exploits->cl_move.trigger && g_exploits->cl_move.shifting;
	auto shooting = (shifting || g_cfg.antihit.silent_onshot) ? g_ctx.shooting : g_utils->is_firing();

	if (g_exploits->recharge.start || choke_amount == 0 || !shifting && shooting && !g_anti_aim->is_fake_ducking())
	{
		if (!*g_ctx.send_packet)
			*g_ctx.send_packet = true;
		return;
	}

	*g_ctx.send_packet = false;

	int choke = interfaces::client_state->choked_commands;

	// too much choked commands
	if (choke >= choke_amount)
	{
		*g_ctx.send_packet = true;
		return;
	}
}

void c_ping_spike::on_procces_packet()
{
	if (!g_cfg.binds[spike_b].toggled || !g_ctx.local || !g_ctx.local->is_alive())
	{
		flipped_state = true;
		return;
	}

	auto netchan = interfaces::client_state->net_channel_ptr;
	if (!netchan)
		return;

	static auto last_reliable_state = -1;

	if (netchan->in_reliable_state != last_reliable_state)
		flipped_state = true;

	last_reliable_state = netchan->in_reliable_state;
}

void c_ping_spike::on_net_chan(c_netchan* netchan, float latency)
{
	if (flipped_state)
	{
		flipped_state = false;
		return;
	}

	int ticks = math::time_to_ticks(latency);
	if (netchan->in_sequence_nr > ticks)
		netchan->in_sequence_nr -= ticks;
}

void c_fake_lag::on_predict_end()
{
	auto& correct = choked_ticks.emplace_front();

	correct.cmd = g_ctx.cmd->command_number;
	correct.choke = interfaces::client_state->choked_commands + 1;
	correct.tickcount = interfaces::global_vars->tick_count;

	if (*g_ctx.send_packet)
		choked_commands.clear();
	else
		choked_commands.emplace_back(correct.cmd);

	while (choked_ticks.size() > (int)(2.f / interfaces::global_vars->interval_per_tick))
		choked_ticks.pop_back();

	//auto& out = commands.emplace_back();
	//out.outgoing = *g_ctx.send_packet;
	//out.used = false;
	//out.cmd = g_ctx.cmd->command_number;
	//out.prev_cmd = 0;
	//while (commands.size() > g_ctx.tick_rate)
	//	commands.pop_front();
}

void c_fake_lag::on_local_death()
{
	if (commands.size() > 0)
		commands.clear();

	if (choked_ticks.size() > 0)
		choked_ticks.clear();

	if (choked_commands.size() > 0)
		choked_commands.clear();
}