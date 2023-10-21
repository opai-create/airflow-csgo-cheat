#include "globals.hpp"
#include "remove_projectiles.hpp"

void c_remove_projectiles::remove_smoke()
{
	if (!mat1)
		mat1 = HACKS->material_system->find_material(CXOR("particle/vistasmokev1/vistasmokev1_smokegrenade"), CXOR("Other textures"));

	if (!mat2)
		mat2 = HACKS->material_system->find_material(CXOR("particle/vistasmokev1/vistasmokev1_emods"), CXOR("Other textures"));

	if (!mat3)
		mat3 = HACKS->material_system->find_material(CXOR("particle/vistasmokev1/vistasmokev1_emods_impactdust"), CXOR("Other textures"));

	if (!mat4)
		mat4 = HACKS->material_system->find_material(CXOR("particle/vistasmokev1/vistasmokev1_fire"), CXOR("Other textures"));

	bool state = (g_cfg.misc.removals & smoke);
	if (override_smoke != state)
	{
		mat1->set_material_var_flag(MATERIAL_VAR_NO_DRAW, state);
		mat2->set_material_var_flag(MATERIAL_VAR_NO_DRAW, state);
		mat3->set_material_var_flag(MATERIAL_VAR_NO_DRAW, state);
		mat4->set_material_var_flag(MATERIAL_VAR_NO_DRAW, state);

		override_smoke = state;
	}

	if (state)
		**offsets::remove_smoke.cast<int**>() = 0;
}