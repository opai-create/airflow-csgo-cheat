#include "globals.hpp"
#include "post_processing.hpp"
#include "entlistener.hpp"

void c_post_processing::init()
{
	m_bUseCustomAutoExposureMin = netvars::get_offset(HASH("DT_EnvTonemapController"), HASH("m_bUseCustomAutoExposureMin"));
	m_bUseCustomAutoExposureMax = netvars::get_offset(HASH("DT_EnvTonemapController"), HASH("m_bUseCustomAutoExposureMax"));
	m_flCustomAutoExposureMin = netvars::get_offset(HASH("DT_EnvTonemapController"), HASH("m_flCustomAutoExposureMin"));
	m_flCustomAutoExposureMax = netvars::get_offset(HASH("DT_EnvTonemapController"), HASH("m_flCustomAutoExposureMax"));
	m_bUseCustomBloomScale = netvars::get_offset(HASH("DT_EnvTonemapController"), HASH("m_bUseCustomBloomScale"));
	m_flCustomBloomScale = netvars::get_offset(HASH("DT_EnvTonemapController"), HASH("m_flCustomBloomScale"));
}

void c_post_processing::override_fog()
{
	if (!(g_cfg.misc.custom_fog))
	{
		if (HACKS->convars.fog_override->get_int())
			HACKS->convars.fog_override->set_value(FALSE);

		return;
	}

	HACKS->convars.fog_override->fn_change_callbacks.remove_count();
	HACKS->convars.fog_start->fn_change_callbacks.remove_count();
	HACKS->convars.fog_maxdensity->fn_change_callbacks.remove_count();
	HACKS->convars.fog_end->fn_change_callbacks.remove_count();
	HACKS->convars.fog_color->fn_change_callbacks.remove_count();

	if (!HACKS->convars.fog_override->get_int())
		HACKS->convars.fog_override->set_value(TRUE);

	if (HACKS->convars.fog_start->get_int() != g_cfg.misc.fog_start)
		HACKS->convars.fog_start->set_value(g_cfg.misc.fog_start);

	if (HACKS->convars.fog_end->get_int() != g_cfg.misc.fog_end)
		HACKS->convars.fog_end->set_value(g_cfg.misc.fog_end);

	float density = g_cfg.misc.fog_density / 100.f;
	if (HACKS->convars.fog_maxdensity->get_float() != density)
		HACKS->convars.fog_maxdensity->set_value(density);

	auto cur_clr = g_cfg.misc.world_clr[fog].base();

	char buffer_color[12];
	sprintf_s(buffer_color, 12, CXOR("%i %i %i"), cur_clr.r(), cur_clr.g(), cur_clr.b());

	if (std::strcmp(HACKS->convars.fog_color->get_string(), buffer_color))
		HACKS->convars.fog_color->set_value(buffer_color);
}

void c_post_processing::remove_bloom()
{
	LISTENER_ENTITY->for_each_entity([&](c_base_entity* entity)
	{
		bool& custom_bloom_scale = *(bool*)((std::uintptr_t)entity + m_bUseCustomBloomScale);
		bool& custom_expo_min = *(bool*)((std::uintptr_t)entity + m_bUseCustomAutoExposureMin);
		bool& custom_expo_max = *(bool*)((std::uintptr_t)entity + m_bUseCustomAutoExposureMax);

		custom_bloom_scale = true;
		custom_expo_min = true;
		custom_expo_max = true;

		*(float*)((std::uintptr_t)entity + m_flCustomBloomScale) = g_cfg.misc.custom_bloom ? g_cfg.misc.bloom_scale : 0.f;
		*(float*)((std::uintptr_t)entity + m_flCustomAutoExposureMin) = g_cfg.misc.custom_bloom ? g_cfg.misc.exposure_min : 1.f;
		*(float*)((std::uintptr_t)entity + m_flCustomAutoExposureMax) = g_cfg.misc.custom_bloom ? g_cfg.misc.exposure_max : 1.f;

	}, ENT_TONEMAP);
}

void c_post_processing::remove()
{
	bool sunset_mode = g_cfg.misc.world_modulation & 2;
	bool viewmodel_move_override = (g_cfg.misc.removals & viewmodel_move);

	auto override = should_override_processing();
	auto shadow_override = !sunset_mode && (g_cfg.misc.removals & shadow);

	if (!HACKS->in_game || HACKS->valve_ds)
	{
		override_processing = !override;
		override_shadow = !shadow_override;
		override_viewmodel_move = !viewmodel_move_override;
		return;
	}

	remove_bloom();
	override_fog();

	// UNSAFE!!!!
	if (override_processing != override)
	{
		HACKS->convars.mat_postprocess_enable->fn_change_callbacks.remove_count();
		HACKS->convars.mat_postprocess_enable->set_value((int)(!override));
		override_processing = override;
	}

	if (override_shadow != shadow_override)
	{
		HACKS->convars.cl_csm_shadows->fn_change_callbacks.remove_count();
		HACKS->convars.cl_csm_shadows->set_value((int)(!shadow_override));
		override_shadow = shadow_override;
	}
}