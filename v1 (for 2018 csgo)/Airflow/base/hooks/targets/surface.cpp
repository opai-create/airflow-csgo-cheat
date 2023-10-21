#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../base/tools/render.h"

namespace tr::surface
{
  void __fastcall on_screen_size_changed( void* ecx, void* edx, int old_w, int old_h )
  {
    static auto original = vtables [ vtables_t::surface ].original< on_screen_size_changed_fn >( xor_int( 116 ) );
    original( ecx, old_w, old_h );
    g_render->update_screen_size( );
  }
}