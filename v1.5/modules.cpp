#include "globals.hpp"

INLINE HMODULE get_module(const std::string& name, bool wait = false)
{
	HMODULE out{};
	while (true)
	{
		out = WINCALL(GetModuleHandleA)(name.c_str());
		if (out)
			break;

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	return out;
}

void c_hacks::modules_t::init(bool start)
{
	serverbrowser		= get_module(XOR("serverbrowser.dll"), true);
	datacache			= get_module(XOR("datacache.dll"), true);
	gameoverlayrenderer = get_module(XOR("gameoverlayrenderer.dll"));
	materialsystem		= get_module(XOR("materialsystem.dll"));
	shaderapidx9		= get_module(XOR("shaderapidx9.dll"), true);
	client				= get_module(XOR("client.dll"));
	engine				= get_module(XOR("engine.dll"));
	vstdlib				= get_module(XOR("vstdlib.dll"));
	vphysics			= get_module(XOR("vphysics.dll"));
	vgui2				= get_module(XOR("vgui2.dll"));
	server				= get_module(XOR("server.dll"));
	tier0				= get_module(XOR("tier0.dll"));
	localize			= get_module(XOR("localize.dll"));
	filesystem_stdio	= get_module(XOR("filesystem_stdio.dll"));
	vguimatsurface		= get_module(XOR("vguimatsurface.dll"));
	studiorender		= get_module(XOR("studiorender.dll"));
}