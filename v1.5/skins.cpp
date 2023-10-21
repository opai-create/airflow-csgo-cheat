#include "globals.hpp"
#include "skins.hpp"

namespace string_convert
{
	INLINE std::string to_string(const std::wstring_view str)
	{
		if (str.empty())
			return {};

		const auto len = WideCharToMultiByte(CP_UTF8, 0, str.data(), str.size(), 0, 0, 0, 0);

		std::string ret{};

		ret.resize(len);

		WideCharToMultiByte(CP_UTF8, 0, str.data(), str.size(), ret.data(), len, 0, 0);

		return ret;
	}

	INLINE std::wstring to_wstring(const std::string_view& str)
	{
		if (str.empty())
			return std::wstring();

		int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), NULL, 0);

		std::wstring out(size_needed, 0);
		MultiByteToWideChar(CP_UTF8, 0, &str[0], static_cast<int>(str.size()), &out[0], size_needed);

		return out;
	}
}

namespace skin_changer
{
	bool call_force_update = false;
	bool in_force_update = false;
	bool update_hud = false;
	float last_update_time = 0.f;

	constexpr auto mask_flags = 0x10000;

#ifdef LEGACY
	constexpr auto max_knifes = 11;
#else
	constexpr auto max_knifes = 20;
#endif

	constexpr auto max_gloves = 7;
	const char* default_mask = CXOR("models/player/holiday/facemasks/facemask_battlemask.mdl");

	std::unordered_map<std::string, int> weapon_indexes{};

#ifdef LEGACY
	std::array< std::string, max_knifes - 1 > knife_models{
		XOR("models/weapons/v_knife_bayonet.mdl"),
			XOR("models/weapons/v_knife_flip.mdl"),
			XOR("models/weapons/v_knife_gut.mdl"),
			XOR("models/weapons/v_knife_karam.mdl"),
			XOR("models/weapons/v_knife_m9_bay.mdl"),
			XOR("models/weapons/v_knife_tactical.mdl"),
			XOR("models/weapons/v_knife_falchion_advanced.mdl"),
			XOR("models/weapons/v_knife_survival_bowie.mdl"),
			XOR("models/weapons/v_knife_butterfly.mdl"),
			XOR("models/weapons/v_knife_push.mdl"),
	};

	std::array< std::string, max_knifes - 1 > world_knife_models{
		XOR("models/weapons/w_knife_bayonet.mdl"),
			XOR("models/weapons/w_knife_flip.mdl"),
			XOR("models/weapons/w_knife_gut.mdl"),
			XOR("models/weapons/w_knife_karam.mdl"),
			XOR("models/weapons/w_knife_m9_bay.mdl"),
			XOR("models/weapons/w_knife_tactical.mdl"),
			XOR("models/weapons/w_knife_falchion_advanced.mdl"),
			XOR("models/weapons/w_knife_survival_bowie.mdl"),
			XOR("models/weapons/w_knife_butterfly.mdl"),
			XOR("models/weapons/w_knife_push.mdl"),
	};
#else
	std::array<std::string, max_knifes - 1> knife_models
	{
		XOR("models/weapons/v_knife_bayonet.mdl"),
			XOR("models/weapons/v_knife_css.mdl"),
			XOR("models/weapons/v_knife_skeleton.mdl"),
			XOR("models/weapons/v_knife_outdoor.mdl"),
			XOR("models/weapons/v_knife_cord.mdl"),
			XOR("models/weapons/v_knife_canis.mdl"),
			XOR("models/weapons/v_knife_flip.mdl"),
			XOR("models/weapons/v_knife_gut.mdl"),
			XOR("models/weapons/v_knife_karam.mdl"),
			XOR("models/weapons/v_knife_m9_bay.mdl"),
			XOR("models/weapons/v_knife_tactical.mdl"),
			XOR("models/weapons/v_knife_falchion_advanced.mdl"),
			XOR("models/weapons/v_knife_survival_bowie.mdl"),
			XOR("models/weapons/v_knife_butterfly.mdl"),
			XOR("models/weapons/v_knife_push.mdl"),
			XOR("models/weapons/v_knife_ursus.mdl"),
			XOR("models/weapons/v_knife_gypsy_jackknife.mdl"),
			XOR("models/weapons/v_knife_stiletto.mdl"),
			XOR("models/weapons/v_knife_widowmaker.mdl"),
	};


