#include "render.h"

#include "../sdk.h"
#include "../global_context.h"

#include "../other/color.h"
#include "../other/byte_arrays.h"

#include "blur_x.h"
#include "blur_y.h"

#include <wrl/client.h>

create_feature_ptr(render);

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

private:
	D3DMATRIX backup_matrix{};

	IDirect3DSurface9* rt_backup = nullptr;
	IDirect3DTexture9* blur_texture1 = nullptr;
	IDirect3DTexture9* blur_texture2 = nullptr;

	c_shader_program blur_shaderx;
	c_shader_program blur_shadery;
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

	void create_textures() noexcept
	{
		if (!blur_texture1)
			blur_texture1 = create_texture(backbufferWidth / blur_down_sample, backbufferHeight / blur_down_sample);
		if (!blur_texture2)
			blur_texture2 = create_texture(backbufferWidth / blur_down_sample, backbufferHeight / blur_down_sample);
	}

	void create_shaders() noexcept
	{
		blur_shaderx.init(blur_x);
		blur_shadery.init(blur_y);
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
	}
}

void imgui_blur::create_blur(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col, float rounding, ImDrawCornerFlags round_flags) noexcept
{
	c_blur_effect::draw(drawList, min, max, col, rounding, round_flags);
}

void __stdcall c_render::create_objects()
{
	ImGui_ImplDX9_CreateDeviceObjects();
	draw_list = ImGui::GetBackgroundDrawList();
}

void __stdcall c_render::invalidate_objects()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	draw_list = nullptr;
}

void __stdcall c_render::start_render(IDirect3DDevice9* device)
{
	direct_device = device;

	ImGui::NewFrame();
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	if (!draw_list)
		draw_list = ImGui::GetBackgroundDrawList();
}

void __stdcall c_render::end_render(IDirect3DDevice9* device)
{
	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void __stdcall c_render::init(IDirect3DDevice9* device)
{
	static bool init = false;
	if (!init)
	{
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(g_ctx.window);
		ImGui_ImplDX9_Init(device);

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameRounding = 0.f;
		style.ChildRounding = 0.f;
		style.WindowRounding = 0.f;
		style.GrabMinSize = 1.f;

		ImFontConfig cfg;
		cfg.OversampleH = 3;
		cfg.OversampleV = 3;

		constexpr ImWchar icon_ranges[] = { 0xE000, 0xF8FF, // Private Use Area
		  0 };

		auto prev_flags = cfg.RasterizerFlags;
		cfg.RasterizerFlags = ImGuiFreeType::RasterizerFlags::MonoHinting | ImGuiFreeType::RasterizerFlags::Monochrome;
		g_fonts.pixel = io.Fonts->AddFontFromMemoryTTF(smallest_pixel_7, sizeof(smallest_pixel_7), 10.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		cfg.RasterizerFlags = prev_flags;

	//	cfg.RasterizerFlags = ImGuiFreeType::RasterizerFlags::ForceAutoHint;
		g_fonts.main = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.large = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 30.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());

		g_fonts.esp = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_Regular, sizeof(SFUIDisplay_Regular), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.pixel_menu = io.Fonts->AddFontDefault(&cfg);

		g_fonts.misc = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.bold = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_Bold, sizeof(SFUIDisplay_Bold), 10.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.bold2 = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_Bold, sizeof(SFUIDisplay_Bold), 14.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.bold_large = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_Bold, sizeof(SFUIDisplay_Bold), 26.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.dmg = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 18.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.eventlog = io.Fonts->AddFontFromMemoryTTF(SFUIDisplay_SemiBold, sizeof(SFUIDisplay_SemiBold), 12.f, &cfg, io.Fonts->GetGlyphRangesCyrillic());
		g_fonts.weapon_icons = io.Fonts->AddFontFromMemoryCompressedTTF(qo0icons_compressed_data, 38581, 13.f, &cfg, icon_ranges);
		g_fonts.weapon_icons_large = io.Fonts->AddFontFromMemoryCompressedTTF(qo0icons_compressed_data, 38581, 26.f, &cfg, icon_ranges);

		ImGuiFreeType::BuildFontAtlas(io.Fonts);
		ImGui_ImplDX9_CreateDeviceObjects();

		g_ctx.render_init = true;

		init = true;
	}
}

void c_render::update_screen_size()
{
	int w, h;
	interfaces::engine->get_screen_size(w, h);
	screen_size = rect2d(w, h);
}

void c_render::string(float x, float y, color clr, int flags, ImFont* font, const std::string& message)
{
	auto char_str = message.c_str();

	draw_list->PushTextureID(font->ContainerAtlas->TexID);

	ImGui::PushFont(font);

	auto coord = ImVec2(x, y);
	auto size = ImGui::CalcTextSize(char_str);
	auto coord_out = ImVec2{ coord.x + 1.f, coord.y + 1.f };
	color outline_clr = color(0, 0, 0, (flags & outline_light) || (flags & dropshadow_light) ? clr.a() * 0.35f : clr.a());

	int width = 0, height = 0;

	if (!(flags & centered_x))
		size.x = 0;
	if (!(flags & centered_y))
		size.y = 0;

	ImVec2 pos = ImVec2(coord.x - (size.x * .5f), coord.y - (size.y * .5f));

	if (flags & outline_ || flags & outline_light)
	{
		draw_list->AddText(ImVec2(pos.x + 1, pos.y - 1), outline_clr.as_imcolor(), char_str);
		draw_list->AddText(ImVec2(pos.x - 1, pos.y + 1), outline_clr.as_imcolor(), char_str);
		draw_list->AddText(ImVec2(pos.x - 1, pos.y - 1), outline_clr.as_imcolor(), char_str);
		draw_list->AddText(ImVec2(pos.x + 1, pos.y + 1), outline_clr.as_imcolor(), char_str);

		draw_list->AddText(ImVec2(pos.x, pos.y + 1), outline_clr.as_imcolor(), char_str);
		draw_list->AddText(ImVec2(pos.x, pos.y - 1), outline_clr.as_imcolor(), char_str);
		draw_list->AddText(ImVec2(pos.x + 1, pos.y), outline_clr.as_imcolor(), char_str);
		draw_list->AddText(ImVec2(pos.x - 1, pos.y), outline_clr.as_imcolor(), char_str);
	}

	if (flags & dropshadow_ || (flags & dropshadow_light))
		draw_list->AddText(ImVec2(pos.x + 1, pos.y + 1), outline_clr.as_imcolor(), char_str);

	draw_list->AddText(pos, clr.as_imcolor(), char_str);
	draw_list->PopTextureID();
	ImGui::PopFont();
}

void c_render::line(float x1, float y1, float x2, float y2, color clr, float thickness)
{
	draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), clr.as_imcolor(), thickness);
}

