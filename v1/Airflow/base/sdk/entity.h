#pragma once
#include "../sdk.h"

#include "../tools/math.h"
#include "../tools/memory/memory.h"
#include "../tools/netvar_parser.h"

#include "../other/game_functions.h"

#include "../global_context.h"

#define backup_globals(name) float prev_##name = interfaces::global_vars->##name
#define restore_globals(name) interfaces::global_vars->##name = prev_##name

enum bone_mask_t {
	bone_used_mask = 0x000FFF00,
	bone_used_by_anything = 0x000FFF00,
	bone_used_by_hitbox = 0x00000100,
	bone_used_by_attachment = 0x00000200,
	bone_used_by_vertex_mask = 0x0003FC00,
	bone_used_by_vertex_lod0 = 0x00000400,
	bone_used_by_vertex_lod1 = 0x00000800,
	bone_used_by_vertex_lod2 = 0x00001000,
	bone_used_by_vertex_lod3 = 0x00002000,
	bone_used_by_vertex_lod4 = 0x00004000,
	bone_used_by_vertex_lod5 = 0x00008000,
	bone_used_by_vertex_lod6 = 0x00010000,
	bone_used_by_vertex_lod7 = 0x00020000,
	bone_used_by_bone_merge = 0x00040000,
	bone_always_setup = 0x00080000,
};

enum entity_flags_t {
	fl_onground = (1 << 0),
	fl_ducking = (1 << 1),
	fl_waterjump = (1 << 3),
	fl_ontrain = (1 << 4),
	fl_inrain = (1 << 5),
	fl_frozen = (1 << 6),
	fl_atcontrols = (1 << 7),
	fl_client = (1 << 8),
	fl_fakeclient = (1 << 9),
	fl_inwater = (1 << 10),
};

enum fieldtype_t {
	field_void = 0,
	field_float,
	field_string,
	field_vector,
	field_quaternion,
	field_integer,
	field_boolean,
	field_short,
	field_character,
	field_color32,
	field_embedded,
	field_custom,
	field_classptr,
	field_ehandle,
	field_edict,
	field_position_vector,
	field_time,
	field_tick,
	field_modelname,
	field_soundname,
	field_input,
	field_function,
	field_vmatrix,
	field_vmatrix_worldspace,
	field_matrix3x4_worldspace,
	field_interval,
	field_modelindex,
	field_materialindex,
	field_vector2d,
	field_typecount,
};

enum {
	td_offset_normal = 0,
	td_offset_packed = 1,
	td_offset_count,
};

enum move_type_t {
	movetype_none = 0,
	movetype_isometric,
	movetype_walk,
	movetype_step,
	movetype_fly,
	movetype_flygravity,
	movetype_vphysics,
	movetype_push,
	movetype_noclip,
	movetype_ladder,
	movetype_observer,
	movetype_custom,
	movetype_last = movetype_custom,
	movetype_max_bits = 4
};

enum weapon_type_t {
	weapontype_knife = 0,
	weapontype_pistol = 1,
	weapontype_submachinegun = 2,
	weapontype_rifle = 3,
	weapontype_shotgun = 4,
	weapontype_sniper = 5,
	weapontype_machinegun = 6,
	weapontype_c4 = 7,
	weapontype_placeholder = 8,
	weapontype_grenade = 9,
	weapontype_healthshot = 11,
	weapontype_fists = 12,
	weapontype_breachcharge = 13,
	weapontype_bumpmine = 14,
	weapontype_tablet = 15,
	weapontype_melee = 16
};

enum item_defenition_index_t {
	weapon_none = 0,
	weapon_deagle = 1,
	weapon_elite = 2,
	weapon_fiveseven = 3,
	weapon_glock = 4,
	weapon_ak47 = 7,
	weapon_aug = 8,
	weapon_awp = 9,
	weapon_famas = 10,
	weapon_g3sg1 = 11,
	weapon_galilar = 13,
	weapon_m249 = 14,
	weapon_m4a1 = 16,
	weapon_mac10 = 17,
	weapon_p90 = 19,
	weapon_zone_repulsor = 20,
	weapon_mp5sd = 23,
	weapon_ump45 = 24,
	weapon_xm1014 = 25,
	weapon_bizon = 26,
	weapon_mag7 = 27,
	weapon_negev = 28,
	weapon_sawedoff = 29,
	weapon_tec9 = 30,
	weapon_taser = 31,
	weapon_hkp2000 = 32,
	weapon_mp7 = 33,
	weapon_mp9 = 34,
	weapon_nova = 35,
	weapon_p250 = 36,
	weapon_shield = 37,
	weapon_scar20 = 38,
	weapon_sg556 = 39,
	weapon_ssg08 = 40,
	weapon_knife_gg = 41,
	weapon_knife = 42,
	weapon_flashbang = 43,
	weapon_hegrenade = 44,
	weapon_smokegrenade = 45,
	weapon_molotov = 46,
	weapon_decoy = 47,
	weapon_incgrenade = 48,
	weapon_c4 = 49,
	weapon_healthshot = 57,
	weapon_knife_t = 59,
	weapon_m4a1_silencer = 60,
	weapon_usp_silencer = 61,
	weapon_cz75a = 63,
	weapon_revolver = 64,
	weapon_tagrenade = 68,
	weapon_fists = 69,
	weapon_breachcharge = 70,
	weapon_tablet = 72,
	weapon_melee = 74,
	weapon_axe = 75,
	weapon_hammer = 76,
	weapon_spanner = 78,
	weapon_knife_ghost = 80,
	weapon_firebomb = 81,
	weapon_diversion = 82,
	weapon_frag_grenade = 83,
	weapon_snowball = 84,
	weapon_bumpmine = 85,
	weapon_knife_bayonet = 500,
	weapon_knife_css = 503,
	weapon_knife_flip = 505,
	weapon_knife_gut = 506,
	weapon_knife_karambit = 507,
	weapon_knife_m9_bayonet = 508,
	weapon_knife_tactical = 509,
	weapon_knife_falchion = 512,
	weapon_knife_survival_bowie = 514,
	weapon_knife_butterfly = 515,
	weapon_knife_push = 516,
	weapon_knife_cord = 517,
	weapon_knife_canis = 518,
	weapon_knife_ursus = 519,
	weapon_knife_gypsy_jackknife = 520,
	weapon_knife_outdoor = 521,
	weapon_knife_stiletto = 522,
	weapon_knife_widowmaker = 523,
	weapon_knife_skeleton = 525,
	glove_studded_brokenfang = 4725,
	glove_studded_bloodhound = 5027,
	glove_t = 5028,
	glove_ct = 5029,
	glove_sporty = 5030,
	glove_slick = 5031,
	glove_leather_handwraps = 5032,
	glove_motorcycle = 5033,
	glove_specialist = 5034,
	glove_studded_hydra = 5035,
};

