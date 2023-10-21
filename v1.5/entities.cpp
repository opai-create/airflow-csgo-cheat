#include "globals.hpp"
#include "animations.hpp"
#include "threaded_animstate.hpp"

mstudioseqdesc_t& c_studio_hdr::get_sequence_desc(int sequence)
{
	auto hdr_model = (void*)((std::uintptr_t)this + 4);
	if (hdr_model)
		return *offsets::get_sequence_desc.cast<mstudioseqdesc_t*(__thiscall*)(void*, int)>()(this, sequence);

	auto studiohdr = *reinterpret_cast<studiohdr_t**>(this);
	return *studiohdr->seq_desc(sequence);
}

bool c_base_entity::compute_hitbox_surrounding_box(vec3_t* mins, vec3_t* maxs)
{
	return offsets::compute_hitbox_surrounding_box.cast<bool(__thiscall*)(decltype(this), vec3_t*, vec3_t*)>()(this, mins, maxs);
}

void c_base_entity::set_abs_origin(const vec3_t& origin) 
{
	offsets::set_abs_origin.cast<void(__thiscall*)(decltype(this), const vec3_t&)>()(this, origin);
}

void c_base_entity::set_abs_angles(const vec3_t& ang) 
{
	offsets::set_abs_angles.cast<void(__thiscall*)(decltype(this), const vec3_t&)>()(this, ang);
}

void c_base_entity::calc_absolute_position() 
{
	offsets::calc_absolute_position.cast<void(__thiscall*)(decltype(this))>()(this);
}

void c_base_entity::invalidate_physics_recursive(int flags)
{
	offsets::invalidate_physics_recursive.cast<void(__thiscall*)(decltype(this), int)>()(this, flags);
}

int c_base_entity::get_sequence_activity(int sequence) 
{
	auto hdr = HACKS->model_info->get_studio_model(this->get_model());
	if (!hdr)
		return 0;

	return offsets::get_sequence_activity.cast<int(__fastcall*)(decltype(this), void*, int)>()(this, hdr, sequence);
}

int c_base_entity::lookup_bone(const char* index)
{
	return offsets::lookup_bone.cast<int(__thiscall*)(decltype(this), const char*)>()(this, index);
}

int c_base_entity::lookup_sequence(const char* name)
{
	return offsets::lookup_sequence.cast<int(__thiscall*)(decltype(this), const char*)>()(this, name);
}

void c_base_entity::attachments_helper()
{
	auto hdr = get_studio_hdr();
	if (!hdr)
		return;

	offsets::attachments_helper.cast<void(__thiscall*)(decltype(this), void*)>()(this, hdr);
}

std::vector<c_base_combat_weapon*> c_cs_player::get_weapons()
{
	auto my_weapons = this->my_weapons();

	std::vector<c_base_combat_weapon*> list{};
	for (auto i = 0; i < 64; ++i)
	{
		auto weapon = HACKS->entity_list->get_client_entity_handle(my_weapons[i]);
		if (weapon)
			list.push_back((c_base_combat_weapon*)weapon);
	}

	return list;
}

vec3_t c_cs_player::get_eye_position()
{
	vec3_t out{};
	offsets::weapon_shootpos.cast<float*(__thiscall*)(void*, vec3_t*)>()(this, &out);
	return out;
}

bool c_cs_player::is_breakable()
{
	static auto is_breakable_entity = offsets::is_breakable_entity.cast<bool(__thiscall*)(void*)>();
	if (!this)
		return false;

	auto client_class = get_client_class();
	if (!client_class)
		return is_breakable_entity(this);

	// on v4c map cheat shoots through this
	// ignore these fucks
	if (client_class->class_id == CBaseButton || client_class->class_id == CPhysicsProp)
		return false;

	// check if it's window by name (pasted from onetap)
	auto v3 = (int)client_class->network_name;
	if (*(DWORD*)v3 == XORN(0x65724243))
	{
		if (*(DWORD*)(v3 + 7) == XORN(0x53656C62))
			return 1;
	}

	if (*(DWORD*)v3 == XORN(0x73614243))
	{
		if (*(DWORD*)(v3 + 7) == XORN(0x79746974))
			return 1;
	}

	return is_breakable_entity(this);
}

