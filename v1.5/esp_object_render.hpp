#pragma once

const auto WHITE_COLOR = c_color{ 255, 255, 255, 255 };
const auto BLACK_COLOR = c_color{ 0, 0, 0, 255 };
constexpr auto MAX_ESP_OBJECTS = 20;

enum esp_object_type_t
{
    ESP_OBJECT_BAR,
    ESP_OBJECT_STRING,
};

enum esp_object_pos_t
{
    ESP_POS_LEFT,
    ESP_POS_UP,
    ESP_POS_RIGHT,
    ESP_POS_DOWN,
};

enum render_fonts_t
{
    FONT_DEFAULT = 0,
    FONT_PIXEL,
    FONT_ICON,
};

INLINE c_d3dfont* get_font_by_index(int idx)
{
    switch (idx)
    {
    case FONT_DEFAULT:
        return &RENDER->fonts.esp;
    case FONT_PIXEL:
        return &RENDER->fonts.pixel;
    case FONT_ICON:
        return &RENDER->fonts.weapon_icons;
    }
}

struct esp_object_t
{
    bool valid = false;

    unsigned int font_flags = 0;
    int string_font = FONT_DEFAULT;
    int position = -1;

    float bar = 0.f;
    float bar_max = 0.f;
    float alpha = 1.f;

    c_color clr{};
    c_color outline_clr{};
    std::string string{};

    INLINE void reset()
    {
        valid = false;

        font_flags = 0;
        string_font = FONT_DEFAULT;
        position = -1;

        bar = 0.f;
        bar_max = 0.f;
        alpha = 1.f;

        clr = WHITE_COLOR;
        outline_clr = BLACK_COLOR;
        string.clear();
    }
};

struct box_t
{
    bool offscreen = false;

    float x{}, y{}, w{}, h{};

    INLINE void reset()
    {
        offscreen = false;

        x = 0.f;
        y = 0.f;
        w = 0.f;
        h = 0.f;
    }
};

namespace esp_objects
{
    extern void render_strings(box_t& box, esp_object_t& object, int max_bar_width, float& offset);
    extern void render_bars(box_t& box, esp_object_t& object, int& offset);
}