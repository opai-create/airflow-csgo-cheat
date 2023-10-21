#include "globals.hpp"
#include <wrl/client.h>
#include "imgui/shader_blur_x.h"
#include "imgui/shader_blur_y.h"

using Microsoft::WRL::ComPtr;

static int backbufferWidth = 0;
static int backbufferHeight = 0;

static IDirect3DDevice9* device;

[[nodiscard]] static IDirect3DTexture9* create_texture(int width, int height) noexcept
{
	IDirect3DTexture9* texture;
	device->CreateTexture(width, height, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &texture, nullptr);
	return texture;
}

static void copy_back_buffer_to_texture(IDirect3DTexture9* texture, D3DTEXTUREFILTERTYPE filtering) noexcept
{
	ComPtr< IDirect3DSurface9 > backBuffer;
	if (device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, backBuffer.GetAddressOf()) == D3D_OK)
	{
		ComPtr< IDirect3DSurface9 > surface;
		if (texture->GetSurfaceLevel(0, surface.GetAddressOf()) == D3D_OK)
			device->StretchRect(backBuffer.Get(), nullptr, surface.Get(), nullptr, filtering);
	}
}

static void set_render_target(IDirect3DTexture9* rtTexture) noexcept
{
	ComPtr< IDirect3DSurface9 > surface;
	if (rtTexture->GetSurfaceLevel(0, surface.GetAddressOf()) == D3D_OK)
		device->SetRenderTarget(0, surface.Get());
}

class c_shader_program
{
public:
	~c_shader_program()
	{
	}

	void use(float uniform, int location) const noexcept
	{
		device->SetPixelShader(pixel_shader.Get());
		const float params[4] = { uniform };
		device->SetPixelShaderConstantF(location, params, 1);
	}

	void init(const BYTE* pixelShaderSrc) noexcept
	{
		if (initialized)
			return;

		initialized = true;
		device->CreatePixelShader(reinterpret_cast<const DWORD*>(pixelShaderSrc), pixel_shader.GetAddressOf());
	}

private:
	ComPtr< IDirect3DPixelShader9 > pixel_shader{};
	bool initialized = false;
};

class c_blur_effect
{
public:
	static void draw(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col, float rounding, ImDrawCornerFlags round_flags) noexcept
	{
		instance()._draw(drawList, min, max, col, rounding, round_flags);
	}

	static void clear_textures() noexcept
	{
		if (instance().blur_texture1)
		{
			instance().blur_texture1->Release();
			instance().blur_texture1 = nullptr;
		}
		if (instance().blur_texture2)
		{
			instance().blur_texture2->Release();
			instance().blur_texture2 = nullptr;
		}
	}

	static void create_textures() noexcept
	{
		if (!instance().blur_texture1)
			instance().blur_texture1 = create_texture(backbufferWidth / blur_down_sample, backbufferHeight / blur_down_sample);
		if (!instance().blur_texture2)
			instance().blur_texture2 = create_texture(backbufferWidth / blur_down_sample, backbufferHeight / blur_down_sample);
	}

	static void create_shaders() noexcept
	{
		instance().blur_shaderx.init(blur_x);
		instance().blur_shadery.init(blur_y);
	}

private:
	D3DMATRIX backup_matrix{};

	IDirect3DSurface9* rt_backup = nullptr;
	IDirect3DTexture9* blur_texture1 = nullptr;
	IDirect3DTexture9* blur_texture2 = nullptr;

	c_shader_program blur_shaderx{};
	c_shader_program blur_shadery{};
	static constexpr auto blur_down_sample = 9;

	c_blur_effect() = default;
	c_blur_effect(const c_blur_effect&) = delete;

	~c_blur_effect()
	{
		if (rt_backup)
			rt_backup->Release();
		if (blur_texture1)
			blur_texture1->Release();
		if (blur_texture2)
			blur_texture2->Release();
	}

	static c_blur_effect& instance() noexcept
	{
		static c_blur_effect blurEffect;
		return blurEffect;
	}

	static void begin(const ImDrawList*, const ImDrawCmd*) noexcept
	{
		instance()._begin();
	}
	static void first_pass(const ImDrawList*, const ImDrawCmd*) noexcept
	{
		instance()._first_pass();
	}
	static void second_pass(const ImDrawList*, const ImDrawCmd*) noexcept
	{
		instance()._second_pass();
	}
	static void end(const ImDrawList*, const ImDrawCmd*) noexcept
	{
		instance()._end();
	}

	void _begin() noexcept
	{
		device->GetRenderTarget(0, &rt_backup);
		copy_back_buffer_to_texture(blur_texture1, D3DTEXF_LINEAR);
		device->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
		device->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, false);

		device->GetVertexShaderConstantF(0, &backup_matrix.m[0][0], 4);

