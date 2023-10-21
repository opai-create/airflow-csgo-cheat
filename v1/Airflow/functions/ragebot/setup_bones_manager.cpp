#include "setup_bones_manager.h"

#include "../../base/other/game_functions.h"
#include "../../base/sdk/c_animstate.h"

#include "../config_vars.h"

#include "animfix.h"
#include "../features.h"

namespace bone_merge
{
	uintptr_t& get_bone_merge(c_csplayer* player)
	{
		return *(uintptr_t*)((uintptr_t)player + *patterns::get_bone_merge.as< uintptr_t* >());
	}

	void update_cache(uintptr_t bonemerge)
	{
		func_ptrs::update_merge_cache(bonemerge);
	}
}

mstudio_pose_param_desc_t* get_pose_parameter(c_studiohdr* hdr, int index)
{
	return func_ptrs::get_pose_parameter(hdr, index);
}

float get_pose_parameter_value(c_studiohdr* hdr, int index, float value)
{
	if (index < 0 || index > 24)
		return 0.f;

	mstudio_pose_param_desc_t* pose_parameter_ptr = get_pose_parameter(hdr, index);

	if (!pose_parameter_ptr)
		return 0.f;

	mstudio_pose_param_desc_t pose_parameter = *pose_parameter_ptr;

	if (pose_parameter.loop)
	{
		float wrap = (pose_parameter.start + pose_parameter.end) / 2.f + pose_parameter.loop / 2.f;
		float shift = pose_parameter.loop - wrap;

		value = value - pose_parameter.loop * std::floorf((value + shift) / pose_parameter.loop);
	}

	return (value - pose_parameter.start) / (pose_parameter.end - pose_parameter.start);
}

void merge_matching_poses(uintptr_t& bone_merge, float* poses, float* target_poses)
{
	bone_merge::update_cache(bone_merge);

	if (*(uintptr_t*)(bone_merge + 0x10) && *(uintptr_t*)(bone_merge + 0x8C))
	{
		int* index = (int*)(bone_merge + 0x20);
		for (int i = 0; i < 24; ++i)
		{
			if (*index != -1)
			{
				c_csplayer* target = *(c_csplayer**)(bone_merge + 0x4);
				c_studiohdr* hdr = target->get_studio_hdr();
				float pose_param_value = 0.f;

				if (hdr && *(studio_hdr_t**)hdr && i >= 0)
				{
					float pose = target_poses[i];
					mstudio_pose_param_desc_t* pose_param = get_pose_parameter(hdr, i);

					pose_param_value = pose * (pose_param->end - pose_param->start) + pose_param->start;
				}

				c_csplayer* second_target = *(c_csplayer**)(bone_merge);
				c_studiohdr* second_hdr = second_target->get_studio_hdr();

				poses[*index] = get_pose_parameter_value(second_hdr, *index, pose_param_value);
			}

			++index;
		}
	}
}

void c_bone_builder::store(c_csplayer* player, matrix3x4_t* matrix, int mask)
{
	c_animstate* state = player->animstate();

	this->animating = player;
	this->origin = player == g_ctx.local ? player->get_abs_origin() : player->origin();
	this->layers = player->anim_overlay();
	this->hdr = player->get_studio_hdr();
	this->layer_count = 13;
	this->angles = player->get_abs_angles();
	this->matrix = matrix;
	this->mask = mask;
	this->time = player == g_ctx.local ? g_ctx.predicted_curtime : player->simulation_time();
	this->attachments = false;
	this->ik_ctx = false;
	this->dispatch = true;
	this->poses = player->pose_parameter();
	this->eye_angles = player->eye_angles();

	c_basecombatweapon* weapon = player->get_active_weapon();
	c_basecombatweapon* world_weapon = nullptr;
	if (weapon)
		world_weapon = weapon->get_weapon_world_model();

	if (world_weapon)
		this->poses_world = world_weapon->pose_parameter();
	else
		this->poses_world = player->pose_parameter();

	this->filled = true;
}

