#include "globals.hpp"
#include "penetration.hpp"
#include "entlistener.hpp"
#include "clip_ray_to_hitbox.hpp"

namespace penetration
{
	INLINE void scale_damage(const int hitgroup, c_cs_player* entity, const float weapon_armor_ratio, const float weapon_head_shot_multiplier, float& damage)
	{
		const bool has_heavy_armor = entity->has_heavy_armor();

		static auto mp_damage_scale_ct_head = HACKS->convars.mp_damage_scale_ct_head;
		static auto mp_damage_scale_t_head = HACKS->convars.mp_damage_scale_t_head;

		static auto mp_damage_scale_ct_body = HACKS->convars.mp_damage_scale_ct_body;
		static auto mp_damage_scale_t_body = HACKS->convars.mp_damage_scale_t_body;

		float head_damage_scale = entity->team() == TEAM_CT
			? mp_damage_scale_ct_head->get_float() : entity->team() == TEAM_TT
			? mp_damage_scale_t_head->get_float() : 1.0f;

		const float body_damage_scale = entity->team() == TEAM_CT
			? mp_damage_scale_ct_body->get_float() : entity->team() == TEAM_TT
			? mp_damage_scale_t_body->get_float() : 1.0f;

		if (has_heavy_armor)
			head_damage_scale *= 0.5f;

		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			damage *= 4.0f * head_damage_scale;
			break;
		case HITGROUP_CHEST:
			damage *= body_damage_scale;
			break;
		case HITGROUP_GEAR:
			damage *= body_damage_scale;
			break;
		case HITGROUP_STOMACH:
			damage *= 1.25f * body_damage_scale;
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			damage *= body_damage_scale;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			damage *= 0.75f * body_damage_scale;
			break;
		default:
			break;
		}

