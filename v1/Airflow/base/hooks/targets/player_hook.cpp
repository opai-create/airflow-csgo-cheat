#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../base/sdk/entity.h"

#include "../../../functions/features.h"
#include "../../../functions/config_vars.h"

namespace tr::player
{
	// fix break-lc extrapalation in lagcomp
	int process_interpolated_list()
	{
		static auto original = hooker.original(&process_interpolated_list);

		static auto allow_extrapolation = *patterns::allow_extrapolation.add(0x1).as< bool** >();

		if (allow_extrapolation)
			*allow_extrapolation = false;

		return original();
	}

	void __fastcall interpolate_server_entities(void* ecx, void* edx)
	{
		static auto original = hooker.original(&interpolate_server_entities);
		original(ecx, edx);

		g_local_animation_fix->force_data_for_render();
		g_animation_fix->force_data_for_render();
	}

	// https://www.unknowncheats.me/forum/3523282-post19.html
	bool __fastcall interpolate(void* ecx, void* edx, float time)
	{
		static auto original = hooker.original(&interpolate);

		auto base_entity = (c_baseentity*)ecx;

		auto owner = (c_csplayer*)interfaces::entity_list->get_entity_handle(base_entity->viewmodel_owner());
		if (!owner || owner->index() != g_ctx.local->index())
			return original(ecx, edx, time);

		auto old_interp_amount = interfaces::global_vars->interpolation_amount;
		auto old_pred_tick = owner->final_predicted_tick();

		if (g_exploits->recharge.start && !g_exploits->recharge.finish)
		{
			owner->final_predicted_tick() = owner->tickbase();
			//interfaces::global_vars->interpolation_amount = 0.f;
		}

		bool ret = original(ecx, edx, time);

		interfaces::global_vars->interpolation_amount = old_interp_amount;
		owner->final_predicted_tick() = old_pred_tick;

		return ret;
	}

	bool __fastcall interpolate_player(void* ecx, void* edx, float time)
	{
		static auto original = hooker.original(&interpolate_player);

		auto base_entity = (c_csplayer*)ecx;
		if (!base_entity || !base_entity->is_player() || !g_ctx.local || g_ctx.local != base_entity)
			return original(ecx, edx, time);

		auto old_interp_amount = interfaces::global_vars->interpolation_amount;
		auto old_pred_tick = base_entity->final_predicted_tick();

		if (g_exploits->recharge.start && !g_exploits->recharge.finish)
		{
			base_entity->final_predicted_tick() = base_entity->tickbase();
	//		interfaces::global_vars->interpolation_amount = 0.f;
		}

		bool ret = original(ecx, edx, time);

		interfaces::global_vars->interpolation_amount = old_interp_amount;
		base_entity->final_predicted_tick() = old_pred_tick;

		return ret;
	}

	// disabling bone interpolation
	void __fastcall standard_blending_rules(c_csplayer* ecx, void* edx, c_studiohdr* hdr, vector3d* pos, vector4d* q, float cur_time, int bone_mask)
	{
		static auto original = hooker.original(&standard_blending_rules);
		if (interfaces::client_state->delta_tick == -1 || g_ctx.uninject || !ecx->is_player() || !ecx->is_alive() || !std::isfinite(cur_time))
			return original(ecx, edx, hdr, pos, q, cur_time, bone_mask);

		original(ecx, edx, hdr, pos, q, cur_time, bone_mask);
	}

	void __fastcall add_view_model_bob(void* ecx, void* edx, c_baseentity* model, vector3d& pos, vector3d& angles)
	{
		static auto original = hooker.original(&add_view_model_bob);

		if (g_cfg.misc.removals & viewmodel_move)
			return;

		original(ecx, edx, model, pos, angles);
	}

	// fix viewmodel pos on fakeduck
	void __fastcall calc_viewmodel_view(void* ecx, void* edx, c_csplayer* owner, vector3d& eye_pos, vector3d& eye_angles)
	{
		static auto original = hooker.original(&calc_viewmodel_view);

		if (interfaces::client_state->delta_tick == -1 || g_ctx.uninject || !owner->is_player() || !owner->is_alive() || !g_ctx.local || owner != g_ctx.local)
			return original(ecx, edx, owner, eye_pos, eye_angles);

		if (std::abs(interfaces::global_vars->cur_time - g_local_visuals->last_duck_time) <= 0.2f)
			eye_pos = owner->get_render_origin() + vector3d(0.f, 0.f, interfaces::game_movement->get_player_view_offset(false).z + 0.064f);

		if (g_cfg.misc.removals & vis_recoil)
			eye_angles -= g_ctx.local->aim_punch_angle() * 0.9f + g_ctx.local->view_punch_angle();

		original(ecx, edx, owner, eye_pos, eye_angles);
	}

