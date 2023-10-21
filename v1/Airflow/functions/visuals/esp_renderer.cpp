#include "esp_store.h"
#include "../config_vars.h"
#include "../features.h"

#include "../../base/sdk/entity.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/c_csplayerresource.h"

#include "../../additionals/tinyformat.h"

constexpr float bar_width = 2.f;
constexpr float bar_height = 2.f;

namespace esp_renderer
{
	esp_box_t get_bounding_box(c_baseentity* entity)
	{
		esp_box_t out{};

		if (entity->is_player())
		{
			auto player = (c_csplayer*)entity;

			auto& sound_info = g_esp_store->sounds[player->index()];
			auto& esp_info = g_esp_store->playerinfo[player->index()];

			vector3d mins{}, maxs{}, abs_origin;
			float stand_pose{}, body_pitch;
			if (esp_info.dormant && sound_info.got_box)
			{
				mins = sound_info.dormant_mins;
				maxs = sound_info.dormant_maxs;

				abs_origin = sound_info.pos;

				stand_pose = sound_info.dormant_pose_body;
				body_pitch = sound_info.dormant_pose_pitch;
			}
			else
			{

				mins = player->bb_mins();
				maxs = player->bb_maxs();

				abs_origin = player->get_abs_origin();

				stand_pose = player->pose_parameter()[1];
				body_pitch = player->pose_parameter()[12];
			}

			auto center = abs_origin + math::interpolate(mins, maxs, 0.5f);

			float offset_top = stand_pose * 18.f + 58.f - body_pitch * 6.f;
			float offset_base = maxs.z * -0.6f;

			auto top = vector3d(center.x, center.y, abs_origin.z + offset_top);
			auto base = vector3d(center.x, center.y, center.z + offset_base);

			vector2d screen[2]{};
			vector3d world[2]{ top, base };

			int valid_bounds = 0;
			for (int i = 0; i < 2; i++)
			{
				if (!g_render->world_to_screen(world[i], screen[i]))
					continue;

				++valid_bounds;
			}

			out.offscreen = valid_bounds == 0;
			if (out.offscreen)
				return out;

			float y2 = screen[0].y + std::abs(screen[0].y - screen[1].y);
			float height = y2 - screen[0].y;

			float cur_w = screen[1].x - screen[0].x;
			float center_x = screen[0].x + cur_w / 2.f;

			float w = height / 2.f;
			float x1 = center_x - w / 2.f;
			float x2 = center_x + w / 2.f;

			out.x = x1;
			out.y = screen[0].y;
			out.w = x2 - x1;
			out.h = y2 - screen[0].y;
			return out;
		}

		vector3d min = entity->bb_mins();
		vector3d max = entity->bb_maxs();

		vector3d points[8] = {
			vector3d(min.x, min.y, min.z),
			vector3d(min.x, max.y, min.z),
			vector3d(max.x, max.y, min.z),
			vector3d(max.x, min.y, min.z),
			vector3d(max.x, max.y, max.z),
			vector3d(min.x, max.y, max.z),
			vector3d(min.x, min.y, max.z),
			vector3d(max.x, min.y, max.z)
		};

		int valid_bounds = 0;
		vector2d points_to_screen[8] = {};
		for (int i = 0; i < 8; i++)
		{
			if (!g_render->world_to_screen(math::get_vector_transform(points[i], entity->coordinate_frame()), points_to_screen[i]))
				continue;

			valid_bounds++;
		}

		out.offscreen = valid_bounds == 0;
		if (out.offscreen)
			return out;

		float left = points_to_screen[3].x;
		float top = points_to_screen[3].y;
		float right = points_to_screen[3].x;
		float bottom = points_to_screen[3].y;

		for (auto i = 1; i < 8; i++)
		{
			if (left > points_to_screen[i].x)
				left = points_to_screen[i].x;
			if (top < points_to_screen[i].y)
				top = points_to_screen[i].y;
			if (right < points_to_screen[i].x)
				right = points_to_screen[i].x;
			if (bottom > points_to_screen[i].y)
				bottom = points_to_screen[i].y;
		}

		out.x = left;
		out.y = bottom;
		out.w = right - left;
		out.h = top - bottom;
		return out;
	}

