#include "globals.hpp"
#include "hud_hacks.hpp"

void c_hud_hacks::on_game_events(c_game_event* event)
{
	if (std::strcmp(event->get_name(), CXOR("round_start")))
		return;

	round_started = true;
}

void c_hud_hacks::preverse_killfeed()
{
	if (!HACKS->local || !HACKS->in_game)
		return;

#ifdef LEGACY
	static auto clear_notices = offsets::clear_killfeed.cast<void(__thiscall*)(kill_feed_t*)>();
	auto death_notice = (kill_feed_t*)offsets::find_hud_element.cast<DWORD(__thiscall*)(void*, const char*)>()
		(*offsets::get_hud_ptr.cast<std::uintptr_t** >(), CXOR("SFHudDeathNoticeAndBotStatus"));

	if (!death_notice)
		return;

	if (round_started || !g_cfg.misc.preverse_killfeed)
	{
		if (reset_killfeed)
		{
			clear_notices(death_notice);
			reset_killfeed = false;
		}

		if (round_started)
			round_started = false;

		return;
	}

	reset_killfeed = true;

	int size = death_notice->notices.count();
	if (!size)
		return;

	for (int i = 0; i < size; ++i)
	{
		auto notice = &death_notice->notices[i];

		if (notice->fade == 1.5f)
			notice->fade = FLT_MAX;
	}
#else
	static auto clear_notices = offsets::clear_killfeed.cast<void(__thiscall*)(std::uintptr_t)>();
	auto death_notice = (std::uintptr_t)offsets::find_hud_element.cast<DWORD(__thiscall*)(void*, const char*)>()
		(*offsets::get_hud_ptr.cast<std::uintptr_t**>(), CXOR("CCSGO_HudDeathNotice"));

	if (!death_notice)
		return;

	if (round_started || !g_cfg.misc.preverse_killfeed)
	{
		if (reset_killfeed)
		{
			clear_notices((std::uintptr_t)death_notice - 0x14);
			reset_killfeed = false;
		}

		if (round_started)
			round_started = false;

		return;
	}

	reset_killfeed = true;

	if (next_update > HACKS->global_vars->realtime)
		return;

	next_update = HACKS->global_vars->realtime + 2.f;

	const auto panel = (*(c_ui_panel**)(*(std::uintptr_t*)(death_notice - 20 + 88) + sizeof(std::uintptr_t)));
	const auto count = panel->get_child_count();

	for (int i = 0; i < count; ++i)
	{
		const auto child = panel->get_child(i);
		if (!child)
			continue;

		if (child->has_class(CXOR("DeathNotice_Killer")))
			child->set_attribute_float(CXOR("SpawnTime"), HACKS->global_vars->curtime);
	}
#endif
}