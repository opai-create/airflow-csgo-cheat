#include "globals.hpp"
#include "bullet_tracers.hpp"
#include "animations.hpp"
#include "win_http.h"

#include <playsoundapi.h>
#include <fstream>

#pragma comment(lib, "Winmm.lib")

std::unordered_map<std::uint8_t*, std::pair<std::vector<std::uint8_t>, float>> sound_cache{};

INLINE void draw_beam(const vec3_t& start, const vec3_t& end, c_color clr)
{
	const auto name = XOR("sprites/purplelaser1.vmt");

	beam_info_t beam_info;
	beam_info.type = BEAM_NORMAL;
	beam_info.model_name = name.c_str();
	beam_info.model_index = -1;
	beam_info.start = start;
	beam_info.end = end;
	beam_info.life = 3.f;
	beam_info.fade_lenght = .1f;
	beam_info.halo_scale = 0.f;
	beam_info.amplitude = 1.f;
	beam_info.segments = 2;
	beam_info.renderable = true;
	beam_info.brightness = clr.a();
	beam_info.red = clr.r();
	beam_info.green = clr.g();
	beam_info.blue = clr.b();
	beam_info.speed = 1.f;
	beam_info.start_frame = 0;
	beam_info.frame_rate = 0.f;
	beam_info.width = 3;
	beam_info.end_width = 3;
	beam_info.flags = BEAM_ONLY_NO_IS_ONCE | BEAM_FADE_IN | BEAM_NOTILE;

	beam_t* final_beam = HACKS->view_render_beams->create_beam_points(beam_info);

	if (final_beam)
		HACKS->view_render_beams->draw_beam(final_beam);
}

INLINE void read_wav_to_file_memory(std::string fname, BYTE** pb, DWORD* fsize)
{
	std::ifstream f(fname, std::ios::binary);

	f.seekg(0, std::ios::end);
	int lim = f.tellg();
	*fsize = lim;

	*pb = new BYTE[lim];
	f.seekg(0, std::ios::beg);

	f.read((char*)*pb, lim);

	f.close();
};

INLINE void modify_volume_sound(char* bytes, ptrdiff_t file_size, float volume)
{
	int offset = 0;
	for (int i = 0; i < file_size / 2; i++)
	{
		if (bytes[i] == 'd' && bytes[i + 1] == 'a' && bytes[i + 2] == 't' && bytes[i + 3] == 'a')
		{
			offset = i;
			break;
		}
	}

	if (offset == 0)
		return;

	char* data_offset = (bytes + offset);
	DWORD sample_bytes = *(DWORD*)(data_offset + 4);
	DWORD samples = sample_bytes / 2;

	SHORT* sample = (SHORT*)(data_offset + 8);
	for (DWORD i = 0; i < samples; i++)
	{
		SHORT sh_sample = *sample;
		sh_sample = (SHORT)(sh_sample * volume);
		*sample = sh_sample;
		sample++;
		if (((char*)sample) >= (bytes + file_size - 1))
			break;
	}
};

INLINE void play_sound_from_memory(uint8_t* bytes, size_t size, float volume)
{
	if (sound_cache.count(bytes) == 0)
		sound_cache[bytes].first.resize(size);

	auto& current = sound_cache[bytes];
	std::uint8_t* stored_bytes = current.first.data();

	// modify sound only when changed volume
	if (current.second != volume)
	{
		std::memcpy(stored_bytes, bytes, size);
		current.second = volume;
		modify_volume_sound((char*)stored_bytes, size, volume);
	}

	PlaySoundA((char*)stored_bytes, NULL, SND_ASYNC | SND_MEMORY);
}

