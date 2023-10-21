#include "../../globals.hpp"
#include "../config_system.h"
#include "../config_vars.h"

#include "menu.h"
#include "../../entlistener.hpp"

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
		return CXOR("[ enabled ]");
		break;
	case 1:
		return CXOR("[ hold ]");
		break;
	case 2:
		return CXOR("[ toggled ]");
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
				binds.time = HACKS->system_time();

				window_size.y += 25.f;
			}
			else
			{
				binds.reset(HACKS->system_time());
				window_size.y -= 25.f;
			}

			prev_bind = current_bind;
		}
	}

	this->create_animation(alpha, g_cfg.misc.menu || HACKS->in_game && window_size.y > 40.f, 1.f, lerp_animation);

	static bool set_position = false;
	if (HACKS->loading_config)
		set_position = true;

	if (g_cfg.misc.keybind_position.x == 0 && g_cfg.misc.keybind_position.y == 0)
	{
		g_cfg.misc.keybind_position.x = 10;
		g_cfg.misc.keybind_position.y = RENDER->screen.y / 2 - window_size.y / 2;

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

	ImGui::PushFont(RENDER->fonts.main.get());
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(CXOR("##bind_window"), &opened, misc_ui_flags);

	auto list = ImGui::GetWindowDrawList();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	// window body
	{
		auto keybinds_size = ImVec2(176.f, 32.f);
		auto window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);
		auto window_alpha = 255.f * alpha;

		// header
		imgui_blur::create_blur(list, window_pos, window_pos + ImVec2(keybinds_size.x, 32.f), c_color(255, 255, 255, window_alpha).as_imcolor(), 4.f, ImDrawCornerFlags_Top);

		list->AddImage((void*)keyboard_texture, ImVec2(window_pos.x + 51, window_pos.y + 9), ImVec2(window_pos.x + 67, window_pos.y + 25), ImVec2(0, 0), ImVec2(1, 1), c_color(255, 255, 255, window_alpha).as_imcolor());

		list->AddText(ImVec2(window_pos.x + 75, window_pos.y + 8), c_color(255, 255, 255, window_alpha).as_imcolor(), CXOR("keybinds"));

		list->AddLine(window_pos + ImVec2(0, 31.f), window_pos + ImVec2(keybinds_size.x, 31), c_color(255, 255, 255, 12.75f * alpha).as_imcolor());

		// body
		imgui_blur::create_blur(list, window_pos + ImVec2(0, 32.f), window_pos + ImVec2(keybinds_size.x, 32.f + window_size.y), c_color(100, 100, 100, (int)(window_alpha)).as_imcolor(), 4.f, ImDrawCornerFlags_Bot);

		// border
		list->AddRect(window_pos, ImVec2(window_pos.x + keybinds_size.x, window_pos.y + window_size.y), c_color(100, 100, 100, 100.f * alpha).as_imcolor(), 4.f);
	}

	// bind text
	auto prev_pos = ImGui::GetCursorPos();

	auto max_pos = 0.f;
	for (auto& keybind : updated_keybinds)
	{
		float time_difference = HACKS->system_time() - keybind.second.time;
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

				animation.start_time = HACKS->system_time();
				animation.was_spectating = true;
			}
		}
		else
		{
			if (animation.was_spectating)
			{
				window_size.y -= 25.f;

				animation.start_time = HACKS->system_time();
				animation.was_spectating = false;
			}
		}
	}

	this->create_animation(alpha, g_cfg.misc.menu || HACKS->in_game && window_size.y > 40.f, 1.f, lerp_animation);

	static bool set_position = false;
	if (HACKS->loading_config)
		set_position = true;

	if (g_cfg.misc.spectators_position.x == 0 && g_cfg.misc.spectators_position.y == 0)
	{
		g_cfg.misc.spectators_position.x = 10;
		g_cfg.misc.spectators_position.y = (RENDER->screen.y / 2) + window_size.y + 10.f;

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

	ImGui::PushFont(RENDER->fonts.main.get());
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(CXOR("##spec_window"), &opened, misc_ui_flags);

	auto list = ImGui::GetWindowDrawList();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	// window body
	{
		auto keybinds_size = ImVec2(176.f, 32.f);
		auto window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);
		auto window_alpha = 255.f * alpha;

		// header
		imgui_blur::create_blur(list, window_pos, window_pos + ImVec2(keybinds_size.x, 32.f), c_color(255, 255, 255, window_alpha).as_imcolor(), 4.f, ImDrawCornerFlags_Top);

		list->AddImage((void*)spectator_texture,
			ImVec2(window_pos.x + 41, window_pos.y + 9),
			ImVec2(window_pos.x + 57, window_pos.y + 25),
			ImVec2(0, 0), ImVec2(1, 1), 
			c_color(255, 255, 255, window_alpha).as_imcolor());

		list->AddText(
			ImVec2(window_pos.x + 65, window_pos.y + 8),
			c_color(255, 255, 255, window_alpha).as_imcolor(),
			CXOR("spectators"));

		list->AddLine(
			window_pos + ImVec2(0, 31.f),
			window_pos + ImVec2(keybinds_size.x, 31), 
			c_color(255, 255, 255, 12.75f * alpha).as_imcolor());

		// body
		imgui_blur::create_blur(list, 
			window_pos + ImVec2(0, 32.f), 
			window_pos + ImVec2(keybinds_size.x, 32.f + window_size.y), 
			c_color(100, 100, 100, (int)(window_alpha)).as_imcolor(), 4.f, ImDrawCornerFlags_Bot);

		// border
		list->AddRect(window_pos, 
			ImVec2(window_pos.x + keybinds_size.x, window_pos.y + window_size.y),
			c_color(100, 100, 100, 100.f * alpha).as_imcolor(),
			4.f);
	}

	// bind text
	auto prev_pos = ImGui::GetCursorPos();

	auto max_pos = 0.f;
	for (int i = 0; i < 50; ++i)
	{
		auto& spectator = spectators[i];
		auto& animation = spectator_animaiton[i];

		float time_difference = HACKS->system_time() - animation.start_time;
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

	if (!(g_cfg.misc.menu_indicators & 2) || !HACKS->in_game || !HACKS->local)
	{
		bomb.reset();
		return;
	}

	bomb.reset();
	LISTENER_ENTITY->for_each_entity([&](c_base_entity* entity) 
	{
		float blow_time = (entity->c4_blow() - HACKS->global_vars->curtime);
		if (blow_time <= 0.f)
			return;

		const auto damagePercentage = 1.0f;

		auto damage = 500.f; // 500 - default, if radius is not written on the map https://i.imgur.com/mUSaTHj.png
		auto bomb_radius = damage * 3.5f;
		auto distance_to_local = (entity->origin() - HACKS->local->origin()).length();
		auto sigma = bomb_radius / 3.0f;
		auto fGaussianFalloff = exp(-distance_to_local * distance_to_local / (2.0f * sigma * sigma));
		auto adjusted_damage = damage * fGaussianFalloff * damagePercentage;

		bomb.defused = entity->bomb_defused();
		bomb.defusing = entity->bomb_defuser() != -1;
		bomb.time = (int)blow_time;
		bomb.health = scale_damage_armor(adjusted_damage, HACKS->local->armor_value());
		bomb.bomb_site = entity->bomb_site() == 0 ? CXOR("A") : CXOR("B");
		bomb.filled = true;

	}, ENT_WEAPON);
}

