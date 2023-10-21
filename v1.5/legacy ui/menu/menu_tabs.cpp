#include "../../globals.hpp"
#include "../config_system.h"
#include "../config_vars.h"

#include "../legacy_str.h"
#include "menu.h"
#include "../../skins.hpp"

#include <ShlObj.h>
#include <algorithm>
#include <shellapi.h>
#include <Windows.h>
#include <map>

std::vector< std::string > visual_tabs = {
	XOR("Enemy"),
	XOR("Local"),
	XOR("Weapons"),
	XOR("Modulate"),
	XOR("Effects"),
	XOR("Camera"),
};

std::vector< std::string > misc_tabs = {
  XOR("Movement"),
  XOR("Events"),
  XOR("Other"),
};

#define begin_child( text )                                                                                                                                                                                                          \
  ImGui::PushStyleColor( ImGuiCol_Text, ImVec4( 1.f, 1.f, 1.f,  get_alpha( ) ) );                                                                                                                                               \
  ImGui::Text( ( text ) );                                                                                                                                                                                                           \
  ImGui::PopStyleColor( );                                                                                                                                                                                                           \
  ImGui::ItemSize( ImVec2( 0, 1 ) );                                                                                                                                                                                                 \
  ImGui::BeginGroup( );

#define end_child                                                                                                                                                                                                                    \
  ImGui::EndGroup( );                                                                                                                                                                                                                \
  ImGui::ItemSize( ImVec2( 0, 9 ) );

std::vector< std::string > config_list{};
std::vector< std::string > empty_list = { XOR("Config folder is empty!") };

bool update_configs = false;

typedef void (*LPSEARCHFUNC)(LPCTSTR lpszFileName);

BOOL search_files(LPCTSTR lpszFileName, LPSEARCHFUNC lpSearchFunc, BOOL bInnerFolders)
{
	LPTSTR part;
	char tmp[MAX_PATH];
	char name[MAX_PATH];

	HANDLE hSearch = NULL;
	WIN32_FIND_DATA wfd;
	memset(&wfd, 0, sizeof(WIN32_FIND_DATA));

	if (bInnerFolders)
	{
		if (GetFullPathNameA(lpszFileName, MAX_PATH, tmp, &part) == 0)
			return FALSE;
		strcpy(name, part);
		strcpy(part, CXOR("*.*"));
		wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
		if (!((hSearch = FindFirstFileA(tmp, &wfd)) == INVALID_HANDLE_VALUE))
			do
			{
				if (!strncmp(wfd.cFileName, CXOR("."), 1) || !strncmp(wfd.cFileName, CXOR(".."), 2))
					continue;

				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					char next[MAX_PATH];
					if (GetFullPathNameA(lpszFileName, MAX_PATH, next, &part) == 0)
						return FALSE;
					strcpy(part, wfd.cFileName);
					strcat(next, CXOR("\\"));
					strcat(next, name);

					search_files(next, lpSearchFunc, TRUE);
				}
			} while (FindNextFileA(hSearch, &wfd));
			FindClose(hSearch);
	}

	if ((hSearch = FindFirstFileA(lpszFileName, &wfd)) == INVALID_HANDLE_VALUE)
		return TRUE;
	do
		if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			char file[MAX_PATH];
			if (GetFullPathNameA(lpszFileName, MAX_PATH, file, &part) == 0)
				return FALSE;
			strcpy(part, wfd.cFileName);

			lpSearchFunc(wfd.cFileName);
		}
	while (FindNextFileA(hSearch, &wfd));
	FindClose(hSearch);
	return TRUE;
}

void read_configs(LPCTSTR lpszFileName)
{
	config_list.push_back(lpszFileName);
}

void refresh_configs()
{
	std::string folder, file;

	config_list.clear();
	std::string config_dir = CXOR("airflow\\*");
	search_files(config_dir.c_str(), read_configs, FALSE);
}

int selector = 0;
int selector_visuals = 0;

const char* rage_hitboxes[] = {
	  xor_strs::hitbox_head.c_str(),
	  xor_strs::hitbox_body.c_str(),
	  xor_strs::hitbox_limbs.c_str(),
};

const char* weapon_configs[] = {
	  xor_strs::weapon_default.c_str(),
	  xor_strs::weapon_auto.c_str(),
	  xor_strs::weapon_heavy_pistols.c_str(),
	  xor_strs::weapon_pistols.c_str(),
	  xor_strs::weapon_ssg08.c_str(),
	  xor_strs::weapon_awp.c_str(),
};

#ifdef LEGACY
const char* knife_models[]{
	xor_strs::knife_default.c_str(),
	xor_strs::knife_bayonet.c_str(),
	xor_strs::knife_flip.c_str(),
	xor_strs::knife_gut.c_str(),
	xor_strs::knife_karambit.c_str(),
	xor_strs::knife_m9.c_str(),
	xor_strs::knife_huntsman.c_str(),
	xor_strs::knife_falchion.c_str(),
	xor_strs::knife_bowie.c_str(),
	xor_strs::knife_butterfly.c_str(),
};

#else
const char* knife_models[]{
	xor_strs::knife_default.c_str(),
	xor_strs::knife_bayonet.c_str(),
	xor_strs::knife_css.c_str(),
	xor_strs::knife_skeleton.c_str(),
	xor_strs::knife_nomad.c_str(),
	xor_strs::knife_paracord.c_str(),
	xor_strs::knife_survival.c_str(),
	xor_strs::knife_flip.c_str(),
	xor_strs::knife_gut.c_str(),
	xor_strs::knife_karambit.c_str(),
	xor_strs::knife_m9.c_str(),
	xor_strs::knife_huntsman.c_str(),
	xor_strs::knife_falchion.c_str(),
	xor_strs::knife_bowie.c_str(),
	xor_strs::knife_butterfly.c_str(),
	xor_strs::knife_shadow.c_str(),
	xor_strs::knife_ursus.c_str(),
	xor_strs::knife_navaga.c_str(),
	xor_strs::knife_stiletto.c_str(),
	xor_strs::knife_talon.c_str(),
};
#endif

const char* defensive_aa_mode[]
{
	xor_strs::defensive_trigger.c_str(),
	xor_strs::defensive_always.c_str(),
};

const char* legit_weapon_configs[] = {
	xor_strs::weapon_cfg_deagle.c_str(),
	xor_strs::weapon_cfg_duals.c_str(),
	xor_strs::weapon_cfg_fiveseven.c_str(),
	xor_strs::weapon_cfg_glock.c_str(),
	xor_strs::weapon_cfg_ak.c_str(),
	xor_strs::weapon_cfg_aug.c_str(),
	xor_strs::weapon_cfg_awp.c_str(),
	xor_strs::weapon_cfg_famas.c_str(),
	xor_strs::weapon_cfg_g3sg1.c_str(),
	xor_strs::weapon_cfg_galil.c_str(),
	xor_strs::weapon_cfg_m249.c_str(),
	xor_strs::weapon_cfg_m4a1.c_str(),
	xor_strs::weapon_cfg_m4a1s.c_str(),
	xor_strs::weapon_cfg_mac10.c_str(),
	xor_strs::weapon_cfg_p90.c_str(),
	xor_strs::weapon_cfg_mp5.c_str(),
	xor_strs::weapon_cfg_ump45.c_str(),
	xor_strs::weapon_cfg_xm1014.c_str(),
	xor_strs::weapon_cfg_bizon.c_str(),
	xor_strs::weapon_cfg_mag7.c_str(),
	xor_strs::weapon_cfg_negev.c_str(),
	xor_strs::weapon_cfg_sawed_off.c_str(),
	xor_strs::weapon_cfg_tec9.c_str(),
	xor_strs::weapon_cfg_p2000.c_str(),
	xor_strs::weapon_cfg_mp7.c_str(),
	xor_strs::weapon_cfg_mp9.c_str(),
	xor_strs::weapon_cfg_nova.c_str(),
	xor_strs::weapon_cfg_p250.c_str(),
	xor_strs::weapon_cfg_scar20.c_str(),
	xor_strs::weapon_cfg_sg553.c_str(),
	xor_strs::weapon_cfg_scout.c_str(),
	xor_strs::weapon_cfg_usps.c_str(),
	xor_strs::weapon_cfg_cz75.c_str(),
	xor_strs::weapon_cfg_revolver.c_str(),
};

