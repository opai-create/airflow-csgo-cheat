#pragma once
#include "../tools/math.h"
#include "../sdk/entity.h"

class color;
class c_glow_object_manager
{
public:
	class c_glow_object_definition
	{
	public:
		c_glow_object_definition()
		{
			memset(this, 0, sizeof(*this));
		}

		int32_t next_slot{};
		c_baseentity* entity{};

		union
		{
			vector3d color;
			struct
			{
				float r;
				float g;
				float b;
			};
		};

		float alpha{};
		bool custoalpha{};
		float alpha_velocity{};
		float alpha_max{};
		float glow_pulse{};
		bool occlued_render{};
		bool unocclued_render{};
		bool bloom{};
		int full_bloom_stencil_test_value{};
		int32_t glow_style{};
		int32_t screen_slot{};

		bool is_unused() const
		{
			return next_slot != entry_in_use;
		}

		static constexpr int end_of_free_list = -1;
		static constexpr int entry_in_use = -2;
	};

	c_utlvector< c_glow_object_definition > glow_objects{};
	int first_slot{};

	struct glow_box_definition_t
	{
		vector3d pos{};
		vector3d ang{};
		vector3d mins{};
		vector3d maxs{};
		float birth_time{};
		float end_time{};
		color clr{};
	};

	c_utlvector< glow_box_definition_t > glow_boxes{};

	void add_glow_box(const vector3d& origin, const vector3d& orient, const vector3d& mins, const vector3d& maxs, const color&clr, float birth_time, float life_time) {
		glow_boxes.add_to_tail({ origin, orient, mins, maxs, birth_time, birth_time + life_time, clr.u32() });
	}

};