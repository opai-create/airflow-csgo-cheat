#include "../../globals.hpp"
#include "menu.h"
#include "snow.h"

#include "../config_system.h"
#include "../config_vars.h"

#include <ShlObj.h>
#include <algorithm>
#include <map>
#include <d3d9.h>
#include <d3dx9.h>

/*
#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif // !_DEBUG
*/

#define add_texture_to_memory D3DXCreateTextureFromFileInMemory

std::vector< std::string > key_strings = { XOR("None"), XOR("M1"), XOR("M2"), XOR("Ctrl+brk"), XOR("M3"), XOR("M4"), XOR("M5"), XOR(" "), XOR("Back"), XOR("Tab"),
  XOR(" "), XOR(" "), XOR(" "), XOR("Enter"), XOR(" "), XOR(" "), XOR("Shift"), XOR("Ctrl"), XOR("Alt"), XOR("Pause"), XOR("Caps"), XOR(" "), XOR(" "),
  XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR("Esc"), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR("Spacebar"), XOR("Pgup"), XOR("Pgdwn"), XOR("End"),
  XOR("Home"), XOR("Left"), XOR("Up"), XOR("Right"), XOR("Down"), XOR(" "), XOR("Print"), XOR(" "), XOR("Prtsc"), XOR("Insert"), XOR("Delete"), XOR(" "),
  XOR("0"), XOR("1"), XOR("2"), XOR("3"), XOR("4"), XOR("5"), XOR("6"), XOR("7"), XOR("8"), XOR("9"), XOR(" "), XOR(" "), XOR(" "), XOR(" "),
  XOR(" "), XOR(" "), XOR(" "), XOR("A"), XOR("B"), XOR("C"), XOR("D"), XOR("E"), XOR("F"), XOR("G"), XOR("H"), XOR("I"), XOR("J"), XOR("K"),
  XOR("L"), XOR("M"), XOR("N"), XOR("O"), XOR("P"), XOR("Q"), XOR("R"), XOR("S"), XOR("T"), XOR("U"), XOR("V"), XOR("W"), XOR("X"), XOR("Y"),
  XOR("Z"), XOR("Lw"), XOR("Rw"), XOR(" "), XOR(" "), XOR(" "), XOR("Num 0"), XOR("Num 1"), XOR("Num 2"), XOR("Num 3"), XOR("Num 4"), XOR("Num 5"),
  XOR("Num 6"), XOR("Num 7"), XOR("Num 8"), XOR("Num 9"), XOR("*"), XOR("+"), XOR("_"), XOR("-"), XOR("."), XOR("/"), XOR("F1"), XOR("F2"), XOR("F3"),
  XOR("F4"), XOR("F5"), XOR("F6"), XOR("F7"), XOR("F8"), XOR("F9"), XOR("F10"), XOR("F11"), XOR("F12"), XOR("F13"), XOR("F14"), XOR("F15"), XOR("F16"),
  XOR("F17"), XOR("F18"), XOR("F19"), XOR("F20"), XOR("F21"), XOR("F22"), XOR("F23"), XOR("F24"), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "),
  XOR(" "), XOR(" "), XOR(" "), XOR("Num lock"), XOR("Scroll lock"), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "),
  XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR("Lshift"), XOR("Rshift"), XOR("Lcontrol"), XOR("Rcontrol"), XOR("Lmenu"), XOR("Rmenu"),
  XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR("Next track"), XOR("Previous track"),
  XOR("Stop"), XOR("Play/pause"), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(";"), XOR("+"), XOR("),"), XOR("-"), XOR("."),
  XOR("/?"), XOR("~"), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "),
  XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "),
  XOR("[{"), XOR("\\|"), XOR("}]"), XOR("'\""), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "),
  XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "),
  XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" "), XOR(" ") };

std::array< std::string, max_tabs > tabs = { XOR("Rage"), XOR("Legit"), XOR("Anti-hit"), XOR("Visuals"), XOR("Misc"), XOR("Skins"), XOR("Configs") };

