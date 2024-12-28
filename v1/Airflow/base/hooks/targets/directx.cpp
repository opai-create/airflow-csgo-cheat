#pragma once
#include "../hooks.h"
#include "../../../includes.h"

#include "../../sdk.h"
#include "../../global_context.h"

#include "../../tools/render.h"
#include "../../tools/key_states.h"

#include "../../../functions/features.h"

#include "../../../functions/config_vars.h"

#include <string>

namespace tr::direct
{
	//note from @cacamelio : hardcore shit !! those who know :skull:

	//HRESULT __stdcall present(IDirect3DDevice9* device, const RECT* src_rect, const RECT* dest_rect, HWND window_override, const RGNDATA* dirty_region)
	HRESULT __stdcall end_scene(IDirect3DDevice9* device)
	{

		static auto original = vtables[vmt_direct].original<decltype(&end_scene)>(xor_int(42));

		if (g_ctx.uninject)
		{
			//return original_present(device, src_rect, dest_rect, window_override, dirty_region);
			return original(device);
		}
			

		IDirect3DStateBlock9* d3d9_state_block = nullptr;
		/*
		if (device->CreateStateBlock(D3DSBT_PIXELSTATE, &d3d9_state_block) < 0)
			return original_present(device, src_rect, dest_rect, window_override, dirty_region);
			*/
		if (device->CreateStateBlock(D3DSBT_ALL, &d3d9_state_block) < 0)
			return original(device);

		d3d9_state_block->Capture();

		static DWORD color_write = NULL;
		static DWORD srgb_write = NULL;

		static DWORD anti_alias = NULL;
		static DWORD multi_sample_anti_alias = NULL;

		static IDirect3DVertexDeclaration9* vert_dec = nullptr;
		static IDirect3DVertexShader9* vert_shader = nullptr;

		device->GetRenderState(D3DRS_COLORWRITEENABLE, &color_write);
		device->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgb_write);

		device->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &multi_sample_anti_alias);
		device->GetRenderState(D3DRS_ANTIALIASEDLINEENABLE, &anti_alias);

		device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);
		device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, FALSE);

		device->GetVertexDeclaration(&vert_dec);
		device->GetVertexShader(&vert_shader);
		device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xffffffff);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_ADDRESSW, D3DTADDRESS_WRAP);
		device->SetSamplerState(NULL, D3DSAMP_SRGBTEXTURE, NULL);

		if (g_ctx.cheat_init && !g_cfg.misc.menu)
		{
			g_cfg.misc.menu = true;
			g_ctx.cheat_init = false;
			g_ctx.cheat_init2 = true;
		}

		g_ctx.update_animations();

		g_render->init(device);
		g_render->start_render(device);
		{
			imgui_blur::set_device(device);
			imgui_blur::new_frame();

			g_visuals_wrapper->on_directx();
			g_menu->draw();

			if (g_cfg.legit.enable)
			{
				if (g_cfg.rage.enable)
					g_cfg.rage.enable = false;
			}

			if (g_cfg.rage.enable)
			{
				if (g_cfg.legit.enable)
					g_cfg.legit.enable = false;
			}
		}
		g_render->end_render(device);

		device->SetRenderState(D3DRS_COLORWRITEENABLE, color_write);
		device->SetRenderState(D3DRS_SRGBWRITEENABLE, srgb_write);

		device->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, anti_alias);
		device->SetRenderState(D3DRS_ANTIALIASEDLINEENABLE, multi_sample_anti_alias);

		device->SetVertexDeclaration(vert_dec);
		device->SetVertexShader(vert_shader);

		d3d9_state_block->Apply();
		d3d9_state_block->Release();

		//return original_present(device, src_rect, dest_rect, window_override, dirty_region);
		return original(device);
	}

	HRESULT __stdcall reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* params)
	{

		static auto original = vtables[vmt_direct].original<decltype(&reset)>(xor_int(16));

		if (g_ctx.uninject)
		{
			//return original_reset(device, params);
			return original(device, params);
		}

		imgui_blur::on_device_reset();

		g_render->invalidate_objects();
		g_render->create_objects();

		//return original_reset(device, params);
		return original(device, params);
	}
}