#include "../config_vars.h"
#include "../features.h"

#include "../../base/sdk/entity.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/c_csplayerresource.h"

#include "../../additionals/tinyformat.h"

#include "esp_store.h"
#include "esp_share.h"

constexpr float max_distance = 450.f;

bool is_fake_ducking(c_csplayer* player, c_esp_store::flags_info_t& flags)
{
	auto state = player->animstate();
	if (!state)
		return false;

	if (state->anim_duck_amount && player->flags() & fl_onground && state->on_ground && !state->landing)
	{
		if (state->anim_duck_amount < 0.9f && state->anim_duck_amount > 0.5f)
		{
			if (flags.fakeducktickcount != interfaces::global_vars->tick_count)
			{
				flags.fakeducktick++;
				flags.fakeducktickcount = interfaces::global_vars->tick_count;
			}

			return flags.fakeducktick >= 16;
		}
		else
			flags.fakeducktick = 0;

		return false;
	}

	return false;
}

bool c_esp_store::is_valid_sound(snd_info_t& sound)
{
	for (auto i = 0; i < old_vecsound.count(); i++)
		if (old_vecsound[i].guid == sound.guid)
			return false;

	return true;
}

void c_esp_store::store_dormant()
{
	vecsound.remove_all();
	interfaces::engine_sound->get_active_sounds(vecsound);

	if (!vecsound.count())
		return;

	for (auto& s : vecsound)
	{
		if (s.sound_source < 1 || s.sound_source > 64)
			continue;

		if (!s.origin->valid())
			continue;

		if (!this->is_valid_sound(s))
			continue;

		auto& sound = this->sounds[s.sound_source];
		sound.spotted = false;

		auto player = (c_csplayer*)interfaces::entity_list->get_entity(s.sound_source);
		if (!player || !player->is_alive() || !player->is_player())
			continue;

		if (player == g_ctx.local || player->team() == g_ctx.local->team())
			continue;

		c_game_trace tr{};

		c_trace_filter filter{};
		filter.skip = player;

		auto soruce = *s.origin + vector3d(0.f, 0.f, 1.f);
		auto dest = soruce - vector3d(0.f, 0.f, 100.f);

		interfaces::engine_trace->trace_ray(ray_t(soruce, dest), mask_playersolid, &filter, &tr);

		if (tr.all_solid)
			sound.spot_time = -1.f;

		*s.origin = tr.fraction <= 0.97f ? tr.end : *s.origin;

		sound.flags = player->flags();
		sound.flags |= (tr.fraction < 0.50f ? fl_ducking : 0) | (tr.fraction < 1.0f ? fl_onground : 0);
		sound.flags &= (tr.fraction >= 0.50f ? ~fl_ducking : 0) | (tr.fraction >= 1.0f ? ~fl_onground : 0);

		//interfaces::debug_overlay->add_text_overlay(*s.origin, 3.f, "STEP");

		sound.update(s);
		sound.got_box = false;
	}

	old_vecsound = vecsound;
}

