#pragma once
#include "../tools/math.h"

enum dispsurf_t
{
	dispsurf_flag_surface = (1 << 0),
	dispsurf_flag_walkable = (1 << 1),
	dispsurf_flag_buildable = (1 << 2),
	dispsurf_flag_surfprop1 = (1 << 3),
	dispsurf_flag_surfprop2 = (1 << 4),
};

enum contents_t
{
	contents_empty = 0,

	contents_solid = 0x1,
	contents_window = 0x2,
	contents_aux = 0x4,
	contents_grate = 0x8,
	contents_slime = 0x10,
	contents_water = 0x20,
	contents_blocklos = 0x40,
	contents_opaque = 0x80,
	last_visible_contents = contents_opaque,

	all_visible_contents = (last_visible_contents | (last_visible_contents - 1)),

	contents_testfogvolume = 0x100,
	contents_unused = 0x200,
	contents_blocklight = 0x400,
	contents_team1 = 0x800,
	contents_team2 = 0x1000,
	contents_ignore_nodraw_opaque = 0x2000,
	contents_moveable = 0x4000,
	contents_areaportal = 0x8000,
	contents_playerclip = 0x10000,
	contents_monsterclip = 0x20000,

	contents_current_0 = 0x40000,
	contents_current_90 = 0x80000,
	contents_current_180 = 0x100000,
	contents_current_270 = 0x200000,
	contents_current_up = 0x400000,
	contents_current_down = 0x800000,

	contents_origin = 0x1000000,

	contents_monster = 0x2000000,
	contents_debris = 0x4000000,
	contents_detail = 0x8000000,
	contents_translucent = 0x10000000,
	contents_ladder = 0x20000000,
	contents_hitbox = 0x40000000,
};

enum surf_t
{
	surf_light = 0x0001,
	surf_sky2d = 0x0002,
	surf_sky = 0x0004,
	surf_warp = 0x0008,
	surf_trans = 0x0010,
	surf_noportal = 0x0020,
	surf_trigger = 0x0040,
	surf_nodraw = 0x0080,
	surf_hint = 0x0100,
	surf_skip = 0x0200,
	surf_nolight = 0x0400,
	surf_bumplight = 0x0800,
	surf_noshadows = 0x1000,
	surf_nodecals = 0x2000,
	surf_nopaint = surf_nodecals,
	surf_nochop = 0x4000,
	surf_hitbox = 0x8000,
};

enum mask_t
{
	mask_all = (0xFFFFFFFF),
	mask_solid = (contents_solid | contents_moveable | contents_window | contents_monster | contents_grate),
	mask_playersolid = (contents_solid | contents_moveable | contents_playerclip | contents_window | contents_monster | contents_grate),
	mask_npcsolid = (contents_solid | contents_moveable | contents_monsterclip | contents_window | contents_monster | contents_grate),
	mask_npcfluid = (contents_solid | contents_moveable | contents_monsterclip | contents_window | contents_monster),
	mask_water = (contents_water | contents_moveable | contents_slime),
	mask_opaque = (contents_solid | contents_moveable | contents_opaque),
	mask_opaque_and_npcs = (mask_opaque | contents_monster),
	mask_blocklos = (contents_solid | contents_moveable | contents_blocklos),
	mask_blocklos_and_npcs = (mask_blocklos | contents_monster),
	mask_visible = (mask_opaque | contents_ignore_nodraw_opaque),
	mask_visible_and_npcs = (mask_opaque_and_npcs | contents_ignore_nodraw_opaque),
	mask_shot = (contents_solid | contents_moveable | contents_monster | contents_window | contents_debris | contents_hitbox),
	mask_shot_brushonly = (contents_solid | contents_moveable | contents_window | contents_debris),
	mask_shot_hull = (contents_solid | contents_moveable | contents_monster | contents_window | contents_debris | contents_grate),
	mask_shot_portal = (contents_solid | contents_moveable | contents_window | contents_monster),
	mask_solid_brushonly = (contents_solid | contents_moveable | contents_window | contents_grate),
	mask_playersolid_brushonly = (contents_solid | contents_moveable | contents_window | contents_playerclip | contents_grate),
	mask_npcsolid_brushonly = (contents_solid | contents_moveable | contents_window | contents_monsterclip | contents_grate),
	mask_npcworldstatic = (contents_solid | contents_window | contents_monsterclip | contents_grate),
	mask_npcworldstatic_fluid = (contents_solid | contents_window | contents_monsterclip),
	mask_splitareaportal = (contents_water | contents_slime),
	mask_current = (contents_current_0 | contents_current_90 | contents_current_180 | contents_current_270 | contents_current_up | contents_current_down),
	mask_deadsolid = (contents_solid | contents_playerclip | contents_window | contents_grate),
};

enum class trace_type_t
{
	trace_everything = 0,
	trace_world_only,
	trace_entities_only,
	trace_everything_filter_props,
};

