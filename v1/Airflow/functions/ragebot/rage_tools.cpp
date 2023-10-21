#include "rage_tools.h"
#include "ragebot.h"
#include "autowall.h"
#include "engine_prediction.h"

#include "../features.h"

namespace rage_tools
{
	constexpr int max_traces = 48;

	rage_weapon_t zeus_config = { true, false, false, 40, 101, 101, 80, 80, 0, 0, 0, 1, 0, {} };

	constexpr int total_seeds = 255;

	vector2d calc_spread_angle(int bullets, float recoil_index, int i)
	{
		auto index = g_ctx.weapon->item_definition_index();

		math::random_seed(i + 1u);

		auto v1 = math::random_float(0.f, 1.f);
		auto v2 = math::random_float(0.f, M_PI * 2.f);

		float v3{}, v4{};
		if (cvars::weapon_accuracy_shotgun_spread_patterns->get_int() > 0)
			func_ptrs::calc_shotgun_spread(index, 0, static_cast<int>(bullets * recoil_index), &v4, &v3);
		else
		{
			v3 = math::random_float(0.f, 1.f);
			v4 = math::random_float(0.f, M_PI * 2.f);
		}

		if (recoil_index < 3.f && index == weapon_negev)
		{
			for (auto i = 3; i > recoil_index; --i)
			{
				v1 *= v1;
				v3 *= v3;
			}

			v1 = 1.f - v1;
			v3 = 1.f - v3;
		}

		const auto inaccuracy = v1 * g_engine_prediction->predicted_inaccuracy;
		const auto spread = v3 * g_engine_prediction->predicted_spread;

		return { std::cos(v2) * inaccuracy + std::cos(v4) * spread, std::sin(v2) * inaccuracy + std::sin(v4) * spread };
	}

	int get_legit_tab(c_basecombatweapon* temp_weapon)
	{
		auto weapon = temp_weapon ? temp_weapon : g_ctx.weapon;
		if (!weapon)
			return 0;

		if (weapon->is_knife())
			return weapon_cfg_knife;

		auto idx = weapon->item_definition_index();

		switch (idx)
		{
		case weapon_deagle:
			return weapon_cfg_deagle;
			break;
		case weapon_elite:
			return weapon_cfg_duals;
			break;
		case weapon_fiveseven:
			return weapon_cfg_fiveseven;
			break;
		case weapon_glock:
			return weapon_cfg_glock;
			break;
		case weapon_ak47:
			return weapon_cfg_ak47;
			break;
		case weapon_aug:
			return weapon_cfg_aug;
			break;
		case weapon_awp:
			return weapon_cfg_awp;
			break;
		case weapon_famas:
			return weapon_cfg_famas;
			break;
		case weapon_g3sg1:
			return weapon_cfg_g3sg1;
			break;
		case weapon_galilar:
			return weapon_cfg_galil;
			break;
		case weapon_m249:
			return weapon_cfg_m249;
			break;
		case weapon_m4a1:
			return weapon_cfg_m4a1;
			break;
		case weapon_mac10:
			return weapon_cfg_mac10;
			break;
		case weapon_p90:
			return weapon_cfg_p90;
			break;
		case weapon_mp5sd:
			return weapon_cfg_mp5sd;
			break;
		case weapon_ump45:
			return weapon_cfg_ump45;
			break;
		case weapon_xm1014:
			return weapon_cfg_xm1014;
			break;
		case weapon_bizon:
			return weapon_cfg_bizon;
			break;
		case weapon_mag7:
			return weapon_cfg_mag7;
			break;
		case weapon_negev:
			return weapon_cfg_negev;
			break;
		case weapon_sawedoff:
			return weapon_cfg_sawedoff;
			break;
		case weapon_tec9:
			return weapon_cfg_tec9;
			break;
		case weapon_hkp2000:
			return weapon_cfg_p2000;
			break;
		case weapon_mp7:
			return weapon_cfg_mp7;
			break;
		case weapon_mp9:
			return weapon_cfg_mp9;
			break;
		case weapon_nova:
			return weapon_cfg_nova;
			break;
		case weapon_p250:
			return weapon_cfg_p250;
			break;
		case weapon_scar20:
			return weapon_cfg_scar20;
			break;
		case weapon_sg556:
			return weapon_cfg_sg556;
			break;
		case weapon_ssg08:
			return weapon_cfg_ssg08;
			break;
		case weapon_m4a1_silencer:
			return weapon_cfg_m4a1s;
			break;
		case weapon_usp_silencer:
			return weapon_cfg_usps;
			break;
		case weapon_cz75a:
			return weapon_cfg_cz75;
			break;
		case weapon_revolver:
			return weapon_cfg_revolver;
			break;
		default:
			return 0;
			break;
		}
	}

