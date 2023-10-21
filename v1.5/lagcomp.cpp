#include "globals.hpp"
#include "animations.hpp"
#include "server_bones.hpp"
#include "entlistener.hpp"
#include "lagcomp.hpp"

#ifdef _DEBUG
#define DEBUG_LC 1

#if DEBUG_LC

#endif
#endif

void c_lag_comp::build_roll_matrix(c_cs_player* player, matrix_t* side, int side_index, float& fresh_tick, vec3_t& fresh_angles, clamp_bones_info_t& clamp_info)
{
	RESTORE(player->eye_angles().z);

	math::memcpy_sse(side->roll_matrix, side->matrix, sizeof(side->roll_matrix));
	player->eye_angles().z = 50.f * -side_index;

	auto& builder = side->bone_builder;
	builder.clamp_bones_in_bbox(player, side->roll_matrix, builder.mask, fresh_tick, fresh_angles, clamp_info);
}

void c_lag_comp::clamp_matrix(c_cs_player* player, matrix_t* side, float& fresh_tick, vec3_t& fresh_angles, clamp_bones_info_t& clamp_info) 
{
	auto& builder = side->bone_builder;
	builder.clamp_bones_in_bbox(player, builder.matrix, builder.mask, fresh_tick, fresh_angles, clamp_info);
}

uint8_t* get_server_edict1(c_cs_player* player)
{
	static uintptr_t server_globals = **offsets::server_edict.add(0x2).cast< uintptr_t** >();
	int max_clients = *(int*)((uintptr_t)server_globals + 0x18);
	int index = player->index();
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

void draw_server_hitbox1(c_cs_player* player)
{
	auto duration = HACKS->global_vars->interval_per_tick * 2.f;

	auto server_player = get_server_edict1(player);
	if (server_player)
	{
		static auto call = offsets::draw_hitbox.cast< uintptr_t* >();

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

#ifdef _DEBUG
void threaded_update_tick(std::unique_ptr<c_lag_comp>& ptr, c_cs_player* player, float& fresh_tick)
#else
void threaded_update_tick(c_lag_comp* ptr, c_cs_player* player, float& fresh_tick)
#endif
{
	if (!player->is_alive())
		return;

	auto anim = ANIMFIX->get_anims(player->index());
	if (!anim || !anim->ptr || anim->ptr != player || anim->records.empty() || anim->dormant_ticks < 1)
		return;
	
	auto fresh_angles = player->eye_angles();
	for (auto& i : anim->records)
	{
		i.valid_lc = ptr->is_tick_valid(i.shifting, false, i.sim_time);

#ifndef LEGACY	
		clamp_bones_info_t clamp_info{};
		clamp_info.store(&i);

		auto rec_angles = i.eye_angles.normalized_angle();
		auto angles = vec3_t{ i.eye_angles.x, fresh_angles.y, i.eye_angles.z }.normalized_angle();

		// update orig matrix
		ptr->clamp_matrix(player, &i.matrix_orig, fresh_tick, angles, clamp_info);

		// update left matrix & left roll matrix
		ptr->clamp_matrix(player, &i.matrix_left, fresh_tick, rec_angles, clamp_info);
		ptr->build_roll_matrix(player, &i.matrix_left, -1, fresh_tick, rec_angles, clamp_info);

		// update right matrix & right roll matrix
		ptr->clamp_matrix(player, &i.matrix_right, fresh_tick, rec_angles, clamp_info);
		ptr->build_roll_matrix(player, &i.matrix_right, 1, fresh_tick, rec_angles, clamp_info);

		// update zero matrix
		ptr->clamp_matrix(player, &i.matrix_zero, fresh_tick, rec_angles, clamp_info);
#endif
	}
}

void c_lag_comp::update_tick_validation()
{
	if (!HACKS->in_game || !HACKS->local || HACKS->client_state->delta_tick == -1)
		return;

	auto fresh_tick = HACKS->local->is_alive() ? HACKS->predicted_time : HACKS->global_vars->curtime;
	LISTENER_ENTITY->for_each_player([&](c_cs_player* player)
	{
#ifdef _DEBUG
		THREAD_POOL->add_task(threaded_update_tick, std::ref(LAGCOMP), player, std::ref(fresh_tick));
#else
		THREAD_POOL->add_task(threaded_update_tick, LAGCOMP, player, std::ref(fresh_tick));
#endif
	});

	THREAD_POOL->wait_all();
}