enum collision_group_t
{
	collision_group_none = 0,
	collision_group_debris,
	collision_group_debris_trigger,
	collision_group_interactive_debris,
	collision_group_interactive,
	collision_group_player,
	collision_group_breakable_glass,
	collision_group_vehicle,
	collision_group_player_movement,
	collision_group_npc,
	collision_group_in_vehicle,
	collision_group_weapon,
	collision_group_vehicle_clip,
	collision_group_projectile,
	collision_group_door_blocker,
	collision_group_passable_door,
	collision_group_dissolving,
	collision_group_pushaway,
	collision_group_npc_actor,
	collision_group_npc_scripted,
	last_shared_collision_group
};

using should_hit_fn = bool(__cdecl*)(void*, int);

class c_client_entity;
class c_baseentity;
class c_csplayer;
class c_handle_entity;
class c_collideable;

class i_trace_filter
{
public:
	virtual bool should_hit_entity(c_baseentity* entity, int mask) = 0;
	virtual trace_type_t get_trace_type() const = 0;
};

class c_trace_filter : public i_trace_filter
{
public:
	bool should_hit_entity(c_baseentity* entity, int /*contentsMask*/);

	virtual trace_type_t get_trace_type() const
	{
		return trace_type_t::trace_everything;
	}

	inline void set_ignore_class(char* Class)
	{
		ignore = Class;
	}

	void* skip;
	char* ignore = new char[1];
};

class c_trace_filter_one_entity : public i_trace_filter
{
public:
	bool should_hit_entity(c_baseentity* entity, int /*contentsMask*/)
	{
		return (entity == entity);
	}

	trace_type_t get_trace_type() const
	{
		return trace_type_t::trace_everything;
	}

	void* entity;
};

class c_trace_filter_skip_entity : public i_trace_filter
{
public:
	c_trace_filter_skip_entity(c_baseentity* entity)
	{
		skip = entity;
	}

	bool should_hit_entity(c_baseentity* entity, int /*contentsMask*/)
	{
		return !(entity == skip);
	}

	virtual trace_type_t get_trace_type() const
	{
		return trace_type_t::trace_everything;
	}

	void* skip;
};

class c_trace_filter_entities_only : public i_trace_filter
{
public:
	bool should_hit_entity(c_baseentity* entity, int /*contentsMask*/)
	{
		return true;
	}
	virtual trace_type_t get_trace_type() const
	{
		return trace_type_t::trace_entities_only;
	}
};

class c_trace_filter_world_only : public i_trace_filter
{
public:
	bool should_hit_entity(c_baseentity* /*pServerEntity*/, int /*contentsMask*/)
	{
		return false;
	}
	virtual trace_type_t get_trace_type() const
	{
		return trace_type_t::trace_world_only;
	}
};

class c_trace_filter_world_and_props_only : public i_trace_filter
{
public:
	bool should_hit_entity(c_baseentity* /*pServerEntity*/, int /*contentsMask*/)
	{
		return false;
	}
	virtual trace_type_t get_trace_type() const
	{
		return trace_type_t::trace_everything;
	}
};

class c_trace_filter_players_only_skip_one : public i_trace_filter
{
public:
	c_trace_filter_players_only_skip_one(c_baseentity* ent)
	{
		e = ent;
	}
	bool should_hit_entity(c_baseentity* entity, int /*contentsMask*/);

	virtual trace_type_t get_trace_type() const
	{
		return trace_type_t::trace_entities_only;
	}

private:
	c_baseentity* e;
};

class c_trace_filter_hit_all : public c_trace_filter
{
public:
	virtual bool should_hit_entity(c_baseentity* /*pServerEntity*/, int /*contentsMask*/)
	{
		return true;
	}
};

enum class debug_trace_counter_behavior_t
{
	ktrace_counter_set = 0,
	ktrace_counter_inc,
};

class i_entity_enumerator
{
public:
	// This gets called with each handle
	virtual bool EnumEntity(c_baseentity* pHandleEntity) = 0;
};

struct brush_side_info_t
{
	vector4d plane;
	uint16_t bevel;
	uint16_t thin;
};

class c_phys_collide;

struct cplane_t
{
	vector3d normal;
	float dist;
	uint8_t type;
	uint8_t signbits;
	uint8_t pad[2];
};

struct vcollide_t
{
	uint16_t solid_count : 15;
	uint16_t is_packed : 1;
	uint16_t desc_size;
	c_phys_collide** solids;
	char* key_values;
	void* user_data;
};

struct cmodel_t
{
	vector3d mins, maxs;
	vector3d origin;
	int32_t headnode;
	vcollide_t vcollision_data;
};

struct csurface_t
{
	const char* name;
	int16_t surface_props;
	uint16_t flags;
};

struct ray_t
{
	vector_aligned start;
	vector_aligned delta;
	vector_aligned start_offset;
	vector_aligned extents;
	const matrix3x4_t* world_axis_transform;
	bool is_ray;
	bool is_swept;

	ray_t() : world_axis_transform(NULL)
	{
	}

	ray_t(vector3d const& start, vector3d const& end)
	{
		delta = end - start;

		is_swept = (delta.length_sqr() != 0);

		extents = vector_aligned();

		world_axis_transform = NULL;
		is_ray = true;

		start_offset = vector_aligned();
		this->start = start;
	}