	void* __fastcall model_renderable_animating(void* ecx, void* edx)
	{
		static auto original = hooker.original(&model_renderable_animating);

		auto player = (c_csplayer*)((uintptr_t)ecx - 4);
		if (!player || player->get_client_class()->class_id != CCSRagdoll)
			return original(ecx, edx);

		return nullptr;
	}

	// remove bone jiggling a.k.a weapon shaking
	void __fastcall build_transformations(c_csplayer* ecx, void* edx, c_studiohdr* hdr, int a3, int a4, int a5, int a6, int a7)
	{
		static auto original = hooker.original(&build_transformations);
		if (interfaces::client_state->delta_tick == -1 || g_ctx.uninject || !ecx || !ecx->is_player() || !ecx->is_alive())
			return original(ecx, edx, hdr, a3, a4, a5, a6, a7);

		auto studio_hdr = ecx->get_studio_hdr();
		auto& jiggle_bones = ecx->jiggle_bones_enabled();
		auto& unk_mask_ptr = *(int*)((uintptr_t)ecx + 0x26B0);

		auto old_jiggle_bones = jiggle_bones;
		jiggle_bones = false;

		auto old_flags = studio_hdr->bone_flags();

		// disable wrong modify body calls
		auto old_mask_ptr = unk_mask_ptr;

		if (ecx != g_ctx.local)
			unk_mask_ptr = 0;

		static int custom_player = *patterns::custom_player_ptr.add(2).as< int* >();

		auto& use_new_animstate = *(bool*)((uintptr_t)ecx + custom_player);

		// disable bone snapshots
		auto old_use_state = use_new_animstate;
		use_new_animstate = false;

		original(ecx, edx, hdr, a3, a4, a5, a6, a7);

		use_new_animstate = old_use_state;
		jiggle_bones = old_jiggle_bones;

		if (ecx != g_ctx.local)
			unk_mask_ptr = old_mask_ptr;
	}

	// allow game to setup bones every frame
	bool __fastcall should_skip_anim_frame(c_csplayer* ecx, void* edx)
	{
		static auto original = hooker.original(&should_skip_anim_frame);
		if (interfaces::client_state->delta_tick == -1 || g_ctx.uninject || !ecx->is_player() || !ecx->is_alive())
			return original(ecx, edx);

		return false;
	}

	void __fastcall on_bbox_change_callback(c_csplayer* ecx, void* edx, vector3d* old_mins, vector3d* new_mins, vector3d* old_maxs, vector3d* new_maxs)
	{
		static auto original = hooker.original(&on_bbox_change_callback);

		if (ecx != g_ctx.local && !g_ctx.modify_body[ecx->index()])
			return;

		original(ecx, edx, old_mins, new_mins, old_maxs, new_maxs);
	}

	// disable leg rotations
	void __fastcall do_extra_bone_processing(c_csplayer* ecx, void* edx, c_studiohdr* hdr, vector3d* pos, vector4d* q, const matrix3x4_t& mat, uint8_t* bone_computed, void* context)
	{
		static auto original = hooker.original(&do_extra_bone_processing);
		if (interfaces::client_state->delta_tick == -1 || g_ctx.uninject || !ecx->is_player() || !ecx->is_alive())
			return original(ecx, edx, hdr, pos, q, mat, bone_computed, context);

		return;
	}

	void __fastcall modify_body_yaw(c_csplayer* ecx, void* edx, matrix3x4_t* bones, int mask)
	{
		static auto original = hooker.original(&modify_body_yaw);

		if (ecx != g_ctx.local && !g_ctx.modify_body[ecx->index()])
			return;

		original(ecx, edx, bones, mask);
	}

	void __fastcall add_renderable(void* ecx, void* edx, c_renderable* pRenderable, bool bRenderWithViewModels, int nType, int nModelType, int nSplitscreenEnabled)
	{
		static auto original = hooker.original(&add_renderable);

		auto renderable_addr = (uintptr_t)pRenderable;
		if (!renderable_addr || renderable_addr == 0x4)
			return original(ecx, edx, pRenderable, bRenderWithViewModels, nType, nModelType, nSplitscreenEnabled);

		auto entity = (c_baseentity*)(renderable_addr - 0x4);
		int index = *(int*)((uintptr_t)entity + 0x64);

		if (index < 1 || index > 64)
			return original(ecx, edx, pRenderable, bRenderWithViewModels, nType, nModelType, nSplitscreenEnabled);

		if (index == interfaces::engine->get_local_player())
			nType = 1;
		else
			nType = 2;

		original(ecx, edx, pRenderable, bRenderWithViewModels, nType, nModelType, nSplitscreenEnabled);
	}

