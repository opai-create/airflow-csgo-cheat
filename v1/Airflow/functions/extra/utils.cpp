#include "utils.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

#include "../config_vars.h"

#include "../features.h"

const char8_t* icons[]
{
	u8"\u2621",
	u8"\u2620",
	u8"\u2623",
	u8"\u262F",
	u8"\u267B",
	u8"\u26A1",
	u8"\u26A3",
};

constexpr auto empty_icon = u8"\u2800";

void c_utils::update_ground_ticks()
{
	if (g_ctx.local->flags() & fl_onground)
	{
		if (ground_ticks <= 3)
			ground_ticks++;
	}
	else
	{
		ground_ticks = 0;
		return;
	}
}

void c_utils::update_shot_cmd()
{
	if (g_exploits->cl_move.trigger && g_exploits->cl_move.shifting)
		return;

	if (this->is_firing())
	{
		g_ctx.shot_cmd = g_ctx.cmd->command_number;
		g_ctx.last_shoot_position = g_ctx.local->get_eye_position();
		g_ctx.last_shoot_angle = g_ctx.cur_angle + g_ctx.local->aim_punch_angle();

		if (g_cfg.antihit.silent_onshot && !g_anti_aim->is_fake_ducking())
			*g_ctx.send_packet = true;
	}

	g_ctx.shooting = g_ctx.shot_cmd == g_ctx.cmd->command_number;
}

void c_utils::update_viewangles()
{
	if (g_ctx.cmd->command_number >= g_ctx.shot_cmd && g_ctx.shot_cmd >= g_ctx.cmd->command_number - interfaces::client_state->choked_commands)
	{
		auto shot_cmd = interfaces::input->get_user_cmd(g_ctx.shot_cmd);
		if (shot_cmd)
		{
			if (*g_ctx.send_packet)
				g_ctx.fake_angle = shot_cmd->viewangles;

			g_ctx.cur_angle = shot_cmd->viewangles;
		}
	}
	else
	{
		if (*g_ctx.send_packet)
			g_ctx.fake_angle = g_ctx.cmd->viewangles;

		g_ctx.cur_angle = g_ctx.cmd->viewangles;
	}

	g_ctx.fake_angle.z = 0.f;
}

void c_utils::extrapolate(c_csplayer* player, vector3d& origin, vector3d& velocity, int& flags, bool on_ground)
{
	if (!(flags & fl_onground))
		velocity.z -= math::ticks_to_time(cvars::sv_gravity->get_float());
	else if (player->flags() & fl_onground && !on_ground)
		velocity.z = cvars::sv_jump_impulse->get_float();

	const auto src = origin;
	auto end = src + velocity * interfaces::global_vars->interval_per_tick;

	c_game_trace t{};
	c_trace_filter filter;
	filter.skip = player;

	interfaces::engine_trace->trace_ray(ray_t(src, end, player->bb_mins(), player->bb_maxs()), mask_playersolid, &filter, &t);

	if (t.fraction != 1.f)
	{
		for (auto i = 0; i < 2; i++)
		{
			velocity -= t.plane.normal * velocity.dot(t.plane.normal);

			const auto dot = velocity.dot(t.plane.normal);
			if (dot < 0.f)
				velocity -= vector3d(dot * t.plane.normal.x, dot * t.plane.normal.y, dot * t.plane.normal.z);

			end = t.end + velocity * math::ticks_to_time(1.f - t.fraction);

			interfaces::engine_trace->trace_ray(ray_t(t.end, end, player->bb_mins(), player->bb_maxs()), mask_playersolid, &filter, &t);

			if (t.fraction == 1.f)
				break;
		}
	}

	origin = end = t.end;
	end.z -= 2.f;

	interfaces::engine_trace->trace_ray(ray_t(origin, end, player->bb_mins(), player->bb_maxs()), mask_playersolid, &filter, &t);

	flags &= ~fl_onground;

	if (t.did_hit() && t.plane.normal.z > .7f)
		flags |= fl_onground;
}

