#pragma once
#include <sys/stat.h>
#include <Windows.h>
#include <string>

class c_baseentity;

namespace main_utils
{
	extern std::string get_executable_file_path();
	extern bool file_exist(const char* path);

	extern std::string get_projectile_name(model_t* model, const int& class_id);
	extern const char8_t* get_projectile_icon(model_t* model, const int& class_id);
	extern bool is_weapon(c_baseentity* entity, int class_id);
	extern void draw_hitbox(c_csplayer* player, matrix3x4_t* bones, int idx, int idx2, bool dur = false);

	inline bool is_bomb(int class_id)
	{
		if (class_id == CC4 || class_id == CPlantedC4)
			return true;

		return false;
	}

	inline bool is_grenade(int class_id)
	{
		switch (class_id)
		{
		case(int)CBaseCSGrenade:
		case(int)CBaseCSGrenadeProjectile:
		case(int)CBreachCharge:
		case(int)CBreachChargeProjectile:
		case(int)CBumpMine:
		case(int)CBumpMineProjectile:
		case(int)CDecoyGrenade:
		case(int)CDecoyProjectile:
		case(int)CMolotovGrenade:
		case(int)CMolotovProjectile:
		case(int)CSensorGrenade:
		case(int)CSensorGrenadeProjectile:
		case(int)CSmokeGrenade:
		case(int)CSmokeGrenadeProjectile:
		case(int)CSnowballProjectile:
		case(int)CIncendiaryGrenade:
		case(int)CInferno:
			return true;
			break;
		}
		return false;
	}

	inline bool is_esp_projectile(int class_id)
	{
		switch (class_id)
		{
		case(int)CBaseCSGrenadeProjectile:
		case(int)CBreachChargeProjectile:
		case(int)CBumpMineProjectile:
		case(int)CDecoyProjectile:
		case(int)CMolotovProjectile:
		case(int)CSensorGrenadeProjectile:
		case(int)CSmokeGrenadeProjectile:
		case(int)CSnowballProjectile:
		case(int)CInferno:
			return true;
			break;
		}
		return false;
	}

	inline float create_clamped_sine(float frequency, float intensity, float offset, float current_time) 
	{
		return std::sinf(frequency * current_time) * intensity + offset;
	}
}