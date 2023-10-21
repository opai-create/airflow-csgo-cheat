#pragma once
#include "../base/other/color.h"
#include "menu/ui_structs.h"

#include <array>

enum fov_t
{
	world = 0,
	arms,
	zoom
};

enum world_clr_t
{
	walls = 0,
	props,
	sky,
	fog,
	lights,
	world_clr_max,
};

enum weapon_group_t
{
	global = 0,
	auto_snipers,
	heavy_pistols,
	pistols,
	scout,
	awp,
	weapon_max
};

enum quick_stop_options_t
{
	early = 1,
	between_shots = 2,
	force_accuracy = 4,
	in_air = 8,
	force_duck = 16,
};

enum removals_t
{
	scope = 1,
	vis_recoil = 2,
	post_process = 4,
	smoke = 8,
	flash = 16,
	fog_ = 32,
	shadow = 64,
	viewmodel_move = 128,
	landing_bob = 256,
};

enum rage_hitbox_t
{
	head = 1,
	chest = 2,
	stomach = 4,
	pelvis = 8,
	arms_ = 16,
	legs = 32
};

enum binds_idx_t
{
	fd_b = 0,
	sw_b,
	dt_b,
	hs_b,
	ap_b,
	tp_b,
	inv_b,
	edge_b,
	left_b,
	right_b,
	back_b,
	override_dmg_b,
	ens_lean_b,
	spike_b,
	toggle_legit_b,
	force_roll_b,
	freestand_b,
	force_body_b,
	force_sp_b,
	binds_max
};

enum esp_type_t
{
	esp_enemy,
	esp_weapon,
	esp_max
};

enum chams_type_t
{
	c_vis,
	c_xqz,
	c_history,
	c_onshot,
	c_ragdolls,
	c_local,
	c_viewmodel,
	c_wpn,
	c_attachments,
	c_fake,
	c_gloves,
	c_max
};

enum weapon_cfg_type_t
{
	weapon_cfg_deagle = 0,
	weapon_cfg_duals,
	weapon_cfg_fiveseven,
	weapon_cfg_glock,
	weapon_cfg_ak47,
	weapon_cfg_aug,
	weapon_cfg_awp,
	weapon_cfg_famas,
	weapon_cfg_g3sg1,
	weapon_cfg_galil,
	weapon_cfg_m249,
	weapon_cfg_m4a1,
	weapon_cfg_m4a1s,
	weapon_cfg_mac10,
	weapon_cfg_p90,
	weapon_cfg_mp5sd,
	weapon_cfg_ump45,
	weapon_cfg_xm1014,
	weapon_cfg_bizon,
	weapon_cfg_mag7,
	weapon_cfg_negev,
	weapon_cfg_sawedoff,
	weapon_cfg_tec9,
	weapon_cfg_p2000,
	weapon_cfg_mp7,
	weapon_cfg_mp9,
	weapon_cfg_nova,
	weapon_cfg_p250,
	weapon_cfg_scar20,
	weapon_cfg_sg556,
	weapon_cfg_ssg08,
	weapon_cfg_usps,
	weapon_cfg_cz75,
	weapon_cfg_revolver,
	weapon_cfg_knife,
	weapon_cfg_max
};

struct chams_t
{
	bool enable{};
	bool interp{};

	int material = 0;
	int glow_fill = 100;
	int shot_duration = 5;

	c_float_color main_color = (255, 255, 255, 255);
	c_float_color glow_color = (255, 255, 255, 255);

	char chams_sprite[128] = "models\\weapons\\customization\\paints\\custom\\money";
};

struct key_binds_t;

struct rage_weapon_t
{
	bool enable{};
	bool quick_stop{};
	bool auto_scope{};
	bool strict_mode{};

	int hitchance{};
	int mindamage = 1;
	int damage_override = 1;
	int scale_head = -1;
	int scale_body = -1;
	int group_type{};
	int hitbox_type{};

	bool prefer_body;
	bool prefer_safe;

	unsigned int hitboxes{};
	unsigned int hitchance_skips{1 | 2};
	unsigned int quick_stop_options{};

	std::array< unsigned int, 6 > hitbox_cond{};
};

struct legit_weapon_t
{
	bool enable{};
	bool backtrack{};
	bool quick_stop{};
	bool fov_cicle{};
	int fov{};
	int smooth{};
	int aim_delay{};
	unsigned int hitboxes{};

	c_float_color circle_color = c_float_color(255, 255, 255, 255);
};

struct skin_weapon_t
{
	bool enable{};
	int skin{};
	int knife_model{};
	int old_model = -1;
	int old_skin = -1;

	char skins_search[256]{};
};

struct configs_t
{
	struct ragebot_t
	{
		bool enable{};

		bool auto_fire{};
		bool air_defensive{};
		bool resolver{};

#if ALPHA || _DEBUG || BETA
		bool jitterfix{};
		int jitterfix_method{};

