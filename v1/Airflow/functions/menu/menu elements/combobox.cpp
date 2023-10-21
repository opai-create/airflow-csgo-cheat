#include "../menu.h"
using namespace ImGui;

bool c_menu::selectable(const char* label, bool selected, float alpha_pass, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
	float alpha = this->get_alpha();

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;

	if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns) // FIXME-OPT: Avoid if vertically clipped.
		PushColumnsBackground();

	ImGuiID id = window->GetID(label);
	ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImVec2 size(size_arg.x != 0.0f ? size_arg.x : label_size.x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
	ImVec2 pos = window->DC.CursorPos;
	// pos.y += window->DC.CurrLineTextBaseOffset;
	ImRect bb_inner(pos, pos + size);
	ItemSize(size, 0.0f);

	// Fill horizontal space.
	ImVec2 window_padding = window->WindowPadding;
	float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
	float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - pos.x);
	ImVec2 size_draw((size_arg.x != 0 && !(flags & ImGuiSelectableFlags_DrawFillAvailWidth)) ? size_arg.x : w_draw, size_arg.y != 0.0f ? size_arg.y : size.y);
	ImRect bb(pos, pos + size_draw);
	if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_DrawFillAvailWidth))
		bb.Max.x += window_padding.x;

	// Selectables are tightly packed together so we extend the box to cover spacing between selectable.
	// const float spacing_x = style.ItemSpacing.x;
	// const float spacing_y = style.ItemSpacing.y;
	// const float spacing_L = IM_FLOOR(spacing_x * 0.50f);
	// const float spacing_U = IM_FLOOR(spacing_y * 0.50f);
	// bb.Min.x -= spacing_L;
	// bb.Min.y -= spacing_U;
	// bb.Max.x += (spacing_x - spacing_L);
	// bb.Max.y += (spacing_y - spacing_U);

	bool item_add;
	if (flags & ImGuiSelectableFlags_Disabled)
	{
		ImGuiItemFlags backup_item_flags = window->DC.ItemFlags;
		window->DC.ItemFlags |= ImGuiItemFlags_Disabled | ImGuiItemFlags_NoNavDefaultFocus;
		item_add = ItemAdd(bb, id);
		window->DC.ItemFlags = backup_item_flags;
	}
	else
	{
		item_add = ItemAdd(bb, id);
	}
	if (!item_add)
	{
		if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
			PopColumnsBackground();
		return false;
	}

	// We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
	ImGuiButtonFlags button_flags = 0;
	if (flags & ImGuiSelectableFlags_NoHoldingActiveID)
		button_flags |= ImGuiButtonFlags_NoHoldingActiveID;
	if (flags & ImGuiSelectableFlags_PressedOnClick)
		button_flags |= ImGuiButtonFlags_PressedOnClick;
	if (flags & ImGuiSelectableFlags_PressedOnRelease)
		button_flags |= ImGuiButtonFlags_PressedOnRelease;
	if (flags & ImGuiSelectableFlags_Disabled)
		button_flags |= ImGuiButtonFlags_Disabled;
	if (flags & ImGuiSelectableFlags_AllowDoubleClick)
		button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
	if (flags & ImGuiSelectableFlags_AllowItemOverlap)
		button_flags |= ImGuiButtonFlags_AllowItemOverlap;

	if (flags & ImGuiSelectableFlags_Disabled)
		selected = false;

	const bool was_selected = selected;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

	auto& mod = item_animations[_fnva1(label) + _fnva1("_sel")];

	this->create_animation(mod.hovered_alpha, hovered, 1.f, lerp_animation);
	this->create_animation(mod.alpha, pressed, 0.6f, lerp_animation);

	// Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with gamepad/keyboard
	if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
	{
		if (!g.NavDisableMouseHover && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
		{
			g.NavDisableHighlight = true;
			SetNavID(id, window->DC.NavLayerCurrent);
		}
	}
	if (pressed)
		MarkItemEdited(id);

	if (flags & ImGuiSelectableFlags_AllowItemOverlap)
		SetItemAllowOverlap();

	// In this branch, Selectable() cannot toggle the selection so this will never trigger.
	if (selected != was_selected) //-V547
		window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

	// on_directx
	if (held && (flags & ImGuiSelectableFlags_DrawHoveredWhenHeld))
		hovered = true;

	if ((flags & ImGuiSelectableFlags_SpanAllColumns) && window->DC.CurrentColumns)
	{
		PopColumnsBackground();
		bb.Max.x -= (GetContentRegionMax().x - max_x);
	}

	auto text_clr = selected ? 1.f : 0.58f + 0.42f * mod.hovered_alpha;
	PushStyleColor(ImGuiCol_Text, ImVec4(text_clr, text_clr, text_clr, text_clr * alpha * alpha_pass));
	RenderTextClipped(bb_inner.Min, bb_inner.Max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
	PopStyleColor();

	// Automatically close popups
	if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_DontClosePopups) && !(window->DC.ItemFlags & ImGuiItemFlags_SelectableDontClosePopup))
		CloseCurrentPopup();

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
	return pressed;
}