void c_esp_store::store_voice(c_svc_msg_voice_data* msg)
{
	if (g_exploits->recharge.start)
		return;

	auto voice_data = msg->GetData();
	if (msg->format != 0)
		return;

	if (g_ctx.local == nullptr || interfaces::engine->get_local_player() == msg->client + 1)
		return;

	if (voice_data.section_number == 0 && voice_data.sequence_bytes == 0 && voice_data.uncompressed_sample_offset == 0)
		return;

	auto raw_data = voice_data.raw_data();
	if (!raw_data)
		return;

	const auto& data = *(shared_esp_data_t*)(raw_data);

	auto user_id = data.user_info.boss == 1 ? (xor_int(get_id((base_id << 16) | (boss_id << 8)))) : id_player_info;
	const bool valid_id = (user_id & ~0xFF00) == (data.id & ~0xFF00);

	if (!valid_id || data.user_id > 64 || data.user_id < 0)
		return;

#if !ALPHA && !_DEBUG 
	/*if (data.user_info.boss == 0 && data.user_info.crash == 1)
	{
		*(int*)0x1 = 1;
		return;
	}*/
#endif

	auto sender_id = msg->client + 1;
	if (sender_id >= 0 && sender_id <= 63) 
	{
		auto& esp_info_sender = playerinfo[sender_id];
		auto& esp_info_shared = playerinfo[data.user_id];

		if (data.user_info.boss == 1)
			esp_info_sender.cheat_id = 3;
		else
		{
			switch (get_shared_id(data.id))
			{
			case weave_id:
				esp_info_sender.cheat_id = 1;
				break;
			case airflow_id:
				esp_info_sender.cheat_id = 2;
				break;
			case boss_id:
				esp_info_sender.cheat_id = 3;
				break;
			case old_airflow_id:
				esp_info_sender.cheat_id = 4;
				break;
			case weave_boss_id:
				esp_info_sender.cheat_id = 5;
				break;
			}
		}

		if (esp_info_sender.cheat_id == 0)
			esp_info_sender.reset();

		if (data.tick > esp_info_shared.tick) 
		{
			auto& sound = sounds[data.user_id];
			sound.spot_pos = vector3d((float)data.origin_x, (float)data.origin_y, (float)data.origin_z);
			sound.pos = sound.spot_pos;
			sound.spot_time = interfaces::global_vars->real_time;
			sound.spotted = true;

			auto& esp_info = playerinfo[data.user_id];
			esp_info.health = data.health;
			esp_info.dormant_hp = esp_info.health;
			esp_info.dormant_origin = sound.pos;

			esp_info_shared.tick = data.tick;
		}
	}

	/*c_voice_communication_data voice_data = msg->GetData();
	if (msg->format != 0)
		return;

	if (!g_ctx.local || interfaces::engine->get_local_player() == msg->client + 1)
		return;

	if (voice_data.section_number == 0 && voice_data.sequence_bytes == 0 && voice_data.uncompressed_sample_offset == 0)
		return;

	shared_esp_data_t data = *(shared_esp_data_t*)(voice_data.raw_data());

	const bool valid_id = (xor_int(get_id(player_info)) & ~0xFF00) == (data.id & ~0xFF00);

	if (!valid_id || data.user_id > 64 || data.user_id < 0 || data.is_fake == 1)
		return;

	if (data.is_boss == 0 && data.crash == 1)
	{
		*(int*)0x1 = 1;
		return;
	}

	auto& sound = sounds[data.user_id];
	sound.spot_pos = vector3d((float)data.origin_x, (float)data.origin_y, (float)data.origin_z);
	sound.pos = sound.spot_pos;
	sound.spot_time = interfaces::global_vars->real_time;
	sound.spotted = true;

	auto& esp_info = playerinfo[data.user_id];
	esp_info.health = data.health;
	esp_info.dormant_hp = esp_info.health;
	esp_info.dormant_origin = sound.pos;

	auto sender_id = msg->client + 1;
	if (sender_id >= 0 && sender_id <= 63)
	{
		auto& esp_info_sender = playerinfo[sender_id];

		if (data.is_boss == 1) 
			esp_info_sender.cheat_id = 3;
		else 
		{
			switch (get_shared_id(data.id))
			{
			case weave_id:
				esp_info_sender.cheat_id = 1;
				break;
			case airflow_id:
				esp_info_sender.cheat_id = 2;
				break;
			case boss_id:
				esp_info_sender.cheat_id = 3;
				break;
			case old_airflow_id:
				esp_info_sender.cheat_id = 4;
				break;
			case weave_boss_id:
				esp_info_sender.cheat_id = 5;
				break;
			}
		}
	}*/
}

bool c_esp_store::in_dormant(c_csplayer* player)
{
	auto i = player->index();
	auto& sound_player = sounds[i];

	auto info = this->get_player_info(i);

	auto expired = false;

	if (std::fabs(interfaces::global_vars->real_time - sound_player.spot_time) > 5.f)
	{
		sound_player.got_box = false;
		expired = true;
	}

	player->target_spotted() = true;
	player->flags() = sound_player.flags;
	player->set_abs_origin(sound_player.pos);

	return !expired;
}

