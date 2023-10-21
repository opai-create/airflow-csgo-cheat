#include "globals.hpp"

void c_hacks::convars_t::init()
{
	mp_teammates_are_enemies		= HACKS->cvar->find_convar(CXOR("mp_teammates_are_enemies"));
	weapon_debug_spread_show		= HACKS->cvar->find_convar(CXOR("weapon_debug_spread_show"));

	cl_foot_contact_shadows			= HACKS->cvar->find_convar(CXOR("cl_foot_contact_shadows"));
	cl_csm_shadows					= HACKS->cvar->find_convar(CXOR("cl_csm_shadows"));
	cl_brushfastpath				= HACKS->cvar->find_convar(CXOR("cl_brushfastpath"));

	sv_cheats						= HACKS->cvar->find_convar(CXOR("sv_cheats"));
	sv_skyname						= HACKS->cvar->find_convar(CXOR("sv_skyname"));

	viewmodel_fov					= HACKS->cvar->find_convar(CXOR("viewmodel_fov"));
	weapon_accuracy_nospread		= HACKS->cvar->find_convar(CXOR("weapon_accuracy_nospread"));
	weapon_recoil_scale				= HACKS->cvar->find_convar(CXOR("weapon_recoil_scale"));

	cl_lagcompensation				= HACKS->cvar->find_convar(CXOR("cl_lagcompensation"));
	cl_sidespeed					= HACKS->cvar->find_convar(CXOR("cl_sidespeed"));

	m_pitch							= HACKS->cvar->find_convar(CXOR("m_pitch"));
	m_yaw							= HACKS->cvar->find_convar(CXOR("m_yaw"));
	sensitivity						= HACKS->cvar->find_convar(CXOR("sensitivity"));

	cl_updaterate					= HACKS->cvar->find_convar(CXOR("cl_updaterate"));
	cl_interp						= HACKS->cvar->find_convar(CXOR("cl_interp"));
	cl_interp_ratio					= HACKS->cvar->find_convar(CXOR("cl_interp_ratio"));

	zoom_sensitivity_ratio_mouse	= HACKS->cvar->find_convar(CXOR("zoom_sensitivity_ratio_mouse"));
	mat_fullbright					= HACKS->cvar->find_convar(CXOR("mat_fullbright"));
	mat_postprocess_enable			= HACKS->cvar->find_convar(CXOR("mat_postprocess_enable"));

	fog_override					= HACKS->cvar->find_convar(CXOR("fog_override"));
	fog_start						= HACKS->cvar->find_convar(CXOR("fog_start"));
	fog_end							= HACKS->cvar->find_convar(CXOR("fog_end"));
	fog_maxdensity					= HACKS->cvar->find_convar(CXOR("fog_maxdensity"));
	fog_color						= HACKS->cvar->find_convar(CXOR("fog_color"));

	sv_gravity						= HACKS->cvar->find_convar(CXOR("sv_gravity"));
	sv_maxunlag						= HACKS->cvar->find_convar(CXOR("sv_maxunlag"));
	sv_unlag						= HACKS->cvar->find_convar(CXOR("sv_unlag"));

	cl_csm_rot_override				= HACKS->cvar->find_convar(CXOR("cl_csm_rot_override"));
	cl_csm_rot_x					= HACKS->cvar->find_convar(CXOR("cl_csm_rot_x"));
	cl_csm_rot_y					= HACKS->cvar->find_convar(CXOR("cl_csm_rot_y"));
	cl_csm_rot_z					= HACKS->cvar->find_convar(CXOR("cl_csm_rot_z"));
	cl_csm_max_shadow_dist			= HACKS->cvar->find_convar(CXOR("cl_csm_max_shadow_dist"));

	sv_footsteps					= HACKS->cvar->find_convar(CXOR("sv_footsteps"));
	cl_clock_correction				= HACKS->cvar->find_convar(CXOR("cl_clock_correction"));
	sv_friction						= HACKS->cvar->find_convar(CXOR("sv_friction"));
	sv_stopspeed					= HACKS->cvar->find_convar(CXOR("sv_stopspeed"));
	sv_jump_impulse					= HACKS->cvar->find_convar(CXOR("sv_jump_impulse"));

	weapon_molotov_maxdetonateslope = HACKS->cvar->find_convar(CXOR("weapon_molotov_maxdetonateslope"));
	molotov_throw_detonate_time		= HACKS->cvar->find_convar(CXOR("molotov_throw_detonate_time"));

	mp_damage_scale_ct_head			= HACKS->cvar->find_convar(CXOR("mp_damage_scale_ct_head"));
	mp_damage_scale_ct_body			= HACKS->cvar->find_convar(CXOR("mp_damage_scale_ct_body"));
	mp_damage_scale_t_head			= HACKS->cvar->find_convar(CXOR("mp_damage_scale_t_head"));
	mp_damage_scale_t_body			= HACKS->cvar->find_convar(CXOR("mp_damage_scale_t_body"));

	ff_damage_reduction_bullets		= HACKS->cvar->find_convar(CXOR("ff_damage_reduction_bullets"));
	ff_damage_bullet_penetration	= HACKS->cvar->find_convar(CXOR("ff_damage_bullet_penetration"));
	sv_accelerate					= HACKS->cvar->find_convar(CXOR("sv_accelerate"));

	weapon_accuracy_shotgun_spread_patterns		= HACKS->cvar->find_convar(CXOR("weapon_accuracy_shotgun_spread_patterns"));

	cl_wpn_sway_interp							= HACKS->cvar->find_convar(CXOR("cl_wpn_sway_interp"));
	cl_forwardspeed								= HACKS->cvar->find_convar(CXOR("cl_forwardspeed"));

	sv_minupdaterate							= HACKS->cvar->find_convar(CXOR("sv_minupdaterate"));
	sv_maxupdaterate							= HACKS->cvar->find_convar(CXOR("sv_maxupdaterate"));

	sv_client_min_interp_ratio					= HACKS->cvar->find_convar(CXOR("sv_client_min_interp_ratio"));
	sv_client_max_interp_ratio					= HACKS->cvar->find_convar(CXOR("sv_client_max_interp_ratio"));

	cl_interpolate								= HACKS->cvar->find_convar(CXOR("cl_interpolate"));
	cl_pred_doresetlatch						= HACKS->cvar->find_convar(CXOR("cl_pred_doresetlatch"));
	cl_cmdrate									= HACKS->cvar->find_convar(CXOR("cl_cmdrate"));
	rate										= HACKS->cvar->find_convar(CXOR("rate"));
	r_DrawSpecificStaticProp					= HACKS->cvar->find_convar(CXOR("r_DrawSpecificStaticProp"));
	sv_maxusrcmdprocessticks					= HACKS->cvar->find_convar(CXOR("sv_maxusrcmdprocessticks"));
	sv_clip_penetration_traces_to_players		= HACKS->cvar->find_convar(CXOR("sv_clip_penetration_traces_to_players"));
	sv_accelerate_use_weapon_speed				= HACKS->cvar->find_convar(CXOR("sv_accelerate_use_weapon_speed"));
	sv_maxvelocity								= HACKS->cvar->find_convar(CXOR("sv_maxvelocity"));
	mp_solid_teammates							= HACKS->cvar->find_convar(CXOR("mp_solid_teammates"));
	sv_clockcorrection_msecs					= HACKS->cvar->find_convar(CXOR("sv_clockcorrection_msecs"));
	con_filter_text								= HACKS->cvar->find_convar(CXOR("con_filter_text"));
	con_filter_enable							= HACKS->cvar->find_convar(CXOR("con_filter_enable"));
	sv_airaccelerate							= HACKS->cvar->find_convar(CXOR("sv_airaccelerate"));
	sv_enablebunnyhopping						= HACKS->cvar->find_convar(CXOR("sv_enablebunnyhopping"));
}