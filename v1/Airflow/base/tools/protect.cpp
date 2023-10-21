#include "protect.h"
#include "cheat_info.h"

namespace xor_strs
{
	std::string hitbox_head{};
	std::string hitbox_chest{};
	std::string hitbox_stomach{};
	std::string hitbox_pelvis{};
	std::string hitbox_arms{};
	std::string hitbox_legs{};
	std::string hitbox_body{};
	std::string hitbox_limbs{};

	std::string weapon_default{};
	std::string weapon_auto{};
	std::string weapon_heavy_pistols{};
	std::string weapon_pistols{};
	std::string weapon_ssg08{};
	std::string weapon_awp{};
	std::string weapon_negev{};
	std::string weapon_m249{};
	std::string weapon_ak47{};
	std::string weapon_aug{};
	std::string weapon_duals{};
	std::string weapon_p250{};
	std::string weapon_cz{};

	std::string aa_disabled{};
	std::string aa_default{};

	std::string aa_pitch_down{};
	std::string aa_pitch_up{};

	std::string aa_yaw_back{};
	std::string aa_yaw_spin{};

	std::string aa_jitter_center{};
	std::string aa_jitter_offset{};
	std::string aa_jitter_random{};
	std::string aa_jitter_3way{};

	std::string aa_desync_jitter{};

	std::string aa_fakelag_max{};
	std::string aa_fakelag_jitter{};

	std::string vis_chams_textured{};
	std::string vis_chams_metallic{};
	std::string vis_chams_flat{};
	std::string vis_chams_glass{};
	std::string vis_chams_glow{};
	std::string vis_chams_bubble{};
	std::string vis_chams_money{};
	std::string vis_chams_fadeup{};

	std::string buybot_none{};

	std::string cfg_main{};
	std::string cfg_additional{};
	std::string cfg_misc{};
	std::string cfg_custom1{};
	std::string cfg_custom2{};

	std::string chams_visible{};
	std::string chams_xqz{};
	std::string chams_history{};
	std::string chams_onshot{};
	std::string chams_ragdolls{};
	std::string chams_viewmodel{};
	std::string chams_wpn{};
	std::string chams_attachments{};
	std::string chams_fake{};
	std::string chams_gloves{};

	std::string sound_metallic{};
	std::string sound_tap{};

	std::string box_default{};
	std::string box_thin{};

	std::string ragdoll_away{};
	std::string ragdoll_fly{};

	std::string target_damage{};
	std::string target_distance{};

	std::string sky_custom{};

	std::string tracer_beam{};
	std::string tracer_line{};
	std::string tracer_glow{};

	std::string defensive_trigger{};
	std::string defensive_always{};

	std::string knife_default{};
	std::string knife_bayonet{};
	std::string knife_css{};
	std::string knife_skeleton{};
	std::string knife_nomad{};
	std::string knife_paracord{};
	std::string knife_survival{};
	std::string knife_flip{};
	std::string knife_gut{};
	std::string knife_karambit{};
	std::string knife_m9{};
	std::string knife_huntsman{};
	std::string knife_falchion{};
	std::string knife_bowie{};
	std::string knife_butterfly{};
	std::string knife_shadow{};
	std::string knife_ursus{};
	std::string knife_navaga{};
	std::string knife_stiletto{};
	std::string knife_talon{};

	std::string weapon_cfg_deagle{};
	std::string weapon_cfg_duals{};
	std::string weapon_cfg_fiveseven{};
	std::string weapon_cfg_glock{};
	std::string weapon_cfg_ak{};
	std::string weapon_cfg_aug{};
	std::string weapon_cfg_awp{};
	std::string weapon_cfg_famas{};
	std::string weapon_cfg_g3sg1{};
	std::string weapon_cfg_galil{};
	std::string weapon_cfg_m249{};
	std::string weapon_cfg_m4a1{};
	std::string weapon_cfg_m4a1s{};
	std::string weapon_cfg_mac10{};
	std::string weapon_cfg_p90{};
	std::string weapon_cfg_mp5{};
	std::string weapon_cfg_ump45{};
	std::string weapon_cfg_xm1014{};
	std::string weapon_cfg_bizon{};
	std::string weapon_cfg_mag7{};
	std::string weapon_cfg_negev{};
	std::string weapon_cfg_sawed_off{};
	std::string weapon_cfg_tec9{};
	std::string weapon_cfg_p2000{};
	std::string weapon_cfg_mp7{};
	std::string weapon_cfg_mp9{};
	std::string weapon_cfg_nova{};
	std::string weapon_cfg_p250{};
	std::string weapon_cfg_scar20{};
	std::string weapon_cfg_sg553{};
	std::string weapon_cfg_scout{};
	std::string weapon_cfg_usps{};
	std::string weapon_cfg_cz75{};
	std::string weapon_cfg_revolver{};
	std::string weapon_cfg_knife{};