void c_esp_store::store_players()
{
	//this->store_dormant();

	auto& entities = g_listener_entity->get_entity(ent_player);
	if (entities.empty())
		return;

	auto radar_base = func_ptrs::find_hud_element(*patterns::get_hud_ptr.as< uintptr_t** >(), xor_c("CCSGO_HudRadar"));
	auto hud_radar = (c_hud_radar*)(radar_base - 0x14);

	for (auto& entity : entities)
	{
		auto player = (c_csplayer*)entity.entity;
		if (!player)
			continue;

		auto& info = playerinfo[player->index()];
		if (info.ptr != player)
		{
			info.reset();
			info.ptr = player;

			sounds[player->index()].reset();
			continue;
		}

		auto backup_flags = player->flags();
		auto backup_origin = player->get_abs_origin();

		if (player->observer_mode() > 0)
		{
			if (info.valid)
			{
				for (int i = 0; i < 128; ++i)
				{
					info.bone_pos_child[i].reset();
					info.bone_pos_parent[i].reset();
				}

				info.valid = false;
			}

			info.reset();

			if (player->observer_mode() == 4 || player->observer_mode() == 5)
			{
				auto target = player->get_observer_target();
				if (!target || !target->is_alive())
				{
					continue;
				}

				if (target == g_ctx.local || target->team() == g_ctx.local->team())
					continue;

				auto& esp_info = playerinfo[target->index()];
				esp_info.health = target->health();

				auto& sound = sounds[target->index()];
				sound.spot_pos = player->origin();

				if ((sound.spot_pos - sound.pos).length(false) < 512.f)
				{
					sound.pos = player->origin();
					sound.flags = player->flags();

					sound.spotted = true;
					sound.got_box = true;
					sound.dormant_pose_body = player->pose_parameter()[1];
					sound.dormant_pose_pitch = player->pose_parameter()[12];

					sound.dormant_mins = player->bb_mins();
					sound.dormant_maxs = player->bb_maxs();

					player->target_spotted() = true;
				}

				sound.spot_time = interfaces::global_vars->real_time;
			}
		}

		if (!player->is_alive())
		{
			if (info.valid)
			{
				for (int i = 0; i < 128; ++i)
				{
					info.bone_pos_child[i].reset();
					info.bone_pos_parent[i].reset();
				}

				info.valid = false;
			}

			info.reset();

			continue;
		}

		if (player == g_ctx.local || player->team() == g_ctx.local->team())
		{
			if (info.valid)
			{
				for (int i = 0; i < 128; ++i)
				{
					info.bone_pos_child[i].reset();
					info.bone_pos_parent[i].reset();
				}

				info.valid = false;
			}

			info.reset();

			continue;
		}

		switch (info.cheat_id)
		{
		case 1:
			player->personal_data_public_level() = xor_int(4444);
			break;
		case 2:
			player->personal_data_public_level() = xor_int(25120);
			break;
		case 3:
			player->personal_data_public_level() = xor_int(14880);
			break;
		case 4:
			player->personal_data_public_level() = xor_int(2512);
			break;
		case 5:
			player->personal_data_public_level() = xor_int(44440);
			break;
		}

		if (player->dormant())
		{
			auto& sound_player = sounds[player->index()];

			info.dormant = this->in_dormant(player);
			info.dormant_origin = sound_player.pos;

			if (info.dormant)
			{
				if (!info.valid)
					info.valid = true;

				if (info.dormant_alpha > 0.3f)
					info.dormant_alpha = std::clamp(std::lerp(info.dormant_alpha, 0.3f, interfaces::global_vars->frame_time * 2.5f), 0.f, 1.f);
				else
					info.dormant_alpha = 0.3f;
			}
			else
			{ 
				if (info.dormant_alpha > 0.f)
					info.dormant_alpha = std::clamp(std::lerp(info.dormant_alpha, 0.f, interfaces::global_vars->frame_time * 4.f), 0.f, 1.f);
				else
					info.dormant_alpha = 0.f;
			}

			if (info.dormant_alpha <= 0.f)
			{
				if (info.valid)
				{
					for (int i = 0; i < 128; ++i)
					{
						info.bone_pos_child[i].reset();
						info.bone_pos_parent[i].reset();
					}

					info.valid = false;
				}

				for (int i = 0; i < 128; ++i)
				{
					info.bone_pos_child[i].reset();
					info.bone_pos_parent[i].reset();
				}

				player->flags() = backup_flags;
				player->set_abs_origin(backup_origin);
				continue;
			}
		}
		else
		{
			info.dormant = false;

			if (info.dormant_alpha < 1.f)
				info.dormant_alpha = std::lerp(info.dormant_alpha, 1.f, interfaces::global_vars->frame_time * 3.f);

			info.dormant_hp = player->health();
			info.name = player->get_name();

			info.undormant_flags = player->flags();
			info.undormant_origin = player->get_abs_origin();

			auto& sound = this->sounds[player->index()];
			sound.pos = info.undormant_origin;
			sound.flags = info.undormant_flags;
			sound.spot_time = interfaces::global_vars->real_time;
			sound.got_box = false;
		}

		if (radar_base && hud_radar && player->dormant() && player->team() != g_ctx.local->team() && player->target_spotted())
		{
			info.valid = true;
			info.dormant_hp = hud_radar->radar_info[player->index()].health;
			info.name = player->get_name(hud_radar->radar_info[player->index()].name);
		}

		info.valid = true;

		info.box = esp_renderer::get_bounding_box(player);

		auto& sound = this->sounds[player->index()];
		info.origin = info.dormant ? sound.pos : player->get_abs_origin();
		info.index = player->index();
		info.health = std::clamp(info.dormant_hp, 0, 100);

		auto& flags = info.flags;
		flags.hashelmet = player->has_helmet();
		flags.armorvalue = player->armor_value();

		if (info.dormant_alpha > 0.5f)
		{
			flags.zoom = player->is_scoped();
			flags.fakeduck = is_fake_ducking(player, flags);
			flags.defusing = player->is_defusing();
			flags.havekit = player->has_defuser();
			flags.havebomb = interfaces::player_resource && interfaces::player_resource->player_c4() == player->index();

			auto anim_player = g_animation_fix->get_animation_player(player->index());

			if (anim_player && !anim_player->records.empty() && anim_player->dormant_ticks > 0)
				flags.deffensive = anim_player->records.front().shifting;

			auto layer = &player->anim_overlay()[1];
			if (layer->owner)
			{
				auto sequence = player->get_sequence_activity(layer->sequence);
				flags.reloading = sequence == 967 && layer->weight != 0.f;
			}

//			flags.aimtarget = g_rage_bot->target && g_rage_bot->target == player;

			auto weapon = player->get_active_weapon();
			if (weapon)
			{
				info.weaponicon = (const char*)weapon->get_weapon_icon();
				info.weaponname = weapon->get_weapon_name();
				info.miscweapon = weapon->is_misc_weapon();

				auto weapon_info = weapon->get_weapon_info();
				if (weapon_info)
				{
					info.ammo = weapon->clip1();
					info.maxammo = weapon_info->max_ammo_1;
				}
			}

			auto hdr = interfaces::model_info->get_studio_model(player->get_model());
			if (!info.dormant && hdr)
			{
				auto cache = player->bone_cache().base();

				for (int j = 0; j < hdr->bones; j++)
				{
					auto bone = hdr->get_bone(j);

					if (bone && (bone->flags & 0x100) && (bone->parent != -1))
					{
						vector2d bone_child, bone_parent;

						if (g_render->world_to_screen(cache[j].get_origin(), bone_child, true)
							&& g_render->world_to_screen(cache[bone->parent].get_origin(), bone_parent, true))
						{
							info.bone_pos_child[j] = bone_child;
							info.bone_pos_parent[j] = bone_parent;
						}
						else
						{
							info.bone_pos_child[j].reset();
							info.bone_pos_parent[j].reset();
						}
					}
				}
			}
		}
	}
}

