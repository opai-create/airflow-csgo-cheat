#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../../base/tools/render.h"
#include "../../../base/tools/key_states.h"

#include "../../../functions/config_vars.h"

#include "../../../additionals/imgui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

__forceinline void toggle_value(int key, bool& flip, WPARAM wparam, UINT msg)
{
	if (wparam == key)
	{
		if (msg == WM_KEYUP)
			flip = !flip;
	}
}

namespace tr
{
	LRESULT __stdcall wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (g_ctx.window != GetActiveWindow())
			return CallWindowProcA(g_ctx.backup_window, hwnd, msg, wparam, lparam);

		toggle_value(VK_INSERT, g_cfg.misc.menu, wparam, msg);
		toggle_value(VK_DELETE, g_cfg.misc.menu, wparam, msg);

#ifdef _DEBUG
		toggle_value(VK_NEXT, g_ctx.uninject, wparam, msg);
#endif

		if (g_ctx.render_init && g_cfg.misc.menu)
		{
			if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam) || ImGui::GetIO().WantTextInput)
				return true;
		}

		g_key_states->update(msg, wparam);

		return CallWindowProcA(g_ctx.backup_window, hwnd, msg, wparam, lparam);
	}
}