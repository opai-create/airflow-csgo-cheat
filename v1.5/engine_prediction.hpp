#pragma once

struct unpred_vars_t
{
	bool in_prediction{};
	bool first_time_predicted{};

	int random_seed{};
	int prediction_player{};

	float recoil_index{};
	float accuracy_penalty{};

	float curtime{};
	float frametime{};

	memory::bits_t flags{};

	vec3_t velocity{};

	c_user_cmd* predicted_cmd{};
	c_user_cmd updated_cmd{};

	std::uint32_t ground_entity{};

	INLINE void store()
	{
		in_prediction = HACKS->prediction->in_prediction;
		first_time_predicted = HACKS->prediction->is_first_time_predicted;

		flags = HACKS->local->flags();
		velocity = HACKS->local->velocity();

		if (HACKS->weapon)
		{
			recoil_index = HACKS->weapon->recoil_index();
			accuracy_penalty = HACKS->weapon->accuracy_penalty();
		}

		curtime = HACKS->global_vars->curtime;
		frametime = HACKS->global_vars->frametime;

		ground_entity = HACKS->local->ground_entity();

		// hardcoded because it doesn't parse with netvars
#ifdef LEGACY
		predicted_cmd = *(c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3314));
		updated_cmd = *(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x326C));
#else
		predicted_cmd = *(c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3348));
		updated_cmd = *(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x3298));
#endif
	}

	INLINE void restore()
	{
		HACKS->prediction->in_prediction = in_prediction;
		HACKS->prediction->is_first_time_predicted = first_time_predicted;

		if (HACKS->weapon)
		{
			HACKS->weapon->recoil_index() = recoil_index;
			HACKS->weapon->accuracy_penalty() = accuracy_penalty;
		}

		HACKS->global_vars->curtime = curtime;
		HACKS->global_vars->frametime = frametime;

#ifdef LEGACY
		* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3314)) = predicted_cmd;
		*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x326C)) = updated_cmd;
#else
		* (c_user_cmd**)((std::uintptr_t)HACKS->local + XORN(0x3348)) = predicted_cmd;
		*(c_user_cmd*)((std::uintptr_t)HACKS->local + XORN(0x3298)) = updated_cmd;
#endif
	}

	INLINE void reset() 
	{
		in_prediction = false;
		first_time_predicted = false;

		random_seed = 0;
		prediction_player = 0;

		recoil_index = 0.f;
		accuracy_penalty = 0.f;

		curtime = 0.f;
		frametime = 0.f;

		flags = 0;

		velocity.reset();

		predicted_cmd = nullptr;
		updated_cmd = {};

		ground_entity = 0;
	}
};

struct viewmodel_info_t 
{
	bool looking_at_weapon{};

	int cmd_tick{};
	int sequence{};
	int animation_parity{};
	int new_sequence_parity{};
	int model_index{};

	float anim_time{};
	float old_anim_time{};

	float cycle{};
	float old_cycle{};

	c_base_entity* active_weapon{};

	// don't ask why..
	INLINE void store(c_user_cmd* cmd, c_cs_player* viewmodel) 
	{
		cmd_tick = cmd->command_number;

		auto owner = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(viewmodel->viewmodel_owner()));
		if (owner) {
			looking_at_weapon = owner->looking_at_weapon();
			active_weapon = (c_base_entity*)(HACKS->entity_list->get_client_entity_handle(owner->active_weapon()));
		}

		sequence = viewmodel->sequence();
		animation_parity = viewmodel->animation_parity();
		new_sequence_parity = viewmodel->new_sequence_parity();
		model_index = viewmodel->model_index();
		model_index = viewmodel->model_index();

		anim_time = viewmodel->anim_time();
		old_anim_time = viewmodel->old_anim_time();

		cycle = viewmodel->cycle();
		old_cycle = viewmodel->old_cycle();
	}

	INLINE void reset() 
	{
		looking_at_weapon = false;

		cmd_tick = 0;
		sequence = 0;
		animation_parity = 0;
		new_sequence_parity = 0;
		model_index = 0;

		anim_time = 0.f;
		old_anim_time = 0.f;

		cycle = 0.f;
		old_cycle = 0.f;

		active_weapon = nullptr;
	}
};

struct networked_vars_t
{
	bool done = false;
	bool walking{};
	bool scoped{};

	int command_number{};
	int tickbase{};
	int move_state{};
	int move_type{};
	memory::bits_t flags{};

	float spread{};
	float inaccuracy{};
	float duck_amount{};

	float stamina{};
	float recoil_index{};
	float acc_penalty{};
	float lower_body_yaw{};
	float thirdperson_recoil{};
	float fall_velocity{};
	float velocity_modifier{};

	vec3_t origin{};
	vec3_t abs_origin{};
	vec3_t velocity{};
	vec3_t viewoffset{};
	vec3_t aimpunch{};
	vec3_t aimpunch_vel{};
	vec3_t viewpunch{};
	vec3_t viewangles{};
	vec3_t ladder_normal{};
	vec3_t base_velocity{};
	vec3_t network_origin{};

	std::uint32_t ground_entity{};