void c_bullet_tracers::on_player_hurt(c_game_event* event)
{
	if (std::strcmp(event->get_name(), CXOR("player_hurt")) || !HACKS->local)
		return;

	auto attacker = HACKS->engine->get_player_for_user_id(event->get_int(CXOR("attacker")));
	if (HACKS->local->index() != attacker)
		return;

	auto user_id = HACKS->engine->get_player_for_user_id(event->get_int(CXOR("userid")));
	auto player = (c_cs_player*)HACKS->entity_list->get_client_entity(user_id);

	if (!player)
		return;

	if (player->is_teammate())
		return;

	static std::string sound_dir = "";
	float volume = 0.45f * (g_cfg.misc.sound_volume * 0.01f);

	auto new_dir = config::sounds_folder + "\\" + g_cfg.misc.sound_name + CXOR(".wav");

	if (g_cfg.misc.sound != 0)
	{
		auto play_custom_sound = [&]()
		{
			if (g_cfg.misc.sound == 1)
				return false;

			if (!main_utils::file_exist(new_dir.c_str()))
				return false;

			return true;
		};

		if (play_custom_sound())
		{
			sound_dir = new_dir;

			if (sound_dir.size() > 0)
			{
				DWORD file_size;
				BYTE* file_bytes;
				read_wav_to_file_memory(sound_dir, &file_bytes, &file_size);

				play_sound_from_memory((uint8_t*)file_bytes, file_size, volume);
			}
		}
		else
			play_sound_from_memory((uint8_t*)gamesense_sound, sizeof(gamesense_sound), volume);
	}

	if (!g_cfg.misc.hitmarker && !g_cfg.misc.damage)
	{
		if (!hitmarkers.empty())
			hitmarkers.clear();

		return;
	}

	hitmarker_t best_impact{};
	auto origin = player->get_abs_origin();

	auto best_impact_distance = -1.f;
	auto time = HACKS->system_time();

	for (int i = 0; i < impacts.size(); i++)
	{
		auto& iter = impacts[i];

		if (time > iter.impact_time + 3.f)
		{
			impacts.erase(impacts.begin() + i);
			continue;
		}

		float distance = iter.pos.dist_to(origin);
		if (distance < best_impact_distance || best_impact_distance == -1)
		{
			best_impact_distance = distance;
			best_impact = iter;
		}
	}

	if (best_impact_distance == -1)
		return;

	auto& hit = hitmarkers.emplace_back();
	hit.dmg = event->get_int(CXOR("dmg_health"));
	hit.time = time;
	hit.dmg_time = time;
	hit.alpha = 1.f;
	hit.pos = best_impact.pos;
	hit.hp = player->health();
}

void c_bullet_tracers::on_bullet_impact(c_game_event* event)
{
	if (std::strcmp(event->get_name(), CXOR("bullet_impact")) || !HACKS->local)
		return;

	auto user_id = HACKS->engine->get_player_for_user_id(event->get_int(CXOR("userid")));
	auto player = (c_cs_player*)HACKS->entity_list->get_client_entity(user_id);
	if (!player)
		return;

	auto x = event->get_float(CXOR("x"));
	auto y = event->get_float(CXOR("y"));
	auto z = event->get_float(CXOR("z"));

	if (player == HACKS->local)
	{
		if (g_cfg.misc.impacts)
		{
			auto clr = g_cfg.misc.server_clr.base();
			const auto& size = g_cfg.misc.impact_size * 0.1f;
			HACKS->debug_overlay->add_box_overlay({ x, y, z }, { -size, -size, -size },
				{ size, size, size },
				{ 0, 0, 0 },
				clr.r(), clr.g(), clr.b(), clr.a(), 4.f);
		}

		auto& impact = impacts.emplace_back();
		impact.pos = { x, y, z };
		impact.impact_time = HACKS->system_time();
	}

	bool is_local = player == HACKS->local;
	bool is_enemy = !is_local && !player->is_teammate();

	if (g_cfg.misc.tracers & 1 && is_enemy
		|| g_cfg.misc.tracers & 2 && !is_enemy && !is_local
		|| g_cfg.misc.tracers & 4 && is_local)
		bullets[player->index()].emplace_back(bullet_tracer_t{ player->origin() + player->view_offset(), {x, y, z} });
}

void c_bullet_tracers::on_game_events(c_game_event* event)
{
	on_player_hurt(event);
	on_bullet_impact(event);
}