	std::array<std::string, max_knifes - 1> world_knife_models
	{
		XOR("models/weapons/w_knife_bayonet.mdl"),
			XOR("models/weapons/w_knife_css.mdl"),
			XOR("models/weapons/w_knife_skeleton.mdl"),
			XOR("models/weapons/w_knife_outdoor.mdl"),
			XOR("models/weapons/w_knife_cord.mdl"),
			XOR("models/weapons/w_knife_canis.mdl"),
			XOR("models/weapons/w_knife_flip.mdl"),
			XOR("models/weapons/w_knife_gut.mdl"),
			XOR("models/weapons/w_knife_karam.mdl"),
			XOR("models/weapons/w_knife_m9_bay.mdl"),
			XOR("models/weapons/w_knife_tactical.mdl"),
			XOR("models/weapons/w_knife_falchion_advanced.mdl"),
			XOR("models/weapons/w_knife_survival_bowie.mdl"),
			XOR("models/weapons/w_knife_butterfly.mdl"),
			XOR("models/weapons/w_knife_push.mdl"),
			XOR("models/weapons/w_knife_ursus.mdl"),
			XOR("models/weapons/w_knife_gypsy_jackknife.mdl"),
			XOR("models/weapons/w_knife_stiletto.mdl"),
			XOR("models/weapons/w_knife_widowmaker.mdl"),
	};
#endif

	std::array<std::string, max_gloves> gloves
	{
		XOR("models/weapons/w_models/arms/w_glove_bloodhound.mdl"),
			XOR("models/weapons/w_models/arms/w_glove_sporty.mdl"),
			XOR("models/weapons/w_models/arms/w_glove_slick.mdl"),
			XOR("models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl"),
			XOR("models/weapons/w_models/arms/w_glove_motorcycle.mdl"),
			XOR("models/weapons/w_models/arms/w_glove_specialist.mdl"),
			XOR("models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl"),
	};

	struct knife_id_t
	{
		short index{};
		std::string name{};
	};

	struct weapon_info
	{
		constexpr weapon_info(const char* model, const char* icon = nullptr, int animindex = -1)
			: model(model), icon(icon), animindex(animindex) {}

		const char* model;
		const char* icon;
		int animindex;
	};