const char* skin_weapon_configs[] = {
	xor_strs::weapon_cfg_deagle.c_str(),
	xor_strs::weapon_cfg_duals.c_str(),
	xor_strs::weapon_cfg_fiveseven.c_str(),
	xor_strs::weapon_cfg_glock.c_str(),
	xor_strs::weapon_cfg_ak.c_str(),
	xor_strs::weapon_cfg_aug.c_str(),
	xor_strs::weapon_cfg_awp.c_str(),
	xor_strs::weapon_cfg_famas.c_str(),
	xor_strs::weapon_cfg_g3sg1.c_str(),
	xor_strs::weapon_cfg_galil.c_str(),
	xor_strs::weapon_cfg_m249.c_str(),
	xor_strs::weapon_cfg_m4a1.c_str(),
	xor_strs::weapon_cfg_m4a1s.c_str(),
	xor_strs::weapon_cfg_mac10.c_str(),
	xor_strs::weapon_cfg_p90.c_str(),
	xor_strs::weapon_cfg_mp5.c_str(),
	xor_strs::weapon_cfg_ump45.c_str(),
	xor_strs::weapon_cfg_xm1014.c_str(),
	xor_strs::weapon_cfg_bizon.c_str(),
	xor_strs::weapon_cfg_mag7.c_str(),
	xor_strs::weapon_cfg_negev.c_str(),
	xor_strs::weapon_cfg_sawed_off.c_str(),
	xor_strs::weapon_cfg_tec9.c_str(),
	xor_strs::weapon_cfg_p2000.c_str(),
	xor_strs::weapon_cfg_mp7.c_str(),
	xor_strs::weapon_cfg_mp9.c_str(),
	xor_strs::weapon_cfg_nova.c_str(),
	xor_strs::weapon_cfg_p250.c_str(),
	xor_strs::weapon_cfg_scar20.c_str(),
	xor_strs::weapon_cfg_sg553.c_str(),
	xor_strs::weapon_cfg_scout.c_str(),
	xor_strs::weapon_cfg_usps.c_str(),
	xor_strs::weapon_cfg_cz75.c_str(),
	xor_strs::weapon_cfg_revolver.c_str(),
	xor_strs::weapon_cfg_knife.c_str(),
};

const char* aa_pitch[] =
{
  xor_strs::aa_disabled.c_str(),
  xor_strs::aa_pitch_down.c_str(),
  xor_strs::aa_pitch_up.c_str(),
};

const char* aa_yaw[] =
{
  xor_strs::aa_disabled.c_str(),
  xor_strs::aa_yaw_back.c_str(),
  xor_strs::aa_yaw_spin.c_str(),
};

const char* aa_jitter_yaw[] =
{
	  xor_strs::aa_disabled.c_str(),
	  xor_strs::aa_jitter_center.c_str(),
	  xor_strs::aa_jitter_offset.c_str(),
	  xor_strs::aa_jitter_random.c_str(),
	  xor_strs::aa_jitter_3way.c_str(),
};

const char* aa_desync_type[] =
{
	  xor_strs::aa_default.c_str(),
	  xor_strs::aa_desync_jitter.c_str(),
};

const char* vis_chams_type[] =
{
	xor_strs::vis_chams_textured.c_str(),
	xor_strs::vis_chams_metallic.c_str(),
	xor_strs::vis_chams_flat.c_str(),
	xor_strs::vis_chams_glass.c_str(),
	xor_strs::vis_chams_glow.c_str(),
	xor_strs::vis_chams_bubble.c_str(),
	xor_strs::vis_chams_money.c_str(),
	xor_strs::vis_chams_fadeup.c_str(),
};

const char* buybot_main[]
{
	xor_strs::buybot_none.c_str(),
	xor_strs::weapon_auto.c_str(),
	xor_strs::weapon_ssg08.c_str(),
	xor_strs::weapon_awp.c_str(),
	xor_strs::weapon_negev.c_str(),
	xor_strs::weapon_m249.c_str(),
	xor_strs::weapon_ak47.c_str(),
	xor_strs::weapon_aug.c_str(),
};

const char* buybot_second[]
{
	xor_strs::buybot_none.c_str(),
	xor_strs::weapon_duals.c_str(),
	xor_strs::weapon_p250.c_str(),
	xor_strs::weapon_cz.c_str(),
	xor_strs::weapon_heavy_pistols.c_str(),
};

const char* enemy_models[]
{
	xor_strs::chams_visible.c_str(),
	xor_strs::chams_xqz.c_str(),
	xor_strs::chams_history.c_str(),
	xor_strs::chams_onshot.c_str(),
	xor_strs::chams_ragdolls.c_str(),
};

const char* local_models[]
{
	xor_strs::chams_visible.c_str(),
	xor_strs::chams_viewmodel.c_str(),
	xor_strs::chams_wpn.c_str(),
	xor_strs::chams_attachments.c_str(),
	//xor_strs::chams_fake.c_str(),
};

const char* hitsound[]
{
	xor_strs::aa_disabled.c_str(),
	xor_strs::sound_metallic.c_str(),
	xor_strs::sound_tap.c_str(),
};

const char* boxes[]
{
	xor_strs::box_default.c_str(),
	xor_strs::box_thin.c_str(),
};

const char* ragdoll_gravities[]
{
	xor_strs::buybot_none.c_str(),
	xor_strs::ragdoll_away.c_str(),
	xor_strs::ragdoll_fly.c_str(),
};

const char* target_selection[]
{
	xor_strs::target_distance.c_str(),
	xor_strs::target_damage.c_str(),
};

const char* agents[]
{
	xor_strs::agent_default.c_str(),
	xor_strs::agent_danger_a.c_str(),
	xor_strs::agent_danger_b.c_str(),
	xor_strs::agent_danger_c.c_str(),
	xor_strs::agent_cmdr_davida.c_str(),
	xor_strs::agent_cmdr_frank.c_str(),
	xor_strs::agent_cmdr_lieutenant.c_str(),
	xor_strs::agent_cmdr_michael.c_str(),
	xor_strs::agent_cmdr_operator.c_str(),
	xor_strs::agent_cmdr_spec_agent_ava.c_str(),
	xor_strs::agent_cmdr_markus.c_str(),
	xor_strs::agent_cmdr_sous.c_str(),
	xor_strs::agent_cmdr_chem_haz.c_str(),
	xor_strs::agent_cmdr_chef_d.c_str(),
	xor_strs::agent_cmdr_aspirant.c_str(),
	xor_strs::agent_cmdr_officer.c_str(),
	xor_strs::agent_cmdr_d_sq.c_str(),
	xor_strs::agent_cmdr_b_sq.c_str(),
	xor_strs::agent_cmdr_seal_team6.c_str(),
	xor_strs::agent_cmdr_bunkshot.c_str(),
	xor_strs::agent_cmdr_lt_commander.c_str(),
	xor_strs::agent_cmdr_bunkshot2.c_str(),
	xor_strs::agent_cmdr_3rd_commando.c_str(),
	xor_strs::agent_cmdr_two_times_.c_str(),
	xor_strs::agent_cmdr_two_times_2.c_str(),
	xor_strs::agent_cmdr_premeiro.c_str(),
	xor_strs::agent_cmdr_cmdr.c_str(),
	xor_strs::agent_cmdr_1st_le.c_str(),
	xor_strs::agent_cmdr_john_van.c_str(),
	xor_strs::agent_cmdr_bio_haz.c_str(),
	xor_strs::agent_cmdr_sergeant.c_str(),
	xor_strs::agent_cmdr_chem_haz__.c_str(),
	xor_strs::agent_cmdr_farwlo.c_str(),
	xor_strs::agent_cmdr_getaway_sally.c_str(),
	xor_strs::agent_cmdr_getaway_number_k.c_str(),
	xor_strs::agent_cmdr_getaway_little_kev.c_str(),
	xor_strs::agent_cmdr_safecracker.c_str(),
	xor_strs::agent_cmdr_bloody_darryl.c_str(),
	xor_strs::agent_cmdr_bloody_loud.c_str(),
	xor_strs::agent_cmdr_bloody_royale.c_str(),
	xor_strs::agent_cmdr_bloody_skullhead.c_str(),
	xor_strs::agent_cmdr_bloody_silent.c_str(),
	xor_strs::agent_cmdr_bloody_miami.c_str(),
	xor_strs::agent_street_solider.c_str(),
	xor_strs::agent_solider.c_str(),
	xor_strs::agent_slingshot.c_str(),
	xor_strs::agent_enforcer.c_str(),
	xor_strs::agent_mr_muhlik.c_str(),
	xor_strs::agent_prof_shahmat.c_str(),
	xor_strs::agent_prof_osiris.c_str(),
	xor_strs::agent_prof_ground_rebek.c_str(),
	xor_strs::agent_prof_elite_muhlik.c_str(),
	xor_strs::agent_prof_trapper.c_str(),
	xor_strs::agent_prof_trapper_aggressor.c_str(),
	xor_strs::agent_prof_vypa_sista.c_str(),
	xor_strs::agent_prof_col_magnos.c_str(),
	xor_strs::agent_prof_crasswater.c_str(),
	xor_strs::agent_prof_crasswater_forgotten.c_str(),
	xor_strs::agent_prof_solman.c_str(),
	xor_strs::agent_prof_romanov.c_str(),
	xor_strs::agent_prof_blackwolf.c_str(),
	xor_strs::agent_prof_maximus.c_str(),
	xor_strs::agent_prof_dragomir.c_str(),
	xor_strs::agent_prof_rezan.c_str(),
	xor_strs::agent_prof_rezan_red.c_str(),
	xor_strs::agent_prof_dragomir2.c_str(),
	xor_strs::agent_gign.c_str(),
	xor_strs::agent_gign_a.c_str(),
	xor_strs::agent_gign_b.c_str(),
	xor_strs::agent_gign_c.c_str(),
	xor_strs::agent_gign_d.c_str(),
	xor_strs::agent_pirate.c_str(),
	xor_strs::agent_pirate_a.c_str(),
	xor_strs::agent_pirate_b.c_str(),
	xor_strs::agent_pirate_c.c_str(),
	xor_strs::agent_pirate_d.c_str(),
	xor_strs::sky_custom.c_str(),
};

