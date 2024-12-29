#include "includes.h"

#include "additionals/tinyformat.h"

#include "base/global_context.h"
#include "base/tools/memory/displacement.h"
#include "base/tools/memory/memory.h"
#include "base/tools/netvar_parser.h"
#include "base/tools/key_states.h"
#include "base/other/game_functions.h"
#include "base/hooks/hooks.h"
#include "base/tools/render.h"
#include "base/tools/threads.h"
#include "base/tools/cheat_info.h"

//#ifndef _DEBUG
//#include "base/tools/connection/app_hack.h"
//#endif

//note from @cacamelio : ez fix !1!!1!!!1!

#include "functions/features.h"
#include "functions/config_system.h"

#include <ShlObj.h>
#include <fstream>
#include <format>
#include <Windows.h>

#define debug_log_create 0

#ifdef _DEBUG
#if debug_log_create
#define debug_log(text) file_stream << xor_c(text) << std::endl;
#else
#define debug_log(text) std::cout << xor_c(text) << std::endl;
#endif
#else
#if debug_log_create
#define debug_log(text) file_stream << xor_c(text) << std::endl;
#else
#define debug_log 
#endif
#endif

#ifdef _DEBUG
FILE* stream{};
std::ofstream file_stream{};
HMODULE base_module{};

void create_console()
{
	AllocConsole();
	freopen_s(&stream, ("CONIN$"), ("r"), stdin);
	freopen_s(&stream, ("CONOUT$"), ("w"), stdout);
	freopen_s(&stream, ("CONOUT$"), ("w"), stderr);
}

void free_console()
{
	HWND console = GetConsoleWindow();
	FreeConsole();
	PostMessage(console, WM_CLOSE, 0, 0);
	fclose(stream);
}

#define force_create_console create_console();
#define force_delete_console free_console();

#else
#define force_create_console 
#define force_delete_console 
#endif

#if ALPHA || BETA
HMODULE cheat_module{};

// credits to @panzerfaust
LONG __stdcall exception_handler(EXCEPTION_POINTERS* ex) {
	// continue execution on useless exceptions.
	if (ex->ExceptionRecord->ExceptionCode <= 0x80000000)
		return EXCEPTION_CONTINUE_SEARCH;

	// try to get module name by crash address.
	HMODULE mod{};
	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (const char*)ex->ContextRecord->Eip, &mod)) {
		// get current mapped memory information.
		MEMORY_BASIC_INFORMATION mem{}, cheat{};
		if (VirtualQuery((void*)ex->ContextRecord->Eip, &mem, sizeof(mem))
			&& VirtualQuery((void*)exception_handler, &cheat, sizeof(cheat))) {
			// check if eip is located somewhere within cheat's .text section.
			if (mem.AllocationBase == cheat.AllocationBase)
				mod = (HMODULE)cheat_module;
		}
	}

	std::string msg{};
	if (!mod) {
		msg = tfm::format(xor_c("Exception 0x%x at 0x%x, no module info!"), ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress);
		MessageBoxA(nullptr, msg.c_str(), xor_c("CRASH!"), MB_TOPMOST | MB_ICONERROR | MB_OK);
		exit(ex->ExceptionRecord->ExceptionCode);
	}

	// get nearby bytes and convert them to hex equivalent.
	std::string bytes;
	for (int i = -10; i < 10; i++) {
		const auto addr = ex->ContextRecord->Eip + i;
		if (addr < (uintptr_t)mod)
			continue;
		bytes += tfm::format(xor_c("%02X "), *(uint8_t*)addr);
	}

	char module_name[256]{};
	GetModuleFileNameA(mod, module_name, sizeof(module_name));

	static auto ntdll_dll = xor_str("ntdll.dll");
	static auto tier0_dll = xor_str("tier0.dll");
	static auto kernelbase_dll = xor_str("KERNELBASE.dll");

	if (std::strstr(module_name, ntdll_dll.c_str()) || std::strstr(module_name, tier0_dll.c_str()) || std::strstr(module_name, kernelbase_dll.c_str()))
		return EXCEPTION_CONTINUE_SEARCH;

	msg = tfm::format(xor_c(R"#(Exception 0x%X at 0x%X
--------------
Module: %s

Last bytes: %s
 
eax: %08X
ebx: %08X
ecx: %08X
edx: %08X
-------------

Press CTRL+C and send info to forum.)#"), ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress,
mod == (HMODULE)cheat_module ? xor_c("AIRFLOW") : module_name, bytes,
ex->ContextRecord->Eax, ex->ContextRecord->Ebx, ex->ContextRecord->Ecx, ex->ContextRecord->Edx);

	// display messagebox.
	MessageBoxA(nullptr, msg.c_str(), xor_c("CRASH!"), MB_TOPMOST | MB_ICONERROR | MB_OK);
	exit(ex->ExceptionRecord->ExceptionCode);

	return EXCEPTION_CONTINUE_EXECUTION;
}
#endif

namespace cheat
{
	void allocate_cheat_pointers()
	{
		MUTATION_START

		update_feature_ptr(world_modulation);
		update_feature_ptr(utils);
		update_feature_ptr(movement);

		update_feature_ptr(event_listener);
		update_feature_ptr(listener_entity);

		update_feature_ptr(fake_lag);
		update_feature_ptr(tickbase);
		update_feature_ptr(exploits);
		update_feature_ptr(anti_aim);
		update_feature_ptr(ping_spike);

		update_feature_ptr(visuals_wrapper);
		update_feature_ptr(local_visuals);
		update_feature_ptr(grenade_warning);
		update_feature_ptr(glow_esp);
		update_feature_ptr(esp_store);
		update_feature_ptr(player_esp);
		update_feature_ptr(weapon_esp);
		update_feature_ptr(chams);
		update_feature_ptr(event_visuals);
		update_feature_ptr(event_logger);

		update_feature_ptr(animation_fix);
		update_feature_ptr(local_animation_fix);
		update_feature_ptr(engine_prediction);
		update_feature_ptr(auto_wall);
		update_feature_ptr(rage_bot);

		update_feature_ptr(menu);

		update_feature_ptr(legit_bot);

		update_feature_ptr(netvar_manager);
		update_feature_ptr(key_states);
		update_feature_ptr(memory);
		update_feature_ptr(render);
		update_feature_ptr(thread_pool);

		MUTATION_END
	}

