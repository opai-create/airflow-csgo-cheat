#include "autowall.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/sdk/entity.h"

constexpr int shot_hull = 0x600400bu;
constexpr int shot_player = 0x4600400bu;

class c_trace_filter_simple
{
public:
	__forceinline c_trace_filter_simple() : vtable{ *patterns::trace_filter.add(0x3D).as< uintptr_t* >() }
	{
	}

	__forceinline c_trace_filter_simple(c_baseentity* const ignore_entity, const int collision_group)
		: vtable{ *patterns::trace_filter.add(0x3D).as< uintptr_t* >() }, ignore_entity{ ignore_entity }, collision_group{ collision_group }
	{
	}

	uintptr_t vtable{};
	c_baseentity* ignore_entity{};
	int collision_group{};
	should_hit_fn_t should_hit_fn{};
};

class c_trace_filter_skip_two_entities
{
public:
	__forceinline c_trace_filter_skip_two_entities() : vtable{ *patterns::trace_filter_skip_entities.add(0x3).as< uintptr_t* >() }
	{
	}

	__forceinline c_trace_filter_skip_two_entities(c_baseentity* const ignore_entity0, c_baseentity* const ignore_entity1, const int collision_group = 0)
		: vtable{ *patterns::trace_filter_skip_entities.add(0x3).as< uintptr_t* >() }, ignore_entity0{ ignore_entity0 }, collision_group{ collision_group }, ignore_entity1{ ignore_entity1 }
	{
	}

	uintptr_t vtable{};
	c_baseentity* ignore_entity0{};
	int collision_group{};
	should_hit_fn_t should_hit_fn{};
	c_baseentity* ignore_entity1{};
};

bool c_auto_wall::is_breakable_entity(c_baseentity* entity)
{
	if (!entity || !entity->index() || !entity->get_client_class())
		return false;

	auto client_class = entity->get_client_class();

	// on v4c map cheat shoots through this
	// ignore these fucks
	if (client_class->class_id == CBaseButton || client_class->class_id == CPhysicsProp)
		return false;

	// check if it's window by name (pasted from onetap)
	auto v3 = (int)client_class->network_name;
	if (*(DWORD*)v3 == 0x65724243)
	{
		if (*(DWORD*)(v3 + 7) == 0x53656C62)
			return true;
	}

	if (*(DWORD*)v3 == 0x73614243)
	{
		if (*(DWORD*)(v3 + 7) == 0x79746974)
			return true;
	}

	auto take_damage = *patterns::is_breakable_entity.add(xor_int(0x26)).as<uintptr_t*>();
	auto backup = *(uint8_t*)((uintptr_t)entity + take_damage);

	if (HASH_RT(client_class->network_name) == HASH("CBreakableSurface"))
		*(uint8_t*)((uintptr_t)entity + take_damage) = 2;
	else if (HASH_RT(client_class->network_name) == HASH("CBaseDoor") || HASH_RT(client_class->network_name) == HASH("CDynamicProp"))
		*(uint8_t*)((uintptr_t)entity + take_damage) = 0;

	auto result = func_ptrs::is_breakable_entity(entity);
	*(uint8_t*)((uintptr_t)entity + take_damage) = backup;

	return result;
}

bool c_auto_wall::can_hit_point(c_csplayer* entity, c_csplayer* starter, const vector3d& point, const vector3d& source, int min_damage)
{
	const auto origin_backup = starter->get_abs_origin();

	starter->set_abs_origin(vector3d(source.x, source.y, origin_backup.z));

	const auto& data = this->fire_bullet(starter, entity, g_ctx.weapon_info, g_ctx.weapon->is_taser(), source, point);

	/*interfaces::debug_overlay->add_line_overlay(source, point, 255, 255, 255, true, interfaces::global_vars->interval_per_tick * 2.f);

	interfaces::debug_overlay->add_text_overlay(source, interfaces::global_vars->interval_per_tick * 2.f, "%d", data.dmg);
	interfaces::debug_overlay->add_text_overlay(point, interfaces::global_vars->interval_per_tick * 2.f, "%d", data.dmg);*/

	starter->set_abs_origin(origin_backup);

	return data.dmg >= min_damage + 1;
}