void c_render::line_gradient(float x1, float y1, float x2, float y2, color clr1, color cl2, float thickness)
{
	draw_list->AddRectFilledMultiColor(ImVec2(x1, y2), ImVec2(x2 + thickness, y2 + thickness), clr1.as_imcolor(), cl2.as_imcolor(), cl2.as_imcolor(), clr1.as_imcolor());
}

void c_render::blur(float x, float y, float w, float h, color clr, float rounding)
{
	imgui_blur::create_blur(draw_list, ImVec2(x, y), ImVec2(x + w, y + h), clr.as_imcolor(), rounding);
}

void c_render::rect(float x, float y, float w, float h, color clr, float rounding, float thickness)
{
	draw_list->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), clr.as_imcolor(), rounding, 15, thickness);
}

void c_render::filled_rect(float x, float y, float w, float h, color clr, float rounding, ImDrawCornerFlags rounding_corners)
{
	draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), clr.as_imcolor(), rounding, rounding_corners);
}

void c_render::filled_rect_gradient(float x, float y, float w, float h, color col_upr_left, color col_upr_right, color col_bot_right, color col_bot_left)
{
	draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + w, y + h), col_upr_left.as_imcolor(), col_upr_right.as_imcolor(), col_bot_right.as_imcolor(), col_bot_left.as_imcolor());
}

void c_render::arc(float x, float y, float radius, float min_angle, float max_angle, color col, float thickness)
{
	draw_list->PathArcTo(ImVec2(x, y), radius, math::deg_to_rad(min_angle), math::deg_to_rad(max_angle), 32);
	draw_list->PathStroke(col.as_imcolor(), true, thickness);
}

void c_render::triangle(float x1, float y1, float x2, float y2, float x3, float y3, color clr, float thickness)
{
	draw_list->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.as_imcolor(), thickness);
}

void c_render::filled_triangle_gradient(float x1, float y1, float x2, float y2, float x3, float y3, color clr, color clr2, color clr3)
{
	draw_list->AddTriangleFilledMulticolor(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.as_imcolor(), clr2.as_imcolor(), clr3.as_imcolor());
}

void c_render::filled_triangle(float x1, float y1, float x2, float y2, float x3, float y3, color clr)
{
	draw_list->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.as_imcolor());
}

void c_render::circle(float x1, float y1, float radius, color col, int segments)
{
	draw_list->AddCircle(ImVec2(x1, y1), radius, col.as_imcolor(), segments);
}

void c_render::filled_circle(float x1, float y1, float radius, color col, int segments)
{
	draw_list->AddCircleFilled(ImVec2(x1, y1), radius, col.as_imcolor(), segments);
}

const matrix3x4_t& c_render::world_to_screen_matrix()
{
	static auto view_matrix = *patterns::screen_matrix.add(0x3).as< uintptr_t* >();
	static auto ptr = view_matrix + 176;
	return *(matrix3x4_t*)(ptr);
}

bool c_render::screen_transform(const vector3d& source, vector2d& output)
{
	const matrix3x4_t& world_to_screen = this->world_to_screen_matrix();

	output.x = world_to_screen.mat[0][0] * source.x + world_to_screen.mat[0][1] 
		* source.y + world_to_screen.mat[0][2] * source.z + world_to_screen.mat[0][3];

	output.y = world_to_screen.mat[1][0] * source.x + world_to_screen.mat[1][1] 
		* source.y + world_to_screen.mat[1][2] * source.z + world_to_screen.mat[1][3];

	float w = world_to_screen.mat[3][0] * source.x + world_to_screen.mat[3][1] 
		* source.y + world_to_screen.mat[3][2] * source.z + world_to_screen.mat[3][3];

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

bool c_render::world_to_screen(const vector3d& source, vector2d& output, bool skip_screen)
{
	bool st = this->screen_transform(source, output);

	float x = screen_size.w / 2;
	float y = screen_size.h / 2;

	x += 0.5 * output.x * screen_size.w + 0.5;
	y -= 0.5 * output.y * screen_size.h + 0.5;

	output.x = x;
	output.y = y;

	if (!skip_screen && (output.x > screen_size.w || output.x < 0 || output.y > screen_size.h) || output.y < 0 || st)
		return false;

	return true;
}