enum animation_layer_t {
	animation_layer_aimmatrix = 0,
	animation_layer_weapon_action,
	animation_layer_weapon_action_recrouch,
	animation_layer_adjust,
	animation_layer_movement_jump_or_fall,
	animation_layer_movement_land_or_climb,
	animation_layer_movement_move,
	animation_layer_movement_strafechange,
	animation_layer_whole_body,
	animation_layer_flashed,
	animation_layer_flinch,
	animation_layer_aliveloop,
	animation_layer_lean,
	animation_layer_count
};

enum observer_type_t {
	obs_mode_none,
	obs_mode_deathcam,
	obs_mode_freezecam,
	obs_mode_fixed,
	obs_mode_in_eye,
	obs_mode_chase,
	obs_mode_roaming,

	num_observer_modes,
};

class c_baseentity;
class c_csplayer;
class c_basecombatweapon;

class c_renderable;
class c_networkable;
class c_thinkable;
class c_unknown;
class c_client_entity;
class c_type_description;

struct datamap_t;
struct inputdata_t;

struct color24_t
{
	unsigned char r{}, g{}, b{};
};

struct ret_counted_t {
private:
	volatile long m_ref_count;
public:
	virtual void destructor(char bDelete) = 0;
	virtual bool on_final_release() = 0;

	void unref() {
		if (InterlockedDecrement(&m_ref_count) == 0 && on_final_release())
			destructor(1);
	}
};

class c_bone_accessor {
public:
	alignas(16) matrix3x4_t* bones{};

	int readable_bones{};
	int writable_bones{};
};

class c_collision_property {
public:
	padding(0x8);

	vector3d mins{};
	vector3d maxs{};
};

struct cmd_context_t {
	bool		needs_processing{};
	c_usercmd	user_cmd{};
	int			cmd_number{};
};

class c_type_description {
public:
	int							field_type{};
	const char* field_name{};
	int							field_offset{};
	unsigned short				fiels_size{};
	short						flags{};

	const char* external_name{};

	void* restore_ops{};
	void* input_func{};

	datamap_t* td{};

	int							field_size_in_butes{};

	class c_type_description* override_field{};

	int							override_count{};

	float						field_tolerance{};

	int							flat_offset[td_offset_count]{};
	unsigned short				flat_group{};
};

struct datamap_t {
	c_type_description* data_desc{};
	int						data_num_fields{};
	const char* data_class_name{};
	datamap_t* base_map{};
	int						packed_size{};
	void* optimized_data_map{};
};

unsigned int find_in_datamap(datamap_t* map, const char* name);

struct weapon_info_t
{
	char pad_0000[20];                 // 0x0000
	uint32_t max_ammo_1;                  // 0x0014
	char pad_0018[12];                 // 0x0018
	uint32_t max_ammo_2;                  // 0x0024
	char pad_0028[84];                 // 0x0028
	char* N00000985;                      // 0x007C
	char pad_0080[8];                  // 0x0080
	char* hud_name;                       // 0x0088
	char* weapon_name;                    // 0x008C
	char pad_0090[56];                 // 0x0090
	uint32_t weapon_type;                 // 0x00C8
	char pad_00CC[4];
	int weapon_price;
	int kill_award;
	char* animation_prefix;
	float cycle_time;
	float cycle_time_alt;
	float time_to_idle;
	float idle_interval;
	bool full_auto;
	char pad_0x00e5[3];
	uint32_t dmg;                         // 0x00F0
	float crosshair_delta_distance;       // 0x00F4
	float armor_ratio;                    // 0x00F8
	uint32_t bullets;                     // 0x00FC
	float penetration;                    // 0x0100
	float flinch_velocity_modifier_large; // 0x0104
	float flinch_velocity_modifier_small; // 0x0108
	float range;                          // 0x010C
	float range_modifier;                 // 0x0110
	float throw_velocity;                 // 0x0114
	char pad_0118[20];                 // 0x0118
	uint32_t crosshair_delta_dist;        // 0x012C
	uint32_t crosshair_min_dist;          // 0x0130
	float max_speed;                      // 0x0134
	float max_speed_alt;                  // 0x0138
	char pad_013C[12];                 // 0x013C
	float inaccuracy_crouch;              // 0x0148
	float inaccuracy_crouch_alt;          // 0x014C
	float inaccuracy_stand;               // 0x0150
	float inaccuracy_stand_alt;           // 0x0154
	float inaccuracy_jump;                // 0x0158
	float inaccuracy_jump_alt;            // 0x015C
	float inaccuracy_land;                // 0x0160
	float inaccuracy_land_alt;            // 0x0164
	char pad_0168[96];                 // 0x0168
	bool unk;                             // 0x01C8
	char pad_0169[4];                     // 0x01C9
	bool hide_viewmodel_in_zoom;          // 0x01CD
};

class c_weapon_system
{
protected:
	~c_weapon_system() = default;

private:
	virtual void pad0() = 0;
	virtual void pad1() = 0;

public:
	virtual weapon_info_t* get_weapon_data(uint32_t item_definition_index) = 0;
};

class c_animation_layers {
public:
	bool		client_blend{};
	float		blend_in{};
	void* studio_hdr{};

	int			dispatch_sequence{};
	int			second_dispatch_sequence{};

