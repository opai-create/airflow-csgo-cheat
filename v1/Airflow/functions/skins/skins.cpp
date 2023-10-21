#include <algorithm>
#include <fstream>

#include "skins.h"
#include "../ragebot/rage_tools.h"
#include "../../base/tools/memory/memory.h"
#include <sys/stat.h>

namespace skin_changer
{
	bool call_force_update = false;
	bool in_force_update = false;
	bool update_hud = false;
	float last_update_time = 0.f;

	constexpr auto mask_flags = 0x10000;
	constexpr auto max_knifes = 20;
	constexpr auto max_gloves = 7;
	const char* default_mask = ("models/player/holiday/facemasks/facemask_battlemask.mdl");

	std::unordered_map<std::string, int> weapon_indexes{};

	std::array<std::string, max_knifes - 1> knife_models
	{
		xor_str("models/weapons/v_knife_bayonet.mdl"),
			xor_str("models/weapons/v_knife_css.mdl"),
			xor_str("models/weapons/v_knife_skeleton.mdl"),
			xor_str("models/weapons/v_knife_outdoor.mdl"),
			xor_str("models/weapons/v_knife_cord.mdl"),
			xor_str("models/weapons/v_knife_canis.mdl"),
			xor_str("models/weapons/v_knife_flip.mdl"),
			xor_str("models/weapons/v_knife_gut.mdl"),
			xor_str("models/weapons/v_knife_karam.mdl"),
			xor_str("models/weapons/v_knife_m9_bay.mdl"),
			xor_str("models/weapons/v_knife_tactical.mdl"),
			xor_str("models/weapons/v_knife_falchion_advanced.mdl"),
			xor_str("models/weapons/v_knife_survival_bowie.mdl"),
			xor_str("models/weapons/v_knife_butterfly.mdl"),
			xor_str("models/weapons/v_knife_push.mdl"),
			xor_str("models/weapons/v_knife_ursus.mdl"),
			xor_str("models/weapons/v_knife_gypsy_jackknife.mdl"),
			xor_str("models/weapons/v_knife_stiletto.mdl"),
			xor_str("models/weapons/v_knife_widowmaker.mdl"),
	};

	std::array<std::string, max_knifes - 1> world_knife_models
	{
		xor_str("models/weapons/w_knife_bayonet.mdl"),
			xor_str("models/weapons/w_knife_css.mdl"),
			xor_str("models/weapons/w_knife_skeleton.mdl"),
			xor_str("models/weapons/w_knife_outdoor.mdl"),
			xor_str("models/weapons/w_knife_cord.mdl"),
			xor_str("models/weapons/w_knife_canis.mdl"),
			xor_str("models/weapons/w_knife_flip.mdl"),
			xor_str("models/weapons/w_knife_gut.mdl"),
			xor_str("models/weapons/w_knife_karam.mdl"),
			xor_str("models/weapons/w_knife_m9_bay.mdl"),
			xor_str("models/weapons/w_knife_tactical.mdl"),
			xor_str("models/weapons/w_knife_falchion_advanced.mdl"),
			xor_str("models/weapons/w_knife_survival_bowie.mdl"),
			xor_str("models/weapons/w_knife_butterfly.mdl"),
			xor_str("models/weapons/w_knife_push.mdl"),
			xor_str("models/weapons/w_knife_ursus.mdl"),
			xor_str("models/weapons/w_knife_gypsy_jackknife.mdl"),
			xor_str("models/weapons/w_knife_stiletto.mdl"),
			xor_str("models/weapons/w_knife_widowmaker.mdl"),
	};

	std::array<std::string, max_gloves> gloves
	{
		xor_str("models/weapons/w_models/arms/w_glove_bloodhound.mdl"),
			xor_str("models/weapons/w_models/arms/w_glove_sporty.mdl"),
			xor_str("models/weapons/w_models/arms/w_glove_slick.mdl"),
			xor_str("models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl"),
			xor_str("models/weapons/w_models/arms/w_glove_motorcycle.mdl"),
			xor_str("models/weapons/w_models/arms/w_glove_specialist.mdl"),
			xor_str("models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl"),
	};

	struct knife_id_t
	{
		short index{};
		std::string name{};
	};