	skin_weapon_t get_skin_weapon_config()
	{
		if (!g_ctx.local || !g_ctx.local->is_alive())
			return {};

		if (!g_ctx.weapon)
			return {};

		int tab = get_legit_tab();
		return g_cfg.skins.skin_weapon[tab];
	}

	legit_weapon_t get_legit_weapon_config()
	{
		if (!g_ctx.local || !g_ctx.local->is_alive())
			return {};

		if (!g_ctx.weapon)
			return {};

		int tab = std::clamp(get_legit_tab(), 0, (int)weapon_cfg_revolver);
		return g_cfg.legit.legit_weapon[tab];
	}

	rage_weapon_t get_weapon_config()
	{
		if (!g_ctx.local || !g_ctx.local->is_alive())
			return {};

		if (!g_ctx.weapon)
			return {};

		if (g_cfg.rage.weapon[auto_snipers].enable && g_ctx.weapon->is_auto_sniper())
			return g_cfg.rage.weapon[auto_snipers];
		else if (g_cfg.rage.weapon[heavy_pistols].enable && g_ctx.weapon->is_heavy_pistols())
			return g_cfg.rage.weapon[heavy_pistols];
		else if (g_cfg.rage.weapon[pistols].enable && g_ctx.weapon->is_pistols())
			return g_cfg.rage.weapon[pistols];
		else if (g_cfg.rage.weapon[scout].enable && g_ctx.weapon->item_definition_index() == weapon_ssg08)
			return g_cfg.rage.weapon[scout];
		else if (g_cfg.rage.weapon[awp].enable && g_ctx.weapon->item_definition_index() == weapon_awp)
			return g_cfg.rage.weapon[awp];
		else if (g_ctx.weapon->is_taser())
			return zeus_config;

		return g_cfg.rage.weapon[global];
	}

	std::string hitbox_to_string(int id)
	{
		switch (id)
		{
		case hitbox_head:
			return xor_c("head");
		case hitbox_neck:
			return xor_c("neck");
		case hitbox_pelvis:
			return xor_c("pelvis");
		case hitbox_stomach:
			return xor_c("stomach");
		case hitbox_lower_chest:
			return xor_c("lower chest");
		case hitbox_chest:
			return xor_c("chest");
		case hitbox_upper_chest:
			return xor_c("upper chest");
		case hitbox_left_thigh:
			return xor_c("left thigh");
		case hitbox_right_thigh:
			return xor_c("right thigh");
		case hitbox_left_calf:
			return xor_c("left calf");
		case hitbox_right_calf:
			return xor_c("right calf");
		case hitbox_left_foot:
			return xor_c("left foot");
		case hitbox_right_foot:
			return xor_c("right foot");
		case hitbox_left_hand:
			return xor_c("left hand");
		case hitbox_right_hand:
			return xor_c("right hand");
		case hitbox_left_upper_arm:
			return xor_c("left upper arm");
		case hitbox_left_forearm:
			return xor_c("left forearm");
		case hitbox_right_upper_arm:
			return xor_c("right upper arm");
		case hitbox_right_forearm:
			return xor_c("right forearm");
		}

		return "";
	}

	std::string hitgroup_to_string(int hitgroup)
	{
		switch (hitgroup)
		{
		case hitgroup_generic:
			return xor_c("generic");
			break;
		case hitgroup_head:
			return xor_c("head");
			break;
		case hitgroup_chest:
			return xor_c("chest");
			break;
		case hitgroup_stomach:
			return xor_c("body");
			break;
		case hitgroup_leftarm:
			return xor_c("left arm");
			break;
		case hitgroup_rightarm:
			return xor_c("right arm");
			break;
		case hitgroup_leftleg:
			return xor_c("left leg");
			break;
		case hitgroup_rightleg:
			return xor_c("right leg");
			break;
		case hitgroup_gear:
			return xor_c("gear");
			break;
		case hitgroup_neck:
			return xor_c("neck");
			break;
		default:
			return xor_c("unknown");
		}
	}

