#pragma once

struct anim_record_t;

namespace bone_merge
{
	std::uintptr_t& get_bone_merge(c_cs_player* player);
	void update_cache(std::uintptr_t& bone_merge);
}

struct clamp_bones_info_t {
	float collision_change_time{};
	float collision_change_origin{};

	vec3_t origin{};
	vec3_t collision_origin{};
	vec3_t view_offset{};
	vec3_t maxs{};

	std::uint32_t ground_entity{};

	void store(c_cs_player* player);
	void store(anim_record_t* record);
};

extern float get_pose_parameter_value(c_studio_hdr* hdr, int index, float value);

class c_bone_builder
{
private:
	void get_skeleton(vec3_t* position, quaternion_t* q);
	void studio_build_matrices(c_studio_hdr* hdr, const matrix3x4_t& world_transform, vec3_t* pos, quaternion_t* q, int mask, matrix3x4_t* out, uint32_t* bone_computed);

public:
	bool filled{};

	bool ik_ctx{};
	bool attachments{};
	bool dispatch{};

	int mask{};
	int layer_count{};
	int* bone_flags_base{};
	int bone_parent_count{};

	float time{};

	matrix3x4_t* matrix{};
	c_studio_hdr* hdr{};
	c_animation_layers* layers{};
	c_cs_player* animating{};

	vec3_t origin{};
	vec3_t angles{};
	vec3_t eye_angles{};

	std::array<float, 24> poses{};
	std::array<float, 24> poses_world{};

	void clamp_bones_in_bbox(c_cs_player* player, matrix3x4_t* matrix, int mask, float curtime, vec3_t& eye_angles, clamp_bones_info_t& clamp_info);
	void store(c_cs_player* player, matrix3x4_t* matrix, int mask, c_studio_hdr* hdr, int* flags_base, int parent_count);
	void setup();

	INLINE void reset()
	{
		filled = false;
		ik_ctx = false;
		attachments = false;
		bone_parent_count = false;

		mask = 0;
		layer_count = 0;
		layer_count = 0;

		time = 0.f;

		matrix = nullptr;
		hdr = nullptr;
		layers = nullptr;
		animating = nullptr;
		bone_flags_base = nullptr;

		origin.reset();
		angles.reset();
		eye_angles.reset();

		poses = {};
		poses_world = {};
	}
};