	uint32_t	order{};
	uint32_t	sequence{};

	float		prev_cycle{};
	float		weight{};
	float		weight_delta_rate{};
	float		playback_rate{};
	float		cycle{};

	void* owner{};

	padding(4);
};

class c_game_rules {
public:
	bool is_valve_ds() {
		if (!this)
			return false;
		return *(bool*)((uintptr_t)this + 0x7C);
	}

	bool is_freeze_time() {
		if (!this)
			return false;
		return *(bool*)((uintptr_t)this + 0x20);
	}
};

class c_ikcontext {
public:
	void constructor() {
		using fn = void(__thiscall*)(void*);
		static auto ik_ctor = patterns::ik_ctx_construct.as<fn>();

		ik_ctor(this);
	}

	void destructor() {
		using fn = void(__thiscall*)(void*);
		static auto ik_dector = patterns::ik_ctx_destruct.as<fn>();

		ik_dector(this);
	}

	void clear_targets() {
		auto i = 0;
		auto count = *(int*)((uintptr_t)this + 0xFF0);

		if (count > 0) {
			auto target = (int*)((uintptr_t)this + 0xD0);

			do {
				*target = -9999;
				target += 85;
				++i;
			} while (i < count);
		}
	}

	void init(c_studiohdr* hdr, vector3d* angles, vector3d* origin, float time, int frames, int mask) {
		using init_fn = void(__thiscall*)(void*, c_studiohdr*, vector3d*, vector3d*, float, int, int);
		patterns::ik_ctx_init.as<init_fn>()(this, hdr, angles, origin, time, frames, mask);
	}

	void update_targets(vector3d* pos, quaternion* qua, matrix3x4_t* matrix, uint8_t* bone_computed) {
		using update_targets_fn = void(__thiscall*)(void*, vector3d*, quaternion*, matrix3x4_t*, uint8_t*);
		patterns::ik_ctx_update_targets.as<update_targets_fn>()(this, pos, qua, matrix, bone_computed);
	}

	void solve_dependencies(vector3d* pos, quaternion* qua, matrix3x4_t* matrix, uint8_t* bone_computed) {
		using solve_dependencies_fn = void(__thiscall*)(void*, vector3d*, quaternion*, matrix3x4_t*, uint8_t*);
		patterns::ik_ctx_solve_dependencies.as<solve_dependencies_fn>()(this, pos, qua, matrix, bone_computed);
	}
};

struct bone_setup_t {
	c_studiohdr* hdr{};
	int				mask{};
	float* pose_parameter{};
	void* pose_debugger{};

	void init_pose(vector3d pos[], quaternion q[], c_studiohdr* hdr) {
		static auto init_pose = patterns::bone_setup_init_pose.as<uint64_t>();

		__asm
		{
			mov eax, this
			mov esi, q
			mov edx, pos
			push dword ptr[hdr + 4]
			mov ecx, [eax]
			push esi
			call init_pose
			add esp, 8
		}
	}

	void accumulate_pose(vector3d pos[], quaternion q[], int sequence, float cycle, float weight, float time, c_ikcontext* ctx) {
		func_ptrs::accumulate_pose(this, pos, q, sequence, cycle, weight, time, ctx);
	}

	void calc_autoplay_sequences(vector3d pos[], quaternion q[], float real_time, c_ikcontext* ctx) {
		static auto calc_autoplay_sequences = patterns::bone_setup_calc_autoplay_sequences.as<uint64_t>();

		__asm
		{
			movss xmm3, real_time
			mov eax, ctx
			mov ecx, this
			push eax
			push q
			push pos
			call calc_autoplay_sequences
		}
	}

	void calc_bone_adjust(vector3d pos[], quaternion q[], float* controllers, int mask) {
		static auto calc_bone_adj = patterns::bone_setup_calc_bone_adjust.as<uint64_t>();

		__asm
		{
			mov     eax, controllers
			mov     ecx, this
			mov     edx, pos; a2
			push    dword ptr[ecx + 4]; a5
			mov     ecx, [ecx]; a1
			push    eax; a4
			push    q; a3
			call    calc_bone_adj
			add     esp, 0xC
		}
	}
};

class c_handle_entity {
public:
	virtual ~c_handle_entity() {}
	virtual void set_ref_handle(const uint32_t& handle) = 0;
	virtual const uint32_t& get_ref_handle() const = 0;
};

class c_collideable {
public:
	virtual c_handle_entity* get_entity_handle() = 0;
	virtual vector3d& get_mins() const = 0;
	virtual vector3d& get_maxs() const = 0;

	int get_solid()
	{
		using fn = int(__thiscall*)(void*);
		return g_memory->getvfunc<fn>(this, 11)(this);
	}
};

class c_client_alpha_property
{
public:
	// Gets at the containing class...
	virtual c_unknown* get_unknown() = 0;

	// Sets a constant alpha modulation value
	virtual void set_alpha_modulation(uint8_t a) = 0;
};

class c_unknown : public c_handle_entity {
public:
	virtual c_collideable* get_collideable() = 0;
	virtual c_networkable* get_networkable_entity() = 0;
	virtual c_renderable* get_renderable_entity() = 0;
	virtual c_client_entity* get_client_entity() = 0;
	virtual c_baseentity* get_base_entity() = 0;
	virtual c_thinkable* get_thinkable_entity() = 0;
	virtual void* get_client_alpha_property() = 0;
};

class c_renderable {
public:
	virtual c_unknown* get_i_unknown_entity() = 0;

	virtual const vector3d& get_render_origin(void) = 0;
	virtual const vector3d& get_render_angles(void) = 0;

	virtual bool should_render(void) = 0;
	virtual int get_render_flags(void) = 0;

	virtual void unused(void) const {}

	virtual unsigned short get_shadow_handle() const = 0;
	virtual unsigned short& get_render_handle() = 0;

	virtual model_t* get_model() const = 0;
	virtual int draw_model(int flags, const int& instance) = 0;

	virtual int get_body() = 0;

	virtual void get_color_modulation(float* color) = 0;