	std::string agent_default{};

	std::string agent_gign{};
	std::string agent_gign_a{};
	std::string agent_gign_b{};
	std::string agent_gign_c{};
	std::string agent_gign_d{};

	std::string agent_pirate{};
	std::string agent_pirate_a{};
	std::string agent_pirate_b{};
	std::string agent_pirate_c{};
	std::string agent_pirate_d{};

	std::string agent_danger_a{};
	std::string agent_danger_b{};
	std::string agent_danger_c{};
	std::string agent_cmdr_davida{};
	std::string agent_cmdr_frank{};
	std::string agent_cmdr_lieutenant{};
	std::string agent_cmdr_michael{};
	std::string agent_cmdr_operator{};
	std::string agent_cmdr_spec_agent_ava{};
	std::string agent_cmdr_markus{};
	std::string agent_cmdr_sous{};
	std::string agent_cmdr_chem_haz{};
	std::string agent_cmdr_chef_d{};
	std::string agent_cmdr_aspirant{};
	std::string agent_cmdr_officer{};
	std::string agent_cmdr_d_sq{};
	std::string agent_cmdr_b_sq{};
	std::string agent_cmdr_seal_team6{};
	std::string agent_cmdr_bunkshot{};
	std::string agent_cmdr_lt_commander{};
	std::string agent_cmdr_bunkshot2{};
	std::string agent_cmdr_3rd_commando{};
	std::string agent_cmdr_two_times_{};
	std::string agent_cmdr_two_times_2{};
	std::string agent_cmdr_premeiro{};
	std::string agent_cmdr_cmdr{};
	std::string agent_cmdr_1st_le{};
	std::string agent_cmdr_john_van{};
	std::string agent_cmdr_bio_haz{};
	std::string agent_cmdr_sergeant{};
	std::string agent_cmdr_chem_haz__{};
	std::string agent_cmdr_farwlo{};
	std::string agent_cmdr_getaway_sally{};
	std::string agent_cmdr_getaway_number_k{};
	std::string agent_cmdr_getaway_little_kev{};
	std::string agent_cmdr_safecracker{};
	std::string agent_cmdr_bloody_darryl{};
	std::string agent_cmdr_bloody_loud{};
	std::string agent_cmdr_bloody_royale{};
	std::string agent_cmdr_bloody_skullhead{};
	std::string agent_cmdr_bloody_silent{};
	std::string agent_cmdr_bloody_miami{};
	std::string agent_street_solider{};
	std::string agent_solider{};
	std::string agent_slingshot{};
	std::string agent_enforcer{};
	std::string agent_mr_muhlik{};
	std::string agent_prof_shahmat{};
	std::string agent_prof_osiris{};
	std::string agent_prof_ground_rebek{};
	std::string agent_prof_elite_muhlik{};
	std::string agent_prof_trapper{};
	std::string agent_prof_trapper_aggressor{};
	std::string agent_prof_vypa_sista{};
	std::string agent_prof_col_magnos{};
	std::string agent_prof_crasswater{};
	std::string agent_prof_crasswater_forgotten{};
	std::string agent_prof_solman{};
	std::string agent_prof_romanov{};
	std::string agent_prof_blackwolf{};
	std::string agent_prof_maximus{};
	std::string agent_prof_dragomir{};
	std::string agent_prof_dragomir2{};
	std::string agent_prof_rezan{};
	std::string agent_prof_rezan_red{};

