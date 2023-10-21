#include "globals.hpp"

#define GET_INTERFACE(handle, name, ptr) memory::get_interface(handle, XOR(name).c_str()).cast<ptr*>();

void c_hacks::init_interfaces()
{
	client				= GET_INTERFACE(modules.client, "VClient018", c_client);
	engine				= GET_INTERFACE(modules.engine, "VEngineClient014", c_engine);
	panel				= GET_INTERFACE(modules.vgui2, "VGUI_Panel009", c_panel);
	cvar				= GET_INTERFACE(modules.vstdlib, "VEngineCvar007", c_engine_cvar);
	model_info			= GET_INTERFACE(modules.engine, "VModelInfoClient004", c_model_info);
	prediction			= GET_INTERFACE(modules.client, "VClientPrediction001", c_prediction);
	entity_list			= GET_INTERFACE(modules.client, "VClientEntityList003", c_entity_list);
	render_view			= GET_INTERFACE(modules.engine, "VEngineRenderView014", c_render_view);
	model_render		= GET_INTERFACE(modules.engine, "VEngineModel016", c_model_render);
	debug_overlay		= GET_INTERFACE(modules.engine, "VDebugOverlay004", c_debug_overlay);
	game_movement		= GET_INTERFACE(modules.client, "GameMovement001", c_game_movement);
	material_system		= GET_INTERFACE(modules.materialsystem, "VMaterialSystem080", c_material_system);
	game_event_manager	= GET_INTERFACE(modules.engine, "GAMEEVENTSMANAGER002", c_game_event_manager2);
	phys_surface_props	= GET_INTERFACE(modules.vphysics, "VPhysicsSurfaceProps001", c_phys_surface_props);
	engine_trace		= GET_INTERFACE(modules.engine, "EngineTraceClient004", c_engine_trace);
	localize			= GET_INTERFACE(modules.localize, "Localize_001", c_localize);
	file_system			= GET_INTERFACE(modules.filesystem_stdio, "VFileSystem017", void);
	surface				= GET_INTERFACE(modules.vguimatsurface, "VGUI_Surface031", c_surface);
	static_prop_manager = GET_INTERFACE(modules.engine, "StaticPropMgrClient005", c_static_prop_manager);
	studio_render		= GET_INTERFACE(modules.studiorender, "VStudioRender026", c_studio_render);
	model_cache			= GET_INTERFACE(modules.datacache, "MDLCache004", c_model_cache);
	network_string_table_container = GET_INTERFACE(modules.engine, "VEngineClientStringTable001", c_network_string_table_container);
	engine_sound		= GET_INTERFACE(modules.engine, "IEngineSoundClient003", void);

	client_mode			= **memory::get_virtual(client, XORN(10)).add(XORN(5)).cast<c_client_mode***>();
	input				= *offsets::input.cast<c_input**>();
	client_state		= **offsets::client_state.cast<c_client_state***>();
	move_helper			= **offsets::move_helper.cast<c_move_helper***>();
	global_vars			= **offsets::global_vars.cast<c_global_vars***>();
	view_render			= **offsets::view_render.cast<c_view_render***>();

#ifdef LEGACY
	glow_object_manager = offsets::glow_object_manager.cast<c_glow_object_manager*(__cdecl*)()>()();
#else
	glow_object_manager = *offsets::glow_object_manager.cast<c_glow_object_manager**>();
#endif
	d3d_device			= **offsets::d3d_device.cast<c_d3d_device***>();
	weapon_system		= *offsets::weapon_system.cast<c_weapon_system**>();
	view_render_beams	= *offsets::view_render_beams.cast<c_view_render_beams**>();

	item_schema			= (c_item_schema*)(offsets::item_system.cast<std::uintptr_t(__cdecl*)()>()() + 0x4);
	key_values_system	= memory::address_t{ WINCALL(GetProcAddress)(modules.vstdlib, CXOR("KeyValuesSystem")) }.cast<key_values_system_fn>()();

	update_dynamic_interfaces();
}

void c_hacks::update_dynamic_interfaces()
{
	game_rules = **offsets::game_rules.cast<c_game_rules***>();
}