bool c_menu::begin_combo(const char* label, const char* preview_value, ImGuiComboFlags flags, int item_cnt)
{
	float alpha = this->get_alpha();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 0.5f * alpha));

	// Always consume the SetNextWindowSizeConstraint() call in our early return paths
	ImGuiContext& g = *GImGui;
	bool has_window_size_constraint = (g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint) != 0;
	g.NextWindowData.Flags &= ~ImGuiNextWindowDataFlags_HasSizeConstraint;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	const float w = 256.f;

	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(w, 48.f));
	const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(0.f, 4.f));

	// const ImVec2 pos = window->DC.CursorPos;
	// const ImRect total_bb(pos, pos + ImVec2(256.f, 48.f));

	ItemSize(total_bb, style.FramePadding.y);
	if (!ItemAdd(total_bb, id, &frame_bb))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(frame_bb, id, &hovered, &held);
	bool popup_open = IsPopupOpen(id);

	auto& mod = item_animations[_fnva1(label)];

	this->create_animation(mod.hovered_alpha, hovered, 1.f, lerp_animation);

	auto back_alpha = (20 + (30 * mod.hovered_alpha)) * alpha;
	color back_clr = color(217, 217, 217).increase(38 * mod.hovered_alpha).new_alpha(back_alpha);
	draw_list->AddRectFilled(frame_bb.Min, frame_bb.Max, back_clr.u32(), 4.f);

	if (label_size.x > 0)
		RenderText(ImVec2(frame_bb.Min.x + 12.f, frame_bb.Min.y + 16.f), label);

	if ((pressed || g.NavActivateId == id) && !popup_open)
	{
		if (window->DC.NavLayerCurrent == 0)
			window->NavLastIds[0] = id;
		OpenPopupEx(id);
		popup_open = true;
	}

	mod.active = popup_open;

	this->create_animation(mod.alpha, popup_open, 0.5f, lerp_animation);

	auto bg_size = ImVec2(123, 31);

	if (preview_value != NULL)
	{
		auto size = CalcTextSize(preview_value);

		float cur_pos = size.x + 36;
		float out_pos = cur_pos + ((bg_size.x - cur_pos)) * mod.alpha;

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, alpha));
		RenderTextClipped(frame_bb.Max - ImVec2(out_pos, 32), frame_bb.Max, preview_value, NULL, NULL, ImVec2(0.0f, 0.0f));
		ImGui::PopStyleColor();
	}

	// combo background (upper part)
	auto combo_bg_min = frame_bb.Min + ImVec2(126, 9);
	auto combo_bg_max = combo_bg_min + bg_size;
	if (popup_open)
	{
		draw_list->AddRectFilled(combo_bg_min, combo_bg_max, color(5, 5, 5, 150 * alpha * mod.alpha).u32(), 4.f, ImDrawCornerFlags_Top);
	}

	// combo background (separator)
	if (popup_open)
		draw_list->AddLine(combo_bg_max - ImVec2(bg_size.x, 1), combo_bg_max - ImVec2(0, 1), color(255, 255, 255, 12.75f * alpha * mod.alpha).u32(), 1.f);

	// arrow
	const auto left_line_min = frame_bb.Min + ImVec2(232, 22);
	const auto left_line_max = frame_bb.Min + ImVec2(236, 26);

	draw_list->AddLine(left_line_min, left_line_max, color(255, 255, 255, 255 * alpha).u32(), 1.5f);
	draw_list->AddLine(left_line_min + ImVec2(9, -1), left_line_max, color(255, 255, 255, 255 * alpha).u32(), 1.5f);

	if (!popup_open)
		return false;

	char name[16];
	ImFormatString(name, IM_ARRAYSIZE(name), xor_c("##Combo_%02d"), g.BeginPopupStack.Size); // Recycle windows based on depth

	SetNextWindowPos(frame_bb.Min + ImVec2(126, 40));

	float max_size = std::clamp(calc_max_popup_height(item_cnt), 0.f, 200.f);

	auto items_size = ImVec2(123.f, max_size * mod.alpha);
	SetNextWindowSize(items_size);

	SetNextWindowBgAlpha(alpha * mod.alpha);

	// We don't use BeginPopupEx() solely because we have a custom name string, which we could make an argument to BeginPopupEx()
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_Popup | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;

	// Horizontally align ourselves with the framed text
	PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(7, style.WindowPadding.y));
	bool ret = Begin(name, NULL, window_flags);
	PopStyleVar();
	if (!ret)
	{
		EndPopup();
		IM_ASSERT(0); // This should never happen as we tested for IsPopupOpen() above
		return false;
	}

	ImGui::PopStyleColor();
	return true;
}

