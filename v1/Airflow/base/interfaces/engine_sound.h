#pragma once
#include "../tools/math.h"
#include "../tools/memory/memory.h"

#include "../sdk/c_utlvector.h"

struct snd_info_t
{
	int guid{};
	void* file_name_handle{};
	int sound_source{};
	int channel{};

	int speaker_entity{};
	float volume{};
	float last_spatialized_volime{};

	float radius{};
	int pitch{};
	vector3d* origin{};
	vector3d* direction{};

	bool update_positions{};
	bool is_sentence{};
	bool dry_mix{};
	bool speaker{};
	bool from_server{};
};

class c_engine_sound
{
public:
	void get_active_sounds(c_utlvector< snd_info_t >& sound)
	{
		using fn = void(__thiscall*)(void*, c_utlvector< snd_info_t >&);
		g_memory->getvfunc< fn >(this, 19)(this, sound);
	}
};