	void esp_strings(esp_box_t& box, esp_objects_t& object, int max_bar_width, float& offset)
	{
		if (object.string == "")
			return;

		auto get_current_font = [&](int idx)
		{
			switch (idx)
			{
			case font_default:
				return g_fonts.esp;
				break;
			case font_pixel:
				return g_fonts.pixel;
				break;
			case font_icon:
				return g_fonts.weapon_icons;
				break;
			}
		};

		ImGui::PushFont(get_current_font(object.font_type));
		auto text_size = ImGui::CalcTextSize(object.string.c_str());
		ImGui::PopFont();

		float add = ((bar_width * 2.f) + 1.f) * max_bar_width;
		float add_h = ((bar_height * 2.f) + 1.f) * max_bar_width;

		auto render_string = [&](float x, float y, bool center = false)
		{
			auto fl = object.dropshadow ? dropshadow_ : outline_light;

			g_render->string(x, y, object.color.new_alpha(object.color.a() * object.custom_alpha), center ? centered_x | fl : fl, get_current_font(object.font_type), object.string.c_str());
		};

		switch (object.pos_type)
		{
		case esp_left:
			render_string(box.x - text_size.x - add - 3.f, box.y + offset);
			break;
		case esp_up:
			render_string(box.x + box.w / 2.f, box.y - add_h - text_size.y - offset - 3.f, true);
			break;
		case esp_right:
			render_string(box.x + box.w + add + 4.f, box.y + offset);
			break;
		case esp_down:
			render_string(box.x + box.w / 2.f, box.y + box.h + add_h + offset + 3.f, true);
			break;
		}

		offset += (text_size.y);
	}

	void esp_bars(esp_box_t& box, esp_objects_t& object, int& offset)
	{
		if (object.bar <= 0.f)
			return;

		float bar_value = std::clamp<float>(object.bar, 0.f, object.bar_max);

		const auto outline_color = color(50, 50, 50, object.color.a() * object.custom_alpha);
		auto bar_color = object.color.new_alpha(object.color.a() * object.custom_alpha);

		auto text_color = color(255, 255, 255, bar_color.a() * object.custom_alpha);

		float add = ((bar_width * 2.f) + 1.f) * offset;
		float add_h = ((bar_height * 2.f) + 1.f) * offset;

		float bar_h = (box.h - ((box.h * bar_value) / object.bar_max));
		bar_h = std::max<float>(std::min<float>(bar_h, box.h), 0.f);

		float bar_w = ((box.w * bar_value) / object.bar_max);
		bar_w = std::max<float>(std::min<float>(bar_w, box.w), 0.f);

		auto value = std::to_string((int)object.bar);

		switch (object.pos_type)
		{
		case esp_left:
		{
			g_render->filled_rect((box.x - (bar_width * 2.f)) - 2.f - add, box.y - 1.f, bar_width * 2.f, box.h + 2.f, outline_color);
			g_render->filled_rect((box.x - (bar_width * 2.f)) - 1.f - add, box.y + bar_h, bar_width, box.h - bar_h, bar_color);

			if (object.bar < object.bar_max)
				g_render->string((box.x - (bar_width * 2.f)) - add + 1.f, (box.y + bar_h) - 3.f, text_color, centered_x | outline_light, g_fonts.pixel, value.c_str());
		}
		break;
		case esp_up:
		{
			g_render->filled_rect(box.x - 1.f, box.y - (bar_height * 2.f) - 2.f - add_h, box.w + 2.f, bar_height * 2.f, outline_color);
			g_render->filled_rect(box.x, box.y - (bar_height * 2.f) - 1.f - add_h, bar_w, bar_height, bar_color);

			if (object.bar < object.bar_max)
				g_render->string(box.x + bar_w, box.y - (bar_height * 2.f) - add_h, text_color, centered_x | centered_y | outline_light, g_fonts.pixel, value.c_str());
		}
		break;
		case esp_right:
		{
			g_render->filled_rect(box.x + box.w + add + 2.f, box.y - 1.f, bar_width * 2.f, box.h + 2.f, outline_color);
			g_render->filled_rect(box.x + box.w + add + 3.f, box.y + bar_h, bar_width, box.h - bar_h, bar_color);

			if (object.bar < object.bar_max)
				g_render->string(box.x + box.w + add + 4.f, (box.y + bar_h) - 1.f, text_color, centered_x | outline_light, g_fonts.pixel, value.c_str());
		}
		break;
		case esp_down:
		{
			g_render->filled_rect(box.x - 1.f, box.y + box.h + add_h + 2.f, box.w + 2.f, bar_height * 2.f, outline_color);
			g_render->filled_rect(box.x, box.y + box.h + add_h + 3.f, bar_w, bar_height, bar_color);

			if (object.bar < object.bar_max)
				g_render->string(box.x + bar_w, box.y + box.h + add_h + 5.f, text_color, centered_x | centered_y | outline_light, g_fonts.pixel, value.c_str());
		}
		break;
		}

		offset++;
	}
}