	void __fastcall update_clientside_animation(c_csplayer* ecx, void* edx)
	{
		static auto original = hooker.original(&update_clientside_animation);
		if (interfaces::client_state->delta_tick == -1 || !g_ctx.local || g_ctx.uninject || !ecx->is_player() || !ecx->is_alive())
			return original(ecx, edx);

		int idx = ecx->index();
		if (g_ctx.update[idx])
			original(ecx, edx);
	}

	void __fastcall calc_view(c_csplayer* ecx, void* edx, vector3d& eye_origin, vector3d& eye_angles, float& near_, float& far_, float& fov)
	{
		static auto original = hooker.original(&calc_view);

		if (!g_ctx.local || !g_ctx.weapon || !g_ctx.weapon_info)
			return 	original(ecx, edx, eye_origin, eye_angles, near_, far_, fov);

		if (ecx == g_ctx.local)
		{
			auto old_default_fov = ecx->default_fov();
			auto old_fov_start = ecx->fov_start();
			auto old_fov = ecx->fov();
			auto old_fov_rate = ecx->fov_rate();

			g_local_visuals->on_calc_view();

			static int custom_player = *patterns::custom_player_ptr.add(2).as< int* >();

			auto& use_new_animstate = *(bool*)((uintptr_t)ecx + custom_player);

			// turn off shake on land
			auto old_use_state = use_new_animstate;
			use_new_animstate = false;

			g_ctx.current_fov = ecx->default_fov();

			original(ecx, edx, eye_origin, eye_angles, near_, far_, fov);
			use_new_animstate = old_use_state;

			ecx->default_fov() = old_default_fov;
			ecx->fov_start() = old_fov_start;
			ecx->fov() = old_fov;
			ecx->fov_rate() = old_fov_rate;
		}
		else
			original(ecx, edx, eye_origin, eye_angles, near_, far_, fov);
	}

	void __fastcall modify_eye_position(c_animstate* ecx, void* edx, vector3d& pos)
	{
		return;
	}

	vector3d* __fastcall weapon_shootpos(c_csplayer* ecx, void* edx, vector3d& eye)
	{
		static auto original = hooker.original(&weapon_shootpos);

		if (ecx == g_ctx.local)
		{
			eye = g_engine_prediction->get_eye_pos(g_ctx.last_shoot_angle);
			return &eye;
		}

		return original(ecx, edx, eye);
	}

	bool __fastcall setup_bones(void* ecx, void* edx, matrix3x4_t* bone_to_world, int max_bones, int mask, float time)
	{
		static auto original = hooker.original(&setup_bones);

		auto player = (c_csplayer*)((uintptr_t)ecx - 4);

		if (interfaces::client_state->delta_tick == -1 || g_ctx.uninject || !player || !player->is_player() || !player->is_alive())
			return original(ecx, edx, bone_to_world, max_bones, mask, time);

		int idx = player->index();
		if (g_ctx.setup_bones[idx])
		{
			return original(ecx, edx, bone_to_world, max_bones, mask, time);
		}
		else
		{
			if (bone_to_world)
				std::memcpy(bone_to_world, player->bone_cache().base(), sizeof(matrix3x4_t) * player->bone_cache().count());

			return true;
		}
	}

	vector3d* __fastcall eye_angles(void* ecx, void* edx)
	{
		static auto original = hooker.original(&eye_angles);
		return original(ecx, edx);
	}

	// force crosshair for 0-1 styled crosshairs
	// why not
	bool __fastcall want_reticle_shown(void* ecx, void* edx)
	{
		static auto original = hooker.original(&want_reticle_shown);

		c_basecombatweapon* weapon = g_ctx.weapon;
		if (!weapon)
			return original(ecx, edx);

		if (!g_cfg.misc.snip_crosshair || g_ctx.local->is_scoped())
			return original(ecx, edx);

		if (!weapon || !weapon->is_scoping_weapon())
			return original(ecx, edx);

		if ((uintptr_t)_ReturnAddress() != patterns::return_addr_process_input.as< uintptr_t >())
			return original(ecx, edx);

		return true;
	}
}