		if (entity->is_armored(hitgroup))
		{
			const int armor = entity->armor_value();
			float heavy_armor_bonus = 1.0f, armor_bonus = 0.5f, armor_ratio = weapon_armor_ratio * 0.5f;

			if (has_heavy_armor)
			{
				heavy_armor_bonus = 0.25f;
				armor_bonus = 0.33f;
				armor_ratio *= 0.20f;
			}

			float damage_to_health = damage * armor_ratio;
			if (const float damage_to_armor = (damage - damage_to_health) * (heavy_armor_bonus * armor_bonus); damage_to_armor > static_cast<float>(armor))
				damage_to_health = damage - static_cast<float>(armor) / armor_bonus;

			damage = damage_to_health;
		}
	}

	INLINE float distance_to_ray(const vec3_t& position, const vec3_t& source, const vec3_t& dest, float* along = nullptr, vec3_t* point_on_ray = nullptr)
	{
		vec3_t to = position - source;
		vec3_t direction = dest - source;
		float length = direction.normalized_float();

		float range_along = direction.dot(to);
		if (along)
			*along = range_along;

		float range;
		if (range_along < 0.0f)
		{
			range = -to.length();

			if (point_on_ray)
				*point_on_ray = source;
		}
		else if (range_along > length)
		{
			range = -(position - dest).length();

			if (point_on_ray)
				*point_on_ray = dest;
		}
		else
		{
			auto on_ray = source + direction * range_along;
			range = (position - on_ray).length();

			if (point_on_ray)
				*point_on_ray = on_ray;
		}

		return range;
	}

	INLINE void clip_trace_to_players(const vec3_t& start, const vec3_t& end, unsigned int mask, i_trace_filter* filter, c_game_trace* tr, should_hit_fn should_hit)
	{
		float smallest_fraction = tr->fraction;

		LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
		{
			if (!player || player == HACKS->local)
				return;

			if (player->dormant() || !player->is_alive())
				return;

			if (should_hit && !should_hit(player, mask))
				return;

			auto collideable = player->get_collideable();
			if (!collideable)
				return;

			auto obb_center = (collideable->get_mins() + collideable->get_maxs()) / 2.f;
			auto position = obb_center + player->origin();
			float range = distance_to_ray(position, start, end);

			if (range < 0.f || range > 60.f)
				return;

			c_game_trace trace;
			HACKS->engine_trace->clip_ray_to_entity(ray_t(start, end), mask, player, &trace);

			if (trace.fraction < smallest_fraction)
			{
				*tr = trace;
				smallest_fraction = trace.fraction;
			}
		}, false);
	}

	INLINE void clip_trace_to_player(const vec3_t start, const vec3_t& end, unsigned int mask, i_trace_filter* filter, c_game_trace* tr, c_cs_player* player, should_hit_fn should_hit)
	{
		if (should_hit && !should_hit(player, mask))
			return;

		auto collideable = player->get_collideable();
		if (!collideable)
			return;

		auto obb_center = (collideable->get_mins() + collideable->get_maxs()) / 2.f;
		auto position = obb_center + player->origin();
		float range = distance_to_ray(position, start, end);

		if (range < 0.f || range > 60.f)
			return;

		c_game_trace trace;
		HACKS->engine_trace->clip_ray_to_entity(ray_t(start, end), mask, player, &trace);

		if (tr->fraction > trace.fraction)
			*tr = trace;
	}

	bool trace_to_exit(c_game_trace* enter_trace, vec3_t start, vec3_t direction, c_game_trace* exit_trace, c_cs_player* player)
	{
		static auto sv_clip_penetration_traces_to_players = HACKS->convars.sv_clip_penetration_traces_to_players;

		constexpr float MAX_DISTANCE = 90.f, STEP_SIZE = 4.f;
		float current_distance = 0.f;

		int first_contents = 0;

		do
		{
			current_distance += STEP_SIZE;
			auto new_end = start + (direction * current_distance);

			if (!first_contents)
				first_contents = HACKS->engine_trace->get_point_contents(new_end, MASK_SHOT_PLAYER);

			int point_contents = HACKS->engine_trace->get_point_contents(new_end, MASK_SHOT_PLAYER);

			if (!(point_contents & MASK_SHOT_HULL) || ((point_contents & CONTENTS_HITBOX) && point_contents != first_contents))
			{
				auto new_start = new_end - (direction * STEP_SIZE);

				HACKS->engine_trace->trace_ray(ray_t(new_end, new_start), MASK_SHOT_PLAYER, nullptr, exit_trace);

				//	HACKS->debug_overlay->add_line_overlay(new_end, new_start, 255, 25, 255, 1.f, 0.1f);

				if (exit_trace->start_solid && exit_trace->surface.flags & SURF_HITBOX)
				{
#ifdef LEGACY
					c_trace_filter_simple filter(player);
#else
					c_trace_filter_skip_two_entities filter(player, exit_trace->entity);
#endif
					HACKS->engine_trace->trace_ray(ray_t(start, new_start), MASK_SHOT_PLAYER, (i_trace_filter*)&filter, exit_trace);

					if (exit_trace->did_hit() && !exit_trace->start_solid)
					{
						new_end = exit_trace->end;
						return true;
					}

					continue;
				}
				else {

					if (!exit_trace->did_hit() || exit_trace->start_solid)
					{
						if (exit_trace->entity)
						{
							if (enter_trace->did_hit_non_world_entity())
							{
								if (((c_cs_player*)enter_trace->entity)->is_breakable())
								{
									exit_trace = enter_trace;
									exit_trace->end = start + direction;
									return true;
								}
							}
						}
					}
					else
					{
						if (((c_cs_player*)enter_trace->entity)->is_breakable() && ((c_cs_player*)exit_trace->entity)->is_breakable())
							return true;

						if (enter_trace->surface.flags & SURF_NODRAW || (!(exit_trace->surface.flags & SURF_NODRAW) && exit_trace->plane.normal.dot(direction) <= 1.f))
						{
							const float mult_amount = exit_trace->fraction * 4.f;

							// get the real end pos
							new_start -= direction * mult_amount;
							return true;
						}

						continue;
					}
				}
			}
		} while (current_distance <= MAX_DISTANCE);

		return false;
	}

	bool handle_bullet_penetration(surface_data_t* surface_data, c_game_trace& enter_trace, vec3_t& source, vec3_t& direction, float& penetration, float& current_damage, c_cs_player* player, int& penetration_count)
	{
		static auto ff_damage_reduction_bullets = HACKS->convars.ff_damage_reduction_bullets;
		static auto ff_damage_bullet_penetration = HACKS->convars.ff_damage_bullet_penetration;

		int enter_material = surface_data->game.material;

		bool solid_surf = ((enter_trace.contents >> 3) & CONTENTS_SOLID);
		bool light_surf = ((enter_trace.surface.flags >> 7) & SURF_LIGHT);
		bool contentes_grate = enter_trace.contents & CONTENTS_GRATE;
		bool draw_surf = !!(enter_trace.surface.flags & (SURF_NODRAW));

		if (penetration_count == 0 &&
			!contentes_grate &&
			!draw_surf &&
			enter_material != CHAR_TEX_GRATE &&
			enter_material != CHAR_TEX_GLASS)
			return true;

		if (penetration <= 0.f || penetration_count == 0)
			return true;

		c_game_trace exit_trace = { };
		if (!trace_to_exit(&enter_trace, enter_trace.end, direction, &exit_trace, player))
		{
			if ((HACKS->engine_trace->get_point_contents(enter_trace.end, MASK_SHOT_HULL) & MASK_SHOT_HULL) == 0)
				return true;
		}

		const auto exit_surface_data = HACKS->phys_surface_props->get_surface_data(exit_trace.surface.surface_props);
		if (!exit_surface_data)
			return true;

		const float enter_penetration_modifier = surface_data->game.penetration_modifier;
		const float exit_penetration_modifier = exit_surface_data->game.penetration_modifier;

		const int exit_material = exit_surface_data->game.material;

		float damage_modifier = 0.16f;
		float penetration_modifier = 0.f;

		if (enter_material == CHAR_TEX_GRATE || enter_material == CHAR_TEX_GLASS)
		{
			damage_modifier = 0.05f;
			penetration_modifier = 3.0f;
		}
		else if (solid_surf || light_surf)
		{
			damage_modifier = 0.16f;
			penetration_modifier = 1.0f;
		}
		else if (enter_material == CHAR_TEX_FLESH
			&& (player->is_teammate(true, (c_cs_player*)enter_trace.entity))
			&& ff_damage_reduction_bullets->get_float() >= 0.f)
		{
			if (ff_damage_bullet_penetration->get_float() == 0.f)
				return true;

			penetration_modifier = ff_damage_bullet_penetration->get_float();
			damage_modifier = 0.16f;
		}
		else
		{
			damage_modifier = 0.16f;
			penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;
		}

		if (enter_material == exit_material)
		{
			if (exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD)
				penetration_modifier = 3.f;
			else if (exit_material == CHAR_TEX_PLASTIC)
				penetration_modifier = 2.f;
		}

		float trace_dist = (exit_trace.end - enter_trace.end).length_sqr();
		float pen_mod = std::max(0.f, 1.f / penetration_modifier);

		float dmg_chunk = current_damage * damage_modifier;
		float pen_weapon_mod = dmg_chunk + std::max(0.f, (3.f / penetration) * 1.25f) * (pen_mod * 3.f);

		float lost_dmg_object = ((pen_mod * (trace_dist)) / 24);
		float lost_damage = pen_weapon_mod + lost_dmg_object;

		if (lost_damage > current_damage)
			return true;

		if (lost_damage > 0.f)
			current_damage -= lost_damage;

		if (current_damage < 1.0f)
			return true;

		source = exit_trace.end;
		--penetration_count;
		return false;
	}

	enum hit_group_type
	{
		hgt_head,
		hgt_stomach,
		hgt_chest,
		hgt_arms,
		hgt_general,
		hgt_legs,
		hgt_max
	};

	struct hit_group_result
	{
		c_game_trace hit_group{};
		int hitbox{};
		int hit_side{};
	};

	bool test_hitboxes(c_cs_player* player, c_game_trace* trace, const ray_t& ray, matrix3x4_t* matrix)
	{
		auto bones = matrix != nullptr ? matrix : player->bone_cache().base();

		// reset trace
		trace->fraction = 1.f;
		trace->start_solid = false;

		// init array
		hit_group_result results[hgt_max]{};
		for (auto i = 0; i < hgt_max; i++)
		{
			auto& r = results[i];
			r.hit_group = *trace;
			r.hitbox = -1;
			r.hit_side = -1;
		}

		const auto hdr = HACKS->model_info->get_studio_model(player->get_model());
		if (!hdr)
			return false;
		
		auto new_hdr = player->get_studio_hdr();
		if (!new_hdr)
			return false;

		const auto hb_set = hdr->hitbox_set(player->hitbox_set());
		if (!hb_set)
			return false;

		const auto scale = player->model_scale();

		auto position = player->get_abs_origin();

		// process hitboxes
		for (auto i = 0; i < hb_set->num_hitboxes; i++)
		{
			const auto hb = hb_set->hitbox(i);

			// skip if bone doesn't match
			if (!(hdr->bone(hb->bone)->contents & (CONTENTS_SOLID | CONTENTS_HITBOX | CONTENTS_DEBRIS)))
				continue;

			// determine result
			auto hg_result = &results[hgt_general];
			switch (hb->group)
			{
			case 1: hg_result = &results[hgt_head]; break;
			case 3: hg_result = &results[hgt_stomach]; break;
			case 2: hg_result = &results[hgt_chest]; break;
			case 4:
			case 5:
				hg_result = &results[hgt_arms]; break;
			case 6:
			case 7:
				hg_result = &results[hgt_legs]; break;
			}

			auto mtx_copy = bones[hb->bone];

			// rotate matrix
			matrix3x4_t orientation{};
			orientation.angle_matrix(hb->rotation);
			mtx_copy.multiply(orientation);

			auto side = -1;
			if (scale < 1.f - FLT_EPSILON || scale > 1.f + FLT_EPSILON)
			{
				// move matrix
				const auto inv_scale = 1.f / scale;
				const auto bone_origin = mtx_copy.get_origin();

				auto new_origin = bone_origin - position;
				new_origin *= inv_scale;
				new_origin += position;

				mtx_copy.set_origin(new_origin);

				// scale
				mtx_copy.scale(0, inv_scale);
				mtx_copy.scale(1, inv_scale);
				mtx_copy.scale(2, inv_scale);

				// recalculate ray
				auto new_start = ray.start - position;
				new_start *= inv_scale;
				new_start += position;

				const auto delta = ray.delta * inv_scale;

				// clip
				ray_t new_ray(new_start, new_start + delta);

				HACKS->debug_overlay->add_text_overlay(new_ray.start, 0.1f, "ASDASD");

				side = ClipRayToHitbox(new_ray, hb, mtx_copy, hg_result->hit_group);
			}
			else
				side = ClipRayToHitbox(ray, hb, mtx_copy, hg_result->hit_group);

			// if there was a side, update info
			if (side >= 0)
			{
				hg_result->hitbox = i;
				hg_result->hit_side = side;
			}
		}

		// csgo specific: test for headshot if goes through body
		auto& r_head = results[hgt_head];
		if (r_head.hitbox >= 0)
		{
			// We have a potential headshot, check if it's penetrating via stomach or chest
			for (int i = hgt_stomach; i <= hgt_chest; i++)
			{
				auto& r = results[i];
				if (r.hit_group.fraction < r_head.hit_group.fraction)
				{
					r_head.hitbox = -1;
					break;
				}
			}
		}

		auto hitbox = -1;
		auto hit_side = -1;

		// pick by damage
		for (auto i = 0; i < hgt_max; i++)
		{
			const auto& r = results[i];
			if (r.hitbox >= 0)
			{
				hitbox = r.hitbox;
				hit_side = r.hit_side;
				*trace = r.hit_group;
				break;
			}
		}

		// check if we didn't hit anything
		if (hitbox < 0)
			return false;

		const auto hb = hb_set->hitbox(hitbox);
		const auto bone = hdr->bone(hb->bone);

		// write trace data
		math::vector_multiply(ray.start, trace->fraction, ray.delta, trace->end);
		trace->hitgroup = hb->group;
		trace->hitbox = hitbox;
		trace->contents = bone->contents | CONTENTS_HITBOX;
		trace->physics_bone = bone->physics_bone;
		trace->surface.name = CXOR("**studio**");
		trace->surface.flags = SURF_HITBOX;
		trace->surface.surface_props = bone->surface_prop_lookup;

		const auto& mtx = bones[hb->bone];
		if (hit_side >= 3)
		{
			hit_side -= 3;
			trace->plane.normal.x = mtx.mat[0][hit_side];
			trace->plane.normal.y = mtx.mat[1][hit_side];
			trace->plane.normal.z = mtx.mat[2][hit_side];
		}
		else
		{
			trace->plane.normal.x = -mtx.mat[0][hit_side];
			trace->plane.normal.y = -mtx.mat[1][hit_side];
			trace->plane.normal.z = -mtx.mat[2][hit_side];
		}

		trace->plane.dist = trace->end.dot(trace->plane.normal);
		trace->plane.type = 3;

		return true;
	}

	bullet_t simulate(c_cs_player* shooter, c_cs_player* target, vec3_t source, const vec3_t& dest, bool ignore_damage, bool simple)
	{
		c_base_combat_weapon* weapon = nullptr;
		weapon_info_t* weapon_info = nullptr;

		if (shooter == HACKS->local)
		{
			weapon = HACKS->weapon;
			weapon_info = HACKS->weapon_info;
		}
		else
		{
			weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(shooter->active_weapon()));
			if (weapon)
				weapon_info = HACKS->weapon_system->get_weapon_data(weapon->item_definition_index());
		}

		if (!weapon || !weapon_info)
			return {};

		auto direction = dest - source;
		direction = direction.normalized();

		bullet_t out{};
		out.penetration_count = simple ? 1 : 4;

		float current_damage = (float)weapon_info->dmg;
		float current_distance = 0.f;
		float max_distance = weapon_info->range;

		c_game_trace enter_trace{};

		c_trace_filter_simple filter(shooter);

		while (current_damage > 0.f)
		{
			max_distance -= current_distance;

			auto extended_dest = source + direction * max_distance;
			auto pass_filter = (i_trace_filter*)&filter;

			HACKS->engine_trace->trace_ray(ray_t(source, extended_dest), MASK_SHOT_PLAYER, pass_filter, &enter_trace);

			if (target)
				clip_trace_to_player(source, extended_dest + direction * 40.f, MASK_SHOT_PLAYER, pass_filter, &enter_trace, target, filter.should_hit);
			else
				clip_trace_to_players(source, extended_dest + direction * 40.f, MASK_SHOT_PLAYER, pass_filter, &enter_trace, filter.should_hit);

			auto enter_surface_data = HACKS->phys_surface_props->get_surface_data(enter_trace.surface.surface_props);
			float enter_penetration_modifier = enter_surface_data->game.penetration_modifier;

			if (enter_trace.fraction == 1.f)
				break;

			current_distance += enter_trace.fraction * max_distance;
			current_damage *= std::pow(weapon_info->range_modifier, current_distance / 500.f);

			if (current_distance > 3000.f || enter_penetration_modifier < 0.1f)
				break;

			if (target && enter_trace.entity)
			{
				auto player_entity = (c_cs_player*)enter_trace.entity;
				auto is_player = player_entity->is_player();

				if (player_entity == target || is_player && !player_entity->is_teammate(true, shooter)
					&& enter_trace.hitgroup != HITGROUP_GENERIC && enter_trace.hitgroup != HITGROUP_GEAR)
				{
					out.hitbox = enter_trace.hitbox;
					out.hitgroup = enter_trace.hitgroup;
					out.traced_target = player_entity;

					if (weapon->is_taser())
						current_damage *= 0.92f;
					else
						scale_damage(out.hitgroup, player_entity, weapon_info->armor_ratio, 4.f, current_damage);

					out.damage = (int)current_damage;
					return out;
				}
			}

			if (handle_bullet_penetration(enter_surface_data, enter_trace, source, direction, weapon_info->penetration, current_damage, shooter, out.penetration_count))
				break;

			if (ignore_damage)
				out.damage = (int)current_damage;
		}

		return out;
	}
}