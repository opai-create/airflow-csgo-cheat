#include "../menu.h"
using namespace ImGui;

void to_clipboard(const char* text)
{
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		char* clip_data = (char*)(GlobalAlloc(GMEM_FIXED, MAX_PATH));
		lstrcpy(clip_data, text);
		SetClipboardData(CF_TEXT, (HANDLE)(clip_data));
		LCID* lcid = (DWORD*)(GlobalAlloc(GMEM_FIXED, sizeof(DWORD)));
		*lcid = MAKELCID(MAKELANGID(LANG_RUSSIAN, SUBLANG_NEUTRAL), SORT_DEFAULT);
		SetClipboardData(CF_LOCALE, (HANDLE)(lcid));
		CloseClipboard();
	}
}

std::string from_clipboard()
{
	std::string fromClipboard = "";
	if (OpenClipboard(0))
	{
		HANDLE hData = GetClipboardData(CF_TEXT);
		char* chBuffer = (char*)GlobalLock(hData);
		fromClipboard = chBuffer;
		GlobalUnlock(hData);
		CloseClipboard();
	}
	return fromClipboard;
}

void RenderColorRectWithAlphaCheckerboard_Wrapper(ImVec2 p_min, ImVec2 p_max, color clr, float grid_step, ImVec2 grid_off, float rounding, int rounding_corners_flags, float alpha_mod = 1.f)
{
	ImGuiWindow* window = GetCurrentWindow();
	color temp_col_bg1 = color(204, 204, 204).multiply(clr, clr.a() / 255.f).new_alpha(255 * alpha_mod);
	color temp_col_bg2 = color(128, 128, 128).multiply(clr, clr.a() / 255.f).new_alpha(255 * alpha_mod);

	ImU32 col_bg1 = GetColorU32(ImVec4(temp_col_bg1.r() / 255.f, temp_col_bg1.g() / 255.f, temp_col_bg1.b() / 255.f, temp_col_bg1.a() / 255.f));
	ImU32 col_bg2 = GetColorU32(ImVec4(temp_col_bg2.r() / 255.f, temp_col_bg2.g() / 255.f, temp_col_bg2.b() / 255.f, temp_col_bg2.a() / 255.f));
	window->DrawList->AddRectFilled(p_min, p_max, col_bg1, rounding, rounding_corners_flags);

	int yi = 0;
	for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
	{
		float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
		if (y2 <= y1)
			continue;
		for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
		{
			float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
			if (x2 <= x1)
				continue;
			int rounding_corners_flags_cell = 0;
			if (y1 <= p_min.y)
			{
				if (x1 <= p_min.x)
					rounding_corners_flags_cell |= ImDrawCornerFlags_TopLeft;
				if (x2 >= p_max.x)
					rounding_corners_flags_cell |= ImDrawCornerFlags_TopRight;
			}
			if (y2 >= p_max.y)
			{
				if (x1 <= p_min.x)
					rounding_corners_flags_cell |= ImDrawCornerFlags_BotLeft;
				if (x2 >= p_max.x)
					rounding_corners_flags_cell |= ImDrawCornerFlags_BotRight;
			}
			rounding_corners_flags_cell &= rounding_corners_flags;
			window->DrawList->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col_bg2, rounding_corners_flags_cell ? rounding : 0.0f, rounding_corners_flags_cell);
		}
	}
}

