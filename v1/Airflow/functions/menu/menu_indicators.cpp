#include "menu.h"

#include "../config_system.h"
#include "../config_vars.h"

#include "../../base/sdk.h"
#include "../../base/sdk/entity.h"
#include "../../base/global_context.h"
#include "../../base/tools/math.h"
#include "../../base/tools/cheat_info.h"
#include "../../base/other/byte_arrays.h"

#include "../features.h"
#include "../ragebot/autowall.h"

#include "../../additionals/tinyformat.h"

#include <ShlObj.h>
#include <algorithm>
#include <map>
#include <format>

constexpr auto misc_ui_flags = ImGuiWindowFlags_NoSavedSettings
| ImGuiWindowFlags_NoResize
| ImGuiWindowFlags_AlwaysAutoResize
| ImGuiWindowFlags_NoCollapse
| ImGuiWindowFlags_NoTitleBar
| ImGuiWindowFlags_NoScrollbar
| ImGuiWindowFlags_NoFocusOnAppearing;

std::string get_bind_type(int type)
{
	switch (type)
	{
	case 0:
		return xor_c("[ enabled ]");
		break;
	case 1:
		return xor_c("[ hold ]");
		break;
	case 2:
		return xor_c("[ toggled ]");
		break;
	}
	return "";
}

