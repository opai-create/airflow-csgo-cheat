#pragma once
#include "esp_object_render.hpp"

struct weapon_esp_t
{
	bool did_smoke{};
	float smoke_time{};
	float inferno_time{};
	float range_lerp{};

	esp_object_t objects[MAX_ESP_OBJECTS]{};

	INLINE void reset()
	{
		did_smoke = false;
		smoke_time = 0.f;
		inferno_time = 0.f;
		range_lerp = 0.f;

		for (auto& i : objects)
			i.reset();
	}
};

struct esp_dormant_t
{
    memory::bits_t flags{};
    float time{};
    float duck_amount = -1;

    vec3_t origin{};
    vec3_t mins{}, maxs{};

    INLINE void update(c_cs_player* player)
    {
        origin = player->get_abs_origin();
        mins = player->bb_mins();
        maxs = player->bb_maxs();
        flags = player->flags();
        duck_amount = player->duck_amount();

        time = HACKS->global_vars->curtime;
    }

    INLINE void reset()
    {
        flags = 0;
        time = 0.f;
        duck_amount = -1.f;

        origin.reset();
        mins.reset();
        maxs.reset();
    }
};

struct esp_player_t
{
    bool valid = false;
    bool planting = false;

    int health{};
    int ammo{};
    int max_ammo{};

    float duck_amount{};
    float alpha = 1.f;

    vec3_t origin{};
    vec3_t mins{};
    vec3_t maxs{};

    float poses[24]{};

    box_t box{};

    std::string name{};
    std::string weapon_name{};
    std::string weapon_icon{};

    esp_dormant_t dormant{};
    esp_object_t objects[MAX_ESP_OBJECTS]{};

    INLINE void update(c_cs_player* player)
    {
        health = player->health();

        origin = player->get_abs_origin();
        mins = player->bb_mins();
        maxs = player->bb_maxs();
        duck_amount = player->duck_amount();
        player->store_poses(poses);

        name = player->get_name();

        auto weapon = (c_base_combat_weapon*)(HACKS->entity_list->get_client_entity_handle(player->active_weapon()));
        if (weapon)
        {
            weapon_name = weapon->get_weapon_name();
            weapon_icon = (const char*)weapon->get_weapon_icon();

            auto info = HACKS->weapon_system->get_weapon_data(weapon->item_definition_index());
            if (info)
            {
                ammo = weapon->clip1();
                max_ammo = info->max_ammo_1;

                planting = weapon->item_definition_index() == WEAPON_C4 && weapon->started_arming();
            }
        }
    }

    INLINE void reset()
    {
        if (valid)
        {
            valid = false;
            planting = false;

            health = 0;
            ammo = 0;

            alpha = 1.f;
            duck_amount = 0.f;
           // dormant.reset();
            box.reset();
            origin.reset();
            mins.reset();
            maxs.reset();

            name.clear();
            weapon_name.clear();
            weapon_icon.clear();

            std::memset(poses, 0, sizeof(poses));

            for (int i = 0; i < MAX_ESP_OBJECTS; ++i)
                objects[i].reset();
        }
    }
};

class c_esp
{
private:
	weapon_esp_t weapon_esp[4096]{};
    esp_player_t player_esp[65]{};

	void draw_smoke_range(float& weapon_alpha, weapon_esp_t& esp, c_base_entity* entity);
	void draw_molotov_range(float& weapon_alpha, weapon_esp_t& esp, c_base_entity* entity);
	void draw_weapon_esp();
    void render_offscreen_esp(esp_player_t& esp);
	void draw_player_esp();

public:
	INLINE void reset()
	{
		for (auto& i : weapon_esp)
			i.reset();

        for (auto& i : player_esp)
            i.reset();
	}

    INLINE esp_player_t* get_esp_player(int idx)
    {
        return &player_esp[idx];
    }

	INLINE void reset_weapon_info(int idx)
	{
		weapon_esp[idx].reset();
	}

    INLINE void reset_player_info(int idx)
    {
        player_esp[idx].reset();
    }

	void render();
	void render_local();
};

#ifdef _DEBUG
inline auto ESP = std::make_unique<c_esp>();
#else
CREATE_DUMMY_PTR(c_esp);
DECLARE_XORED_PTR(c_esp, GET_XOR_KEYUI32);

#define ESP XORED_PTR(c_esp)
#endif