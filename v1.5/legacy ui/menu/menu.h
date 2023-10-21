#pragma once
#include "../../globals.hpp"
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/imgui_freetype.h"
#include "../../imgui/imgui_impl_dx9.h"
#include "../../imgui/imgui_impl_win32.h"

#include "ui_structs.h"

class ImDrawList;
class c_game_event;
struct ImVec2;

constexpr int max_tabs = 7;

enum animation_flags_t
{
	skip_enable = (1 << 0),
	skip_disable = (1 << 1),
	lerp_animation = (1 << 2),
};

enum button_flags_t
{
	button_wait = (1 << 0),
	button_danger = (1 << 1),
};

extern const char* skyboxes[];

constexpr auto default_picker = ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview |
ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_DisplayRGB;

constexpr auto no_alpha_picker =
ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_DisplayRGB;

class c_menu
{
private:

	int tab_selector{};

	ImDrawList* draw_list{};
	float tab_alpha{};
	float subtab_alpha{};
	float subtab_alpha2{};
	float bomb_health_text_lerp{};

	ImVec2 window_pos{};

	spectator_animation_t spectator_animaiton[50]{};
	key_binds_t prev_keys[binds_max]{};
	std::map<std::string, menu_key_binds_t> updated_keybinds{};
	std::map< std::string, std::array< tab_animation_t, 6 > > subtab_info{};
	std::array<tab_animation_t, max_tabs> tab_info{};

	LPDIRECT3DTEXTURE9 logo_texture{};
	LPDIRECT3DTEXTURE9 keyboard_texture{};
	LPDIRECT3DTEXTURE9 bomb_texture{};
	LPDIRECT3DTEXTURE9 warning_texture{};
	LPDIRECT3DTEXTURE9 spectator_texture{};

	std::map< std::uint32_t, item_animation_t > item_animations{};
	std::array< LPDIRECT3DTEXTURE9, max_tabs > icon_textures{};
	std::map< std::string, std::string > combo_items{};

	std::vector< std::string > skin_names{};

	static bool vector_getter(void* vec, int idx, const char** out_text)
	{
		auto& vector = *static_cast<std::vector< std::string >*>(vec);
		if (idx < 0 || idx >= static_cast<int>(vector.size()))
		{
			return false;
		}
		*out_text = vector.at(idx).c_str();
		return true;
	};

	static bool items_array_getter(void* data, int idx, const char** out_text)
	{
		const char* const* items = (const char* const*)data;
		if (out_text)
			*out_text = items[idx];
		return true;
	}

	static float calc_max_popup_height(int items_count)
	{
		ImGuiContext& g = *GImGui;
		if (items_count <= 0)
			return FLT_MAX;
		return (g.FontSize + g.Style.ItemSpacing.y) * items_count - g.Style.ItemSpacing.y + (g.Style.WindowPadding.y * 2);
	}

	static void render_arrows_for_vertical_bar(ImDrawList* draw_list, ImVec2 pos, ImVec2 half_sz, float bar_w, float alpha)
	{
		ImU32 alpha8 = IM_F32_TO_INT8_SAT(alpha);
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x + 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Right, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + half_sz.x, pos.y), half_sz, ImGuiDir_Right, IM_COL32(255, 255, 255, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x - 1, pos.y), ImVec2(half_sz.x + 2, half_sz.y + 1), ImGuiDir_Left, IM_COL32(0, 0, 0, alpha8));
		ImGui::RenderArrowPointingAt(draw_list, ImVec2(pos.x + bar_w - half_sz.x, pos.y), half_sz, ImGuiDir_Left, IM_COL32(255, 255, 255, alpha8));
	}

	bool button_wrapper(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0, bool* hovered_ptr = nullptr, int button_flags = 0);
	void button(const char* label, void (*callback)(), int flags = 0);

	bool input_text_wrapper(const char* label, const char* hint, char* buf, int buf_size, const ImVec2& size_arg, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
	bool input_text(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);

	bool checkbox(const char* label, bool* v);
	bool checkbox_columns(const char* label, unsigned int& v, const std::vector< std::string >& conditions);

	bool selectable(const char* label, bool selected, float alpha_pass = 1.f, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
	bool begin_combo(const char* label, const char* preview_value, ImGuiComboFlags flags, int item_cnt = 0);
	bool combo_wrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int popup_max_height_in_items);
	bool combo(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);

	bool selectable2(const char* label, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
	bool selectable_flags(const char* label, unsigned int* flags, unsigned int flags_value);
	void multi_combo(const char* name, unsigned int& var, std::vector< std::string > elements);

	bool slider_scalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, float power = 1.0f);
	bool slider_int(const char* label, int* v, int v_min, int v_max, const char* format = CXOR("%d"));

	bool key_bind(const char* label, key_binds_t& var);

	bool color_picker_wrapper(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);
	bool color_button_wrapper(const char* desc_id, const ImVec4& col, ImGuiColorEditFlags flags = 0);
	bool color_picker(const char* name, c_float_color& v, ImGuiColorEditFlags flags = default_picker);

	bool listbox_selectable(const char* label, bool selected, float alpha_pass, ImGuiSelectableFlags flags, const ImVec2& size_arg, int iter);
	bool list_box_header(const char* label, const ImVec2& size_arg);
	bool list_box_header_start(const char* label, int items_count, int height_in_items);
	void list_box_footer();
	bool list_box_wrapper(const char* label, int* current_item, bool (*items_getter)(void*, int, const char**), void* data, int items_count, int height_in_items, const std::string& compare_text = "");
	bool listbox(const char* label, int* currIndex, std::vector< std::string >& values, int height, const std::string& compare_text = "");

public:
	bool opened{};

	LPDIRECT3DTEXTURE9 avatar{};

	float alpha{};
	menu_bomb_t bomb{};

	spectator_t spectators[50]{};

#if ALPHA
	std::string prefix = XOR("airflow (alpha)");
#elif BETA
	std::string prefix = XOR("airflow (beta)");
#else
	std::string prefix = XOR("airflow");
#endif

	void set_draw_list(ImDrawList* list);
	ImDrawList* get_draw_list();

	void draw_snow();
	void draw_binds();
	void store_bomb();
	void store_spectators();
	void draw_bomb_indicator();
	void draw_spectators();

	void draw_watermark();

	void on_game_events(c_game_event* event);

	void set_window_pos(const ImVec2& pos);
	ImVec2 get_window_pos();

	void init_textures();
	void draw_ui_background();
	void draw_tabs();
	void draw_sub_tabs(int& selector, const std::vector< std::string >& tabs);
	void draw_ui_items();
	void update_alpha();
	float get_alpha();
	void window_begin();
	void window_end();

	void create_animation(float& mod, bool cond, float speed_multiplier = 1.f, unsigned int animation_flags = 0);
	void draw();

	INLINE void reset_game_info()
	{
		for (auto& i : spectator_animaiton)
			i.reset();

		for (auto& i : spectators)
			i.reset();

		bomb_health_text_lerp = 0.f;
		bomb.reset();
	}

	INLINE void reset_init()
	{
		for (auto& i : prev_keys)
			i.reset();

		updated_keybinds.clear();
		subtab_info.clear();
		item_animations.clear();
		combo_items.clear();
		skin_names.clear();

		for (auto& i : tab_info)
			i.reset();

		for (auto& i : icon_textures)
			i = nullptr;

		reset_game_info();
	}
};

inline c_menu g_menu{};