	std::array<knife_id_t, max_knifes> knifes
	{
		knife_id_t{ weapon_none, xor_str("def") },
			knife_id_t{ weapon_knife_bayonet, xor_str("bayonet") },
			knife_id_t{ weapon_knife_css, xor_str("css") },
			knife_id_t{ weapon_knife_skeleton, xor_str("skeleton") },
			knife_id_t{ weapon_knife_outdoor, xor_str("outdoor") },
			knife_id_t{ weapon_knife_cord, xor_str("cord") },
			knife_id_t{ weapon_knife_canis, xor_str("canis") },
			knife_id_t{ weapon_knife_flip, xor_str("flip") },
			knife_id_t{ weapon_knife_gut, xor_str("gut") },
			knife_id_t{ weapon_knife_karambit, xor_str("karambit") },
			knife_id_t{ weapon_knife_m9_bayonet, xor_str("m9 bayonet") },
			knife_id_t{ weapon_knife_tactical, xor_str("tactical") },
			knife_id_t{ weapon_knife_falchion, xor_str("falchion") },
			knife_id_t{ weapon_knife_survival_bowie, xor_str("bowie") },
			knife_id_t{ weapon_knife_butterfly, xor_str("butterfly") },
			knife_id_t{ weapon_knife_push, xor_str("push") },
			knife_id_t{ weapon_knife_ursus, xor_str("ursus") },
			knife_id_t{ weapon_knife_gypsy_jackknife, xor_str("gypsy") },
			knife_id_t{ weapon_knife_stiletto, xor_str("stiletto") },
			knife_id_t{ weapon_knife_widowmaker, xor_str("widowmaker") }
	};

	enum knife_sequcence_t : int
	{
		sequence_default_draw = 0,
		sequence_default_idle1 = 1,
		sequence_default_idle2 = 2,
		sequence_default_light_miss1 = 3,
		sequence_default_light_miss2 = 4,
		sequence_default_heavy_miss1 = 9,
		sequence_default_heavy_hit1 = 10,
		sequence_default_heavy_backstab = 11,
		sequence_default_lookat01 = 12,

		sequence_butterfly_draw = 0,
		sequence_butterfly_draw2 = 1,
		sequence_butterfly_lookat01 = 13,
		sequence_butterfly_lookat03 = 15,

		sequence_falchion_idle1 = 1,
		sequence_falchion_heavy_miss1 = 8,
		sequence_falchion_heavy_miss1_noflip = 9,
		sequence_falchion_lookat01 = 12,
		sequence_falchion_lookat02 = 13,

		sequence_css_lookat01 = 14,
		sequence_css_lookat02 = 15,

		sequence_daggers_idle1 = 1,
		sequence_daggers_light_miss1 = 2,
		sequence_daggers_light_miss5 = 6,
		sequence_daggers_heavy_miss2 = 11,
		sequence_daggers_heavy_miss1 = 12,

		sequence_bowie_idle1 = 1,
	};

	struct weapon_info
	{
		constexpr weapon_info(const char* model, const char* icon = nullptr, int animindex = -1)
			: model(model), icon(icon), animindex(animindex) {}

		const char* model;
		const char* icon;
		int animindex;
	};