bool c_menu::color_picker_wrapper(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col)
{
	auto window_alpha = this->get_alpha();

	auto& picker_mod = item_animations[_fnva1(label)];
	float picker_alpha = picker_mod.alpha;

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImDrawList* draw_list = window->DrawList;
	ImGuiStyle& style = g.Style;
	ImGuiIO& io = g.IO;

	const float width = 152.f;
	g.NextItemData.ClearFlags();

	if (!(flags & ImGuiColorEditFlags_NoSidePreview))
		flags |= ImGuiColorEditFlags_NoSmallPreview;

	// Context menu: display and store options.
	if (!(flags & ImGuiColorEditFlags_NoOptions))
		ColorPickerOptionsPopup(col, flags);

	// Read stored options
	if (!(flags & ImGuiColorEditFlags__PickerMask))
		flags |= ((g.ColorEditOptions & ImGuiColorEditFlags__PickerMask) ? g.ColorEditOptions : ImGuiColorEditFlags__OptionsDefault) & ImGuiColorEditFlags__PickerMask;
	if (!(flags & ImGuiColorEditFlags__InputMask))
		flags |= ((g.ColorEditOptions & ImGuiColorEditFlags__InputMask) ? g.ColorEditOptions : ImGuiColorEditFlags__OptionsDefault) & ImGuiColorEditFlags__InputMask;
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__PickerMask)); // Check that only 1 is selected
	IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags__InputMask));  // Check that only 1 is selected
	if (!(flags & ImGuiColorEditFlags_NoOptions))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags_AlphaBar);

	// setup
	int components = (flags & ImGuiColorEditFlags_NoAlpha) ? 3 : 4;
	bool alpha_bar = (flags & ImGuiColorEditFlags_AlphaBar) && !(flags & ImGuiColorEditFlags_NoAlpha);
	ImVec2 picker_pos = window->DC.CursorPos + ImVec2(4.f, 1.f);
	float square_sz = 12.f;
	float bars_width = square_sz;                                                                                              // Arbitrary smallish width of Hue/Alpha picking bars
	float sv_picker_size = ImMax(bars_width * 1, width - (alpha_bar ? 2 : 1) * (bars_width + style.ItemInnerSpacing.x)); // Saturation/Value picking box
	float bar0_pos_x = picker_pos.x + sv_picker_size + style.ItemInnerSpacing.x;
	float bar1_pos_x = bar0_pos_x + bars_width + style.ItemInnerSpacing.x;
	float bars_triangles_half_sz = IM_FLOOR(bars_width * 0.20f);

	float backup_initial_col[4];
	std::memcpy(backup_initial_col, col, components * sizeof(float));

	float wheel_thickness = sv_picker_size * 0.08f;
	float wheel_r_outer = sv_picker_size * 0.50f;
	float wheel_r_inner = wheel_r_outer - wheel_thickness;
	ImVec2 wheel_center(picker_pos.x + (sv_picker_size + bars_width) * 0.5f, picker_pos.y + sv_picker_size * 0.5f);

	// Note: the triangle is displayed rotated with triangle_pa pointing to Hue, but most coordinates stays unrotated for logic.
	float triangle_r = wheel_r_inner - (int)(sv_picker_size * 0.027f);
	ImVec2 triangle_pa = ImVec2(triangle_r, 0.0f);                            // Hue point.
	ImVec2 triangle_pb = ImVec2(triangle_r * -0.5f, triangle_r * -0.866025f); // Black point.
	ImVec2 triangle_pc = ImVec2(triangle_r * -0.5f, triangle_r * +0.866025f); // White point.

	float H = col[0], S = col[1], V = col[2];
	float R = col[0], G = col[1], B = col[2];
	if (flags & ImGuiColorEditFlags_InputRGB)
	{
		// Hue is lost when converting from greyscale rgb (saturation=0). Restore it.
		ColorConvertRGBtoHSV(R, G, B, H, S, V);
		if (S == 0 && memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) == 0)
			H = g.ColorEditLastHue;
	}
	else if (flags & ImGuiColorEditFlags_InputHSV)
	{
		ColorConvertHSVtoRGB(H, S, V, R, G, B);
	}

	bool value_changed = false, value_changed_h = false, value_changed_sv = false;

	PushID(label);
	BeginGroup();
	{
		PushItemFlag(ImGuiItemFlags_NoNav, true);
		if (flags & ImGuiColorEditFlags_PickerHueBar)
		{
			// SV rectangle logic
			InvisibleButton("sv", ImVec2(sv_picker_size, sv_picker_size));
			if (IsItemActive())
			{
				S = ImSaturate((io.MousePos.x - picker_pos.x) / (sv_picker_size - 1));
				V = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
				value_changed = value_changed_sv = true;
			}
			if (!(flags & ImGuiColorEditFlags_NoOptions))
				OpenPopupOnItemClick("context");

			// Hue bar logic
			SetCursorScreenPos(ImVec2(bar0_pos_x, picker_pos.y));
			InvisibleButton("hue", ImVec2(bars_width, sv_picker_size));
			if (IsItemActive())
			{
				H = ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
				value_changed = value_changed_h = true;
			}
		}

		// Alpha bar logic
		if (alpha_bar)
		{
			SetCursorScreenPos(ImVec2(bar1_pos_x, picker_pos.y));
			InvisibleButton("alpha", ImVec2(bars_width, sv_picker_size));
			if (IsItemActive())
			{
				col[3] = 1.0f - ImSaturate((io.MousePos.y - picker_pos.y) / (sv_picker_size - 1));
				value_changed = true;
			}
		}
		PopItemFlag(); // ImGuiItemFlags_NoNav

		if (!(flags & ImGuiColorEditFlags_NoSidePreview))
		{
			SameLine(0, style.ItemInnerSpacing.x);
			BeginGroup();
		}

		if (!(flags & ImGuiColorEditFlags_NoLabel))
		{
			const char* label_display_end = FindRenderedTextEnd(label);
			if (label != label_display_end)
			{
				if ((flags & ImGuiColorEditFlags_NoSidePreview))
					SameLine(0, style.ItemInnerSpacing.x);
				TextEx(label, label_display_end);
			}
		}

		// Convert back color to RGB
		if (value_changed_h || value_changed_sv)
		{
			if (flags & ImGuiColorEditFlags_InputRGB)
			{
				ColorConvertHSVtoRGB(H >= 1.0f ? H - 10 * 1e-6f : H, S > 0.0f ? S : 10 * 1e-6f, V > 0.0f ? V : 1e-6f, col[0], col[1], col[2]);
				g.ColorEditLastHue = H;
				std::memcpy(g.ColorEditLastColor, col, sizeof(float) * 3);
			}
			else if (flags & ImGuiColorEditFlags_InputHSV)
			{
				col[0] = H;
				col[1] = S;
				col[2] = V;
			}
		}

		// R,G,B and H,S,V slider color editor
		bool value_changed_fix_hue_wrap = false;
		if ((flags & ImGuiColorEditFlags_NoInputs) == 0)
		{
			PushItemWidth((alpha_bar ? bar1_pos_x : bar0_pos_x) + bars_width - picker_pos.x);
			ImGuiColorEditFlags sub_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoOptions |
				ImGuiColorEditFlags_NoSmallPreview | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf;
			ImGuiColorEditFlags sub_flags = (flags & sub_flags_to_forward) | ImGuiColorEditFlags_NoPicker;
			if (flags & ImGuiColorEditFlags_DisplayRGB || (flags & ImGuiColorEditFlags__DisplayMask) == 0)
				if (ColorEdit4("##rgb", col, sub_flags | ImGuiColorEditFlags_DisplayRGB))
				{
					// FIXME: Hackily differenciating using the DragInt (ActiveId != 0 && !ActiveIdAllowOverlap) vs. using the InputText or DropTarget.
					// For the later we don't want to run the hue-wrap canceling code. If you are well versed in HSV picker please provide your input! (See #2050)
					value_changed_fix_hue_wrap = (g.ActiveId != 0 && !g.ActiveIdAllowOverlap);
					value_changed = true;
				}
			if (flags & ImGuiColorEditFlags_DisplayHSV || (flags & ImGuiColorEditFlags__DisplayMask) == 0)
				value_changed |= ColorEdit4("##hsv", col, sub_flags | ImGuiColorEditFlags_DisplayHSV);
			if (flags & ImGuiColorEditFlags_DisplayHex || (flags & ImGuiColorEditFlags__DisplayMask) == 0)
				value_changed |= ColorEdit4("##hex", col, sub_flags | ImGuiColorEditFlags_DisplayHex);
			PopItemWidth();
		}

		// Try to cancel hue wrap (after ColorEdit4 call), if any
		if (value_changed_fix_hue_wrap && (flags & ImGuiColorEditFlags_InputRGB))
		{
			float new_H, new_S, new_V;
			ColorConvertRGBtoHSV(col[0], col[1], col[2], new_H, new_S, new_V);
			if (new_H <= 0 && H > 0)
			{
				if (new_V <= 0 && V != new_V)
					ColorConvertHSVtoRGB(H, S, new_V <= 0 ? V * 0.5f : new_V, col[0], col[1], col[2]);
				else if (new_S <= 0)
					ColorConvertHSVtoRGB(H, new_S <= 0 ? S * 0.5f : new_S, new_V, col[0], col[1], col[2]);
			}
		}

		if (value_changed)
		{
			if (flags & ImGuiColorEditFlags_InputRGB)
			{
				R = col[0];
				G = col[1];
				B = col[2];
				ColorConvertRGBtoHSV(R, G, B, H, S, V);
				if (S == 0 && memcmp(g.ColorEditLastColor, col, sizeof(float) * 3) == 0) // Fix local Hue as display below will use it immediately.
					H = g.ColorEditLastHue;
			}
			else if (flags & ImGuiColorEditFlags_InputHSV)
			{
				H = col[0];
				S = col[1];
				V = col[2];
				ColorConvertHSVtoRGB(H, S, V, R, G, B);
			}
		}

		const int style_alpha8 = IM_F32_TO_INT8_SAT(style.Alpha * window_alpha * picker_alpha);
		const ImU32 col_black = IM_COL32(0, 0, 0, style_alpha8);
		const ImU32 col_white = IM_COL32(255, 255, 255, style_alpha8);
		const ImU32 col_midgrey = IM_COL32(128, 128, 128, style_alpha8);
		const ImU32 col_hues[6 + 1] = { IM_COL32(255, 0, 0, style_alpha8), IM_COL32(255, 255, 0, style_alpha8), IM_COL32(0, 255, 0, style_alpha8), IM_COL32(0, 255, 255, style_alpha8), IM_COL32(0, 0, 255, style_alpha8),
		  IM_COL32(255, 0, 255, style_alpha8), IM_COL32(255, 0, 0, style_alpha8) };

		ImVec4 hue_color_f(1, 1, 1, style.Alpha * window_alpha * picker_alpha);
		ColorConvertHSVtoRGB(H, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
		ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);
		ImU32 user_col32_striped_of_alpha = ColorConvertFloat4ToU32(ImVec4(R, G, B, style.Alpha * window_alpha * picker_alpha)); // Important: this is still including the main rendering/style alpha!!

		ImVec2 sv_cursor_pos;

		if (flags & ImGuiColorEditFlags_PickerHueBar)
		{
			// on_directx SV Square
			draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), col_white, hue_color32, hue_color32, col_white);
			draw_list->AddRectFilledMultiColor(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0, 0, col_black, col_black);
			RenderFrameBorder(picker_pos, picker_pos + ImVec2(sv_picker_size, sv_picker_size), 0.0f);
			sv_cursor_pos.x = ImClamp(IM_ROUND(picker_pos.x + ImSaturate(S) * sv_picker_size), picker_pos.x + 2, picker_pos.x + sv_picker_size - 2); // Sneakily prevent the circle to stick out too much
			sv_cursor_pos.y = ImClamp(IM_ROUND(picker_pos.y + ImSaturate(1 - V) * sv_picker_size), picker_pos.y + 2, picker_pos.y + sv_picker_size - 2);

			// on_directx Hue Bar
			for (int i = 0; i < 6; ++i)
				draw_list->AddRectFilledMultiColor(ImVec2(bar0_pos_x, picker_pos.y + i * (sv_picker_size / 6)), ImVec2(bar0_pos_x + bars_width, picker_pos.y + (i + 1) * (sv_picker_size / 6)), col_hues[i], col_hues[i],
					col_hues[i + 1], col_hues[i + 1]);
			float bar0_line_y = IM_ROUND(picker_pos.y + H * sv_picker_size);
			RenderFrameBorder(ImVec2(bar0_pos_x, picker_pos.y), ImVec2(bar0_pos_x + bars_width, picker_pos.y + sv_picker_size), 0.0f);

			render_arrows_for_vertical_bar(draw_list, ImVec2(bar0_pos_x - 1, bar0_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f, style.Alpha * window_alpha * picker_alpha);
		}

		// on_directx cursor/preview circle (clamp S/V within 0..1 range because floating points colors may lead HSV values to be out of range)
		float sv_cursor_rad = 3.0f;
		draw_list->AddCircleFilled(sv_cursor_pos, sv_cursor_rad, col_white, 15);
		draw_list->AddCircle(sv_cursor_pos, sv_cursor_rad + 1.f, col_black, 15);

		// on_directx alpha bar
		if (alpha_bar)
		{
			float alpha = ImSaturate(col[3]);
			ImRect bar1_bb(bar1_pos_x, picker_pos.y, bar1_pos_x + bars_width, picker_pos.y + sv_picker_size);
			draw_list->AddRectFilledMultiColor(bar1_bb.Min, bar1_bb.Max, user_col32_striped_of_alpha, user_col32_striped_of_alpha, user_col32_striped_of_alpha & ~IM_COL32_A_MASK, user_col32_striped_of_alpha & ~IM_COL32_A_MASK);
			float bar1_line_y = IM_ROUND(picker_pos.y + (1.0f - alpha) * sv_picker_size);
			RenderFrameBorder(bar1_bb.Min, bar1_bb.Max, 0.0f);
			render_arrows_for_vertical_bar(draw_list, ImVec2(bar1_pos_x - 1, bar1_line_y), ImVec2(bars_triangles_half_sz + 1, bars_triangles_half_sz), bars_width + 2.0f, style.Alpha * window_alpha * picker_alpha);
		}
	}
	EndGroup();

	BeginGroup();
	{

		auto& pos = window->Pos;
		auto& size = window->Size;

		auto old_flags = draw_list->Flags;

		draw_list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

		auto button_size = ImVec2(76, 24);
		BeginGroup();
		{
			auto& copy_mod = item_animations[_fnva1(label) + __fnva1("Copy")];
			bool copy_hover = false;
			auto copy_button = ImGui::ButtonEx(xor_c("##copybutton"), ImVec2(76, 24), 0, &copy_hover);

			this->create_animation(copy_mod.hovered_alpha, copy_hover, 1.f, lerp_animation);

			auto button_clr_alpha = (80 + 30 * copy_mod.hovered_alpha) * this->get_alpha();
			color button_clr = color(35, 35, 35).increase(20 * copy_mod.hovered_alpha).new_alpha(button_clr_alpha);

			draw_list->AddRectFilled(ImVec2(pos.x + 11.f, (pos.y + size.y) - button_size.y - 6.f), ImVec2(pos.x + 9.f + button_size.x, (pos.y + size.y) - 6.f), button_clr.as_imcolor(), 2.f);

			draw_list->AddText(ImVec2(pos.x + button_size.x / 2 - 6.f, (pos.y + size.y) - button_size.y - 3.f), color(255, 255, 255, 255 * picker_alpha).as_imcolor(), xor_c("Copy"));

			if (copy_button)
			{
				std::string new_str = std::to_string(color(col[0] * 255.f, col[1] * 255.f, col[2] * 255.f, col[3] * 255.f).u32());
				to_clipboard(new_str.c_str());

				ImGui::CloseCurrentPopup();
			}
		}
		EndGroup();

		ImGui::SameLine();

		BeginGroup();
		{
			auto& paste_mod = item_animations[_fnva1(label) + __fnva1("Paste")];
			bool paste_hover = false;
			auto paste_button = ImGui::ButtonEx(xor_c("##pastebutton"), ImVec2(76, 24), 0, &paste_hover);

			this->create_animation(paste_mod.hovered_alpha, paste_hover, 1.f, lerp_animation);

			auto button_clr_alpha = (80 + 30 * paste_mod.hovered_alpha) * this->get_alpha();
			color button_clr = color(35, 35, 35).increase(20 * paste_mod.hovered_alpha).new_alpha(button_clr_alpha);

			auto base_pos = pos.x + button_size.x + 5.f;

			draw_list->AddRectFilled(ImVec2(base_pos + 11.f, (pos.y + size.y) - button_size.y - 6.f), ImVec2(base_pos + 9.f + button_size.x, (pos.y + size.y) - 6.f), button_clr.as_imcolor(), 2.f);

			draw_list->AddText(ImVec2(base_pos + button_size.x / 2 - 6.f, (pos.y + size.y) - button_size.y - 3.f), color(255, 255, 255, 255 * picker_alpha).as_imcolor(), xor_c("Paste"));

			if (paste_button)
			{
				auto clip_clr = color(std::atoll(from_clipboard().c_str()));
				auto temp_clr = color::hsb(clip_clr.hue(), clip_clr.saturation(), clip_clr.brightness()).new_alpha(clip_clr.a());

				col[0] = temp_clr.r() / 255.f;
				col[1] = temp_clr.g() / 255.f;
				col[2] = temp_clr.b() / 255.f;
				col[3] = temp_clr.a() / 255.f;

				ImGui::CloseCurrentPopup();
			}
		}
		EndGroup();

		draw_list->Flags = old_flags;
	}
	EndGroup();

	if (value_changed && memcmp(backup_initial_col, col, components * sizeof(float)) == 0)
		value_changed = false;
	if (value_changed)
		MarkItemEdited(window->DC.LastItemId);

	PopID();

	return value_changed;
}

