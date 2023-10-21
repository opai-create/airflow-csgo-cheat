#pragma once

class c_world_modulation
{
private:
	bool old_sunset_mode = false;
	float old_max_shadow_dist{}, old_rot_x{}, old_rot_y{}, old_rot_z{};

	std::string old_sky_name{};
	std::string ingame_sky_name{};

public:
	INLINE void reset(bool connect = true)
	{
		old_sunset_mode = false;
		old_max_shadow_dist = 0.f;
		old_rot_x = -1.f;
		old_rot_y = -1.f;
		old_rot_z = -1.f;

		old_sky_name = "";
		ingame_sky_name = "";

		if (connect)
			update_real_sky_name();
	}	

	INLINE void update_real_sky_name()
	{
		ingame_sky_name = HACKS->convars.sv_skyname->get_string();
	}

	void override_shadows();
	void override_sky_convar();
	void override_prop_color();
	void override_world_color(i_material* material, float* r, float* g, float* b);
};

#ifdef _DEBUG
inline auto WORLD_MODULATION = std::make_unique<c_world_modulation>();
#else
CREATE_DUMMY_PTR(c_world_modulation);
DECLARE_XORED_PTR(c_world_modulation, GET_XOR_KEYUI32);

#define WORLD_MODULATION XORED_PTR(c_world_modulation)
#endif