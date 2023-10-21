#include "globals.hpp"

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
		msg = tfm::format(CXOR("Exception 0x%x at 0x%x, no module info!"), ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress);
		MessageBoxA(nullptr, msg.c_str(), CXOR("CRASH!"), MB_TOPMOST | MB_ICONERROR | MB_OK);
		exit(ex->ExceptionRecord->ExceptionCode);
	}

	// get nearby bytes and convert them to hex equivalent.
	std::string bytes;
	for (int i = -10; i < 10; i++) {
		const auto addr = ex->ContextRecord->Eip + i;
		if (addr < (uintptr_t)mod)
			continue;
		bytes += tfm::format(CXOR("%02X "), *(uint8_t*)addr);
	}

	char module_name[256]{};
	GetModuleFileNameA(mod, module_name, sizeof(module_name));

	static auto ntdll_dll = ("ntdll.dll");
	static auto tier0_dll = ("tier0.dll");
	static auto kernelbase_dll = ("KERNELBASE.dll");

	if (std::strstr(module_name, ntdll_dll) || std::strstr(module_name, tier0_dll) || std::strstr(module_name, kernelbase_dll))
		return EXCEPTION_CONTINUE_SEARCH;

	msg = tfm::format(CXOR(R"#(Exception 0x%X at 0x%X
--------------
Module: %s

Last bytes: %s
 
eax: %08X
ebx: %08X
ecx: %08X
edx: %08X
-------------

Press CTRL+C and send info to forum.)#"), ex->ExceptionRecord->ExceptionCode, ex->ExceptionRecord->ExceptionAddress,
mod == (HMODULE)cheat_module ? CXOR("AIRFLOW") : module_name, bytes,
ex->ContextRecord->Eax, ex->ContextRecord->Ebx, ex->ContextRecord->Ecx, ex->ContextRecord->Edx);

	// display messagebox.
	MessageBoxA(nullptr, msg.c_str(), CXOR("CRASH!"), MB_TOPMOST | MB_ICONERROR | MB_OK);
	exit(ex->ExceptionRecord->ExceptionCode);

	return EXCEPTION_CONTINUE_EXECUTION;
}
#endif

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
#ifdef _DEBUG
		HACKS->modules.dllmain = module;

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)init_cheat, 0, 0, 0);
#else
#if ALPHA || BETA
		cheat_module = module;

		AddVectoredExceptionHandler(TRUE, exception_handler);
#endif

		std::thread(init_cheat, reserved).detach();
#endif

		return TRUE;
	}

	return false;
}