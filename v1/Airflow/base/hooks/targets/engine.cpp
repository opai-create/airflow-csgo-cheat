#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../functions/config_vars.h"

#include "../../../base/tools/render.h"
#include "../../../base/sdk/entity.h"
#include "../../../functions/features.h"
#include "../../../functions/extra/cmd_shift.h"

#define max_coord_float (16384.0f)
#define min_coord_float (-max_coord_float)

namespace tr
{
	namespace client_state
	{
		void __fastcall packet_start(void* ecx, void* edx, int incoming, int outgoing)
		{
			static auto original = vtables[vmt_client_state_].original<decltype(&packet_start)>(xor_int(5));

			auto shifting = cmd_shift::shifting || g_exploits->cl_move.trigger && g_exploits->cl_move.shifting;

			if (!g_ctx.local || !g_ctx.local->is_alive() || shifting)
			{
				if (!g_fake_lag->commands.empty())
					g_fake_lag->commands.clear();

				return original(ecx, edx, incoming, outgoing);
			}

			if (g_fake_lag->commands.empty())
				return;

			if (std::find(g_fake_lag->commands.begin(), g_fake_lag->commands.end(), outgoing) != g_fake_lag->commands.end())
				original(ecx, edx, incoming, outgoing);

			std::erase_if(g_fake_lag->commands, [&](int command)
				{
					return std::abs(command - outgoing) >= 150
						|| command < outgoing;
				});
		}

		void __fastcall packet_end(void* ecx, void* edx)
		{
			static auto original = vtables[vmt_client_state_].original<decltype(&packet_end)>(xor_int(6));
			if (!g_ctx.local || !g_ctx.local->is_alive())
				return original(ecx, edx);

			auto clientstate = (c_clientstate*)ecx;

			if (interfaces::client_state->clock_drift_mgr.server_tick == interfaces::client_state->delta_tick)
			{
				g_engine_prediction->net_compress_apply(clientstate->last_outgoing_command);

				auto ack_cmd = clientstate->last_command_ack;

				auto correct = std::find_if(g_fake_lag->choked_ticks.begin(), g_fake_lag->choked_ticks.end(), [&ack_cmd](const choked_ticks_t& other_data) { return other_data.cmd == ack_cmd; });

				if (correct != g_fake_lag->choked_ticks.end())
				{
					if (g_ctx.velocity_modifier != g_ctx.local->velocity_modifier())
						g_ctx.velocity_modifier = g_ctx.local->velocity_modifier();
				}
			}

			original(ecx, edx);
		}
	}

	namespace engine
	{
		void __fastcall emit_sound(c_engine_sound* thisptr, uint32_t edx, void* filter, int ent_index, int channel, const char* sound_entry, unsigned int sound_entry_hash,
			const char* sample, float volume, float attenuation, int seed, int flags, int pitch, const vector3d* origin, const vector3d* direction,
			void* vec_origins, bool update_positions, float sound_time, int speaker_entity, int test) {

			static auto original = vtables[vmt_engine_sound].original< decltype(&emit_sound) >(xor_int(5));

			auto call_original = [&]() {
				original(thisptr, edx, filter, ent_index, channel, sound_entry, sound_entry_hash, sample, volume, attenuation, seed, flags,
					pitch, origin, direction, vec_origins, update_positions, sound_time, speaker_entity, test);
			};

			if (g_ctx.is_alive)
			{
				if (g_ctx.in_prediction)
				{
					flags |= 1 << 2;
					call_original();
					return;
				}
			}

			call_original();
		}

		int __fastcall list_leaves_in_box(void* ecx, void* edx, const vector3d& mins, const vector3d& maxs, unsigned short* list, int list_max)
		{
			static auto original = vtables[vmt_engine_bsp].original<decltype(&list_leaves_in_box)>(xor_int(6));

			if (!g_ctx.local || (uintptr_t)_ReturnAddress() != patterns::list_leaves_in_box.as< uintptr_t >())
				return original(ecx, edx, mins, maxs, list, list_max);

			auto info = *(renderable_info_t**)((uintptr_t)_AddressOfReturnAddress() + 0x14);

			if (!info)
				return original(ecx, edx, mins, maxs, list, list_max);

			if (!info->m_pRenderable)
				return original(ecx, edx, mins, maxs, list, list_max);

			auto unknown = info->m_pRenderable->get_i_unknown_entity();
			if (!unknown)
				return original(ecx, edx, mins, maxs, list, list_max);

			c_baseentity* entity = unknown->get_base_entity();

			if (!entity || !entity->is_player())
				return original(ecx, edx, mins, maxs, list, list_max);

			auto player = (c_csplayer*)entity;
			if (!player->is_alive() || player == g_ctx.local || player->team() == g_ctx.local->team())
				return original(ecx, edx, mins, maxs, list, list_max);

			// fuck struct
			// all my homies do hardcode
			// tbh it works better than doing changes in struct vars
			// todo: reclass this bullshit
			*(uint16_t*)((uintptr_t)info + 0x0016) &= ~0x100;
			*(uint16_t*)((uintptr_t)info + 0x0018) |= 0xC0;

			const vector3d map_min = vector3d{ min_coord_float, min_coord_float, min_coord_float };
			const vector3d map_max = vector3d{ max_coord_float, max_coord_float, max_coord_float };

			return original(ecx, edx, map_min, map_max, list, list_max);
		}

