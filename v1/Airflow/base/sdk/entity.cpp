#include "entity.h"
#include "c_animstate.h"
#include "c_csplayerresource.h"

#include "../other/game_functions.h"

#include "../../functions/features.h"

c_baseentity* c_baseentity::get_move_parent()
{
	c_baseentity* move_parent = (c_baseentity*)interfaces::entity_list->get_entity_handle(this->move_parent());
	return move_parent;
}

c_baseentity* c_baseentity::get_view_model()
{
	c_baseentity* view_model = (c_baseentity*)interfaces::entity_list->get_entity_handle(this->view_model());
	return view_model;
}

c_csplayer* c_baseentity::get_ragdoll_player()
{
	c_csplayer* view_model = (c_csplayer*)interfaces::entity_list->get_entity_handle(this->ragdoll_player());
	return view_model;
}

c_basecombatweapon* c_baseentity::get_view_model_weapon()
{
	c_basecombatweapon* weapon_world_model = (c_basecombatweapon*)interfaces::entity_list->get_entity_handle(this->viewmodel_weapon());
	return weapon_world_model;
}

void c_baseentity::compute_hitbox_surrounding_box(vector3d* mins, vector3d* maxs)
{
	func_ptrs::compute_hitbox_surrounding_box(this, mins, maxs);
}

void c_baseentity::set_abs_origin(const vector3d& origin)
{
	func_ptrs::set_abs_origin(this, origin);
}

void c_baseentity::set_abs_angles(const vector3d& ang)
{
	func_ptrs::set_abs_angles(this, ang);
}

void c_baseentity::calc_absolute_position()
{
	func_ptrs::calc_absolute_position(this);
}

int c_baseentity::get_sequence_activity(int sequence)
{
	using fn = int(__fastcall*)(void*, void*, int);
	auto hdr = interfaces::model_info->get_studio_model(this->get_model());
	if (!hdr)
		return 0;

	return patterns::get_sequence_activity.as< fn >()(this, hdr, sequence);
}

c_csplayer* c_csplayer::get_observer_target()
{
	c_csplayer* observer_target = (c_csplayer*)interfaces::entity_list->get_entity_handle(this->observer_target());
	return observer_target;
}

c_csplayer* c_csplayer::get_ragdoll()
{
	c_csplayer* ragdoll = (c_csplayer*)interfaces::entity_list->get_entity_handle(this->ragdoll());
	return ragdoll;
}

bool c_csplayer::should_fix_modify_eye_pos()
{
	auto state = this->animstate();
	if (!state)
		return false;

	return state->landing || state->anim_duck_amount != 0.0f || !(flags() & fl_onground);
}

void c_csplayer::update_weapon_dispatch_layers()
{
	auto weapon = this->get_active_weapon();
	if (weapon)
	{
		auto world_weapon = (c_csplayer*)weapon->get_weapon_world_model();
		if (world_weapon)
		{
			for (int i = 0; i < 13; i++)
			{
				auto layer = &this->anim_overlay()[i];
				layer->owner = this;
				layer->studio_hdr = this->get_studio_hdr();

				if (layer->sequence >= 2 && layer->weight > 0.f)
					this->update_dispatch_layer(layer, world_weapon->get_studio_hdr(), layer->sequence);
			}
		}
	}
}

vector3d c_csplayer::get_bone_position(int bone_index)
{
	matrix3x4_t* matrix = this->bone_accessor()->bones;
	return matrix[bone_index].get_origin();
}

vector3d c_csplayer::get_hitbox_position(int hitbox, matrix3x4_t* matrix)
{
	if (!matrix)
		matrix = this->bone_cache().base();

	auto hdr = interfaces::model_info->get_studio_model(this->get_model());

	if (!hdr)
		return vector3d();

	auto hitbox_set = hdr->get_hitbox_set(this->hitbox_set());

	if (!hitbox_set || !hitbox_set->hitboxes)
		return vector3d();

	auto hdr_hitbox = hitbox_set->get_hitbox(hitbox);

	if (!hdr_hitbox)
		return vector3d();

	vector3d min, max;

	math::vector_transform(hdr_hitbox->bbmin, matrix[hdr_hitbox->bone], min);
	math::vector_transform(hdr_hitbox->bbmax, matrix[hdr_hitbox->bone], max);

	return (min + max) * 0.5f;
}

c_basecombatweapon* c_csplayer::get_active_weapon()
{
	c_basecombatweapon* active_weapon = (c_basecombatweapon*)interfaces::entity_list->get_entity_handle(this->active_weapon());
	return active_weapon;
}