	virtual bool lod_test() = 0;

	virtual bool setup_bones(matrix3x4_t* matrix, int max_bones, int mask, float time) = 0;
	virtual void setup_weights(const matrix3x4_t* matrix, int flex_cnt, float* flex_weights, float* flex_delayed_weights) = 0;

	virtual void useless_func1(void) = 0;

	virtual void* useless_func2() = 0;

	virtual void get_render_bounds(vector3d& mins, vector3d& maxs) = 0;
};

class c_networkable {
public:
	virtual c_unknown* get_n_unknown_entity() = 0;
	virtual void release() = 0;
	virtual c_client_class* get_client_class() = 0;
	virtual void should_transmit(int state) = 0;
	virtual void pre_data_changed(int update_type) = 0;
	virtual void data_changed(int update_type) = 0;
	virtual void pre_update(int update_type) = 0;
	virtual void post_update(int update_type) = 0;
	virtual void unk(void) = 0;
	virtual bool dormant(void) = 0;
	virtual int index(void) = 0;
	virtual void recieve_message() = 0;
	virtual void* get_data_table_base_ptr() = 0;
	virtual void set_destroyed_on_recreate_entities(void) = 0;
};

class c_thinkable {
public:
	virtual c_unknown* get_t_unknown_entity() = 0;
};

class c_client_entity : public c_unknown, public c_renderable, public c_networkable, public c_thinkable {
public:
	virtual void release(void) = 0;

	virtual const vector3d& get_abs_origin(void) = 0;
	virtual const vector3d& get_abs_angles(void) = 0;
};

class c_multiplayer_physics {
public:
	virtual int		get_physics_mode() = 0;
};

class c_breakable_prop_with_data {
public:
	virtual void		set_dmg_mod_bullet(float mod) = 0;
	virtual void		set_dmg_mod_dub(float mod) = 0;
	virtual void		set_cmd_mod_explosive(float mod) = 0;
	virtual float		get_dmg_mod_bullet() = 0;
};

class c_baseentity : public c_client_entity {
public:
	netvar_ref(render_angles, vector3d, offsets::deadflag + 4)
	netvar_ref(view_offset, vector3d, offsets::m_vecViewOffset)
	netvar_ref(eye_angles, vector3d, offsets::m_angEyeAngles)
	netvar_ref(origin, vector3d, offsets::m_vecOrigin)
	netvar_ref(bb_mins, vector3d, offsets::m_vecMins)
	netvar_ref(bb_maxs, vector3d, offsets::m_vecMaxs)
	netvar_ref(vec_force, vector3d, offsets::m_vecForce)
	netvar_ref(ragdoll_velocity, vector3d, offsets::m_vecRagdollVelocity)
	netvar_ref(ang_rotation, vector3d, offsets::m_angRotation)

	rnetvar(smoke_effect_tick_begin, int, offsets::m_nSmokeEffectTickBegin)
	rnetvar(owner, int, offsets::m_hOwnerEntity)
	rnetvar(fire_count, int, offsets::m_fireCount)
	rnetvar(life_state, int, offsets::m_lifeState)
	rnetvar(move_type, int, 0x25C)
	rnetvar(sequence, int, offsets::m_nSequence)

	netvar_ref(model_index, int, offsets::m_nModelIndex)
	netvar_ref(skybox_area, int, offsets::m_skyBoxArea)
	netvar_ref(shots_fired, int, offsets::m_iShotsFired)
	netvar_ptr(fire_x_delta, int, offsets::m_fireXDelta)
	netvar_ptr(fire_y_delta, int, offsets::m_fireYDelta)
	netvar_ptr(fire_z_delta, int, offsets::m_fireZDelta)

	rnetvar(bomb_site, int, offsets::m_nBombSite)
	rnetvar(defuse_count_down, float, offsets::m_flDefuseCountDown)
	rnetvar(bomb_timer_length, float, offsets::m_flTimerLength)
	rnetvar(c4_blow, float, offsets::m_flC4Blow)

	netvar_ref(velocity_modifier, float, offsets::m_flVelocityModifier)
	netvar_ref(fog_start, float, offsets::m_fogStart)
	netvar_ref(fog_end, float, offsets::m_fogEnd)
	netvar_ref(anim_time, float, offsets::m_flAnimTime)
	netvar_ref(cycle, float, offsets::m_flCycle)

	rnetvar(did_smoke_effect, bool, offsets::m_bDidSmokeEffect)
	rnetvar(bomb_ticking, bool, offsets::m_bBombTicking)
	rnetvar(bomb_defused, bool, offsets::m_bBombDefused)
	rnetvar(nade_exploded, bool, offsets::m_nExplodeEffectTickBegin + 4)

	netvar_ref(coordinate_frame, matrix3x4_t, offsets::m_CollisionGroup - 0x30)

	netvar_ptr(fire_is_burning, bool, offsets::m_bFireIsBurning)
	netvar_ptr(collision, c_collision_property, 0x320)

	netvar_ref(fog_color_primary, color, offsets::m_fogColorPrimary)
	netvar_ref(ground_entity, uint32_t, offsets::m_hGroundEntity)

	rnetvar(viewmodel_weapon, uint32_t, offsets::m_hWeapon)
	rnetvar(viewmodel_owner, uint32_t, offsets::m_hOwner)
	rnetvar(observer_target, uint32_t, offsets::m_hObserverTarget)
	rnetvar(ragdoll_player, uint32_t, offsets::m_hPlayer)
	rnetvar(active_weapon, uint32_t, offsets::m_hActiveWeapon)
	rnetvar(bomb_defuser, uint32_t, offsets::m_hBombDefuser)
	rnetvar(move_parent, uint32_t, offsets::m_hMoveParent)
	rnetvar(view_model, uint32_t, offsets::m_hViewModel)
	rnetvar(ragdoll, uint32_t, offsets::m_hRagdoll)

	netvar_ptr(wearables, uint32_t, offsets::m_hMyWearables)

	std::array<float, 24>& pose_parameter() {
		return *(std::array<float, 24>*)((uintptr_t)this + offsets::m_flPoseParameter);
	}

