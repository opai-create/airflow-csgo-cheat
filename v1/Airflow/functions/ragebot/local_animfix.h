#pragma once
#include <memory>
#include <deque>
#include <unordered_map>

#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

#include "../../base/tools/math.h"
#include "../../base/tools/memory/displacement.h"

#include "../../base/other/game_functions.h"

#include "setup_bones_manager.h"

struct local_anims_t
{
	bool landing{};
	bool started_moving_this_frame{};
	bool stopped_moving_this_frame{};

	int old_movetype{};
	int old_flags{};

	float foot_yaw{};
	float aim_matrix_width_range{};
	float max_desync_range{};

	vector3d v_angle{};
	c_animation_layers sent_layers[13]{};
	c_animation_layers updated_layers[13]{};
	std::array<float, 24> sent_pose_params{};

	c_bone_builder fake_builder{};
	c_bone_builder real_builder{};

	c_animstate rebuilt_state{};

	matrix3x4_t fake_bones[128]{};
	matrix3x4_t real_bones[128]{};

	inline void reset()
	{
		this->landing = false;
		this->started_moving_this_frame = false;
		this->stopped_moving_this_frame = false;
		this->rebuilt_state = {};
		this->old_movetype = 0;
		this->old_flags = 0;
		this->foot_yaw = 0;
		this->aim_matrix_width_range = 0;
		this->max_desync_range = 0;
		this->v_angle.reset();

		std::memset(this->sent_layers, 0, sizeof(this->sent_layers));
		std::memset(this->updated_layers, 0, sizeof(this->updated_layers));
		std::memset(this->real_bones, 0, sizeof(this->real_bones));
		std::memset(this->fake_bones, 0, sizeof(this->fake_bones));

		this->sent_pose_params = {};
		this->fake_builder = {};
		this->real_builder = {};
	}
};

struct fake_animstate_t
{
	bool reset_state{};
	float old_spawn_time{};
	std::uint32_t old_ref_handle{};

	c_animstate* state{};
};

class c_local_animation_fix
{
private:
	void update_fake();
	void update_viewmodel();

	int max_choke{};

	fake_animstate_t fake_animstate{};
	local_anims_t updated_vars{};
	local_anims_t backup_vars{};

public:
	inline fake_animstate_t* get_fake_animstate()
	{
		return &fake_animstate;
	}

	inline local_anims_t* get_updated_netvars()
	{
		return &updated_vars;
	}

	inline void on_local_death()
	{
		this->max_choke = 0;

		this->updated_vars.reset();
		this->backup_vars.reset();
	}

	void update();
	void force_data_for_render();
};