#include "../menu.h"

#include <ranges>
#include <algorithm>

using namespace ImGui;

bool c_menu::listbox_selectable(const char* label, bool selected, float alpha_pass, ImGuiSelectableFlags flags, const ImVec2& size_arg, int iter)
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
	ImVec2 size(220.f, 36.f);
	ImVec2 pos = window->DC.CursorPos;
	pos.x += 6.f;
	ImRect bb_inner(pos, pos + size);
	bb_inner.Min.x += 11.f;
	bb_inner.Max.x += 11.f;

	bb_inner.Min.y += 8.f;
	bb_inner.Max.y += 8.f;

	ItemSize(size, 0.0f);

	// Fill horizontal space.
	ImVec2 window_padding = window->WindowPadding;
	float max_x = (flags & ImGuiSelectableFlags_SpanAllColumns) ? GetWindowContentRegionMax().x : GetContentRegionMax().x;
	float w_draw = ImMax(label_size.x, window->Pos.x + max_x - window_padding.x - pos.x);
	ImRect bb(pos, pos + size);
	if (size_arg.x == 0.0f || (flags & ImGuiSelectableFlags_DrawFillAvailWidth))
		bb.Max.x += window_padding.x;

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

	auto& mod = item_animations[CONST_HASH(label) + iter];

	if (alpha_pass > 0.f)
	{
		this->create_animation(mod.hovered_alpha, hovered, 1.f, lerp_animation);
		this->create_animation(mod.alpha, pressed, 0.6f, lerp_animation);
	}

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

	if (alpha_pass > 0.f)
	{
		auto text_clr = selected ? 1.f : 0.58f + 0.42f * mod.hovered_alpha;
		auto back_alpha = (20 + (30 * mod.hovered_alpha) + (50 * (hovered && held))) * alpha;
		c_color back_clr = c_color(217, 217, 217).increase(38 * mod.hovered_alpha).new_alpha(back_alpha);

		window->DrawList->AddRectFilled(bb.Min, bb.Max - ImVec2(0.f, 4.f), back_clr.as_imcolor(), 4.f);

		PushStyleColor(ImGuiCol_Text, ImVec4(text_clr, text_clr, text_clr, text_clr * alpha * alpha_pass));
		RenderTextClipped(bb_inner.Min, bb_inner.Max, label, NULL, &label_size, style.SelectableTextAlign, &bb);
		PopStyleColor();
	}

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags);
	return pressed;
}

bool c_menu::list_box_header(const char* label, const ImVec2& size_arg)
{
	float alpha = this->get_alpha();
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f * alpha));

	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	const ImGuiStyle& style = g.Style;
	const ImGuiID id = GetID(label);
	const ImVec2 label_size = CalcTextSize(label, NULL, true);

	// Size default to hold ~7 items. Fractional number of items helps seeing that we can scroll down/up without looking at scrollbar.
	ImVec2 size = CalcItemSize(size_arg, 256.f, GetTextLineHeightWithSpacing() * 7.4f + style.ItemSpacing.y);
	ImVec2 frame_size = ImVec2(size.x, ImMax(size.y, label_size.y));
	ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
	ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
	window->DC.LastItemRect = bb; // Forward storage for ListBoxFooter.. dodgy.
	g.NextItemData.ClearFlags();

	if (!IsRectVisible(bb.Min, bb.Max))
	{
		ItemSize(bb.GetSize(), style.FramePadding.y);
		ItemAdd(bb, 0, &frame_bb);
		return false;
	}

	BeginGroup();

	auto back_alpha = 20 * alpha;
	c_color back_clr = c_color(217, 217, 217).new_alpha(back_alpha);

	PushStyleVar(ImGuiStyleVar_ChildRounding, 4.f);
	PushStyleColor(ImGuiCol_ChildBg, back_clr.as_imvec4());

	BeginChild(id, size, true, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_NoScrollbar);

	PopStyleColor(2);
	PopStyleVar();

	return true;
}

