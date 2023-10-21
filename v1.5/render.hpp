#pragma once
#include <mutex>

enum font_flags_t : int
{
    FONT_CENTERED_X = 1 << 0,
    FONT_CENTERED_Y = 1 << 1,
    FONT_DROPSHADOW = 1 << 2,
    FONT_OUTLINE = 1 << 3,
    FONT_LIGHT_BACK = 1 << 4,
};

namespace imgui_blur
{
    void set_device(IDirect3DDevice9* device) noexcept;
    void clear_blur_textures() noexcept;
    void on_device_reset() noexcept;
    void new_frame() noexcept;
    void create_blur(ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col = ImColor(255, 255, 255, 255), float rounding = 0.f, ImDrawCornerFlags round_flags = 15) noexcept;
}

class c_d3dfont
{
private:
    ImFont* ptr;
    float size;
public:
    INLINE void init(const ImGuiIO& io, unsigned char* bytes, unsigned int bytes_size, float font_size, ImFontConfig* cfg, const ImWchar* ranges)
    {
        ptr = io.Fonts->AddFontFromMemoryTTF(bytes, bytes_size, font_size, cfg, ranges);
        size = font_size;
    }

    INLINE void init_compressed(const ImGuiIO& io, void* bytes, unsigned int bytes_size, float font_size, ImFontConfig* cfg, const ImWchar* ranges)
    {
        ptr = io.Fonts->AddFontFromMemoryCompressedTTF(bytes, bytes_size, font_size, cfg, ranges);
        size = font_size;
    }

    INLINE void init(const ImGuiIO& io, const char* name, float font_size, ImFontConfig* cfg, const ImWchar* ranges)
    {
        ptr = io.Fonts->AddFontFromFileTTF(name, font_size, cfg, ranges);
        size = font_size;
    }

    INLINE ImFont* get() 
    {
        return ptr;
    }

    INLINE float get_size()
    {
        return size;
    }
};

class c_render
{
private:
    struct fonts_t
    {
        c_d3dfont main{};
        c_d3dfont misc{};
        c_d3dfont esp{};
        c_d3dfont bold{};
        c_d3dfont bold2{};
        c_d3dfont bold_large{};
        c_d3dfont dmg{};
        c_d3dfont large{};
        c_d3dfont eventlog{};
        c_d3dfont pixel_menu{};
        c_d3dfont pixel{};
        c_d3dfont weapon_icons{};
        c_d3dfont weapon_icons_large{};
    };

    float last_time_updated;
    float animation_speed;

    c_d3d_device* device;

    ImDrawList* draw_list;
    ImDrawList* temp_draw_list;
    ImDrawList* second_temp_draw_list;

    std::mutex mutex;

    const matrix3x4_t& world_to_screen_matrix();
    bool screen_transform(const vec3_t& source, vec2_t& output);

public:
    bool done;
    IDirect3DStateBlock9* d3d9_state_block;
    vec2_t screen;
    fonts_t fonts;

    INLINE ImDrawList* get_draw_list()
    {
        return draw_list;
    }

    INLINE void update_screen(bool hook = false)
    {
        if (!hook && screen.x > 0.f && screen.y > 0.f)
            return;

        int w, h;
        HACKS->engine->get_screen_size(w, h);

        screen.x = (float)w;
        screen.y = (float)h;
    }

    INLINE void set_device(c_d3d_device* device)
    {
        this->device = device;
    }

    INLINE c_d3d_device* get_device() 
    {
        return this->device;
    }

    INLINE float get_animation_speed()
    {
        return animation_speed;
    }

