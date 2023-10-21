#pragma once

enum panel_types_t : int
{
	FOCUS_OVERLAY_PANEL = 0,
	MAT_SYS_TOP_PANEL,
	HUD_ZOOM,
};

class c_vgui_panel
{
private:
	struct panel_t
	{
		hash32_t name;
		unsigned int panel_id;
	};

	std::vector<panel_t> panels
	{
	  { HASH("FocusOverlayPanel") },
	  { HASH("MatSystemTopPanel") },
	  { HASH("HudZoom") },
	};

	INLINE unsigned int get_new_panel(unsigned int id, const hash32_t& name)
	{
		auto panel_name = CONST_HASH(HACKS->panel->get_name(id));
		if (panel_name == name)
			return id;

		return 0;
	}

public:
	INLINE bool is_valid_panel(int idx, unsigned int id)
	{
		return panels[idx].panel_id == id;
	}

	void update_panels(unsigned int id);
};

#ifdef _DEBUG
inline auto VGUI_PANEL = std::make_unique<c_vgui_panel>();
#else
CREATE_DUMMY_PTR(c_vgui_panel);
DECLARE_XORED_PTR(c_vgui_panel, GET_XOR_KEYUI32);

#define VGUI_PANEL XORED_PTR(c_vgui_panel)
#endif