	const weapon_info* get_weapon_info(int defindex)
	{
		const static std::map< int, weapon_info > Info =
		{
			{ weapon_knife, { xor_c("models/weapons/v_knife_default_ct.mdl"), xor_c("knife_default_ct"), 2 } },
			{ weapon_knife_t, { xor_c("models/weapons/v_knife_default_t.mdl"), xor_c("knife_t"), 12 } },
			{ weapon_knife_bayonet, { xor_c("models/weapons/v_knife_bayonet.mdl"), xor_c("bayonet"), 0 } },
			{ weapon_knife_flip, { xor_c("models/weapons/v_knife_flip.mdl"), xor_c("knife_flip"), 4 } },
			{ weapon_knife_gut, { xor_c("models/weapons/v_knife_gut.mdl"), xor_c("knife_gut"), 5 } },
			{ weapon_knife_karambit, { xor_c("models/weapons/v_knife_karam.mdl"), xor_c("knife_karambit"), 7 } },
			{ weapon_knife_m9_bayonet, { xor_c("models/weapons/v_knife_m9_bay.mdl"), xor_c("knife_m9_bayonet"), 8 } },
			{ weapon_knife_tactical, { xor_c("models/weapons/v_knife_tactical.mdl"), xor_c("knife_tactical") } },
			{ weapon_knife_falchion, { xor_c("models/weapons/v_knife_falchion_advanced.mdl"), xor_c("knife_falchion"), 3 } },
			{ weapon_knife_survival_bowie, { xor_c("models/weapons/v_knife_survival_bowie.mdl"), xor_c("knife_survival_bowie"), 11 } },
			{ weapon_knife_butterfly, { xor_c("models/weapons/v_knife_butterfly.mdl"), xor_c("knife_butterfly"), 1 } },
			{ weapon_knife_push, { xor_c("models/weapons/v_knife_push.mdl"), xor_c("knife_push"), 9 } },
			{ weapon_knife_ursus, { xor_c("models/weapons/v_knife_ursus.mdl"), xor_c("knife_ursus"), 13 } },
			{ weapon_knife_gypsy_jackknife, { xor_c("models/weapons/v_knife_gypsy_jackknife.mdl"), xor_c("knife_gypsy_jackknife"), 6 } },
			{ weapon_knife_stiletto, { xor_c("models/weapons/v_knife_stiletto.mdl"), xor_c("knife_stiletto"), 10 } },
			{ weapon_knife_widowmaker, { xor_c("models/weapons/v_knife_widowmaker.mdl"), xor_c("knife_widowmaker"), 14 } },
			{ glove_studded_bloodhound, { xor_c("models/weapons/w_models/arms/w_glove_bloodhound.mdl") } },
			{ glove_t, { xor_c("models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl") } },
			{ glove_ct, { xor_c("models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl") } },
			{ glove_sporty, { xor_c("models/weapons/w_models/arms/w_glove_sporty.mdl") } },
			{ glove_slick, { xor_c("models/weapons/w_models/arms/w_glove_slick.mdl") } },
			{ glove_leather_handwraps, { xor_c("models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl") } },
			{ glove_motorcycle, { xor_c("models/weapons/w_models/arms/w_glove_motorcycle.mdl") } },
			{ glove_specialist, { xor_c("models/weapons/w_models/arms/w_glove_specialist.mdl") } },
			{ glove_studded_hydra, { xor_c("models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl") } },
			{ 521, { xor_c("models/weapons/v_knife_outdoor.mdl"), xor_c("knife_outdoor"), 14 } },
			{ 518, { xor_c("models/weapons/v_knife_canis.mdl"), xor_c("knife_canis"), 14 } },
			{ 517, { xor_c("models/weapons/v_knife_cord.mdl"), xor_c("knife_cord"), 14 } },
			{ 525, { xor_c("models/weapons/v_knife_skeleton.mdl"), xor_c("knife_skeleton"), 14 } },
			{ 503, { xor_c("models/weapons/v_knife_css.mdl"), xor_c("knife_css"), 14 } } };

		const auto entry = Info.find(defindex);
		return entry == end(Info) ? nullptr : &entry->second;
	}

	bool precached_on_round_start = true;

	__forceinline bool should_precache(const char* name)
	{
		if (name == "")
			return false;

		if (name == nullptr)
			return false;

		auto modelprecache = interfaces::network_string_table_container->find_table(xor_c("modelprecache"));
		if (!modelprecache)
			return false;

		auto idx = modelprecache->add_string(false, name);
		if (idx == -1)
			return false;

		return true;
	}

	__forceinline const char* get_mask_model_name()
	{
		static std::vector<const char*> models =
		{
		  ("models\\player\\holiday\\facemasks\\facemask_battlemask.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_hoxton.mdl"),
		  ("models\\player\\holiday\\facemasks\\porcelain_doll.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_skull.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_samurai.mdl"),
		  ("models\\player\\holiday\\facemasks\\evil_clown.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_wolf.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_sheep_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_bunny_gold.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_anaglyph.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_porcelain_doll_kabuki.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_dallas.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_pumpkin.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_sheep_bloody.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_devil_plastic.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_boar.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_chains.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tiki.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_bunny.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_sheep_gold.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_zombie_fortune_plastic.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_chicken.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_skull_gold.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_demo_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_engi_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_heavy_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_medic_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_pyro_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_scout_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_sniper_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_soldier_model.mdl"),
		  ("models\\player\\holiday\\facemasks\\facemask_tf2_spy_model.mdl"),
		  ("models\\props\\holiday_light\\holiday_light.mdl"),
		};

		if (g_cfg.skins.masks == 0 || g_cfg.skins.masks >= models.size())
			return "";

		return models[g_cfg.skins.masks - 1];
	}

