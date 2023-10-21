#include "globals.hpp"

namespace main_utils
{
	rage_weapon_t zeus_config = { true, false, false, false, 60, 101, 101, -1, -1, 0, 0, false, false, chest | stomach | pelvis, 0, 0, {} };

	std::string get_executable_file_path()
	{
		char result[MAX_PATH]{};
		return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
	}

	INLINE bool file_have_extension(const char* path, const char* extension)
	{
		auto path_exist = std::filesystem::exists(path);
		if (path_exist)
		{
			std::filesystem::path model_path(path);
			if (!model_path.has_extension())
				return false;

			const auto& str = model_path.extension().string();
			return std::strstr(str.c_str(), extension);
		}
	}

	INLINE bool file_exist(const char* path)
	{
		return std::filesystem::exists(path);
	}

	int get_legit_tab(c_base_combat_weapon* temp_weapon)
	{
		auto weapon = temp_weapon ? temp_weapon : HACKS->weapon;
		if (!weapon)
			return 0;

		if (weapon->is_knife())
			return weapon_cfg_knife;

		auto idx = weapon->item_definition_index();

		switch (idx)
		{
		case WEAPON_DEAGLE:
			return weapon_cfg_deagle;
			break;
		case WEAPON_ELITE:
			return weapon_cfg_duals;
			break;
		case WEAPON_FIVESEVEN:
			return weapon_cfg_fiveseven;
			break;
		case WEAPON_GLOCK:
			return weapon_cfg_glock;
			break;
		case WEAPON_AK47:
			return weapon_cfg_ak47;
			break;
		case WEAPON_AUG:
			return weapon_cfg_aug;
			break;
		case WEAPON_AWP:
			return weapon_cfg_awp;
			break;
		case WEAPON_FAMAS:
			return weapon_cfg_famas;
			break;
		case WEAPON_G3SG1:
			return weapon_cfg_g3sg1;
			break;
		case WEAPON_GALILAR:
			return weapon_cfg_galil;
			break;
		case WEAPON_M249:
			return weapon_cfg_m249;
			break;
		case WEAPON_M4A1:
			return weapon_cfg_m4a1;
			break;
		case WEAPON_MAC10:
			return weapon_cfg_mac10;
			break;
		case WEAPON_P90:
			return weapon_cfg_p90;
			break;
		case WEAPON_MP5SD:
			return weapon_cfg_mp5sd;
			break;
		case WEAPON_UMP45:
			return weapon_cfg_ump45;
			break;
		case WEAPON_XM1014:
			return weapon_cfg_xm1014;
			break;
		case WEAPON_BIZON:
			return weapon_cfg_bizon;
			break;
		case WEAPON_MAG7:
			return weapon_cfg_mag7;
			break;
		case WEAPON_NEGEV:
			return weapon_cfg_negev;
			break;
		case WEAPON_SAWEDOFF:
			return weapon_cfg_sawedoff;
			break;
		case WEAPON_TEC9:
			return weapon_cfg_tec9;
			break;
		case WEAPON_HKP2000:
			return weapon_cfg_p2000;
			break;
		case WEAPON_MP7:
			return weapon_cfg_mp7;
			break;
		case WEAPON_MP9:
			return weapon_cfg_mp9;
			break;
		case WEAPON_NOVA:
			return weapon_cfg_nova;
			break;
		case WEAPON_P250:
			return weapon_cfg_p250;
			break;
		case WEAPON_SCAR20:
			return weapon_cfg_scar20;
			break;
		case WEAPON_SG556:
			return weapon_cfg_sg556;
			break;
		case WEAPON_SSG08:
			return weapon_cfg_ssg08;
			break;
		case WEAPON_M4A1_SILENCER:
			return weapon_cfg_m4a1s;
			break;
		case WEAPON_USP_SILENCER:
			return weapon_cfg_usps;
			break;
		case WEAPON_CZ75A:
			return weapon_cfg_cz75;
			break;
		case WEAPON_REVOLVER:
			return weapon_cfg_revolver;
			break;
		default:
			return 0;
			break;
		}
	}

	skin_weapon_t get_skin_weapon_config()
	{
		if (!HACKS->local || !HACKS->local->is_alive())
			return {};

		if (!HACKS->weapon)
			return {};

		int tab = get_legit_tab();
		return g_cfg.skins.skin_weapon[tab];
	}

	legit_weapon_t get_legit_weapon_config()
	{
		if (!HACKS->local || !HACKS->local->is_alive())
			return {};

		if (!HACKS->weapon)
			return {};

		int tab = std::clamp(get_legit_tab(), 0, (int)weapon_cfg_revolver);
		return g_cfg.legit.legit_weapon[tab];
	}