const char* masks[]
{
	xor_strs::mask_none.c_str(),
	xor_strs::mask_battle.c_str(),
	xor_strs::mask_hoxton.c_str(),
	xor_strs::mask_doll.c_str(),
	xor_strs::mask_skull.c_str(),
	xor_strs::mask_samurai.c_str(),
	xor_strs::mask_evil_clown.c_str(),
	xor_strs::mask_wolf.c_str(),
	xor_strs::mask_sheep.c_str(),
	xor_strs::mask_bunny_gold.c_str(),
	xor_strs::mask_anaglyph.c_str(),
	xor_strs::mask_kabuki_doll.c_str(),
	xor_strs::mask_dallas.c_str(),
	xor_strs::mask_pumpkin.c_str(),
	xor_strs::mask_sheep_bloody.c_str(),
	xor_strs::mask_devil_plastic.c_str(),
	xor_strs::mask_boar.c_str(),
	xor_strs::mask_chains.c_str(),
	xor_strs::mask_tiki.c_str(),
	xor_strs::mask_bunny.c_str(),
	xor_strs::mask_sheep_gold.c_str(),
	xor_strs::mask_zombie_plastic.c_str(),
	xor_strs::mask_chicken.c_str(),
	xor_strs::mask_skull_gold.c_str(),
	xor_strs::mask_demon_man.c_str(),
	xor_strs::mask_engineer.c_str(),
	xor_strs::mask_heavy.c_str(),
	xor_strs::mask_medic.c_str(),
	xor_strs::mask_pyro.c_str(),
	xor_strs::mask_scout.c_str(),
	xor_strs::mask_sniper.c_str(),
	xor_strs::mask_solider.c_str(),
	xor_strs::mask_spy.c_str(),
	xor_strs::mask_holiday_light.c_str(),
};

const char* skyboxes[]
{
	xor_strs::aa_default.c_str(),
	xor_strs::sky_tibet.c_str(),
	xor_strs::sky_bagage.c_str(),
	xor_strs::sky_italy.c_str(),
	xor_strs::sky_jungle.c_str(),
	xor_strs::sky_office.c_str(),
	xor_strs::sky_daylight.c_str(),
	xor_strs::sky_daylight2.c_str(),
	xor_strs::sky_vertigo_blue.c_str(),
	xor_strs::sky_vertigo.c_str(),
	xor_strs::sky_day.c_str(),
	xor_strs::sky_nuke_bank.c_str(),
	xor_strs::sky_venice.c_str(),
	xor_strs::sky_daylight3.c_str(),
	xor_strs::sky_daylight4.c_str(),
	xor_strs::sky_cloudy.c_str(),
	xor_strs::sky_night.c_str(),
	xor_strs::sky_nightb.c_str(),
	xor_strs::sky_night_flat.c_str(),
	xor_strs::sky_dust.c_str(),
	xor_strs::sky_vietnam.c_str(),
	xor_strs::sky_lunacy.c_str(),
	xor_strs::sky_embassy.c_str(),
	xor_strs::sky_custom.c_str(),
};

const char* glove_models[]
{
	xor_strs::glove_default.c_str(),
	xor_strs::glove_bloodhound.c_str(),
	xor_strs::glove_sporty.c_str(),
	xor_strs::glove_slick.c_str(),
	xor_strs::glove_leather_wrap.c_str(),
	xor_strs::glove_motorcycle.c_str(),
	xor_strs::glove_specialist.c_str(),
	xor_strs::glove_hydra.c_str(),
};

const char* glove_skins[]
{
	xor_strs::glove_skin_charred.c_str(),
	xor_strs::glove_skin_snakebite.c_str(),
	xor_strs::glove_skin_bronzed.c_str(),
	xor_strs::glove_skin_leather.c_str(),
	xor_strs::glove_skin_spruce.c_str(),
	xor_strs::glove_skin_lunar.c_str(),
	xor_strs::glove_skin_convoy.c_str(),
	xor_strs::glove_skin_crimson.c_str(),
	xor_strs::glove_skin_superconductor.c_str(),
	xor_strs::glove_skin_arid.c_str(),
	xor_strs::glove_skin_slaugher.c_str(),
	xor_strs::glove_skin_eclipse.c_str(),
	xor_strs::glove_skin_spearmint.c_str(),
	xor_strs::glove_skin_boom.c_str(),
	xor_strs::glove_skin_coolmint.c_str(),
	xor_strs::glove_skin_forest.c_str(),
	xor_strs::glove_skin_crimson_kimono.c_str(),
	xor_strs::glove_skin_emerald_web.c_str(),
	xor_strs::glove_skin_foundation.c_str(),
	xor_strs::glove_skin_badlands.c_str(),
	xor_strs::glove_skin_pandora.c_str(),
	xor_strs::glove_skin_hedge.c_str(),
	xor_strs::glove_skin_guerilla.c_str(),
	xor_strs::glove_skin_diamondback.c_str(),
	xor_strs::glove_skin_king.c_str(),
	xor_strs::glove_skin_imperial.c_str(),
	xor_strs::glove_skin_overtake.c_str(),
	xor_strs::glove_skin_racing.c_str(),
	xor_strs::glove_skin_amphibious.c_str(),
	xor_strs::glove_skin_bronze.c_str(),
	xor_strs::glove_skin_omega.c_str(),
	xor_strs::glove_skin_vice.c_str(),
	xor_strs::glove_skin_pow.c_str(),
	xor_strs::glove_skin_turtle.c_str(),
	xor_strs::glove_skin_transport.c_str(),
	xor_strs::glove_skin_polygon.c_str(),
	xor_strs::glove_skin_cobalt.c_str(),
	xor_strs::glove_skin_overprint.c_str(),
	xor_strs::glove_skin_duct.c_str(),
	xor_strs::glove_skin_arboreal.c_str(),
	xor_strs::glove_skin_emerald.c_str(),
	xor_strs::glove_skin_mangrove.c_str(),
	xor_strs::glove_skin_rattler.c_str(),
	xor_strs::glove_skin_case.c_str(),
	xor_strs::glove_skin_crimson_web.c_str(),
	xor_strs::glove_skin_buñkshot.c_str(),
	xor_strs::glove_skin_fade.c_str(),
	xor_strs::glove_skin_mogul.c_str(),
};

const char* tracers[]
{
	xor_strs::tracer_beam.c_str(),
	xor_strs::tracer_line.c_str(),
//	xor_strs::tracer_glow.c_str(),
};

#if ALPHA || _DEBUG || BETA
const char* jitterfix_method[]
{
	"Old angle",
	"Angle diff"
};
#endif

