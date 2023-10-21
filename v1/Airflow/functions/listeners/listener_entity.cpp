#include "listener_entity.h"
#include "listener_entity.h"

#include "../visuals/esp_store.h"
#include "../features.h"

/*
  implementation from https://github.com/EternityX/DEADCELL-CSGO/blob/master/csgo/features/entity%20listener/ent_listener.cpp
*/

bool should_delete_entity(int index, std::vector< listened_entity_t >& info)
{
	const auto it = std::find_if(info.begin(), info.end(), [&](const listened_entity_t& data) { return data.idx == index; });

	if (it == info.end())
		return false;

	info.erase(it);
	return true;
}

const std::vector< listened_entity_t >& c_listener_entity::get_entity(int type)
{
	return this->ent_lists[type];
}

void c_listener_entity::on_entity_created(c_baseentity* entity)
{
	if (!entity)
		return;

	int index = entity->index();
	c_client_class* client_class = entity->get_client_class();
	int class_id = client_class->class_id;

	//printf("%d -> %s\n", class_id, client_class->network_name);

	if (class_id == CCSPlayerResource)
		interfaces::player_resource = (c_player_resource*)entity;

	auto get_entity_type = [&]()
	{
		if (class_id == CPlantedC4)
			return ent_c4;

		if (main_utils::is_weapon(entity, class_id))
			return ent_weapon;

		if (entity->is_player())
			return ent_player;

		if (class_id == CFogController)
			return ent_fog;

		if (class_id == CEnvTonemapController)
			return ent_tonemap;

		if (class_id == CSprite)
			return ent_light;

		if (class_id == CCSRagdoll)
			return ent_ragdoll;

		return ent_invalid;
	};

	int type = get_entity_type();
	if (type == ent_invalid)
		return;

	this->ent_lists[type].emplace_back(entity);
}

void c_listener_entity::on_entity_deleted(c_baseentity* entity)
{
	if (!entity)
		return;

	int index = entity->index();
	if (index < 0)
		return;

	if (should_delete_entity(index, this->ent_lists[ent_player]))
		g_esp_store->reset_player_info(index);

	if (should_delete_entity(index, this->ent_lists[ent_weapon]))
	{
		g_esp_store->reset_weapon_info(index);
		g_grenade_warning->erase_handle(entity->get_ref_handle());
	}

	for (int i = ent_fog; i < ent_max; i++)
		should_delete_entity(index, this->ent_lists[i]);
}

void c_listener_entity::init_entities()
{
	interfaces::entity_list->add_listener_entity(this);
}

void c_listener_entity::remove_entities()
{
	interfaces::entity_list->remove_listener_entity(this);
}