#include "engine_trace.h"

#include "../sdk/entity.h"

bool c_game_trace::did_hit_world() const
{
	return entity == interfaces::entity_list->get_entity(0);
}

bool c_game_trace::did_hit_non_world_entity() const
{
	return entity != nullptr && !did_hit_world();
}

bool c_trace_filter::should_hit_entity(c_baseentity* entity, int mask)
{
	if (skip)
	{
		auto ecc = ((c_baseentity*)skip)->get_client_class();
		if (ecc && strcmp(ignore, ""))
		{
			if (ecc->network_name == ignore)
				return false;
		}
	}

	return !(skip == entity);
}

void c_engine_trace::trace_line(const vector3d& src, const vector3d& dst, int mask, c_handle_entity* entity, int collision_group, c_game_trace* trace)
{
	static auto trace_filter_simple = patterns::trace_filter.add(0x3D).as< void* >();

	std::uintptr_t filter[4] = { *(std::uintptr_t*)(trace_filter_simple), (std::uintptr_t)(entity), collision_group, 0 };

	this->trace_ray(ray_t(src, dst), mask, (c_trace_filter*)(&filter), trace);
}

void c_engine_trace::trace_hull(const vector3d& src, const vector3d& dst, const vector3d& mins, const vector3d& maxs, int mask, c_handle_entity* entity, int collision_group, c_game_trace* trace)
{
	static auto trace_filter_simple = patterns::trace_filter.add(0x3D).as< void* >();

	std::uintptr_t filter[4] = { *(std::uintptr_t*)(trace_filter_simple), (std::uintptr_t)(entity), collision_group, 0 };

	this->trace_ray(ray_t(src, dst, mins, maxs), mask, (c_trace_filter*)(&filter), trace);
}

bool c_trace_filter_players_only_skip_one::should_hit_entity(c_baseentity* entity, int)
{
	return entity != e && (int)entity->get_client_class()->class_id == 40;
}