void c_menu::create_animation(float& mod, bool cond, float speed_multiplier, unsigned int animation_flags)
{
	float time = (RENDER->get_animation_speed() * 5.f) * speed_multiplier;

	if ((animation_flags & skip_enable) && cond)
		mod = 1.f;
	else if ((animation_flags & skip_disable) && !cond)
		mod = 0.f;

	if (animation_flags & lerp_animation)
		mod = std::lerp(mod, (float)cond, time);
	else
	{
		if (cond && mod <= 1.f)
			mod += time;
		else if (!cond && mod >= 0.f)
			mod -= time;
	}

	mod = std::clamp(mod, 0.f, 1.f);
}

void c_menu::update_alpha()
{
	this->create_animation(alpha, g_cfg.misc.menu, 0.3f);
}

float c_menu::get_alpha()
{
	return alpha;
}

void c_menu::set_window_pos(const ImVec2& pos)
{
	window_pos = pos;
}

ImVec2 c_menu::get_window_pos()
{
	return window_pos + ImVec2(45, 15);
}

void c_menu::init_textures()
{
	if (!logo_texture)
		add_texture_to_memory(RENDER->get_device(), cheatLogo, sizeof(cheatLogo), &logo_texture);

	if (!keyboard_texture)
		add_texture_to_memory(RENDER->get_device(), keyboard_icon, sizeof(keyboard_icon), &keyboard_texture);

	if (!warning_texture)
		add_texture_to_memory(RENDER->get_device(), warning_icon, sizeof(warning_icon), &warning_texture);

	if (!spectator_texture)
		add_texture_to_memory(RENDER->get_device(), spectators_icon, sizeof(spectators_icon), &spectator_texture);

	if (!bomb_texture)
		add_texture_to_memory(RENDER->get_device(), bomb_indicator, sizeof(bomb_indicator), &bomb_texture);

	if (!icon_textures[0])
		add_texture_to_memory(RENDER->get_device(), rage_icon, sizeof(rage_icon), &icon_textures[0]);

	if (!icon_textures[1])
		add_texture_to_memory(RENDER->get_device(), legit_icon, sizeof(legit_icon), &icon_textures[1]);

	if (!icon_textures[2])
		add_texture_to_memory(RENDER->get_device(), antihit_icon, sizeof(antihit_icon), &icon_textures[2]);

	if (!icon_textures[3])
		add_texture_to_memory(RENDER->get_device(), visuals_icon, sizeof(visuals_icon), &icon_textures[3]);

	if (!icon_textures[4])
		add_texture_to_memory(RENDER->get_device(), misc_icon, sizeof(misc_icon), &icon_textures[4]);

	if (!icon_textures[5])
		add_texture_to_memory(RENDER->get_device(), skins_icon, sizeof(skins_icon), &icon_textures[5]);

	if (!icon_textures[6])
		add_texture_to_memory(RENDER->get_device(), cfg_icon, sizeof(cfg_icon), &icon_textures[6]);

	//MessageBoxA(0, g_cheat_info->user_avatar.c_str(), 0, 0);

	if (!avatar && !HACKS->cheat_info.user_avatar.empty() && HACKS->cheat_info.user_avatar.size())
		add_texture_to_memory(RENDER->get_device(), HACKS->cheat_info.user_avatar.data(), HACKS->cheat_info.user_avatar.size(), &avatar);
}

void c_menu::set_draw_list(ImDrawList* list)
{
	if (draw_list)
		return;

	draw_list = list;
}

ImDrawList* c_menu::get_draw_list()
{
	return draw_list;
}

