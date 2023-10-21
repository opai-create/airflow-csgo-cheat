#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../functions/config_vars.h"
#include "../../../functions/skins/skins.h"
#include "../../../functions/extra/cmd_shift.h"
#include "../../../functions/features.h"

#include <string>

namespace tr::client
{
    bool __fastcall dispatch_user_message(void* _this, void* edx, int msg_type, int arg, int arg1, void* data)
    {
        static auto original = vtables[vmt_client].original<decltype(&dispatch_user_message)>(xor_int(38));

        if (!interfaces::game_rules->is_valve_ds())
        {
            if (g_cfg.misc.remove_ads && (msg_type == 7 || msg_type == 8 || msg_type == 5))
                return true;
        }

        return original(_this, edx, msg_type, arg, arg1, data);
    }

    // https://www.unknowncheats.me/forum/counterstrike-global-offensive/501277-fix-white-chams-office-dust.html
    void __fastcall get_exposure_range(float* min, float* max)
    {
        static auto original = hooker.original(&get_exposure_range);

        *min = 1.f;
        *max = 1.f;

        original(min, max);
    }

    void __fastcall perform_screen_overlay(void* _this, void* edx, int x, int y, int w, int h)
    {
        if (g_cfg.misc.remove_ads)
            return;

        static auto original = hooker.original(&perform_screen_overlay);
        return original(_this, edx, x, y, w, h);
    }

    void __fastcall frame_stage_notify(void* ecx, void* edx, int stage)
    {
        static auto original = hooker.original(&frame_stage_notify);

        // dynamic intefaces are changing their address ingame
        // so we need to get them when map changes
        interfaces::init_dynamic_interfaces();

        g_ctx.fsn_stage = stage;
        g_ctx.in_game = interfaces::engine->is_connected() && interfaces::engine->is_in_game();

        g_ctx.update_local_player();

        g_ctx.is_alive = g_ctx.local && g_ctx.local->is_alive();
        g_ctx.open_console = g_utils->chat_opened() || interfaces::engine->console_opened();

        skin_changer::on_postdataupdate_start(stage);
        skin_changer::on_frame_render_end(stage);

        g_world_modulation->on_render_start(stage);
        g_event_visuals->on_render_start(stage);
        g_local_visuals->on_render_start(stage);
        g_utils->on_postdata_update_start(stage);
        g_grenade_warning->on_render_start(stage);

        original(ecx, edx, stage);

        g_animation_fix->on_net_update_and_render_after(stage);
        g_utils->on_net_update_end_after(stage);
        g_local_visuals->on_render_start_after(stage);
    }

    void __fastcall create_move_wrapper(void* _this, int, int sequence_number, float input_sample_frametime, bool active);

    void __stdcall create_move(int sequence_number, float input_sample_frametime, bool active, bool& bSendPacket)
    {
        static auto original = vtables[vmt_client].original<decltype(&create_move_wrapper)>(xor_int(22));

        // ghetto fix for latest csgo update (19.02)
        g_ctx.send_packet = &bSendPacket;
        original(interfaces::client, 0, sequence_number, input_sample_frametime, active);

        auto shifting = cmd_shift::shifting || g_exploits->cl_move.trigger && g_exploits->cl_move.shifting;
        if (g_ctx.local && g_ctx.local->is_alive() && interfaces::client_state && !shifting)
        {
            auto net_channel = interfaces::client_state->net_channel_ptr;
            if (net_channel)
            {
                if (!*g_ctx.send_packet && net_channel->choked_packets > 0)
                {
                    auto old_choke = net_channel->choked_packets;

                    net_channel->choked_packets = 0;
                    net_channel->send_datagram();
                    --net_channel->out_sequence_nr;

                    net_channel->choked_packets = old_choke;
                }
                else
                    g_fake_lag->commands.emplace_back(g_ctx.cmd->command_number);
            }
        }
    }

    // push sendpacket to func and then modify it
    __declspec(naked) void __fastcall create_move_wrapper(void* _this, int, int sequence_number, float input_sample_frametime, bool active)
    {
        __asm
        {
            push ebp
            mov  ebp, esp
            push ebx
            push esp
            push dword ptr[active]
            push dword ptr[input_sample_frametime]
            push dword ptr[sequence_number]
            call create_move
            pop  ebx
            pop  ebp
            retn 0Ch
        }
    }

    void __fastcall setup_clr_modulation_brushes(void* ecx, void* edx, int cnt, model_render_system_data_t* models, brush_array_instance_data_t* data, int render_mode)
    {
        for (int i = 0; i < cnt; ++i)
        {
            auto renderable = (c_renderable*)models[i].renderable;
            if (renderable)
            {
                // get base color and modulate then
                renderable->get_color_modulation(data[i].diffuse_modulation.base());

                if (g_cfg.misc.world_modulation & 1)
                {
                    data[i].diffuse_modulation.x *= g_cfg.misc.world_clr[world][0];
                    data[i].diffuse_modulation.y *= g_cfg.misc.world_clr[world][1];
                    data[i].diffuse_modulation.w *= g_cfg.misc.world_clr[world][2];
                }

                float alpha = (models[i].instance_data.alpha / 255.f);
                data[i].diffuse_modulation.h = alpha;
            }
        }
    }

