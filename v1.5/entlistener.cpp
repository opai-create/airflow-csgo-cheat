#include "globals.hpp"
#include "entlistener.hpp"
#include "esp_object_render.hpp"
#include "esp.hpp"
#include "grenade_prediction.hpp"

INLINE bool erase_entity(int index, listened_entities& entities) 
{
	const auto it = std::find_if(entities.begin(), entities.end(), [&](const listened_entity_t& entity)
		{ return entity.idx == index; });

	if (it == entities.end())
		return false;

	entities.erase(it);
	return true;
}

void c_listener_entity::on_entity_created(c_base_entity* entity) 
{
	auto client_class = entity->get_client_class();
	if (!client_class)
		return;

	auto class_id = client_class->class_id;
	if (class_id == CCSPlayerResource)
		HACKS->player_resource = (c_cs_player_resource*)entity;

	int index = entity->index();
	if (index < 0)
		return;

	auto get_entity_type = [&]() 
	{
		if (entity->is_player())
			return ENT_PLAYER;

		if (entity->is_weapon() || entity->is_grenade(class_id) || entity->is_bomb(class_id))
			return ENT_WEAPON;

		if (class_id == CFogController)
			return ENT_FOG;

		if (class_id == CEnvTonemapController)
			return ENT_TONEMAP;

		if (class_id == CSprite)
			return ENT_LIGHT;

		if (class_id == CCSRagdoll)
			return ENT_RAGDOLL;

		return ENT_INVALID;
	};

	int type = get_entity_type();
	if (type == ENT_INVALID)
		return;

	entities[type].emplace_back(entity);
}

void c_listener_entity::on_entity_deleted(c_base_entity* entity)
{
	int index = entity->index();
	if (index < 0)
		return;

	if (erase_entity(index, entities[ENT_PLAYER]))
		ESP->reset_player_info(index);

	if (erase_entity(index, entities[ENT_WEAPON]))
	{
		ESP->reset_weapon_info(index);
		GRENADE_PREDICTION->erase_handle(entity->get_ref_handle());
	}

	for (int i = ENT_FOG; i < ENT_MAX; ++i)
		erase_entity(index, entities[i]);
}