	int hitbox_to_hitgroup(int hitbox)
	{
		switch (hitbox)
		{
		case hitbox_head:
		case hitbox_neck:
			return hitgroup_head;
			break;
		case hitbox_pelvis:
		case hitbox_stomach:
			return hitgroup_stomach;
			break;
		case hitbox_lower_chest:
		case hitbox_chest:
		case hitbox_upper_chest:
			return hitgroup_chest;
			break;
		case hitbox_left_thigh:
		case hitbox_left_calf:
		case hitbox_left_foot:
			return hitgroup_leftleg;
			break;
		case hitbox_right_thigh:
		case hitbox_right_calf:
		case hitbox_right_foot:
			return hitgroup_rightleg;
			break;
		case hitbox_left_hand:
		case hitbox_left_upper_arm:
		case hitbox_left_forearm:
			return hitgroup_leftarm;
			break;
		case hitbox_right_hand:
		case hitbox_right_upper_arm:
		case hitbox_right_forearm:
			return hitgroup_rightarm;
			break;
		default:
			return hitgroup_generic;
			break;
		}
	}

	bool can_hit_hitbox_wrap(const vector3d& start, const vector3d& end, c_csplayer* player, int hitbox, records_t* record, matrix3x4_t* matrix)
	{
		auto current_bones = matrix ? matrix : record->sim_orig.bone;
		auto model = player->get_model();
		if (!model)
			return false;

		auto studio_model = interfaces::model_info->get_studio_model(player->get_model());
		auto set = studio_model->get_hitbox_set(0);

		if (!set)
			return false;

		auto studio_box = set->get_hitbox(hitbox);
		if (!studio_box)
			return false;

		vector3d min{}, max{};

		math::vector_transform(studio_box->bbmin, current_bones[studio_box->bone], min);
		math::vector_transform(studio_box->bbmax, current_bones[studio_box->bone], max);

		if (studio_box->radius != 1.f)
			return math::segment_to_segment(start, end, min, max) < studio_box->radius;

		math::vector_i_transform(start, current_bones[studio_box->bone], min);
		math::vector_i_transform(end, current_bones[studio_box->bone], max);
		return math::intersect_line_with_bb(min, max, studio_box->bbmin, studio_box->bbmax);
	}

	bool can_hit_hitbox(const vector3d& start, const vector3d& end, c_csplayer* player, int hitbox, records_t* record, matrix3x4_t* matrix)
	{
		return can_hit_hitbox_wrap(start, end, player, hitbox, record, matrix);
	}

	bool is_accuracy_valid(c_csplayer* player, point_t& point, float amount, float* out_chance)
	{
#ifdef _DEBUG
		spread_point.reset();
		current_spread = 0.f;
		spread_points.clear();
#endif

		if (cvars::weapon_accuracy_nospread->get_int() > 0 || amount <= 0.f)
			return true;

		auto model = interfaces::model_info->get_studio_model(player->get_model());
		if (!model)
			return false;

		auto set = model->get_hitbox_set(player->hitbox_set());

		if (!set)
			return false;

		auto studio_box = set->get_hitbox(point.hitbox);
		if (!studio_box)
			return false;

		auto weapon = g_ctx.weapon;
		if (!weapon)
			return false;

		auto weapon_info = weapon->get_weapon_info();
		if (!weapon_info)
			return false;

		auto state = g_ctx.local->animstate();
		if (!state)
			return false;

		auto range = weapon_info->range;

		vector3d forward, right, up;
		vector3d start = g_ctx.eye_position;
		vector3d pos = math::angle_from_vectors(start, point.position);
		math::angle_to_vectors(pos, forward, right, up);

#ifdef _DEBUG
		if (debug_hitchance)
		{
			current_spread = g_ctx.spread;
			g_render->world_to_screen(point.position, spread_point);
		}
#endif
		const auto round = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };

		float accuracy_limit = 0.f;