void c_cs_player::run_pre_think()
{
	static auto physics_run_think = offsets::physics_run_think.cast<bool(__thiscall*)(void*, int)>();

	if (physics_run_think(this, 0))
		memory::get_virtual(this, XORN(PHYSICS_RUN_THINK_VFUNC)).cast<void(__thiscall*)(void*)>()(this);
}

void c_cs_player::run_think()
{
#ifdef LEGACY
	const auto next_think = (int*)((std::uintptr_t)this + XORN(0xF8));
#else
	const auto next_think = (int*)((std::uintptr_t)this + XORN(0xFC));
#endif

	static auto think = offsets::think.cast<void(__thiscall*)(void*, int)>();

	if (*next_think > 0 && *next_think <= this->tickbase())
	{
		*next_think = -1;

		think(this, 0);
		memory::get_virtual(this, XORN(UNK_THINK_VFUNC)).cast<void(__thiscall*)(void*)>()(this);
	}
}

void c_cs_player::run_post_think()
{
	static auto post_think_physics = offsets::post_think_physics.cast<bool(__thiscall*)(void*)>();
	static auto simulate_player_simulated_entities = offsets::simulate_player_simulated_entities.cast<void(__thiscall*)(void*)>();

	memory::get_virtual(HACKS->model_cache, XORN(MDL_CACHE_LOCK_VFUNC)).cast<void(__thiscall*)(void*)>()(HACKS->model_cache);

	if (this->is_alive())
	{
		memory::get_virtual(this, XORN(UPDATE_BOUNDS_VFUNC)).cast<void(__thiscall*)(void*)>()(this);

		if (this->flags().has(FL_ONGROUND))
			this->fall_velocity() = 0.f;

		if (this->sequence() == -1)
			memory::get_virtual(this, XORN(SET_SEQUENCE_VFUNC)).cast<void(__thiscall*)(void*, int)>()(this, 0);

		memory::get_virtual(this, XORN(UNK_POST_THINK_VFUNC)).cast<void(__thiscall*)(void*)>()(this);
		post_think_physics(this);
	}

	simulate_player_simulated_entities(this);
	memory::get_virtual(HACKS->model_cache, XORN(MDL_CACHE_UNLOCK_VFUNC)).cast<void(__thiscall*)(void*)>()(HACKS->model_cache);
}

void c_cs_player::update_weapon_dispatch_layers()
{
	auto hdr = get_studio_hdr();
	if (!hdr)
		return;

	auto weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(active_weapon()));
	if (weapon)
	{
		auto world_weapon = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(weapon->weapon_world_model()));
		if (world_weapon)
		{
			for (int i = 0; i < 13; i++)
			{
				auto layer = &animlayers()[i];
				layer->owner = this;
				layer->studio_hdr = hdr;

				if (layer->sequence >= 2 && layer->weight > 0.f)
				{
					auto weapon_hdr = world_weapon->get_studio_hdr();

					if (weapon_hdr)
						update_dispatch_layer(layer, weapon_hdr, layer->sequence);
				}
			}
		}
	}
}

void c_cs_player::force_update_animations(anims_t* anim)
{
	auto state = animstate();
	if (!state)
		return;

	for (int i = 0; i < 13; i++)
	{
		auto layer = &animlayers()[i];
		layer->owner = this;
		layer->studio_hdr = get_studio_hdr();
	}

	if (state->last_update_time == HACKS->global_vars->curtime)
		state->last_update_time = HACKS->global_vars->curtime + HACKS->global_vars->interval_per_tick;

	if (state->last_update_frame == HACKS->global_vars->framecount)
		state->last_update_frame = HACKS->global_vars->framecount - 1;

	{
		RESTORE(client_side_animation());
		client_side_animation() = true;

		{
			RESTORE(client_side_animation());
			client_side_animation() = true;

			anim->update_anims = false;

#ifdef LEGACY
			auto animation_time = this == HACKS->local ? TICKS_TO_TIME(HACKS->tickbase) : this->old_sim_time() + HACKS->global_vars->interval_per_tick;
#else
			auto animation_time = this == HACKS->local ? TICKS_TO_TIME(HACKS->tickbase) : this->sim_time();
#endif
			auto animation_ticks = this == HACKS->local ? HACKS->tickbase : TIME_TO_TICKS(animation_time);

			state->player = this;

			auto& angles = this == HACKS->local ? this->render_angles() : this->eye_angles();
			THREADED_STATE->update(this, state, angles.y, angles.x, animation_time, animation_ticks);

			// update latch interpolated variables & sequence
#ifdef LEGACY
			if (*reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + XORN(0x28AC)) != -1)
				memory::get_virtual(this, XORN(107)).cast<void(__thiscall*)(void*, int)>()(this, 1);
#else
			if (*reinterpret_cast<int*>(reinterpret_cast<std::uintptr_t>(this) + XORN(0x28C0)) != -1)
				memory::get_virtual(this, XORN(108)).cast<void(__thiscall*)(void*, int)>()(this, 1);
#endif
		}
	}
}