void c_menu::draw_binds()
{
	if (!(g_cfg.misc.menu_indicators & 1))
		return;

	static auto opened = true;
	static float alpha = 0.f;

	static auto window_size = ImVec2(184, 40.f);

	static std::map<std::string, menu_key_binds_t> updated_keybinds{};
	static key_binds_t prev_keys[binds_max]{};

	for (int i = 0; i < binds_max; ++i)
	{
		if (i == tp_b)
			continue;

		auto& prev_bind = prev_keys[i];
		auto& current_bind = g_cfg.binds[i];
		auto& binds = updated_keybinds[current_bind.name];
		if (prev_bind.toggled != current_bind.toggled)
		{
			if (current_bind.toggled)
			{
				binds.name = current_bind.name;
				binds.type = current_bind.type;
				binds.time = g_ctx.system_time();

				window_size.y += 25.f;
			}
			else
			{
				binds.reset(g_ctx.system_time());
				window_size.y -= 25.f;
			}

			prev_bind = current_bind;
		}
	}

	this->create_animation(alpha, g_cfg.misc.menu || g_ctx.in_game && window_size.y > 40.f, 1.f, lerp_animation);

	static bool set_position = false;
	if (g_ctx.loading_config)
		set_position = true;

	if (g_cfg.misc.keybind_position.x == 0 && g_cfg.misc.keybind_position.y == 0)
	{
		g_cfg.misc.keybind_position.x = 10;
		g_cfg.misc.keybind_position.y = g_render->screen_size.h / 2 - window_size.y / 2;

		set_position = true;
	}

	if (alpha <= 0.f)
		return;

	if (set_position)
	{
		ImGui::SetNextWindowPos(g_cfg.misc.keybind_position);
		set_position = false;
	}

	ImGui::SetNextWindowSize(window_size + ImVec2(0.f, 2.f));

	ImGui::PushFont(g_fonts.main);
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(xor_c("##bind_window"), &opened, misc_ui_flags);

	auto list = ImGui::GetWindowDrawList();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	// window body
	{
		auto keybinds_size = ImVec2(176.f, 32.f);
		auto window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);
		auto window_alpha = 255.f * alpha;

		// header
		imgui_blur::create_blur(list, window_pos, window_pos + ImVec2(keybinds_size.x, 32.f), color(255, 255, 255, window_alpha).as_imcolor(), 4.f, ImDrawCornerFlags_Top);

		list->AddImage((void*)keyboard_texture, ImVec2(window_pos.x + 51, window_pos.y + 9), ImVec2(window_pos.x + 67, window_pos.y + 25), ImVec2(0, 0), ImVec2(1, 1), color(255, 255, 255, window_alpha).as_imcolor());

		list->AddText(ImVec2(window_pos.x + 75, window_pos.y + 8), color(255, 255, 255, window_alpha).as_imcolor(), xor_c("keybinds"));

		list->AddLine(window_pos + ImVec2(0, 31.f), window_pos + ImVec2(keybinds_size.x, 31), color(255, 255, 255, 12.75f * alpha).as_imcolor());

		// body
		imgui_blur::create_blur(list, window_pos + ImVec2(0, 32.f), window_pos + ImVec2(keybinds_size.x, 32.f + window_size.y), color(100, 100, 100, (int)(window_alpha)).as_imcolor(), 4.f, ImDrawCornerFlags_Bot);

		// border
		list->AddRect(window_pos, ImVec2(window_pos.x + keybinds_size.x, window_pos.y + window_size.y), color(100, 100, 100, 100.f * alpha).as_imcolor(), 4.f);
	}

	// bind text
	auto prev_pos = ImGui::GetCursorPos();

	auto max_pos = 0.f;
	for (auto& keybind : updated_keybinds)
	{
		float time_difference = g_ctx.system_time() - keybind.second.time;
		float animation = std::clamp(time_difference / 0.2f, 0.f, 1.f);

		if (keybind.second.type == -1)
			animation = 1.f - animation;

		if (animation <= 0.f)
			continue;

		ImGui::SetCursorPos(ImVec2(16.f, 40.f + max_pos));

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, alpha * animation));
		ImGui::Text(keybind.second.name.c_str());
		ImGui::PopStyleColor();

		auto bind_type = get_bind_type(keybind.second.type);
		auto textsize = ImGui::CalcTextSize(bind_type.c_str()).x;

		ImGui::SameLine(ImGui::GetWindowSize().x - textsize - 15.f);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.4f * alpha * animation));
		ImGui::Text(bind_type.c_str());
		ImGui::PopStyleColor();

		this->create_animation(keybind.second.alpha, keybind.second.type != -1, 0.6f, lerp_animation);
		max_pos += 25.f * keybind.second.alpha;
	}

	ImGui::SetCursorPos(prev_pos);

	if (!set_position)
		g_cfg.misc.keybind_position = ImGui::GetWindowPos();

	list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

	ImGui::End(false);
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void c_menu::draw_spectators()
{
	const std::unique_lock<std::mutex> lock(mutexes::spectators);
	if (!(g_cfg.misc.menu_indicators & 8))
		return;

	static auto opened = true;
	static float alpha = 0.f;

	static auto window_size = ImVec2(184, 40.f);

	static spectator_animation_t spectator_animaiton[50]{};

	auto sanitize = [&](const char* name)
	{
		std::string tmp(name);

		for (int i = 0; i < (int)tmp.length(); i++)
		{
			if ((
				tmp[i] >= 'a' && tmp[i] <= 'z' ||
				tmp[i] >= 'A' && tmp[i] <= 'Z' ||
				tmp[i] >= '0' && tmp[i] <= '9' ||
				tmp[i] == ' ' || tmp[i] == '.' || tmp[i] == '/' || tmp[i] == ':' ||
				tmp[i] == ',' || tmp[i] == '_' || tmp[i] == '#' || tmp[i] == '$' ||
				tmp[i] == '<' || tmp[i] == '>' || tmp[i] == '-' || tmp[i] == '+' ||
				tmp[i] == '*' || tmp[i] == '%' || tmp[i] == '@' || tmp[i] == '(' ||
				tmp[i] == ')' || tmp[i] == '{' || tmp[i] == '}' || tmp[i] == '[' || tmp[i] == ']' ||
				tmp[i] == '!' || tmp[i] == '&' || tmp[i] == '~' || tmp[i] == '^'
				) == false)
			{
				tmp[i] = '_';
			}
		}

		if (tmp.length() > 20)
		{
			tmp.erase(20, (tmp.length() - 20));
			tmp.append("...");
		}

		return tmp;
	};

	for (int i = 0; i < 50; ++i)
	{
		auto& spectator = spectators[i];
		auto& animation = spectator_animaiton[i];

		if (animation.name.empty())
			animation.name = spectator.name;

		if (animation.name != spectator.name)
		{
			if (animation.was_spectating && window_size.y > 40.f)
				window_size.y -= 25.f;

			animation.reset();
			continue;
		}

		if (spectator.spectated)
		{
			if (!animation.was_spectating)
			{
				window_size.y += 25.f;

				animation.start_time = g_ctx.system_time();
				animation.was_spectating = true;
			}
		}
		else
		{
			if (animation.was_spectating)
			{
				window_size.y -= 25.f;

				animation.start_time = g_ctx.system_time();
				animation.was_spectating = false;
			}
		}
	}

	this->create_animation(alpha, g_cfg.misc.menu || g_ctx.in_game && window_size.y > 40.f, 1.f, lerp_animation);

	static bool set_position = false;
	if (g_ctx.loading_config)
		set_position = true;

	if (g_cfg.misc.spectators_position.x == 0 && g_cfg.misc.spectators_position.y == 0)
	{
		g_cfg.misc.spectators_position.x = 10;
		g_cfg.misc.spectators_position.y = (g_render->screen_size.h / 2) + window_size.y + 10.f;

		set_position = true;
	}

	if (alpha <= 0.f)
		return;

	if (set_position)
	{
		ImGui::SetNextWindowPos(g_cfg.misc.spectators_position);
		set_position = false;
	}

	ImGui::SetNextWindowSize(window_size + ImVec2(0.f, 2.f));

	ImGui::PushFont(g_fonts.main);
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(xor_c("##spec_window"), &opened, misc_ui_flags);

	auto list = ImGui::GetWindowDrawList();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	// window body
	{
		auto keybinds_size = ImVec2(176.f, 32.f);
		auto window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);
		auto window_alpha = 255.f * alpha;

		// header
		imgui_blur::create_blur(list, window_pos, window_pos + ImVec2(keybinds_size.x, 32.f), color(255, 255, 255, window_alpha).as_imcolor(), 4.f, ImDrawCornerFlags_Top);

		list->AddImage((void*)spectator_texture,
			ImVec2(window_pos.x + 41, window_pos.y + 9),
			ImVec2(window_pos.x + 57, window_pos.y + 25),
			ImVec2(0, 0), ImVec2(1, 1), 
			color(255, 255, 255, window_alpha).as_imcolor());

		list->AddText(
			ImVec2(window_pos.x + 65, window_pos.y + 8),
			color(255, 255, 255, window_alpha).as_imcolor(), 
			xor_c("spectators"));

		list->AddLine(
			window_pos + ImVec2(0, 31.f),
			window_pos + ImVec2(keybinds_size.x, 31), 
			color(255, 255, 255, 12.75f * alpha).as_imcolor());

		// body
		imgui_blur::create_blur(list, 
			window_pos + ImVec2(0, 32.f), 
			window_pos + ImVec2(keybinds_size.x, 32.f + window_size.y), 
			color(100, 100, 100, (int)(window_alpha)).as_imcolor(), 4.f, ImDrawCornerFlags_Bot);

		// border
		list->AddRect(window_pos, 
			ImVec2(window_pos.x + keybinds_size.x, window_pos.y + window_size.y),
			color(100, 100, 100, 100.f * alpha).as_imcolor(),
			4.f);
	}

	// bind text
	auto prev_pos = ImGui::GetCursorPos();

	auto max_pos = 0.f;
	for (int i = 0; i < 50; ++i)
	{
		auto& spectator = spectators[i];
		auto& animation = spectator_animaiton[i];

		float time_difference = g_ctx.system_time() - animation.start_time;
		float anim_progress = std::clamp(time_difference / 0.2f, 0.f, 1.f);

		if (!spectator.spectated && !animation.was_spectating)
			anim_progress = 1.f - anim_progress;

		if (anim_progress <= 0.f)
			continue;

		ImGui::SetCursorPos(ImVec2(16.f, 40.f + max_pos));

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, alpha * anim_progress));
		ImGui::Text(spectator.name.c_str());
		ImGui::PopStyleColor();

		auto chase_type = spectator.chase_mode;
		auto text_size = ImGui::CalcTextSize(chase_type.c_str()).x;

		ImGui::SameLine(ImGui::GetWindowSize().x - text_size - 15.f);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.4f * alpha * anim_progress));
		ImGui::Text(chase_type.c_str());
		ImGui::PopStyleColor();

		this->create_animation(animation.anim_step, spectator.spectated && animation.was_spectating, 0.6f, lerp_animation);
		max_pos += 25.f * animation.anim_step;
	}

	ImGui::SetCursorPos(prev_pos);

	if (!set_position)
		g_cfg.misc.spectators_position = ImGui::GetWindowPos();

	list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

	ImGui::End(false);
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

