#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../functions/config_vars.h"

#include "../../../base/tools/render.h"

#include "../../../base/sdk/entity.h"

#include "../../../functions/features.h"

#define MAX_COORD_FLOAT ( 16384.0f )
#define MIN_COORD_FLOAT ( -MAX_COORD_FLOAT )

namespace tr
{
  namespace client_state
  {
    void __fastcall packet_start( void* ecx, void* edx, int incoming, int outgoing )
    {
      static auto original = vtables [ vtables_t::client_state_ ].original< packet_start_fn >( xor_int( 5 ) );

      if( !g_ctx.local || !g_ctx.local->is_alive( ) || g_exploits->cl_move.shifting )
        return original( ecx, incoming, outgoing );

      auto& cmd = g_fake_lag->commands;
      if( cmd.empty( ) )
        return original( ecx, incoming, outgoing );

      for( auto it = cmd.rbegin( ); it != cmd.rend( ); ++it )
      {
        if( !it->outgoing )
          continue;

        if( it->cmd == outgoing || outgoing > it->cmd && ( !it->used || it->prev_cmd == outgoing ) )
        {
          it->prev_cmd = outgoing;
          it->used = true;
          original( ecx, incoming, outgoing );
          break;
        }
      }

      auto result = false;

      for( auto it = cmd.begin( ); it != cmd.end( ); )
      {
        if( outgoing == it->cmd || outgoing == it->prev_cmd )
          result = true;

        if( outgoing > it->cmd && outgoing > it->prev_cmd )
          it = cmd.erase( it );
        else
          ++it;
      }

      if( !result )
        original( ecx, incoming, outgoing );
    }

    void __fastcall packet_end( void* ecx, void* edx )
    {
      static auto original = vtables [ vtables_t::client_state_ ].original< packet_end_fn >( xor_int( 6 ) );
      if( !g_ctx.local || !g_ctx.local->is_alive( ) )
        return original( ecx );

      auto clientstate = ( c_clientstate* )ecx;

      if( clientstate->clock_drift_mgr.cur_clock_offset == clientstate->clock_drift_mgr.server_tick )
      {
        auto ack_cmd = clientstate->last_command_ack;

        auto correct = std::find_if( g_fake_lag->choked_ticks.begin( ), g_fake_lag->choked_ticks.end( ), [ &ack_cmd ]( const choked_ticks_t& other_data ) { return other_data.cmd == ack_cmd; } );

        if( correct != g_fake_lag->choked_ticks.end( ) )
        {
          if( g_ctx.velocity_modifier > g_ctx.local->velocity_modifier( ) + 0.1f )
          {
            auto weapon = g_ctx.weapon;

            if( !weapon || weapon && weapon->item_definition_index( ) != weapon_revolver && !weapon->is_grenade( ) )
            {
              for( auto& number : g_fake_lag->choked_commands )
              {
                auto cmd = &interfaces::input->commands [ number % 150 ];
                auto verified = &interfaces::input->verified_commands [ number % 150 ];

                if( cmd->buttons & ( in_attack | in_attack2 ) )
                {
                  cmd->buttons &= ~in_attack;

                  verified->cmd = *cmd;
                  verified->crc = cmd->get_check_sum( );
                }
              }
            }
          }

          if( g_ctx.velocity_modifier != g_ctx.local->velocity_modifier( ) )
            g_ctx.velocity_modifier = g_ctx.local->velocity_modifier( );
        }
      }

      return original( ecx );
    }
  }

  namespace engine
  {
    int __fastcall list_leaves_in_box( void* ecx, void* edx, const vector3d& mins, const vector3d& maxs, unsigned short* list, int list_max )
    {
      static auto original = vtables [ vtables_t::engine_bsp ].original< list_leaves_in_box_fn >( xor_int( 6 ) );

      if( ( uintptr_t )_ReturnAddress( ) != patterns::list_leaves_in_box.as< uintptr_t >( ) )
        return original( ecx, mins, maxs, list, list_max );

      auto info = *( renderable_info_t** )( ( uintptr_t )_AddressOfReturnAddress( ) + 0x14 );

      if( !info )
        return original( ecx, mins, maxs, list, list_max );

      if( !info->m_pRenderable )
        return original( ecx, mins, maxs, list, list_max );

      c_baseentity* entity = info->m_pRenderable->get_i_unknown_entity( )->get_base_entity( );

      if( !entity || !entity->is_player( ) )
        return original( ecx, mins, maxs, list, list_max );

      auto player = ( c_csplayer* )entity;
      if( !player->is_alive( ) || player == g_ctx.local )
        return original( ecx, mins, maxs, list, list_max );

      info->m_Flags &= ~0x100;
      info->m_nTranslucencyType = 2;

      static const vector3d map_min = vector3d( MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT );
      static const vector3d map_max = vector3d( MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT );

      return original( ecx, map_min, map_max, list, list_max );
    }