		if (state->landing || g_ctx.local->fall_velocity() > 0.f)
			accuracy_limit = weapon_info->inaccuracy_land;
		else if (g_utils->on_ground())
		{
			if (g_ctx.local->flags() & fl_ducking && !g_anti_aim->is_fake_ducking())
				accuracy_limit = round(g_ctx.scoped ? weapon_info->inaccuracy_crouch_alt : weapon_info->inaccuracy_crouch);
			else
				accuracy_limit = round(g_ctx.scoped ? weapon_info->inaccuracy_stand_alt : weapon_info->inaccuracy_stand);
		}
		else
			accuracy_limit = weapon_info->inaccuracy_jump;

		auto heavy_weapon = g_ctx.weapon->item_definition_index() == weapon_awp;

		static int chance_ticks2, chance_ticks;

		float rounded_acuracy = round(g_engine_prediction->predicted_inaccuracy);

		bool valid_accuracy = g_ctx.spread > 0.f && g_ctx.spread <= g_ctx.ideal_spread && (g_ctx.spread / g_ctx.ideal_spread) >= amount
			|| accuracy_limit > 0.f && rounded_acuracy == accuracy_limit;

		if (!g_rage_bot->weapon_config.strict_mode && !heavy_weapon && point.center && !point.limbs)
		{
			if (valid_accuracy)
				++chance_ticks;

			if (chance_ticks >= 5)
			{
				*out_chance = amount;
				chance_ticks = 0;
				return true;
			}
		}

		int hits = 0;
		for (int i = 0; i < total_seeds; ++i)
		{
			auto spread_angle = calc_spread_angle(weapon_info->bullets, g_ctx.weapon->recoil_index(), i);

			auto direction = forward + (right * spread_angle.x) + (up * spread_angle.y);
			direction = direction.normalized();

			auto end = start + direction * range;
#ifdef _DEBUG
			if (debug_hitchance)
			{
				vector2d scr_end;
				if (g_render->world_to_screen(end, scr_end))
					spread_points.emplace_back(scr_end);
			}
#endif
			if (can_hit_hitbox(start, end, player, point.hitbox, point.record))
			{
				if (!point.center)
				{
					g_rage_bot->store(player);
					g_rage_bot->set_record(player, point.record);

					auto awall = g_auto_wall->fire_bullet(g_ctx.local, player, g_ctx.weapon_info, g_ctx.weapon->is_taser(), start, end);
					if (awall.dmg > 0)
						++hits;

					g_rage_bot->restore(player);
				}
				else
					++hits;
			}
		}

		*out_chance = (float)hits / (float)total_seeds;

		bool valid_weapon = g_ctx.weapon->is_auto_sniper() || g_ctx.weapon->is_heavy_pistols();

		if (!g_rage_bot->weapon_config.strict_mode && g_utils->on_ground() && valid_weapon && point.center && !point.limbs)
		{
			static float old_hitchance = 0.f;

			if (old_hitchance != *out_chance)
			{
				chance_ticks2 = 0;
				old_hitchance = *out_chance;
			}
			else
				++chance_ticks2;

			if (chance_ticks2 >= 3 || g_exploits->dt_bullet == 1)
			{
				chance_ticks2 = 0;
				return true;
			}
		}