bool c_auto_wall::trace_to_exit(const vector3d& src, const vector3d& dir, const c_game_trace& enter_trace, c_game_trace& exit_trace, c_csplayer* shooter)
{
	constexpr float MAX_DISTANCE = 90.f, STEP_SIZE = 4.f;
	float current_distance = 0.f;

	int first_contents = 0;

	do
	{
		current_distance += STEP_SIZE;
		auto new_end = src + (dir * current_distance);

		int point_contents = interfaces::engine_trace->get_point_contents(new_end, shot_player);

		if (!first_contents)
			first_contents = point_contents;

		if (!(point_contents & mask_shot_hull) || ((point_contents & contents_hitbox) && point_contents != first_contents))
		{
			auto new_start = new_end - (dir * STEP_SIZE);

			interfaces::engine_trace->trace_ray(ray_t(new_end, new_start), shot_player, nullptr, &exit_trace);

			//	HACKS->debug_overlay->add_line_overlay(new_end, new_start, 255, 25, 255, 1.f, 0.1f);

			if (exit_trace.start_solid && exit_trace.surface.flags & surf_hitbox)
			{
				c_trace_filter_skip_two_entities filter(shooter, exit_trace.entity);

				interfaces::engine_trace->trace_ray(ray_t(src, new_start), shot_player, (i_trace_filter*)&filter, &exit_trace);

				if (exit_trace.did_hit() && !exit_trace.start_solid)
				{
					new_end = exit_trace.end;
					return true;
				}

				continue;
			}
			else {

				if (!exit_trace.did_hit() || exit_trace.start_solid)
				{
					if (exit_trace.entity)
					{
						if (enter_trace.did_hit_non_world_entity())
						{
							if (this->is_breakable_entity(enter_trace.entity))
							{
								exit_trace = enter_trace;
								exit_trace.end = src + dir;
								return true;
							}
						}
					}
				}
				else
				{
					if (this->is_breakable_entity(enter_trace.entity) && this->is_breakable_entity(exit_trace.entity))
						return true;

					if (enter_trace.surface.flags & surf_nodraw || (!(exit_trace.surface.flags & surf_nodraw) && exit_trace.plane.normal.dot(dir) <= 1.f))
					{
						const float mult_amount = exit_trace.fraction * 4.f;

						// get the real end pos
						new_start -= dir * mult_amount;
						return true;
					}

					continue;
				}
			}
		}
	} while (current_distance <= MAX_DISTANCE);

	return false;
}

void c_auto_wall::clip_trace_to_player(const vector3d& src, const vector3d& dst, c_game_trace& trace, c_csplayer* const player, const should_hit_fn_t& should_hit_fn)
{
	if (should_hit_fn && !should_hit_fn(player, shot_player))
		return;

	const auto pos = player->origin() + (player->bb_mins() + player->bb_maxs()) * 0.5f;
	const auto to = pos - src;

	auto dir = src - dst;
	const auto len = dir.normalized_float();
	const auto range_along = dir.dot(to);

	const auto range = range_along < 0.f ? -(to).length(false) : range_along > len ? -(pos - dst).length(false) : (pos - (src + dir * range_along)).length(false);

	if (range > 60.f)
		return;

	c_game_trace new_trace{};
	interfaces::engine_trace->clip_ray_to_entity({ src, dst }, shot_player, player, &new_trace);

	if (new_trace.fraction > trace.fraction)
		return;

	trace = new_trace;
}