float scale_damage_armor(float flDamage, int armor_value)
{
	float flArmorRatio = 0.5f;
	float flArmorBonus = 0.5f;
	if (armor_value > 0)
	{
		float flNew = flDamage * flArmorRatio;
		float flArmor = (flDamage - flNew) * flArmorBonus;

		if (flArmor > static_cast<float>(armor_value))
		{
			flArmor = static_cast<float>(armor_value) * (1.f / flArmorBonus);
			flNew = flDamage - flArmor;
		}

		flDamage = flNew;
	}
	return flDamage;
}

void c_menu::store_bomb()
{
	const std::unique_lock< std::mutex > lock(mutexes::bomb);

	if (!(g_cfg.misc.menu_indicators & 2))
	{
		bomb.reset();
		return;
	}

	auto& weapon_array = g_listener_entity->get_entity(ent_c4);
	if (weapon_array.empty())
	{
		bomb.reset();
		return;
	}

	bomb.reset();
	for (auto& weapon_info : weapon_array)
	{
		auto entity = weapon_info.entity;
		if (!entity)
			continue;

		float blow_time = (entity->c4_blow() - interfaces::global_vars->cur_time);
		if (blow_time <= 0.f)
			continue;

		const auto damagePercentage = 1.0f;

		auto damage = 500.f; // 500 - default, if radius is not written on the map https://i.imgur.com/mUSaTHj.png
		auto bomb_radius = damage * 3.5f;
		auto distance_to_local = (entity->view_offset() + entity->origin() - g_ctx.eye_position).length(false);
		auto sigma = bomb_radius / 3.0f;
		auto fGaussianFalloff = exp(-distance_to_local * distance_to_local / (2.0f * sigma * sigma));
		auto adjusted_damage = damage * fGaussianFalloff * damagePercentage;

		bomb.time = (int)blow_time;
		bomb.health = scale_damage_armor(adjusted_damage, g_ctx.local->armor_value());
		bomb.bomb_site = entity->bomb_site() == 0 ? xor_c("A") : xor_c("B");
		bomb.filled = true;
	}
}

