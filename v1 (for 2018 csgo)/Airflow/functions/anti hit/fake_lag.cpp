#include "exploits.h"
#include "fake_lag.h"
#include "anti_aim.h"

#include "../config_vars.h"
#include "../features.h"
#include "../extra/movement.h"

#include "../../base/sdk.h"
#include "../../base/global_context.h"

#include "../../base/sdk/c_usercmd.h"
#include "../../base/sdk/c_animstate.h"
#include "../../base/sdk/entity.h"

int c_fake_lag::get_max_choke( )
{
  if( g_exploits->recharge )
    return 0;

  if( g_cfg.binds [ dt_b ].toggled || g_cfg.binds [ hs_b ].toggled )
    return 1;

  if( g_cfg.binds [ sw_b ].toggled )
    return std::clamp( g_cfg.antihit.fakewalk_speed, 0, 16 );

  if( g_anti_aim->is_fake_ducking( ) )
    return 14;

  if( g_cfg.antihit.fakelag )
    return g_cfg.antihit.fakelag_limit;

  return ( int )g_cfg.antihit.desync;
}

void c_fake_lag::bypass_choke_limit( )
{
  static int old_choke = 0;

  if( old_choke != g_ctx.max_choke )
  {
    auto address = patterns::send_move_addr.add( 1 ).as< uint8_t* >( );

    uint32_t choke_clamp = g_ctx.max_choke + 3;

    DWORD old_protect = 0;
    VirtualProtect( ( void* )address, sizeof( uint32_t ), PAGE_EXECUTE_READWRITE, &old_protect );
    *( uint32_t* )address = choke_clamp;
    VirtualProtect( ( void* )address, sizeof( uint32_t ), old_protect, &old_protect );

    old_choke = g_ctx.max_choke;
  }
}

int c_fake_lag::get_choke_amount( )
{
  if( g_cfg.binds [ sw_b ].toggled || g_rage_bot->should_slide || g_anti_aim->is_fake_ducking( ) )
    return this->get_max_choke( );

  auto state = g_ctx.local->animstate( );

  int max_choke = 1;

  bool landing = g_ctx.local->fall_velocity( ) > 0.f;
  bool standing = g_ctx.local->velocity( ).length( true ) < 10.f;

  if( ( g_cfg.antihit.fakelag_conditions & 1 ) && standing || ( g_cfg.antihit.fakelag_conditions & 2 ) && !( standing || !g_utils->on_ground( ) ) || ( g_cfg.antihit.fakelag_conditions & 4 ) && !g_utils->on_ground( ) && !landing ||
      ( g_cfg.antihit.fakelag_conditions & 8 ) && landing )
    max_choke = this->get_max_choke( );

  return max_choke;
}

void c_fake_lag::on_predict_start( )
{
  this->bypass_choke_limit( );

  if( interfaces::game_rules->is_freeze_time( ) || g_ctx.local->flags( ) & fl_frozen || g_ctx.local->gun_game_immunity( ) )
  {
    if( !*g_ctx.send_packet )
      *g_ctx.send_packet = true;
    return;
  }

  int choke_amount = this->get_choke_amount( );
  if( choke_amount == 0 || !g_cfg.binds [ sw_b ].toggled && g_utils->is_firing() && !g_anti_aim->is_fake_ducking( ) )
  {
    if( !*g_ctx.send_packet )
      *g_ctx.send_packet = true;
    return;
  }

  *g_ctx.send_packet = false;

  int choke = interfaces::client_state->choked_commands;

  // too much choked commands
  if( choke > 0 && std::abs( interfaces::global_vars->tick_count - g_ctx.sent_tick_count ) > choke_amount || choke >= choke_amount )
  {
    *g_ctx.send_packet = true;
    return;
  }
}

void c_ping_spike::on_procces_packet( )
{
  if( !g_cfg.binds [ spike_b ].toggled || !g_ctx.local || !g_ctx.local->is_alive( ) )
  {
    flipped_state = true;
    return;
  }

  auto netchan = interfaces::client_state->net_channel_ptr;
  if( !netchan )
    return;

  static auto last_reliable_state = -1;

  if( netchan->in_reliable_state != last_reliable_state )
    flipped_state = true;

  last_reliable_state = netchan->in_reliable_state;
}

void c_ping_spike::on_net_chan( c_netchan* netchan, float latency )
{
  if( flipped_state )
  {
    flipped_state = false;
    return;
  }

  int ticks = math::time_to_ticks( latency );
  if( netchan->in_sequence_nr > ticks )
    netchan->in_sequence_nr -= ticks;
}

void c_fake_lag::on_predict_end( )
{
  auto& correct = choked_ticks.emplace_front( );

  correct.cmd = g_ctx.cmd->command_number;
  correct.choke = interfaces::client_state->choked_commands + 1;
  correct.tickcount = interfaces::global_vars->tick_count;

  if( *g_ctx.send_packet )
    choked_commands.clear( );
  else
    choked_commands.emplace_back( correct.cmd );

  while( choked_ticks.size( ) > ( int )( 2.f / interfaces::global_vars->interval_per_tick ) )
    choked_ticks.pop_back( );

  auto& out = commands.emplace_back( );
  out.outgoing = *g_ctx.send_packet;
  out.used = false;
  out.cmd = g_ctx.cmd->command_number;
  out.prev_cmd = 0;
  while( commands.size( ) > g_ctx.tick_rate )
    commands.pop_front( );

  if( !*g_ctx.send_packet )
  {
    auto net_channel = interfaces::client_state->net_channel_ptr;
    if( net_channel->choked_packets > 0 && !( net_channel->choked_packets % 4 ) )
    {
      auto backup_choke = net_channel->choked_packets;
      net_channel->choked_packets = 0;

      net_channel->send_datagram( );
      --net_channel->out_sequence_nr;

      net_channel->choked_packets = backup_choke;
    }
  }
}

void c_fake_lag::on_local_death( )
{
  if( commands.size( ) > 0 )
    commands.clear( );

  if( choked_ticks.size( ) > 0 )
    choked_ticks.clear( );

  if( choked_commands.size( ) > 0 )
    choked_commands.clear( );
}