	__forceinline std::string get_model_name(int team)
	{
		static const std::vector<std::string> models =
		{
			xor_c("models\\player\\custom_player\\legacy\\tm_jumpsuit_varianta.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jumpsuit_variantb.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jumpsuit_variantc.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_diver_varianta.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_diver_variantb.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_diver_variantc.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_fbi_varianth.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_fbi_variantf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_fbi_variantb.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_fbi_variantg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gendarmerie_varianta.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variantb.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variantc.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variantd.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variante.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_sas_variantg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_sas_variantf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_variante.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_variantg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_varianti.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_variantj.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_variantk.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_variantl.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_variantm.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_st6_variantn.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_swat_variante.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_swat_variantf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_swat_variantg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_swat_varianth.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_swat_varianti.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_swat_variantj.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_swat_variantk.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varj.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_vari.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varh.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varf5.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varf4.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varf3.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varf2.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varf1.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_professional_varf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_phoenix_varianti.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_phoenix_varianth.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_phoenix_variantg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_phoenix_variantf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_leet_variantj.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_leet_varianti.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_leet_varianth.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_leet_variantg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_leet_variantf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantf2.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jungle_raider_variante.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantd.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantb2.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantb.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_jungle_raider_varianta.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_balkan_varianth.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_balkan_variantj.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_balkan_varianti.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_balkan_variantf.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_balkan_variantg.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_balkan_variantk.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_balkan_variantl.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gign.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gign_variantA.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gign_variantB.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gign_variantC.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\ctm_gign_variantD.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_pirate.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_pirate_variantA.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_pirate_variantB.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_pirate_variantC.mdl"),
			xor_c("models\\player\\custom_player\\legacy\\tm_pirate_variantD.mdl"),
		};

		switch (team)
		{
		case 2:
		{
			if (g_cfg.skins.model_t == 76)
				return g_cfg.skins.custom_model_t;

			return (size_t)(g_cfg.skins.model_t - 1) < models.size() ? models[g_cfg.skins.model_t - 1] : std::string();
		}
		case 3:
		{
			if (g_cfg.skins.model_ct == 76)
				return g_cfg.skins.custom_model_ct;

			return (size_t)(g_cfg.skins.model_ct - 1) < models.size() ? models[g_cfg.skins.model_ct - 1] : std::string();
		}
		default:
			return {};
		}
	};

	// special thakns to infirms1337 for this code
	__forceinline void mask_changer(int stage)
	{
		static auto current_mask = *patterns::mask_ptr.as< char*** >();

		static int old_mask = -1;

		if (!g_ctx.local || stage != frame_render_start && stage != frame_net_update_postdataupdate_start)
			return;

		if (!precached_on_round_start)
			return;

		auto mask = get_mask_model_name();

		if (!should_precache(default_mask) || !should_precache(mask))
			return;

		if (g_cfg.skins.masks > 0)
		{
			g_ctx.local->addon_bits() |= mask_flags;

			if (old_mask != g_cfg.skins.masks)
			{
				*current_mask = (char*)mask;
				func_ptrs::update_addon_models(g_ctx.local, true);
				old_mask = g_cfg.skins.masks;
			}
		}
		else
		{
			if (g_ctx.local->addon_bits() & mask_flags)
				g_ctx.local->addon_bits() &= ~mask_flags;
		}
	}

	inline int get_original_model_idx()
	{
		return interfaces::model_info->get_model_index(xor_c("models/player/custom_player/legacy/ctm_fbi_varianth.mdl"));
	}

	__forceinline void agent_changer(int stage)
	{
		if (!g_ctx.is_alive || stage != frame_net_update_postdataupdate_start && stage != frame_render_end)
			return;

		if (!precached_on_round_start)
			return;

		static std::string old_model_path{};
		static bool reset_model = false;
		static bool set_original_index = false;
		static auto get_viewmodel_config = patterns::viewmodel_arm_cfg.as<std::add_pointer_t<const char** __fastcall(const char*)>>();

		int team = g_ctx.local->team();
		bool set_original = team == 2 && g_cfg.skins.model_t == 0 || team == 3 && g_cfg.skins.model_ct == 0;
		bool use_custom_models = team == 2 && g_cfg.skins.model_t == 76 || team == 3 && g_cfg.skins.model_ct == 76;

		auto original_model = get_original_model_idx();

		if (set_original)
		{
			if (reset_model)
			{
				// i can't get original model index
				// so just force update and game will restore all model shit here
				call_force_update = true;

				reset_model = false;
			}

			return;
		}
		else
			reset_model = true;

		auto model = get_model_name(g_ctx.local->team());
		auto full_model_path = g_ctx.exe_path + model;

		if (old_model_path != full_model_path)
		{
			set_original_index = !main_utils::file_exist(full_model_path.c_str());
			old_model_path = full_model_path;
		}

		if (use_custom_models && set_original_index)
		{
			if (original_model != 0)
				g_ctx.local->set_model_index(original_model);
			else
			{
				// i can't get original model index
				// so just force update and game will restore all model shit here
				call_force_update = true;

				return;
			}

			//printf("file %s doest not exist! \n", full_model_path.c_str());
			return;
		}

		if (stage == frame_net_update_postdataupdate_start)
		{
			if (should_precache(model.c_str()))
			{
				const auto viewmodel_arm_config = get_viewmodel_config(model.c_str());
				if (!viewmodel_arm_config || !should_precache(viewmodel_arm_config[2]) || !should_precache(viewmodel_arm_config[3]))
					return;
			}
		}

		auto idx = interfaces::model_info->get_model_index(model.c_str());
		if (idx == 0)
			return;

		g_ctx.local->set_model_index(idx);
	}