	std::string mask_none{};
	std::string mask_battle{};
	std::string mask_hoxton{};
	std::string mask_doll{};
	std::string mask_skull{};
	std::string mask_samurai{};
	std::string mask_evil_clown{};
	std::string mask_wolf{};
	std::string mask_sheep{};
	std::string mask_bunny_gold{};
	std::string mask_anaglyph{};
	std::string mask_kabuki_doll{};
	std::string mask_dallas{};
	std::string mask_pumpkin{};
	std::string mask_sheep_bloody{};
	std::string mask_devil_plastic{};
	std::string mask_boar{};
	std::string mask_chains{};
	std::string mask_tiki{};
	std::string mask_bunny{};
	std::string mask_sheep_gold{};
	std::string mask_zombie_plastic{};
	std::string mask_chicken{};
	std::string mask_skull_gold{};
	std::string mask_demon_man{};
	std::string mask_engineer{};
	std::string mask_heavy{};
	std::string mask_medic{};
	std::string mask_pyro{};
	std::string mask_scout{};
	std::string mask_sniper{};
	std::string mask_solider{};
	std::string mask_spy{};
	std::string mask_holiday_light{};

	std::string sky_tibet{};
	std::string sky_bagage{};
	std::string sky_italy{};
	std::string sky_jungle{};
	std::string sky_office{};
	std::string sky_daylight{};
	std::string sky_daylight2{};
	std::string sky_vertigo_blue{};
	std::string sky_vertigo{};
	std::string sky_day{};
	std::string sky_nuke_bank{};
	std::string sky_venice{};
	std::string sky_daylight3{};
	std::string sky_daylight4{};
	std::string sky_cloudy{};
	std::string sky_night{};
	std::string sky_nightb{};
	std::string sky_night_flat{};
	std::string sky_dust{};
	std::string sky_vietnam{};
	std::string sky_lunacy{};
	std::string sky_embassy{};

	std::string glove_default{};
	std::string glove_bloodhound{};
	std::string glove_sporty{};
	std::string glove_slick{};
	std::string glove_leather_wrap{};
	std::string glove_motorcycle{};
	std::string glove_specialist{};
	std::string glove_hydra{};

	std::string glove_skin_charred{};
	std::string glove_skin_snakebite{};
	std::string glove_skin_bronzed{};
	std::string glove_skin_leather{};
	std::string glove_skin_spruce{};
	std::string glove_skin_lunar{};
	std::string glove_skin_convoy{};
	std::string glove_skin_crimson{};
	std::string glove_skin_superconductor{};
	std::string glove_skin_arid{};
	std::string glove_skin_slaugher{};
	std::string glove_skin_eclipse{};
	std::string glove_skin_spearmint{};
	std::string glove_skin_boom{};
	std::string glove_skin_coolmint{};
	std::string glove_skin_forest{};
	std::string glove_skin_crimson_kimono{};
	std::string glove_skin_emerald_web{};
	std::string glove_skin_foundation{};
	std::string glove_skin_badlands{};
	std::string glove_skin_pandora{};
	std::string glove_skin_hedge{};
	std::string glove_skin_guerilla{};
	std::string glove_skin_diamondback{};
	std::string glove_skin_king{};
	std::string glove_skin_imperial{};
	std::string glove_skin_overtake{};
	std::string glove_skin_racing{};
	std::string glove_skin_amphibious{};
	std::string glove_skin_bronze{};
	std::string glove_skin_omega{};
	std::string glove_skin_vice{};
	std::string glove_skin_pow{};
	std::string glove_skin_turtle{};
	std::string glove_skin_transport{};
	std::string glove_skin_polygon{};
	std::string glove_skin_cobalt{};
	std::string glove_skin_overprint{};
	std::string glove_skin_duct{};
	std::string glove_skin_arboreal{};
	std::string glove_skin_emerald{};
	std::string glove_skin_mangrove{};
	std::string glove_skin_rattler{};
	std::string glove_skin_case{};
	std::string glove_skin_crimson_web{};
	std::string glove_skin_buñkshot{};
	std::string glove_skin_fade{};
	std::string glove_skin_mogul{};

