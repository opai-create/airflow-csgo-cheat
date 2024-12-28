#include "globals.hpp"
#include "legacy ui/legacy_str.h"
#include "features.hpp"

/*
#ifndef _DEBUG
#include <VirtualizerSDK.h>
#endif
*/

#ifdef _DEBUG
FILE* stream = NULL;

void create_console()
{
	AllocConsole();
	freopen_s(&stream, ("CONIN$"), ("r"), stdin);
	freopen_s(&stream, ("CONOUT$"), ("w"), stdout);
	freopen_s(&stream, ("CONOUT$"), ("w"), stderr);
}

void destroy_console()
{
	HWND console = GetConsoleWindow();
	FreeConsole();
	PostMessage(console, WM_CLOSE, 0, 0);
	fclose(stream);
}
#endif

INLINE void init_listeners()
{
	LISTENER_ENTITY->init();
	LISTENER_EVENT->init();
}

INLINE void remove_listeners()
{
	LISTENER_ENTITY->remove();
	LISTENER_EVENT->remove();
}

void c_hacks::init_local_player()
{
	local = **offsets::local.cast<c_cs_player***>();

	if (!local || !local->is_alive())
	{
		weapon = nullptr;
		weapon_info = nullptr;
		return;
	}

	weapon = local->get_handle_entity<c_base_combat_weapon>(local->active_weapon());
	if (weapon)
		weapon_info = weapon_system->get_weapon_data(weapon->item_definition_index());
}

void c_hacks::init_main(LPVOID reserved)
{
	init(reserved);
}

void c_hacks::init(LPVOID reserved)
{
#ifdef _DEBUG
	create_console();

	DEBUG_LOG("Welcome to debug mode \n");
	DEBUG_LOG("Build date: %s \n\n", __DATE__);
#endif

	while (!(window = WINCALL(FindWindowA)(CXOR("Valve001"), NULL)))
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	while (!WINCALL(GetModuleHandleA)(CXOR("serverbrowser.dll")))
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	DEBUG_LOG(" [+] Modules \n");
	modules.init(false);
	
	DEBUG_LOG(" [+] Offsets \n");
	offsets::init();

	DEBUG_LOG(" [+] Xored strings \n");
	xor_strs::init();

	DEBUG_LOG(" [+] Configs \n");
	config::create_config_folder();

	DEBUG_LOG(" [+] Interfaces \n");
	init_interfaces(); 

	DEBUG_LOG(" [+] Listeners \n");
	init_listeners();
	 
	DEBUG_LOG(" [+] Convars \n");
	convars.init();

	DEBUG_LOG(" [+] Netvars \n");
	netvars::init();

	DEBUG_LOG(" [+] Threads \n");
	THREAD_POOL->init();

	DEBUG_LOG(" [+] Skin parser \n");
	skin_changer::init_parser();

	DEBUG_LOG(" [+] Update screen size \n");
	RENDER->update_screen();

	DEBUG_LOG(" [+] Reset GUI & Binds \n");
	g_menu.reset_init();
	g_cfg.reset_init();

	DEBUG_LOG(" [+] Chams materials \n");
	CHAMS->init_materials();

	DEBUG_LOG(" [+] Rage seeds \n");
	RAGEBOT->build_seeds();

	DEBUG_LOG(" [+] Post processing offsets \n");
	POST_PROCESSING->init();

	DEBUG_LOG(" [+] Sky name \n");
	WORLD_MODULATION->update_real_sky_name();

	DEBUG_LOG(" [+] EXE Path \n");
	exe_path = main_utils::get_executable_file_path();
	exe_path.erase(exe_path.end() - 8, exe_path.end());
	exe_path.append(CXOR("\\csgo\\"));

	DEBUG_LOG(" [+] Hooks \n");
	hooks::init();

	cheat_init = true;

#ifdef _DEBUG
	DEBUG_LOG(" [+] Updating entity listener \n\n");

	if (engine->is_connected() && engine->is_in_game())
		engine->execute_client_cmd("cl_fullupdate");
#endif

	DEBUG_LOG("Hack was injected successfuly! \n");

#ifdef _DEBUG
	while (!unload)
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

	cheat_init = false;

	THREAD_POOL->remove();
	remove_listeners();
	hooks::remove();
	destroy_console();
	FreeLibraryAndExitThread(modules.dllmain, 0);
#endif
}

c_hacks::c_hacks()
{

}