	bool is_player() {
		using fn = bool(__thiscall*)(void*);
		return g_memory->getvfunc<fn>(this, 158)(this);
	}

	bool is_weapon() {
		using fn = bool(__thiscall*)(void*);
		return g_memory->getvfunc<fn>(this, 166)(this);
	}

	void set_model_index(int model) {
		using fn = void(__thiscall*)(void*, int);
		return g_memory->getvfunc<fn>(this, 75)(this, model);
	}

	void set_viewmodel_matching_sequence(int sequence) {
		typedef  void(__thiscall* fn)(void*, int);
		return g_memory->getvfunc<fn>(this, 247)(this, sequence);
	}

	c_baseentity* get_move_parent();
	c_baseentity* get_view_model();

	c_csplayer* get_ragdoll_player();

	c_basecombatweapon* get_view_model_weapon();

	void compute_hitbox_surrounding_box(vector3d* mins, vector3d* maxs);

	void set_abs_origin(const vector3d& origin);
	void set_abs_angles(const vector3d& ang);

	void calc_absolute_position();

	int get_sequence_activity(int sequence);
};

class c_csplayer : public c_baseentity {
public:
	netvar_ref(velocity, vector3d, offsets::m_vecVelocity)

	netvar_ref(aim_punch_angle_vel, vector3d, offsets::m_aimPunchAngleVel)
	netvar_ref(view_punch_angle, vector3d, offsets::m_viewPunchAngle)
	netvar_ref(aim_punch_angle, vector3d, offsets::m_aimPunchAngle)
	netvar_ref(base_velocity, vector3d, offsets::m_vecBaseVelocity)
	netvar_ref(collision_change_origin, vector3d, 0x9920)
	netvar_ref(abs_velocity, vector3d, find_in_datamap(get_pred_desc_map(), xor_c("m_vecAbsVelocity")))
	netvar_ref(network_origin, vector3d, find_in_datamap(get_pred_desc_map(), xor_c("m_vecNetworkOrigin")))

	rnetvar(hitbox_set, int, offsets::m_nHitboxSet)
	rnetvar(player_state, int, offsets::m_iPlayerState)
	rnetvar(health, int, offsets::m_iHealth)
	rnetvar(team, int, offsets::m_iTeamNum)
	rnetvar(collision_group, int, offsets::m_CollisionGroup)

	netvar_ref(button_disabled, int, offsets::m_hViewEntity - 0xC)
	netvar_ref(button_released, int, find_in_datamap(get_pred_desc_map(), xor_c("m_afButtonReleased")))
	netvar_ref(button_pressed, int, find_in_datamap(get_pred_desc_map(), xor_c("m_afButtonPressed")))
	netvar_ref(button_forced, int, offsets::m_hViewEntity - 0x8)
	netvar_ref(button_last, int, find_in_datamap(get_pred_desc_map(), xor_c("m_afButtonLast")))
	netvar_ref(explode_effect_tick_begin, int, offsets::m_nExplodeEffectTickBegin)
	netvar_ref(observer_mode, int, offsets::m_iObserverMode)
	netvar_ref(move_state, int, offsets::m_iMoveState)
	netvar_ref(ik_context, int, 0x2670)
	netvar_ref(default_fov, int, offsets::m_iDefaultFOV)
	netvar_ref(old_buttons, int, find_in_datamap(get_pred_desc_map(), xor_c("m_nOldButtons")))
	netvar_ref(addon_bits, int, offsets::m_iAddonBits)
	netvar_ref(anim_flags, int, 0xA28)
	netvar_ref(fov_start, int, offsets::m_iFOVStart)
	netvar_ref(ent_flags, int, 0x68)
	netvar_ref(tickbase, int, offsets::m_nTickBase)
	netvar_ref(final_predicted_tick, int, offsets::m_nFinalPredictedTick)
	netvar_ref(effects, int, 0xF0)
	netvar_ref(e_flags, int, 0xE8)
	netvar_ref(flags, int, offsets::m_fFlags)
	netvar_ref(armor_value, int, offsets::m_ArmorValue)
	netvar_ref(fov, int, offsets::m_iFOV)

	netvar_ptr(buttons, int, find_in_datamap(get_pred_desc_map(), xor_c("m_nButtons")))

	netvar_ref(no_interp_perty, bool, offsets::m_ubEFNoInterpParity)
	netvar_ref(old_no_interp_perty, bool, offsets::m_ubEFNoInterpParityOld)

	rnetvar(surface_friction, float, 0x323C)
	rnetvar(next_attack, float, 0x2D80)
	rnetvar(spawn_time, float, offsets::m_iAddonBits - 4)
	rnetvar(flash_time, float, offsets::m_flFlashtime)

	netvar_ref(last_bone_setup_time, float, 0x2924)
	netvar_ref(target_spotted, bool, 0x93D)
	netvar_ref(thirdperson_recoil, float, offsets::m_flThirdpersonRecoil)
	netvar_ref(old_simtime, float, offsets::m_flSimulationTime + 4)
	netvar_ref(flash_max_alpha, float, offsets::m_flFlashMaxAlpha)
	netvar_ref(simulation_time, float, offsets::m_flSimulationTime)
	netvar_ref(fall_velocity, float, offsets::m_flFallVelocity)
	netvar_ref(duck_amount, float, offsets::m_flDuckAmount)
	netvar_ref(duck_speed, float, offsets::m_flDuckSpeed)
	netvar_ref(max_speed, float, offsets::m_flMaxSpeed)
	netvar_ref(model_scale, float, offsets::m_flModelScale)
	netvar_ref(fov_rate, float, offsets::m_flFOVRate)
	netvar_ref(grenade_spawn_time, float, 0x2A04)
	netvar_ref(collision_change_time, float, 0x9924)
	netvar_ref(lby, float, offsets::m_flLowerBodyYawTarget)