c_studiohdr* c_csplayer::get_studio_hdr()
{
	return *(c_studiohdr**)((uintptr_t)this + *patterns::studio_hdr_ptr.add(2).as< uintptr_t* >() + 4);
}

void c_csplayer::run_pre_think()
{
	if (func_ptrs::physics_run_think(this, 0))
	{
		using pre_think_fn = void(__thiscall*)(void*);
		g_memory->getvfunc< pre_think_fn >(this, 318)(this);
	}
}

void c_csplayer::run_think()
{
	const auto next_think = (int*)((uintptr_t)this + 0xFC);
	if (*next_think > 0 && *next_think <= this->tickbase())
	{
		*next_think = -1;

		// unk func inside RunThink()
		func_ptrs::think(this, 0);

		using think_fn = void(__thiscall*)(void*);
		g_memory->getvfunc< think_fn >(this, 139)(this);
	}
}

void c_csplayer::post_think()
{
	g_memory->getvfunc< void(__thiscall*)(void*) >(interfaces::model_cache, 33)(interfaces::model_cache);

	if (this->is_alive())
	{
		g_memory->getvfunc< void(__thiscall*)(void*) >(this, 340)(this);

		if (this->flags() & fl_onground)
			this->fall_velocity() = 0.f;

		if (this->sequence() == -1)
			g_memory->getvfunc< void(__thiscall*)(void*, int) >(this, 219)(this, 0);

		g_memory->getvfunc< void(__thiscall*)(void*) >(this, 220)(this);
		func_ptrs::post_think_physics(this);
	}

	func_ptrs::simulate_player_simulated_entities(this);
	g_memory->getvfunc< void(__thiscall*)(void*) >(interfaces::model_cache, 34)(interfaces::model_cache);
}

void c_csplayer::draw_server_hitbox()
{
	auto duration = interfaces::global_vars->interval_per_tick * 2.f;

	auto server_player = this->get_server_edict();
	if (server_player)
	{
		static auto call = patterns::draw_hitbox.as< uintptr_t* >();

		float current_duration = duration;

		__asm
		{
			pushad
			movss xmm1, current_duration
			push 1
			mov ecx, server_player
			call call
			popad
		}
	}
}

uint8_t* c_csplayer::get_server_edict()
{
	static uintptr_t server_globals = **patterns::server_edict.add(0x2).as< uintptr_t** >();
	int max_clients = *(int*)((uintptr_t)server_globals + 0x18);
	int index = this->index();
	if (index > 0 && max_clients >= 1)
	{
		if (index <= max_clients)
		{
			int v10 = index * 16;
			uintptr_t v11 = *(uintptr_t*)(server_globals + 96);
			if (v11)
			{
				if (!((*(uintptr_t*)(v11 + v10) >> 1) & 1))
				{
					uintptr_t v12 = *(uintptr_t*)(v10 + v11 + 12);
					if (v12)
					{
						uint8_t* ret = nullptr;
						__asm
						{
							pushad
							mov ecx, v12
							mov eax, dword ptr[ecx]
							call dword ptr[eax + 0x14]
							mov ret, eax
							popad
						}

						return ret;
					}
				}
			}
		}
	}
	return nullptr;
}

void c_csplayer::force_update()
{
	auto state = this->animstate();
	if (!state)
		return;

	if (state->last_update_time == interfaces::global_vars->cur_time)
		--state->last_update_time;

	if (state->last_update_frame == interfaces::global_vars->frame_count)
		--state->last_update_frame;

	auto weapon = this->get_active_weapon();
	if (weapon)
		state->weapon_last = weapon;

	for (auto i = 0; i < 13; i++) {
		auto layer = &this->anim_overlay()[i];
		layer->owner = this;
		layer->studio_hdr = this->get_studio_hdr();
	}

	bool old_update = this->client_side_animation();

	g_ctx.update[this->index()] = true;

	this->client_side_animation() = true;

	state->update(this == g_ctx.local ? this->render_angles() : this->eye_angles());

	g_ctx.update[this->index()] = false;

	this->client_side_animation() = old_update;
}

// for no reason main weapon (moveparent) may jitter
// if you shoot or enemy using break lc
// lets fix it
void c_csplayer::interpolate_moveparent_pos()
{
	this->e_flags() |= 1 << 11;

	// update moveparent + local pos
	this->calc_absolute_position();

	// mark that we gonna change our pos
	this->invalidate_physics_recursive(0x1);

	auto moveparent = this->get_move_parent();
	if (moveparent)
	{
		// set new moveparent pos to interpolated one
		auto& frame = moveparent->coordinate_frame();

		frame.set_origin(this->get_abs_origin());
	}
}

