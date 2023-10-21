#include "protect.h"
#include "../sdk/entity.h"

#include "main_utils.h"

#include <filesystem>

namespace main_utils
{
	std::string get_executable_file_path()
	{
		char result[MAX_PATH]{};
		return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
	}

	inline bool file_exist(const char* path)
	{
		static auto mdl_extension = xor_c(".mdl");

		auto path_exist = std::filesystem::exists(path); 
		if (path_exist)
		{
			std::filesystem::path model_path(path);
			if (!model_path.has_extension())
				return false;

			const auto& str = model_path.extension().string();
			return std::strstr(str.c_str(), mdl_extension);
		}
	}

	inline bool is_weapon(c_baseentity* entity, int class_id)
	{
		if (is_grenade(class_id))
			return true;

		if (is_bomb(class_id))
			return true;

		if (entity->is_weapon())
			return true;

		return false;
	}

	inline std::string get_projectile_name(model_t* model, const int& class_id)
	{
		switch (class_id)
		{
		case CBaseCSGrenadeProjectile:
			return strstr(model->name, xor_str("flashbang").c_str()) ? xor_str("FLASH") : xor_str("HE");
			break;
		case CBreachChargeProjectile:
			return xor_str("BREACH");
			break;
		case CBumpMineProjectile:
			return xor_str("MINE");
			break;
		case CDecoyProjectile:
			return xor_str("DECOY");
			break;
		case CMolotovProjectile:
		case CIncendiaryGrenade:
			return xor_str("FIRE");
			break;
		case CSensorGrenadeProjectile:
			return xor_str("SENSOR");
			break;
		case CSmokeGrenadeProjectile:
			return xor_str("SMOKE");
			break;
		case CSnowballProjectile:
			return xor_str("SNOW");
			break;
		}
		return xor_str(" ");
	}

	inline const char8_t* get_projectile_icon(model_t* model, const int& class_id)
	{
		switch (class_id)
		{
		case CBaseCSGrenadeProjectile:
			return strstr(model->name, xor_str("flashbang").c_str()) ? u8"\uE02B" : u8"\uE02C";
			break;
		case CDecoyProjectile:
			return u8"\uE02F";
			break;
		case CMolotovProjectile:
		case CIncendiaryGrenade:
			return u8"\uE02E";
			break;
		case CSmokeGrenadeProjectile:
			return u8"\uE02D";
			break;
		}
		return u8" ";
	}

	void draw_hitbox(c_csplayer* player, matrix3x4_t* bones, int idx, int idx2, bool dur)
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
				interfaces::debug_overlay->add_capsule_overlay(vMin, vMax, hitbox->radius, 255, 255 * idx, 255 * idx2, 150, dur ? interfaces::global_vars->interval_per_tick * 2 : 5.f, 0, 1);
		}
	}
}