void c_bullet_tracers::render_tracers()
{
	if (!HACKS->local || !HACKS->in_game)
		return;

#ifdef LEGACY
	auto& impact_list = *(c_utl_vector<client_verify_t>*)((std::uintptr_t)HACKS->local + 0xBA84);
#else
	auto& impact_list = *(c_utl_vector<client_verify_t>*)((std::uintptr_t)HACKS->local + 0x11C50);
#endif

	if (g_cfg.misc.impacts)
	{
		auto clr = g_cfg.misc.client_clr.base();

		const auto& size = g_cfg.misc.impact_size * 0.1f;
		for (auto i = impact_list.count(); i > last_impact_size; i--)
		{
			HACKS->debug_overlay->add_box_overlay(impact_list[i - 1].pos, 
				{ -size, -size, -size },
				{ size, size, size },
				{ 0, 0, 0 },
				clr.r(), clr.g(), clr.b(), clr.a(), 4);
		}

		impact_list.remove_all();
	}

	if (impact_list.count() != last_impact_size)
		last_impact_size = impact_list.count();

	if (g_cfg.misc.tracers)
	{
		for (auto i = 0; i < 65; i++)
		{
			auto& a = bullets[i];
			if (a.size() > 0)
			{
				auto player = (c_cs_player*)HACKS->entity_list->get_client_entity(i);
				if (player)
				{
					bool is_local = player == HACKS->local;
					bool is_enemy = !is_local && !player->is_teammate();

					c_float_color trace_clr = c_float_color{ 255, 255, 255 };
					if (is_local)
						trace_clr = g_cfg.misc.trace_clr[2];
					else if (!is_enemy && !is_local)
						trace_clr = g_cfg.misc.trace_clr[1];
					else
						trace_clr = g_cfg.misc.trace_clr[0];

					auto base = trace_clr.base();
					switch (g_cfg.misc.tracer_type)
					{
					case 0:
						draw_beam(a.back().eye_position, a.back().impact_position, base);
						break;
					case 1:
						HACKS->debug_overlay->add_line_overlay(a.back().eye_position, a.back().impact_position, base.r(), base.g(), base.b(), false, 3.f);
						break;
					}
				}

				a.clear();
			}
		}
	}
}

void c_bullet_tracers::render_hitmarkers()
{
	if (!HACKS->local || !HACKS->in_game)
		return;

	if (hitmarkers.empty())
		return;

	auto draw_list = RENDER->get_draw_list();
	RESTORE(draw_list->Flags);

	draw_list->Flags &= ~ImDrawListFlags_AntiAliasedLines;

	float screen_alpha = 0.f;
	for (int i = 0; i < hitmarkers.size(); ++i)
	{
		auto& hit = hitmarkers[i];
		if (hit.time == 0.f || hit.dmg <= 0)
		{
			screen_alpha = 0.f;
			continue;
		}

		float diff = std::clamp(HACKS->system_time() - hit.time, 0.f, 1.f);
		if (diff >= 1.f)
		{
			hit.alpha = std::lerp(hit.alpha, 0.f, RENDER->get_animation_speed() * 1.5f);
			if (hit.alpha <= 0.f)
			{
				screen_alpha = 0.f;

				hitmarkers.erase(hitmarkers.begin() + i);
				continue;
			}
		}

		if (hit.alpha > 0.f)
		{
			screen_alpha = hit.alpha;

			vec2_t position{};
			if (RENDER->world_to_screen(hit.pos, position))
			{
				if (g_cfg.misc.hitmarker & 1)
				{
					auto clr = g_cfg.misc.hitmarker_clr.base();

					RENDER->line(position.x - 2, position.y - 2, position.x - 8, position.y - 8, clr.new_alpha(255.f * hit.alpha), 1.f);
					RENDER->line(position.x + 2, position.y + 2, position.x + 8, position.y + 8, clr.new_alpha(255.f * hit.alpha), 1.f);
					RENDER->line(position.x - 2, position.y + 2, position.x - 8, position.y + 8, clr.new_alpha(255.f * hit.alpha), 1.f);
					RENDER->line(position.x + 2, position.y - 2, position.x + 8, position.y - 8, clr.new_alpha(255.f * hit.alpha), 1.f);
				}

				if (g_cfg.misc.damage)
				{
					auto clr = g_cfg.misc.damage_clr.base();
					RENDER->text(position.x, position.y - 30.f,
						clr.new_alpha(255.f * hit.alpha), 
						FONT_CENTERED_X | FONT_DROPSHADOW | FONT_LIGHT_BACK,
						&RENDER->fonts.dmg, 
						tfm::format(CXOR("%d"), hit.dmg));
				}
			}
		}
	}

	if (screen_alpha && (g_cfg.misc.hitmarker & 2))
	{
		auto clr = g_cfg.misc.hitmarker_clr.base();

		auto position = vec2_t{ RENDER->screen.x / 2.f, RENDER->screen.y / 2.f };

		RENDER->line(position.x - 3, position.y - 3, position.x - 9, position.y - 9, clr.new_alpha(255.f * screen_alpha), 1.f);
		RENDER->line(position.x + 3, position.y + 3, position.x + 9, position.y + 9, clr.new_alpha(255.f * screen_alpha), 1.f);
		RENDER->line(position.x - 3, position.y + 3, position.x - 9, position.y + 9, clr.new_alpha(255.f * screen_alpha), 1.f);
		RENDER->line(position.x + 3, position.y - 3, position.x + 9, position.y - 9, clr.new_alpha(255.f * screen_alpha), 1.f);
	}
}