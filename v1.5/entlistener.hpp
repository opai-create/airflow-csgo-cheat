#pragma once

enum entity_types_t
{
	ENT_INVALID = -1,
	ENT_PLAYER,
	ENT_WEAPON,
	ENT_FOG,
	ENT_TONEMAP,
	ENT_LIGHT,
	ENT_RAGDOLL,
	ENT_C4,
	ENT_MAX
};

struct listened_entity_t
{
	int idx{};
	int class_id{};
	c_base_entity* entity{};

	listened_entity_t() = default;

	INLINE listened_entity_t(c_base_entity* entity) : entity(entity)
	{
		auto client_class = entity->get_client_class();
		if (!client_class)
			return;

		idx = entity->index();
		class_id = client_class->class_id;
	}
};

using listened_entities = std::vector<listened_entity_t>;

class c_listener_entity : public c_entity_listener
{
private:
	listened_entities entities[ENT_MAX]{};
public:
	void on_entity_created(c_base_entity* entity) override;
	void on_entity_deleted(c_base_entity* entity) override;

	INLINE void for_each_player(std::function<void(c_cs_player*)> func, bool enemy = true)
	{
		auto& players = entities[ENT_PLAYER];
		if (players.empty())
			return;

		for (auto& i : players)
		{
			auto player = (c_cs_player*)i.entity;
			if (!player)
				continue;

			if (enemy && (player == HACKS->local || player->is_teammate()))
				continue;

			func(player);
		}
	}

	INLINE void for_each_entity(std::function<void(c_base_entity*)> func, int type)
	{
		auto& ents = entities[type];
		if (ents.empty())
			return;

		for (auto& i : ents)
		{
			if (!i.entity)
				continue;

			func(i.entity);
		}
	}

	INLINE listened_entities get_entity(int type)
	{
		return entities[type];
	}

	INLINE void reset()
	{
		for (auto& i : entities)
			i.clear();
	}

	INLINE void init()
	{
		HACKS->entity_list->add_listener(this);
	}

	INLINE void remove()
	{
		HACKS->entity_list->remove_listener(this);
	}
};

#ifdef _DEBUG
inline auto LISTENER_ENTITY = std::make_unique<c_listener_entity>();
#else
CREATE_DUMMY_PTR(c_listener_entity);
DECLARE_XORED_PTR(c_listener_entity, GET_XOR_KEYUI32);

#define LISTENER_ENTITY XORED_PTR(c_listener_entity)
#endif