bool c_menu::combo_wrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items)
{
	ImGuiContext& g = *GImGui;

	// Call the getter to obtain the preview string which is a parameter to BeginCombo()
	const char* preview_value = NULL;
	if (*current_item >= 0 && *current_item < items_count)
		items_getter(data, *current_item, &preview_value);
	else
		preview_value = xor_c("Select");

	auto str = std::string(preview_value);
	if (str.length() > 15)
	{
		str.erase(15, str.length() - 15);
		str.append(xor_c("..."));
	}

	// The old Combo() API exposed "popup_max_height_in_items". The new more general BeginCombo() API doesn't have/need it, but we emulate it here.
	if (popup_max_height_in_items != -1 && !(g.NextWindowData.Flags & ImGuiNextWindowDataFlags_HasSizeConstraint))
		SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, calc_max_popup_height(popup_max_height_in_items)));

	if (!this->begin_combo(label, str.c_str(), ImGuiComboFlags_None, items_count))
		return false;

	auto& mod = item_animations[_fnva1(label)];

	// Display items
	// FIXME-OPT: Use clipper (but we need to disable it on the appearing frame to make sure our call to SetItemDefaultFocus() is processed)
	bool value_changed = false;
	for (int i = 0; i < items_count; i++)
	{
		PushID((void*)(intptr_t)i);
		const bool item_selected = (i == *current_item);
		const char* item_text;
		if (!items_getter(data, i, &item_text))
			item_text = xor_str("item?").c_str();

		if (this->selectable(item_text, item_selected, mod.alpha))
		{
			value_changed = true;
			*current_item = i;
		}
		/*if (item_selected)
		  SetItemDefaultFocus();*/
		PopID();
	}

	EndCombo();
	return value_changed;
}

bool c_menu::combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items)
{
	const bool value_changed = this->combo_wrapper(label, current_item, items_array_getter, (void*)items, items_count, height_in_items);
	return value_changed;
}

bool c_menu::selectable2(const char* label, bool* p_selected, ImGuiSelectableFlags flags, const ImVec2& size_arg)
{
	if (this->selectable(label, *p_selected, flags, 1.f, size_arg))
	{
		*p_selected = !*p_selected;
		return true;
	}
	return false;
}

bool c_menu::selectable_flags(const char* label, unsigned int* flags, unsigned int flags_value)
{
	bool v = ((*flags & flags_value) == flags_value);
	bool pressed = this->selectable2(label, &v, ImGuiSelectableFlags_DontClosePopups);
	if (pressed)
	{
		if (v)
			*flags |= flags_value;
		else
			*flags &= ~flags_value;
	}

	return pressed;
}

void c_menu::multi_combo(const char* name, unsigned int& var, std::vector< std::string > elements)
{
	int t = 0;
	auto& items = combo_items[name];
	items.clear();

	int items_cnt = 0;
	for (int i = 0; i < elements.size(); i++)
	{
		if (var & (1 << i))
		{
			if (t++ > 0)
				items += xor_c(", ");
			items += elements[i];

			items_cnt++;
		}
	}

	if (items.length() >= 12)
	{
		items.erase(12);
		items.append(xor_c("..."));
	}

	if (this->begin_combo(name, items.empty() ? xor_c("Select") : items.c_str(), 0, elements.size()))
	{
		for (int i = 0; i < elements.size(); i++)
			this->selectable_flags(elements[i].c_str(), &var, (1 << i));
		ImGui::EndCombo();
	}
}