	rage_weapon_t get_weapon_config()
	{
		if (!HACKS->local || !HACKS->local->is_alive())
			return {};

		if (!HACKS->weapon)
			return {};

		auto is_sniper = HACKS->weapon->is_auto_sniper();
		auto is_heavy = HACKS->weapon->is_heavy_pistols();
		auto is_pistols = HACKS->weapon->is_pistols();
		auto is_ssg = HACKS->weapon->item_definition_index() == WEAPON_SSG08;
		auto is_awp = HACKS->weapon->item_definition_index() == WEAPON_AWP;
		auto is_taser = HACKS->weapon->is_taser();

		bool valid_weapons[]{
			false,
			is_sniper,
			is_heavy,
			is_pistols,
			is_ssg,
			is_awp,
		};

		if (HACKS->weapon->is_taser())
			return zeus_config;

		for (int i = 1; i < weapon_max; i++) {
			auto& current_cfg = g_cfg.rage.weapon[i];
			if (valid_weapons[i] && current_cfg.enable)
				return current_cfg;
		}

		return g_cfg.rage.weapon[global];
	}

	std::string hitgroup_to_string(int hitgroup)
	{
		switch (hitgroup)
		{
		case HITGROUP_GENERIC:
			return XOR("generic");
			break;
		case HITGROUP_HEAD:
			return XOR("head");
			break;
		case HITGROUP_CHEST:
			return XOR("chest");
			break;
		case HITGROUP_STOMACH:
			return XOR("body");
			break;
		case HITGROUP_LEFTARM:
			return XOR("left arm");
			break;
		case HITGROUP_RIGHTARM:
			return XOR("right arm");
			break;
		case HITGROUP_LEFTLEG:
			return XOR("left leg");
			break;
		case HITGROUP_RIGHTLEG:
			return XOR("right leg");
			break;
		case HITGROUP_GEAR:
			return XOR("gear");
			break;
		case HITGROUP_NECK:
			return XOR("neck");
			break;
		default:
			return XOR("unknown");
		}
	}

	std::string hitbox_to_string(int id)
	{
		switch (id)
		{
		case HITBOX_HEAD:
			return XOR("head");
		case HITBOX_NECK:
			return XOR("neck");
		case HITBOX_PELVIS:
			return XOR("pelvis");
		case HITBOX_STOMACH:
			return XOR("stomach");
		case HITBOX_THORAX:
			return XOR("lower chest");
		case HITBOX_CHEST:
			return XOR("chest");
		case HITBOX_UPPER_CHEST:
			return XOR("upper chest");
		case HITBOX_LEFT_THIGH:
			return XOR("left thigh");
		case HITBOX_RIGHT_THIGH:
			return XOR("right thigh");
		case HITBOX_LEFT_CALF:
			return XOR("left calf");
		case HITBOX_RIGHT_CALF:
			return XOR("right calf");
		case HITBOX_LEFT_FOOT:
			return XOR("left foot");
		case HITBOX_RIGHT_FOOT:
			return XOR("right foot");
		case HITBOX_LEFT_HAND:
			return XOR("left hand");
		case HITBOX_RIGHT_HAND:
			return XOR("right hand");
		case HITBOX_LEFT_UPPER_ARM:
			return XOR("left upper arm");
		case HITBOX_LEFT_FOREARM:
			return XOR("left forearm");
		case HITBOX_RIGHT_UPPER_ARM:
			return XOR("right upper arm");
		case HITBOX_RIGHT_FOREARM:
			return XOR("right forearm");
		}

		return "";
	}

	/*void draw_hitbox(c_cs_player* player, matrix3x4_t* bones, int idx, int idx2, bool dur)
	{
		studio_hdr_t* studio_model = interfaces::model_info->get_studio_model(player->get_model());
		if (!studio_model)
			return;

		mstudio_hitbox_set_t* hitbox_set = studio_model->get_hitbox_set(0);
		if (!hitbox_set)
			return;

		for (int i = 0; i < hitbox_set->hitboxes; i++)
		{
			mstudio_bbox_t* hitbox = hitbox_set->get_hitbox(i);
			if (!hitbox)
				continue;

			vector3d vMin, vMax;
			math::vector_transform(hitbox->bbmin, bones[hitbox->bone], vMin);
			math::vector_transform(hitbox->bbmax, bones[hitbox->bone], vMax);

			if (hitbox->radius != -1.f)
				interfaces::debug_overlay->add_capsule_overlay(vMin, vMax, hitbox->radius, 255, 255 * idx, 255 * idx2, 150, dur ? interfaces::global_vars->interval_per_tick * 2.f : 5.f, 0, 1);
		}
	*/
}