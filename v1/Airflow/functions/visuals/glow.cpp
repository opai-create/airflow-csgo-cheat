#include "glow.h"
#include "../config_vars.h"

void c_glow_esp::on_post_screen_effects()
{
	if (!g_ctx.in_game || !g_ctx.local)
		return;

	auto& enemy_esp = g_cfg.visuals.esp[esp_enemy];
	auto& weapon_esp = g_cfg.visuals.esp[esp_weapon];

	for (int i = 0; i < interfaces::glow_object_manager->glow_objects.count(); i++)
	{
		auto object = &interfaces::glow_object_manager->glow_objects[i];
		if (!object->entity || object->is_unused())
			continue;

		auto is_projectile = [&](int class_id)
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
		};

		auto draw_glow = [&](c_float_color& clr, bool ignore = false)
		{
			object->color = vector3d(clr[0], clr[1], clr[2]);

			object->alpha = clr[3];

			if (ignore)
				object->occlued_render = true;

			object->unocclued_render = false;
		};

		if (object->entity->is_player())
		{
			c_csplayer* player = (c_csplayer*)object->entity;

			if (!player->is_alive() || player->dormant())
				continue;

			if (g_ctx.local == player)
			{
				if (g_ctx.local->is_alive() && g_cfg.visuals.local_glow)
					draw_glow(g_cfg.visuals.local_glow_color, true);

				continue;
			}

			if (g_ctx.local->team() == player->team())
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