		const D3DMATRIX projection{ { { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -1.0f / (backbufferWidth / blur_down_sample), 1.0f / (backbufferHeight / blur_down_sample), 0.0f, 1.0f } } };
		device->SetVertexShaderConstantF(0, &projection.m[0][0], 4);
	}

	void _first_pass() noexcept
	{
		blur_shaderx.use(1.0f / (backbufferWidth / blur_down_sample), 0);
		set_render_target(blur_texture2);
	}

	void _second_pass() noexcept
	{
		blur_shadery.use(1.0f / (backbufferHeight / blur_down_sample), 0);
		set_render_target(blur_texture1);
	}

	void _end() noexcept
	{
		device->SetRenderTarget(0, rt_backup);
		rt_backup->Release();

		device->SetPixelShader(nullptr);
		device->SetRenderState(D3DRS_SCISSORTESTENABLE, true);
	}

	void _draw(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col, float rounding, ImDrawCornerFlags round_flags = 15) noexcept
	{
		create_textures();
		create_shaders();

		if (!blur_texture1 || !blur_texture2)
			return;
		drawList->AddCallback(&begin, nullptr);
		for (int i = 0; i < 8; ++i)
		{
			drawList->AddCallback(&first_pass, nullptr);
			drawList->AddImage(reinterpret_cast<ImTextureID>(blur_texture1), { -1.0f, -1.0f }, { 1.0f, 1.0f });
			drawList->AddCallback(&second_pass, nullptr);
			drawList->AddImage(reinterpret_cast<ImTextureID>(blur_texture2), { -1.0f, -1.0f }, { 1.0f, 1.0f });
		}
		drawList->AddCallback(&end, nullptr);
		drawList->AddCallback(ImDrawCallback_ResetRenderState, nullptr);
		drawList->AddImageRounded(reinterpret_cast<ImTextureID>(blur_texture1), min, max, { min.x / backbufferWidth, min.y / backbufferHeight }, { max.x / backbufferWidth, max.y / backbufferHeight },
			ImGui::GetColorU32(col.Value), rounding, round_flags);
	}
};

void imgui_blur::set_device(IDirect3DDevice9* device) noexcept
{
	::device = device;
}

void imgui_blur::clear_blur_textures() noexcept
{
	c_blur_effect::clear_textures();
}

void imgui_blur::on_device_reset() noexcept
{
	c_blur_effect::clear_textures();
}

void imgui_blur::new_frame() noexcept
{
	const int width = ImGui::GetIO().DisplaySize.x;
	const int height = ImGui::GetIO().DisplaySize.y;
	if (backbufferWidth != static_cast<int>(width) || backbufferHeight != static_cast<int>(height))
	{
		c_blur_effect::clear_textures();
		backbufferWidth = static_cast<int>(width);
		backbufferHeight = static_cast<int>(height);
		return;
	}
}

void imgui_blur::create_blur(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col, float rounding, ImDrawCornerFlags round_flags) noexcept
{
	c_blur_effect::draw(drawList, min, max, col, rounding, round_flags);
}

inline void init_custom_style(ImGuiStyle& style)
{
    style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.85f, 0.85f, 0.85f, 0.85f);
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.70f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.14f, 0.16f, 0.19f, 0.60f);
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 0.30f, 0.10f, 0.50f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.20f, 0.24f, 0.28f, 0.75f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.70f);
    style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.50f, 0.50f, 0.50f, 0.70f);
    style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_Header] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.90f, 0.90f, 0.90f, 0.75f);
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.30f, 0.10f, 0.50f);
    style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.30f, 0.10f, 0.50f);
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 0.30f, 0.10f, 1.00f);
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.07f, 0.70f);

    style.Alpha = 1.f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.FramePadding = ImVec2(1, 1);
    style.ScrollbarSize = 10.f;
    style.ScrollbarRounding = 0.f;
    style.GrabMinSize = 5.f;
}

void c_render::list_start()
{
	draw_list->Clear();
	draw_list->PushClipRect({}, { screen.x, screen.y });
}

void c_render::list_end()
{
    draw_list->PopClipRect();

    // swap our drawlist
    mutex.lock();
    *temp_draw_list = *draw_list;
    mutex.unlock();
}

void c_render::draw()
{
    if (screen.x == 0.f && screen.y == 0.f)
        return;

	if (temp_draw_list->VtxBuffer.Size > 0) 
	{
		mutex.lock();

		ImDrawList* dl = temp_draw_list;

		ImDrawData drawData{ };
		drawData.Valid = true;
		drawData.CmdLists = &dl;
		drawData.CmdListsCount = 1;
		drawData.TotalVtxCount = dl->VtxBuffer.size();
		drawData.TotalIdxCount = dl->IdxBuffer.size();

		drawData.DisplayPos = ImVec2{ 0.0f, 0.0f };
		drawData.DisplaySize = ImVec2{ screen.x, screen.y };
		drawData.FramebufferScale = ImVec2{ 1.0f, 1.0f };

		ImGui_ImplDX9_RenderDrawData(&drawData);

		mutex.unlock();
	}
}