void c_menu::window_begin()
{
	ImGui::GetMousePos();

	static auto opened = true;

	const auto window_size = ImVec2(800, 550.f);
	ImGui::SetNextWindowPos(ImVec2((RENDER->screen.x / 2.f - window_size.x / 2.f), RENDER->screen.y / 2.f - window_size.y / 2.f), ImGuiCond_Once);

	ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
	ImGui::PushFont(RENDER->fonts.main.get());
	ImGui::SetNextWindowBgAlpha(0.f);
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::Begin(CXOR("##base_window"), &opened, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoScrollbar);

	ImGui::PushItemWidth(256);

	this->set_draw_list(ImGui::GetWindowDrawList());
	this->set_window_pos(ImGui::GetWindowPos());

	auto list = this->get_draw_list();
	list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	auto window_pos = this->get_window_pos();

	// push rect for render
	list->PushClipRect(window_pos, ImVec2(window_pos.x + 720, window_pos.y + 520));

	// push rect for window
	ImGui::PushClipRect(window_pos, ImVec2(window_pos.x + 720, window_pos.y + 520), false);
}

void c_menu::window_end()
{
	auto list = this->get_draw_list();
	list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);
	list->PopClipRect();
	ImGui::PopClipRect();

	ImGui::PopItemWidth();

	ImGui::End(false);
	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void c_menu::draw_ui_background()
{
	auto list = this->get_draw_list();

	auto alpha = this->get_alpha();
	auto window_alpha = 255.f * this->get_alpha();

	auto window_pos = this->get_window_pos();

	auto header_size = ImVec2(720, 47);
	// header
	imgui_blur::create_blur(list, window_pos, ImVec2(window_pos.x + header_size.x, window_pos.y + header_size.y), c_color(255, 255, 255, window_alpha).as_imcolor(), 6.f, ImDrawCornerFlags_Top);

	// image
	auto image_size = ImVec2(14, 14);
	auto image_pos_min = ImVec2((header_size.x / 2) - (image_size.x - 2), (header_size.y / 2) - (image_size.y - 2));
	auto image_pos_max = ImVec2((header_size.x / 2) + (image_size.x - 2), (header_size.y / 2) + (image_size.y - 2));

	auto clr = g_cfg.misc.ui_color.base();
	
	auto letter = XOR("airflow");

	ImGui::PushFont(RENDER->fonts.dmg.get());
	auto text_size = ImGui::CalcTextSize(letter.c_str());

	list->AddImage(
		(void*)logo_texture, window_pos + image_pos_min - ImVec2(text_size.x - 18, 0.f), window_pos + image_pos_max - ImVec2(text_size.x - 18, 0.f), ImVec2(0, 0), ImVec2(1, 1), clr.new_alpha(window_alpha).as_imcolor());

	auto base_x = window_pos.x + image_pos_min.x - text_size.x + 18.f;
	list->AddText(ImVec2(base_x + 30.f, window_pos.y + 13), c_color(255, 255, 255, 150.f * alpha).as_imcolor(), letter.c_str());
	ImGui::PopFont();

	// body
	imgui_blur::create_blur(list, window_pos + ImVec2(0, 47), ImVec2(window_pos.x + 720, window_pos.y + 520), ImColor(80, 80, 80, (int)(window_alpha)), 6.f, ImDrawCornerFlags_Bot);

	// header end (separator)
	list->AddLine(window_pos + ImVec2(0, 46), window_pos + ImVec2(720, 46), c_color(255, 255, 255, 12.75f * alpha).as_imcolor());

	// tab separator
	list->AddLine(window_pos + ImVec2(160, 47), window_pos + ImVec2(160, 520), c_color(255, 255, 255, 12.75f * alpha).as_imcolor(), 1.f);

	// border
	list->AddRect(window_pos, ImVec2(window_pos.x + 720, window_pos.y + 520), c_color(100, 100, 100, 100.f * alpha).as_imcolor(), 6.f);
}