	void wait_for_modules()
	{
		MUTATION_START

		while (!(g_ctx.window = FindWindowA(xor_c("Valve001"), NULL)))
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

		while (!GetModuleHandleA(xor_c("serverbrowser.dll")))
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

		MUTATION_END
	}

#ifdef _DEBUG
	void destroy()
	{
		force_delete_console;

		g_listener_entity->remove_entities();
		g_event_listener->remove_events();

		g_thread_pool->remove();

		hooks::unhook();
	}
#endif

	void initialize()
	{
		MUTATION_START

		auto current_process = GetCurrentProcess();
		auto priority_class = GetPriorityClass(current_process);

		if (priority_class != HIGH_PRIORITY_CLASS && priority_class != REALTIME_PRIORITY_CLASS)
			SetPriorityClass(current_process, HIGH_PRIORITY_CLASS);

		wait_for_modules();

		RegisterHotKey(g_ctx.window, 100, 0, VK_MENU);
		RegisterHotKey(g_ctx.window, 100, 0, VK_LMENU);
		RegisterHotKey(g_ctx.window, 100, 0, VK_RMENU);

/*
#ifndef _DEBUG
		g_cheat_info->user_token = (const char*)g_cheat_info->reserved;

		network::app = std::make_shared< network::app_hack >();
		NETWORK()->start();

		while (!NETWORK()->is_verified)
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

		g_cheat_info->user_name = NETWORK()->username;

		if (NETWORK()->avatar_size > 0)
		{
			auto decoded = network::get_decoded_avatar();
			if (decoded.size() > 0)
				g_cheat_info->user_avatar = decoded;
		}
#endif
		*/                                   //note from @cacamelio : ez fix again !!!!

		force_create_console;
		debug_text("WELCOME TO DEBUG MODE. TIMESTAMP: %s:%s\n", __DATE__, __TIME__);

#if debug_log_create
		file_stream.open(xor_c("airflow_inject.txt"), std::ios::binary);

		char cur_time[128]{};

		time_t t;
		struct tm* ptm;

		t = time(NULL);
		ptm = localtime(&t);

		strftime(cur_time, 128, xor_c("%c"), ptm);

		file_stream << cur_time << std::endl;

		file_stream << xor_c("START") << std::endl;
#endif

		debug_log(("INIT STARTED"));

		debug_log(("UPDATE PTR"));
		allocate_cheat_pointers();

		debug_log(("UPDATE CFG"));
		config::create_config_folder();

		debug_log(("UPDATE MODULES"));
		modules::init();

		debug_log(("UPDATE PATTERNS"));
		patterns::init();

		debug_log(("UPDATE INTERFACES"));
		interfaces::init();

		debug_log(("UPDATE NETVARS"));
		netvars::init();

		debug_log(("UPDATE FUNC POINTERS"));
		func_ptrs::init();

		debug_log(("UPDATE X"));
		xor_strs::init();

		debug_log(("UPDATE CVARS"));
		cvars::init();

		{
			debug_log(("UPDATE ENGINE PRED"));
			g_engine_prediction->init();

			debug_log(("UPDATE CHAMS"));
			g_chams->init_materials();

			debug_log(("UPDATE SKY"));
			if (cvars::sv_skyname)
				g_ctx.sky_name = cvars::sv_skyname->string;

			debug_log(("UPDATE RENDER"));
			g_render->update_screen_size();

			debug_log(("UPDATE ENT LISTENER"));
			g_listener_entity->init_entities();

			debug_log(("UPDATE EVENT LISTENER"));
			g_event_listener->init_events();

			debug_log(("UPDATE PARSER"));
			skin_changer::init_parser();

			debug_log(("ALLOC THREADS"));
			g_thread_pool->init();

			debug_log(("UPDATE GAMEPATH"));
			g_ctx.exe_path = main_utils::get_executable_file_path();
			g_ctx.exe_path.erase(g_ctx.exe_path.end() - 8, g_ctx.exe_path.end());
			g_ctx.exe_path.append(xor_c("\\csgo\\"));
		}

		debug_log(("INIT HOOKS"));
		hooks::init();

		debug_log(("DONE"));
		g_ctx.cheat_init = true;

#ifdef _DEBUG
		interfaces::engine->execute_cmd_unrestricted("cl_fullupdate");

		while (!g_ctx.uninject)
			std::this_thread::sleep_for(std::chrono::milliseconds(200));

		destroy();

		FreeLibraryAndExitThread(base_module, 0);
#endif

#ifdef DEBUG_LOG
		file_stream.close();
#endif

		MUTATION_END
	}
}

void do_something_special(LPVOID reserved)
{
	g_cheat_info->reserved = reserved;

	g_cheat_info->init();
}

BOOL APIENTRY DllMain(HMODULE module, uintptr_t reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
#if ALPHA || BETA
		cheat_module = module;

		AddVectoredExceptionHandler(TRUE, exception_handler);
#endif

#ifdef _DEBUG
		base_module = module;
#endif

		MUTATION_START

		g_cheat_info->cheat_module = module;

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)do_something_special, reserved, 0, 0);

		MUTATION_END

		//
		return TRUE;
	}

	return FALSE;
}