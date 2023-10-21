#pragma once

enum strafe_dir_t
{
	STRAFE_FORWARDS = 0,
	STRAFE_BACKWARDS = 180,
	STRAFE_LEFT = 90,
	STRAFE_RIGHT = -90,
	STRAFE_BACK_LEFT = 135,
	STRAFE_BACK_RIGHT = -135
};

struct auto_peek_info_t 
{
	bool old_move{};
	bool peek_init{};
	bool peek_execute{};

	vec3_t start_pos{};

	std::vector<ImVec2> render_points{};
	
	INLINE bool valid()
	{
		return peek_init && start_pos.valid();
	}

	INLINE void reset()
	{
		old_move = false;
		peek_init = false;
		peek_execute = false;

		start_pos.reset();
		render_points.clear();
	}
};

class c_movement 
{
private:
	bool last_jumped = false;
	bool should_fake = false;
	bool complete_fast_stop = false;

	int ground_ticks{};

	float switch_key = 1.f;
	float circle_yaw = 0.f;
	float old_yaw = 0.f;
	
	vec3_t base_view_angle{};

	auto_peek_info_t peek_info{};

	void update_ground_ticks();

	void auto_jump();
	void auto_strafe();

	void fast_stop();

	void edge_jump();

	bool can_use_auto_peek();
	void auto_peek();

	INLINE float calc_move_angle(c_user_cmd* cmd, float base_angle)
	{
		float angle = std::cos(DEG2RAD(base_angle)) * -cmd->forwardmove + (std::sin(DEG2RAD(base_angle))) * cmd->sidemove;
		return angle;
	}

public:
	INLINE void reset() 
	{
		last_jumped = false;
		should_fake = false;
		complete_fast_stop = false;

		ground_ticks = 0;

		switch_key = 1.f;
		circle_yaw = 0.f;
		old_yaw = 0.f;

		base_view_angle.reset();
		peek_info.reset();
	}

	INLINE vec3_t& get_base_angle()
	{
		return base_view_angle;
	}
	
	INLINE bool on_ground()
	{
		return ground_ticks >= 3;
	}

	INLINE bool is_stop_completed()
	{
		return complete_fast_stop;
	}

	INLINE auto_peek_info_t& get_peek_info()
	{
		return peek_info;
	}

	void instant_stop();
	void rotate_movement(c_user_cmd* cmd, vec3_t& ang);
	void run();
	void run_predicted();
	void render_peek_position();
};

#ifdef _DEBUG
inline auto MOVEMENT = std::make_unique<c_movement>();
#else
CREATE_DUMMY_PTR(c_movement);
DECLARE_XORED_PTR(c_movement, GET_XOR_KEYUI32);

#define MOVEMENT XORED_PTR(c_movement)
#endif