bool c_utils::on_ground()
{
	return ground_ticks >= 3;
}

bool c_utils::chat_opened()
{
	if (!g_ctx.local)
		return false;

	auto chat = (CCSGO_HudChat*)func_ptrs::find_hud_element(*patterns::get_hud_ptr.as< uintptr_t** >(), xor_c("CCSGO_HudChat"));
	if (!chat)
		return false;

	return chat->chat_opened;
}

__forceinline void marquee_text(std::string& text)
{
	std::string temp = text;
	text.erase(0, 1);
	text += temp[0];
}

void c_utils::clantag()
{
	auto set_clan_tag = [](const char* tag, const char* name)
	{
		static auto fn = patterns::clantag.as< void(__fastcall*)(const char*, const char*) >();
		fn(tag, name);
	};

	static auto tag_desc = xor_str("airflow");
	static bool reset_tag = true;
	static int last_clantag_time = 0;
	static float next_update_time = 0.f;

	float predicted_curtime = math::ticks_to_time(g_ctx.tick_base);
	int clantag_time = (int)(interfaces::global_vars->cur_time * 2.f) + g_ctx.ping;

	if (g_cfg.misc.clantag)
	{
		reset_tag = false;

		if (clantag_time != last_clantag_time)
		{
			if (next_update_time <= predicted_curtime || next_update_time - predicted_curtime > 1.f)
			{
				static std::string tag_text_2 = xor_str("airflow");

				std::string tag_start = (const char*)icons[int(interfaces::global_vars->cur_time * 2.4f) % ARRAYSIZE(icons)];

				std::string tag_text = tag_start + tag_text_2 + tag_start;
				set_clan_tag(tag_text.c_str(), tag_desc.c_str());
			}

			last_clantag_time = clantag_time;
		}
	}
	else
	{
		if (!reset_tag)
		{
			if (clantag_time != last_clantag_time)
			{
				set_clan_tag("", "");

				reset_tag = true;
				last_clantag_time = clantag_time;
			}
		}
	}
}

void c_utils::buybot()
{
	if (!g_cfg.misc.buybot.enable)
		return;

	if (start_buybot)
	{
		std::string buy_str{};

		switch (g_cfg.misc.buybot.main_weapon)
		{
		case 1:
			buy_str += (xor_c("buy scar20; "));
			buy_str += (xor_c("buy g3sg1; "));
			break;
		case 2:
			buy_str += (xor_c("buy ssg08; "));
			break;
		case 3:
			buy_str += (xor_c("buy awp; "));
			break;
		case 4:
			buy_str += (xor_c("buy negev; "));
			break;
		case 5:
			buy_str += (xor_c("buy m249; "));
			break;
		case 6:
			buy_str += (xor_c("buy ak47; "));
			buy_str += (xor_c("buy m4a1; "));
			buy_str += (xor_c("buy m4a1_silencer; "));
			break;
		case 7:
			buy_str += (xor_c("buy aug; "));
			buy_str += (xor_c("buy sg556; "));
			break;
		}

		switch (g_cfg.misc.buybot.second_weapon)
		{
		case 1:
			buy_str += (xor_c("buy elite; "));
			break;
		case 2:
			buy_str += (xor_c("buy p250; "));
			break;
		case 3:
			buy_str += (xor_c("buy tec9; "));
			buy_str += (xor_c("buy fn57; "));
			break;
		case 4:
			buy_str += (xor_c("buy deagle; "));
			buy_str += (xor_c("buy revolver; "));
			break;
		}

		if (g_cfg.misc.buybot.other_items & 1)
			buy_str += (xor_c("buy vesthelm; "));
		if (g_cfg.misc.buybot.other_items & 2)
			buy_str += (xor_c("buy vest; "));

		if (g_cfg.misc.buybot.other_items & 4)
			buy_str += (xor_c("buy hegrenade; "));

		if (g_cfg.misc.buybot.other_items & 8)
		{
			buy_str += (xor_c("buy molotov; "));
			buy_str += (xor_c("buy incgrenade; "));
		}

		if (g_cfg.misc.buybot.other_items & 16)
			buy_str += (xor_c("buy smokegrenade; "));
		if (g_cfg.misc.buybot.other_items & 32)
			buy_str += (xor_c("buy taser; "));
		if (g_cfg.misc.buybot.other_items & 64)
			buy_str += (xor_c("buy defuser; "));

		interfaces::engine->execute_cmd_unrestricted(buy_str.c_str());
	}

	start_buybot = false;
}