bool c_menu::color_button_wrapper(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags)
{
	float alpha = this->get_alpha();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.5f * alpha));

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiID id = window->GetID(desc_id);
	float default_size = GetFrameHeight();

	const auto picker_size = ImVec2(32, 16);

	const auto pos = window->DC.CursorPos;
	auto back_size = ImVec2(256, 48);
	const ImRect bb(pos + ImVec2(209.f, 15.f), pos + ImVec2(209.f, 15.f) + picker_size);
	const ImRect render_bb(pos, pos + back_size + ImVec2(0.f, 4.f));
	ItemSize(render_bb);
	if (!ItemAdd(render_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);

	bool hovered_frame, held_frame;
	bool pressed_frame = ButtonBehavior(ImRect(pos, pos + back_size), id, &hovered_frame, &held_frame);

	auto& mod = item_animations[_fnva1(desc_id) + _fnva1("color")];
	this->create_animation(mod.hovered_alpha, hovered_frame, 1.f, lerp_animation);

	// background
	auto back_alpha = (20 + (30 * mod.hovered_alpha)) * alpha;
	color back_clr = color(217, 217, 217).increase(38 * mod.hovered_alpha).new_alpha(back_alpha);
	draw_list->AddRectFilled(pos, pos + back_size, back_clr.as_imcolor(), 4.f);

	if (flags & ImGuiColorEditFlags_NoAlpha)
		flags &= ~(ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_AlphaPreviewHalf);

	ImVec4 col_rgb = ImVec4(col.x, col.y, col.z, col.w * alpha);
	if (flags & ImGuiColorEditFlags_InputHSV)
		ColorConvertHSVtoRGB(col_rgb.x, col_rgb.y, col_rgb.z, col_rgb.x, col_rgb.y, col_rgb.z);

	ImVec4 col_rgb_without_alpha(col_rgb.x, col_rgb.y, col_rgb.z, alpha);
	float grid_step = ImMin(picker_size.x, picker_size.y) / 2.99f;
	float rounding = 4.f;
	ImRect bb_inner = bb;
	float off = -0.75f; // The border (using Col_FrameBg) tends to look off when color is near-opaque and rounding is enabled. This offset seemed like a good middle ground to reduce those artifacts.
	bb_inner.Expand(off);
	if ((flags & ImGuiColorEditFlags_AlphaPreviewHalf) && col_rgb.w < 1.0f)
	{
		float mid_x = IM_ROUND((bb_inner.Min.x + bb_inner.Max.x) * 0.5f);
		RenderColorRectWithAlphaCheckerboard_Wrapper(ImVec2(bb_inner.Min.x + grid_step, bb_inner.Min.y), bb_inner.Max, color(col_rgb.x * 255, col_rgb.y * 255, col_rgb.z * 255, col_rgb.w * 255), grid_step,
			ImVec2(-grid_step + off, off), rounding, ImDrawCornerFlags_All, alpha);

		window->DrawList->AddRectFilled(bb_inner.Min, ImVec2(mid_x, bb_inner.Max.y), GetColorU32(col_rgb_without_alpha), rounding, ImDrawCornerFlags_All);
	}
	else
	{
		// Because GetColorU32() multiplies by the global style Alpha and we don't want to display a checkerboard if the source code had no alpha
		ImVec4 col_source = (flags & ImGuiColorEditFlags_AlphaPreview) ? col_rgb : col_rgb_without_alpha;
		if (col_source.w < 1.0f)
			RenderColorRectWithAlphaCheckerboard_Wrapper(
				bb_inner.Min, bb_inner.Max, color(col_source.x * 255, col_source.y * 255, col_source.z * 255, col_source.w * 255), grid_step, ImVec2(off, off), rounding, ImDrawCornerFlags_All, alpha);
		else
			window->DrawList->AddRectFilled(bb_inner.Min, bb_inner.Max, GetColorU32(col_source), rounding, ImDrawCornerFlags_All);
	}

	RenderText(ImVec2(pos.x + 12, pos.y + 16), desc_id);

	RenderNavHighlight(bb, id);

	ImGui::PopStyleColor();
	return pressed;
}