	__forceinline void init_parser()
	{
		for (int i = 0; i <= interfaces::item_schema->paint_kits.last_element; ++i)
		{
			const auto& paint_kit = interfaces::item_schema->paint_kits.memory[i]._value;
			if (paint_kit->id == 9001 || paint_kit->id >= 10000)
				continue;

			auto name = string_convert::to_string(interfaces::localize->find_safe(paint_kit->item_name.buffer + 1));
			if (name == "-")
				continue;

			paint_kits.emplace_back(std::make_pair(name, paint_kit->id));
		}

		std::sort(paint_kits.begin(), paint_kits.end(), [&](const std::pair< std::string, int >& a, const std::pair< std::string, int >& b) { return a.first < b.first; });
	}

	static auto random_sequence(const int low, const int high) -> int
	{
		return rand() % (high - low + 1) + low;
	}

	__forceinline int correct_sequence(const short& index, const int sequence)
	{
		switch (index)
		{
		case weapon_knife_butterfly:
		{
			switch (sequence)
			{
			case sequence_default_draw:
				return random_sequence(sequence_butterfly_draw, sequence_butterfly_draw2);
			case sequence_default_lookat01:
				return random_sequence(sequence_butterfly_lookat01, sequence_butterfly_lookat03);
			default:
				return sequence + 1;
			}
		}
		case weapon_knife_falchion:
		{
			switch (sequence)
			{
			case sequence_default_idle2:
				return sequence_falchion_idle1;
			case sequence_default_heavy_miss1:
				return random_sequence(sequence_falchion_heavy_miss1, sequence_falchion_heavy_miss1_noflip);
			case sequence_default_lookat01:
				return random_sequence(sequence_falchion_lookat01, sequence_falchion_lookat02);
			case sequence_default_draw:
			case sequence_default_idle1:
				return sequence;
			default:
				return sequence - 1;
			}
		}
		case weapon_knife_css:
		{
			switch (sequence)
			{
			case sequence_default_lookat01:
				return random_sequence(sequence_css_lookat01, sequence_css_lookat02);
			default:
				return sequence;
			}
		}
		case weapon_knife_push:
		{
			switch (sequence)
			{
			case sequence_default_idle2:
				return sequence_daggers_idle1;
			case sequence_default_light_miss1:
			case sequence_default_light_miss2:
				return random_sequence(sequence_daggers_light_miss1, sequence_daggers_light_miss5);
			case sequence_default_heavy_miss1:
				return random_sequence(sequence_daggers_heavy_miss2, sequence_daggers_heavy_miss1);
			case sequence_default_heavy_hit1:
			case sequence_default_heavy_backstab:
			case sequence_default_lookat01:
				return sequence + 3;
			case sequence_default_draw:
			case sequence_default_idle1:
				return sequence;
			default:
				return sequence + 2;
			}
		}
		case weapon_knife_survival_bowie:
		{
			switch (sequence)
			{
			case sequence_default_draw:
			case sequence_default_idle1:
				return sequence;
			case sequence_default_idle2:
				return sequence_bowie_idle1;
			default:
				return sequence - 1;
			}
		}
		case weapon_knife_ursus:
		case weapon_knife_cord:
		case weapon_knife_canis:
		case weapon_knife_outdoor:
		case weapon_knife_skeleton:
		{
			switch (sequence)
			{
			case sequence_default_draw:
				return random_sequence(sequence_butterfly_draw, sequence_butterfly_draw2);
			case sequence_default_lookat01:
				return random_sequence(sequence_butterfly_lookat01, 14);
			default:
				return sequence + 1;
			}
		}
		case weapon_knife_stiletto:
		{
			switch (sequence)
			{
			case sequence_default_lookat01:
				return random_sequence(12, 13);
			default:
				return sequence;
			}
		}
		case weapon_knife_widowmaker:
		{
			switch (sequence)
			{
			case sequence_default_lookat01:
				return random_sequence(14, 15);
			default:
				return sequence;
			}
		}
		default:
			return sequence;
		}
	}