void c_menu::store_spectators()
{
	const std::unique_lock<std::mutex> lock(mutexes::spectators);
	if (!(g_cfg.misc.menu_indicators & 8) || !g_ctx.in_game || !g_ctx.local)
	{
		for (auto& i : spectators)
			i.reset();

		return;
	}
	
	auto& players = g_listener_entity->get_entity(ent_player);
	if (players.empty())
		return;

	auto iterator = 0;
	for (auto& i : players)
	{
		auto player = (c_csplayer*)i.entity;

		auto& spectator = spectators[iterator++];
		spectator.ptr	= player;

		if (player->is_alive() || player->dormant() || player->gun_game_immunity())
		{
			spectator.spectated = false;
			continue;
		}

		auto spec_target = player->get_observer_target();
		if (!spec_target || player->observer_mode() == obs_mode_none)
		{
			spectator.spectated = false;
			continue;
		}

		auto spectated_name = [&]() -> std::string
		{
			auto name = spec_target->get_name();

			if (name.size() > 8) {
				name.erase(8, name.length() - 8);
				name.append(xor_c("..."));
			}

			return name;
		};

		spectator.chase_mode	= tfm::format(xor_c("[ %s ]"), spectated_name());
		spectator.spectated		= true;
		spectator.name			= player->get_name();

		if (spectator.name.size() > 10) {
			spectator.name.erase(10, spectator.name.length() - 10);
			spectator.name.append(xor_c("..."));
		}
	}
}

