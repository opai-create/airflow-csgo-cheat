#include "globals.hpp"
#include "clantag.hpp"

const char8_t* icons[]
{
	u8"\u2621",
	u8"\u2620",
	u8"\u2623",
	u8"\u262F",
	u8"\u267B",
	u8"\u26A1",
	u8"\u26A3",
};

void c_clantag::run()
{
	if (!HACKS->local)
		return;

	auto predicted_curtime = TICKS_TO_TIME(HACKS->local->tickbase());
	auto clantag_time = (int)((HACKS->global_vars->curtime * 2.f) + HACKS->ping);

	if (g_cfg.misc.clantag)
	{
		reset_tag = false;

		if (clantag_time != last_change_time)
		{
			if (next_update_time <= predicted_curtime || next_update_time - predicted_curtime > 1.f)
				set_clan_tag(tag_desc.c_str(), tag_desc.c_str());

			last_change_time = clantag_time;
		}
	}
	else
	{
		if (!reset_tag)
		{
			if (clantag_time != last_change_time)
			{
				set_clan_tag("", "");

				reset_tag = true;
				last_change_time = clantag_time;
			}
		}
	}
}