	__forceinline void init()
	{
		MUTATION_START

		hitbox_head = xor_str("Head");
		hitbox_chest = xor_str("Chest");
		hitbox_stomach = xor_str("Stomach");
		hitbox_pelvis = xor_str("Pelvis");
		hitbox_arms = xor_str("Arms");
		hitbox_legs = xor_str("Legs");
		hitbox_body = xor_str("Body");
		hitbox_limbs = xor_str("Limbs");

		weapon_default = xor_str("General");
		weapon_auto = xor_str("Auto snipers");
		weapon_heavy_pistols = xor_str("Deagle/R8");
		weapon_pistols = xor_str("Pistols");
		weapon_ssg08 = xor_str("Scout");
		weapon_awp = xor_str("AWP");
		weapon_negev = xor_str("Negev");
		weapon_m249 = xor_str("M249");
		weapon_ak47 = xor_str("AK47/M4A1");
		weapon_aug = xor_str("AUG/SG553");
		weapon_duals = xor_str("Dual berettas");
		weapon_p250 = xor_str("P250");
		weapon_cz = xor_str("CZ75/Five seven");

		aa_disabled = xor_str("Disabled");
		aa_default = xor_str("Default");

		aa_pitch_down = xor_str("Down");
		aa_pitch_up = xor_str("Up");

		aa_yaw_back = xor_str("Backward");
		aa_yaw_spin = xor_str("Spin");

		aa_jitter_center = xor_str("Center");
		aa_jitter_offset = xor_str("Offset");
		aa_jitter_random = xor_str("Random");
		aa_jitter_3way = xor_str("3-Way");

		aa_desync_jitter = xor_str("Jitter");

		aa_fakelag_max = xor_str("Maximum");
		aa_fakelag_jitter = xor_str("Jitter");

		vis_chams_textured = xor_str("Textured");
		vis_chams_metallic = xor_str("Shaded");
		vis_chams_flat = xor_str("Flat");
		vis_chams_glass = xor_str("Metallic");
		vis_chams_glow = xor_str("Glow");
		vis_chams_bubble = xor_str("Bubble");
		vis_chams_money = xor_str("Custom Sprite");
		vis_chams_fadeup = xor_str("Fade Up");

		buybot_none = xor_str("None");

		cfg_main = xor_str("Main");
		cfg_additional = xor_str("Additional");
		cfg_misc = xor_str("Misc");
		cfg_custom1 = xor_str("Custom1");
		cfg_custom2 = xor_str("Custom2");

		chams_visible = xor_str("Visible");
		chams_xqz = xor_str("Through walls");
		chams_history = xor_str("History");
		chams_onshot = xor_str("On-shot");
		chams_ragdolls = xor_str("Ragdolls");
		chams_viewmodel = xor_str("Viewmodel");
		chams_wpn = xor_str("Weapon");
		chams_attachments = xor_str("Attachments");
		chams_fake = xor_str("Fake");
		chams_gloves = xor_str("Gloves");

		sound_metallic = xor_str("Metallic");
		sound_tap = xor_str("Custom");

		box_default = xor_str("Default");
		box_thin = xor_str("Thin");

		ragdoll_away = xor_str("Away");
		ragdoll_fly = xor_str("Fly up");

		target_distance = xor_str("Lowest distance");
		target_damage = xor_str("Highest damage");

		sky_custom = xor_str("Custom");

		tracer_beam = xor_str("Beam");
		tracer_line = xor_str("Line");
		tracer_glow = xor_str("Glow");

		defensive_trigger = xor_str("Trigger");
		defensive_always = xor_str("Always on");

		knife_default = xor_str_s("Default");
		knife_bayonet = xor_str_s("Bayonet");
		knife_css = xor_str_s("CS:S");
		knife_skeleton = xor_str_s("Skeleton");
		knife_nomad = xor_str_s("Nomad");
		knife_paracord = xor_str_s("Paracord");
		knife_survival = xor_str_s("Survival");
		knife_flip = xor_str_s("Flip knife");
		knife_gut = xor_str_s("Gut knife");
		knife_karambit = xor_str_s("Karambit");
		knife_m9 = xor_str_s("M9 Bayonet");
		knife_huntsman = xor_str_s("Huntsman");
		knife_falchion = xor_str_s("Falchion");
		knife_bowie = xor_str_s("Bowie");
		knife_butterfly = xor_str_s("Butterfly");
		knife_shadow = xor_str_s("Shadow Daggers");
		knife_ursus = xor_str_s("Ursus");
		knife_navaga = xor_str_s("Navaja");
		knife_stiletto = xor_str_s("Stiletto");
		knife_talon = xor_str_s("Talon");

		weapon_cfg_deagle = xor_str("Deagle");
		weapon_cfg_duals = xor_str("Duals");
		weapon_cfg_fiveseven = xor_str("Five-Seven");
		weapon_cfg_glock = xor_str("Glock");
		weapon_cfg_ak = xor_str("AK47");
		weapon_cfg_aug = xor_str("AUG");
		weapon_cfg_awp = xor_str("AWP");
		weapon_cfg_famas = xor_str("FAMAS");
		weapon_cfg_g3sg1 = xor_str("G3SG1");
		weapon_cfg_galil = xor_str("GALIL-AR");
		weapon_cfg_m249 = xor_str("M249");
		weapon_cfg_m4a1 = xor_str("M4A1");
		weapon_cfg_m4a1s = xor_str("M4A1-s");
		weapon_cfg_mac10 = xor_str("MAC-10");
		weapon_cfg_p90 = xor_str("P-90");
		weapon_cfg_mp5 = xor_str("MP5");
		weapon_cfg_ump45 = xor_str("UMP45");
		weapon_cfg_xm1014 = xor_str("XM1014");
		weapon_cfg_bizon = xor_str("Bizon");
		weapon_cfg_mag7 = xor_str("MAG7");
		weapon_cfg_negev = xor_str("Negev");
		weapon_cfg_sawed_off = xor_str("Sawed Off");
		weapon_cfg_tec9 = xor_str("TEC9");
		weapon_cfg_p2000 = xor_str("P2000");
		weapon_cfg_mp7 = xor_str("MP7");
		weapon_cfg_mp9 = xor_str("MP9");
		weapon_cfg_nova = xor_str("NOVA");
		weapon_cfg_p250 = xor_str("P250");
		weapon_cfg_scar20 = xor_str("SCAR-20");
		weapon_cfg_sg553 = xor_str("SG553");
		weapon_cfg_scout = xor_str("SCOUT");
		weapon_cfg_usps = xor_str("USP-S");
		weapon_cfg_cz75 = xor_str("CZ-75");
		weapon_cfg_revolver = xor_str("Revolver");
		weapon_cfg_knife = xor_str("Knife");

		agent_default = xor_str("Default");

		agent_gign = xor_str("Gign");
		agent_gign_a = xor_str("Gign A");
		agent_gign_b = xor_str("Gign B");
		agent_gign_c = xor_str("Gign C");
		agent_gign_d = xor_str("Gign D");

		agent_pirate = xor_str("Pirate");
		agent_pirate_a = xor_str("Pirate A");
		agent_pirate_b = xor_str("Pirate B");
		agent_pirate_c = xor_str("Pirate C");
		agent_pirate_d = xor_str("Pirate D");

		agent_danger_a = xor_str("Danger zone A");
		agent_danger_b = xor_str("Danger zone B");
		agent_danger_c = xor_str("Danger zone C");
		agent_cmdr_davida = xor_str("Fernandez");
		agent_cmdr_frank = xor_str("Frank Baroud");
		agent_cmdr_lieutenant = xor_str("Rex Krikey");
		agent_cmdr_michael = xor_str("Michael Syfers");
		agent_cmdr_operator = xor_str("Operator");
		agent_cmdr_spec_agent_ava = xor_str("Agent Ava");
		agent_cmdr_markus = xor_str("Markus Delrow");
		agent_cmdr_sous = xor_str("Medic");
		agent_cmdr_chem_haz = xor_str("Haz Capitaine");
		agent_cmdr_chef_d = xor_str("Rouchard");
		agent_cmdr_aspirant = xor_str("Aspirant");
		agent_cmdr_officer = xor_str("Jacques Beltram");
		agent_cmdr_d_sq = xor_str("Squadron");
		agent_cmdr_b_sq = xor_str("Squadron 2");
		agent_cmdr_seal_team6 = xor_str("SASD-6");
		agent_cmdr_bunkshot = xor_str("Buckshot");
		agent_cmdr_lt_commander = xor_str("Ricksaw");
		agent_cmdr_bunkshot2 = xor_str("Buckshot 2");
		agent_cmdr_3rd_commando = xor_str("Company");
		agent_cmdr_two_times_ = xor_str("McCoy");
		agent_cmdr_two_times_2 = xor_str("McCoy New");
		agent_cmdr_premeiro = xor_str("Battalion");
		agent_cmdr_cmdr = xor_str("Mae Jamison");
		agent_cmdr_1st_le = xor_str("Farlow");
		agent_cmdr_john_van = xor_str("Healen Kask");
		agent_cmdr_bio_haz = xor_str("Haz Specialist");
		agent_cmdr_sergeant = xor_str("Bombson");
		agent_cmdr_chem_haz__ = xor_str("Chem Specialist");
		agent_cmdr_farwlo = xor_str("Farlow New");
		agent_cmdr_getaway_sally = xor_str("Sally");
		agent_cmdr_getaway_number_k = xor_str("Number K");
		agent_cmdr_getaway_little_kev = xor_str("Little Kev");
		agent_cmdr_safecracker = xor_str("Voltzmann");
		agent_cmdr_bloody_darryl = xor_str("Darryl Strapped");
		agent_cmdr_bloody_loud = xor_str("Loudmouth");
		agent_cmdr_bloody_royale = xor_str("Darryl Royale");
		agent_cmdr_bloody_skullhead = xor_str("Skullhead");
		agent_cmdr_bloody_silent = xor_str("Silent Darryl");
		agent_cmdr_bloody_miami = xor_str("Miami Darryl");
		agent_street_solider = xor_str("Street Soldier");
		agent_solider = xor_str("Soldier");
		agent_slingshot = xor_str("Slingshot");
		agent_enforcer = xor_str("Enforcer");
		agent_mr_muhlik = xor_str("Mr Muhlik");
		agent_prof_shahmat = xor_str("Prof Shahmat");
		agent_prof_osiris = xor_str("Osiris");
		agent_prof_ground_rebek = xor_str("Ground Rebel");
		agent_prof_elite_muhlik = xor_str("MR Muhlik");
		agent_prof_trapper = xor_str("Trapper");
		agent_prof_trapper_aggressor = xor_str("Aggressor");
		agent_prof_vypa_sista = xor_str("Revolution");
		agent_prof_col_magnos = xor_str("Dabisi");
		agent_prof_crasswater = xor_str("Medium Rare");
		agent_prof_crasswater_forgotten = xor_str("Forgotten");
		agent_prof_solman = xor_str("Solman");
		agent_prof_romanov = xor_str("Romanov");
		agent_prof_blackwolf = xor_str("Blackwolf");
		agent_prof_maximus = xor_str("Maximus");
		agent_prof_dragomir = xor_str("Dragomir");
		agent_prof_dragomir2 = xor_str("Dragomir 2");
		agent_prof_rezan = xor_str("Rezan The Ready");
		agent_prof_rezan_red = xor_str("Redshirt");

		mask_none = xor_str("None");
		mask_battle = xor_str("Battlemask");
		mask_hoxton = xor_str("Hoxton");
		mask_doll = xor_str("Doll");
		mask_skull = xor_str("Skull");
		mask_samurai = xor_str("Samurai");
		mask_evil_clown = xor_str("Evil Clown");
		mask_wolf = xor_str("Wolf");
		mask_sheep = xor_str("Sheep");
		mask_bunny_gold = xor_str("Bunny Gold");
		mask_anaglyph = xor_str("Anaglyph");
		mask_kabuki_doll = xor_str("Kabuki Doll");
		mask_dallas = xor_str("Dallas");
		mask_pumpkin = xor_str("Pumpkin");
		mask_sheep_bloody = xor_str("Sheep Bloody");
		mask_devil_plastic = xor_str("Devil Plastic");
		mask_boar = xor_str("Boar");
		mask_chains = xor_str("Chains");
		mask_tiki = xor_str("Tiki");
		mask_bunny = xor_str("Bunny");
		mask_sheep_gold = xor_str("Sheep gold");
		mask_zombie_plastic = xor_str("Zombie plastic");
		mask_chicken = xor_str("Chicken");
		mask_skull_gold = xor_str("Skull gold");
		mask_demon_man = xor_str("Demon man");
		mask_engineer = xor_str("Engineer");
		mask_heavy = xor_str("Heavy");
		mask_medic = xor_str("Medic");
		mask_pyro = xor_str("Pyro");
		mask_scout = xor_str("Scout");
		mask_sniper = xor_str("Sniper");
		mask_solider = xor_str("Solider");
		mask_spy = xor_str("Spy");
		mask_holiday_light = xor_str("Holiday Light");

		sky_tibet = xor_str("Tibet");
		sky_bagage = xor_str("Bagage");
		sky_italy = xor_str("Italy");
		sky_jungle = xor_str("Jungle");
		sky_office = xor_str("Office");
		sky_daylight = xor_str("Daylight 1");
		sky_daylight2 = xor_str("Daylight 2");
		sky_vertigo_blue = xor_str("Vertigo Blue");
		sky_vertigo = xor_str("Vertigo");
		sky_day = xor_str("Day");
		sky_nuke_bank = xor_str("Nuke Bank");
		sky_venice = xor_str("Venice");
		sky_daylight3 = xor_str("Daylight 3");
		sky_daylight4 = xor_str("Daylight 4");
		sky_cloudy = xor_str("Cloudy");
		sky_night = xor_str("Night");
		sky_nightb = xor_str("Night B");
		sky_night_flat = xor_str("Night Flat");
		sky_dust = xor_str("Dust");
		sky_vietnam = xor_str("Vietnam");
		sky_lunacy = xor_str("Lunacy");
		sky_embassy = xor_str("Embassy");

		glove_default = xor_str("Default");
		glove_bloodhound = xor_str("Bloodhound");
		glove_sporty = xor_str("Sporty");
		glove_slick = xor_str("Slick");
		glove_leather_wrap = xor_str("Leather Wrap");
		glove_motorcycle = xor_str("Motorcycle");
		glove_specialist = xor_str("Specialist");
		glove_hydra = xor_str("Hydra");

		glove_skin_charred = xor_str("Charred");
		glove_skin_snakebite = xor_str("Snakebite");
		glove_skin_bronzed = xor_str("Bronzed");
		glove_skin_leather = xor_str("Leather");
		glove_skin_spruce = xor_str("Spruce DDPAT");
		glove_skin_lunar = xor_str("Lunar Weave");
		glove_skin_convoy = xor_str("Convoy");
		glove_skin_crimson = xor_str("Crimson Weave");
		glove_skin_superconductor = xor_str("Superconductor");
		glove_skin_arid = xor_str("Arid");
		glove_skin_slaugher = xor_str("Slaughter");
		glove_skin_eclipse = xor_str("Eclipse");
		glove_skin_spearmint = xor_str("Spearmint");
		glove_skin_boom = xor_str("Boom!");
		glove_skin_coolmint = xor_str("Cool Mint");
		glove_skin_forest = xor_str("Forest DDPAT");
		glove_skin_crimson_kimono = xor_str("Crimson Kimono");
		glove_skin_emerald_web = xor_str("Emerald Web");
		glove_skin_foundation = xor_str("Foundation");
		glove_skin_badlands = xor_str("Badlands");
		glove_skin_pandora = xor_str("Pandora's Box");
		glove_skin_hedge = xor_str("Hedge Maze");
		glove_skin_guerilla = xor_str("Guerrilla");
		glove_skin_diamondback = xor_str("Diamondback");
		glove_skin_king = xor_str("King Snake");
		glove_skin_imperial = xor_str("Imperial Plaid");
		glove_skin_overtake = xor_str("Overtake");
		glove_skin_racing = xor_str("Racing Green");
		glove_skin_amphibious = xor_str("Amphibious");
		glove_skin_bronze = xor_str("Bronze Morph");
		glove_skin_omega = xor_str("Omega");
		glove_skin_vice = xor_str("Vice");
		glove_skin_pow = xor_str("POW!");
		glove_skin_turtle = xor_str("Turtle");
		glove_skin_transport = xor_str("Transport");
		glove_skin_polygon = xor_str("Polygon");
		glove_skin_cobalt = xor_str("Cobalt Skulls");
		glove_skin_overprint = xor_str("Overprint");
		glove_skin_duct = xor_str("Duct Tape");
		glove_skin_arboreal = xor_str("Arboreal");
		glove_skin_emerald = xor_str("Emerald");
		glove_skin_mangrove = xor_str("Mangrove");
		glove_skin_rattler = xor_str("Rattler");
		glove_skin_case = xor_str("Case Hardened");
		glove_skin_crimson_web = xor_str("Crimson Web");
		glove_skin_buñkshot = xor_str("Buckshot");
		glove_skin_fade = xor_str("Fade");
		glove_skin_mogul = xor_str("Mogul");

		MUTATION_END
	}
}