		return ((float)hits / (float)total_seeds) >= amount;
	}

	inline float get_dynamic_scale(vector3d& point, const float& hitbox_radius)
	{
		auto spread = g_engine_prediction->predicted_spread + g_engine_prediction->predicted_inaccuracy;
		auto distance = point.dist_to(g_ctx.eye_position);

		auto new_dist = distance / std::sin(math::deg_to_rad(90.f - math::rad_to_deg(spread)));
		auto scale = (hitbox_radius - new_dist * spread) + 0.1f;

		return std::clamp(scale, 0.f, 0.95f);
	}

	std::vector< std::pair< vector3d, bool > > get_multipoints(c_csplayer* player, int hitbox, matrix3x4_t* matrix)
	{
		std::vector< std::pair< vector3d, bool > > points = {};

		auto model = player->get_model();
		if (!model)
			return points;

		auto hdr = interfaces::model_info->get_studio_model(model);
		if (!hdr)
			return points;

		auto set = hdr->get_hitbox_set(0);
		if (!set)
			return points;

		auto bbox = set->get_hitbox(hitbox);
		if (!bbox)
			return points;

		if (bbox->radius <= 0.f)
		{
			matrix3x4_t rot_matrix = {};
			rot_matrix.angle_matrix(bbox->rotation);

			matrix3x4_t mat = {};
			math::contact_transforms(matrix[bbox->bone], rot_matrix, mat);

			vector3d origin = mat.get_origin();

			vector3d center = (bbox->bbmin + bbox->bbmax) * 0.5f;

			if (hitbox == hitbox_left_foot || hitbox == hitbox_right_foot)
				points.emplace_back(center, true);

			if (points.empty())
				return points;

			for (auto& p : points)
			{
				p.first = { p.first.dot(mat.mat[0]), p.first.dot(mat.mat[1]), p.first.dot(mat.mat[2]) };
				p.first += origin;
			}
		}
		else
		{
			vector3d max = bbox->bbmax;
			vector3d min = bbox->bbmin;
			vector3d center = (bbox->bbmin + bbox->bbmax) * 0.5f;

			auto dynamic_scale = get_dynamic_scale(center, bbox->radius);

			auto head_slider = g_rage_bot->weapon_config.scale_head == -1 ? std::clamp(dynamic_scale, 0.f, 0.75f) : (0.75f * (g_rage_bot->weapon_config.scale_head * 0.01f));
			auto body_slider = g_rage_bot->weapon_config.scale_body == -1 ? std::clamp(dynamic_scale, 0.f, 0.20f) : (0.35f * (g_rage_bot->weapon_config.scale_body * 0.01f));

			float head_scale = bbox->radius * head_slider;
			float body_scale = bbox->radius * body_slider;

			constexpr float rotation = 0.70710678f;
			float near_center_scale = bbox->radius * (head_slider / 2.f);

			if (hitbox == hitbox_head)
			{
				points.emplace_back(center, true);

				vector3d point{};
				point = { max.x + rotation * head_scale, max.y - rotation * head_scale, max.z };
				points.emplace_back(point, false);

				point = { max.x + rotation * (std::max< float >(0.1f, head_scale) / 2.f), max.y - rotation * (std::max< float >(0.1f, head_scale) / 2.f), max.z };
				points.emplace_back(point, false);

				point = { max.x, max.y, max.z + head_scale };
				points.emplace_back(point, false);

				point = { max.x, max.y, max.z - head_scale };
				points.emplace_back(point, false);

				point = { max.x, max.y - head_scale, max.z };
				points.emplace_back(point, false);

				point = { max.x + near_center_scale, max.y + head_scale, max.z };
				points.emplace_back(point, false);
			}
			else
			{
				if (hitbox == hitbox_stomach)
				{
					points.emplace_back(center, true);
					points.emplace_back(vector3d(center.x, center.y, min.z + body_scale), false);
					points.emplace_back(vector3d(center.x, center.y, max.z - body_scale), false);
					points.emplace_back(vector3d{ center.x, max.y - body_scale, center.z }, false);
				}
				else if (hitbox == hitbox_pelvis || hitbox == hitbox_upper_chest)
				{
					points.emplace_back(center, true);
					points.emplace_back(vector3d(center.x, center.y, max.z + body_scale), false);
					points.emplace_back(vector3d(center.x, center.y, min.z - body_scale), false);
				}
				else if (hitbox == hitbox_lower_chest || hitbox == hitbox_chest)
				{
					points.emplace_back(center, true);
					points.emplace_back(vector3d(center.x, center.y, max.z + body_scale), false);
					points.emplace_back(vector3d(center.x, center.y, min.z - body_scale), false);

					points.emplace_back(vector3d{ center.x, max.y - body_scale, center.z }, false);
				}
				else if (hitbox == hitbox_right_calf || hitbox == hitbox_left_calf)
				{
					points.emplace_back(center, true);
					points.emplace_back(vector3d{ max.x - (bbox->radius / 2.f), max.y, max.z }, false);
				}
				else if (hitbox == hitbox_right_thigh || hitbox == hitbox_left_thigh)
				{
					points.emplace_back(center, true);
				}
				else if (hitbox == hitbox_right_upper_arm || hitbox == hitbox_left_upper_arm)
				{
					points.emplace_back(vector3d{ max.x + bbox->radius, center.y, center.z }, false);
				}
				else
					points.emplace_back(center, true);
			}

			if (points.empty())
				return points;

			for (auto& p : points)
				math::vector_transform(p.first, matrix[bbox->bone], p.first);
		}

		return points;
	}
}