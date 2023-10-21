#include "../menu.h"
#include "../../../base/tools/math.h"
#include "../../config_vars.h"

using namespace ImGui;

bool c_menu::checkbox(const char* label, bool* v)
{
	float alpha = this->get_alpha();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.5f * alpha));

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	const ImVec2 pos = window->DC.CursorPos;
	const ImRect total_bb(pos, pos + ImVec2(256, 36));
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	auto& mod = item_animations[_fnva1(label)];

	bool hovered, held;
	bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);

	if (pressed)
	{
		*v = !(*v);
		MarkItemEdited(id);
	}

	this->create_animation(mod.hovered_alpha, hovered, 1.f, lerp_animation);
	this->create_animation(mod.alpha, *v, 0.4f, lerp_animation);

	auto back_size = ImVec2(256, 32);

	// background
	auto back_alpha = (20 + (30 * mod.hovered_alpha)) * alpha;
	color back_clr = color(217, 217, 217).increase(38 * mod.hovered_alpha).new_alpha(back_alpha);
	draw_list->AddRectFilled(pos, pos + back_size, back_clr.as_imcolor(), 4.f);

	// main body
	auto body_size = ImVec2(28, 14);
	auto body_min = pos + ImVec2(220, 9);
	auto body_max = body_min + body_size;
	draw_list->AddRectFilled(body_min, body_max, color(0, 0, 0, 80 * alpha).as_imcolor(), 8.f);

	auto clr = g_cfg.misc.ui_color.base();

	auto circle_offset = ImVec2(14 * mod.alpha, 0);
	auto circle_pos = body_min + ImVec2(body_size.y / 2, body_size.y / 2) + circle_offset;
	auto circle_clr = color().multiply(clr, mod.alpha).new_alpha(255 * alpha);
	draw_list->AddCircleFilled(circle_pos, 4.f, circle_clr.as_imcolor());

	if (label_size.x > 0.0f)
		RenderText(ImVec2(pos.x + 12, pos.y + 7), label);

	ImGui::PopStyleColor();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | (*v ? ImGuiItemStatusFlags_Checked : 0));
	return pressed;
}

bool c_menu::checkbox_columns(const char* label, unsigned int& v, const std::vector< std::string >& conditions)
{
	float alpha = this->get_alpha();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.5f * alpha));

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	int cond_size = conditions.size();
	auto back_size = ImVec2(256, (43.75f * cond_size) + 13.f);
	const ImVec2 pos = window->DC.CursorPos;
	const ImRect total_bb(pos, pos + back_size);
	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id))
		return false;

	auto& mod = item_animations[_fnva1(label)];

	// background
	color back_clr = color(217, 217, 217).new_alpha(20 * alpha);
	draw_list->AddRectFilled(pos, pos + back_size, back_clr.as_imcolor(), 4.f);

	PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f * alpha));
	if (label_size.x > 0.0f)
		RenderText(ImVec2(pos.x + 12, pos.y + 8), label);
	PopStyleColor();

	auto square_sz = 16.f;
	const float pad = ImMax(1.0f, IM_FLOOR(square_sz / 6.0f));

	auto clr = g_cfg.misc.ui_color.base();

	// separator
	auto separator_pos = pos + ImVec2(0, 31);
	draw_list->AddLine(separator_pos, separator_pos + ImVec2(256, 0), color(0, 0, 0, 80 * alpha).as_imcolor());

	// checkboxes for changing condition
	auto checkboxes_pos = separator_pos + ImVec2(0, 17);
	for (int i = 0; i < cond_size; ++i)
	{
		// checkbox name
		RenderText(ImVec2(checkboxes_pos.x + 12, checkboxes_pos.y + 32 * i), conditions[i].c_str());

		// get checkbox bounds for render, etc.
		auto checkbox_bounds = [&](int idx)
		{
			ImVec2 checkbox_size = ImVec2(16, 16);
			ImVec2 checkbox_base = checkboxes_pos + ImVec2(200 + (24 * idx), (checkbox_size.y * i) * 2);
			ImRect checkbox_bounds = ImRect(checkbox_base, checkbox_base + checkbox_size);

			return checkbox_bounds;
		};

		// hover and press logic
		auto checkbox_active = [&](int idx) -> std::pair< bool, bool >
		{
			const auto id_str = conditions[i] + std::to_string(idx);
			const ImGuiID check_id = window->GetID(id_str.c_str());

			bool hovered, held;
			bool pressed = ButtonBehavior(checkbox_bounds(idx), check_id, &hovered, &held);

			return std::make_pair(pressed, hovered);
		};

		auto render_checkbox = [&](float hovered, float active, int idx)
		{
			auto bounds = checkbox_bounds(idx);

			// background
			auto checkbox_alpha = (80 + (30 * hovered)) * alpha;
			color checkbox_back_clr = color(0, 0, 0).increase(50 * hovered).new_alpha(checkbox_alpha);

			draw_list->AddRectFilled(bounds.Min, bounds.Max, checkbox_back_clr.as_imcolor(), 3.f);

			// active body
			auto checkbox_active_clr = color().multiply(clr, active).new_alpha(255 * alpha * active);
			draw_list->AddRectFilled(bounds.Min, bounds.Max, checkbox_active_clr.as_imcolor(), 3.f);

			// checkmark
			auto checkmark_clr = color().new_alpha(255 * alpha * active);
			draw_list->AddLine(ImVec2(bounds.Min.x + 3.f, bounds.Min.y + 8.f), ImVec2(bounds.Min.x + 6.f, bounds.Min.y + 11.f), checkmark_clr.as_imcolor(), 1.5f);

			draw_list->AddLine(ImVec2(bounds.Min.x + 6.f, bounds.Min.y + 11.f), ImVec2(bounds.Min.x + 12.f, bounds.Min.y + 4.f), checkmark_clr.as_imcolor(), 1.5f);
		};

		auto left_active = checkbox_active(0);
		auto& left_mod = item_animations[_fnva1(std::string(conditions[i] + "0").c_str())];

		auto right_active = checkbox_active(1);
		auto& right_mod = item_animations[_fnva1(std::string(conditions[i] + "1").c_str())];

		// states saves from right to left
		// like 1 << (0 << 2) will be here:
		// 0000 0001
		// and 1 << (1 << 2) + 1 will be here
		// 0000 0010
		// credits: ekzi for explaining about it

		int left_flag = 1 << (i << 2);
		int right_flag = 1 << ((i << 2) + 1);

		if (left_active.first)
		{
			if (!(v & left_flag))
				v |= left_flag;
			else
				v &= ~left_flag;

			v &= ~right_flag;
		}
		else if (right_active.first)
		{
			if (!(v & right_flag))
				v |= right_flag;
			else
				v &= ~right_flag;

			v &= ~left_flag;
		}

		// checkbox left
		{
			this->create_animation(left_mod.hovered_alpha, left_active.second, 1.f, lerp_animation);
			this->create_animation(left_mod.alpha, v & left_flag, 0.8f, lerp_animation);
			render_checkbox(left_mod.hovered_alpha, left_mod.alpha, 0);
		}

		// checkbox right
		{
			this->create_animation(right_mod.hovered_alpha, right_active.second, 1.f, lerp_animation);
			this->create_animation(right_mod.alpha, v & right_flag, 0.8f, lerp_animation);
			render_checkbox(right_mod.hovered_alpha, right_mod.alpha, 1);
		}
	}
	return true;
}