void c_menu::draw_ui_items()
{
	auto window_pos = get_window_pos();
	auto prev_pos = ImGui::GetCursorPos();

	auto new_pos = ImVec2(215, 77);
	ImGui::SetCursorPos(new_pos);

	auto list = get_draw_list();

	ImRect window_bb = ImRect(window_pos + ImVec2(160, 47), window_pos + ImVec2(725, 520));
	ImGui::PushClipRect(window_bb.Min, window_bb.Max, false);

	float old_alpha = get_alpha();

	static int prev_sel = 0;
	if (tab_selector != prev_sel)
	{
		for (auto& a : item_animations)
		{
			a.second.reset();
			tab_alpha = 0.f;
		}

		prev_sel = tab_selector;
	}

	// change alpha everytime user clicks on new tab
	create_animation(tab_alpha, g_cfg.misc.menu && tab_selector == prev_sel, 0.2f, skip_disable | lerp_animation);

	alpha = tab_alpha;

	auto render_warning_message = [&](const std::string& type, const std::string& type2)
	{
		ImGui::PushFont(RENDER->fonts.dmg.get());

		auto old_pos = ImGui::GetCursorPos();

#ifdef LEGACY
		auto text = /*XOR("Please turn off ") + type +*/ type2 + XOR(" is not available on legacy!");
#else
		auto text = /*XOR("Please turn off ") + type +*/ XOR("Currently ") + type2 + XOR(" is under construction!");
#endif
		auto text_size = ImGui::CalcTextSize(text.c_str());

		auto half_text_size = ImVec2(text_size.x / 2.f, text_size.y / 2.f);
		auto image_size = ImVec2(32, 32);

		auto half_window = ((window_bb.Min + window_bb.Max) - ImVec2(0.f, image_size.y * 2.f)) / 2.f;
		auto half_image = image_size / 2.f;

		list->AddImage((void*)warning_texture, half_window - half_image, half_window + half_image, ImVec2(0, 0), ImVec2(1, 1), c_color(255, 255, 255, 150 * alpha).as_imcolor());

		ImGui::SetCursorPos(ImVec2(new_pos.x + 110.f / 2.f - half_text_size.x, new_pos.y + 290.f / 2.f - half_text_size.y));

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 0.6f * alpha));
		ImGui::Text(text.c_str());
		ImGui::PopStyleColor();

		ImGui::PopFont();
		ImGui::SetCursorPos(old_pos);
	};

	if (tab_selector == 6)
	{
		if (!update_configs)
		{
			refresh_configs();
			update_configs = true;
		}
	}
	else
		update_configs = false;

	ImGui::BeginChild(CXOR("##tab_child"), ImVec2(), true);
	{
		switch (tab_selector)
		{
		case 0:
		{
			// please turn off legit before configuring rage!
			//if (g_cfg.legit.enable)
			//	render_warning_message(XOR("legit"), XOR("rage"));
		//	else
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("Choose your weapon"))
					{
						combo(CXOR("Setting up..."), &g_cfg.rage.group_type, weapon_configs, IM_ARRAYSIZE(weapon_configs));
						if (g_cfg.rage.group_type > 0)
						{
							auto name = XOR("Enable config##group_") + std::to_string(g_cfg.rage.group_type);
							checkbox(name.c_str(), &g_cfg.rage.weapon[g_cfg.rage.group_type].enable);
						}
					}
					end_child;

					begin_child(CXOR("General"))
					{
						checkbox(CXOR("Enable"), &g_cfg.rage.enable);
						checkbox(CXOR("Auto fire"), &g_cfg.rage.auto_fire);

						checkbox(CXOR("Anti aim correction"), &g_cfg.rage.resolver);
					}
					end_child;

					begin_child(CXOR("Exploits"))
					{
						key_bind(CXOR("Double tap"), g_cfg.binds[dt_b]);

#ifndef LEGACY
						key_bind(CXOR("Hide shots"), g_cfg.binds[hs_b]);
#endif

						checkbox(CXOR("Defensive in air"), &g_cfg.rage.air_defensive);
					}
					end_child;

					begin_child(CXOR("Bindings"))
					{
						key_bind(CXOR("Override damage"), g_cfg.binds[override_dmg_b]);
						key_bind(CXOR("Force body"), g_cfg.binds[force_body_b]);

#ifndef LEGACY
						key_bind(CXOR("Force safe point"), g_cfg.binds[force_sp_b]);
#endif

					}
					end_child;

					begin_child(CXOR("Ping spike"))
					{
						int max_spike_amount = (int)(HACKS->max_unlag * 1000.f);

						key_bind(CXOR("Ping spike"), g_cfg.binds[spike_b]);
						slider_int(CXOR("Amount"), &g_cfg.rage.spike_amt, 0, max_spike_amount, CXOR("%d%ms"));
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					auto& weapon_settings = g_cfg.rage.weapon[g_cfg.rage.group_type];

					begin_child(CXOR("Accuracy"))
					{
						if (g_cfg.rage.group_type == global || g_cfg.rage.group_type == auto_snipers || g_cfg.rage.group_type == scout || g_cfg.rage.group_type == awp)
							checkbox(CXOR("Auto scope"), &weapon_settings.auto_scope);

						auto dmg_str = weapon_settings.mindamage == 100 ? CXOR("HP") : weapon_settings.mindamage > 100 ? CXOR("HP + ") + std::to_string(weapon_settings.mindamage - 100) : CXOR("%dHP");
						auto override_str = weapon_settings.damage_override == 100 ? CXOR("HP") : weapon_settings.damage_override > 100 ? CXOR("HP + ") + std::to_string(weapon_settings.damage_override - 100) : CXOR("%dHP");

				//		multi_combo(CXOR("Skip hitchance"), weapon_settings.hitchance_skips, { XOR("Low spread"), XOR("DT / Still") });

						slider_int(CXOR("Hitchance"), &weapon_settings.hitchance, 0, 100, CXOR("%d%%"));

						slider_int(CXOR("Min damage"), &weapon_settings.mindamage, 1, 120, dmg_str.c_str());
						slider_int(CXOR("Min damage (override)"), &weapon_settings.damage_override, 1, 120, override_str.c_str());

						checkbox(CXOR("Quick stop"), &weapon_settings.quick_stop);
						multi_combo(CXOR("Options##quick_stop"), weapon_settings.quick_stop_options, { CXOR("Early"), CXOR("Between shots"), CXOR("Force accuracy"), CXOR("In air")/*, CXOR("Force duck")*/ });
					}
					end_child;

					begin_child(CXOR("Hitscan"))
					{
						multi_combo(CXOR("Hitboxes"), weapon_settings.hitboxes, { XOR("Head"), XOR("Chest"), XOR("Stomach"), XOR("Pelvis"), XOR("Arms"), XOR("Legs") });

						static auto percent_xored = XOR("%d");

						auto scale_head_str = weapon_settings.scale_head == -1 ? CXOR("Auto") : percent_xored;
						auto scale_body_str = weapon_settings.scale_body == -1 ? CXOR("Auto") : percent_xored;

						slider_int(CXOR("Head scale"), &weapon_settings.scale_head, -1, 100, scale_head_str.c_str());
						slider_int(CXOR("Body scale"), &weapon_settings.scale_body, -1, 100, scale_body_str.c_str());
						checkbox( CXOR("Prefer body"), &weapon_settings.prefer_body );
						checkbox( CXOR("Prefer safe point"), &weapon_settings.prefer_safe );

				/*		checkbox(CXOR("Prefer safe point"), &weapon_settings.prefer_safe);
						checkbox(CXOR("Prefer body"), &weapon_settings.prefer_body);*/
					}
					end_child;

					//begin_child(CXOR("Roll resolver"))
					//{
					//	key_bind(CXOR("Roll override"), g_cfg.binds[force_roll_b]);
					//	slider_int(CXOR("Angle"), &g_cfg.rage.roll_amt, -90, 90, CXOR("%d degree"));
					//	slider_int(CXOR("Pitch angle"), &g_cfg.rage.roll_amt_pitch, 0, 20, CXOR("%d degree"));
					//}
					//end_child;
				}
			}
		}
		break;
		case 1:
		{
			// please turn off rage before configuring legit!
		//	if (g_cfg.rage.enable)
				render_warning_message(XOR("rage"), XOR("legit"));
			/*else
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("Choose your weapon    "))
					{
						combo(CXOR("Setting up...##legit"), &g_cfg.legit.group_type, legit_weapon_configs, IM_ARRAYSIZE(legit_weapon_configs));
					}
					end_child;

					begin_child(CXOR("General     "))
					{
						checkbox(CXOR("Enable##legit_cfg"), &g_cfg.legit.enable);

						key_bind(CXOR("Aim##legit_cfg"), g_cfg.binds[toggle_legit_b]);
						checkbox(CXOR("Autowall##legit_cfg"), &g_cfg.legit.autowall);

						auto dmg_str = g_cfg.legit.min_damage == 100 ? CXOR("HP") : g_cfg.legit.min_damage > 100 ? CXOR("HP + ") + std::to_string(g_cfg.legit.min_damage - 100) : CXOR("%dHP");

						slider_int(CXOR("Min damage"), &g_cfg.legit.min_damage, 1, 120, dmg_str.c_str());

						checkbox(CXOR("Disable on flash"), &g_cfg.legit.flash_check);
						checkbox(CXOR("Disable on smoke"), &g_cfg.legit.smoke_check);
					}
					end_child;

					begin_child(CXOR("RCS settings "))
					{
						auto& rcs_cfg = g_cfg.legit.rcs;
						checkbox(CXOR("Enable##legit_aimbot_rcs"), &rcs_cfg.enable);

						slider_int(CXOR("Amount##legit_aimbot_rcs"), &rcs_cfg.amount, 1, 100, CXOR("%d%%"));
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("Weapon settings "))
					{
						auto& legit_cfg = g_cfg.legit.legit_weapon[g_cfg.legit.group_type];
						checkbox(CXOR("Enable##legit_aimbot"), &legit_cfg.enable);
						checkbox(CXOR("Draw FOV##legit_aimbot"), &legit_cfg.fov_cicle);
						color_picker(CXOR("FOV color##legit_aimbot"), legit_cfg.circle_color);
						checkbox(CXOR("Backtrack##legit_aimbot"), &legit_cfg.backtrack);
						checkbox(CXOR("Quick stop##legit_aimbot"), &legit_cfg.quick_stop);

						multi_combo(CXOR("Hitboxes"), legit_cfg.hitboxes, { XOR("Head"), XOR("Chest"), XOR("Stomach"), XOR("Pelvis") });

						slider_int(CXOR("FOV"), &legit_cfg.fov, 1, 100);
						slider_int(CXOR("Smooth"), &legit_cfg.smooth, 1, 100);

						slider_int(CXOR("Shot delay"), &legit_cfg.aim_delay, 0, 1000, CXOR("%d% ms"));
					}
					end_child;
				}
			}*/
		}
		break;
		case 2:
		{
			ImGui::Columns(2, 0, false);
			ImGui::SetColumnOffset(1, 270);
			{
				begin_child(CXOR("Main"))
				{
					checkbox(CXOR("Enable##antiaim"), &g_cfg.antihit.enable);
#ifndef LEGACY
					//checkbox(CXOR("Silent on-shot##antiaim"), &g_cfg.antihit.silent_onshot);
#endif
					checkbox(CXOR("At targets##antiaim"), &g_cfg.antihit.at_targets);

					combo(CXOR("Pitch"), &g_cfg.antihit.pitch, aa_pitch, IM_ARRAYSIZE(aa_pitch));
					combo(CXOR("Yaw"), &g_cfg.antihit.yaw, aa_yaw, IM_ARRAYSIZE(aa_yaw));
					slider_int(CXOR("Yaw add"), &g_cfg.antihit.yaw_add, -180, 180);

					combo(CXOR("Yaw jitter"), &g_cfg.antihit.jitter_mode, aa_jitter_yaw, IM_ARRAYSIZE(aa_jitter_yaw));
					if (g_cfg.antihit.jitter_mode > 0)
						slider_int(CXOR("Range##jitter"), &g_cfg.antihit.jitter_range, -180, 180);

					key_bind(CXOR("Edge yaw"), g_cfg.binds[edge_b]);
					key_bind(CXOR("Freestanding"), g_cfg.binds[freestand_b]);

					key_bind(CXOR("Manual left"), g_cfg.binds[left_b]);
					key_bind(CXOR("Manual right"), g_cfg.binds[right_b]);
					key_bind(CXOR("Manual back"), g_cfg.binds[back_b]);
				}
				end_child;

#ifndef LEGACY
				begin_child(CXOR("Defensive Anti-Aims"))
				{
					checkbox(CXOR("Change pitch##antiaim"), &g_cfg.antihit.def_pitch);
					checkbox(CXOR("Change yaw##antiaim"), &g_cfg.antihit.def_yaw);

					combo(CXOR("Defensive type##antiaim"), &g_cfg.antihit.def_aa_mode, defensive_aa_mode, IM_ARRAYSIZE(defensive_aa_mode));
				}
				end_child;
#endif
			}
			ImGui::NextColumn();
			{
				begin_child(CXOR("Fake lag"))
				{
					checkbox(CXOR("Enable##fakelag"), &g_cfg.antihit.fakelag);

					multi_combo(CXOR("Conditions##fakelag"), g_cfg.antihit.fakelag_conditions,
						{
						  XOR("Standing"),
						  XOR("Moving"),
						  XOR("Air"),
						});

					slider_int(CXOR("Limit"), &g_cfg.antihit.fakelag_limit, 1, HACKS->max_choke ? HACKS->max_choke + 1 : 15, CXOR("%d ticks"));

					checkbox(CXOR("Fluctuate in air##fakelag"), &g_cfg.antihit.fluctiate_in_air);
				}
				end_child;

#ifndef LEGACY
				begin_child(CXOR("Enhancement"))
				{
				//	checkbox(CXOR("Jitter move"), &g_cfg.antihit.jitter_move);
					checkbox(CXOR("Randomize yaw jitter"), &g_cfg.antihit.random_jitter);
					checkbox(CXOR("Randomize fake jitter"), &g_cfg.antihit.random_dsy);
					checkbox(CXOR("Randomize fake amount"), &g_cfg.antihit.random_amount);
				}
				end_child;
#endif

				begin_child(CXOR("Movement"))
				{
#ifdef LEGACY
					key_bind(CXOR("Fake walk"), g_cfg.binds[sw_b]);
#else
					key_bind(CXOR("Fake duck"), g_cfg.binds[fd_b]);
					key_bind(CXOR("Slow walk"), g_cfg.binds[sw_b]);
#endif
				}
				end_child;

				begin_child(CXOR("Fake angle"))
				{
					checkbox(CXOR("Enable##fake_angle"), &g_cfg.antihit.desync);
					key_bind(CXOR("Inverter"), g_cfg.binds[inv_b]);
#ifndef LEGACY
					combo(CXOR("Fake type"), &g_cfg.antihit.desync_mode, aa_desync_type, IM_ARRAYSIZE(aa_desync_type));
					slider_int(CXOR("Left amount##desync"), &g_cfg.antihit.desync_left, 0, 58, CXOR("%d"));
					slider_int(CXOR("Right amount##desync"), &g_cfg.antihit.desync_right, 0, 58, CXOR("%d"));
#else
				//	combo(CXOR("Breaker type"), &g_cfg.antihit.desync_mode, aa_desync_type, IM_ARRAYSIZE(aa_desync_type));
					slider_int(CXOR("LBY Delta##desync"), &g_cfg.antihit.desync_left, 0, 180, CXOR("%d"));
#endif
				}
				end_child;

#ifndef LEGACY
				begin_child(CXOR("Extended fake"))
				{
					checkbox(CXOR("Enable##distortion"), &g_cfg.antihit.distortion);
					key_bind(CXOR("Force extend"), g_cfg.binds[ens_lean_b]);

					slider_int(CXOR("Amount##body_lean"), &g_cfg.antihit.distortion_range, 0, 100, CXOR("%d%%"));
					slider_int(CXOR("Height##body_lean"), &g_cfg.antihit.distortion_pitch, 0, 50);
				}
				end_child;
#endif
			}
		}
		break;
		case 3:
		{
			static int selector = 0;
			draw_sub_tabs(selector, visual_tabs);

			static int prev_sub_sel = 0;
			if (selector != prev_sub_sel)
			{
				for (auto& a : item_animations)
				{
					a.second.reset();
					subtab_alpha = 0.f;
				}

				prev_sub_sel = selector;
			}

			create_animation(subtab_alpha, g_cfg.misc.menu && selector == prev_sub_sel, 0.2f, skip_disable | lerp_animation);
			alpha = subtab_alpha * tab_alpha;

			ImGui::SetCursorPosX(0);
			ImGui::BeginChild(CXOR("subtab_vis"), {}, false);
			switch (selector)
			{
			case 0:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child("General ");
					{
						auto& enemy_esp = g_cfg.visuals.esp[esp_enemy];
						checkbox(CXOR("Enable##esp_enemy"), &enemy_esp.enable);
						multi_combo(CXOR("Elements##esp_enemy"), enemy_esp.elements,
							{
							  XOR("Box"),
							  XOR("Name"),
							  XOR("Health"),
							  XOR("Icon"),
							  XOR("Weapon"),
							  XOR("Ammo"),
							  XOR("Flags"),
							  XOR("OOF Arrow"),
							  XOR("Glow"),
#if _DEBUG || ALPHA || BETA
							  XOR("Resolver mode"),
#endif
							//  XOR("Skeleton"),
							});

						if (enemy_esp.elements & 1)
							color_picker(CXOR("Box color##esp_enemy"), enemy_esp.colors.box);

						if (enemy_esp.elements & 2)
							color_picker(CXOR("Name color##esp_enemy"), enemy_esp.colors.name);

						if (enemy_esp.elements & 4)
							color_picker(CXOR("Health color##esp_enemy"), enemy_esp.colors.health);

						if (enemy_esp.elements & 8 || enemy_esp.elements & 16)
							color_picker(CXOR("Weapon color##esp_enemy"), enemy_esp.colors.weapon);

						if (enemy_esp.elements & 32)
							color_picker(CXOR("Ammo color##esp_enemy"), enemy_esp.colors.ammo_bar);

						if (enemy_esp.elements & 256)
							color_picker(CXOR("Glow color##esp_enemy"), enemy_esp.colors.glow);

						/*if (enemy_esp.elements & 512)
							color_picker(CXOR("Skeleton color##esp_enemy"), enemy_esp.colors.skeleton);*/
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					auto& enemy_esp = g_cfg.visuals.esp[esp_enemy];

					begin_child(CXOR("Chams  "));
					{
						static int chams_model = 0;

						combo(CXOR("Type"), &chams_model, enemy_models, IM_ARRAYSIZE(enemy_models));

						auto& enemy_config = g_cfg.visuals.chams[chams_model];

						checkbox(CXOR("Enable"), &enemy_config.enable);
						if (chams_model == 2)
							checkbox(CXOR("Full history"), &g_cfg.visuals.show_all_history);

						combo(CXOR("Material"), &enemy_config.material, vis_chams_type, IM_ARRAYSIZE(vis_chams_type));
						color_picker(CXOR("Material color"), enemy_config.main_color);
						if (enemy_config.material == 4)
						{
							slider_int(CXOR("Fill"), &enemy_config.glow_fill, 0, 100, CXOR("%d%%"));
							color_picker(CXOR("Glow color"), enemy_config.glow_color);
						}

						if (chams_model == 3)
							slider_int(CXOR("Shot duration"), &enemy_config.shot_duration, 1, 10, CXOR("%d s"));

						if (enemy_config.material == 6)
							input_text(CXOR("Sprite path"), enemy_config.chams_sprite, 128);
					}
					end_child;

					begin_child(CXOR("Player OOF "));
					{
						slider_int(CXOR("Distance  "), &enemy_esp.offscreen_dist, 1, 450, CXOR("%d ft"));
						slider_int(CXOR("Size  "), &enemy_esp.offscreen_size, 5, 25, CXOR("%d px"));
						color_picker(CXOR("Color##esp_enemy_arrow"), enemy_esp.colors.offscreen_arrow);
						color_picker(CXOR("Outline color##esp_enemy_arrow"), enemy_esp.colors.offscreen_arrow_outline);
					}
					end_child;
				}
			}
			break;
			case 1:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("General    "))
					{
						checkbox(CXOR("Glow##local_player"), &g_cfg.visuals.local_glow);

						if (g_cfg.visuals.local_glow)
							color_picker(CXOR("Glow color##local_esp_glow"), g_cfg.visuals.local_glow_color);

						slider_int(CXOR("Attachments blend"), &g_cfg.misc.attachments_amt, 0, 100, CXOR("%d%%"));
					}
					end_child;

					begin_child(CXOR("Crosshair    "))
					{
						checkbox(CXOR("Penetration crosshair"), &g_cfg.misc.pen_xhair);
						checkbox(CXOR("Force crosshair on sniper"), &g_cfg.misc.snip_crosshair);
						checkbox(CXOR("Fix zoom sensitivity"), &g_cfg.misc.fix_sensitivity);
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("Chams    "))
					{
						static int chams_local_model = 0;

						combo(CXOR("Type##local"), &chams_local_model, local_models, IM_ARRAYSIZE(local_models));

						auto& local_config = g_cfg.visuals.chams[chams_local_model + 5];

						checkbox(CXOR("Enable##local"), &local_config.enable);

						combo(CXOR("Material##local"), &local_config.material, vis_chams_type, IM_ARRAYSIZE(vis_chams_type));
						color_picker(CXOR("Material color##local"), local_config.main_color);
						if (local_config.material == 4)
						{
							slider_int(CXOR("Fill##local"), &local_config.glow_fill, 0, 100, CXOR("%d%%"));
							color_picker(CXOR("Glow color##local"), local_config.glow_color);
						}

						if (local_config.material == 6)
							input_text(CXOR("Sprite path"), local_config.chams_sprite, 128);
					}
					end_child;
				}
			}
			break;
			case 2:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("General     "))
					{
						auto& weapon_esp = g_cfg.visuals.esp[esp_weapon];
						checkbox(CXOR("Enable##esp_weapon"), &weapon_esp.enable);
						multi_combo(CXOR("Elements##esp_weapon"), weapon_esp.elements,
							{
							  XOR("Box"),
							  XOR("Icon"),
							  XOR("Name"),
							  XOR("Ammo"),
							  XOR("Inferno"),
							  XOR("Smoke"),
							  XOR("Glow"),
							});

						if (weapon_esp.elements & 1)
							color_picker(CXOR("Box color##esp_weapon"), weapon_esp.colors.box);

						if (weapon_esp.elements & 2 || weapon_esp.elements & 4)
							color_picker(CXOR("Name color##esp_weapon"), weapon_esp.colors.name);

						if (weapon_esp.elements & 8)
							color_picker(CXOR("Ammo color##esp_weapon"), weapon_esp.colors.ammo_bar);

						if (weapon_esp.elements & 16)
							color_picker(CXOR("Inferno color##esp_weapon"), weapon_esp.colors.molotov_range);

						if (weapon_esp.elements & 32)
							color_picker(CXOR("Smoke color##esp_weapon"), weapon_esp.colors.smoke_range);

						if (weapon_esp.elements & 64)
							color_picker(CXOR("Glow color##esp_weapon"), weapon_esp.colors.glow);
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("Predictions"))
					{
						checkbox(CXOR("Projectile prediction"), &g_cfg.visuals.grenade_predict);
						if (g_cfg.visuals.grenade_predict)
							color_picker(CXOR("Prediction color"), g_cfg.visuals.predict_clr);

						checkbox(CXOR("Projectile warning"), &g_cfg.visuals.grenade_warning);
						checkbox(CXOR("Warning line"), &g_cfg.visuals.grenade_warning_line);
						color_picker(CXOR("Warning color"), g_cfg.visuals.warning_clr);
					}
					end_child;

					auto& weapon_esp = g_cfg.visuals.esp[esp_weapon];
					begin_child(CXOR("Prediction OOF "));
					{
						checkbox(CXOR("Enable##esp_pred_arrow"), &weapon_esp.nade_offscreen);
						slider_int(CXOR("Distance  "), &weapon_esp.offscreen_dist, 1, 450, CXOR("%d ft"));
						slider_int(CXOR("Size  "), &weapon_esp.offscreen_size, 5, 25, CXOR("%d px"));
						color_picker(CXOR("Color##esp_pred_arrow"), weapon_esp.colors.offscreen_arrow);
						color_picker(CXOR("Outline color##esp_pred_arrow"), weapon_esp.colors.offscreen_arrow_outline);
					}
					end_child;
				}
			}
			break;
			case 3:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("Sky & props"))
					{
						combo(CXOR("Skybox"), &g_cfg.misc.skybox, skyboxes, IM_ARRAYSIZE(skyboxes));
						if (g_cfg.misc.skybox == 23)
							input_text(CXOR("Sky name"), g_cfg.misc.skybox_name, 128);

						slider_int(CXOR("Props alpha"), &g_cfg.misc.prop_alpha, 0, 100, CXOR("%d%%"));
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("World modulation"))
					{
						multi_combo(CXOR("Modifiers"), g_cfg.misc.world_modulation,
							{
								XOR("Night mode"),
								XOR("Sunset mode"),
								XOR("Fullbright")
							});

						if (g_cfg.misc.world_modulation & 1)
						{
							color_picker(CXOR("World color"), g_cfg.misc.world_clr[world], no_alpha_picker);
							color_picker(CXOR("Props color"), g_cfg.misc.world_clr[props], no_alpha_picker);
							color_picker(CXOR("Skybox color"), g_cfg.misc.world_clr[sky], no_alpha_picker);
						}

						if (g_cfg.misc.world_modulation & 2)
						{
							slider_int(CXOR("Sun pitch"), &g_cfg.misc.sunset_angle.x, -180, 180, CXOR("%d degrees"));
							slider_int(CXOR("Sun yaw"), &g_cfg.misc.sunset_angle.y, -180, 180, CXOR("%d degrees"));
							slider_int(CXOR("Sun roll"), &g_cfg.misc.sunset_angle.z, -180, 180, CXOR("%d degrees"));
						}
					}
					end_child;
				}
			}
			break;
			case 4:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("Screen effects"))
					{
						multi_combo(CXOR("Removals"), g_cfg.misc.removals,
							{
								XOR("Scope"),
								XOR("Visual recoil"),
								XOR("Post processing"),
								XOR("Smoke"),
								XOR("Flash"),
								XOR("Fog"),
								XOR("Shadows"),
							});
					}
					end_child;

					begin_child(CXOR("World bloom"))
					{
						checkbox(CXOR("Customize bloom"), &g_cfg.misc.custom_bloom);
						if (g_cfg.misc.custom_bloom)
						{
							slider_int(CXOR("Bloom scale"), &g_cfg.misc.bloom_scale, 0, 5);
							slider_int(CXOR("Exposure min"), &g_cfg.misc.exposure_min, 1, 10);
							slider_int(CXOR("Exposure max"), &g_cfg.misc.exposure_max, 1, 10);
						}
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("World fog"))
					{
						checkbox(CXOR("Customize fog"), &g_cfg.misc.custom_fog);
						if (g_cfg.misc.custom_fog)
						{
							slider_int(CXOR("Fog start"), &g_cfg.misc.fog_start, 0, 10000);
							slider_int(CXOR("Fog end"), &g_cfg.misc.fog_end, 0, 10000);
							slider_int(CXOR("Density"), &g_cfg.misc.fog_density, 0, 100, CXOR("%d%%"));
							color_picker(CXOR("Fog color"), g_cfg.misc.world_clr[fog], no_alpha_picker);
						}
					}
					end_child;
				}
			}
			break;
			case 5:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("World camera"))
					{
						key_bind(CXOR("Thirdperson"), g_cfg.binds[tp_b]);
						slider_int(CXOR("Distance"), &g_cfg.misc.thirdperson_dist, 10, 300);
						checkbox(CXOR("While dead"), &g_cfg.misc.thirdperson_dead);
					}
					end_child;

					begin_child(CXOR("Viewmodel  "))
					{
						slider_int(CXOR("Viewmodel FOV"), &g_cfg.misc.fovs[arms], 0, 30);
						checkbox(CXOR("Show viewmodel in scope"), &g_cfg.misc.viewmodel_scope);
						slider_int(CXOR("Offset X"), &g_cfg.misc.viewmodel_pos[0], -20, 20);
						slider_int(CXOR("Offset Y"), &g_cfg.misc.viewmodel_pos[1], -20, 20);
						slider_int(CXOR("Offset Z"), &g_cfg.misc.viewmodel_pos[2], -20, 20);
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("World view"))
					{
						slider_int(CXOR("Aspect ratio"), &g_cfg.misc.aspect_ratio, 0, 200);
						slider_int(CXOR("FOV"), &g_cfg.misc.fovs[world], 0, 60);
						slider_int(CXOR("Zoom FOV"), &g_cfg.misc.fovs[zoom], 0, 100, CXOR("%d%%"));
						checkbox(CXOR("Skip second zoom"), &g_cfg.misc.skip_second_zoom);
					}
					end_child;
				}
			}
			break;
			}
			ImGui::EndChild(false);

			ImGui::PopClipRect();
			draw_list->PopClipRect();
		}
		break;
		case 4:
		{
			static int selector2 = 0;
			draw_sub_tabs(selector2, misc_tabs);

			static int prev_sub_sel2 = 0;
			if (selector2 != prev_sub_sel2)
			{
				for (auto& a : item_animations)
				{
					a.second.reset();
					subtab_alpha2 = 0.f;
				}

				prev_sub_sel2 = selector2;
			}

			create_animation(subtab_alpha2, g_cfg.misc.menu && selector2 == prev_sub_sel2, 0.2f, skip_disable | lerp_animation);
			alpha = subtab_alpha2 * tab_alpha;

			ImGui::SetCursorPosX(0);
			ImGui::BeginChild(CXOR("subtab_misc"), {}, false);
			switch (selector2)
			{
			case 0:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("Main"))
					{
						checkbox(CXOR("Auto jump"), &g_cfg.misc.auto_jump);

						checkbox(CXOR("Auto strafe"), &g_cfg.misc.auto_strafe);
						slider_int(CXOR("Turn smooth"), &g_cfg.misc.strafe_smooth, 0, 100, CXOR("%d%%"));

						key_bind(CXOR("Edge jump"), g_cfg.binds[ej_b]);

						checkbox(CXOR("Fast stop"), &g_cfg.misc.fast_stop);
						checkbox(CXOR("Slide walk"), &g_cfg.misc.slide_walk);
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("Helpers"))
					{
						key_bind(CXOR("Auto peek"), g_cfg.binds[ap_b]);
						color_picker(CXOR("Start color##peek"), g_cfg.misc.autopeek_clr);
						color_picker(CXOR("Move color##peek"), g_cfg.misc.autopeek_clr_back);
						checkbox(CXOR("Return on key release"), &g_cfg.misc.retrack_peek);
					}
					end_child;
				}
			}
			break;
			case 1:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("Tracers"))
					{
						checkbox(CXOR("Bullet impacts"), &g_cfg.misc.impacts);
						color_picker(CXOR("Server color"), g_cfg.misc.server_clr);
						color_picker(CXOR("Client color"), g_cfg.misc.client_clr);
						slider_int(CXOR("Size"), &g_cfg.misc.impact_size, 5, 15);

						multi_combo(CXOR("Bullet tracers"), g_cfg.misc.tracers, { XOR("Enemy"), XOR("Team"), XOR("Local") });

						combo(CXOR("Tracer type"), &g_cfg.misc.tracer_type, tracers, IM_ARRAYSIZE(tracers));

						if (g_cfg.misc.tracers & 1)
							color_picker(CXOR("Enemy tracer"), g_cfg.misc.trace_clr[0]);

						if (g_cfg.misc.tracers & 2)
							color_picker(CXOR("Team tracer"), g_cfg.misc.trace_clr[1]);

						if (g_cfg.misc.tracers & 4)
							color_picker(CXOR("Local tracer"), g_cfg.misc.trace_clr[2]);
					}
					end_child;

					begin_child(CXOR("Event log"))
					{
						checkbox(CXOR("Enable##eventlog"), &g_cfg.visuals.eventlog.enable);
						multi_combo(CXOR("Type##eventlog"), g_cfg.visuals.eventlog.logs, { XOR("Hits"), XOR("Misses"), XOR("Debug"), XOR("Purchases"), XOR("Bomb plant") });
						checkbox(CXOR("Filter console##eventlog"), &g_cfg.visuals.eventlog.filter_console);
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("Hit indicators"))
					{
						multi_combo(CXOR("Hitmarker"), g_cfg.misc.hitmarker, { XOR("On enemy"), XOR("On screen") });
						color_picker(CXOR("Hitmarker color"), g_cfg.misc.hitmarker_clr);

						combo(CXOR("Hitsound"), &g_cfg.misc.sound, hitsound, IM_ARRAYSIZE(hitsound));
						slider_int(CXOR("Volume"), &g_cfg.misc.sound_volume, 0, 100, CXOR("%d%%"));

						if (g_cfg.misc.sound == 2)
							input_text(CXOR("Sound name"), g_cfg.misc.sound_name, 128);

						checkbox(CXOR("Damage indicator"), &g_cfg.misc.damage);
						color_picker(CXOR("Indicator color"), g_cfg.misc.damage_clr);
					}
					end_child;

					begin_child(CXOR("Buy bot"))
					{
						checkbox(CXOR("Enable##buybot"), &g_cfg.misc.buybot.enable);
						combo(CXOR("Primary weapon##buybot"), &g_cfg.misc.buybot.main_weapon, buybot_main, IM_ARRAYSIZE(buybot_main));
						combo(CXOR("Secondary weapon##buybot"), &g_cfg.misc.buybot.second_weapon, buybot_second, IM_ARRAYSIZE(buybot_second));

						multi_combo(CXOR("Items##buybot"), g_cfg.misc.buybot.other_items,
							{ XOR("Head armor"), XOR("Armor"), XOR("HE grenade"), XOR("Molotov"), XOR("Smoke"), XOR("Taser"), XOR("Defuse kit") });
					}
					end_child;
				}
			}
			break;
			case 2:
			{
				ImGui::Columns(2, 0, false);
				ImGui::SetColumnOffset(1, 270);
				{
					begin_child(CXOR("Hud"))
					{
						checkbox(CXOR("Force engine radar"), &g_cfg.misc.force_radar);
						checkbox(CXOR("Preserve killfeed"), &g_cfg.misc.preverse_killfeed);
						checkbox(CXOR("Unlock inventory"), &g_cfg.misc.unlock_inventory);
						checkbox(CXOR("Remove server ads"), &g_cfg.misc.remove_ads);
					}
					end_child;

					begin_child(CXOR("Menu"))
					{
						color_picker(CXOR("Accent color"), g_cfg.misc.ui_color);
						multi_combo(CXOR("Indicators##menu"), g_cfg.misc.menu_indicators, { XOR("Keybind list"), XOR("Bomb"), XOR("Watermark"), XOR("Spectators") });
					}
					end_child;
				}
				ImGui::NextColumn();
				{
					begin_child(CXOR("Changers"))
					{
						combo(CXOR("Ragdoll gravity"), &g_cfg.misc.ragdoll_gravity, ragdoll_gravities, IM_ARRAYSIZE(ragdoll_gravities));
						checkbox(CXOR("Clantag changer"), &g_cfg.misc.clantag);
					}
					end_child;

					begin_child(CXOR("Bypass"))
					{
						checkbox(CXOR("Unlock hidden cvars"), &g_cfg.misc.unlock_hidden_cvars);
						checkbox(CXOR("Bypass sv_pure"), &g_cfg.misc.bypass_sv_pure);

#if ALPHA || _DEBUG
						checkbox(CXOR("Force crash"), &g_cfg.misc.force_crash);
#endif
					}
					end_child;
				}
			}
			break;
			}
			ImGui::EndChild(false);

			ImGui::PopClipRect();
			draw_list->PopClipRect();
		}
		break;
		case 5:
		{
			ImGui::Columns(2, 0, false);
			ImGui::SetColumnOffset(1, 270);
			{
				begin_child(CXOR("Choose your weapon     "))
				{
					combo(CXOR("Painting up...##skins"), &g_cfg.skins.group_type, skin_weapon_configs, IM_ARRAYSIZE(skin_weapon_configs));

					if (g_cfg.skins.group_type == weapon_cfg_knife)
						combo(CXOR("Model  "), &g_cfg.skins.skin_weapon[g_cfg.skins.group_type].knife_model, knife_models, IM_ARRAYSIZE(knife_models));
				}
				end_child;

				begin_child(CXOR("Gloves"))
				{
					combo(CXOR("Glove model"), &g_cfg.skins.model_glove, glove_models, IM_ARRAYSIZE(glove_models));
					combo(CXOR("Glove skin"), &g_cfg.skins.glove_skin, glove_skins, IM_ARRAYSIZE(glove_skins));
				}
				end_child;

#ifndef LEGACY
				begin_child(CXOR("Player"))
				{
					combo(CXOR("Mask"), &g_cfg.skins.masks, masks, IM_ARRAYSIZE(masks));

					combo(CXOR("Agent CT"), &g_cfg.skins.model_ct, agents, IM_ARRAYSIZE(agents));

					if (g_cfg.skins.model_ct == 76)
						input_text(CXOR("Model path##agent_ct"), g_cfg.skins.custom_model_ct, 128);

					combo(CXOR("Agent T"), &g_cfg.skins.model_t, agents, IM_ARRAYSIZE(agents));

					if (g_cfg.skins.model_t == 76)
						input_text(CXOR("Model path##agent_t"), g_cfg.skins.custom_model_t, 128);
				}
				end_child;
#endif
			}
			ImGui::NextColumn();
			{
				if (skin_names.size() < skin_changer::paint_kits.size())
				{
					for (auto& s : skin_changer::paint_kits)
						skin_names.emplace_back(s.first);
				}

				auto& config = g_cfg.skins.skin_weapon[g_cfg.skins.group_type];
				auto skin_list = skin_changer::paint_kits.size() <= 0 ? empty_list : skin_names;
				std::string skin_list_child_name = XOR("Skins | ") + std::to_string(skin_list.size()) + XOR(" total");

				begin_child(skin_list_child_name.c_str())
				{
					checkbox(CXOR("Enable"), &config.enable);

					input_text(CXOR("Skins search"), config.skins_search, 256);
					listbox(CXOR("skinlist_____"), &config.skin, skin_list, 17, config.skins_search);
				}
				end_child;
			}
		}
		break;
		case 6:
		{
			static int config_select = -1;
			static char config_name[256]{};

			bool wrong_config = config_select == -1 || config_list.size() <= 0 || config_select > config_list.size() - 1;

			int cfg_flags = 0;
			if (wrong_config)
				cfg_flags |= button_wait;

			ImGui::Columns(2, 0, false);
			ImGui::SetColumnOffset(1, 270);
			{
				begin_child(CXOR("General   "))
				{
					input_text(CXOR("Config name"), config_name, 256);

					int len = std::strlen(config_name);

					button(
						CXOR("Create"),
						[]()
						{
							config::create(config_name);
							refresh_configs();
						},
						len <= 0 ? button_wait : 0);

					button(CXOR("Refresh"), []() { refresh_configs(); });

					button(CXOR("Open configs folder"),
						[]()
						{
							std::string folder = CXOR("airflow\\");
							ShellExecuteA(NULL, NULL, folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
						});
				}
				end_child;

				std::string config_child_name = XOR("Config: ") + (wrong_config ? XOR("None") : config_list[config_select]);

				begin_child(config_child_name.c_str())
				{
					button(
						CXOR("Load"),
						[]()
						{
							config::load(config_list[config_select]);
							if (!HACKS->loading_config)
								HACKS->loading_config = true;
						},
						cfg_flags);

					button(
						CXOR("Save"), []() { config::save(config_list[config_select]); }, cfg_flags);

					button(
						CXOR("Reset"),
						[]()
						{
							g_cfg = configs_t();
							config::save(config_list[config_select]);
						},
						cfg_flags);

					int delete_flags = button_danger;
					if (wrong_config)
						delete_flags |= button_wait;

					button(
						CXOR("Delete"),
						[]()
						{
							config::erase(config_list[config_select]);
							refresh_configs();
						},
						delete_flags);
				}
				end_child;
			}
			ImGui::NextColumn();
			{
				static char config_search[256];

				auto list = config_list.size() <= 0 ? empty_list : config_list;
				std::string config_list_child_name = XOR("Configs | ") + std::to_string(list.size()) + XOR(" total");

				begin_child(config_list_child_name.c_str())
				{
					input_text(CXOR("Config search"), config_search, 256);
					listbox(CXOR("configlist___"), &config_select, list, 19, config_search);
				}
				end_child;
			}
		}
		break;
		}
	}
	ImGui::EndChild(false);

	alpha = old_alpha;
	ImGui::PopClipRect();
	ImGui::SetCursorPos(prev_pos);
}