void c_utils::update_viewmodel_sequence(c_usercmd* cmd, bool restore)
{
	static float backup_cycle = 0.f;
	static bool weapon_triggered = false;

	if (!g_ctx.local->is_alive())
	{
		backup_cycle = 0.f;
		weapon_triggered = false;
		g_ctx.cycle_changed = false;
		g_ctx.fix_cycle = false;
		return;
	}

	auto viewmodel = g_ctx.local->get_view_model();
	if (viewmodel)
	{
		if (restore)
		{
			weapon_triggered = cmd->weaponselect > 0 || cmd->buttons & (in_attack | in_attack2);
			backup_cycle = viewmodel->cycle();
		}
		else if (weapon_triggered && !g_ctx.cycle_changed)
			g_ctx.cycle_changed = viewmodel->cycle() == 0.f && backup_cycle > 0.f;
	}
}

void c_utils::update_mouse_delta()
{
	vector3d old_view_angles{};
	interfaces::engine->get_view_angles(old_view_angles);

	float delta_x = std::remainderf(g_ctx.cmd->viewangles.x - old_view_angles.x, 360.0f);
	float delta_y = std::remainderf(g_ctx.cmd->viewangles.y - old_view_angles.y, 360.0f);

	if (delta_x != 0.0f)
	{
		float mouse_y = -((delta_x / cvars::m_pitch->get_float()) / cvars::sensitivity->get_float());
		short mousedy;
		if (mouse_y <= 32767.0f)
		{
			if (mouse_y >= -32768.0f)
			{
				if (mouse_y >= 1.0f || mouse_y < 0.0f)
				{
					if (mouse_y <= -1.0f || mouse_y > 0.0f)
						mousedy = static_cast<short>(mouse_y);
					else
						mousedy = -1;
				}
				else
				{
					mousedy = 1;
				}
			}
			else
			{
				mousedy = 0x8000u;
			}
		}
		else
		{
			mousedy = 0x7FFF;
		}

		g_ctx.cmd->mousedy = mousedy;
	}

	if (delta_y != 0.0f)
	{
		float mouse_x = -((delta_y / cvars::m_yaw->get_float()) / cvars::sensitivity->get_float());
		short mousedx;
		if (mouse_x <= 32767.0f)
		{
			if (mouse_x >= -32768.0f)
			{
				if (mouse_x >= 1.0f || mouse_x < 0.0f)
				{
					if (mouse_x <= -1.0f || mouse_x > 0.0f)
						mousedx = static_cast<short>(mouse_x);
					else
						mousedx = -1;
				}
				else
				{
					mousedx = 1;
				}
			}
			else
			{
				mousedx = 0x8000u;
			}
		}
		else
		{
			mousedx = 0x7FFF;
		}

		g_ctx.cmd->mousedx = mousedx;
	}
}