void c_menu::draw_tabs()
{
	auto list = this->get_draw_list();

	auto prev_pos = ImGui::GetCursorPos();

	auto window_alpha = 255.f * this->get_alpha();
	auto child_pos = this->get_window_pos() + ImVec2(0, 49);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, this->get_alpha()));

	auto clr = g_cfg.misc.ui_color.base();

	for (int i = 0; i < tabs.size(); ++i)
	{
		auto& info = tab_info[i];

		ImGui::SetCursorPos(ImVec2(53, 78 + 40 * i));

		auto tab_str = CXOR("##tab_") + std::to_string(i);
		info.selected = ImGui::ButtonEx(tab_str.c_str(), ImVec2(144, 32), 0, &info.hovered);
		if (info.selected)
			tab_selector = i;

		this->create_animation(info.hovered_alpha, info.hovered, 1.f, lerp_animation);
		this->create_animation(info.alpha, tab_selector == i, 0.8f, skip_disable | lerp_animation);

		auto tab_min = ImVec2(8, 14 + 40 * i);
		auto tab_max = ImVec2(152, 46 + 40 * i);

		auto tab_pos_min = child_pos + tab_min;
		auto tab_pos_max = child_pos + tab_max;

		float rgb_val = tab_selector == i ? 255 : 150 + 105 * info.hovered_alpha;
		c_color text_clr = c_color(rgb_val, rgb_val, rgb_val, rgb_val * this->get_alpha());

		// tab background
		if (tab_selector == i)
		{
			list->AddRectFilled(tab_pos_min, tab_pos_max - ImVec2(2, 0), c_color(255, 255, 255, 10 * info.alpha * this->get_alpha()).as_imcolor(), 4.f, ImDrawCornerFlags_Left);

			// i can't draw rounded rect for 2 pixels
			// so i decided to limit render range and draw rect for 4 pixels
			list->PushClipRect(tab_pos_max - ImVec2(2, 32), tab_pos_max);
			list->AddRectFilled(tab_pos_max - ImVec2(4, 32), tab_pos_max, clr.new_alpha(255 * info.alpha * this->get_alpha()).as_imcolor(), 2.f, ImDrawCornerFlags_Right);
			list->PopClipRect();
		}

		if (icon_textures[i])
			list->AddImage((void*)icon_textures[i], tab_pos_min + ImVec2(10, 9), tab_pos_min + ImVec2(25, 24), ImVec2(0, 0), ImVec2(1, 1), text_clr.as_imcolor());

		list->AddText(ImVec2(tab_pos_min.x + 32, tab_pos_min.y + 8), text_clr.as_imcolor(), tabs[i].c_str());
	}

	ImGui::PopStyleColor();
	ImGui::SetCursorPos(prev_pos);
}