	short item_def_glove()
	{
		switch (g_cfg.skins.model_glove)
		{
		case 1:
			return glove_studded_bloodhound;
			break;
		case 2:
			return glove_sporty;
			break;
		case 3:
			return glove_slick;
			break;
		case 4:
			return glove_leather_handwraps;
			break;
		case 5:
			return glove_motorcycle;
			break;
		case 6:
			return glove_specialist;
			break;
		case 7:
			return glove_studded_hydra;
			break;
		default:
			break;
		}
	}

	int glove_skins_id()
	{
		switch (g_cfg.skins.glove_skin)
		{
		case 0:
			return 10006;
			break;
		case 1:
			return 10007;
			break;
		case 2:
			return 10008;
			break;
		case 3:
			return 10009;
			break;
		case 4:
			return 10010;
			break;
		case 5:
			return 10013;
			break;
		case 6:
			return 10015;
			break;
		case 7:
			return 10016;
			break;
		case 8:
			return 10018;
			break;
		case 9:
			return 10019;
			break;
		case 10:
			return 10021;
			break;
		case 11:
			return 10024;
			break;
		case 12:
			return 10026;
			break;
		case 13:
			return 10027;
			break;
		case 14:
			return 10028;
			break;
		case 15:
			return 10030;
			break;
		case 16:
			return 10033;
			break;
		case 17:
			return 10034;
			break;
		case 18:
			return 10035;
			break;
		case 19:
			return 10036;
			break;
		case 20:
			return 10037;
			break;
		case 21:
			return 10038;
			break;
		case 22:
			return 10039;
			break;
		case 23:
			return 10040;
			break;
		case 24:
			return 10041;
			break;
		case 25:
			return 10042;
			break;
		case 26:
			return 10043;
			break;
		case 27:
			return 10044;
			break;
		case 28:
			return 10045;
			break;
		case 29:
			return 10046;
			break;
		case 30:
			return 10047;
			break;
		case 31:
			return 10048;
			break;
		case 32:
			return 10049;
			break;
		case 33:
			return 10050;
			break;
		case 34:
			return 10051;
			break;
		case 35:
			return 10052;
			break;
		case 36:
			return 10053;
			break;
		case 37:
			return 10054;
			break;
		case 38:
			return 10055;
			break;
		case 39:
			return 10056;
			break;
		case 40:
			return 10057;
			break;
		case 41:
			return 10058;
			break;
		case 42:
			return 10059;
			break;
		case 43:
			return 10060;
			break;
		case 44:
			return 10061;
			break;
		case 45:
			return 10062;
			break;
		case 46:
			return 10063;
			break;
		case 47:
			return 10064;
			break;
		default:
			break;
		}
	}

	static __forceinline auto get_wearable_create_fn() -> create_client_class_fn
	{
		auto classes = interfaces::client->get_client_classes();

		while (classes->class_id != CEconWearable)
			classes = classes->next_ptr;

		return classes->create_fn;
	}

	static __forceinline c_basecombatweapon* make_glove(int entry, int serial) noexcept
	{
		get_wearable_create_fn()(entry, serial);

		auto glove = (c_basecombatweapon*)(interfaces::entity_list->get_entity(entry));

		if (!glove)
			return nullptr;

		static auto Fn = g_memory->find_pattern(modules::client, xor_c("55 8B EC 83 E4 F8 51 53 56 57 8B F1"));
		static auto set_abs_origin = Fn.as< void(__thiscall*)(void*, const vector3d&) >();

		set_abs_origin(glove, vector3d(16384.0f, 16384.0f, 16384.0f));
		return glove;
	}