bool c_auto_wall::handle_bullet_penetration(
	c_csplayer* const shooter, const weapon_info_t* const wpn_data, const c_game_trace& enter_trace, vector3d& src, const vector3d& dir, int& pen_count, float& cur_dmg, const float pen_modifier)
{
	const auto enter_surface_data = interfaces::phys_surface_props->get_surface_data(enter_trace.surface.surface_props);

	int enter_material = enter_surface_data->game.material;

	bool solid_surf = ((enter_trace.contents >> 3) & contents_solid);
	bool light_surf = ((enter_trace.surface.flags >> 7) & surf_light);
	bool contentes_grate = enter_trace.contents & contents_grate;
	bool draw_surf = !!(enter_trace.surface.flags & (surf_nodraw));

	if (pen_count == 0 &&
		!contentes_grate &&
		!draw_surf &&
		enter_material != char_tex_grate &&
		enter_material != char_tex_glass)
		return false;

	if (wpn_data->penetration <= 0.f || pen_count == 0)
		return false;

	c_game_trace exit_trace = { };
	if (!this->trace_to_exit(enter_trace.end, dir, enter_trace, exit_trace, shooter))
	{
		if ((interfaces::engine_trace->get_point_contents(enter_trace.end, mask_shot_hull) & mask_shot_hull) == 0)
			return false;
	}

	const auto exit_surface_data = interfaces::phys_surface_props->get_surface_data(exit_trace.surface.surface_props);
	if (!exit_surface_data)
		return false;

	const float enter_penetration_modifier = enter_surface_data->game.penetration_modifier;
	const float exit_penetration_modifier = exit_surface_data->game.penetration_modifier;
	const float exit_damage_modifier = exit_surface_data->game.penetration_modifier;

	const int exit_material = exit_surface_data->game.material;

	float damage_modifier = 0.f;
	float penetration_modifier = 0.f;

	damage_modifier = 0.16f;
	penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;

	if (enter_material == char_tex_grate || enter_material == char_tex_glass)
	{
		damage_modifier = 0.05f;
		penetration_modifier = 3.0f;
	}
	else if (solid_surf || light_surf)
	{
		damage_modifier = 0.16f;
		penetration_modifier = 1.0f;
	}
	else if (enter_material == char_tex_flesh
		&& (((c_csplayer*)enter_trace.entity)->team() == shooter->team())
		&& cvars::ff_damage_reduction_bullets->get_float() >= 0.f)
	{
		if (cvars::ff_damage_bullet_penetration->get_float() == 0.f)
			return false;

		penetration_modifier = cvars::ff_damage_bullet_penetration->get_float();
		damage_modifier = 0.16f;
	}
	else
	{
		damage_modifier = 0.16f;
		penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == char_tex_wood || exit_material == char_tex_cardboard)
			penetration_modifier = 3.f;
		else if (exit_material == char_tex_plastic)
			penetration_modifier = 2.f;
	}

	float trace_distance = (exit_trace.end - enter_trace.end).length(false);
	float penetration_mod = std::max(0.0f, 1.0f / penetration_modifier);

	trace_distance = trace_distance * trace_distance * penetration_mod * 0.041666668f;

	auto lost_damage = std::max(0.0f, 3.0f / wpn_data->penetration * 1.25f)
		* penetration_mod * 3.0f + cur_dmg * damage_modifier + trace_distance;

	const float clamped_lost_damage = fmaxf(lost_damage, 0.f);
	if (clamped_lost_damage > cur_dmg)
		return false;

	if (clamped_lost_damage > 0.0f)
		cur_dmg -= clamped_lost_damage;

	if (cur_dmg < 1.0f)
		return false;

	src = exit_trace.end;
	--pen_count;
	return true;
}

bool is_armored(c_csplayer* player, const int hitgroup)
{
	bool is_armored = false;

	if (player->armor_value() > 0)
	{
		switch (hitgroup)
		{
		case hitgroup_generic:
		case hitgroup_chest:
		case hitgroup_stomach:
		case hitgroup_leftarm:
		case hitgroup_rightarm:
		case hitgroup_neck:
			is_armored = true;
			break;
		case hitgroup_head:
			if (player->has_helmet())
				is_armored = true;
			[[fallthrough]];
		case hitgroup_leftleg:
		case hitgroup_rightleg:
			if (player->has_heavy_armor())
				is_armored = true;
			break;
		default:
			break;
		}
	}

	return is_armored;
}

