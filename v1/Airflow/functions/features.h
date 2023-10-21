#pragma once
#include "extra/movement.h"
#include "extra/utils.h"
#include "extra/world_modulation.h"
#include "extra/cmd_shift.h"

#include "skins/skins.h"

#include "listeners/listener_entity.h"
#include "listeners/listener_event.h"

#include "anti hit/fake_lag.h"
#include "anti hit/exploits.h"
#include "anti hit/anti_aim.h"

#include "visuals/visuals.h"
#include "visuals/local_player_visuals.h"
#include "visuals/grenade_warning.h"
#include "visuals/glow.h"
#include "visuals/esp_store.h"
#include "visuals/esp_share.h"
#include "visuals/esp_player.h"
#include "visuals/esp_weapon.h"
#include "visuals/chams.h"
#include "visuals/event/event_visuals.h"
#include "visuals/event/event_logger.h"

#include "ragebot/animfix.h"
#include "ragebot/local_animfix.h"
#include "ragebot/rage_tools.h"
#include "ragebot/resolver.h"
#include "ragebot/engine_prediction.h"
#include "ragebot/autowall.h"
#include "ragebot/ragebot.h"
#include "ragebot/game_movement.h"
#include "ragebot/server_animations.h"

#include "menu/menu.h"

#include "legitbot/legitbot.h"

declare_feature_ptr(world_modulation);
declare_feature_ptr(utils);
declare_feature_ptr(movement);

declare_feature_ptr(event_listener);
declare_feature_ptr(listener_entity);

declare_feature_ptr(fake_lag);
declare_feature_ptr(tickbase);
declare_feature_ptr(exploits);
declare_feature_ptr(anti_aim);
declare_feature_ptr(ping_spike);

declare_feature_ptr(visuals_wrapper);
declare_feature_ptr(local_visuals);
declare_feature_ptr(grenade_warning);
declare_feature_ptr(glow_esp);
declare_feature_ptr(esp_store);
declare_feature_ptr(player_esp);
declare_feature_ptr(weapon_esp);
declare_feature_ptr(chams);
declare_feature_ptr(event_visuals);
declare_feature_ptr(event_logger);

declare_feature_ptr(animation_fix);
declare_feature_ptr(local_animation_fix);
declare_feature_ptr(engine_prediction);
declare_feature_ptr(auto_wall);
declare_feature_ptr(rage_bot);

declare_feature_ptr(menu);

declare_feature_ptr(legit_bot);

namespace ping_reducer
{
	struct ping_backup_t
	{
		float curtime{ };
		float frametime{ };
		int tickcount{ };
		int cs_tickcount{};

		void read()
		{
			curtime = interfaces::global_vars->cur_time;
			frametime = interfaces::global_vars->frame_time;
			tickcount = interfaces::global_vars->tick_count;
			cs_tickcount = interfaces::client_state->old_tickcount;
		}

		void write()
		{
			interfaces::global_vars->cur_time = curtime;
			interfaces::global_vars->frame_time = frametime;
			interfaces::global_vars->tick_count = tickcount;
			interfaces::client_state->old_tickcount = cs_tickcount;
		}

		inline void reset()
		{
			curtime = 0.f;
			frametime = 0.f;
			tickcount = 0;
			cs_tickcount = 0;
		}
	};

	inline ping_backup_t ping_data{ };

	__forceinline bool available()
	{
		if (!g_ctx.in_game)
			return false;

		auto net_chan = interfaces::engine->get_net_channel_info();
		if (!net_chan)
			return false;

		if (net_chan->is_loopback())
			return false;

		//if (g_exploits->cl_move.trigger && g_exploits->cl_move.shifting || cmd_shift::shifting)
		//	return false;

		return true;
	}

	__forceinline bool should_work()
	{
		if (!available())
			return false;

		ping_data.write();
		return true;
	}

	__forceinline void update_ping_values(bool final_tick)
	{
		if (!available())
			return;

		static auto original_packet = hooker.original(&tr::engine::read_packets);

		ping_backup_t backup{ };
		backup.read();

		original_packet(final_tick);
		ping_data.read();

		backup.write();
	}
}