    bool __fastcall temp_entities( c_clientstate* ecx, void* edx, void* msg )
    {
      static auto original = hooker.original( &temp_entities );

      auto old_max_clients = ecx->max_clients;
      ecx->max_clients = 1;

      bool ret = original( ecx, edx, msg );

      ecx->max_clients = old_max_clients;

      return ret;
    }

    bool __fastcall send_net_msg( i_net_channel_info* ecx, void* edx, c_net_message& msg, bool force_reliable, bool voice )
    {
      static auto original = hooker.original( &send_net_msg );

      if( ecx != interfaces::engine->get_net_channel_info( ) )
        return original( ecx, edx, msg, force_reliable, voice );

      if( msg.get_type( ) == 14 )
        return false;

      return original( ecx, edx, msg, force_reliable, voice );
    }

    bool __fastcall using_static_props_debug( void* ecx, void* edx )
    {
      return true;
    }

    float __fastcall get_screen_aspect_ratio( void* ecx, void* edx, int width, int height )
    {
      static auto original = vtables [ vtables_t::engine_ ].original< get_screen_aspect_ratio_fn >( xor_int( 101 ) );
      return g_cfg.misc.aspect_ratio > 0 ? g_cfg.misc.aspect_ratio / 100.f : original( ecx, width, height );
    }

    void __fastcall check_file_crc_with_server( void* ecx, void* edx )
    {
      return;
    }

    bool __fastcall is_connected( void* ecx, void* edx )
    {
      static auto original = vtables [ vtables_t::engine_ ].original< is_connected_fn >( xor_int( 27 ) );

      if( g_cfg.misc.unlock_inventory && ( uintptr_t )_ReturnAddress( ) == patterns::return_addr_loadout_allowed.as< uintptr_t >( ) )
        return false;

      return original( ecx );
    }

    bool __fastcall is_paused( void* ecx, void* edx )
    {
      static auto original = vtables [ vtables_t::engine_ ].original< is_paused_fn >( xor_int( 90 ) );
      return original( ecx );
    }

    bool __fastcall is_hltv( void* ecx, void* edx )
    {
      static auto original = vtables [ vtables_t::engine_ ].original< is_hltv_fn >( xor_int( 93 ) );

      if( ( uintptr_t )_ReturnAddress( ) == patterns::return_addr_setup_velocity.as< uintptr_t >( ) || ( uintptr_t )_ReturnAddress( ) == patterns::return_addr_accumulate_layers.as< uintptr_t >( ) ||
          ( uintptr_t )_ReturnAddress( ) == patterns::return_addr_reevaluate_anim_lod.as< uintptr_t >( ) )
        return true;

      return original( ecx );
    }

    void __vectorcall read_packets( bool final_tick )
    {
      static auto original = hooker.original( &read_packets );
    //  if( !ping_reducer::should_work( ) )
        original( final_tick );
    }

    void __vectorcall cl_move( float accumulated_extra_samples, bool final_tick )
    {
      static auto original = hooker.original( &cl_move );
    //  ping_reducer::update_ping_values( final_tick );

      if( g_ctx.is_alive && g_ctx.cmd && g_exploits->recharging( g_ctx.cmd ) )
        return;

      original( accumulated_extra_samples, final_tick );

    //  g_exploits->on_cl_move( accumulated_extra_samples, final_tick );
    }

    int __fastcall send_datagram( void* netchan, void* edx, void* datagram )
    {
      static auto original = hooker.original( &send_datagram );

      if( g_ctx.uninject || !g_ctx.in_game || !g_cfg.binds [ spike_b ].toggled || netchan != interfaces::client_state->net_channel_ptr )
        return original( netchan, edx, datagram );

      auto netchan_ptr = ( c_netchan* )netchan;
      auto netchannel_info = interfaces::engine->get_net_channel_info( );

      float spike_amount = ( g_cfg.rage.spike_amt / 1000.f );
      if( g_cfg.rage.adaptive_spike )
      {
        spike_amount -= netchannel_info->get_latency( flow_outgoing );
        spike_amount -= g_ctx.lerp_time;
      }

      float correct = std::max< float >( 0.f, spike_amount );

      auto old_sequence = netchan_ptr->in_sequence_nr;
      auto old_state = netchan_ptr->in_reliable_state;

      g_ping_spike->on_net_chan( netchan_ptr, correct );

      auto ret = original( netchan, edx, datagram );

      netchan_ptr->in_sequence_nr = old_sequence;
      netchan_ptr->in_reliable_state = old_state;

      return ret;
    }

    void __fastcall process_packet( c_netchan* ecx, void* edx, void* packet, bool header )
    {
      static auto original = hooker.original( &process_packet );
      original( ecx, edx, packet, header );

      g_ping_spike->on_procces_packet( );
    }
  }
}