		bool round_lagcomp{};
		bool delay_on_breaklc{};
#endif

		int spike_amt{};
		int roll_amt = 50;
		int roll_amt_pitch = 0;

		int group_type{};

		std::array< rage_weapon_t, 6 > weapon{};
	} rage;

	struct legitbot_t
	{
		bool enable{};
		bool smoke_check{};
		bool flash_check{};
		bool autowall{};
		int min_damage{};
		int group_type{};

		struct
		{
			bool enable{};
			int amount{};
		} rcs;

		std::array< legit_weapon_t, weapon_cfg_max > legit_weapon{};
	} legit;

	struct anti_hit_t
	{
		bool enable{};

		int pitch{};
		int yaw{};

		bool at_targets{};

		bool def_pitch{};
		bool def_yaw{};
		bool random_jitter{};
		bool random_dsy{};
		bool random_amount{};

		int def_aa_mode{};

		bool desync{};
		int desync_mode{};

		int desync_left = 58;
		int desync_right = 58;

		int jitter_mode{};
		int jitter_range{};

		int yaw_add{};

		bool distortion{};
		int distortion_range = 100;
		int distortion_pitch{};

		bool fakelag{};
		bool fluctiate_in_air{};
		int fakelag_limit = 14;
		unsigned int fakelag_conditions{};

		bool jitter_move{};
		bool silent_onshot{};
	} antihit;

	struct visuals_t
	{
		chams_t chams[c_max]{
		  chams_t{ false, false, 0, 100, 5, c_float_color(116, 113, 254), c_float_color(80, 78, 138, 150) },        // C_VIS
		  chams_t{ false, false, 0, 100, 5, c_float_color(77, 104, 253), c_float_color(80, 78, 138, 150) },         // C_XQZ
		  chams_t{ false, false, 3, 100, 5, c_float_color(169, 166, 255, 80), c_float_color(80, 78, 138, 80) },     // C_HISTORY
		  chams_t{ false, false, 4, 100, 5, c_float_color(0, 0, 0, 0), c_float_color(102, 92, 138, 100) },          // C_ONSHOT
		  chams_t{ false, false, 0, 100, 5, c_float_color(108, 126, 184), c_float_color(80, 78, 138, 150) },        // C_RAGDOLLS
		  chams_t{ false, false, 3, 100, 5, c_float_color(148, 168, 255, 100), c_float_color(100, 100, 100, 150) }, // C_LOCAL
		  chams_t{ false, false, 3, 100, 5, c_float_color(169, 125, 255, 100), c_float_color(168, 169, 255, 150) }, // C_VIEWMODEL
		  chams_t{ false, false, 3, 100, 5, c_float_color(218, 125, 255, 100), c_float_color(100, 100, 100, 150) }, // C_WPN
		  chams_t{ false, false, 3, 100, 5, c_float_color(218, 125, 255, 100), c_float_color(100, 100, 100, 150) }, // C_ATTACHMENTS
		  chams_t{ false, false, 3, 100, 5, c_float_color(255, 255, 255, 100), c_float_color(0, 0, 0, 0) },         // C_FAKE
		};

		struct event_logs_t
		{
			bool enable{};
			bool filter_console{};
			unsigned int logs{};
		} eventlog;

		struct esp_t
		{
			bool enable{};
			bool nade_offscreen{};
			unsigned int elements{};

			int offscreen_size{15};
			int offscreen_dist{250};

			struct esp_colors_t
			{
				c_float_color box = c_float_color(255, 255, 255, 255);
				c_float_color name = c_float_color(255, 255, 255, 255);
				c_float_color health = c_float_color(162, 255, 115);
				c_float_color weapon = c_float_color(255, 255, 255, 255);
				c_float_color ammo_bar = c_float_color(164, 128, 255);
				c_float_color offscreen_arrow = c_float_color(139, 166, 226);
				c_float_color offscreen_arrow_outline = c_float_color(0, 0, 0);
				c_float_color glow = c_float_color(204, 94, 253, 130);
				c_float_color skeleton = c_float_color(219, 232, 255, 130);

				c_float_color smoke_range = c_float_color(94, 112, 251, 50);
				c_float_color molotov_range = c_float_color(251, 94, 204, 50);
			} colors;
		};

		std::array< esp_t, esp_max > esp{};

		bool grenade_predict{};

		c_float_color predict_clr = c_float_color(156, 150, 255, 255);

		bool grenade_warning{};
		bool grenade_warning_line{};

		c_float_color warning_clr = c_float_color(156, 200, 255, 255);

		bool local_glow{};
		c_float_color local_glow_color = c_float_color{ 255, 158, 158, 130 };

		bool show_all_history{};
	} visuals;

	struct misc_t
	{
		bool menu = false;

		bool auto_jump{};
		bool auto_strafe{};

		bool fast_stop{};
		bool slide_walk{};

