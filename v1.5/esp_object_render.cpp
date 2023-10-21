#include "globals.hpp"
#include "esp_object_render.hpp"
#include "render.hpp"

constexpr float BAR_WIDTH_X = 2.f;
constexpr float BAR_WIDTH_Y = 2.f;

namespace esp_objects
{
	void render_strings(box_t& box, esp_object_t& object, int max_bar_width, float& offset)
	{
		if (object.string == "")
			return;

		auto font = get_font_by_index(object.string_font);
		if (!font)
			return;

		auto font_ptr = font->get();

		auto list = RENDER->get_draw_list();
		ImVec2 text_size = RENDER->get_text_size(font, object.string.c_str());

		auto add = ((BAR_WIDTH_X * 2.f)) * max_bar_width;
		auto add_h = ((BAR_WIDTH_Y * 2.f)) * max_bar_width;

		vec2_t position{};
		switch (object.position)
		{
		case ESP_POS_LEFT:
		{
			position.x = box.x - text_size.x - add - 2.f;
			position.y = box.y + offset;
		}
		break;
		case ESP_POS_UP:
		{
			position.x = (box.x + box.w / 2.f);
			position.y = box.y - add_h - text_size.y - offset - 3.f;
		}
		break;
		case ESP_POS_RIGHT:
		{
			position.x = box.x + box.w + add + 4.f;
			position.y = box.y + offset;
		}
		break;
		case ESP_POS_DOWN:
		{
			position.x = (box.x + box.w / 2.f);
			position.y = box.y + box.h + add_h + offset + 3.f;
		}
		break;
		}

		auto centered = object.position == ESP_POS_UP || object.position == ESP_POS_DOWN;
		auto flags = (centered ? FONT_CENTERED_X : 0) | object.font_flags;
		RENDER->text(position.x, position.y, object.clr.new_alpha((int)(object.clr.a() * object.alpha)),
			flags, font, object.string);

		offset += text_size.y;
	}

	void render_bars(box_t& box, esp_object_t& object, int& offset)
	{
		if (object.bar <= 0.f)
			return;

		auto bar_value = std::clamp<float>(object.bar, 0.f, object.bar_max);
		auto add = ((BAR_WIDTH_X * 2.f)) * offset;
		auto add_h = ((BAR_WIDTH_Y * 2.f)) * offset;
		auto bar_h = std::clamp<float>((box.h - ((box.h * bar_value) / object.bar_max)), 0.f, box.h);
		auto bar_w = std::clamp<float>(((box.w * bar_value) / object.bar_max), 0.f, box.w);

		auto value = std::to_string((int)object.bar);

		vec2_t bar_outline_min{}, bar_outline_max{},
			bar_min{}, bar_max{}, text_pos{};

		switch (object.position)
		{
		case ESP_POS_LEFT:
		{
			bar_outline_min.x = (box.x - (BAR_WIDTH_X * 2.f)) - 2.f - add;
			bar_outline_min.y = box.y - 1.f;

			bar_outline_max.x = BAR_WIDTH_X * 2.f;
			bar_outline_max.y = box.h + 2.f;

			bar_min.x = (box.x - (BAR_WIDTH_X * 2.f)) - 1.f - add;
			bar_min.y = box.y + bar_h;

			bar_max.x = BAR_WIDTH_X;
			bar_max.y = box.h - bar_h;

			text_pos.x = ((box.x - (BAR_WIDTH_X * 2.f)) - add + 1.f) - 2.f;
			text_pos.y = (box.y + bar_h) - 3.f;
		}
		break;
		case ESP_POS_UP:
		{
			bar_outline_min.x = box.x - 1.f;
			bar_outline_min.y = box.y - (BAR_WIDTH_Y * 2.f) - 2.f - add_h;

			bar_outline_max.x = box.w + 2.f;
			bar_outline_max.y = BAR_WIDTH_Y * 2.f;

			bar_min.x = box.x;
			bar_min.y = box.y - (BAR_WIDTH_Y * 2.f) - 1.f - add_h;

			bar_max.x = bar_w;
			bar_max.y = BAR_WIDTH_Y;

			text_pos.x = box.x + bar_w;
			text_pos.y = box.y - (BAR_WIDTH_Y * 2.f) - add_h;
		}
		break;
		case ESP_POS_RIGHT:
		{
			bar_outline_min.x = box.x + box.w + add + 2.f;
			bar_outline_min.y = box.y - 1.f;

			bar_outline_max.x = BAR_WIDTH_X * 2.f;
			bar_outline_max.y = box.h + 2.f;

			bar_min.x = box.x + box.w + add + 3.f;
			bar_min.y = box.y + bar_h;

			bar_max.x = BAR_WIDTH_X;
			bar_max.y = box.h - bar_h;

			text_pos.x = box.x + box.w + add + 4.f;
			text_pos.y = (box.y + bar_h) - 1.f;
		}
		break;
		case ESP_POS_DOWN:
		{
			bar_outline_min.x = box.x - 1.f;
			bar_outline_min.y = box.y + box.h + add_h + 2.f;

			bar_outline_max.x = box.w + 2.f;
			bar_outline_max.y = BAR_WIDTH_Y * 2.f;

			bar_min.x = box.x;
			bar_min.y = box.y + box.h + add_h + 3.f;

			bar_max.x = bar_w;
			bar_max.y = BAR_WIDTH_Y;

			text_pos.x = box.x + bar_w;
			text_pos.y = box.y + box.h + add_h + 4.f;
		}
		break;
		}

		RENDER->filled_rect(bar_outline_min.x, bar_outline_min.y, bar_outline_max.x, bar_outline_max.y,
			object.outline_clr.new_alpha((int)(object.outline_clr.a() * 0.6f * object.alpha)));

		RENDER->filled_rect(bar_min.x, bar_min.y, bar_max.x, bar_max.y,
			object.clr.new_alpha((int)(object.clr.a() * object.alpha)));

		if (object.bar < object.bar_max)
		{
			auto string = std::to_string((int)object.bar);
			auto align = ((object.position == ESP_POS_LEFT || object.position == ESP_POS_RIGHT)
				? FONT_CENTERED_X : FONT_CENTERED_X | FONT_CENTERED_Y) | FONT_OUTLINE | FONT_LIGHT_BACK;

			RENDER->text(text_pos.x, text_pos.y, c_color{ 255, 255, 255, (int)(230 * object.alpha) },align, &RENDER->fonts.pixel, string);
		}

		offset++;
	}
}