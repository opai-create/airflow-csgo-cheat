#include "globals.hpp"
#include "skins.hpp"
#include "predfix.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

INLINE void toggle_key(int key, bool& flip, WPARAM wparam, UINT msg)
{
	if (wparam == key)
	{
		if (msg == WM_KEYUP)
			flip = !flip;
	}
}

namespace hooks
{
	recv_var_proxy_fn original_sequence{};
	recv_var_proxy_fn orignal_velocity_modifier{};
	recv_var_proxy_fn original_simulation_time{};
	recv_var_proxy_fn original_vec_force{};

	WNDPROC old_wndproc;

	LRESULT WINAPI hooked_wnd_proc(HWND wnd, uint32_t msg, WPARAM wp, LPARAM lp)
	{
		toggle_key(VK_INSERT, g_cfg.misc.menu, wp, msg);

#ifdef _DEBUG
		toggle_key(VK_DELETE, HACKS->unload, wp, msg);

		if (HACKS->unload)
			return CallWindowProcA(old_wndproc, wnd, msg, wp, lp);
#endif

		if (WINCALL(GetForegroundWindow)() != HACKS->window)
			return CallWindowProcA(old_wndproc, wnd, msg, wp, lp);

		KEY_STATES->update(msg, wp);

		if (RENDER->done && g_cfg.misc.menu)
		{
			if (ImGui_ImplWin32_WndProcHandler(wnd, msg, wp, lp) || ImGui::GetIO().WantTextInput)
				return true;
		}

		return WINCALL(CallWindowProcA)(old_wndproc, wnd, msg, wp, lp);
	}

	void __cdecl viewmodel_sequence(c_recv_proxy_data* data, void* entity, void* out)
	{
		if (HACKS->local && HACKS->local->is_alive() && HACKS->client_state->delta_tick != -1 && g_cfg.skins.skin_weapon[weapon_cfg_knife].enable)
		{
			auto base_entity = (c_base_entity*)entity;
			if (!base_entity)
				return original_sequence(data, entity, out);

			auto owner = (c_base_entity*)HACKS->entity_list->get_client_entity_handle(base_entity->viewmodel_owner());
			if (!owner || owner->index() != HACKS->local->index())
				return original_sequence(data, entity, out);

			auto view_model_weapon = (c_base_combat_weapon*)HACKS->entity_list->get_client_entity_handle(base_entity->viewmodel_weapon());
			if (!view_model_weapon)
				return original_sequence(data, entity, out);

			auto client_class = view_model_weapon->get_client_class();
			if (!client_class || client_class->class_id != CKnife)
				return original_sequence(data, entity, out);

			data->value.int_ = skin_changer::correct_sequence(view_model_weapon->item_definition_index(), data->value.int_);
			original_sequence(data, entity, out);
		}
		else
			original_sequence(data, entity, out);
	}

	void __cdecl vec_force(c_recv_proxy_data* data, void* entity, void* out)
	{
		constexpr float ragdoll_force_amt = 100000.f;

		auto base_entity = (c_base_entity*)entity;
		if (base_entity)
		{
			auto player = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(base_entity->ragdoll_player()));
			if (g_cfg.misc.ragdoll_gravity > 0 && player && !player->is_teammate())
			{
				auto base_value = vec3_t{ data->value.vector[0], data->value.vector[1], data->value.vector[2] };

				if (g_cfg.misc.ragdoll_gravity == 1)
				{
					base_value.x *= ragdoll_force_amt;
					base_value.y *= ragdoll_force_amt;

					if (base_value.z <= 1.f)
						base_value.z = 2.f;

					base_value.z *= 2.f;
				}
				else
					base_value.z = ragdoll_force_amt * 100.f;

				base_entity->ragdoll_velocity() *= 10000000.f;

				base_value.x = std::clamp(base_value.x, FLT_MIN, FLT_MAX);
				base_value.y = std::clamp(base_value.y, FLT_MIN, FLT_MAX);
				base_value.z = std::clamp(base_value.z, FLT_MIN, FLT_MAX);

				base_entity->ragdoll_velocity().x = std::clamp(base_entity->ragdoll_velocity().x, FLT_MIN, FLT_MAX);
				base_entity->ragdoll_velocity().y = std::clamp(base_entity->ragdoll_velocity().y, FLT_MIN, FLT_MAX);
				base_entity->ragdoll_velocity().z = std::clamp(base_entity->ragdoll_velocity().z, FLT_MIN, FLT_MAX);

				data->value.vector[0] = base_value.x;
				data->value.vector[1] = base_value.y;
				data->value.vector[2] = base_value.z;
			}
		}

		original_vec_force(data, entity, out);
	}

	void __cdecl velocity_modifier(c_recv_proxy_data* data, void* entity, void* out)
	{
		auto base_entity = (c_base_entity*)entity;
		if (base_entity && entity == HACKS->local)
		{
			orignal_velocity_modifier(data, entity, out);

			if (data->value.flt_ != PREDFIX->velocity_modifier)
			{
				PREDFIX->velocity_modifier = data->value.flt_;
			}

			return;
		}

		orignal_velocity_modifier(data, entity, out);
	}

	INLINE void init()
	{
		vmt::init();
		detour::init();
		hooker::hook();

		netvars::hook_prop(HASH("DT_BaseViewModel"), HASH("m_nSequence"), (recv_var_proxy_fn)viewmodel_sequence, &original_sequence);
		netvars::hook_prop(HASH("DT_CSRagdoll"), HASH("m_vecForce"), (recv_var_proxy_fn)vec_force, &original_vec_force);
		netvars::hook_prop(HASH("DT_CSPlayer"), HASH("m_flVelocityModifier"), (recv_var_proxy_fn)velocity_modifier, &orignal_velocity_modifier);

		old_wndproc = (WNDPROC)WINCALL(SetWindowLongA)(HACKS->window, GWL_WNDPROC, (LONG_PTR)hooked_wnd_proc);
	}

	INLINE void remove()
	{
#ifdef _DEBUG
		netvars::hook_prop(HASH("DT_BaseViewModel"), HASH("m_nSequence"), original_sequence, nullptr);
		netvars::hook_prop(HASH("DT_CSRagdoll"), HASH("m_vecForce"), original_vec_force, nullptr);
		netvars::hook_prop(HASH("DT_CSPlayer"), HASH("m_flVelocityModifier"), orignal_velocity_modifier, nullptr);

		hooker::unhook();

		SetWindowLongPtrA(HACKS->window, GWL_WNDPROC, (LONG_PTR)old_wndproc);
#endif
	}
}