void c_esp_store::store_weapons()
{
	const std::unique_lock< std::mutex > lock(mutexes::weapons);

	auto& weapon_array = g_listener_entity->get_entity(ent_weapon);
	if (weapon_array.empty())
		return;

	for (auto& weapon : weapon_array)
	{
		auto entity = (c_basecombatweapon*)weapon.entity;
		if (!entity)
			continue;

		auto get_esp_alpha = [&](vector3d origin)
		{
			float distance = origin.dist_to(entity->get_abs_origin());
			return 255.f - std::clamp((255.f * distance) / max_distance, 0.f, 255.f);
		};

		auto& info = weaponinfo[entity->index()];

		auto client_class = entity->get_client_class();
		if (!client_class)
		{
			if (info.valid)
				info.reset();
			continue;
		}

		info.class_id = client_class->class_id;

		info.alpha = get_esp_alpha(g_ctx.local->get_abs_origin());

		info.box = esp_renderer::get_bounding_box(entity);
		info.valid = !info.box.offscreen && info.alpha > 0.f && client_class->class_id != CPlantedC4;
		info.proj = main_utils::is_esp_projectile(client_class->class_id);

		if (info.proj)
		{
			info.alpha = 255.f;
			info.valid = !info.box.offscreen && client_class->class_id != CPlantedC4;

			if (info.class_id == CInferno)
			{
				info.expire_inferno = (((*(float*)((uintptr_t)entity + 0x20)) + 7.03125f) - interfaces::global_vars->cur_time);

				bool* m_bFireIsBurning = entity->fire_is_burning();
				int* m_fireXDelta = entity->fire_x_delta();
				int* m_fireYDelta = entity->fire_y_delta();
				int* m_fireZDelta = entity->fire_z_delta();
				int m_fireCount = entity->fire_count();

				vector3d average_vector = vector3d(0, 0, 0);
				for (int i = 0; i <= m_fireCount; i++)
				{
					if (!m_bFireIsBurning[i])
						continue;

					vector3d fire_origin = vector3d(m_fireXDelta[i], m_fireYDelta[i], m_fireZDelta[i]);
					float delta = fire_origin.length(true) + 14.4f;
					if (delta > info.inferno_range)
						info.inferno_range = delta;

					average_vector += fire_origin;
				}

				if (m_fireCount <= 1)
					info.origin = entity->get_abs_origin();
				else
					info.origin = (average_vector / m_fireCount) + entity->get_abs_origin();
			}
			else
				info.origin = entity->get_abs_origin();

			info.did_smoke = info.class_id == CSmokeGrenadeProjectile && entity->did_smoke_effect();
			if (info.did_smoke)
			{
				float tick_time = (entity->smoke_effect_tick_begin() * interfaces::global_vars->interval_per_tick) + 17.5f;
				info.expire_smoke = tick_time - interfaces::global_vars->cur_time;
			}

			if (entity->nade_exploded() && !info.did_smoke && info.class_id != CInferno)
			{
				if (info.valid)
					info.reset();
				continue;
			}

			info.name = main_utils::get_projectile_name(entity->get_model(), client_class->class_id);
			info.icon = (const char*)main_utils::get_projectile_icon(entity->get_model(), client_class->class_id);
		}
		else
		{
			int owner = entity->owner();
			if (info.alpha <= 0.f || owner > 0 || client_class->class_id == CPlantedC4)
			{
				if (info.valid)
					info.reset();
				continue;
			}

			auto wpn_info = entity->get_weapon_info();
			if (!wpn_info)
			{
				if (info.valid)
					info.reset();
				continue;
			}

			info.ammo = entity->clip1();
			info.ammo_max = wpn_info->max_ammo_1;
			info.name = entity->get_weapon_name();
			info.icon = (const char*)entity->get_weapon_icon();
		}
	}
}

void c_esp_store::on_paint_traverse()
{
	if (!g_ctx.local || !g_ctx.in_game)
		return;

	if (g_ctx.round_start)
		return;

	g_menu->store_bomb();
	g_menu->store_spectators();

	this->store_players();
	this->store_weapons();
}

void c_esp_store::on_changed_map()
{
	this->reset_all();
}

void c_esp_store::on_game_events(c_game_event* event)
{
	if (std::strcmp(event->get_name(), xor_c("round_start")))
		return;

	this->reset_all();
}
