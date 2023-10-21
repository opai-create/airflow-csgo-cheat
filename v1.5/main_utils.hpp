#pragma once

struct skin_weapon_t;
struct legit_weapon_t;
struct rage_weapon_t;

namespace main_utils
{
	extern std::string get_executable_file_path();
	extern bool file_have_extension(const char* path, const char* extension);
	extern bool file_exist(const char* path);

	extern void draw_hitbox(c_cs_player* player, matrix3x4_t* bones, int idx, int idx2, bool dur = false);

	extern int get_legit_tab(c_base_combat_weapon* temp_weapon = nullptr);

	extern skin_weapon_t get_skin_weapon_config();
	extern legit_weapon_t get_legit_weapon_config();
	extern rage_weapon_t get_weapon_config();

	extern std::string hitgroup_to_string(int hitgroup);
	extern std::string hitbox_to_string(int id);

	inline float create_clamped_sine(float frequency, float intensity, float offset, float current_time)
	{
		return std::sinf(frequency * current_time) * intensity + offset;
	}
}