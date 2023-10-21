#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../base/tools/render.h"

namespace tr::surface
{
	void __fastcall on_screen_size_changed(void* ecx, void* edx, int old_w, int old_h)
	{
		static auto original = vtables[vmt_surface].original<decltype(&on_screen_size_changed)>(xor_int(116));
		original(ecx, edx, old_w, old_h);
		g_render->update_screen_size();
	}
}