	const std::map< int, weapon_info > weaponInfoMdls =
	{
		{ WEAPON_KNIFE, { CXOR("models/weapons/v_knife_default_ct.mdl"), CXOR("knife_default_ct"), 2 } },
		{ WEAPON_KNIFE_T, { CXOR("models/weapons/v_knife_default_t.mdl"), CXOR("knife_t"), 12 } },
		{ WEAPON_KNIFE_BAYONET, { CXOR("models/weapons/v_knife_bayonet.mdl"), CXOR("bayonet"), 0 } },
		{ WEAPON_KNIFE_CSS, { CXOR("models/weapons/v_knife_css.mdl"), CXOR("css"), 0 } },
		{ WEAPON_KNIFE_FLIP, {CXOR("models/weapons/v_knife_flip.mdl"), CXOR("knife_flip"), 4}},
		{ WEAPON_KNIFE_GUT, { CXOR("models/weapons/v_knife_gut.mdl"), CXOR("knife_gut"), 5 } },
		{ WEAPON_KNIFE_KARAMBIT, {CXOR("models/weapons/v_knife_karam.mdl"), CXOR("knife_karambit"), 7}},
		{ WEAPON_KNIFE_M9_BAYONET, { CXOR("models/weapons/v_knife_m9_bay.mdl"), CXOR("knife_m9_bayonet"), 8 } },
		{ WEAPON_KNIFE_TACTICAL, { CXOR("models/weapons/v_knife_tactical.mdl"), CXOR("knife_tactical") } },
		{ WEAPON_KNIFE_FALCHION, { CXOR("models/weapons/v_knife_falchion_advanced.mdl"), CXOR("knife_falchion"), 3 } },
		{ WEAPON_KNIFE_SURVIVAL_BOWIE, { CXOR("models/weapons/v_knife_survival_bowie.mdl"), CXOR("knife_survival_bowie"), 11 } },
		{ WEAPON_KNIFE_BUTTERFLY, { CXOR("models/weapons/v_knife_butterfly.mdl"), CXOR("knife_butterfly"), 1 } },
		{ WEAPON_KNIFE_PUSH, { CXOR("models/weapons/v_knife_push.mdl"), CXOR("knife_push"), 9 } },
		{ WEAPON_KNIFE_URSUS, { CXOR("models/weapons/v_knife_ursus.mdl"), CXOR("knife_ursus"), 13 } },
		{ WEAPON_KNIFE_GYPSY_JACKKNIFE, { CXOR("models/weapons/v_knife_gypsy_jackknife.mdl"), CXOR("knife_gypsy_jackknife"), 6 } },
		{ WEAPON_KNIFE_STILETTO, { CXOR("models/weapons/v_knife_stiletto.mdl"), CXOR("knife_stiletto"), 10 } },
		{ WEAPON_KNIFE_WIDOWMAKER, { CXOR("models/weapons/v_knife_widowmaker.mdl"), CXOR("knife_widowmaker"), 14 } },
		{ GLOVE_STUDDED_BLOODHOUND, { CXOR("models/weapons/w_models/arms/w_glove_bloodhound.mdl") } },
		{ GLOVE_T, { CXOR("models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl") } },
		{ GLOVE_CT, { CXOR("models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl") } },
		{ GLOVE_SPORTY, { CXOR("models/weapons/w_models/arms/w_glove_sporty.mdl") } },
		{ GLOVE_SLICK, { CXOR("models/weapons/w_models/arms/w_glove_slick.mdl") } },
		{ GLOVE_LEATHER_HANDWRAPS, { CXOR("models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl") } },
		{ GLOVE_MOTORCYCLE, { CXOR("models/weapons/w_models/arms/w_glove_motorcycle.mdl") } },
		{ GLOVE_SPECIALIST, { CXOR("models/weapons/w_models/arms/w_glove_specialist.mdl") } },
		{ GLOVE_STUDDED_HYDRA, { CXOR("models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl") } },
		{ 521, { CXOR("models/weapons/v_knife_outdoor.mdl"), CXOR("knife_outdoor"), 14 } },
		{ 518, { CXOR("models/weapons/v_knife_canis.mdl"), CXOR("knife_canis"), 14 } },
		{ 517, { CXOR("models/weapons/v_knife_cord.mdl"), CXOR("knife_cord"), 14 } },
		{ 525, { CXOR("models/weapons/v_knife_skeleton.mdl"), CXOR("knife_skeleton"), 14 } },
		{ 503, { CXOR("models/weapons/v_knife_css.mdl"), CXOR("knife_css"), 14 } }
	};

#ifdef LEGACY
	std::array<knife_id_t, max_knifes> knifes
	{
		knife_id_t{ WEAPON_NONE, XOR("def") },
			knife_id_t{ WEAPON_KNIFE_BAYONET, XOR("bayonet") },
			knife_id_t{ WEAPON_KNIFE_FLIP, XOR("flip") },
			knife_id_t{ WEAPON_KNIFE_GUT, XOR("gut") },
			knife_id_t{ WEAPON_KNIFE_KARAMBIT, XOR("karambit") },
			knife_id_t{ WEAPON_KNIFE_M9_BAYONET, XOR("m9 bayonet") },
			knife_id_t{ WEAPON_KNIFE_TACTICAL, XOR("tactical") },
			knife_id_t{ WEAPON_KNIFE_FALCHION, XOR("falchion") },
			knife_id_t{ WEAPON_KNIFE_SURVIVAL_BOWIE, XOR("bowie") },
			knife_id_t{ WEAPON_KNIFE_BUTTERFLY, XOR("butterfly") },
			knife_id_t{ WEAPON_KNIFE_PUSH, XOR("push") },
	};
#else
	std::array<knife_id_t, max_knifes> knifes
	{
		knife_id_t{ WEAPON_NONE, XOR("def") },
			knife_id_t{ WEAPON_KNIFE_BAYONET, XOR("bayonet") },
			knife_id_t{ WEAPON_KNIFE_CSS, XOR("css") },
			knife_id_t{ WEAPON_KNIFE_SKELETON, XOR("skeleton") },
			knife_id_t{ WEAPON_KNIFE_OUTDOOR, XOR("outdoor") },
			knife_id_t{ WEAPON_KNIFE_CORD, XOR("cord") },
			knife_id_t{ WEAPON_KNIFE_CANIS, XOR("canis") },
			knife_id_t{ WEAPON_KNIFE_FLIP, XOR("flip") },
			knife_id_t{ WEAPON_KNIFE_GUT, XOR("gut") },
			knife_id_t{ WEAPON_KNIFE_KARAMBIT, XOR("karambit") },
			knife_id_t{ WEAPON_KNIFE_M9_BAYONET, XOR("m9 bayonet") },
			knife_id_t{ WEAPON_KNIFE_TACTICAL, XOR("tactical") },
			knife_id_t{ WEAPON_KNIFE_FALCHION, XOR("falchion") },
			knife_id_t{ WEAPON_KNIFE_SURVIVAL_BOWIE, XOR("bowie") },
			knife_id_t{ WEAPON_KNIFE_BUTTERFLY, XOR("butterfly") },
			knife_id_t{ WEAPON_KNIFE_PUSH, XOR("push") },
			knife_id_t{ WEAPON_KNIFE_URSUS, XOR("ursus") },
			knife_id_t{ WEAPON_KNIFE_GYPSY_JACKKNIFE, XOR("gypsy") },
			knife_id_t{ WEAPON_KNIFE_STILETTO, XOR("stiletto") },
			knife_id_t{ WEAPON_KNIFE_WIDOWMAKER, XOR("widowmaker") }
	};
#endif

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

