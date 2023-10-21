#pragma once
#include "../tools/math.h"
#include "../tools/netvar_parser.h"

#include "c_utlvector.h"
#include "c_material.h"

enum mod_types_t
{
	mod_bad = 0,
	mod_brush = 1,
	mod_sprite = 2,
	mod_studio = 3,
};

enum damage_type_t
{
	damage_no = 0,
	damage_events_only = 1,
	damage_yes = 2,
	damage_aim = 3,
};

enum bsp_contents
{
	bc_empty,
	bc_solid = 0x1,
	bc_window = 0x2,
	bc_aux = 0x4,
	bc_grate = 0x8,
	bc_slime = 0x10,
	bc_water = 0x20,
	bc_block_los = 0x40,
	bc_opaque = 0x80,
	bc_test_fog_volume = 0x100,
	bc_block_light = 0x400,
	bc_team_1 = 0x800,
	bc_team_2 = 0x1000,
	bc_ignore_no_draw_opaque = 0x2000,
	bc_movable = 0x4000,
	bc_area_portal = 0x8000,
	bc_player_clip = 0x10000,
	bc_monster_clip = 0x20000,
	bc_brush_paint = 0x40000,
	bc_grenade_clip = 0x80000,
	bc_origin = 0x1000000,
	bc_monster = 0x2000000,
	bc_debris = 0x4000000,
	bc_detail = 0x8000000,
	bc_translucent = 0x10000000,
	bc_ladder = 0x20000000,
	bc_hitbox = 0x40000000,
};

enum bsp_surf
{
	bs_light = 0x1,
	bs_sky_2d = 0x2,
	bs_sky = 0x4,
	bs_warp = 0x8,
	bs_trans = 0x10,
	bs_no_portal = 0x20,
	bs_trigger = 0x40,
	bs_no_draw = 0x80,
	bs_hint = 0x100,
	bs_skip = 0x200,
	bs_no_light = 0x400,
	bs_bump_light = 0x800,
	bs_no_shadows = 0x1000,
	bs_no_decals = 0x2000,
	bs_no_paint = 0x2000,
	bs_no_chop = 0x4000,
	bs_hitbox = 0x8000,
};

enum bsp_mask
{
	bm_all = 0xFFFFFFFF,
	bm_solid = bc_solid | bc_movable | bc_window | bc_monster | bc_grate,
	bm_player_solid = bm_solid | bc_player_clip,
	bm_npc_solid = bm_solid | bc_monster_clip,
	bm_npc_fluid = bm_npc_solid & ~bc_grate,
	bm_water = bc_water | bc_movable | bc_slime,
	bm_opaque = bc_solid | bc_movable | bc_opaque,
	bm_opaque_and_npc = bm_opaque | bc_monster,
	bm_block_los = bc_solid | bc_movable | bc_block_los,
	bm_block_los_and_npc = bm_block_los | bc_monster,
	bm_visible = bm_opaque | bc_ignore_no_draw_opaque,
	bm_visible_and_npc = bm_visible | bc_monster,
	bm_shot = bm_solid & ~bc_grate | bc_debris | bc_hitbox,
	bm_shot_grate = bm_solid | bc_debris | bc_hitbox,
	bm_floor_trace = bm_shot & ~bc_monster & ~bc_hitbox,
	bm_weapon_clipping = bm_shot & ~bc_hitbox,
	bm_shot_brush_only = bm_floor_trace,
	bm_shot_hull = bm_shot & ~bc_hitbox | bc_grate,
	bm_shot_hull_hitbox = bm_shot_hull | bc_hitbox
};

enum hitboxes_t : int
{
	hitbox_head = 0,
	hitbox_neck,
	hitbox_pelvis,
	hitbox_stomach,
	hitbox_lower_chest,
	hitbox_chest,
	hitbox_upper_chest,
	hitbox_left_thigh,
	hitbox_right_thigh,
	hitbox_left_calf,
	hitbox_right_calf,
	hitbox_left_foot,
	hitbox_right_foot,
	hitbox_left_hand,
	hitbox_right_hand,
	hitbox_left_upper_arm,
	hitbox_left_forearm,
	hitbox_right_upper_arm,
	hitbox_right_forearm,
	hitbox_max
};

enum hitgroup_t
{
	hitgroup_generic = 0,
	hitgroup_head = 1,
	hitgroup_chest = 2,
	hitgroup_stomach = 3,
	hitgroup_leftarm = 4,
	hitgroup_rightarm = 5,
	hitgroup_leftleg = 6,
	hitgroup_rightleg = 7,
	hitgroup_neck = 8,
	hitgroup_gear = 10
};

struct studio_lod_data_t
{
	void* mesh_data{};
	float switch_point{};
	int num_materials{};
	c_material** material_pointers{};
	int* material_flags{};
	int* morph_decal_bone_map{};
	int decal_bone_count{};
};