float c_cs_player::studio_set_pose_parameter(int index, float value, float& ctl_value)
{
	static auto get_pose_parameter = offsets::get_pose_parameter.cast<mstudioposeparamdesc_t * (__thiscall*)(c_studio_hdr*, int)>();

	auto hdr = get_studio_hdr();
	if (!hdr)
		return 0.f;

	if (index < 0 || index > 24)
		return 0.f;

	auto pose_parameter = get_pose_parameter(hdr, index);
	if (!pose_parameter)
		return 0.f;

	if (pose_parameter->loop)
	{
		float wrap = (pose_parameter->start + pose_parameter->end) / 2.0 + pose_parameter->loop / 2.0;
		float shift = pose_parameter->loop - wrap;

		value = value - pose_parameter->loop * floor((value + shift) / pose_parameter->loop);
	}

	ctl_value = (value - pose_parameter->start) / (pose_parameter->end - pose_parameter->start);

	if (ctl_value < 0) ctl_value = 0;
	if (ctl_value > 1) ctl_value = 1;

	return ctl_value * (pose_parameter->end - pose_parameter->start) + pose_parameter->start;
}

// for no reason main weapon (moveparent) may jitter
// if you shoot or enemy using break lc
// lets fix it
void c_cs_player::interpolate_moveparent_pos()
{
	// update moveparent + local pos
	calc_absolute_position();

	// mark that we gonna change our pos
	invalidate_physics_recursive(0x1);

	auto moveparent = (c_base_entity*)(HACKS->entity_list->get_client_entity_handle(move_parent()));
	if (moveparent)
	{
		// set new moveparent pos to interpolated one
		auto& frame = moveparent->coordinate_frame();
		frame.set_origin(get_abs_origin());
	}
}

void c_cs_player::setup_uninterpolated_bones(anims_t* anim, matrix3x4_t* matrix)
{
	invalidate_bone_cache();

	RESTORE(entity_flags());
	RESTORE(ik_ctx());
	RESTORE(effects());
	RESTORE(HACKS->global_vars->framecount);
	RESTORE(HACKS->global_vars->curtime);

	float curtime = this == HACKS->local ? TICKS_TO_TIME(HACKS->tickbase) : sim_time();
	HACKS->global_vars->curtime = curtime;

	// pass checks for existing ik ctx and skip calcs then
	entity_flags().force(2);
	ik_ctx() = 0;

	// skip bone lerp
	effects().force(8);

	HACKS->global_vars->framecount = -999;

	anim->setup_bones = true;
	setup_bones(matrix, matrix == nullptr ? -1 : 128, 0x7FF00, curtime);
	anim->setup_bones = false;

#ifndef LEGACY
	anim->clamp_bones_in_bbox = true;
	offsets::clamp_bones_in_bbox.cast<void(__thiscall*)(void*, matrix3x4_t*, int)>()(this, matrix ? matrix : bone_cache().base(), 0x7FF00);
	anim->clamp_bones_in_bbox = false;
#endif
}

vec3_t c_cs_player::get_hitbox_position(int hitbox, matrix3x4_t* matrix)
{
	if (!matrix)
		matrix = bone_cache().base();

	auto hdr = HACKS->model_info->get_studio_model(get_model());

	if (!hdr)
		return {};

	auto hitbox_set = hdr->hitbox_set(this->hitbox_set());

	if (!hitbox_set)
		return {};

	auto hdr_hitbox = hitbox_set->hitbox(hitbox);

	if (!hdr_hitbox)
		return {};

	vec3_t min, max;

	math::vector_transform(hdr_hitbox->min, matrix[hdr_hitbox->bone], min);
	math::vector_transform(hdr_hitbox->max, matrix[hdr_hitbox->bone], max);

	return (min + max) * 0.5f;
}