void c_utils::auto_revolver()
{
	c_basecombatweapon* weapon = g_ctx.weapon;
	if (!weapon)
		return;

	static auto last_checked = 0;
	static auto last_spawn_time = 0.f;
	static auto tick_cocked = 0;
	static auto tick_strip = 0;

	auto next_secondary_attack = g_ctx.weapon->next_secondary_attack();

	if (!g_cfg.rage.enable || g_exploits->recharge.start && !g_exploits->recharge.finish || weapon->item_definition_index() != weapon_revolver || weapon->clip1() <= 0)
	{
		last_checked = 0;
		tick_cocked = 0;
		tick_strip = 0;
		next_secondary_attack = 0.f;

		revolver_fire = false;
		return;
	}

	auto time = math::ticks_to_time(g_ctx.tick_base - g_exploits->tickbase_offset());
	const auto max_ticks = math::time_to_ticks(.25f) - 1;
	const auto tick_base = math::time_to_ticks(time);

	if (g_ctx.local->next_attack() > time)
		return;

	if (g_ctx.local->spawn_time() != last_spawn_time)
	{
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
		last_spawn_time = g_ctx.local->spawn_time();
	}

	if (g_ctx.weapon->next_primary_attack() > time)
	{
		g_ctx.cmd->buttons &= ~in_attack;
		revolver_fire = false;
		return;
	}

	if (last_checked == tick_base)
		return;

	last_checked = tick_base;
	revolver_fire = false;

	if (tick_base - tick_strip > 2 && tick_base - tick_strip < 14)
		revolver_fire = true;

	if (g_ctx.cmd->buttons & in_attack && revolver_fire)
		return;

	g_ctx.cmd->buttons |= in_attack;

	if (next_secondary_attack >= time)
		g_ctx.cmd->buttons |= in_attack2;

	if (tick_base - tick_cocked > max_ticks * 2 + 1)
	{
		tick_cocked = tick_base;
		tick_strip = tick_base - max_ticks - 1;
	}

	const auto cock_limit = tick_base - tick_cocked >= max_ticks;
	const auto after_strip = tick_base - tick_strip <= max_ticks;

	if (cock_limit || after_strip)
	{
		tick_cocked = tick_base;
		g_ctx.cmd->buttons &= ~in_attack;

		if (cock_limit)
			tick_strip = tick_base;
	}
}

void c_utils::auto_pistol()
{
	if (!g_ctx.local || !g_ctx.local->is_alive() || !g_ctx.weapon || !g_cfg.rage.enable)
		return;

	short idx = g_ctx.weapon->item_definition_index();
	if (idx == weapon_c4 || idx == weapon_healthshot || idx == weapon_revolver || (idx == weapon_glock || idx == weapon_famas) && g_ctx.weapon->burst_shots_remaining() > 0)
		return;

	if (g_ctx.weapon->is_auto_sniper() || g_ctx.weapon->is_misc_weapon() && !g_ctx.weapon->is_knife())
		return;

	float next_attack = g_ctx.local->next_attack();
	float next_primary_attack = g_ctx.weapon->next_primary_attack();
	float next_secondary_attack = g_ctx.weapon->next_secondary_attack();

	if (g_ctx.predicted_curtime < next_attack || g_ctx.predicted_curtime < next_primary_attack)
	{
		if (g_ctx.cmd->buttons & in_attack)
			g_ctx.cmd->buttons &= ~in_attack;
	}

	if (g_ctx.predicted_curtime < next_secondary_attack)
	{
		if (g_ctx.cmd->buttons & in_attack2)
			g_ctx.cmd->buttons &= ~in_attack2;
	}
}

bool c_utils::is_able_to_shoot(bool revolver)
{
	if (!g_ctx.local || !g_ctx.weapon)
		return false;

	if (g_ctx.cmd->weaponselect != 0)
		return false;

	if (!g_ctx.weapon_info)
		return false;

	if (g_ctx.local->flags() & 0x40)
		return false;

	if (g_ctx.local->wait_for_no_attack())
		return false;

	if (g_ctx.local->is_defusing())
		return false;

	if (g_ctx.weapon_info->weapon_type >= 1 && g_ctx.weapon_info->weapon_type <= 6 && g_ctx.weapon->clip1() < 1)
		return false;

	if (g_ctx.local->player_state() > 0)
		return false;

	float time = g_ctx.predicted_curtime;
	auto idx = g_ctx.weapon->item_definition_index();

	if ((idx == weapon_glock || idx == weapon_famas) && g_ctx.weapon->burst_shots_remaining() > 0)
		return time >= g_ctx.weapon->next_burst_shot();

	if (idx == weapon_revolver && revolver)
		return revolver_fire;

	float next_attack = g_ctx.local->next_attack();
	float next_primary_attack = g_ctx.weapon->next_primary_attack();

	return time >= next_attack && time >= next_primary_attack;
}