void c_menu::store_spectators()
{
	const std::unique_lock<std::mutex> lock(mutexes::spectators);
	if (!(g_cfg.misc.menu_indicators & 8) || !HACKS->in_game || !HACKS->local)
	{
		for (auto& i : spectators)
			i.reset();

		return;
	}
	
	auto iterator = 0;
	LISTENER_ENTITY->for_each_player([&](c_cs_player* player) 
	{
		auto& spectator = spectators[iterator++];
		spectator.ptr = player;

		if (player->is_alive() || player->dormant() || player->has_gun_game_immunity())
		{
			spectator.spectated = false;
			return;
		}

		auto spec_target = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(player->observer_target()));
		if (!spec_target || player->observer_mode() == OBS_MODE_NONE)
		{
			spectator.spectated = false;
			return;
		}

		auto spectated_name = [&]() -> std::string
		{
			auto name = spec_target->get_name();

			if (name.size() > 8) {
				name.erase(8, name.length() - 8);
				name.append(CXOR("..."));
			}

			return name;
		};

		spectator.chase_mode = tfm::format(CXOR("[ %s ]"), spectated_name());
		spectator.spectated = true;
		spectator.name = player->get_name();

		if (spectator.name.size() > 10) {
			spectator.name.erase(10, spectator.name.length() - 10);
			spectator.name.append(CXOR("..."));
		}

	}, false);
}