void c_bone_builder::setup()
{
	alignas(16) vector3d position[128] = {};
	alignas(16) quaternion q[128] = {};

	c_ikcontext* ik_context = ik_ctx ? (c_ikcontext*)animating->ik_context() : nullptr;

	hdr = animating->get_studio_hdr();

	if (!hdr)
		return;

	uint32_t bone_computed[8];
	std::memset(bone_computed, 0, 8 * sizeof(uint32_t));

	bool sequences_available = !*(int*)(*(uintptr_t*)hdr + 0x150) || *(int*)((uintptr_t)hdr + 0x4);

	if (ik_context)
	{
		ik_context->init(hdr, &angles, &origin, time, math::time_to_ticks(time), mask);

		if (sequences_available)
			this->get_skeleton(position, q);

		animating->update_ik_locks(time);
		ik_context->update_targets(position, q, matrix, (uint8_t*)bone_computed);
		animating->calc_ik_locks(time);
		ik_context->solve_dependencies(position, q, matrix, (uint8_t*)bone_computed);
	}
	else if (sequences_available)
		this->get_skeleton(position, q);

	matrix3x4_t transform;
	transform.angle_matrix(angles, origin);

	this->studio_build_matrices(hdr, transform, position, q, mask, matrix, bone_computed);

	if (mask & bone_used_by_attachment && attachments)
		animating->attachments_helper();

	animating->last_bone_setup_time() = time;

	animating->bone_accessor()->readable_bones |= mask;
	animating->bone_accessor()->writable_bones |= mask;

	static auto invalidate_bone_cache = patterns::invalidate_bone_cache.as< uint64_t >();
	static auto model_bone_counter = *(uintptr_t*)(invalidate_bone_cache + 0xA);

	animating->model_recent_bone_counter() = *(uint32_t*)model_bone_counter;

	// new bones func from 21.09 update
	const auto old_eye_angles = animating->eye_angles();
	animating->eye_angles() = this->eye_angles;

	g_ctx.modify_body[this->animating->index()] = true;
	func_ptrs::modify_body_yaw(this->animating, this->matrix, this->mask);
	g_ctx.modify_body[this->animating->index()] = false;

	animating->eye_angles() = old_eye_angles;
}

bool can_be_animated(c_csplayer* player)
{
	static int custom_player = *patterns::custom_player_ptr.add(2).as< int* >();

	if (!*(bool*)((uintptr_t)player + custom_player) || !player->animstate())
		return false;

	c_basecombatweapon* weapon = player->get_active_weapon();

	if (!weapon)
		return false;

	c_csplayer* world_model = (c_csplayer*)weapon->get_weapon_world_model();

	if (!world_model || *(short*)((uintptr_t)world_model + 0x26E) == -1)
		return player == g_ctx.local;

	return true;
}