	__forceinline bool set_paint_kit(c_basecombatweapon* weapon, skin_weapon_t& cfg)
	{
		auto& skin_data = paint_kits[cfg.skin];

		auto client_class = weapon->get_client_class();
		if (!client_class)
			return false;

		weapon->fallback_paint_kit() = skin_data.second;

		weapon->original_owner_xuid_low() = 0;
		weapon->original_owner_xuid_high() = 0;
		weapon->fallback_wear() = 0.001f;
		weapon->item_id_high() = -1;

		if (client_class->class_id == CKnife)
		{
			if (cfg.knife_model > 0)
			{
				auto model_index = interfaces::model_info->get_model_index(knife_models.at(cfg.knife_model - 1).data());
				auto world_model_index = interfaces::model_info->get_model_index(world_knife_models.at(cfg.knife_model - 1).data());

				auto item_index = knifes[cfg.knife_model].index;

				if (item_index != weapon->item_definition_index())
				{
					weapon->item_definition_index() = item_index;

					weapon->set_model_index(model_index);

					const auto networkable = weapon->get_networkable_entity();

					using fn = void(__thiscall*)(void*, const int);
					g_memory->getvfunc< fn >(networkable, 6)(networkable, 0);
				}

				auto view_model = g_ctx.local->get_view_model();
				if (view_model)
				{
					int sequence = view_model->sequence();
					int original_activity = view_model->get_sequence_activity(view_model->sequence()); // Get the original sequence activity

					auto world_weapon = view_model->get_view_model_weapon();
					if (world_weapon && world_weapon == weapon)
					{
						auto world_model = weapon->get_weapon_world_model();
						if (world_model)
						{
							view_model->set_model_index(model_index);
							world_model->set_model_index(world_model_index);
						}
					}
				}
			}

			auto updated = cfg.old_skin != cfg.skin;

			cfg.old_skin = cfg.skin;

			if (!updated)
				updated = cfg.old_model != cfg.knife_model;

			cfg.old_model = cfg.knife_model;

			return updated;
		}

		const auto updated = cfg.old_skin != cfg.skin;

		cfg.old_skin = cfg.skin;

		return updated;
	}

	__forceinline void force_update_skin(c_basecombatweapon* weapon)
	{
		*(bool*)((uintptr_t)weapon + 0x3370u) = false;

		auto& vec0 = *(c_utlvector< ret_counted_t* >*)((uintptr_t)weapon + offsets::m_Item + 0x14u);
		for (int i{}; i < vec0.m_size; ++i)
			vec0.m_memory.base()[i] = nullptr;

		vec0.m_size = 0;

		auto& vec1 = *(c_utlvector< ret_counted_t* >*)((uintptr_t)weapon + 0x9dcu);
		for (int i{}; i < vec1.m_size; ++i)
			vec1.m_memory.base()[i] = nullptr;

		vec1.m_size = 0;

		auto& vec2 = *(c_utlvector< ret_counted_t* >*)((uintptr_t)weapon + offsets::m_Item + 0x230u);
		for (int i{}; i < vec2.m_size; ++i)
		{
			auto& element = vec2.m_memory.base()[i];
			if (!element)
				continue;

			element->unref();
			element = nullptr;
		}

		vec2.m_size = 0;

		const auto networkable = weapon->get_networkable_entity();

		using fn = void(__thiscall*)(void*, const int);
		g_memory->getvfunc< fn >(networkable, 7)(networkable, 0);
		g_memory->getvfunc< fn >(networkable, 5)(networkable, 0);
	}

	__forceinline void force_update_hud()
	{
		auto hud_base = func_ptrs::find_hud_element(*patterns::get_hud_ptr.as< uintptr_t** >(), xor_c("CCSGO_HudWeaponSelection"));
		auto hud_weapons = (int*)hud_base - 0x28;

		for (auto i = 0; i < *(hud_weapons + 0x20); i++)
			i = patterns::clear_hud_weapons.as< int(__thiscall*)(int*, int) >()(hud_weapons, i);
	}

