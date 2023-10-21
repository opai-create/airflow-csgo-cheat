#include "../menu.h"
#include "../../config_vars.h"

using namespace ImGui;

static const char* patch_formant_string_float_to_int(const char* fmt)
{
	if (fmt[0] == '%' && fmt[1] == '.' && fmt[2] == '0' && fmt[3] == 'f' && fmt[4] == 0) // Fast legacy path for "%.0f" which is expected to be the most common case.
		return "%d";
	const char* fmt_start = ImParseFormatFindStart(fmt);   // Find % (if any, and ignore %%)
	const char* fmt_end = ImParseFormatFindEnd(fmt_start); // Find end of format specifier, which itself is an exercise of confidence/recklessness (because snprintf is dependent on libc or user).
	if (fmt_end > fmt_start && fmt_end[-1] == 'f')
	{
		if (fmt_start == fmt && fmt_end[0] == 0)
			return CXOR("%d");
		ImGuiContext& g = *GImGui;
		ImFormatString(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), CXOR("%.*s%%d%s"), (int)(fmt_start - fmt), fmt, fmt_end); // Honor leading and trailing decorations, but lose alignment/precision.
		return g.TempBuffer;
	}
	return fmt;
}

bool c_menu::slider_scalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, float power)
{
	float alpha = this->get_alpha();

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const float w = 256.f;

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, 56.f));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(0.f, 4.f));

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id, &frame_bb))
		return false;

	// Default format string when passing NULL
	if (format == NULL)
		format = DataTypeGetInfo(data_type)->PrintFmt;
	else if (data_type == ImGuiDataType_S32 && strcmp(format, CXOR("%d")) != 0) // (FIXME-LEGACY: Patch old "%.0f" format string to use "%d", read function more details.)
		format = patch_formant_string_float_to_int(format);

	auto& mod_frame = item_animations[CONST_HASH(label) + HASH("frame")];
	auto& mod = item_animations[CONST_HASH(label)];

	const bool hovered_frame = ItemHoverable(frame_bb, id);
	this->create_animation(mod_frame.hovered_alpha, hovered_frame, 1.f, lerp_animation);
	this->create_animation(mod_frame.alpha, true, 0.3f, skip_disable | lerp_animation);

	auto slider_start = frame_bb.Min + ImVec2(12.f, 28.f);
	auto slider_size = ImVec2(232, 24.f);
	auto slider_bounds = ImRect(slider_start, slider_start + slider_size);

	// Tabbing or CTRL-clicking on Slider turns it into an input box
	const bool hovered = ItemHoverable(slider_bounds, id);
	const bool focus_requested = FocusableItemRegister(window, id);
	const bool clicked = (hovered && g.IO.MouseClicked[0]);
	if (focus_requested || clicked || g.NavActivateId == id || g.NavInputId == id)
	{
		SetActiveID(id, window);
		SetFocusID(id, window);
		FocusWindow(window);
		g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
		if (focus_requested || g.NavInputId == id)
			FocusableItemUnregister(window);
	}

	// background
	auto back_alpha = (20 + (30 * mod_frame.hovered_alpha)) * alpha;
	c_color back_clr = c_color(217, 217, 217).increase(38 * mod_frame.hovered_alpha).new_alpha(back_alpha);
	draw_list->AddRectFilled(frame_bb.Min, frame_bb.Max, back_clr.as_imcolor(), 4.f);

	auto render_bounds = ImRect(slider_bounds.Min + ImVec2(0.f, 9.f), slider_bounds.Max - ImVec2(0.f, 9.f));

	// slider body
	draw_list->AddRectFilled(render_bounds.Min, render_bounds.Max, c_color(0, 0, 0, 80 * alpha).as_imcolor(), 3.f);

	// Slider behavior
	ImRect grab_bb;
	const bool value_changed = SliderBehavior(slider_bounds, id, data_type, p_data, p_min, p_max, format, power, ImGuiSliderFlags_IgnoreGrabCalc, &grab_bb);
	if (value_changed)
		MarkItemEdited(id);

	// on_directx grab
	if (grab_bb.Max.x > grab_bb.Min.x)
	{
		auto clr = g_cfg.misc.ui_color.base();

		auto render_grab_bb = ImRect(grab_bb.Min + ImVec2(0.f, 9.f), grab_bb.Max - ImVec2(0.f, 9.f));

		// slider colored body
		auto& gradient = ImDrawPaint().SetLinearGradient(render_bounds.Min, render_grab_bb.Max + ImVec2(0.f, 2.f), clr.new_alpha(clr.a() * alpha * 0.5f).as_imvec4(), clr.new_alpha(clr.a() * alpha).as_imvec4());

		float bb_nomove = render_grab_bb.Max.x - 20.f;
		float modifier = bb_nomove < render_bounds.Min.x ? 1.f : mod_frame.alpha;
		float bb_movepart = 20.f * modifier;
		float grab_offset = std::clamp(bb_nomove + bb_movepart, render_bounds.Min.x, render_grab_bb.Max.x);

		draw_list->AddRectFilledMultiColor(render_bounds.Min, ImVec2(grab_offset, render_grab_bb.Max.y + 2.f), gradient, 4.f);

		// slider grab circle
		float circle_radius = 8.f;
		float circle_nomove = (render_grab_bb.Min.x + ((render_grab_bb.Max.x - render_grab_bb.Min.x) / 2.f)) - 20.f;
		float circle_offset = std::clamp(circle_nomove + bb_movepart, render_bounds.Min.x, render_grab_bb.Max.x);

		auto circle_pos = ImVec2(circle_offset, render_grab_bb.Min.y + 1.f);
		draw_list->AddCircleFilled(circle_pos, circle_radius, c_color(75, 75, 75, 255 * alpha).as_imcolor());
		draw_list->AddCircleFilled(circle_pos, circle_radius - 3.f, c_color(255, 255, 255, 255 * alpha).as_imcolor());
	}

	// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
	char value_buf[64];
	const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);

	auto size = CalcTextSize(value_buf);

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, alpha));
	RenderTextClipped(frame_bb.Max - ImVec2(size.x + 23.f, 79.f), frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));
	ImGui::PopStyleColor();

	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.5f * alpha));
	if (label_size.x > 0.0f)
		RenderText(ImVec2(frame_bb.Min.x + 12.f, frame_bb.Min.y + 9.f), label);
	ImGui::PopStyleColor();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
	return value_changed;
}

bool c_menu::slider_int(const char* label, int* v, int v_min, int v_max, const char* format)
{
	return this->slider_scalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format);
}