struct studio_hw_data_t
{
	int root_lods{};
	int num_lods{};

	studio_lod_data_t* lods{};

	int studio_meshes{};

	inline float get_lod_metrics(float sphere_size) const
	{
		return (sphere_size != 0.f) ? (100.f / sphere_size) : 0.f;
	}

	inline int get_lod_for_metric(float lod_metric) const
	{
		if (!num_lods)
			return 0;

		int num_lods = (lods[num_lods - 1].switch_point < 0.0f) ? num_lods - 1 : num_lods;

		for (int i = root_lods; i < num_lods - 1; i++)
		{
			if (lods[i + 1].switch_point > lod_metric)
				return i;
		}

		return num_lods - 1;
	}
};

struct mstudio_bone_t
{
	int name_index{};

	inline char* const get_name(void) const
	{
		return ((char*)this) + name_index;
	}

	int parent{};
	int bone_controller[6]{};

	vector3d pos{};
	vector4d quat{};

	float rot[3]{};

	vector3d pos_scale{};
	vector3d rot_scale{};

	matrix3x4_t pose_to_bone{};

	quaternion alignment{};

	int flags{};

	int proc_type{};
	int proc_index{};

	mutable int physics_bone{};

	inline void* get_procedure() const
	{
		if (proc_index == 0)
			return NULL;
		else
			return (void*)(((unsigned char*)this) + proc_index);
	};

	int surface_prop_iindex;

	inline char* const get_surface_prop_name(void) const
	{
		return ((char*)this) + surface_prop_iindex;
	}

	inline int get_surface_prop(void) const
	{
		return surface_prop_lookup;
	}

	int contents{};
	int surface_prop_lookup{};
	int unused[7]{};
};

struct mstudio_bbox_t
{
	int bone{};
	int group{};

	vector3d bbmin{};
	vector3d bbmax{};

	int hitbox_name_index{};

	vector3d rotation{};

	float radius{};

	int pad2[4]{};
};

struct mstudio_pose_param_desc_t
{
	int name_index{};

	inline char* const get_name(void) const
	{
		return ((char*)this) + name_index;
	}

	int flags{};

	float start{};
	float end{};
	float loop{};
};

struct mstudio_hitbox_set_t
{
	int index_name{};

	int hitboxes{};
	int hitbox_index{};

	inline char* const get_name(void) const
	{
		return ((char*)this) + index_name;
	}

	inline mstudio_bbox_t* get_hitbox(int i) const
	{
		return (mstudio_bbox_t*)(((unsigned char*)this) + hitbox_index) + i;
	}
};

struct studio_hdr_t
{
	int id{};
	int version{};

	long checksum{};

	char name[64]{};

	int length{};

	vector3d eye_pos{};
	vector3d ilum_pos{};

	vector3d hull_min{};
	vector3d hull_max{};

	vector3d bb_min{};
	vector3d bb_max{};

	int flags{};

	int bones{};
	int bone_index{};

	int bone_controllers{};
	int bone_controller_index{};

	int hitbox_sets{};
	int hitbox_set_index{};

	int local_anim{};
	int local_anim_index{};

	int local_seq{};
	int local_seq_index{};

	int activity_list_version{};
	int events_indexed{};

	int textures{};
	int texture_index{};

	int numskinref{};
	int numskinfamilies{};
	int skinindex{};

	int numbodyparts{};
	int bodypartindex{};

	int numlocalattachments{};
	int localattachmentindex{};

	int numlocalnodes{};
	int localnodeindex{};
	int localnodenameindex{};

	int numflexdesc{};
	int flexdescindex{};


	int numflexcontrollers;
	int flexcontrollerindex;

	int numflexrules;
	int flexruleindex;

	int numikchains;
	int ikchainindex;

	int nummouths;
	int mouthindex;

	int numlocalposeparameters;
	int localposeparamindex;

	inline mstudio_pose_param_desc_t* local_pose_parameter(int i) const { return (mstudio_pose_param_desc_t*)(((byte*)this) + localposeparamindex) + i; };

	inline const char* get_name(void) const
	{
		return name;
	}

	mstudio_hitbox_set_t* get_hitbox_set(int i)
	{
		if (i > hitbox_sets)
			return nullptr;

		return (mstudio_hitbox_set_t*)((uint8_t*)this + hitbox_set_index) + i;
	}

	mstudio_bone_t* get_bone(int i)
	{
		if (i > bones)
			return nullptr;

		return (mstudio_bone_t*)((uint8_t*)this + bone_index) + i;
	}
};

class c_studiohdr
{
public:
	netvar_ref(bone_flags, c_utlvector< int >, 0x30)

	void* get_sequence_desc(int sequence);
};