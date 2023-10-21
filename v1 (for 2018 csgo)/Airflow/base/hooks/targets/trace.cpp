#include "../hooks.h"
#include "../../../includes.h"
#include "../../sdk.h"

#include <string>

namespace tr::trace
{
  void __fastcall clip_ray_to_collideable( void* ecx, void* edx, const ray_t& ray, unsigned int mask, c_collideable* collide, c_game_trace* trace )
  {
    static auto original = vtables [ vtables_t::trace ].original< clip_ray_to_collideable_fn >( xor_int( 4 ) );
    original( ecx, ray, mask, collide, trace );
  }

  void __fastcall trace_ray( void* ecx, void* edx, const ray_t& ray, unsigned int mask, i_trace_filter* filter, c_game_trace* trace )
  {
    static auto original = vtables [ vtables_t::trace ].original< trace_ray_fn >( xor_int( 5 ) );
    original( ecx, ray, mask, filter, trace );
  }

  bool __fastcall trace_filter_for_head_collision( void* ecx, void* edx, c_csplayer* player, void* trace_params )
  {
    static auto original = hooker.original( &trace_filter_for_head_collision );

    if( !g_ctx.local || !g_ctx.local->is_alive( ) )
      return original( ecx, edx, player, trace_params );

    if( !player || !player->is_player( ) || player->index( ) - 1 > 63 || player == g_ctx.local )
      return original( ecx, edx, player, trace_params );

    if( std::abs( player->origin( ).z - g_ctx.local->origin( ).z ) < 10.f )
      return false;

    return original( ecx, edx, player, trace_params );
  }
}