// bone rebuild just sucks
// i prefer to fix client's one
// currently for fix it requires disabling all bone interpolation or useless calculations
// like ik context or etc.
void c_csplayer::setup_uninterpolated_bones(matrix3x4_t* matrix, vector3d angs)
{
	this->invalidate_bone_cache();

	auto a = this->ent_flags();
	auto c = this->effects();

	// pass checks for existing ik ctx and skip calcs then
	this->ent_flags() |= 2;

	// skip bone lerp
	this->disable_interpolation();

	backup_globals(frame_count);

	interfaces::global_vars->frame_count = -999;

	g_ctx.setup_bones[this->index()] = true;
	this->setup_bones(matrix, matrix == nullptr ? -1 : 128, 0x7FF00, this == g_ctx.local ? math::ticks_to_time(this->tickbase()) : this->simulation_time());
	g_ctx.setup_bones[this->index()] = false;

	restore_globals(frame_count);

	this->ent_flags() = a;
	this->effects() = c;
}

void c_csplayer::invalidate_physics_recursive(int flags)
{
	func_ptrs::invalidate_physics_recursive(this, flags);
}

void c_csplayer::attachments_helper()
{
	auto hdr = this->get_studio_hdr();
	if (!hdr)
		return;

	func_ptrs::attachments_helper(this, hdr);
}

vector3d c_csplayer::get_eye_position()
{
	vector3d out{};
	func_ptrs::weapon_shootpos(this, &out);
	return out;
}

int& c_csplayer::personal_data_public_level()
{
	static int invalid = -1;
	if (offsets::m_nPersonaDataPublicLevel != 0)
		return *(int*)(interfaces::player_resource + offsets::m_nPersonaDataPublicLevel + this->index() * 4);

	return invalid;
}

vector3d c_csplayer::get_render_eye_position()
{
	vector3d out{};
	func_ptrs::weapon_shootpos(this, &out);
	return out;
}

std::vector< c_basecombatweapon* > c_csplayer::get_weapons()
{
	int* m_hMyWeapons = reinterpret_cast<int*>((DWORD)this + 0x2E08);
	std::vector< c_basecombatweapon* > list = {};
	for (auto i = 0; i < 64; ++i)
	{
		auto Weapon = interfaces::entity_list->get_entity_handle(m_hMyWeapons[i]);
		if (Weapon)
		{
			list.push_back((c_basecombatweapon*)Weapon);
		}
	}
	return list;
}

weapon_info_t* c_basecombatweapon::get_weapon_info()
{
	if ((*(uintptr_t*)this) == 0)
		return nullptr;

	short idx = this->item_definition_index();
	return interfaces::weapon_system->get_weapon_data(idx);
}

c_basecombatweapon* c_basecombatweapon::get_weapon_world_model()
{
	c_basecombatweapon* weapon_world_model = (c_basecombatweapon*)interfaces::entity_list->get_entity_handle(this->weapon_world_model());
	return weapon_world_model;
}

unsigned int find_in_datamap(datamap_t* map, const char* name)
{
	while (map)
	{
		for (int i = 0; i < map->data_num_fields; i++)
		{
			if (map->data_desc[i].field_name == nullptr)
				continue;

			if (!strcmp(name, map->data_desc[i].field_name))
				return map->data_desc[i].field_offset;

			if (map->data_desc[i].field_type == field_embedded)
			{
				if (map->data_desc[i].td)
				{
					unsigned int offset = {};

					if ((offset = find_in_datamap(map->data_desc[i].td, name)) != 0)
						return offset;
				}
			}
		}
		map = map->base_map;
	}

	return 0;
}

void* c_studiohdr::get_sequence_desc(int sequence)
{
	void* sequence_desc = nullptr;
	auto hdr_model = (void*)((std::uintptr_t)this + 4);
	if (hdr_model)
		sequence_desc = patterns::get_sequence_desc.as<void* (__thiscall*)(void*, int)>()(this, sequence);
	else
	{
		auto hdr_sequence = *(int*)((std::uintptr_t)this + 0xBC);
		auto hdr_unk = *(int*)((std::uintptr_t)this + 0xC0);

		if (sequence < 0 || sequence >= hdr_sequence)
			sequence = 0;

		sequence_desc = *(void**)((std::uintptr_t)this + hdr_unk + 212 * sequence);
	}

	return sequence_desc;
}