void c_menu::draw_bomb_indicator()
{
	const std::unique_lock<std::mutex> lock(mutexes::bomb);

	if (!(g_cfg.misc.menu_indicators & 2))
	{
		bomb.reset();
		return;
	}

	const auto window_size = ImVec2(147, 63);
	static auto opened = true;
	static float alpha = 0.f;

	this->create_animation(alpha, g_cfg.misc.menu || bomb.filled, 1.f, lerp_animation);

	static bool set_position = false;
	if (HACKS->loading_config)
		set_position = true;

	if (g_cfg.misc.bomb_position.x == 0 && g_cfg.misc.bomb_position.y == 0)
	{
		g_cfg.misc.bomb_position.x = RENDER->screen.x / 2 - window_size.x / 2;
		g_cfg.misc.bomb_position.y = RENDER->screen.y / 7 - window_size.y;

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
	ImGui::PushFont(RENDER->fonts.main.get());
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(CXOR("##bomb_window"), &opened, misc_ui_flags);

	auto list = ImGui::GetWindowDrawList();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	// window body
	{
		auto bomb_size = ImVec2(134.f, 51.f);
		auto window_pos = ImGui::GetWindowPos() + ImVec2(4.f, 1.f);
		auto window_alpha = 255.f * alpha;

		// header
		imgui_blur::create_blur(list, window_pos, ImVec2(window_pos.x + bomb_size.x, window_pos.y + bomb_size.y), c_color(155, 155, 155, window_alpha).as_imcolor(), 8.f);

		list->AddImage((void*)bomb_texture, ImVec2(window_pos.x + 19, window_pos.y + 16), ImVec2(window_pos.x + 51, window_pos.y + 47), ImVec2(0, 0), ImVec2(1, 1), c_color(255, 255, 255, window_alpha).as_imcolor());

		// border
		list->AddRect(window_pos, ImVec2(window_pos.x + bomb_size.x, window_pos.y + bomb_size.y), c_color(100, 100, 100, 100.f * alpha).as_imcolor(), 8.f);

		ImGui::PushFont(RENDER->fonts.bold_large.get());
		list->AddText(window_pos + ImVec2(27.f, 7.f), c_color(255, 255, 255, 255 * alpha).as_imcolor(), bomb.bomb_site.c_str());
		ImGui::PopFont();
		
		auto real_health_calc = HACKS->in_game;

		auto hide_health = false;
		if (real_health_calc && bomb.health <= 0)
			hide_health = true;

		if (bomb.defusing || bomb.defused)
			hide_health = true;

		g_menu.create_animation(bomb_health_text_lerp, hide_health, 0.4f, lerp_animation);

		auto string = tfm::format(CXOR("%ds"), bomb.time);
		if (bomb.defusing)
			string = CXOR("defuse");
		if (bomb.defused)
			string = CXOR("defused");

		ImVec2 text_size{};
		float text_offset{};
		{
			ImGui::PushFont(RENDER->fonts.bold2.get());

			text_size = ImGui::CalcTextSize(string.c_str());
			text_offset = text_size.x < 20 ? 4.f : 0.f;
			list->AddText(window_pos + ImVec2(60.f + text_offset, 10.f + 8.f * (bomb_health_text_lerp)), c_color(255, 255, 255, 255 * alpha).as_imcolor(), string.c_str());

			ImGui::PopFont();
		}

		ImGui::PushFont(RENDER->fonts.misc.get());

		if (!bomb.defusing && !bomb.defused)
			list->AddText(window_pos + ImVec2(text_size.x + 64 + text_offset, 10.f + 8.f * (bomb_health_text_lerp)), c_color(255, 255, 255, 255 * alpha).as_imcolor(), CXOR("left"));

		if (!real_health_calc || bomb_health_text_lerp > 0.f)
		{
			std::string str2 = tfm::format(CXOR("-%d HP"), bomb.health);

			auto alpha_mod = real_health_calc ? (1.f - bomb_health_text_lerp) : 1.f;
			list->AddText(window_pos + ImVec2(62.f, 25.f), c_color(255, 255, 255, 255 * alpha * alpha_mod).as_imcolor(), str2.c_str());
		}

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
	if (!HACKS->cheat_init2 || !(g_cfg.misc.menu_indicators & 4))
		return;

#ifndef _DEBUG
	/*if (!avatar || HACKS->cheat_info.user_name.empty() || HACKS->cheat_info.user_avatar.empty() || HACKS->cheat_info.user_token.empty())
		return;*/
#endif

	auto image_size = ImVec2{ 16, 16 };

	char cur_time[128]{};

	time_t t;
	struct tm* ptm;

	t = time(NULL);
	ptm = localtime(&t);

	strftime(cur_time, 128, CXOR("%H:%M"), ptm);

	auto calculated_ping = HACKS->real_ping == -1.f ? 0 : (int)(HACKS->real_ping * 1000.f);
	auto ping = tfm::format(CXOR("%dms"), calculated_ping);

	std::string current_username{};

	ImVec2 text_size{};
	ImGui::PushFont(RENDER->fonts.main.get());
	{
		current_username = HACKS->cheat_info.user_name;
		auto watermark_string = tfm::format(CXOR("%s %s"), this->prefix, tfm::format(CXOR("%s | %s  %s"), current_username, cur_time, ping));

		text_size = ImGui::CalcTextSize(watermark_string.c_str());
	}
	ImGui::PopFont();

	auto window_size = ImVec2(75.f + text_size.x + image_size.x, 32.f);
	static auto opened = true;

	static bool set_position = false;
	if (HACKS->loading_config)
		set_position = true;

	if (g_cfg.misc.watermark_position.x == 0 && g_cfg.misc.watermark_position.y == 0)
	{
		g_cfg.misc.watermark_position.x = RENDER->screen.x - window_size.x - 10.f;
		g_cfg.misc.watermark_position.y = 10;

		set_position = true;
	}

	auto current_pos = g_cfg.misc.watermark_position.x + window_size.x;
	if (current_pos > (RENDER->screen.x - 10.f))
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
	ImGui::PushFont(RENDER->fonts.main.get());
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(CXOR("##watermark_window"), &opened, misc_ui_flags);
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

			if (avatar && !HACKS->cheat_info.user_avatar.empty() && HACKS->cheat_info.user_avatar.size())
				list->AddImageRounded((void*)avatar, avatar_base, avatar_base + image_size, ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255), 20.f);
			else
				list->AddRectFilled(avatar_base, avatar_base + image_size, ImColor(255, 255, 255), 20.f);

			auto avatar_end = ImVec2{ avatar_base.x + image_size.x + 6.f, left_side_end.y + 6.5f };

			auto current_text = tfm::format(CXOR("%s | %s %s"), current_username, cur_time, ping);
			list->AddText(avatar_end, ImColor(255, 255, 255, 255), current_text.c_str());
		}

		// border
		list->AddRect(base_window_pos, ImVec2(base_window_pos.x + window_size.x - 10.f, base_window_pos.y + window_size.y - 3.f), c_color(120, 120, 120, 100.f).as_imcolor(), 3.f);

		list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);
	}
	ImGui::End(false);
	ImGui::PopFont();
	ImGui::PopStyleColor();
}

void c_menu::on_game_events(c_game_event* event)
{
	if (std::strcmp(event->get_name(), CXOR("round_start")))
		return;

	for (auto& i : spectators)
		i.reset();

	bomb.reset();
}