		int __fastcall send_datagram(void* netchan, void* edx, void* datagram)
		{
			static auto original = hooker.original(&send_datagram);

			if (netchan != interfaces::client_state->net_channel_ptr || g_ctx.uninject || !g_ctx.in_game || !g_ctx.is_alive || !g_cfg.binds[spike_b].toggled)
				return original(netchan, edx, datagram);

			auto netchan_ptr = (c_netchan*)netchan;
			auto netchannel_info = interfaces::engine->get_net_channel_info();

			float spike_amount = (g_cfg.rage.spike_amt / 1000.f);
			spike_amount -= netchannel_info->get_latency(flow_outgoing);
			spike_amount -= g_ctx.lerp_time;

			float correct = std::clamp(spike_amount, 0.f, g_ctx.max_unlag);

			auto old_sequence = netchan_ptr->in_sequence_nr;
			auto old_state = netchan_ptr->in_reliable_state;

			g_ping_spike->on_net_chan(netchan_ptr, correct);

			auto ret = original(netchan, edx, datagram);

			netchan_ptr->in_sequence_nr = old_sequence;
			netchan_ptr->in_reliable_state = old_state;

			return ret;
		}

		bool __fastcall temp_entities(c_clientstate* ecx, void* edx, void* msg)
		{
			static auto original = hooker.original(&temp_entities);

			auto old_max_clients = ecx->max_clients;
			ecx->max_clients = 1;

			bool ret = original(ecx, edx, msg);

			ecx->max_clients = old_max_clients;

			patterns::cl_fireevents.as<void(*)()>()();

			return ret;
		}

		int __fastcall can_load_third_party_files(void* _this)
		{
			return 1;
		}

		int __stdcall get_unverified_file_hashes(void* _this, void* someclass, int nMaxFiles)
		{
			return 0;
		}

		bool __fastcall send_net_msg(i_net_channel_info* ecx, void* edx, c_net_message& msg, bool force_reliable, bool voice)
		{
			static auto original = hooker.original(&send_net_msg);

			if (ecx != interfaces::engine->get_net_channel_info())
				return original(ecx, edx, msg, force_reliable, voice);

			if (g_cfg.misc.bypass_sv_pure && msg.get_type() == 14)
				return false;

			return original(ecx, edx, msg, force_reliable, voice);
		}

		bool __fastcall using_static_props_debug(void* ecx, void* edx)
		{
			return true;
		}

		float __fastcall get_screen_aspect_ratio(void* ecx, void* edx, int width, int height)
		{
			static auto original = vtables[vmt_engine_].original<decltype(&get_screen_aspect_ratio)>(xor_int(101));
			return g_cfg.misc.aspect_ratio > 0 ? g_cfg.misc.aspect_ratio / 100.f : original(ecx, edx, width, height);
		}

		void __fastcall check_file_crc_with_server(void* ecx, void* edx)
		{
			static auto original = hooker.original(&check_file_crc_with_server);
			return original(ecx, edx);
		}

		bool __fastcall is_connected(void* ecx, void* edx)
		{
			static auto original = vtables[vmt_engine_].original<decltype(&is_connected)>(xor_int(27));

			if (g_cfg.misc.unlock_inventory && (uintptr_t)_ReturnAddress() == patterns::return_addr_loadout_allowed.as< uintptr_t >())
				return false;

			return original(ecx, edx);
		}

		bool __fastcall is_paused(void* ecx, void* edx)
		{
			static auto original = vtables[vmt_engine_].original<decltype(&is_paused)>(xor_int(90));
			return original(ecx, edx);
		}

		bool __fastcall msg_voice_data(void* ecx, void* edx, c_svc_msg_voice_data* message)
		{
			static auto original = hooker.original(&msg_voice_data);

			g_esp_store->store_voice(message);
			return original(ecx, edx, message);
		}

		bool __fastcall is_hltv(void* ecx, void* edx)
		{
			static auto original = vtables[vmt_engine_].original<decltype(&is_hltv)>(xor_int(93));

			if ((uintptr_t)_ReturnAddress() == patterns::return_addr_setup_velocity.as< uintptr_t >()
				|| (uintptr_t)_ReturnAddress() == patterns::return_addr_accumulate_layers.as< uintptr_t >()
				|| (uintptr_t)_ReturnAddress() == patterns::return_addr_reevaluate_anim_lod.as< uintptr_t >())
				return true;

			return original(ecx, edx);
		}