	__forceinline void glove_changer()
	{
		static int old_kit = -1;
		static short old_glove = -1;

		if (!g_ctx.in_game)
		{
			old_kit = -1;
			old_glove = -1;
			return;
		}

		player_info_t info{};
		interfaces::engine->get_player_info(g_ctx.local->index(), &info);

		if (g_cfg.skins.model_glove == 0)
		{
			if (old_kit != -1 || old_glove != -1)
			{
				call_force_update = true;

				old_kit = -1;
				old_glove = -1;
			}

			return;
		}

		static auto glove_handle = uint32_t(0);
		auto wearables = g_ctx.local->wearables();
		auto glove = (c_basecombatweapon*)(interfaces::entity_list->get_entity_handle(wearables[0]));

		if (!glove)
		{
			auto our_glove = (c_basecombatweapon*)(interfaces::entity_list->get_entity_handle(glove_handle));

			if (our_glove)
			{
				wearables[0] = glove_handle;
				glove = our_glove;
			}
		}

		if (!g_ctx.local->is_alive())
		{
			if (glove)
			{
				glove->set_destroyed_on_recreate_entities();
				glove->release();
			}

			old_glove = -1;
			old_kit = -1;
			return;
		}

		if (!g_ctx.weapon)
			return;

		auto glove_index = item_def_glove();
		auto glove_kit = glove_skins_id();

		if (glove_index)
		{
			if (!glove)
			{
				auto entry = interfaces::entity_list->get_highest_ent_index() + 1;
				auto serial = rand() % 0x1000;

				glove = make_glove(entry, serial);
				wearables[0] = entry | serial << 16;
				glove_handle = wearables[0];
			}

			static int force_update_count = 0;

			*reinterpret_cast<int*>(uintptr_t(glove) + 0x64) = -1;

			auto& paint_kit = glove->fallback_paint_kit();

			if (glove_kit && glove_kit != paint_kit)
			{
				paint_kit = glove_kit;

				if (old_kit != glove_kit)
				{
					force_update_count = 0;
					old_kit = glove_kit;
				}

				if (g_cfg.misc.menu && force_update_count < 1)
				{
					++force_update_count;
					call_force_update = true;
				}
			}

			glove->fallback_wear() = 0.001f;
			glove->item_id_high() = -1;

			auto& definition_index = glove->item_definition_index();

			const auto& replacement_item = get_weapon_info(glove_index);

			if (glove_index && glove_index != definition_index)
			{
				if (!replacement_item)
					return;

				definition_index = glove_index;

				if (weapon_indexes.find(replacement_item->model) == weapon_indexes.end())
					weapon_indexes.emplace(replacement_item->model, interfaces::model_info->get_model_index(replacement_item->model));

				glove->set_model_index(weapon_indexes.at(replacement_item->model));
				const auto networkable = glove->get_networkable_entity();

				using fn = void(__thiscall*)(void*, const int);
				g_memory->getvfunc< fn >(networkable, 6)(networkable, 0);

				if (old_glove != glove_index)
				{
					force_update_count = 0;
					old_glove = glove_index;
				}

				// ghetto fix
				// it gets 0 item definition index and call force update multiple times
				// but glove is still applied
				// idk how to fix it, let's just update it once then
				if (g_cfg.misc.menu && force_update_count < 1)
				{
					++force_update_count;
					call_force_update = true;
				}
			}
		}
	}

	__forceinline void on_postdataupdate_start(int stage)
	{
		if (stage == frame_net_update_end && interfaces::client_state->delta_tick > 0)
			in_force_update = false;

		if (!g_ctx.local)
			return;

		mask_changer(stage);

		if (stage == frame_net_update_postdataupdate_end)
			glove_changer();

		if (!g_ctx.weapon)
			return;

		if (stage != frame_net_update_postdataupdate_start)
			return;

		agent_changer(stage);

		auto weapon_list = g_ctx.local->get_weapons();

		for (auto weapon : weapon_list)
		{
			if (!weapon)
				continue;

			auto& skin_cfg = g_cfg.skins.skin_weapon[rage_tools::get_legit_tab(weapon)];
			if (!skin_cfg.enable)
			{
				if (skin_cfg.old_skin != -1)
				{
					call_force_update = true;
					skin_cfg.old_skin = -1;
				}

				continue;
			}

			if (weapon->is_misc_weapon() && !weapon->is_knife())
				continue;

			if (!set_paint_kit(weapon, skin_cfg))
				continue;

			if (g_cfg.misc.menu)
				call_force_update = true;
		}

		if (!call_force_update || interfaces::client_state->delta_tick == -1 || std::abs(interfaces::global_vars->cur_time - last_update_time) < 1.f)
		{
			if (update_hud && !in_force_update)
			{
				for (auto weapon : weapon_list)
				{
					if (!weapon)
						continue;

					force_update_skin(weapon);
				}

				force_update_hud();
				update_hud = false;
			}

			return;
		}

		interfaces::client_state->delta_tick = -1;

		call_force_update = false;
		in_force_update = update_hud = true;

		last_update_time = interfaces::global_vars->cur_time;
	}

	__forceinline void on_game_events(c_game_event* event)
	{
		if (std::strcmp(event->get_name(), xor_c("player_connect_full")) || std::strcmp(event->get_name(), xor_c("round_start")))
			return;

		int user_id = interfaces::engine->get_player_for_user_id(event->get_int(xor_c("userid")));

		c_csplayer* player = (c_csplayer*)interfaces::entity_list->get_entity(user_id);
		if (!player)
			return;

		if (player->index() != interfaces::engine->get_local_player())
			return;

		auto mask = get_mask_model_name();
		precached_on_round_start = should_precache(default_mask) && should_precache(mask);

		if (!std::strcmp(event->get_name(), xor_c("round_start")))
		{
			if (player->is_alive() && !should_precache(xor_c("models/player/custom_player/legacy/ctm_fbi_varianth.mdl")))
				precached_on_round_start = false;
		}
	}

	__forceinline void on_frame_render_end(int stage)
	{
		if (stage != frame_render_end)
			return;

		if (!g_ctx.local || !g_ctx.weapon)
			return;

		agent_changer(stage);
	}
}