void c_auto_wall::scale_dmg(c_csplayer* const player, float& dmg, const float weapon_armor_ratio, const float headshot_mult, const int hitgroup)
{
	const bool has_heavy_armor = player->has_heavy_armor();

	float head_damage_scale = player->team() == 3
		? cvars::mp_damage_scale_ct_head->get_float() : player->team() == 2
		? cvars::mp_damage_scale_t_head->get_float() : 1.0f;

	const float body_damage_scale = player->team() == 3
		? cvars::mp_damage_scale_ct_body->get_float() : player->team() == 2
		? cvars::mp_damage_scale_t_body->get_float() : 1.0f;

	if (has_heavy_armor)
		head_damage_scale *= 0.5f;

	switch (hitgroup)
	{
	case hitgroup_head:
		dmg *= headshot_mult * head_damage_scale;
		break;
	case hitgroup_chest:
	case hitgroup_leftarm:
	case hitgroup_rightarm:
	case hitgroup_neck:
		dmg *= body_damage_scale;
		break;
	case hitgroup_stomach:
		dmg *= 1.25f * body_damage_scale;
		break;
	case hitgroup_leftleg:
	case hitgroup_rightleg:
		dmg *= 0.75f * body_damage_scale;
		break;
	default:
		break;
	}

	if (is_armored(player, hitgroup))
	{
		const int armor = player->armor_value();
		float heavy_armor_bonus = 1.0f, armor_bonus = 0.5f, armor_ratio = weapon_armor_ratio * 0.5f;

		if (has_heavy_armor)
		{
			heavy_armor_bonus = 0.25f;
			armor_bonus = 0.33f;
			armor_ratio *= 0.20f;
		}

		float damage_to_health = dmg * armor_ratio;
		if (const float damage_to_armor = (dmg - damage_to_health) * (heavy_armor_bonus * armor_bonus); damage_to_armor > static_cast<float>(armor))
			damage_to_health = dmg - static_cast<float>(armor) / armor_bonus;

		dmg = damage_to_health;
	}
}

pen_data_t c_auto_wall::fire_bullet(c_csplayer* const shooter, c_csplayer* const target, const weapon_info_t* const wpn_data, const bool is_taser, vector3d src, const vector3d& dst, bool ignore_target, c_game_trace* tr)
{
	auto pen_modifier = std::max((3.f / wpn_data->penetration) * 1.25f, 0.f);

	float cur_dist{};
	pen_data_t data{};

	data.remaining_pen = 4;

	auto cur_dmg = static_cast<float>(wpn_data->dmg);
	auto dir = dst - src;
	dir = dir.normalized();

	c_game_trace trace{};
	c_trace_filter_simple trace_filter{};
	trace_filter.ignore_entity = shooter;

	auto max_dist = wpn_data->range;

	while (cur_dmg > 0.f)
	{
		max_dist -= cur_dist;

		const auto cur_dst = src + dir * max_dist;

		interfaces::engine_trace->trace_ray(ray_t(src, cur_dst), shot_player, (i_trace_filter*)(&trace_filter), &trace);

		if (target)
			this->clip_trace_to_player(src, cur_dst + dir * 40.f, trace, target, trace_filter.should_hit_fn);

		if (trace.fraction == 1.f)
			break;

		cur_dist += trace.fraction * max_dist;
		cur_dmg *= std::pow(wpn_data->range_modifier, cur_dist / 500.f);

		const auto enter_surface = interfaces::phys_surface_props->get_surface_data(trace.surface.surface_props);

		if (cur_dist > 3000.f || enter_surface->game.penetration_modifier < 0.1f)
			break;

		if (target)
		{
			if (tr)
				*tr = trace;

			if (trace.entity)
			{
				const auto is_player = trace.entity->is_player();
				if ((trace.entity == target || (is_player && ((c_csplayer*)trace.entity)->team() != shooter->team())) && trace.hitgroup != hitgroup_generic && trace.hitgroup != hitgroup_gear)
				{
					data.hit_player = (c_csplayer*)trace.entity;
					data.hitbox = trace.hitbox;
					data.hitgroup = trace.hitgroup;

					if (is_taser)
						data.hitgroup = 0;

					this->scale_dmg(data.hit_player, cur_dmg, wpn_data->armor_ratio, wpn_data->crosshair_delta_distance, data.hitgroup);

					data.dmg = static_cast<int>(cur_dmg);

					return data;
				}
			}
		}

		if (is_taser)
			break;

		if (!handle_bullet_penetration(shooter, wpn_data, trace, src, dir, data.remaining_pen, cur_dmg, pen_modifier))
			break;

		if (ignore_target)
			data.dmg = static_cast<int>(cur_dmg);
	}

	return data;
}