void c_bone_builder::get_skeleton(vector3d* position, quaternion* q)
{
	alignas(16) vector3d new_position[128];
	alignas(16) quaternion new_q[128];

	c_ikcontext* ik_context = ik_ctx ? (c_ikcontext*)animating->ik_context() : nullptr;

	alignas(16) char buffer[32];
	alignas(16) bone_setup_t* bone_setup = (bone_setup_t*)&buffer;

	bone_setup->hdr = hdr;
	bone_setup->mask = mask;
	bone_setup->pose_parameter = poses.data();
	bone_setup->pose_debugger = nullptr;

	bone_setup->init_pose(position, q, hdr);
	bone_setup->accumulate_pose(position, q, animating->sequence(), animating->cycle(), 1.f, time, ik_context);

	int layer[13] = {};

	for (int i = 0; i < layer_count; ++i)
	{
		c_animation_layers& final_layer = layers[i];

		if (final_layer.weight > 0.f && final_layer.order != 13 && final_layer.order >= 0 && final_layer.order < layer_count)
			layer[final_layer.order] = i;
	}

	char tmp_buffer[4208];
	c_ikcontext* world_ik = (c_ikcontext*)tmp_buffer;

	c_basecombatweapon* weapon = animating->get_active_weapon();

	c_csplayer* world_weapon = nullptr;
	if (weapon)
		world_weapon = (c_csplayer*)weapon->get_weapon_world_model();

	auto wrong_weapon = [&]()
	{
		if (can_be_animated(animating) && world_weapon)
		{
			uintptr_t bone_merge = bone_merge::get_bone_merge(world_weapon);
			if (bone_merge)
			{
				merge_matching_poses(bone_merge, poses_world.data(), poses.data());

				c_studiohdr* world_hdr = world_weapon->get_studio_hdr();

				world_ik->constructor();
				world_ik->init(world_hdr, &angles, &origin, time, 0, bone_used_by_bone_merge);

				alignas(16) char buffer2[32];
				alignas(16) bone_setup_t* world_setup = (bone_setup_t*)&buffer2;

				world_setup->hdr = world_hdr;
				world_setup->mask = bone_used_by_bone_merge;
				world_setup->pose_parameter = poses_world.data();
				world_setup->pose_debugger = nullptr;

				world_setup->init_pose(new_position, new_q, world_hdr);

				for (int i = 0; i < layer_count; ++i)
				{
					c_animation_layers* layer = &layers[i];

					if (layer && layer->sequence > 1 && layer->weight > 0.f)
					{
						if (dispatch && animating == g_ctx.local)
							animating->update_dispatch_layer(layer, world_hdr, layer->sequence);

						if (!dispatch || layer->second_dispatch_sequence <= 0 || layer->second_dispatch_sequence >= (*(studio_hdr_t**)world_hdr)->local_seq)
							bone_setup->accumulate_pose(position, q, layer->sequence, layer->cycle, layer->weight, time, ik_context);
						else if (dispatch)
						{
							func_ptrs::copy_from_follow(bone_merge, position, q, bone_used_by_bone_merge, new_position, new_q);

							if (ik_context)
								func_ptrs::add_dependencies(ik_context, *(float*)((uintptr_t)animating + 0xA14), layer->sequence, layer->cycle, poses.data(), layer->weight);

							world_setup->accumulate_pose(new_position, new_q, layer->second_dispatch_sequence, layer->cycle, layer->weight, time, world_ik);

							func_ptrs::copy_to_follow(bone_merge, new_position, new_q, bone_used_by_bone_merge, position, q);
						}
					}
				}

				world_ik->destructor();
				return false;
			}
		}
		return true;
	};

	if (wrong_weapon())
	{
		for (int i = 0; i < this->layer_count; ++i)
		{
			int layer_count = layer[i];

			if (layer_count >= 0 && layer_count < this->layer_count)
			{
				c_animation_layers* final_layer = &layers[i];
				bone_setup->accumulate_pose(position, q, final_layer->sequence, final_layer->cycle, final_layer->weight, time, ik_context);
			}
		}
	}

	if (ik_context)
	{
		world_ik->constructor();
		world_ik->init(hdr, &angles, &origin, time, 0, mask);
		bone_setup->calc_autoplay_sequences(position, q, time, world_ik);
		world_ik->destructor();
	}
	else
		bone_setup->calc_autoplay_sequences(position, q, time, nullptr);

	bone_setup->calc_bone_adjust(position, q, (float*)((uintptr_t)animating + 0xA54), mask);
}

void c_bone_builder::studio_build_matrices(c_studiohdr* hdr, const matrix3x4_t& world_transform, vector3d* pos, quaternion* q, int mask, matrix3x4_t* out, uint32_t* bone_computed)
{
	int i = 0;
	int chain_length = 0;
	int bone = -1;
	studio_hdr_t* studio_hdr = *(studio_hdr_t**)hdr;

	if (bone < -1 || bone >= studio_hdr->bones)
		bone = 0;

	c_utlvector< int >* bone_parent = (c_utlvector< int >*)((uintptr_t)hdr + 0x44);
	c_utlvector< int >* bone_flags = (c_utlvector< int >*)((uintptr_t)hdr + 0x30);

	int chain[128];

	if (bone <= -1)
	{
		chain_length = studio_hdr->bones;

		for (i = 0; i < studio_hdr->bones; ++i)
			chain[chain_length - i - 1] = i;
	}
	else
	{
		i = bone;

		do
		{
			chain[chain_length++] = i;
			i = bone_parent->m_memory[i];
		} while (i != -1);
	}

	matrix3x4_t bone_matrix;

	for (int j = chain_length - 1; j >= 0; --j)
	{
		i = chain[j];

		if ((1 << (i & 0x1F)) & bone_computed[i >> 5])
			continue;

		int flag = bone_flags->m_memory[i];
		int parent = bone_parent->m_memory[i];

		if ((flag & mask) && q)
		{
			bone_matrix.quaternion_matrix(q[i], pos[i]);

			if (parent == -1)
				out[i] = world_transform.contact_transforms(bone_matrix);
			else
				out[i] = out[parent].contact_transforms(bone_matrix);
		}
	}
}