void c_menu::draw_bomb_indicator()
{
	const std::unique_lock<std::mutex> lock(mutexes::bomb);

	if (!(g_cfg.misc.menu_indicators & 2))
	{
		bomb.reset();
		return;
	}

	const auto window_size = ImVec2(125, 60);
	static auto opened = true;
	static float alpha = 0.f;

	this->create_animation(alpha, g_cfg.misc.menu || bomb.filled, 1.f, lerp_animation);

	static bool set_position = false;
	if (g_ctx.loading_config)
		set_position = true;

	if (g_cfg.misc.bomb_position.x == 0 && g_cfg.misc.bomb_position.y == 0)
	{
		g_cfg.misc.bomb_position.x = g_render->screen_size.w / 2 - window_size.x / 2;
		g_cfg.misc.bomb_position.y = g_render->screen_size.h / 7 - window_size.y;

		set_position = true;
	}

	if (alpha <= 0.f)
		return;

	if (set_position)
	{
		ImGui::SetNextWindowPos(g_cfg.misc.bomb_position);
		set_position = false;
	}

	ImGui::SetNextWindowSize(window_size);
	ImGui::PushFont(g_fonts.main);
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(xor_c("##bomb_window"), &opened, misc_ui_flags);

	auto list = ImGui::GetWindowDrawList();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	// window body
	{
		auto bomb_size = ImVec2(112.f, 48.f);
		auto window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);
		auto window_alpha = 255.f * alpha;

		// header
		imgui_blur::create_blur(list, window_pos, ImVec2(window_pos.x + bomb_size.x, window_pos.y + bomb_size.y), color(155, 155, 155, window_alpha).as_imcolor(), 8.f);

		list->AddImage((void*)bomb_texture, ImVec2(window_pos.x + 9, window_pos.y + 16), ImVec2(window_pos.x + 41, window_pos.y + 47), ImVec2(0, 0), ImVec2(1, 1), color(255, 255, 255, window_alpha).as_imcolor());

		// border
		list->AddRect(window_pos, ImVec2(window_pos.x + bomb_size.x, window_pos.y + bomb_size.y), color(100, 100, 100, 100.f * alpha).as_imcolor(), 8.f);

		ImGui::PushFont(g_fonts.bold_large);
		list->AddText(window_pos + ImVec2(17.f, 7.f), color(255, 255, 255, 255 * alpha).as_imcolor(), bomb.bomb_site.c_str());
		ImGui::PopFont();

		ImGui::PushFont(g_fonts.bold2);
		auto str = std::to_string(bomb.time) + xor_c("s");
		auto text_size = ImGui::CalcTextSize(str.c_str());
		list->AddText(window_pos + ImVec2(52.f, 10.f), color(255, 255, 255, 255 * alpha).as_imcolor(), str.c_str());
		ImGui::PopFont();

		ImGui::PushFont(g_fonts.misc);
		list->AddText(window_pos + ImVec2(text_size.x + 55, 10.f), color(255, 255, 255, 255 * alpha).as_imcolor(), xor_c("left"));

		std::string str2 = "";
		if (bomb.health > 0)
			str2 += xor_c("-");

		str2 += std::to_string(bomb.health);
		str2 += xor_c("HP");

		list->AddText(window_pos + ImVec2(55.f, 25.f), color(255, 255, 255, 255 * alpha).as_imcolor(), str2.c_str());
		ImGui::PopFont();
	}

	if (!set_position)
		g_cfg.misc.bomb_position = ImGui::GetWindowPos();

	list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

	ImGui::End(false);
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void c_menu::draw_watermark()
{
	if (!g_ctx.cheat_init2 || !(g_cfg.misc.menu_indicators & 4))
		return;

	auto image_size = ImVec2{ 16, 16 };

	char cur_time[128]{};

	time_t t;
	struct tm* ptm;

	t = time(NULL);
	ptm = localtime(&t);

	strftime(cur_time, 128, xor_c("%H:%M"), ptm);

	auto calculated_ping = g_ctx.real_ping == -1.f ? 0 : (int)(g_ctx.real_ping * 1000.f);
	auto ping = tfm::format(xor_c("%dms"), calculated_ping);

	std::string current_username{};

	ImVec2 text_size{};
	ImGui::PushFont(g_fonts.main);
	{
		current_username = g_cheat_info->user_name.size() > 0 ? g_cheat_info->user_name : xor_str("empty");
		auto watermark_string = tfm::format(xor_c("%s %s"), this->prefix, tfm::format(xor_c("%s | %s  %s"), current_username, cur_time, ping));

		text_size = ImGui::CalcTextSize(watermark_string.c_str());
	}
	ImGui::PopFont();

	auto window_size = ImVec2(75.f + text_size.x + image_size.x, 32.f);
	static auto opened = true;

	static bool set_position = false;
	if (g_ctx.loading_config)
		set_position = true;

	if (g_cfg.misc.watermark_position.x == 0 && g_cfg.misc.watermark_position.y == 0)
	{
		g_cfg.misc.watermark_position.x = g_render->screen_size.w - window_size.x - 10.f;
		g_cfg.misc.watermark_position.y = 10;

		set_position = true;
	}

	auto current_pos = g_cfg.misc.watermark_position.x + window_size.x;
	if (current_pos > (g_render->screen_size.w - 10.f))
	{
		if (!set_position)
		{
			g_cfg.misc.watermark_position.x -= 5.f;
			set_position = true;
		}
	}

	if (set_position)
	{
		ImGui::SetNextWindowPos(g_cfg.misc.watermark_position);
		set_position = false;
	}

	ImGui::SetNextWindowSize(window_size);
	ImGui::PushFont(g_fonts.main);
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(xor_c("##watermark_window"), &opened, misc_ui_flags);
	{
		auto list = ImGui::GetWindowDrawList();
		list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

		auto prefix_size = ImGui::CalcTextSize(this->prefix.c_str()) + image_size;

		auto base_window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);

		// left side
		{
			imgui_blur::create_blur(list, base_window_pos, base_window_pos + ImVec2{ prefix_size.x + 30.f, window_size.y - 3.f},
				ImColor(255, 255, 255, 255), 3.f, ImDrawCornerFlags_Left);

			auto base_offset = ImVec2{ base_window_pos.x + 10.f, base_window_pos.y + 7.f };

			auto clr = g_cfg.misc.ui_color.base();
			list->AddImage((void*)logo_texture, base_offset, base_offset + image_size, ImVec2(0, 0), ImVec2(1, 1), clr.as_imcolor());

			auto watermark_offset = base_offset + ImVec2{ image_size.x + 6.f, -0.5f };
			list->AddText(watermark_offset, ImColor(255, 255, 255, 255), this->prefix.c_str());
		}

		// right side
		{
			auto left_side_watermark_end = prefix_size.x + 30.f;
			auto left_side_end = base_window_pos + ImVec2{ left_side_watermark_end, 0.f };
			imgui_blur::create_blur(list, left_side_end, left_side_end + ImVec2{ window_size.x - left_side_watermark_end - 10.f, window_size.y - 3.f },
				ImColor(100, 100, 100, 255), 3.f, ImDrawCornerFlags_Right);

			auto avatar_base = ImVec2{ left_side_end.x + 7.f, left_side_end.y + 6.f };

			if (avatar && !g_cheat_info->user_avatar.empty() && g_cheat_info->user_avatar.size())
				list->AddImageRounded((void*)avatar, avatar_base, avatar_base + image_size, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255), 20.f);
			else
				list->AddRectFilled(avatar_base, avatar_base + image_size, ImColor(255, 255, 255), 20.f);

			auto avatar_end = ImVec2{ avatar_base.x + image_size.x + 6.f, left_side_end.y + 6.5f };

			auto current_text = tfm::format(xor_c("%s | %s %s"), current_username, cur_time, ping);
			list->AddText(avatar_end, ImColor(255, 255, 255, 255), current_text.c_str());
		}

		// border
		list->AddRect(base_window_pos, ImVec2(base_window_pos.x + window_size.x - 10.f, base_window_pos.y + window_size.y - 3.f), color(120, 120, 120, 100.f).as_imcolor(), 3.f);

		list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);
	}
	ImGui::End(false);
	ImGui::PopFont();
	ImGui::PopStyleColor();

	/*char cur_time[128]{};

	time_t t;
	struct tm* ptm;

	t = time(NULL);
	ptm = localtime(&t);

	strftime(cur_time, 128, xor_c("%H:%M"), ptm);

	auto ping = g_ctx.real_ping == -1.f ? 0 : (int)(g_ctx.real_ping * 1000.f);
	auto time_str = xor_str("  |  ") + std::string(cur_time) + " " + std::to_string(ping) + xor_str("ms");

	ImGui::PushFont(g_fonts.main);
	auto nick = g_ctx.username.size() > 0 ? g_ctx.username : xor_str("empty");
	auto info_str = this->prefix + "   " + nick + time_str;
	auto right_info_str = nick + time_str;
	auto info_text_size = ImGui::CalcTextSize(info_str.c_str());

	ImGui::PopFont();

	auto window_size = ImVec2(info_text_size.x * 2.f, 30);
	static auto opened = true;

	static bool set_position = false;
	if (g_ctx.loading_config)
		set_position = true;

	if (g_cfg.misc.watermark_position.x == 0 && g_cfg.misc.watermark_position.y == 0)
	{
		g_cfg.misc.watermark_position.x = g_render->screen_size.w - window_size.x - 10.f;
		g_cfg.misc.watermark_position.y = 10;

		set_position = true;
	}

	auto current_pos = g_cfg.misc.watermark_position.x + window_size.x;
	if (current_pos > (g_render->screen_size.w - 10.f))
	{
		if (!set_position)
		{
			g_cfg.misc.watermark_position.x -= 5.f;
			set_position = true;
		}
	}

	if (set_position)
	{
		ImGui::SetNextWindowPos(g_cfg.misc.watermark_position);
		set_position = false;
	}

	ImGui::SetNextWindowSize(window_size);
	ImGui::PushFont(g_fonts.main);
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(xor_c("##watermark_window"), &opened,
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoFocusOnAppearing);

	auto list = ImGui::GetWindowDrawList();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
	{
		auto watermark_size = ImVec2(window_size.x - 13.f, window_size.y - 2.f);
		auto window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);
		auto window_alpha = 255.f;

		ImGui::PushFont(g_fonts.main);

		// left part base
		{
			imgui_blur::create_blur(list, window_pos, ImVec2((window_pos.x + watermark_size.x / 3.f), window_pos.y + watermark_size.y), color(255, 255, 255, window_alpha).as_imcolor(), 4.f, ImDrawCornerFlags_Left);

			// logo
			auto base_pos = (watermark_size.x / 3.f) / 2.f;
			auto image_size = ImVec2(10, 10);
			auto image_base = ImVec2(base_pos, watermark_size.y);

			auto image_pos_min = ImVec2((image_base.x / 2) - (image_size.x - 2), (image_base.y / 2) - (image_size.y - 2));
			auto image_pos_max = ImVec2((image_base.x / 2) + (image_size.x - 2), (image_base.y / 2) + (image_size.y - 2));

			auto clr = g_cfg.misc.ui_color.base();

			auto text_size = ImGui::CalcTextSize(this->prefix.c_str());

			list->AddImage((void*)logo_texture, window_pos + image_pos_min, window_pos + image_pos_max, ImVec2(0, 0), ImVec2(1, 1), clr.new_alpha(window_alpha).as_imcolor());

			auto base_x = window_pos.x + image_pos_max.x + 4.f;
			list->AddText(ImVec2(base_x + 5.f, window_pos.y + ((watermark_size.y / 2.f) - text_size.y / 2.f) - 1.f), color(255, 255, 255, 150.f).as_imcolor(), this->prefix.c_str());
		}

		// right part
		{
			auto right_base = window_pos + ImVec2((watermark_size.x / 3.f) + 1.f, 0.f);

			imgui_blur::create_blur(list, right_base, ImVec2(window_pos.x + watermark_size.x - 1.f, window_pos.y + watermark_size.y), color(80, 80, 80, window_alpha).as_imcolor(), 4.f, ImDrawCornerFlags_Right);

			auto image_size = ImVec2(10, 10);
			auto image_base = ImVec2((watermark_size.x / 3.f) - 13.f, watermark_size.y);

			auto image_pos_min = ImVec2((image_base.x / 2) - (image_size.x - 2), (image_base.y / 2) - (image_size.y - 2));
			auto image_pos_max = ImVec2((image_base.x / 2) + (image_size.x - 2), (image_base.y / 2) + (image_size.y - 2));

			if (avatar && !g_ctx.avatar.empty() && g_ctx.avatar.size())
				list->AddImageRounded((void*)avatar, right_base + image_pos_min, right_base + image_pos_max, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255), 20.f);
			else
				list->AddRectFilled(right_base + image_pos_min, right_base + image_pos_max, ImColor(255, 255, 255), 20.f);

			auto base_x = right_base.x + image_pos_max.x + 1.f;
			list->AddText(ImVec2(base_x + 5.f, right_base.y + ((watermark_size.y / 2.f) - info_text_size.y / 2.f) - 1.f), color(255, 255, 255, 150.f).as_imcolor(), right_info_str.c_str());
		}

		// separator
		list->AddLine(window_pos + ImVec2((watermark_size.x / 3.f), 0.f), window_pos + ImVec2((watermark_size.x / 3.f), watermark_size.y), color(100, 100, 100, 100).as_imcolor());

		// border
		list->AddRect(window_pos, ImVec2(window_pos.x + watermark_size.x + 1.f, window_pos.y + watermark_size.y), color(100, 100, 100, 100.f).as_imcolor(), 4.f);

		ImGui::PopFont();
	}

	list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

	ImGui::PopStyleColor();
	ImGui::End(false);*/
}

void c_menu::on_game_events(c_game_event* event)
{
	if (std::strcmp(event->get_name(), xor_c("round_start")))
		return;

	for (auto& i : spectators)
		i.reset();

	bomb.reset();
}