	const weapon_info* get_weapon_info(int defindex)
	{
		const auto entry = weaponInfoMdls.find(defindex);
		return entry == weaponInfoMdls.end() ? nullptr : &entry->second;
	}

	bool precached_on_round_start = true;

	__forceinline bool should_precache(const char* name)
	{
		if (name == "")
			return false;

		if (name == nullptr)
			return false;

		auto modelprecache = HACKS->network_string_table_container->find_table(CXOR("modelprecache"));
		if (!modelprecache)
			return false;

		auto idx = modelprecache->add_string(false, name);
		if (idx == -1)
			return false;

		return true;
	}

	const std::vector<std::string> mask_models =
	{
		 XOR("models\\player\\holiday\\facemasks\\facemask_battlemask.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_hoxton.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\porcelain_doll.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_skull.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_samurai.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\evil_clown.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_wolf.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_sheep_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_bunny_gold.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_anaglyph.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_porcelain_doll_kabuki.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_dallas.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_pumpkin.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_sheep_bloody.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_devil_plastic.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_boar.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_chains.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tiki.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_bunny.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_sheep_gold.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_zombie_fortune_plastic.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_chicken.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_skull_gold.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_demo_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_engi_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_heavy_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_medic_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_pyro_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_scout_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_sniper_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_soldier_model.mdl"),
		 XOR("models\\player\\holiday\\facemasks\\facemask_tf2_spy_model.mdl"),
		 XOR("models\\props\\holiday_light\\holiday_light.mdl"),
	};