	rnetvar(gun_game_immunity, bool, offsets::m_bGunGameImmunity)
	rnetvar(has_defuser, bool, offsets::m_bHasDefuser)
	rnetvar(is_defusing, bool, offsets::m_bIsDefusing)
	rnetvar(has_helmet, bool, offsets::m_bHasHelmet)
	rnetvar(has_heavy_armor, bool, offsets::m_bHasHeavyArmor)
	rnetvar(wait_for_no_attack, bool, offsets::m_bWaitForNoAttack)
	rnetvar(ducking, bool, offsets::m_bDucking)
	rnetvar(ducked, bool, offsets::m_bDucked)

	netvar_ref(client_side_animation, bool, offsets::m_bClientSideAnimation)
	netvar_ref(jiggle_bones_enabled, bool, 0x2930)
	netvar_ref(is_walking, bool, offsets::m_bIsWalking)
	netvar_ref(is_scoped, bool, offsets::m_bIsScoped)
	netvar_ref(strafing, bool, offsets::m_bStrafing)

	netvar_ptr_ref(animstate, c_animstate, 0x9960)
	netvar_ptr_ref(anim_overlay, c_animation_layers, 0x2990)

	netvar_ptr(bone_accessor, c_bone_accessor, offsets::m_nForceBone + 28)

	netvar_ref(bone_cache, c_utlvector<matrix3x4_t>, 0x2914)
	netvar_ref(model_recent_bone_counter, uint32_t, 0x2690)

	netvar_ref(cmd_context, cmd_context_t, 0x350C)

	netvar_ptr(my_weapons, uint32_t, 0x2E08)

	__forceinline bool is_alive()
	{
		return this->life_state() == 0 && this->health() > 0;
	}

	__forceinline void store_layer(c_animation_layers* layer)
	{
		std::memcpy(layer, this->anim_overlay(), sizeof(c_animation_layers) * 13);
	}

	__forceinline bool is_flashed()
	{
		return *(float*)((uintptr_t)this + offsets::m_flFlashMaxAlpha - 0x8) > 200.f;
	}

	__forceinline void set_layer(c_animation_layers* layer)
	{
		std::memcpy(this->anim_overlay(), layer, sizeof(c_animation_layers) * 13);
	}

	__forceinline void store_poses(std::array< float, 24 >& pose)
	{
		pose = this->pose_parameter();
	}

	__forceinline void set_poses(std::array< float, 24 >& pose)
	{
		this->pose_parameter() = pose;
	}

	__forceinline void store_bone_cache(matrix3x4_t* matrix)
	{
		memcpy_fast(matrix, this->bone_cache().base(), sizeof(matrix3x4_t) * this->bone_cache().count());
	}

	__forceinline void set_bone_cache(matrix3x4_t* matrix)
	{
		auto accessor = this->bone_accessor();
		if (!accessor)
			return;

		memcpy_fast(this->bone_cache().base(), matrix, sizeof(matrix3x4_t) * this->bone_cache().count());
		memcpy_fast(accessor->bones, matrix, sizeof(matrix3x4_t) * this->bone_cache().count());
	}

	// 8 is an EFL_NOINTERP flag
	__forceinline void enable_interpolation()
	{
		this->effects() &= ~8;
	}

	__forceinline void disable_interpolation()
	{
		this->effects() |= 8;
	}

	__forceinline player_info_t get_player_info()
	{
		player_info_t info{};
		interfaces::engine->get_player_info(this->index(), &info);
		return info;
	}

	__forceinline std::string get_name(const std::string& base_name = "")
	{
		std::string name = !base_name.empty() ? base_name : get_player_info().name;

		if (name.size() > 20) {
			name.erase(20, name.length() - 20);
			name.append(xor_c("..."));
		}

		return name;
	}

	__forceinline void invalidate_bone_cache()
	{
		auto accessor = this->bone_accessor();
		if (!accessor)
			return;

		accessor->readable_bones = accessor->writable_bones = 0;

		uintptr_t cache = *patterns::invalidate_bone_cache.add(0xA).as< uintptr_t* >();

		this->model_recent_bone_counter() = (*(uint32_t*)cache) - 1;
		this->last_bone_setup_time() = -FLT_MAX;
	}

	int& personal_data_public_level();

	__forceinline bool is_bot()
	{
		auto info = this->get_player_info();
		return info.fakeplayer;
	}

	float get_sequence_cycle_rate(void* hdr, int sequence)
	{
		using fn = float(__thiscall*)(void*, void*, int);
		return g_memory->getvfunc< fn >(this, 222)(this, hdr, sequence);
	}

	float get_layer_sequence_cycle_rate(c_animation_layers* layer, int sequence)
	{
		using fn = float(__thiscall*)(void*, c_animation_layers*, int);
		return g_memory->getvfunc< fn >(this, 223)(this, layer, sequence);
	}

	void update_clientside_animation()
	{
		using fn = void(__thiscall*)(void*);
		g_memory->getvfunc< fn >(this, 224)(this);
	}

	void update_dispatch_layer(c_animation_layers* layers, c_studiohdr* hdr, int sequence)
	{
		using fn = void(__thiscall*)(void*, c_animation_layers*, c_studiohdr*, int);
		g_memory->getvfunc< fn >(this, 247)(this, layers, hdr, sequence);
	}

	void update_ik_locks(float curtime)
	{
		using fn = void(__thiscall*)(void*, float);
		g_memory->getvfunc< fn >(this, 192)(this, curtime);
	}

	void calc_ik_locks(float curtime)
	{
		using fn = void(__thiscall*)(void*, float);
		g_memory->getvfunc< fn >(this, 193)(this, curtime);
	}

	datamap_t* get_pred_desc_map()
	{
		using fn = datamap_t * (__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 17)(this);
	}

	void update_weapon_dispatch_layers();

	vector3d get_bone_position(int bone_index = 8);
	vector3d get_hitbox_position(int hitbox = 0, matrix3x4_t* matrix = nullptr);

	c_csplayer* get_observer_target();
	c_csplayer* get_ragdoll();
	bool should_fix_modify_eye_pos();
	c_basecombatweapon* get_active_weapon();
	c_studiohdr* get_studio_hdr();