void c_render::update_animation_speed()
{
	if (last_time_updated == -1.f)
		last_time_updated = HACKS->system_time();

	animation_speed = std::fabsf(last_time_updated - HACKS->system_time()) * 5.f;
	last_time_updated = HACKS->system_time();
}

bool c_render::init()
{
    if (done)
        return true;

    if (!ImGui::CreateContext())
        return false;

    if (!ImGui_ImplWin32_Init(HACKS->window))
        return false;

    if (!ImGui_ImplDX9_Init(device))
        return false;

    auto& style = ImGui::GetStyle();
    style.AntiAliasedLines = false;
    style.AntiAliasedFill = false;

    auto& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

	ImFontConfig cfg;
	cfg.OversampleH = 3;
	cfg.OversampleV = 3;

	constexpr ImWchar icon_ranges[] = { 0xE000, 0xF8FF, 0 };
	{
		RESTORE(cfg.RasterizerFlags);

		cfg.RasterizerFlags = ImGuiFreeType::RasterizerFlags::MonoHinting | ImGuiFreeType::RasterizerFlags::Monochrome;
		fonts.pixel.init(io, smallest_pixel_7, sizeof(smallest_pixel_7), 10.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		fonts.eventlog.init(io, CXOR("C:\\Windows\\Fonts\\lucon.ttf"), 10.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	}

	//	cfg.RasterizerFlags = ImGuiFreeType::RasterizerFlags::ForceAutoHint;
	fonts.main.init(io, SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.large.init(io, SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 30.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.esp.init(io, CXOR("C:\\Windows\\Fonts\\verdana.ttf"), 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.misc.init(io, SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.bold.init(io, SFUIDisplay_Bold, sizeof(SFUIDisplay_Bold), 10.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.bold2.init(io, SFUIDisplay_Bold, sizeof(SFUIDisplay_Bold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.bold_large.init(io, SFUIDisplay_Bold, sizeof(SFUIDisplay_Bold), 26.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.dmg.init(io, SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 18.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
	fonts.weapon_icons.init_compressed(io, qo0icons_compressed_data, 38581, 13.f, &cfg, icon_ranges);
	fonts.weapon_icons_large.init_compressed(io, qo0icons_compressed_data, 38581, 24.f, &cfg, icon_ranges);

    ImGuiFreeType::BuildFontAtlas(io.Fonts);

    init_custom_style(style);

    if (!ImGui_ImplDX9_CreateDeviceObjects())
        return false;

    draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
    temp_draw_list = new ImDrawList(ImGui::GetDrawListSharedData());
    second_temp_draw_list = new ImDrawList(ImGui::GetDrawListSharedData());

    done = true;
    return false;
}

void c_render::begin()
{
    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

	ImGui::GetIO( ).DisplaySize.x = RENDER->screen.x;
	ImGui::GetIO( ).DisplaySize.y = RENDER->screen.y;
}

void c_render::end()
{
	ImGui::Render();

	draw();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

const matrix3x4_t& c_render::world_to_screen_matrix()
{
	static auto view_matrix = *offsets::world_to_screen_matrix.add(0x3).cast<std::uintptr_t*>();
	static auto ptr = view_matrix + 176;
	return *(matrix3x4_t*)(ptr);
}

bool c_render::screen_transform(const vec3_t& source, vec2_t& output)
{
    const matrix3x4_t& world_to_screen = world_to_screen_matrix();

    output.x = world_to_screen.mat[0][0] * source.x + world_to_screen.mat[0][1] * source.y + world_to_screen.mat[0][2] * source.z + world_to_screen.mat[0][3];
    output.y = world_to_screen.mat[1][0] * source.x + world_to_screen.mat[1][1] * source.y + world_to_screen.mat[1][2] * source.z + world_to_screen.mat[1][3];
    float w = world_to_screen.mat[3][0] * source.x + world_to_screen.mat[3][1] * source.y + world_to_screen.mat[3][2] * source.z + world_to_screen.mat[3][3];

    bool behind = false;
    if (w < 0.001f)
    {
        behind = true;

        float invw = -1.0f / w;
        output.x *= invw;
        output.y *= invw;
    }
    else
    {
        behind = false;

        float invw = 1.0f / w;
        output.x *= invw;
        output.y *= invw;
    }
    return behind;
}

bool c_render::world_to_screen(const vec3_t& source, vec2_t& output, bool oob)
{
    bool st = screen_transform(source, output);

    float x = screen.x / 2;
    float y = screen.y / 2;

    x += 0.5f * output.x * screen.x + 0.5f;
    y -= 0.5f * output.y * screen.y + 0.5f;

    output.x = x;
    output.y = y;

    if (!oob && (output.x > screen.x || output.x < 0 || output.y > screen.y) || output.y < 0 || st)
        return false;

    return true;
}