		int fovs[3] = { 0, 0, 100 };
		int viewmodel_pos[3] = {};

		bool viewmodel_scope{};
		bool penetration_crosshair{};
		bool fix_sensitivity{};
		bool skip_second_zoom{};

		int aspect_ratio{};
		int strafe_smooth = 70;

		int thirdperson_dist = 150;
		bool thirdperson_dead{};

		bool blend_scope{};
		int scope_amt = 50;
		int attachments_amt = 100;

		bool impacts{};

		c_float_color server_clr = c_float_color(0, 0, 255, 127);
		c_float_color client_clr = c_float_color(255, 0, 0, 127);

		int impact_size = 10;

		unsigned int tracers{};
		int tracer_type{};

		c_float_color trace_clr[3] = { c_float_color(245, 66, 66), c_float_color(66, 164, 245), c_float_color(191, 66, 245) };

		bool custom_bloom{};
		int bloom_scale{};
		int exposure_min{};
		int exposure_max{};

		bool custom_fog{};
		int fog_start{};
		int fog_end = 9300;
		int fog_density = 70;

		char skybox_name[128]{};
		int skybox = 0;
		int prop_alpha = 100;

		unsigned int world_material_options{};
		unsigned int world_modulation{};

		struct
		{
			int x = 0, y = -140, z = 0;
		} sunset_angle;

		bool pen_xhair{};
		bool retrack_peek{};

		c_float_color texture_reflect_clr = { 255, 255, 255, 255 };
		c_float_color autopeek_clr_back = c_float_color(184, 255, 97, 255);
		c_float_color autopeek_clr = c_float_color(156, 200, 255, 255);

		c_float_color world_clr[world_clr_max]
		{
			c_float_color(43, 53, 73),
			c_float_color(255, 255, 255),
			c_float_color(255, 255, 255),
			c_float_color(255, 255, 255),
			c_float_color(255, 255, 255),
		};

		unsigned int hitmarker = 0;
		c_float_color hitmarker_clr = c_float_color(255, 255, 255);

		bool damage{};
		c_float_color damage_clr = c_float_color(150, 113, 220);

		int sound{};
		int sound_volume = 50;
		char sound_name[128]{};

		bool unlock_inventory{};
		bool snip_crosshair{};
		bool preverse_killfeed{};
		bool remove_ads{};
		bool force_radar{};

		int ragdoll_gravity{};

		bool clantag{};
		bool bypass_sv_pure{};
		bool spoof_sv_cheats{};
		bool unlock_hidden_cvars{};
		bool force_crash{};

		unsigned int removals{};

		struct buy_bot_t
		{
			bool enable{};
			int main_weapon{};
			int second_weapon{};
			unsigned int other_items{};
		} buybot;

		unsigned int menu_indicators = 1 | 2 | 4;
		unsigned int animation_changes{};

		ImVec2 keybind_position{};
		ImVec2 bomb_position{};
		ImVec2 watermark_position{};
		ImVec2 spectators_position{};

		c_float_color ui_color{ 168, 168, 255 };
	} misc;

	struct skinchanger_t
	{
		int model_t{};
		int model_ct{};
		int group_type{};

		int model_glove{};
		int glove_skin{};

		int masks{};

		char custom_model_t[128]{};
		char custom_model_ct[128]{};

		std::array< skin_weapon_t, weapon_cfg_max > skin_weapon{};
	} skins;

	std::array< key_binds_t, binds_max > binds = {
	  key_binds_t{ -1, 1, false, xor_c("Fake duck") },
	  key_binds_t{ -1, 1, false, xor_c("Slow walk") },
	  key_binds_t{ -1, 2, false, xor_c("Double tap") },
	  key_binds_t{ -1, 2, false, xor_c("Hide shots") },
	  key_binds_t{ -1, 2, false, xor_c("Auto peek") },
	  key_binds_t{ -1, 2, false, xor_c("Thirdperson") },
	  key_binds_t{ -1, 2, false, xor_c("Inverter") },
	  key_binds_t{ -1, 2, false, xor_c("Edge yaw") },
	  key_binds_t{ -1, 2, false, xor_c("Manual left") },
	  key_binds_t{ -1, 2, false, xor_c("Manual right") },
	  key_binds_t{ -1, 2, false, xor_c("Manual back") },
	  key_binds_t{ -1, 2, false, xor_c("Override dmg") },
	  key_binds_t{ -1, 1, false, xor_c("Force extend") },
	  key_binds_t{ -1, 1, false, xor_c("Ping spike") },
	  key_binds_t{ -1, 1, false, xor_c("Aim (legit)") },
	  key_binds_t{ -1, 1, false, xor_c("Roll override") },
	  key_binds_t{ -1, 1, false, xor_c("Freestanding") },
	  key_binds_t{ -1, 1, false, xor_c("Force body") },
	  key_binds_t{ -1, 1, false, xor_c("Force safe") },
	};
};

extern configs_t g_cfg;