	void run_pre_think();
	void run_think();

	void post_think();

	void draw_server_hitbox();
	uint8_t* get_server_edict();
	void force_update();

	void interpolate_moveparent_pos();

	void invalidate_physics_recursive(int flags);
	void attachments_helper();

	void setup_uninterpolated_bones(matrix3x4_t* matrix, vector3d angs = {});

	vector3d get_eye_position();
	vector3d get_render_eye_position();

	std::vector< c_basecombatweapon* > get_weapons();
};

class c_basecombatweapon : public c_baseentity {
public:
	netvar_ref(fallback_stat_trak, int, offsets::m_nFallbackStatTrak)
	netvar_ref(fallback_paint_kit, int, offsets::m_nFallbackPaintKit)
	netvar_ref(fallback_seed, int, offsets::m_nFallbackSeed)
	netvar_ref(fallback_wear, float, offsets::m_flFallbackWear)
	netvar_ref(account_id, int, offsets::m_iAccountID)
	netvar_ref(item_id_high, int, offsets::m_iItemIDHigh)
	netvar_ptr(custom_name, char, offsets::m_szCustomName)
	netvar_ref(original_owner_xuid_low, int, offsets::m_OriginalOwnerXuidLow)
	netvar_ref(original_owner_xuid_high, int, offsets::m_OriginalOwnerXuidHigh)
	netvar_ref(entity_quality, int, offsets::m_iEntityQuality)

	rnetvar(burst_shots_remaining, int, offsets::m_iBurstShotsRemaining)
	rnetvar(clip1, int, offsets::m_iClip1)
	rnetvar(zoom_level, int, offsets::m_zoomLevel)
	rnetvar(next_secondary_attack, float, offsets::m_flNextSecondaryAttack)
	rnetvar(next_primary_attack, float, offsets::m_flNextPrimaryAttack)
	rnetvar(next_burst_shot, float, offsets::m_fNextBurstShot)
	rnetvar(last_shot_time, float, offsets::m_fLastShotTime)
	rnetvar(throw_time, float, offsets::m_fThrowTime)

	netvar_ref(postpone_fire_ready_time, float, offsets::m_flPostponeFireReadyTime)
	netvar_ref(accuracy_penalty, float, offsets::m_fAccuracyPenalty)
	netvar_ref(throw_strength, float, offsets::m_flThrowStrength)
	netvar_ref(recoil_index, float, offsets::m_flRecoilIndex)

	netvar_ref(item_definition_index, short, offsets::m_iItemDefinitionIndex)

	netvar_ref(initalized, bool, offsets::m_bInitialized)

	rnetvar(pin_pulled, bool, offsets::m_bPinPulled)
	rnetvar(weapon_world_model, uint32_t, offsets::m_hWeaponWorldModel)

	netvar_ptr(thrower, c_csplayer, offsets::m_hThrower)

	float get_max_speed()
	{
		using fn = float(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 442)(this);
	}

	float get_spread()
	{
		using fn = float(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 453)(this);
	}

	float get_inaccuracy()
	{
		using fn = float(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 483)(this);
	}

	void update_accuracy_penalty()
	{
		using fn = void(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 484)(this);
	}

	int get_zoom_fov(int lvl)
	{
		using fn = int(__thiscall*)(void*, int);
		return g_memory->getvfunc< fn >(this, 457)(this, lvl);
	}

	float get_zoom_time(int lvl)
	{
		using fn = float(__thiscall*)(void*, int);
		return g_memory->getvfunc< fn >(this, 458)(this, lvl);
	}

	__forceinline bool is_heavy_pistols()
	{
		short idx = this->item_definition_index();
		return idx == weapon_deagle || idx == weapon_revolver;
	}

	__forceinline bool is_pistols()
	{
		auto info = this->get_weapon_info();
		if (!info)
			return false;

		int type = info->weapon_type;
		return type == weapontype_pistol && !is_heavy_pistols();
	}

	__forceinline bool is_knife()
	{
		auto info = this->get_weapon_info();
		if (!info)
			return false;

		short idx = this->item_definition_index();
		return info->weapon_type == weapontype_knife && idx != weapon_taser;
	}

	__forceinline bool is_grenade()
	{
		short idx = this->item_definition_index();
		return idx >= weapon_flashbang && idx <= weapon_incgrenade;
	}

	__forceinline bool is_rifle()
	{
		auto info = this->get_weapon_info();
		if (!info)
			return false;

		short idx = this->item_definition_index();
		return info->weapon_type == weapontype_rifle;
	}

	__forceinline bool is_taser()
	{
		auto info = this->get_weapon_info();
		if (!info)
			return false;

		short idx = this->item_definition_index();
		int type = info->weapon_type;
		return idx == weapon_taser;
	}

	__forceinline bool is_misc_weapon()
	{
		auto info = this->get_weapon_info();
		if (!info)
			return false;

		short idx = this->item_definition_index();
		int type = info->weapon_type;
		return idx == weapon_taser || type == weapontype_knife || type >= weapontype_c4 && type <= weapontype_melee;
	}

	__forceinline bool is_auto_sniper()
	{
		short idx = this->item_definition_index();

		switch (idx)
		{
		case weapon_g3sg1:
		case weapon_scar20:
			return true;
		default:
			return false;
		}
	}

	__forceinline bool is_sniper()
	{
		short idx = this->item_definition_index();

		switch (idx)
		{
		case weapon_awp:
		case weapon_g3sg1:
		case weapon_scar20:
		case weapon_ssg08:
			return true;
		default:
			return false;
		}
	}

	__forceinline bool is_scoping_weapon()
	{
		short idx = this->item_definition_index();

		switch (idx)
		{
		case weapon_awp:
		case weapon_g3sg1:
		case weapon_scar20:
		case weapon_ssg08:
		case weapon_sg556:
		case weapon_aug:
			return true;
		default:
			return false;
		}
	}

