#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"
#include "../../../functions/config_vars.h"

#include "../../../base/sdk/entity.h"

namespace tr::convars
{
  int __fastcall cl_foot_contact_shadows_get_int( void* ecx, void* edx )
  {
    static auto original = vtables [ vtables_t::cl_foot_contact_shadows ].original< get_int_fn >( xor_int( 13 ) );
    if( g_ctx.uninject )
      return original( ecx );

    return 0;
  }

  int __fastcall sv_cheats_get_int( void* ecx, void* edx )
  {
    static auto original = vtables [ vtables_t::sv_cheats ].original< get_int_fn >( xor_int( 13 ) );
    if( g_ctx.uninject )
      return original( ecx );

    if( ( uintptr_t )_ReturnAddress( ) == patterns::return_addr_cam_think.as< uintptr_t >( ) )
      return 1;

    return original( ecx );
  }

  int __fastcall cl_csm_shadows_get_int( void* ecx, void* edx )
  {
    static auto original = vtables [ vtables_t::cl_csm_shadows ].original< get_int_fn >( xor_int( 13 ) );
    if( g_ctx.uninject || g_cfg.misc.world_modulation & 2 )
      return original( ecx );

    return !( g_cfg.misc.removals & shadow );
  }

  int __fastcall cl_brushfastpath_get_int( void* ecx, void* edx )
  {
    static auto original = vtables [ vtables_t::cl_brushfastpath ].original< get_int_fn >( xor_int( 13 ) );
    return original( ecx );
  }

  int __fastcall debug_show_spread_get_int( void* ecx, void* edx )
  {
    static auto original = vtables [ vtables_t::debug_show_spread ].original< get_int_fn >( xor_int( 13 ) );

    if( g_ctx.uninject || !g_ctx.local || !g_ctx.local->is_alive( ) )
      return original( ecx );

    c_basecombatweapon* weapon = g_ctx.weapon;
    if( !weapon )
      return original( ecx );

    if( !g_cfg.misc.snip_crosshair || g_ctx.local->is_scoped( ) )
      return original( ecx );

    if( !weapon || !weapon->is_scoping_weapon( ) )
      return original( ecx );

    return 3;
  }

  int __fastcall cl_clock_correction_get_int( void* ecx, void* edx )
  {
    static auto original = vtables [ vtables_t::cl_clock_correction ].original< get_int_fn >( xor_int( 13 ) );

    return original( ecx );
  }

  bool __fastcall net_showfragments_get_bool( void* ecx, void* edx )
  {
    static auto original = vtables [ vtables_t::cl_clock_correction ].original< get_bool_fn >( xor_int( 13 ) );
    return original( ecx );
  }
}