	INLINE void store(c_user_cmd* cmd, bool no_ground_entity = false) {
		walking = HACKS->local->is_walking();
		scoped = HACKS->local->is_scoped();

		command_number = cmd->command_number;

		tickbase = HACKS->local->tickbase();
		move_state = HACKS->local->move_state();
		move_type = HACKS->local->move_type();
		flags = HACKS->local->flags();

		if (HACKS->weapon) {
			spread = HACKS->weapon->get_spread();
			inaccuracy = HACKS->weapon->get_inaccuracy();

			recoil_index = HACKS->weapon->recoil_index();
			acc_penalty = HACKS->weapon->accuracy_penalty();
		}

		lower_body_yaw = HACKS->local->lower_body_yaw();
		thirdperson_recoil = HACKS->local->thirdperson_recoil();
		duck_amount = HACKS->local->duck_amount();
		stamina = HACKS->local->stamina();
		fall_velocity = HACKS->local->fall_velocity();
		velocity_modifier = HACKS->local->velocity_modifier();

		origin = HACKS->local->origin();
		abs_origin = HACKS->local->get_abs_origin();
		velocity = HACKS->local->velocity();
		viewoffset = HACKS->local->view_offset();
		aimpunch = HACKS->local->aim_punch_angle();
		aimpunch_vel = HACKS->local->aim_punch_angle_vel();
		viewpunch = HACKS->local->view_punch_angle();
		ladder_normal = HACKS->local->ladder_normal();
		base_velocity = HACKS->local->base_velocity();
		network_origin = HACKS->local->network_origin();

		if (!no_ground_entity)
			ground_entity = HACKS->local->ground_entity();

		viewangles = cmd->viewangles;

		done = true;
	}

	INLINE void restore(bool animations = false) {
		if (!done)
			return;

		if (HACKS->weapon) {
			HACKS->weapon->recoil_index() = recoil_index;
			HACKS->weapon->accuracy_penalty() = acc_penalty;
		}

		HACKS->local->duck_amount() = duck_amount;
		HACKS->local->origin() = origin;
		HACKS->local->set_abs_origin(abs_origin);
		HACKS->local->view_offset() = viewoffset;
		HACKS->local->aim_punch_angle() = aimpunch;
		HACKS->local->aim_punch_angle_vel() = aimpunch_vel;
		HACKS->local->view_punch_angle() = viewpunch;

		if (animations) {
			HACKS->local->is_walking() = walking;
			HACKS->local->is_scoped() = scoped;

			HACKS->local->lower_body_yaw() = lower_body_yaw;
			HACKS->local->thirdperson_recoil() = thirdperson_recoil;

			HACKS->local->move_state() = move_state;
			HACKS->local->move_type() = move_type;

			HACKS->local->ground_entity() = ground_entity;
			HACKS->local->flags().bits = flags.bits;

			HACKS->local->abs_velocity() = HACKS->local->velocity() = velocity;
			HACKS->local->ladder_normal() = ladder_normal;
		}
	}

	INLINE void reset() {
		done = false;
		walking = false;
		scoped = false;

		tickbase = 0;
		flags = 0;

		duck_amount = 0.f;

		spread = 0.f;
		inaccuracy = 0.f;
		stamina = 0.f;
		fall_velocity = 0.f;
		velocity_modifier = 0.f;

		recoil_index = 0.f;
		acc_penalty = 0.f;
		lower_body_yaw = 0.f;
		thirdperson_recoil = 0.f;

		origin.reset();
		abs_origin.reset();
		velocity.reset();
		viewoffset.reset();
		aimpunch.reset();
		aimpunch_vel.reset();
		viewpunch.reset();
		viewangles.reset();
		ladder_normal.reset();
		base_velocity.reset();
		network_origin.reset();

		ground_entity = 0;
	}
};

class c_engine_prediction
{
private:
	c_move_data move_data{};

	networked_vars_t initial_vars{};
	unpred_vars_t unpred_vars{};
	networked_vars_t networked_vars[150]{};
	networked_vars_t restore_vars[150]{};
	viewmodel_info_t viewmodel_info[150]{};
	
	std::string last_sound_name{};
public:
	bool in_prediction{};

	int* prediction_player{};
	int* prediction_random_seed{};

	INLINE networked_vars_t* get_networked_vars(int cmd)
	{
		return &networked_vars[cmd % 150];
	}

	INLINE networked_vars_t* get_vars_to_restore(int cmd)
	{
		return &restore_vars[cmd % 150];
	}

	INLINE networked_vars_t* get_initial_vars()
	{
		return &initial_vars;
	}

	INLINE unpred_vars_t* get_unpredicted_vars()
	{
		return &unpred_vars;
	}

	INLINE c_move_data* get_move_data()
	{
		return &move_data;
	}

	INLINE std::string& get_last_sound_name()
	{
		return last_sound_name;
	}

	INLINE void reset()
	{
		in_prediction = false;
		move_data = {};
		last_sound_name = "";
		unpred_vars.reset();
		initial_vars.reset();

		for (auto& i : networked_vars)
			i.reset();

		for (auto& i : restore_vars)
			i.reset();

		for (auto& i : viewmodel_info)
			i.reset();
	}

	void update();
	void start();
	void update_viewmodel_info(c_user_cmd* cmd);
	void fix_viewmodel(int stage);
	void repredict();
	void end();
};

#ifdef _DEBUG
inline auto ENGINE_PREDICTION = std::make_unique<c_engine_prediction>();
#else
CREATE_DUMMY_PTR(c_engine_prediction);
DECLARE_XORED_PTR(c_engine_prediction, GET_XOR_KEYUI32);

#define ENGINE_PREDICTION XORED_PTR(c_engine_prediction)
#endif