    void __fastcall setup_clr_modulation(void* ecx, void* edx, int cnt, model_list_by_type_t* list)
    {
        static auto models_weapons = xor_str("models/weapons/w_");

        for (int i = 0; i < cnt; ++i)
        {
            auto& l = list[i];
            if (!l.count)
                continue;

            for (int j = 0; j < l.count; ++j)
            {
                auto model = &l.render_models[j];
                auto renderable = (c_renderable*)model->entry.renderable;
                if (!renderable)
                    continue;

                auto is_grenade = [](int class_id)
                {
                    switch (class_id)
                    {
                    case(int)CBaseCSGrenade:
                    case(int)CBaseCSGrenadeProjectile:
                    case(int)CBreachCharge:
                    case(int)CBreachChargeProjectile:
                    case(int)CBumpMine:
                    case(int)CBumpMineProjectile:
                    case(int)CDecoyGrenade:
                    case(int)CDecoyProjectile:
                    case(int)CMolotovGrenade:
                    case(int)CMolotovProjectile:
                    case(int)CSensorGrenade:
                    case(int)CSensorGrenadeProjectile:
                    case(int)CSmokeGrenade:
                    case(int)CSmokeGrenadeProjectile:
                    case(int)CSnowballProjectile:
                    case(int)CIncendiaryGrenade:
                    case(int)CInferno:
                        return true;
                        break;
                    }
                    return false;
                };

                auto force_prop_color = [&](bool original = false)
                {
                    // get base color and modulate then
                    renderable->get_color_modulation(model->diffuse_modulation.base());

                    if (!original)
                    {
                        model->diffuse_modulation.x *= g_cfg.misc.world_clr[props][0];
                        model->diffuse_modulation.y *= g_cfg.misc.world_clr[props][1];
                        model->diffuse_modulation.w *= g_cfg.misc.world_clr[props][2];
                    }

                    float alpha = (model->entry.instance_data.alpha / 255.f);
                    model->diffuse_modulation.h = alpha;
                };

                auto force_prop_alpha = [&]() { model->diffuse_modulation.h *= (g_cfg.misc.prop_alpha / 100.f); };

                auto entity = (c_baseentity*)renderable->get_i_unknown_entity()->get_client_entity();
                if (entity)
                {
                    // ignore misc entities
                    auto class_id = entity->get_client_class()->class_id;
                    if (is_grenade(class_id) || class_id == CHostage || class_id == CChicken || class_id == CBaseDoor)
                    {
                        force_prop_color(true);
                        continue;
                    }

                    // ignore weapon that player have in his inventory
                    auto weapon_entity = (c_basecombatweapon*)entity;
                    if (weapon_entity->owner() > 0)
                        continue;

                    force_prop_color(!(g_cfg.misc.world_modulation & 1));
                }
                else
                {
                    force_prop_color(!(g_cfg.misc.world_modulation & 1));
                    force_prop_alpha();
                }
            }
        }
    }

    void __fastcall draw_models(void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, char a7)
    {
        static auto original = hooker.original(&draw_models);
        original(ecx, edx, a2, a3, a4, a5, a6, a7);
    }

    void __fastcall add_view_model_bob(void* ecx, void* edx, void* model, vector3d& origin, vector3d& angles)
    {
        static auto original = hooker.original(&add_view_model_bob);
        original(ecx, edx, model, origin, angles);
    }

    void __fastcall calc_view_model_bob(void* ecx, void* edx, vector3d& position)
    {
        static auto original = hooker.original(&calc_view_model_bob);
        if (!(g_cfg.misc.removals & landing_bob))
            return original(ecx, edx, position);

        return;
    }

    void __fastcall level_init_pre_entity(void* ecx, void* edx, const char* map)
    {
        static auto original = vtables[vmt_client].original<decltype(&level_init_pre_entity)>(xor_int(5));

        g_ctx.reset_local_player();
        g_ctx.reset_ctx();

        g_utils->on_changed_map();
        g_rage_bot->on_changed_map();

        int ticks = (int)(1.f / interfaces::global_vars->interval_per_tick);

        if (cvars::cl_cmdrate)
            cvars::cl_cmdrate->set_value(ticks);

        if (cvars::cl_updaterate)
            cvars::cl_updaterate->set_value(ticks);

        if (cvars::cl_interp)
            cvars::cl_interp->set_value(interfaces::global_vars->interval_per_tick);

        if (cvars::rate)
            cvars::rate->set_value(786432);

        g_fake_lag->commands.clear();

        g_menu->bomb.reset();

        for (auto& i : g_menu->spectators)
            i.reset();

        g_engine_prediction->on_local_death();
        g_engine_prediction->init();

        g_rage_bot->on_local_death();
        g_fake_lag->on_local_death();
        g_local_animation_fix->on_local_death();
        g_exploits->reset();

        ping_reducer::ping_data.reset();

        original(ecx, edx, map);
    }

