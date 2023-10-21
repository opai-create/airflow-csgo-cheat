#include <thread>
#include "key_states.h"

#include "../global_context.h"
#include "../../functions/config_vars.h"
#include "../../functions/features.h"

#include "../sdk.h"

create_feature_ptr(key_states);

c_key_states::c_key_states()
{
	std::thread(
		[this]
		{
			while (!g_ctx.uninject)
			{
				for (int i = 1; i < 255; ++i)
					array_states[i] = GetAsyncKeyState(i);

				static constexpr DWORD sleep_time = 1000 / 64;

				if (g_ctx.tick_rate > 0)
					std::this_thread::sleep_for(std::chrono::milliseconds(1000 / g_ctx.tick_rate));
				else
					std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
			}
		})
		.detach();
}

bool c_key_states::proc_key(int idx, int key, bool state)
{
	return idx == key && state;
}

c_key_states::key_info_t c_key_states::get_key_state(UINT uMsg, WPARAM wParam)
{
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONDBLCLK:
		return { VK_LBUTTON, true };
	case WM_LBUTTONUP:
		return { VK_LBUTTON, false };
	case WM_RBUTTONDOWN:
	case WM_RBUTTONDBLCLK:
		return { VK_RBUTTON, true };
	case WM_RBUTTONUP:
		return { VK_RBUTTON, false };
	case WM_MBUTTONDOWN:
	case WM_MBUTTONDBLCLK:
		return { VK_MBUTTON, true };
	case WM_MBUTTONUP:
		return { VK_MBUTTON, false };
	case WM_XBUTTONDOWN:
	case WM_XBUTTONDBLCLK:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
			return { VK_XBUTTON1, true };
		else if (button == XBUTTON2)
			return { VK_XBUTTON2, true };

		break;
	}
	case WM_XBUTTONUP:
	{
		UINT button = GET_XBUTTON_WPARAM(wParam);
		if (button == XBUTTON1)
			return { VK_XBUTTON1, false };
		else if (button == XBUTTON2)
			return { VK_XBUTTON2, false };

		break;
	}
	case WM_KEYDOWN:
		return { wParam, true };
	case WM_KEYUP:
		return { wParam, false };
	case WM_SYSKEYDOWN:
		return { wParam, true };
	case WM_SYSKEYUP:
		return { wParam, false };
	}

	return { 0, false };
}

bool c_key_states::key_updated(int key, UINT uMsg, WPARAM wParam)
{
	auto key_state = get_key_state(uMsg, wParam);
	return key == key_state.key && key_state.state;
}

void c_key_states::update(UINT uMsg, WPARAM wParam)
{
	if (g_ctx.open_console)
		return;

	for (int i = 0; i < binds_max; ++i)
	{
		auto& b = g_cfg.binds[i];

		if (b.key == -1 && b.type != 0)
		{
			b.toggled = false;
			continue;
		}

		switch (b.type)
		{
		case 0:
			b.toggled = true;
			break;
		case 1:
			b.toggled = this->at(b.key);
			break;
		}

		if (b.type == 2)
		{
			bool key_changed = key_updated(b.key, uMsg, wParam);

			if (i == left_b || i == right_b || i == back_b)
			{
				if (!g_cfg.antihit.enable)
				{
					b.toggled = false;
					continue;
				}

				b.type = 2;
				if (b.key > 0 && key_changed)
				{
					if (i == left_b)
					{
						b.toggled = !b.toggled;
						if (b.toggled)
						{
							g_cfg.binds[right_b].toggled = false;
							g_cfg.binds[back_b].toggled = false;
						}
					}
					else if (i == right_b)
					{
						b.toggled = !b.toggled;
						if (b.toggled)
						{
							g_cfg.binds[back_b].toggled = false;
							g_cfg.binds[left_b].toggled = false;
						}
					}
					else if (i == back_b)
					{
						b.toggled = !b.toggled;
						if (b.toggled)
						{
							g_cfg.binds[right_b].toggled = false;
							g_cfg.binds[left_b].toggled = false;
						}
					}
				}
			}
			else
			{
				if (key_changed)
					b.toggled = !b.toggled;
			}
		}
	}
}