		void __vectorcall read_packets(bool final_tick)
		{
			static auto original = hooker.original(&read_packets);
			original(final_tick);
		}

		void __vectorcall cl_move(float accumulated_extra_samples, bool final_tick)
		{
			static auto original = hooker.original(&cl_move);

			if (!g_ctx.local || !g_ctx.is_alive || !g_ctx.in_game)
			{
				original(accumulated_extra_samples, final_tick);
				return;
			}

			if (g_exploits->should_recharge())
			{
				const auto frame = interfaces::client_state->last_outgoing_command + 1;

				auto& verified = interfaces::input->verified_commands[frame % 150];
				auto& cmd = interfaces::input->commands[frame % 150];
				cmd = interfaces::input->commands[(frame - 1) % 150];
				cmd.tickcount = INT_MAX;
				cmd.command_number = frame;

				verified.cmd = cmd;
				verified.crc = cmd.get_check_sum();

				interfaces::client_state->last_outgoing_command = interfaces::client_state->net_channel_ptr->send_datagram();

				if (g_ctx.weapon->is_grenade())
					g_ctx.predicted_curtime = math::ticks_to_time(g_ctx.tick_base);
				else
					g_ctx.predicted_curtime = math::ticks_to_time(g_ctx.tick_base - g_exploits->tickbase_offset());

				// predict ticbase on low fps
				// https://github.com/rollraw/qo0-base/blob/master/base/features/prediction.cpp#L172
				g_ctx.tick_base = [&]()
				{
					static int tick_base = 0;

					if (g_ctx.cmd != nullptr)
					{
						static c_usercmd* last_cmd = nullptr;

						// if command was not predicted - increment tickbase
						if (last_cmd == nullptr || last_cmd->predicted)
							tick_base = g_ctx.local->tickbase();
						else
							tick_base++;

						last_cmd = g_ctx.cmd;
					}

					return tick_base;
				}();

				auto vars = g_local_animation_fix->get_updated_netvars();
				vars->landing = false;
				vars->started_moving_this_frame = false;
				vars->stopped_moving_this_frame = false;
				vars->rebuilt_state = {};
				vars->old_movetype = 0;
				vars->old_flags = 0;
				vars->aim_matrix_width_range = 0;
				vars->max_desync_range = 0;

				g_animation_fix->update_valid_ticks();
				return;
			}

			original(accumulated_extra_samples, final_tick);
			g_exploits->shift_clmove(accumulated_extra_samples, final_tick);
		}

		void __fastcall process_packet(c_netchan* ecx, void* edx, void* packet, bool header)
		{
			static auto original = hooker.original(&process_packet);
			original(ecx, edx, packet, header);

			g_ping_spike->on_procces_packet();
		}

		void __stdcall host_shutdown()
		{
			static auto original = hooker.original(&host_shutdown);
			original();
		}

		int __fastcall start_sound_immediate(start_sound_params_t* ecx, void* edx)
		{
			static auto original = hooker.original(&start_sound_immediate);

			if (!g_ctx.in_game || !g_ctx.local || !ecx || !ecx->sfx)
				return original(ecx, edx);

			char sound_name[260]{};
			ecx->sfx->get_sound_name(sound_name, 260);

			static auto semiauto_switch = xor_c("auto_semiauto_switch");
			static auto weapons = xor_c("weapons");
			static auto player_land = xor_c("player\\land");
			static auto player_footsteps = xor_c("player\\footsteps");

			const auto is_valid_sound = [sound_name]() -> bool {
				return strstr(sound_name, semiauto_switch)
					|| strstr(sound_name, weapons)
					|| strstr(sound_name, player_land)
					|| strstr(sound_name, player_footsteps);
			};

			auto entity = (c_baseentity*)interfaces::entity_list->get_entity(ecx->sound_source);
			if (!entity || !is_valid_sound())
				return original(ecx, edx);

			auto player = (c_csplayer*)entity;
			if (player && !player->is_player())
			{
				if (entity->is_weapon())
					player = (c_csplayer*)(interfaces::entity_list->get_entity_handle(entity->owner()));
			}

			auto origin = ecx->origin;
			if (strstr(sound_name, weapons))
				origin.z -= strstr(sound_name, semiauto_switch) ? 32.f : 64.f;

			if (player && player->is_player())
			{
				if (player != g_ctx.local && player->team() != g_ctx.local->team())
				{
					auto& sound = g_esp_store->sounds[player->index()];
					sound.spot_time = interfaces::global_vars->real_time;
					sound.pos = origin;
					sound.flags = player->flags();
					sound.spotted = true;
					sound.got_box = true;
					sound.dormant_pose_body = 0.f;
					sound.dormant_pose_pitch = 1.f;

					sound.dormant_mins = vector3d{ -16.f, -16.f, 0.f };
					sound.dormant_maxs = vector3d{ 16.f, 16.f, 72.f };

					player->target_spotted() = true;
				}
			}

			return original(ecx, edx);
		}
	}
}