    INLINE void filled_rect(float x, float y, float w, float h, c_color color, float rounding = 0.f, ImDrawCornerFlags flags = 0)
    {
        draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), color.as_imcolor(), rounding, flags);
    }

    INLINE void line(float x1, float y1, float x2, float y2, c_color clr, float thickness = 1.f)
    {
        draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), clr.as_imcolor(), thickness);
    }

    INLINE void rect(float x, float y, float w, float h, c_color clr, float rounding = 0.f, ImDrawCornerFlags flags = 0, float thickness = 1.f)
    {
        draw_list->AddRect(ImVec2(x, y), ImVec2(x + w, y + h), clr.as_imcolor(), rounding, flags, thickness);
    }

    INLINE void filled_rect_gradient(float x, float y, float w, float h, c_color col_upr_left, c_color col_upr_right, c_color col_bot_right, c_color col_bot_left)
    {
        draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x + w, y + h), col_upr_left.as_imcolor(), col_upr_right.as_imcolor(), col_bot_right.as_imcolor(), col_bot_left.as_imcolor());
    }

    INLINE void triangle(float x1, float y1, float x2, float y2, float x3, float y3, c_color clr, float thickness = 1.f)
    {
        draw_list->AddTriangle(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.as_imcolor(), thickness);
    }

    INLINE void triangle_filled(float x1, float y1, float x2, float y2, float x3, float y3, c_color clr)
    {
        draw_list->AddTriangleFilled(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.as_imcolor());
    }

    INLINE void triangle_filled_gradient(float x1, float y1, float x2, float y2, float x3, float y3, c_color clr, c_color clr2, c_color clr3)
    {
        draw_list->AddTriangleFilledMulticolor(ImVec2(x1, y1), ImVec2(x2, y2), ImVec2(x3, y3), clr.as_imcolor(), clr2.as_imcolor(), clr3.as_imcolor());
    }

    INLINE void circle(float x1, float y1, float radius, c_color col, int segments, float thickness = 1.f)
    {
        draw_list->AddCircle(ImVec2(x1, y1), radius, col.as_imcolor(), segments, thickness);
    }

    INLINE void circle_filled(float x1, float y1, float radius, c_color col, int segments)
    {
        draw_list->AddCircleFilled(ImVec2(x1, y1), radius, col.as_imcolor(), segments);
    }

    INLINE ImVec2 get_text_size(c_d3dfont* font, const std::string& text)
    {
        const auto& font_base = font->get();
        const auto& font_size = font->get_size();

        if (text.size() == 0 || text.empty())
            return ImVec2(0.0f, font_size);

        ImVec2 text_size = font_base->CalcTextSizeA(font_size, FLT_MAX, -1.f, text.c_str(), 0, NULL);
        text_size.x = IM_FLOOR(text_size.x + 0.99999f);

        return text_size;
    }

    INLINE void text(float x, float y, c_color color, memory::bits_t flags, c_d3dfont* font, const std::string& text)
    {
        const auto& font_base = font->get();
        const auto& font_size = font->get_size();

        const auto& str = text.c_str();

        draw_list->PushTextureID(font_base->ContainerAtlas->TexID);

        auto coord = ImVec2(x, y);
        auto size = get_text_size(font, text);
        auto scale = (flags.has(FONT_LIGHT_BACK)) ? 0.3f : 1.f;

        auto outline_clr = c_color(0, 0, 0, ((int)color.a() * scale));

        if (!(flags.has(FONT_CENTERED_X)))
            size.x = 0.f;

        if (!(flags.has(FONT_CENTERED_Y)))
            size.y = 0.f;

        ImVec2 pos = ImVec2(coord.x - (size.x * 0.5f), coord.y - (size.y * 0.5f));
        if (flags.has(FONT_DROPSHADOW))
            draw_list->AddText(font_base, font_size, ImVec2(pos.x + 1, pos.y + 1), outline_clr.as_imcolor(), str);

        if (flags.has(FONT_OUTLINE))
        {
            draw_list->AddText(font_base, font_size, ImVec2(pos.x + 1, pos.y - 1), outline_clr.as_imcolor(), str);
            draw_list->AddText(font_base, font_size, ImVec2(pos.x - 1, pos.y + 1), outline_clr.as_imcolor(), str);
            draw_list->AddText(font_base, font_size, ImVec2(pos.x - 1, pos.y - 1), outline_clr.as_imcolor(), str);
            draw_list->AddText(font_base, font_size, ImVec2(pos.x + 1, pos.y + 1), outline_clr.as_imcolor(), str);

            draw_list->AddText(font_base, font_size, ImVec2(pos.x, pos.y + 1), outline_clr.as_imcolor(), str);
            draw_list->AddText(font_base, font_size, ImVec2(pos.x, pos.y - 1), outline_clr.as_imcolor(), str);
            draw_list->AddText(font_base, font_size, ImVec2(pos.x + 1, pos.y), outline_clr.as_imcolor(), str);
            draw_list->AddText(font_base, font_size, ImVec2(pos.x - 1, pos.y), outline_clr.as_imcolor(), str);
        }

        draw_list->AddText(font_base, font_size, pos, color.as_imcolor(), str);

        draw_list->PopTextureID();
    }

    INLINE void blur(float x, float y, float w, float h, c_color clr, float rounding = 0.f)
    {
        imgui_blur::create_blur(draw_list, ImVec2(x, y), ImVec2(x + w, y + h), clr.as_imcolor(), rounding);
    }

    bool world_to_screen(const vec3_t& source, vec2_t& output, bool oob = false);

    bool init();
    void begin();
    void end();
    void list_start();
    void list_end();
    void draw();
    void update_animation_speed();
};

#ifdef _DEBUG
inline auto RENDER = std::make_unique<c_render>();
#else
CREATE_DUMMY_PTR(c_render);
DECLARE_XORED_PTR(c_render, GET_XOR_KEYUI32);

#define RENDER XORED_PTR(c_render)
#endif