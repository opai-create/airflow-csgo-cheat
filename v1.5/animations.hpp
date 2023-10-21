#pragma once
#include "server_bones.hpp"
#include "engine_prediction.hpp"

struct matrix_t
{
	c_bone_builder bone_builder{};
	c_animation_layers layers[13]{};
	matrix3x4_t matrix[128]{};
	matrix3x4_t roll_matrix[128]{};

	INLINE void reset()
	{
		bone_builder.reset();

		std::memset(layers, 0, sizeof(layers));
		std::memset(matrix, 0, sizeof(matrix));
		std::memset(roll_matrix, 0, sizeof(roll_matrix));
	}
};

struct anim_record_t
{
	bool filled{};
	bool landing{};
	bool landed{};
	bool on_ground{};
	bool real_on_ground{};
	bool dormant{};
	bool walking{};
	bool strafing{};
	bool valid_lc{};
	bool shifting{};
	bool break_lc{};
	bool shooting{};
	bool extrapolated{};
	bool fakewalking{};

	int server_tick_estimation{};
	int extrapolate_ticks{};
	int choke{};
	memory::bits_t flags{};

	float duck_amt{};
	float land_time{};
	float sim_time{};
	float old_sim_time{};
	float last_shot_time{};
	float anim_speed{};
	float jitter_diff{};
	float old_jitter_diff{};
	float last_jitter_time{};
	float collision_change_time{};
	float collision_change_origin{};
	float lby{};
	float thirdperson_recoil{};

	vec3_t velocity{};
	vec3_t velocity_for_animfix{};
	vec3_t abs_velocity{};
	vec3_t abs_origin{};
	vec3_t origin{};
	vec3_t eye_angles{};
	vec3_t view_offset{};
	vec3_t mins{};
	vec3_t maxs{};
	vec3_t abs_angles{};
	vec3_t last_reliable_angle{};

	std::uint32_t ground_entity{};
	c_animation_layers layers[13]{};

#ifndef LEGACY
	matrix_t matrix_left{};
	matrix_t matrix_right{};
	matrix_t matrix_zero{};
#endif

	matrix_t matrix_orig{};

	struct prediction_t
	{
		vec3_t origin{};
		vec3_t velocity{};

		float time{};
		memory::bits_t flags{};

		INLINE void reset()
		{
			origin.reset();
			velocity.reset();

			time = 0.f;
			flags.bits = 0;
		}
	} prediction;

	INLINE void predict()
	{
		prediction.origin = origin;
		prediction.velocity = velocity;
		prediction.time = sim_time;
		prediction.flags = flags;
	}

	matrix3x4_t predicted_matrix[128]{};

	INLINE void store(c_cs_player* player)
	{
		flags = player->flags();
		duck_amt = player->duck_amount();
		velocity = player->velocity();
		abs_velocity = player->abs_velocity();
		origin = player->origin();
		abs_origin = player->get_abs_origin();
		sim_time = player->sim_time();
		old_sim_time = player->old_sim_time();
		eye_angles = player->eye_angles();
		walking = player->is_walking();
		strafing = player->strafing();
#ifndef LEGACY
		collision_change_time = player->collision_change_time();
		collision_change_origin = player->collision_change_origin();
#endif
		lby = player->lower_body_yaw();
		thirdperson_recoil = player->thirdperson_recoil();
		view_offset = player->view_offset();
		abs_angles = player->get_abs_angles();
		
		auto simulation_tick = TIME_TO_TICKS(sim_time);
		auto old_simulation_tick = TIME_TO_TICKS(old_sim_time);

		auto collideable = player->get_collideable();
		if (collideable)
		{
			mins = collideable->get_mins();
			maxs = collideable->get_maxs();
		}

		auto weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(player->active_weapon()));
		if (weapon)
			last_shot_time = weapon->last_shot_time();

		choke = std::clamp(simulation_tick - old_simulation_tick, 0, 16);
		on_ground = player->flags().has(FL_ONGROUND);

		landed = false;
		land_time = 0.f;

		ground_entity = player->ground_entity();
		player->store_layers(layers);

		server_tick_estimation = HACKS->client_state->clock_drift_mgr.server_tick;

		filled = true;
	}

	INLINE void restore(c_cs_player* player)
	{
		if (!filled)
			return;

		player->is_walking() = walking;
		player->strafing() = strafing;

		player->flags() = flags;
		player->duck_amount() = duck_amt;
		player->velocity() = velocity;
		player->abs_velocity() = abs_velocity;
		player->origin() = origin;

		player->set_layers(layers);
		player->set_abs_origin(abs_origin);
	}

	INLINE void reset()
	{
		filled = false;
		valid_lc = false;
		landing = false;
		landed = false;
		on_ground = false;
		real_on_ground = false;
		dormant = false;
		shifting = false;
		break_lc = false;
		shooting = false;
		fakewalking = false;

		choke = 0;
		extrapolate_ticks = 0;
		flags = {};

		duck_amt = 0.f;
		land_time = 0.f;
		sim_time = 0.f;
		old_sim_time = 0.f;
		anim_speed = 0.f;
		jitter_diff = 0.f;
		last_jitter_time = 0.f;
		old_jitter_diff = 0.f;
		collision_change_time = 0.f;
		collision_change_origin = 0.f;
		last_shot_time = 0.f;
		lby = 0.f;
		thirdperson_recoil = 0.f;

		velocity.reset();
		velocity_for_animfix.reset();
		abs_velocity.reset();
		abs_origin.reset();
		origin.reset();
		eye_angles.reset();
		view_offset.reset();
		mins.reset();
		maxs.reset();
		abs_angles.reset();
		last_reliable_angle.reset();
		ground_entity = 0;

		std::memset(layers, 0, sizeof(layers));

#ifndef LEGACY
		matrix_left.reset();
		matrix_right.reset();
		matrix_zero.reset();
#endif
		matrix_orig.reset();

		std::memset(predicted_matrix, 0, sizeof(predicted_matrix));
		prediction.reset();
	}
};