	__forceinline const char* get_mask_model_name()
	{
		if (g_cfg.skins.masks == 0 || g_cfg.skins.masks >= mask_models.size())
			return "";

		return mask_models[g_cfg.skins.masks - 1].c_str();
	}

	const std::vector<std::string> player_models =
	{
		CXOR("models\\player\\custom_player\\legacy\\tm_jumpsuit_varianta.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jumpsuit_variantb.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jumpsuit_variantc.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_diver_varianta.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_diver_variantb.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_diver_variantc.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_fbi_varianth.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_fbi_variantf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_fbi_variantb.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_fbi_variantg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gendarmerie_varianta.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variantb.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variantc.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variantd.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gendarmerie_variante.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_sas_variantg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_sas_variantf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_variante.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_variantg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_varianti.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_variantj.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_variantk.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_variantl.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_variantm.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_st6_variantn.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_swat_variante.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_swat_variantf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_swat_variantg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_swat_varianth.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_swat_varianti.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_swat_variantj.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_swat_variantk.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varj.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_vari.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varh.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varf5.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varf4.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varf3.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varf2.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varf1.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_professional_varf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_phoenix_varianti.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_phoenix_varianth.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_phoenix_variantg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_phoenix_variantf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_leet_variantj.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_leet_varianti.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_leet_varianth.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_leet_variantg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_leet_variantf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantf2.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jungle_raider_variante.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantd.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantb2.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jungle_raider_variantb.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_jungle_raider_varianta.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_balkan_varianth.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_balkan_variantj.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_balkan_varianti.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_balkan_variantf.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_balkan_variantg.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_balkan_variantk.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_balkan_variantl.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gign.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gign_variantA.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gign_variantB.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gign_variantC.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\ctm_gign_variantD.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_pirate.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_pirate_variantA.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_pirate_variantB.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_pirate_variantC.mdl"),
		CXOR("models\\player\\custom_player\\legacy\\tm_pirate_variantD.mdl"),
	};

	__forceinline std::string get_model_name(int team)
	{
		switch (team)
		{
		case 2:
		{

			if (g_cfg.skins.model_t == 76)
				return g_cfg.skins.custom_model_t;

			return (size_t)(g_cfg.skins.model_t - 1) < player_models.size() ? player_models[g_cfg.skins.model_t - 1] : std::string();
		}
		case 3:
		{
			if (g_cfg.skins.model_ct == 76)
				return g_cfg.skins.custom_model_ct;

			return (size_t)(g_cfg.skins.model_ct - 1) < player_models.size() ? player_models[g_cfg.skins.model_ct - 1] : std::string();
		}
		default:
			return {};
		}
	};