void c_menu::draw_sub_tabs(int& selector, const std::vector< std::string >& tabs)
{
	auto& style = ImGui::GetStyle();
	auto alpha = this->get_alpha();
	auto window_alpha = 255.f * alpha;
	auto child_pos = this->get_window_pos() + ImVec2(178, 62);
	auto prev_pos = ImGui::GetCursorPos();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, this->get_alpha()));

	draw_list->AddRectFilled(child_pos, child_pos + ImVec2(528, 58), c_color(217, 217, 217, 20 * alpha).as_imcolor(), 4.f);

	auto clr = g_cfg.misc.ui_color.base();

	for (int i = 0; i < tabs.size(); ++i)
	{
		auto& info = subtab_info[tabs[0]][i];

		const auto cursor_pos = ImVec2(27 + 80 * i, 14);
		ImGui::SetCursorPos(cursor_pos);

		const auto tab_size = ImVec2(70.f, 32.f);

		auto tab_str = CXOR("##sub_tab_") + tabs[i];
		info.selected = ImGui::ButtonEx(tab_str.c_str(), tab_size, 0, &info.hovered);

		if (info.selected)
			selector = i;

		this->create_animation(info.hovered_alpha, info.hovered, 1.f, lerp_animation);
		this->create_animation(info.alpha, selector == i, 0.8f, skip_disable | lerp_animation);

		// idk why but base pos offsets by 8 pixels
		// so move to 8 pixels left for correct pos
		const auto tab_min = child_pos + cursor_pos - ImVec2(8, 0);
		const auto tab_max = tab_min + tab_size;
		const auto tab_bb = ImRect(tab_min, tab_max);

		if (selector == i)
		{
			draw_list->AddRectFilled(tab_bb.Min, tab_bb.Max, c_color(217, 217, 217, 20 * alpha * info.alpha).as_imcolor(), 4.f);

			// i can't draw rounded rect for 2 pixels
			// so i decided to limit render range and draw rect for 4 pixels
			draw_list->PushClipRect(ImVec2(tab_bb.Min.x + 15.f, tab_bb.Max.y - 2.f), ImVec2(tab_bb.Max.x - 15.f, tab_bb.Max.y));
			draw_list->AddRectFilled(ImVec2(tab_bb.Min.x + 15.f, tab_bb.Max.y - 2.f), ImVec2(tab_bb.Max.x - 15.f, tab_bb.Max.y + 4.f), clr.new_alpha(info.alpha * window_alpha).as_imcolor(), 2.f, ImDrawCornerFlags_Top);
			draw_list->PopClipRect();
		}

		float rgb_val = selector == i ? 255 : 150 + 105 * info.hovered_alpha;
		c_color text_clr = c_color(rgb_val, rgb_val, rgb_val, rgb_val * this->get_alpha());

		const ImVec2 label_size = ImGui::CalcTextSize(tabs[i].c_str(), NULL, true);

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(text_clr.r() / 255.f, text_clr.g() / 255.f, text_clr.b() / 255.f, text_clr.a() / 255.f));
		ImGui::RenderTextClipped(tab_bb.Min, tab_bb.Max, tabs[i].c_str(), NULL, &label_size, style.ButtonTextAlign, &tab_bb);
		ImGui::PopStyleColor();
	}

	ImGui::PopStyleColor();

	ImGui::SetCursorPos(prev_pos);

	// spacing for tab elements
	ImGui::ItemSize(ImVec2(0, 62));

	ImGui::PushClipRect(child_pos + ImVec2(0.f, 62.f), child_pos + ImVec2(540, 457), false);
	draw_list->PushClipRect(child_pos + ImVec2(0.f, 62.f), child_pos + ImVec2(540, 457));
}

std::vector< Snowflake::Snowflake > snow;

void c_menu::draw_snow()
{
	
}

void c_menu::draw()
{
	this->init_textures();

	if (RENDER->screen.x <= 0.f || RENDER->screen.y <= 0.f)
	{
		g_cfg.misc.watermark_position.x = 0.f;
		g_cfg.misc.watermark_position.y = 0.f;

		g_cfg.misc.keybind_position.x = 0.f;
		g_cfg.misc.keybind_position.y = 0.f;

		g_cfg.misc.bomb_position.x = 0.f;
		g_cfg.misc.bomb_position.y = 0.f;

		g_cfg.misc.spectators_position.x = 0.f;
		g_cfg.misc.spectators_position.y = 0.f;
		return;
	}

	this->update_alpha();
	this->draw_binds();
	this->draw_bomb_indicator();
	this->draw_watermark();
	this->draw_spectators();

	static auto reset = false;
	static auto set_ui_focus = false;

	if (!g_cfg.misc.menu && alpha <= 0.f)
	{
		if (reset)
		{
			for (auto& a : item_animations)
			{
				a.second.reset();
				tab_alpha = 0.f;
				subtab_alpha = 0.f;
				subtab_alpha2 = 0.f;
			}
			reset = false;
		}

		HACKS->loading_config = false;
		set_ui_focus = false;
		return;
	}

	reset = true;

	if (!set_ui_focus)
	{
		ImGui::SetNextWindowFocus();
		set_ui_focus = true;
	}

	ImGui::SetColorEditOptions(ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_DisplayRGB);

	//this->draw_snow( );
	this->window_begin();
	this->draw_ui_background();
	this->draw_tabs();
	this->draw_ui_items();
	this->window_end();

	HACKS->loading_config = false;
}