struct anims_t
{
	bool update_anims = true;
	bool setup_bones = true;
	bool clamp_bones_in_bbox = true;
	bool attachments_helper = true;
	bool modify_eye_pos = true;

	int dormant_ticks{};
	int server_tick{};

	float old_spawn_time{};
	float old_aliveloop_cycle{};
	float old_aliveloop_rate{};
	float old_simulation_time{};
	float old_simulation_time2{};
	float dt_interpolation_fraction{};
	float last_valid_time{};

	c_cs_player* ptr{};
	c_animation_state old_animstate{};

	std::deque<anim_record_t> records{};

	INLINE void reset()
	{
		update_anims = false;
		setup_bones = false;
		clamp_bones_in_bbox = false;
		attachments_helper = false;
		modify_eye_pos = false;

		server_tick = 0;
		dormant_ticks = 0;
		old_spawn_time = 0.f;
		old_aliveloop_cycle = 0.f;
		old_aliveloop_rate = 0.f;
		old_simulation_time = 0.f;
		old_simulation_time2 = 0.f;
		dt_interpolation_fraction = 0.f;
		last_valid_time = 0.f;

		ptr = nullptr;
		old_animstate = {};
		records.clear();
	}
};

struct bone_blending_t {
	vec3_t v_angle{};

	c_animation_layers layers[13]{};
	float poses[24]{};

	INLINE void store(c_cs_player* player) {
		v_angle = player->render_angles();

		player->store_layers(layers);
		player->store_poses(poses);
	}

	INLINE void restore(c_cs_player* player) {
		player->render_angles() = player->eye_angles() = v_angle;

		player->set_layers(layers);
		player->set_poses(poses);
	}

	INLINE void reset() {
		v_angle.reset();

		for (auto& i : layers)
			i = {};

		for (auto& i : poses)
			i = 0.f;
	}
};

struct local_anims_t
{
	bool landing{};
	bool on_ground{};
	int max_choke{};
	int last_outgoing_predicted_cmd{};
	int old_move_type{};

	float foot_yaw{};
	float aim_matrix_width_range{};
	float max_desync_range{};
	float last_lby_time{};
	float lby_angle{};

	vec3_t eye_pos{};
	vec3_t sent_eye_pos{};

	memory::bits_t old_flags{};

	c_animation_layers backup_layers[13]{};
	c_animation_layers layers[13]{};
	matrix3x4_t matrix[128]{};

	bone_blending_t real_blending{};
	bone_blending_t restore_blending{};
	networked_vars_t old_vars{};

	c_bone_builder bone_builder{};
	c_animation_state old_state{};

	INLINE void reset()
	{
		landing = false;
		on_ground = false;
		max_choke = 0;
		last_outgoing_predicted_cmd = 0;
		old_move_type = 0;

		foot_yaw = 0.f;
		aim_matrix_width_range = 0.f;
		max_desync_range = 0.f;
		last_lby_time = 0.f;
		lby_angle = 0.f;

		eye_pos.reset();
		sent_eye_pos.reset();
		real_blending.reset();
		restore_blending.reset();
		old_vars.reset();

		old_flags = 0;

		for (auto& i : backup_layers)
			i = {};

		for (auto& i : layers)
			i = {};

		for (auto& i : matrix)
			i = {};

		old_state = {};
		bone_builder.reset();
	}
};

class c_animation_fix
{
private:
	anims_t anims[65]{};
	anim_record_t backups[65]{};
	local_anims_t local_anims{};

public:
	INLINE void reset()
	{
		local_anims.reset();

		for (auto& i : anims)
			i.reset();

		for (auto& i : backups)
			i.reset();
	}

	INLINE anims_t* get_anims(int idx)
	{
		return &anims[idx];
	}

	INLINE anim_record_t* get_backup_record(int idx)
	{
		return &backups[idx];
	}

	INLINE local_anims_t* get_local_anims()
	{
		return &local_anims;
	}

	void update_enemies();
	void handle_jump_animations(c_animation_state* state, c_animation_layers* layers, c_user_cmd* cmd);
	void handle_strafing(c_animation_state* state, c_user_cmd* cmd);
	void update_local();
	vec3_t get_eye_position(float angle);
	void render_matrices(c_cs_player* player);
	void restore_all();
};

#ifdef _DEBUG
inline auto ANIMFIX = std::make_unique<c_animation_fix>();
#else
CREATE_DUMMY_PTR(c_animation_fix);
DECLARE_XORED_PTR(c_animation_fix, GET_XOR_KEYUI32);

#define ANIMFIX XORED_PTR(c_animation_fix)
#endif