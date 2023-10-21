#include "event_logger.h"

#include "../../../base/tools/render.h"
#include "../../../base/global_context.h"
#include "../../../base/sdk.h"

#include "../../config_vars.h"
#include "../../features.h"

#pragma optimize( "", off )

std::pair< std::string, color > c_event_logger::get_message_prefix_type(int type)
{
	auto clr = g_cfg.misc.ui_color.base();
	switch (type)
	{
	case event_hit:
		return std::make_pair(xor_str("HIT"), clr);
		break;
	case event_miss:
		return std::make_pair(xor_str("MISS"), color(204, 60, 60));
		break;
	case event_plant:
		return std::make_pair(xor_str("BOMB"), color(235, 66, 66));
		break;
	case event_server:
		return std::make_pair(xor_str("SERVER"), clr.decrease(20));
		break;
	case event_buy:
		return std::make_pair(xor_str("BUY"), clr.decrease(15));
		break;
	}

	return {};
}

void c_event_logger::add_message(const std::string& text, int message_type, bool debug)
{
	if (!g_cfg.visuals.eventlog.enable)
		return;

	static auto string_prefix = xor_str("%s");
	static auto double_string_prefix = xor_str("%s %s");
	auto clr = g_cfg.misc.ui_color.base();

	interfaces::convar->print_console_color(clr, xor_c("[AIRFLOW] "));

	if (debug)
	{
		interfaces::convar->print_console_color(color(150, 150, 150), string_prefix.c_str(), text.c_str());
		interfaces::convar->print_console_color(color(0, 0, 0), " \n", text.c_str());
	}
	else
	{
		auto& new_message = messages.emplace_back();

		new_message.msgtype = message_type;
		new_message.time = g_ctx.system_time();
		new_message.alpha = 0.f;
		new_message.text = text;

		auto prefix = this->get_message_prefix_type(message_type);
		interfaces::convar->print_console_color(prefix.second, double_string_prefix.c_str(), prefix.first.c_str(), text.c_str());
		interfaces::convar->print_console_color(color(0, 0, 0), " \n", text.c_str());
	}
}

void c_event_logger::on_directx()
{
	if (!g_cfg.visuals.eventlog.enable)
	{
		if (!messages.empty())
			messages.clear();
		return;
	}

	if (messages.empty())
		return;

	g_render->enable_aa();

	// 32 is the height of single log
	auto maximum_logs_size = (int)((g_render->screen_size.h / 2) / 32);

	for (int i = messages.size() - 1; i >= 0; --i)
	{
		auto prefix = this->get_message_prefix_type(messages[i].msgtype);

		ImGui::PushFont(g_fonts.main);
		auto text_size = ImGui::CalcTextSize(messages[i].text.c_str());
		auto prefix_size = ImGui::CalcTextSize(prefix.first.c_str());
		ImGui::PopFont();

		auto log_size = vector2d(text_size.x + prefix_size.x + 39.f, 24);
		float out_time = std::clamp(g_ctx.system_time() - messages[i].time, 0.f, 5.f);

		bool time_expired = out_time >= 5.f;
		messages[i].alpha = std::clamp(std::lerp(messages[i].alpha, time_expired ? 0.f : 1.f, g_ctx.animation_speed), 0.f, 1.f);

		if (messages[i].alpha > 0.f)
		{
			float animated_offset = text_size.x * (1.f - messages[i].alpha);
			vector2d position = vector2d(10.f - animated_offset, 10.f + ((log_size.y + 8.f) * i));

			// log body
			g_render->filled_rect(position.x, position.y, log_size.x, log_size.y, color(25, 25, 25, 200 * messages[i].alpha), 4.f);
			g_render->rect(position.x + 1.f, position.y, log_size.x, log_size.y, color(255, 255, 255, 12.75f * messages[i].alpha), 4.f, 1.f);

			// left line
			g_render->filled_rect(position.x, position.y, 4.f, log_size.y, prefix.second.new_alpha(255 * messages[i].alpha), 4.f, ImDrawCornerFlags_Left);

			// log prefix
			g_render->string(position.x + 12.f, position.y + 4.f, color(255, 255, 255, messages[i].alpha * 255), 0, g_fonts.main, prefix.first.c_str());

			// log separator
			auto separator_pos = position.x + prefix_size.x + 19.f;
			g_render->rect(separator_pos, position.y, 2.f, log_size.y, color(255, 255, 255, 12.75f * messages[i].alpha), 4.f, 2.f);

			// main log
			g_render->string(separator_pos + 10.f, position.y + 4.f, color(255, 255, 255, 120.f * messages[i].alpha), 0, g_fonts.main, messages[i].text.c_str());
		}

		if (messages[i].alpha <= 0.1f && time_expired)
			messages.erase(messages.begin() + i);
	}

	g_render->disable_aa();
}

#pragma optimize( "", on )