bool c_menu::color_picker(const char* name, c_float_color& v, ImGuiColorEditFlags flags)
{
	float window_alpha = this->get_alpha();
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const float w_extra = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (g.FontSize + g.Style.FramePadding.y * 2.0f + style.ItemInnerSpacing.x);
	const float w_items_all = CalcItemWidth() - w_extra;
	const char* label_display_end = FindRenderedTextEnd(name);

	const bool alpha = (flags & ImGuiColorEditFlags_NoAlpha) == 0;
	const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
	const int components = alpha ? 4 : 3;
	const ImGuiColorEditFlags flags_untouched = flags;

	BeginGroup();
	PushID(name);

	// If we're not showing any slider there's no point in doing any HSV conversions
	if (flags & ImGuiColorEditFlags_NoInputs)
		flags = (flags & (~ImGuiColorEditFlags__InputMask)) | ImGuiColorEditFlags_RGB | ImGuiColorEditFlags_NoOptions;

	// Read stored options
	if (!(flags & ImGuiColorEditFlags__InputMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__InputMask);
	if (!(flags & ImGuiColorEditFlags__DataTypeMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__DataTypeMask);
	if (!(flags & ImGuiColorEditFlags__PickerMask))
		flags |= (g.ColorEditOptions & ImGuiColorEditFlags__PickerMask);
	flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask));

	// Convert to the formats we need
	float f[4] = {
	  v[0],
	  v[1],
	  v[2],
	  alpha ? v[3] / 255.0f : 1.0f,
	};

	if (flags & ImGuiColorEditFlags_HSV)
		ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
	int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

	bool value_changed = false;
	bool value_changed_as_float = false;

	auto current_list = ImGui::GetWindowDrawList();
	current_list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;

	bool picker_active = false;
	if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
	{
		if (!(flags & ImGuiColorEditFlags_NoInputs))
			SameLine(0, style.ItemInnerSpacing.x);

		const ImVec4 col_v4(v[0], v[1], v[2], alpha ? v[3] : 1.0f);

		auto button_str = name + xor_str("##color");
		auto enabled_picker = this->color_button_wrapper(button_str.c_str(), col_v4, flags);

		SetNextWindowBgAlpha(0.f);
		if (enabled_picker)
		{
			if (!(flags & ImGuiColorEditFlags_NoPicker))
			{
				// store current color and open a picker
				g.ColorPickerRef = col_v4;
				OpenPopup(xor_c("picker"));

				SetNextWindowPos(window->DC.LastItemRect.GetBL() - ImVec2(-35.f, 38.f));
			}
		}

		if (!(flags & ImGuiColorEditFlags_NoOptions) && ImGui::IsItemHovered() && IsMouseClicked(1))
			OpenPopup(xor_c("context"));

		bool begin_popup = BeginPopupEx(g.CurrentWindow->GetID(xor_c("picker")), ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

		auto picker_name = name + xor_str("picker");
		auto& picker_mod = item_animations[_fnva1(picker_name.c_str())];
		this->create_animation(picker_mod.alpha, begin_popup, 0.3f, skip_disable | lerp_animation);

		auto base_pos = ImGui::GetWindowPos();
		ImGui::GetWindowDrawList()->AddRectFilled(base_pos, base_pos + ImGui::GetWindowSize(), color(16, 16, 16, 255 * window_alpha * picker_mod.alpha).as_imcolor(), 10.f);

		if (begin_popup)
		{

			picker_active = true;
			ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags__DataTypeMask | ImGuiColorEditFlags__PickerMask | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
			ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags__InputMask | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
			PushItemWidth(176.f); // Use 256 + bar sizes?
			value_changed |= this->color_picker_wrapper(picker_name.c_str(), v.float_base(), picker_flags, &g.ColorPickerRef.x);
			PopItemWidth();
			EndPopup();
		}
	}

	current_list->Flags &= ~(ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines);

	// Convert back
	if (!picker_active)
	{
		if (!value_changed_as_float)
			for (int n = 0; n < 4; n++)
				f[n] = i[n] / 255.0f;
		if (flags & ImGuiColorEditFlags_HSV)
			ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
		if (value_changed)
		{
			v[0] = f[0];
			v[1] = f[1];
			v[2] = f[2];
			if (alpha)
				v[3] = f[3];
		}
	}

	PopID();
	EndGroup();

	return value_changed;
}