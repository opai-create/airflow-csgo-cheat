#include "globals.hpp"
#include "glow.hpp"

void c_glow::run()
{
	if (!HACKS->in_game || !HACKS->local)
		return;

	auto& enemy_esp = g_cfg.visuals.esp[esp_enemy];
	auto& weapon_esp = g_cfg.visuals.esp[esp_weapon];

	for (int i = 0; i < HACKS->glow_object_manager->glow_objects.count(); i++)
	{
		auto object = &HACKS->glow_object_manager->glow_objects[i];
		if (!object->entity || object->is_unused())
			continue;

		auto is_projectile = [&](int class_id)
		{
			switch (class_id)
			{
			case CBaseCSGrenadeProjectile:
#ifndef LEGACY
			case CBreachChargeProjectile:
			case CBumpMineProjectile:
			case CSnowballProjectile:
#endif
			case CDecoyProjectile:
			case CMolotovProjectile:
			case CSensorGrenadeProjectile:
			case CSmokeGrenadeProjectile:
			case CInferno:
				return true;
				break;
			}
			return false;
		};

		auto draw_glow = [&](c_float_color& clr, bool ignore = false)
		{
			object->color = vec3_t{ clr[0], clr[1], clr[2] };
			object->alpha = clr[3];

			if (ignore)
				object->occlued_render = true;

			object->unocclued_render = false;
		};

		if (object->entity->is_player())
		{
			auto player = (c_cs_player*)object->entity;

			if (!player->is_alive() || player->dormant())
				continue;

			if (HACKS->local == player)
			{
				if (HACKS->local->is_alive() && g_cfg.visuals.local_glow)
					draw_glow(g_cfg.visuals.local_glow_color, true);

				continue;
			}

			if (player->is_teammate(false))
				continue;

			if (enemy_esp.enable && (enemy_esp.elements & 256))
				draw_glow(enemy_esp.colors.glow, true);
		}

		auto class_id = object->entity->get_client_class()->class_id;
		if ((object->entity->is_weapon() || class_id == CC4 || class_id == CPlantedC4 || is_projectile(class_id)))
		{
			object->occlued_render = weapon_esp.enable && (weapon_esp.elements & 64);
			draw_glow(weapon_esp.colors.glow);
		}
	}
}