// FIXME: In principle this function should be called EndListBox(). We should rename it after re-evaluating if we want to keep the same signature.
bool c_menu::list_box_header_start(const char* label, int items_count, int height_in_items)
{
	// Size default to hold ~7.25 items.
	// We add +25% worth of item height to allow the user to see at a glance if there are more items up/down, without looking at the scrollbar.
	// We don't add this extra bit if items_count <= height_in_items. It is slightly dodgy, because it means a dynamic list of items will make the widget resize occasionally when it crosses that size.
	// interfaces am expecting that someone will come and complain about this behavior in a remote future, then we can advise on a better solution.
	if (height_in_items < 0)
		height_in_items = ImMin(items_count, 7);
	const ImGuiStyle& style = GetStyle();
	float height_in_items_f = (height_in_items < items_count) ? (height_in_items + 0.25f) : (height_in_items + 0.00f);

	// We include ItemSpacing.y so that a list sized for the exact number of items doesn't make a scrollbar appears. We could also enforce that by passing a flag to BeginChild().
	ImVec2 size;
	size.x = 0.0f;
	size.y = ImFloor(GetTextLineHeightWithSpacing() * height_in_items_f + style.FramePadding.y * 2.0f);
	return this->list_box_header(label, size);
}

// FIXME: In principle this function should be called EndListBox(). We should rename it after re-evaluating if we want to keep the same signature.
void c_menu::list_box_footer()
{
	ImGuiWindow* parent_window = GetCurrentWindow()->ParentWindow;
	const ImRect bb = parent_window->DC.LastItemRect;
	const ImGuiStyle& style = GetStyle();

	EndChildFrame();

	// Redeclare item size so that it includes the label (we have stored the full size in LastItemRect)
	// We call SameLine() to restore DC.CurrentLine* data
	SameLine();
	parent_window->DC.CursorPos = bb.Min;
	ItemSize(bb, style.FramePadding.y);
	EndGroup();
}

bool equals(std::string a, std::string b)
{
	std::transform(a.begin(), a.end(), a.begin(), ::tolower);
	return std::strstr(a.c_str(), b.c_str());
}

bool c_menu::list_box_wrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items, const std::string& compare_text)
{
	if (!this->list_box_header_start(label, items_count, height_in_items))
		return false;

	// Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can create a custom version of ListBox() in your code without using the clipper.
	ImGuiContext& g = *GImGui;
	bool value_changed = false;

	auto lower_string = compare_text;
	std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(), ::tolower);

	std::vector< std::pair< std::string, int > > filtered_elements{};
	if (lower_string.size() > 0)
	{
		for (int i = 0; i < items_count; ++i)
		{
			const char* item_text;
			if (!items_getter(data, i, &item_text))
				item_text = ("*Unknown item*");

			// push filtered elements for listbox
			if (equals(item_text, lower_string))
				filtered_elements.emplace_back(std::make_pair(item_text, i));
		}
	}

	// then use filtered elements with clipper for optimization
	ImGuiListClipper clipper(filtered_elements.size() > 0 ? filtered_elements.size() : items_count, GetTextLineHeightWithSpacing());
	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
		{
			PushID(i);

			const char* item_text;
			if (!items_getter(data, i, &item_text))
				item_text = ("*Unknown item*");

			int index = filtered_elements.size() > 0 ? filtered_elements[i].second : i;
			const char* name = filtered_elements.size() > 0 ? filtered_elements[i].first.c_str() : item_text;

			const bool item_selected = (index == *current_item);
			if (this->listbox_selectable(name, item_selected, 1.f, 0, {}, index))
			{
				*current_item = index;
				value_changed = true;
			}

			PopID();
		}
	}

	this->list_box_footer();
	if (value_changed)
		MarkItemEdited(g.CurrentWindow->DC.LastItemId);

	return value_changed;
}

bool c_menu::listbox(const char* label, int* currIndex, std::vector< std::string >& values, int height, const std::string& compare_text)
{
	if (values.empty())
	{
		return false;
	}

	return this->list_box_wrapper(label, currIndex, this->vector_getter, static_cast<void*>(&values), values.size(), height, compare_text);
}