    void __fastcall level_init_post_entity(void* ecx, void* edx)
    {
        static auto original = vtables[vmt_client].original<decltype(&level_init_post_entity)>(xor_int(6));

        g_ctx.reset_local_player();
        g_ctx.reset_ctx();

        ping_reducer::ping_data.reset();

        original(ecx, edx);
    }

    void __fastcall level_shutdown(void* ecx, void* edx)
    {
        static auto original = vtables[vmt_client].original<decltype(&level_shutdown)>(xor_int(7));

        g_ctx.reset_local_player();
        g_ctx.reset_ctx();

        g_utils->on_changed_map();
        g_rage_bot->on_changed_map();

        g_fake_lag->commands.clear();

        g_menu->bomb.reset();

        for (auto& i : g_menu->spectators)
            i.reset();

        g_engine_prediction->on_local_death();
        g_engine_prediction->init();

        g_fake_lag->on_local_death();
        g_local_animation_fix->on_local_death();
        g_exploits->reset();

        ping_reducer::ping_data.reset();

        original(ecx, edx);
    }

    void __fastcall physics_simulate(c_csplayer* ecx, void* edx)
    {
        static auto original = hooker.original(&physics_simulate);
        int& simulation_tick = *(int*)((uintptr_t)ecx + 0x2AC);
        if (!ecx || !ecx->is_alive() 
            || interfaces::global_vars->tick_count == simulation_tick 
            || ecx != g_ctx.local 
            || interfaces::engine->is_playing_demo() 
            || interfaces::engine->is_hltv() 
            || ecx->flags() & 0x40)
        {
            original(ecx, edx);
            return;
        }

        auto& ctx = ecx->cmd_context();

        auto viewmodel = g_ctx.local->get_view_model();
        if (!viewmodel)
        {
            original(ecx, edx);
            return;
        }

        auto old_mod = ecx->velocity_modifier();

        if (ctx.user_cmd.command_number == interfaces::client_state->last_command_ack + 1)
            ecx->velocity_modifier() = g_ctx.velocity_modifier;

        g_utils->update_viewmodel_sequence(&ctx.user_cmd, true);
        original(ecx, edx);
        g_utils->update_viewmodel_sequence(&ctx.user_cmd, false);

        g_engine_prediction->net_compress_store(ctx.user_cmd.command_number);
    }

    bool __fastcall write_usercmd_to_delta_buffer(void* ecx, void* edx, int slot, void* buf, int from, int to, bool isnewcommand)
    {
        static auto original = vtables[vmt_client].original<decltype(&write_usercmd_to_delta_buffer)>(xor_int(24));

        if (!g_ctx.local || !g_ctx.local->is_alive())
            return original(ecx, edx, slot, buf, from, to, isnewcommand);

        if (!g_exploits->enabled() || !g_exploits->tick_to_shift)
            return original(ecx, edx, slot, buf, from, to, isnewcommand);

        if (from != -1)
            return true;

        uintptr_t frame_ptr{};
        __asm mov frame_ptr, ebp;

        int* backup_commands = (int*)(frame_ptr + 0xFD8);
        int* new_commands = (int*)(frame_ptr + 0xFDC);

        return g_exploits->should_shift_cmd(new_commands, backup_commands, ecx, edx, slot, buf, from, to);
    }

    // remove irongsight (blur and gray shader) effect in scope
    void __fastcall update_postscreen_effects(void* ecx, void* edx)
    {
        static auto original = hooker.original(&update_postscreen_effects);

        if (!g_ctx.local || !g_ctx.local->is_alive())
            return original(ecx, edx);

        bool prev_scoped = g_ctx.local->is_scoped();

        if (g_cfg.misc.removals & scope)
            g_ctx.local->is_scoped() = false;

        original(ecx, edx);

        if (g_cfg.misc.removals & scope)
            g_ctx.local->is_scoped() = prev_scoped;
    }

    void __fastcall render_glow_boxes(c_glow_object_manager* ecx, void* edx, int pass, void* ctx)
    {
        static auto original = hooker.original(&render_glow_boxes);
        original(ecx, edx, pass, ctx);
    }

    bool __fastcall process_spotted_entity_update(void* ecx, uint32_t edx, c_spotted_entity_update_message* message)
    {
        static auto original = hooker.original(&process_spotted_entity_update);

        return original(ecx, edx, message);
    }
}