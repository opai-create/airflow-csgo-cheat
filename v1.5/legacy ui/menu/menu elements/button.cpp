#include "../menu.h"
using namespace ImGui;

bool c_menu::button_wrapper(const char* label, const ImVec2& size_arg, ImGuiButtonFlags flags, bool* hovered_ptr, int button_flags)
{
	bool wait = button_flags & button_wait;
	bool danger = !(button_flags & button_danger);

	float alpha = this->get_alpha();
	float alpha_add = 0.5f + 0.5f * (!wait);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f * danger, 1.f * danger, alpha_add * alpha));

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
	{
		ImGui::PopStyleColor();
		return false;
	}

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = size_arg;

	const ImRect bb(pos, pos + size + ImVec2(0.f, 4.f));
	ItemSize(bb, style.FramePadding.y);
	if (!ItemAdd(bb, id))
	{
		ImGui::PopStyleColor();
		return false;
	}

	ImRect button_rect = ImRect(pos, pos + size_arg);

	bool hovered, held;
	bool pressed = ButtonBehavior(button_rect, id, &hovered, &held, flags);

	auto& mod = item_animations[CONST_HASH(label)];
	this->create_animation(mod.hovered_alpha, hovered, 1.f, lerp_animation);

	// background
	auto back_alpha = (20 + (30 * mod.hovered_alpha) + (50 * (!wait && hovered && held))) * alpha;
	c_color back_clr = c_color(217, 217, 217).increase(38 * mod.hovered_alpha).new_alpha(back_alpha);
	draw_list->AddRectFilled(button_rect.Min, button_rect.Max, back_clr.as_imcolor(), 4.f);

	// button name
	RenderTextClipped(button_rect.Min, button_rect.Max, label, NULL, &label_size, style.ButtonTextAlign, &bb);

	if (hovered_ptr)
		*hovered_ptr = hovered;

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);

	ImGui::PopStyleColor();
	return pressed;
}

void c_menu::button(const char* label, void (*callback)(), int flags)
{
	auto button_ = this->button_wrapper(label, ImVec2(256, 32), 0, nullptr, flags);
	if (button_ && !(flags & button_wait))
		std::thread(callback).detach();
}