bool c_utils::is_firing()
{
	auto weapon = g_ctx.weapon;
	if (!weapon)
		return false;

	bool attack = (g_ctx.cmd->buttons & in_attack) || (g_ctx.cmd->buttons & in_attack2);
	short idx = weapon->item_definition_index();

	if (idx == weapon_c4)
		return false;

	if ((idx == weapon_glock || idx == weapon_famas) && weapon->burst_shots_remaining() > 0)
	{
		return g_ctx.predicted_curtime >= weapon->next_burst_shot();
	}

	if (weapon->is_grenade())
		return !weapon->pin_pulled() && weapon->throw_time() > 0.f && weapon->throw_time() < g_ctx.predicted_curtime;

	if (weapon->is_knife())
		return attack && this->is_able_to_shoot();

	return (g_ctx.cmd->buttons & in_attack) && this->is_able_to_shoot(true);
}

void c_utils::on_postdata_update_start(int stage)
{
	if (!g_ctx.in_game)
		return;

	if (!g_ctx.local)
		return;

	static float networked_cycle = 0.f;
	static float animation_time = 0.f;
	static int nerworked_sequence = 0;

	if (!g_ctx.local->is_alive())
	{
		networked_cycle = animation_time = 0.f;
		g_ctx.fix_cycle = false;
		return;
	}

	static float old_cycle = 0.f;
	static float old_animtime = 0.f;

	auto viewmodel = g_ctx.local->get_view_model();
	if (viewmodel)
	{
		if (stage == frame_net_update_postdataupdate_start && g_ctx.fix_cycle && viewmodel->cycle() == 0.f)
		{
			viewmodel->cycle() = networked_cycle;
			viewmodel->anim_time() = animation_time;
			g_ctx.fix_cycle = false;
		}

		networked_cycle = viewmodel->cycle();
		nerworked_sequence = viewmodel->sequence();
		animation_time = viewmodel->anim_time();
	}
}

void c_utils::on_net_update_end_after(int stage)
{
	if (stage != frame_net_update_end)
		return;

	if (!g_ctx.in_game)
		return;

	if (!g_ctx.local)
		return;

	if (!g_ctx.local->is_alive())
		return;


}

void c_utils::on_game_events(c_game_event* event)
{
	if (std::strcmp(event->get_name(), xor_c("round_start")))
		return;

	start_buybot = true;
}

void c_utils::on_pre_predict()
{
	this->update_ground_ticks();
	this->update_mouse_delta();
	this->auto_pistol();
	this->auto_revolver();
	this->buybot();
}

void c_utils::on_predict_start()
{
	this->update_shot_cmd();
	this->update_viewangles();
}

void c_utils::on_predict_end()
{
	if (!g_ctx.weapon || !g_ctx.weapon_info)
		return;

	if (*g_ctx.send_packet)
	{
		if (g_ctx.cycle_changed)
		{
			g_ctx.cycle_changed = false;
			g_ctx.fix_cycle = true;
		}
	}

	g_ctx.weapon_info->hide_viewmodel_in_zoom = !g_cfg.misc.viewmodel_scope;
}

void c_utils::on_changed_map()
{
	if (g_event_visuals->impacts.size() > 0)
		g_event_visuals->impacts.clear();

	if (g_event_visuals->hitmarkers.size() > 0)
		g_event_visuals->hitmarkers.clear();

	g_ctx.sky_name = cvars::sv_skyname->string;
	g_world_modulation->old_sky_name = "";

	if (g_fake_lag->commands.size() > 0)
		g_fake_lag->commands.clear();
}