	std::string get_weapon_name()
	{
		auto replace = [&](std::string& str, const std::string& from, const std::string& to)
		{
			size_t start_pos = str.find(from);
			if (start_pos == std::string::npos)
				return false;
			str.replace(start_pos, from.length(), to);
			return true;
		};

		auto weapon_info = this->get_weapon_info();
		if (!weapon_info)
			return "";

		// skip SFUI_WEAPON thing
		std::string name = weapon_info->hud_name + 13;

		// make word in upper letters
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);

		// remove _ like in DESERT_EAGLE
		replace(name, "_", " ");
		return name;
	}

	__forceinline const char8_t* get_weapon_icon()
	{
		switch (this->item_definition_index())
		{
		case weapon_deagle:
			return u8"\uE001";
		case weapon_elite:
			return u8"\uE002";
		case weapon_fiveseven:
			return u8"\uE003";
		case weapon_glock:
			return u8"\uE004";
		case weapon_ak47:
			return u8"\uE007";
		case weapon_aug:
			return u8"\uE008";
		case weapon_awp:
			return u8"\uE009";
		case weapon_famas:
			return u8"\uE00A";
		case weapon_g3sg1:
			return u8"\uE00B";
		case weapon_galilar:
			return u8"\uE00D";
		case weapon_m249:
			return u8"\uE00E";
		case weapon_m4a1:
			return u8"\uE010";
		case weapon_mac10:
			return u8"\uE011";
		case weapon_p90:
			return u8"\uE013";
		case weapon_mp5sd:
			return u8"\uE017";
		case weapon_ump45:
			return u8"\uE018";
		case weapon_xm1014:
			return u8"\uE019";
		case weapon_bizon:
			return u8"\uE01A";
		case weapon_mag7:
			return u8"\uE01B";
		case weapon_negev:
			return u8"\uE01C";
		case weapon_sawedoff:
			return u8"\uE01D";
		case weapon_tec9:
			return u8"\uE01E";
		case weapon_taser:
			return u8"\uE01F";
		case weapon_hkp2000:
			return u8"\uE020";
		case weapon_mp7:
			return u8"\uE021";
		case weapon_mp9:
			return u8"\uE022";
		case weapon_nova:
			return u8"\uE023";
		case weapon_p250:
			return u8"\uE024";
		case weapon_scar20:
			return u8"\uE026";
		case weapon_sg556:
			return u8"\uE027";
		case weapon_ssg08:
			return u8"\uE028";
		case weapon_knife:
			return u8"\uE02A";
		case weapon_flashbang:
			return u8"\uE02B";
		case weapon_hegrenade:
			return u8"\uE02C";
		case weapon_smokegrenade:
			return u8"\uE02D";
		case weapon_molotov:
		case weapon_firebomb:
			return u8"\uE02E";
		case weapon_decoy:
		case weapon_diversion:
			return u8"\uE02F";
		case weapon_incgrenade:
			return u8"\uE030";
		case weapon_c4:
			return u8"\uE031";
		case weapon_healthshot:
			return u8"\uE039";
		case weapon_knife_gg:
		case weapon_knife_t:
			return u8"\uE03B";
		case weapon_m4a1_silencer:
			return u8"\uE03C";
		case weapon_usp_silencer:
			return u8"\uE03D";
		case weapon_cz75a:
			return u8"\uE03F";
		case weapon_revolver:
			return u8"\uE040";
		case weapon_tagrenade:
			return u8"\uE044";
		case weapon_fists:
			return u8"\uE045";
		case weapon_tablet:
			return u8"\uE048";
		case weapon_melee:
			return u8"\uE04A";
		case weapon_axe:
			return u8"\uE04B";
		case weapon_hammer:
			return u8"\uE04C";
		case weapon_spanner:
			return u8"\uE04E";
		case weapon_knife_bayonet:
			return u8"\uE1F4";
		case weapon_knife_css:
			return u8"\uE1F7";
		case weapon_knife_flip:
			return u8"\uE1F9";
		case weapon_knife_gut:
			return u8"\uE1FA";
		case weapon_knife_karambit:
			return u8"\uE1FB";
		case weapon_knife_m9_bayonet:
			return u8"\uE1FC";
		case weapon_knife_tactical:
			return u8"\uE1FD";
		case weapon_knife_falchion:
			return u8"\uE200";
		case weapon_knife_survival_bowie:
			return u8"\uE202";
		case weapon_knife_butterfly:
			return u8"\uE203";
		case weapon_knife_push:
			return u8"\uE204";
		case weapon_knife_cord:
			return u8"\uE205";
		case weapon_knife_canis:
			return u8"\uE206";
		case weapon_knife_ursus:
			return u8"\uE207";
		case weapon_knife_gypsy_jackknife:
			return u8"\uE208";
		case weapon_knife_outdoor:
			return u8"\uE209";
		case weapon_knife_stiletto:
			return u8"\uE20A";
		case weapon_knife_widowmaker:
			return u8"\uE20B";
		case weapon_knife_skeleton:
			return u8"\uE20D";
		default:
			return u8"\u003F";
		}
	}

	const char* get_grenade_name(const int& class_id)
	{
		model_t* model = this->get_model();
		if (!model)
			return "";

		switch (class_id)
		{
		case(int)CBaseCSGrenadeProjectile:
			return strstr(model->name, "flashbang") ? "FLASH" : "HE GRENADE";
			break;
		case(int)CBreachChargeProjectile:
			return "BREACH";
			break;
		case(int)CBumpMineProjectile:
			return "MINE";
			break;
		case(int)CDecoyProjectile:
			return "DECOY";
			break;
		case(int)CMolotovProjectile:
			return "FIRE";
			break;
		case(int)CSensorGrenadeProjectile:
			return "SENSOR";
			break;
		case(int)CSmokeGrenadeProjectile:
			return "SMOKE";
			break;
		case(int)CSnowballProjectile:
			return "SNOW";
			break;
		}
		return "UNK";
	}

	weapon_info_t* get_weapon_info();
	c_basecombatweapon* get_weapon_world_model();
};

class c_breakable_surface : public c_baseentity, public c_breakable_prop_with_data {
public:
	rnetvar(m_bIsBroken, bool, offsets::m_bIsBroken)
};