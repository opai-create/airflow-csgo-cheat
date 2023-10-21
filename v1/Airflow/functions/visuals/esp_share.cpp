#include "../features.h"
#include "esp_store.h"
#include "esp_share.h"

namespace esp_share
{
	void send_data_msg(c_voice_communication_data* data)
	{
		c_clc_msg_voice_data msg;
		std::memset(&msg, 0, sizeof(msg));

		patterns::construct_voice_data_message.as< uint32_t(__fastcall*)(void*, void*) >()(&msg, nullptr);

		msg.set_data(data);
		communication_string_t comm_str{};

		msg.data = (uintptr_t)&comm_str;
		msg.format = 0; // VoiceFormat_Steam
		msg.flags = 63; // All flags

		auto net_chan = interfaces::engine->get_net_channel_info();
		if (net_chan && !net_chan->is_loopback())
		{
			auto cs_net_chan = interfaces::client_state->net_channel_ptr;
			if (cs_net_chan)
				cs_net_chan->send_net_msg((uintptr_t)&msg, false, true);
		}

		patterns::destruct_voice_data_message.as< uint32_t(__fastcall*)(void*) >()(&msg);
	}

	void send_shared_esp_data(const std::vector<uint8_t>& voice)
	{
		c_voice_communication_data new_data{};
		std::memcpy(new_data.raw_data(), voice.data(), voice.size());
		send_data_msg(&new_data);
	}

	void send_shared_esp_data(broken_shared_esp_data_t data)
	{
		c_voice_communication_data new_data{};
		std::memcpy(new_data.raw_data(), &data, sizeof(data));
		send_data_msg(&new_data);
	}

	void share_player_position(c_csplayer* player)
	{
	/*	if (g_exploits->recharge)
			return;*/

		{
			shared_esp_data_t data{};
			if (player == g_ctx.local)
				data.user_info.boss = (uint8_t)g_ctx.is_boss;
			else
				data.user_info.boss = 0;

			if (data.user_info.boss == 1)
				data.id = xor_int(get_id((base_id << 16) | (boss_id << 8)));
			else
				data.id = id_player_info;

			const vector3d origin = player->origin();
			data.origin_x = (int16_t)origin.x;
			data.origin_y = (int16_t)origin.y;
			data.origin_z = (int16_t)origin.z;
			data.user_id = player->index();
			data.health = player->health();
			data.tick = interfaces::global_vars->tick_count & 0xFFFF;

			data.user_info.id = 0;
			data.user_info.boss = 0;

#if ALPHA
			data.user_info.crash = (uint8_t)g_cfg.misc.force_crash;
#else
			data.user_info.crash = 0;
#endif

			std::vector<uint8_t> bytes{};
			bytes.resize(sizeof(data));
			std::memcpy(bytes.data(), &data, sizeof(data));
			send_shared_esp_data(bytes);
		}

			/*shared_esp_data_t data{};
		

			if (data.is_boss == 1)
				data.id = xor_int(get_id((base_id << 16) | (boss_id << 8)));
			else
				data.id = id_player_info;

			const vector3d origin = player->origin();
			data.origin_x = (int16_t)origin.x;
			data.origin_y = (int16_t)origin.y;
			data.origin_z = (int16_t)origin.z;
			data.user_id = player->index();
			data.health = player->health();
			data.is_fake = 0;
			data.crash = g_cfg.misc.force_crash;

			send_shared_esp_data(data);*/
	//	}

		// fuck crackers.
		//broken_shared_esp_data_t data{};
		//data.id = xor_int(get_id((base_id << 16) | (old_airflow_id << 8)));
		//data.is_fake = 1;
		//data.is_boss = 0;
		//data.user_id = player->index();

		//const vector3d origin = player->origin();
		//data.origin_x = INT_MAX;
		//data.origin_y = INT_MAX;
		//data.origin_z = INT_MAX;
		//data.health = INT_MAX;

		//send_shared_esp_data(data);
	}
}