	ray_t(vector3d const& start, vector3d const& end, vector3d const& mins, vector3d const& maxs)
	{
		delta = { end - start };
		world_axis_transform = nullptr;
		is_swept = delta.length_sqr() != 0.f;
		extents = { maxs - mins };
		extents *= 0.5f;
		is_ray = extents.length_sqr() < 1e-6;
		start_offset = { mins + maxs };
		start_offset *= 0.5f;
		this->start = { start + start_offset };
		start_offset *= -1.f;
	}

	void init(vector3d const& start, vector3d const& end)
	{
		delta = end - start;

		is_swept = (delta.length_sqr() != 0);

		extents = vector_aligned();

		world_axis_transform = NULL;
		is_ray = true;

		start_offset = vector_aligned();
		this->start = start;
	}

	void init(vector3d const& start, vector3d const& end, vector3d const& mins, vector3d const& maxs)
	{
		delta = { end - start };
		world_axis_transform = nullptr;
		is_swept = delta.length_sqr() != 0.f;
		extents = { maxs - mins };
		extents *= 0.5f;
		is_ray = extents.length_sqr() < 1e-6;
		start_offset = { mins + maxs };
		start_offset *= 0.5f;
		this->start = { start + start_offset };
		start_offset *= -1.f;
	}

	vector3d inv_delta() const
	{
		vector3d inv_delta{};
		for (int i = 0; i < 3; ++i)
		{
			if (delta[i] != 0.0f)
				inv_delta[i] = 1.0f / delta[i];
			else
				inv_delta[i] = FLT_MAX;
		}

		return inv_delta;
	}
};

class c_base_trace
{
public:
	__forceinline bool is_disp_surface()
	{
		return ((disp_flags & dispsurf_flag_surface) != 0);
	}
	__forceinline bool is_disp_surface_walkable()
	{
		return ((disp_flags & dispsurf_flag_walkable) != 0);
	}
	__forceinline bool is_disp_surface_buildable()
	{
		return ((disp_flags & dispsurf_flag_buildable) != 0);
	}
	__forceinline bool is_disp_surface_prop1()
	{
		return ((disp_flags & dispsurf_flag_surfprop1) != 0);
	}
	__forceinline bool is_disp_surface_prop2()
	{
		return ((disp_flags & dispsurf_flag_surfprop2) != 0);
	}

public:
	vector3d start;
	vector3d end;
	cplane_t plane;

	float fraction;

	int contents;
	uint16_t disp_flags;

	bool all_solid;
	bool start_solid;

	c_base_trace()
	{
	}
};

class c_game_trace : public c_base_trace
{
public:
	bool did_hit_world() const;
	bool did_hit_non_world_entity() const;
	bool did_hit() const;
	bool is_visible() const;

public:
	float fraction_left_solid;
	csurface_t surface;
	uint32_t hitgroup;
	int16_t physics_bone;
	uint16_t world_surface_index;
	c_baseentity* entity;
	int32_t hitbox;

	c_game_trace()
	{
	}

	__forceinline void clear()
	{
		std::memset(this, 0, sizeof(c_game_trace));

		fraction = 1.f;
		surface.name = "**empty**";
	}

private:
	// No copy constructors allowed
	c_game_trace(const c_game_trace& other)
		: fraction_left_solid(other.fraction_left_solid), surface(other.surface), hitgroup(other.hitgroup), physics_bone(other.physics_bone), world_surface_index(other.world_surface_index), entity(other.entity),
		hitbox(other.hitbox)
	{
		start = other.start;
		end = other.end;
		plane = other.plane;
		fraction = other.fraction;
		contents = other.contents;
		disp_flags = other.disp_flags;
		all_solid = other.all_solid;
		start_solid = other.start_solid;
	}
};

inline bool c_game_trace::did_hit() const
{
	return fraction < 1 || all_solid || start_solid;
}

inline bool c_game_trace::is_visible() const
{
	return fraction > 0.97f;
}

class c_engine_trace
{
public:
	virtual int get_point_contents(const vector3d& abs_position, int mask = mask_all, c_handle_entity** entity = nullptr) = 0;
	virtual int get_point_contents_world_only(const vector3d& abs_position, int mask = mask_all) = 0;
	virtual int get_point_contents_collideable(void* collideable, const vector3d& abs_position) = 0;

	virtual void clip_ray_to_entity(const ray_t& ray, unsigned int mask, c_handle_entity* entity, c_game_trace* trace) = 0;
	virtual void clip_ray_to_collideable(const ray_t& ray, unsigned int mask, c_collideable* collideable, c_game_trace* trace) = 0;

	virtual void trace_ray(const ray_t& ray, unsigned int mask, i_trace_filter* filter, c_game_trace* trace) = 0;

	void trace_line(const vector3d& src, const vector3d& dst, int mask, c_handle_entity* entity, int collision_group, c_game_trace* trace);
	void trace_hull(const vector3d& src, const vector3d& dst, const vector3d& mins, const vector3d& maxs, int mask, c_handle_entity* entity, int collision_group, c_game_trace* trace);
};