	// special thakns to infirms1337 for this code
	__forceinline void mask_changer(int stage)
	{
#ifndef LEGACY
		static auto current_mask = *offsets::mask_ptr.cast< char*** >();
		static int old_mask = -1;

		if (!HACKS->local || stage != FRAME_RENDER_START && stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			return;

		if (!precached_on_round_start)
			return;

		auto mask = get_mask_model_name();

		if (!should_precache(default_mask) || !should_precache(mask))
			return;

		if (g_cfg.skins.masks > 0)
		{
			HACKS->local->addon_bits().force(mask_flags);

			if (old_mask != g_cfg.skins.masks)
			{
				*current_mask = (char*)mask;
				offsets::update_addon_models.cast<void(__thiscall*)(void*, bool)>()(HACKS->local, true);
				old_mask = g_cfg.skins.masks;
			}
		}
		else
		{
			if (HACKS->local->addon_bits().has(mask_flags))
				HACKS->local->addon_bits().remove(mask_flags);
		}
#endif
	}

	inline int get_original_model_idx()
	{
		static int result{};
		while (!result)
			result = HACKS->model_info->get_model_index(CXOR("models/player/custom_player/legacy/ctm_fbi_varianth.mdl"));

		return result;
	}

	__forceinline void agent_changer(int stage)
	{
#ifndef LEGACY
		if (!HACKS->local || !HACKS->local->is_alive() || stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START && stage != FRAME_RENDER_END)
			return;

		if (!precached_on_round_start)
			return;

		static std::string old_model_path{};
		static bool reset_model = false;
		static bool set_original_index = false;
		static auto get_viewmodel_config = offsets::viewmodel_arm_cfg.cast<std::add_pointer_t<const char** __fastcall(const char*)>>();

		int team = HACKS->local->team();
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

		auto model = get_model_name(HACKS->local->team());
		auto full_model_path = HACKS->exe_path + model;

		if (old_model_path != full_model_path)
		{
			set_original_index = !main_utils::file_have_extension(full_model_path.c_str(), CXOR(".mdl"));
			old_model_path = full_model_path;
		}

		if (use_custom_models && set_original_index)
		{
			if (original_model != 0)
				HACKS->local->set_model_index(original_model);
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

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		{
			if (should_precache(model.c_str()))
			{
				const auto viewmodel_arm_config = get_viewmodel_config(model.c_str());
				if (!viewmodel_arm_config || !should_precache(viewmodel_arm_config[2]) || !should_precache(viewmodel_arm_config[3]))
					return;
			}
		}

		auto idx = HACKS->model_info->get_model_index(model.c_str());
		if (idx == 0)
			return;

		HACKS->local->set_model_index(idx);
#endif
	}

	__forceinline void init_parser()
	{
		for (int i = 0; i <= HACKS->item_schema->paint_kits.last_element; ++i)
		{
			const auto& paint_kit = HACKS->item_schema->paint_kits.memory[i]._value;
			if (paint_kit->id == 9001 || paint_kit->id >= 10000)
				continue;

			auto name = string_convert::to_string(HACKS->localize->find_safe(paint_kit->item_name.buffer + 1));
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
		case WEAPON_KNIFE_BUTTERFLY:
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
		case WEAPON_KNIFE_FALCHION:
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
		case WEAPON_KNIFE_CSS:
		{
			switch (sequence)
			{
			case sequence_default_lookat01:
				return random_sequence(sequence_css_lookat01, sequence_css_lookat02);
			default:
				return sequence;
			}
		}
		case WEAPON_KNIFE_PUSH:
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
		case WEAPON_KNIFE_SURVIVAL_BOWIE:
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
		case WEAPON_KNIFE_URSUS:
		case WEAPON_KNIFE_CORD:
		case WEAPON_KNIFE_CANIS:
		case WEAPON_KNIFE_OUTDOOR:
		case WEAPON_KNIFE_SKELETON:
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
		case WEAPON_KNIFE_STILETTO:
		{
			switch (sequence)
			{
			case sequence_default_lookat01:
				return random_sequence(12, 13);
			default:
				return sequence;
			}
		}
		case WEAPON_KNIFE_WIDOWMAKER:
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
			return GLOVE_STUDDED_BLOODHOUND;
			break;
		case 2:
			return GLOVE_SPORTY;
			break;
		case 3:
			return GLOVE_SLICK;
			break;
		case 4:
			return GLOVE_LEATHER_HANDWRAPS;
			break;
		case 5:
			return GLOVE_MOTORCYCLE;
			break;
		case 6:
			return GLOVE_SPECIALIST;
			break;
		case 7:
			return GLOVE_STUDDED_HYDRA;
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
		auto classes = HACKS->client->get_client_classes();

		while (classes->class_id != CEconWearable)
			classes = classes->next_ptr;

		return classes->create_fn;
	}

	static __forceinline c_base_combat_weapon* make_glove(int entry, int serial) noexcept
	{
		get_wearable_create_fn()(entry, serial);

		auto glove = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity(entry));
		if (!glove)
			return nullptr;

		glove->set_abs_origin({ 16384.0f, 16384.0f, 16384.0f });
		return glove;
	}

	__forceinline bool set_paint_kit(c_base_combat_weapon* weapon, skin_weapon_t& cfg)
	{
		cfg.knife_model = std::clamp(cfg.knife_model, 0, max_knifes);

		int skin = std::clamp(cfg.skin, 0, (int)paint_kits.size() - 1);

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
				auto model_index = HACKS->model_info->get_model_index(knife_models.at(cfg.knife_model - 1).data());
				auto world_model_index = HACKS->model_info->get_model_index(world_knife_models.at(cfg.knife_model - 1).data());

				auto item_index = knifes[cfg.knife_model].index;

				if (item_index != weapon->item_definition_index())
				{
					weapon->item_definition_index() = item_index;

					weapon->set_model_index(model_index);

					const auto networkable = weapon->get_networkable_entity();

					using fn = void(__thiscall*)(void*, const int);
					memory::get_virtual(networkable, 6).cast<fn>()(networkable, 0);
				}

				auto view_model = (c_cs_player*)(HACKS->entity_list->get_client_entity_handle(HACKS->local->viewmodel()));
				if (view_model)
				{
					int sequence = view_model->sequence();
					int original_activity = view_model->get_sequence_activity(view_model->sequence()); // Get the original sequence activity

					auto world_weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(view_model->viewmodel_weapon()));
					if (world_weapon && world_weapon == weapon)
					{
						auto world_model = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(weapon->weapon_world_model()));
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

	__forceinline void force_update_skin(c_base_combat_weapon* weapon)
	{
		static auto m_Item = netvars::get_offset(HASH("DT_BaseCombatWeapon"), HASH("m_Item"));

#ifdef LEGACY
		* (bool*)((uintptr_t)weapon + 0x32DD) = weapon->fallback_paint_kit() <= 0;
#else
		* (bool*)((uintptr_t)weapon + 0x3370) = false;
#endif

		auto& vec0 = *(c_utl_vector< ret_counted_t* >*)((uintptr_t)weapon + m_Item + 0x14);
		for (int i{}; i < vec0.count(); ++i)
			vec0.base()[i] = nullptr;

		vec0.remove_count();

		auto& vec1 = *(c_utl_vector< ret_counted_t* >*)((uintptr_t)weapon + 0x9DC);
		for (int i{}; i < vec1.count(); ++i)
			vec1.base()[i] = nullptr;

		vec1.remove_count();

#ifdef LEGACY
		auto& vec2 = *(c_utl_vector< ret_counted_t* >*)((uintptr_t)weapon + m_Item + 0x220);
#else
		auto& vec2 = *(c_utl_vector< ret_counted_t* >*)((uintptr_t)weapon + m_Item + 0x230);
#endif
		for (int i{}; i < vec2.count(); ++i)
		{
			auto& element = vec2.base()[i];
			if (!element)
				continue;

			element->unref();
			element = nullptr;
		}

		vec2.remove_count();

		const auto networkable = weapon->get_networkable_entity();

		using fn = void(__thiscall*)(void*, const int);
		memory::get_virtual(networkable, 7).cast<fn>()(networkable, 0);
		memory::get_virtual(networkable, 5).cast<fn>()(networkable, 0);

#ifdef LEGACY
		auto hud_selection = (void*)offsets::find_hud_element.cast<DWORD(__thiscall*)(void*, const char*)>()(*offsets::get_hud_ptr.cast< uintptr_t** >(),
			CXOR("SFWeaponSelection"));

		if (!hud_selection)
			return;

		offsets::show_and_update_selection.cast<void(__thiscall*)(void*, int, c_base_combat_weapon*, bool)>()(hud_selection, 0, weapon, false);
#endif
	}

	__forceinline void force_update_hud()
	{
#ifndef LEGACY
		auto hud_base = offsets::find_hud_element.cast<DWORD(__thiscall*)(void*, const char*)>()
			(*offsets::get_hud_ptr.cast< uintptr_t** >(), CXOR("CCSGO_HudWeaponSelection"));
		auto hud_weapons = (int*)hud_base - 0x28;

		for (auto i = 0; i < *(hud_weapons + 0x20); i++)
			i = offsets::clear_hud_weapons.cast< int(__thiscall*)(int*, int) >()(hud_weapons, i);
#endif
	}

	__forceinline void glove_changer()
	{
		static int old_kit = -1;
		static short old_glove = -1;

		if (!HACKS->in_game)
		{
			old_kit = -1;
			old_glove = -1;
			return;
		}

		player_info_t info{};
		HACKS->engine->get_player_info(HACKS->local->index(), &info);

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
		auto wearables = HACKS->local->wearables();
		auto glove = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(wearables[0]));

		if (!glove)
		{
			auto our_glove = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(glove_handle));

			if (our_glove)
			{
				wearables[0] = glove_handle;
				glove = our_glove;
			}
		}

		if (!HACKS->local->is_alive())
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

		if (!HACKS->weapon)
			return;

		auto glove_index = item_def_glove();
		auto glove_kit = glove_skins_id();

		if (glove_index)
		{
			if (!glove)
			{
				auto entry = HACKS->entity_list->get_highest_entity_index() + 1;
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
					weapon_indexes.emplace(replacement_item->model, HACKS->model_info->get_model_index(replacement_item->model));

				glove->set_model_index(weapon_indexes.at(replacement_item->model));
				const auto networkable = glove->get_networkable_entity();

				using fn = void(__thiscall*)(void*, const int);
				memory::get_virtual(networkable, 6).cast<fn>()(networkable, 0);

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
		if (stage == FRAME_NET_UPDATE_END && HACKS->client_state->delta_tick > 0)
			in_force_update = false;

		if (!HACKS->local)
			return;

		mask_changer(stage);

		if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END)
			glove_changer();

		if (!HACKS->weapon)
			return;

		if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
			return;

		agent_changer(stage);

		auto weapon_list = HACKS->local->get_weapons();

		for (auto weapon : weapon_list)
		{
			if (!weapon)
				continue;

			auto& skin_cfg = g_cfg.skins.skin_weapon[main_utils::get_legit_tab(weapon)];
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

		if (!call_force_update || HACKS->client_state->delta_tick == -1 || std::abs(HACKS->global_vars->curtime - last_update_time) < 1.f)
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

		HACKS->client_state->delta_tick = -1;

		call_force_update = false;
		in_force_update = update_hud = true;

		last_update_time = HACKS->global_vars->curtime;
	}

	__forceinline void on_game_events(c_game_event* event)
	{
		if (std::strcmp(event->get_name(), CXOR("player_connect_full")) || std::strcmp(event->get_name(), CXOR("round_start")))
			return;

		int user_id = HACKS->engine->get_player_for_user_id(event->get_int(CXOR("userid")));

		auto player = (c_cs_player*)HACKS->entity_list->get_client_entity(user_id);
		if (!player)
			return;

		if (player->index() != HACKS->engine->get_local_player())
			return;

		auto mask = get_mask_model_name();
		if (!precached_on_round_start)
		{
			auto result = should_precache(default_mask) && should_precache(mask);
			if (result)
				precached_on_round_start = true;
		}

		if (!std::strcmp(event->get_name(), CXOR("round_start")))
		{
			if (player->is_alive() && !should_precache(CXOR("models/player/custom_player/legacy/ctm_fbi_varianth.mdl")))
				precached_on_round_start = false;
		}
	}

	__forceinline void on_frame_render_end(int stage)
	{
		if (stage != FRAME_RENDER_END)
			return;

		if (!HACKS->local || !HACKS->weapon)
			return;

		agent_changer(stage);
	}
}