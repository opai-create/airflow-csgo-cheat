#pragma once
#include "../tools/math.h"
#include "../other/color.h"

#include "app_system.h"

enum font_flags : unsigned int
{
	none,
	italic = 0x001,
	underline = 0x002,
	strikeout = 0x004,
	symbol = 0x008,
	antialias = 0x010,
	gaussianblur = 0x020,
	rotary = 0x040,
	dropshadow = 0x080,
	additive = 0x100,
	outline = 0x200,
	custom = 0x400,
	bitmap = 0x800,
};

class c_surface : public c_app_system
{
public:
	virtual void run_frame() = 0;
	virtual unsigned int get_embedded_panel() = 0;
	virtual void set_embedded_panel(unsigned int pPanel) = 0;
	virtual void push_make_current(unsigned int panel, bool useInsets) = 0;
	virtual void pop_make_current(unsigned int panel) = 0;
	virtual void draw_set_color(int r, int g, int b, int a) = 0;
	virtual void draw_set_color(color col) = 0;
	virtual void draw_filled_rect(int x0, int y0, int x1, int y1) = 0;
	virtual void draw_filled_rect_array(void* pRects, int numRects) = 0;
	virtual void draw_outlined_rect(int x0, int y0, int x1, int y1) = 0;
	virtual void draw_line(int x0, int y0, int x1, int y1) = 0;
	virtual void draw_poly_line(int* px, int* py, int numPoints) = 0;
	virtual void draw_sent_apparent_depth(float f) = 0;
	virtual void draw_clear_apparent_depth(void) = 0;
	virtual void draw_set_text_font(unsigned long font) = 0;
	virtual void draw_set_text_color(int r, int g, int b, int a) = 0;
	virtual void draw_set_text_color(color col) = 0;
	virtual void draw_set_text_pos(int x, int y) = 0;
	virtual void draw_get_text_pos(int& x, int& y) = 0;
	virtual void draw_print_text(const wchar_t* text, int textLen, int drawType = 0) = 0;
	virtual void draw_unicode_char(wchar_t wch, int drawType = 0) = 0;
	virtual void draw_flush_text() = 0;
	virtual void* create_html_window(void* events, unsigned int context) = 0;
	virtual void paint_html_window(void* htmlwin) = 0;
	virtual void delete_html_window(void* htmlwin) = 0;
	virtual int draw_get_texture_id(char const* filename) = 0;
	virtual bool draw_get_texture_file(int id, char* filename, int maxlen) = 0;
	virtual void draw_set_texture_file(int id, const char* filename, int hardwareFilter, bool forceReload) = 0;
	virtual void draw_set_texture_rgba(int id, const unsigned char* rgba, int wide, int tall) = 0;
	virtual void draw_set_texture(int id) = 0;
	virtual void delete_texture_by_id(int id) = 0;
	virtual void draw_set_texture_size(int id, int& wide, int& tall) = 0;
	virtual void draw_textured_rect(int x0, int y0, int x1, int y1) = 0;
	virtual bool is_texture_id_valid(int id) = 0;
	virtual int create_new_texture_id(bool procedural = false) = 0;
	virtual void get_screen_size(int& wide, int& tall) = 0;
	virtual void set_as_top_most(unsigned int panel, bool state) = 0;
	virtual void bring_to_font(unsigned int panel) = 0;
	virtual void set_foreground_window(unsigned int panel) = 0;
	virtual void set_panel_visible(unsigned int panel, bool state) = 0;
	virtual void set_minimized(unsigned int panel, bool state) = 0;
	virtual bool is_minimized(unsigned int panel) = 0;
	virtual void flash_window(unsigned int panel, bool state) = 0;
	virtual void set_title(unsigned int panel, const wchar_t* title) = 0;
	virtual void set_as_tool_bar(unsigned int panel, bool state) = 0;
	virtual void create_popup(unsigned int panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0;
	virtual void swap_buffers(unsigned int panel) = 0;
	virtual void invalidate(unsigned int panel) = 0;
	virtual void set_cursor(unsigned long cursor) = 0;
	virtual bool is_cursor_visible() = 0;
	virtual void apply_changes() = 0;
	virtual bool is_within(int x, int y) = 0;
	virtual bool has_focus() = 0;
	virtual bool supports_feature(int feature) = 0;
	virtual void restrict_panel_to_single(unsigned int panel, bool bForceAllowNonModalSurface = false) = 0;
	virtual void set_modal_panel(unsigned int) = 0;
	virtual unsigned int get_modal_panel() = 0;
	virtual void unlock_cursor() = 0;
	virtual void lock_cursor() = 0;
	virtual void set_translate_extended_keys(bool state) = 0;
	virtual unsigned int get_topmost_popup() = 0;
	virtual void set_top_level_focus(unsigned int panel) = 0;
	virtual unsigned long font_create() = 0;
	virtual bool set_font_glyph_set(unsigned long font, const char* windowsFontName, int tall, int weight, int blur, int scanlines, unsigned int flags, int nRangeMin = 0, int nRangeMax = 0) = 0;
	virtual bool add_custom_font_file(const char* fontFileName) = 0;
	virtual int get_font_tall(unsigned long font) = 0;
	virtual int get_font_ascent(unsigned long font, wchar_t wch) = 0;
	virtual bool is_font_additive(unsigned long font) = 0;
	virtual void get_char_wide(unsigned long font, int ch, int& a, int& b, int& c) = 0;
	virtual int get_character_width(unsigned long font, int ch) = 0;
	virtual void get_text_size(unsigned long font, const wchar_t* text, int& wide, int& tall) = 0;
	virtual unsigned int get_notify_panel() = 0;
	virtual void set_notify_icon(unsigned int context, unsigned long icon, unsigned int panelToReceiveMessages, const char* text) = 0;
	virtual void play_sound(const char* fileName) = 0;
	virtual int get_popup_count() = 0;
	virtual unsigned int get_popup(int index) = 0;
	virtual bool should_paint_child_panel(unsigned int childPanel) = 0;
	virtual bool recreate_context(unsigned int panel) = 0;
	virtual void add_panel(unsigned int panel) = 0;
	virtual void release_panel(unsigned int panel) = 0;
	virtual void move_popup_to_front(unsigned int panel) = 0;
	virtual void move_popup_to_back(unsigned int panel) = 0;
	virtual void solve_traverse(unsigned int panel, bool forceApplySchemeSettings = false) = 0;
	virtual void paint_traverse(unsigned int panel) = 0;
	virtual void enable_mouse_capture(unsigned int panel, bool state) = 0;
	virtual void get_workspace_bounds(int& x, int& y, int& wide, int& tall) = 0;
	virtual void get_abs_window_bounds(int& x, int& y, int& wide, int& tall) = 0;
	virtual void get_proportional_base(int& width, int& height) = 0;
	virtual void calc_mouse_vis() = 0;
	virtual bool net_kbi_input() = 0;
	virtual bool has_cursor_pos_funcs() = 0;
	virtual void surface_get_cursor_pos(int& x, int& y) = 0;
	virtual void surface_set_cursor_pos(int x, int y) = 0;
	virtual void draw_textured_line(const vertex& a, const vertex& b) = 0;
	virtual void draw_outlined_circle(int x, int y, int radius, int segments) = 0;
	virtual void add_text_poly_line(const vertex* p, int n) = 0;
	virtual void add_text_sub_rect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1) = 0;
	virtual void add_text_poly(int n, const vertex* pVertice, bool bClipVertices = true) = 0;
};