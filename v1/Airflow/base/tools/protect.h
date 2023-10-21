#pragma once
#include "bit_vector.h"
#include <memory>
#include <string>
#include <functional>

using hash_t = uint32_t;

#ifdef _DEBUG
#define xor_str_s( s ) std::string( s )
#define xor_c_s( s ) ( s )

#define xor_str( s ) std::string( s )
#define xor_c( s ) ( s )

#define xor_int( n ) ( n )

#define xor_wstr( s ) std::wstring( s )
#define xor_wc( s ) ( s )
#else
namespace numbers
{
	constexpr uint32_t xs32_from_seed(uint32_t seed)
	{
		seed ^= seed << 13;
		seed ^= seed >> 17;
		seed ^= seed << 15;
		return seed;
	}

	class number_obfuscated
	{
	private:
		uint32_t m_key = 0;
		uint32_t m_obfuscated = 0;

	public:
		constexpr number_obfuscated(uint32_t num, uint32_t key)
		{
			m_key = xs32_from_seed(key);
			m_obfuscated = bits32(num)._xor(m_key).get();
		}
		uint32_t get() const
		{
			return bits32(this->m_obfuscated)._xor(this->m_key).get();
		}
	};

	template < uint32_t num, uint32_t seed >
	uint32_t obfuscate()
	{
		constexpr auto x = number_obfuscated(num, seed + __TIME__[0] - '0' + __TIME__[1] - '0' + __TIME__[3] - '0' + __TIME__[4] - '0' + __TIME__[6] - '0' + __TIME__[7] - '0');

		return x.get();
	}
}

#define xor_int( n ) numbers::obfuscate< n, __COUNTER__ >( )

template < std::size_t strLen >
class c_xor_string
{
protected:
	static constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol)
	{
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}
	bool crypt_once{};
	mutable bool decrypted{};
	mutable char string[strLen]{};
	std::uint64_t xor_hash{};

public:
	constexpr c_xor_string(const char(&str)[strLen], std::uint64_t hashingSol, bool cryptOnce) : decrypted(false), string{ 0 }, xor_hash(hashingSol), crypt_once(cryptOnce)
	{
		for (std::size_t i = 0; i < strLen; ++i)
			this->string[i] = str[i] ^ c_xor_string< strLen >::hash(i, this->xor_hash);
	}
	operator std::string() const
	{
		if (crypt_once)
		{
			if (!this->decrypted)
			{
				this->decrypted = true;
				for (std::size_t i = 0; i < strLen; ++i)
					this->string[i] ^= c_xor_string< strLen >::hash(i, this->xor_hash);
			}
		}
		else
		{
			for (std::size_t i = 0; i < strLen; ++i)
				this->string[i] ^= c_xor_string< strLen >::hash(i, this->xor_hash);
		}
		return { this->string, this->string + strLen - 1 };
	}
};

template < std::size_t strLen >
class c_xor_wstring
{
protected:
	static constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol)
	{
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}
	bool crypt_once{};
	mutable bool decrypted{};
	mutable wchar_t string[strLen]{};
	std::uint64_t xor_hash{};

public:
	constexpr c_xor_wstring(const wchar_t(&str)[strLen], std::uint64_t hashingSol, bool cryptOnce) : decrypted(false), string{ 0 }, xor_hash(hashingSol), crypt_once(cryptOnce)
	{
		for (std::size_t i = 0; i < strLen; ++i)
			this->string[i] = str[i] ^ c_xor_wstring< strLen >::hash(i, this->xor_hash);
	}
	operator std::wstring() const
	{
		if (crypt_once)
		{
			if (!this->decrypted)
			{
				this->decrypted = true;
				for (std::size_t i = 0; i < strLen; ++i)
					this->string[i] ^= c_xor_wstring< strLen >::hash(i, this->xor_hash);
			}
		}
		else
		{
			for (std::size_t i = 0; i < strLen; ++i)
				this->string[i] ^= c_xor_wstring< strLen >::hash(i, this->xor_hash);
		}
		return { this->string, this->string + strLen - 1 };
	}
};

// decrypt once
#define xor_str_s( s )                                                                                                                                                                                                               \
  (                                                                                                                                                                                                                                  \
    []( ) -> std::string                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                \
      static constexpr c_xor_string str{ s, __COUNTER__, true };                                                                                                                                                                     \
      return str;                                                                                                                                                                                                                    \
    } )( )

#define xor_c_s( s ) ( xor_str_s( s ) ).c_str( )

// decrypt on every call
#define xor_str( s )                                                                                                                                                                                                                 \
  (                                                                                                                                                                                                                                  \
    []( ) -> std::string                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                \
      static constexpr c_xor_string str{ s, __COUNTER__, true };                                                                                                                                                                     \
      return str;                                                                                                                                                                                                                    \
    } )( )

#define xor_c( s ) ( xor_str( s ) ).c_str( )

#define xor_wstr( s )                                                                                                                                                                                                                \
  (                                                                                                                                                                                                                                  \
    []( ) -> std::wstring                                                                                                                                                                                                            \
    {                                                                                                                                                                                                                                \
      static constexpr c_xor_wstring str{ s, __COUNTER__, true };                                                                                                                                                                    \
      return str;                                                                                                                                                                                                                    \
    } )( )

#define xor_wc( s ) ( xor_wstr( s ) ).c_str( )
#endif

template < typename T, T value >
struct constant_holder_t
{
	enum class e_value_holder : T
	{
		m_value = value
	};
};

#define CONSTANT( value ) ( static_cast< decltype( value ) >( constant_holder_t< decltype( value ), value >::e_value_holder::m_value ) )

namespace fnv1a
{
	constexpr auto fnv_basis = 14695981039346656037ull;
	constexpr auto fnv_prime = 1099511628211ull;

	template < typename _ty >
	unsigned long long rt(const _ty* txt)
	{
		auto hash = fnv_basis;

		std::size_t length = 0;
		while (txt[length])
			++length;

		for (auto i = 0u; i < length; i++)
		{
			hash ^= txt[i];
			hash *= fnv_prime;
		}

		return hash;
	}

	template < typename _ty >
	constexpr unsigned long long ct(const _ty* txt, unsigned long long value = fnv_basis)
	{
		return !*txt ? value : ct(txt + 1, static_cast<unsigned long long>(1ull * (value ^ static_cast<unsigned short>(*txt)) * fnv_prime));
	}
}

#define HASH( s ) CONSTANT( fnv1a::ct( s ) )
#define HASH_RT( s ) fnv1a::rt( s )

namespace character
{
	template < typename Type >
	constexpr bool is_upper(const Type character)
	{
		return (character >= static_cast<const Type>(65) && character <= static_cast<const Type>(90));
	}

	template < typename Type >
	constexpr Type to_lower(const Type character)
	{
		if (is_upper(character))
		{
			return (character + static_cast<const Type>(32));
		}

		return character;
	}

	template < typename Type >
	constexpr bool is_terminator(const Type character)
	{
		return (character == static_cast<const Type>(0));
	}

	template < typename Type >
	constexpr bool is_question(const Type character)
	{
		return (character == static_cast<const Type>(63));
	}

	template < typename Type >
	constexpr std::size_t get_length(const Type* const data)
	{
		std::size_t length = 0;

		while (true)
		{
			if (is_terminator(data[length]))
			{
				break;
			}

			length++;
		}

		return length;
	}

}

namespace hash
{
	constexpr std::uint64_t hash_prime = 1099511628211ull;
	constexpr std::uint64_t hash_basis = 14695981039346656037ull;

	template < typename Type >
	constexpr hash_t hash_compute(hash_t hash_basis, const Type* const data, std::size_t size, bool ignore_case)
	{
		const auto element = static_cast<hash_t>(ignore_case ? character::to_lower(data[0]) : data[0]);
		return (size == 0) ? hash_basis : hash_compute((hash_basis * hash_prime) ^ element, data + 1, size - 1, ignore_case);
	}

	template < typename Type >
	constexpr hash_t fnva1_hash(const Type* const data, std::size_t size, bool ignore_case)
	{
		return hash_compute(hash_basis, data, size, ignore_case);
	}

	constexpr hash_t fnva1_hash(const char* const data, bool ignore_case)
	{
		const auto length = character::get_length(data);
		return fnva1_hash(data, length, ignore_case);
	}

	constexpr hash_t fnva1_hash(const wchar_t* const data, bool ignore_case)
	{
		const auto length = character::get_length(data);
		return fnva1_hash(data, length, ignore_case);
	}

	template < typename Type >
	constexpr hash_t fnva1_hash(const std::basic_string< Type >& data, bool ignore_case)
	{
		return fnva1_hash(data.c_str(), data.size(), ignore_case);
	}
}

#define __fnva1( Data )                                                                                                                                                                                                              \
  [ & ]( )                                                                                                                                                                                                                           \
  {                                                                                                                                                                                                                                  \
    constexpr auto hash = hash::fnva1_hash( Data, true );                                                                                                                                                                            \
    return hash;                                                                                                                                                                                                                     \
  }( )

#define _fnva1( Data ) hash::fnva1_hash( Data, true )

// ghetto method of security
// because xor is compiletime
namespace xor_strs
{
	extern std::string hitbox_head;
	extern std::string hitbox_chest;
	extern std::string hitbox_stomach;
	extern std::string hitbox_pelvis;
	extern std::string hitbox_arms;
	extern std::string hitbox_legs;
	extern std::string hitbox_body;
	extern std::string hitbox_limbs;

	extern std::string weapon_default;
	extern std::string weapon_auto;
	extern std::string weapon_heavy_pistols;
	extern std::string weapon_pistols;
	extern std::string weapon_ssg08;
	extern std::string weapon_awp;
	extern std::string weapon_negev;
	extern std::string weapon_m249;
	extern std::string weapon_ak47;
	extern std::string weapon_aug;
	extern std::string weapon_duals;
	extern std::string weapon_p250;
	extern std::string weapon_cz;

	extern std::string aa_disabled;
	extern std::string aa_default;

	extern std::string aa_pitch_down;
	extern std::string aa_pitch_up;

	extern std::string aa_yaw_back;
	extern std::string aa_yaw_spin;

	extern std::string aa_jitter_center;
	extern std::string aa_jitter_offset;
	extern std::string aa_jitter_random;
	extern std::string aa_jitter_3way;

	extern std::string aa_desync_jitter;

	extern std::string aa_fakelag_max;
	extern std::string aa_fakelag_jitter;

	extern std::string vis_chams_textured;
	extern std::string vis_chams_metallic;
	extern std::string vis_chams_flat;
	extern std::string vis_chams_glass;
	extern std::string vis_chams_glow;
	extern std::string vis_chams_bubble;
	extern std::string vis_chams_money;
	extern std::string vis_chams_fadeup;

	extern std::string buybot_none;

	extern std::string cfg_main;
	extern std::string cfg_additional;
	extern std::string cfg_misc;
	extern std::string cfg_custom1;
	extern std::string cfg_custom2;

	extern std::string chams_visible;
	extern std::string chams_xqz;
	extern std::string chams_history;
	extern std::string chams_onshot;
	extern std::string chams_ragdolls;
	extern std::string chams_viewmodel;
	extern std::string chams_wpn;
	extern std::string chams_attachments;
	extern std::string chams_fake;
	extern std::string chams_gloves;

	extern std::string sound_metallic;
	extern std::string sound_tap;

	extern std::string box_default;
	extern std::string box_thin;

	extern std::string ragdoll_away;
	extern std::string ragdoll_fly;

	extern std::string target_damage;
	extern std::string target_distance;

	extern std::string sky_custom;

	extern std::string tracer_beam;
	extern std::string tracer_line;
	extern std::string tracer_glow;

	extern std::string defensive_trigger;
	extern std::string defensive_always;

	extern std::string knife_default;
	extern std::string knife_bayonet;
	extern std::string knife_css;
	extern std::string knife_skeleton;
	extern std::string knife_nomad;
	extern std::string knife_paracord;
	extern std::string knife_survival;
	extern std::string knife_flip;
	extern std::string knife_gut;
	extern std::string knife_karambit;
	extern std::string knife_m9;
	extern std::string knife_huntsman;
	extern std::string knife_falchion;
	extern std::string knife_bowie;
	extern std::string knife_butterfly;
	extern std::string knife_shadow;
	extern std::string knife_ursus;
	extern std::string knife_navaga;
	extern std::string knife_stiletto;
	extern std::string knife_talon;

	extern std::string weapon_cfg_deagle;
	extern std::string weapon_cfg_duals;
	extern std::string weapon_cfg_fiveseven;
	extern std::string weapon_cfg_glock;
	extern std::string weapon_cfg_ak;
	extern std::string weapon_cfg_aug;
	extern std::string weapon_cfg_awp;
	extern std::string weapon_cfg_famas;
	extern std::string weapon_cfg_g3sg1;
	extern std::string weapon_cfg_galil;
	extern std::string weapon_cfg_m249;
	extern std::string weapon_cfg_m4a1;
	extern std::string weapon_cfg_m4a1s;
	extern std::string weapon_cfg_mac10;
	extern std::string weapon_cfg_p90;
	extern std::string weapon_cfg_mp5;
	extern std::string weapon_cfg_ump45;
	extern std::string weapon_cfg_xm1014;
	extern std::string weapon_cfg_bizon;
	extern std::string weapon_cfg_mag7;
	extern std::string weapon_cfg_negev;
	extern std::string weapon_cfg_sawed_off;
	extern std::string weapon_cfg_tec9;
	extern std::string weapon_cfg_p2000;
	extern std::string weapon_cfg_mp7;
	extern std::string weapon_cfg_mp9;
	extern std::string weapon_cfg_nova;
	extern std::string weapon_cfg_p250;
	extern std::string weapon_cfg_scar20;
	extern std::string weapon_cfg_sg553;
	extern std::string weapon_cfg_scout;
	extern std::string weapon_cfg_usps;
	extern std::string weapon_cfg_cz75;
	extern std::string weapon_cfg_revolver;
	extern std::string weapon_cfg_knife;

	extern std::string agent_default;
	extern std::string agent_gign;
	extern std::string agent_gign_a;
	extern std::string agent_gign_b;
	extern std::string agent_gign_c;
	extern std::string agent_gign_d;
	extern std::string agent_pirate;
	extern std::string agent_pirate_a;
	extern std::string agent_pirate_b;
	extern std::string agent_pirate_c;
	extern std::string agent_pirate_d;
	extern std::string agent_danger_a;
	extern std::string agent_danger_b;
	extern std::string agent_danger_c;
	extern std::string agent_cmdr_davida;
	extern std::string agent_cmdr_frank;
	extern std::string agent_cmdr_lieutenant;
	extern std::string agent_cmdr_michael;
	extern std::string agent_cmdr_operator;
	extern std::string agent_cmdr_spec_agent_ava;
	extern std::string agent_cmdr_markus;
	extern std::string agent_cmdr_sous;
	extern std::string agent_cmdr_chem_haz;
	extern std::string agent_cmdr_chef_d;
	extern std::string agent_cmdr_aspirant;
	extern std::string agent_cmdr_officer;
	extern std::string agent_cmdr_d_sq;
	extern std::string agent_cmdr_b_sq;
	extern std::string agent_cmdr_seal_team6;
	extern std::string agent_cmdr_bunkshot;
	extern std::string agent_cmdr_lt_commander;
	extern std::string agent_cmdr_bunkshot2;
	extern std::string agent_cmdr_3rd_commando;
	extern std::string agent_cmdr_two_times_;
	extern std::string agent_cmdr_two_times_2;
	extern std::string agent_cmdr_premeiro;
	extern std::string agent_cmdr_cmdr;
	extern std::string agent_cmdr_1st_le;
	extern std::string agent_cmdr_john_van;
	extern std::string agent_cmdr_bio_haz;
	extern std::string agent_cmdr_sergeant;
	extern std::string agent_cmdr_chem_haz__;
	extern std::string agent_cmdr_farwlo;
	extern std::string agent_cmdr_getaway_sally;
	extern std::string agent_cmdr_getaway_number_k;
	extern std::string agent_cmdr_getaway_little_kev;
	extern std::string agent_cmdr_safecracker;
	extern std::string agent_cmdr_bloody_darryl;
	extern std::string agent_cmdr_bloody_loud;
	extern std::string agent_cmdr_bloody_royale;
	extern std::string agent_cmdr_bloody_skullhead;
	extern std::string agent_cmdr_bloody_silent;
	extern std::string agent_cmdr_bloody_miami;
	extern std::string agent_street_solider;
	extern std::string agent_solider;
	extern std::string agent_slingshot;
	extern std::string agent_enforcer;
	extern std::string agent_mr_muhlik;
	extern std::string agent_prof_shahmat;
	extern std::string agent_prof_osiris;
	extern std::string agent_prof_ground_rebek;
	extern std::string agent_prof_elite_muhlik;
	extern std::string agent_prof_trapper;
	extern std::string agent_prof_trapper_aggressor;
	extern std::string agent_prof_vypa_sista;
	extern std::string agent_prof_col_magnos;
	extern std::string agent_prof_crasswater;
	extern std::string agent_prof_crasswater_forgotten;
	extern std::string agent_prof_solman;
	extern std::string agent_prof_romanov;
	extern std::string agent_prof_blackwolf;
	extern std::string agent_prof_maximus;
	extern std::string agent_prof_dragomir;
	extern std::string agent_prof_dragomir2;
	extern std::string agent_prof_rezan;
	extern std::string agent_prof_rezan_red;

	extern std::string mask_none;
	extern std::string mask_battle;
	extern std::string mask_hoxton;
	extern std::string mask_doll;
	extern std::string mask_skull;
	extern std::string mask_samurai;
	extern std::string mask_evil_clown;
	extern std::string mask_wolf;
	extern std::string mask_sheep;
	extern std::string mask_bunny_gold;
	extern std::string mask_anaglyph;
	extern std::string mask_kabuki_doll;
	extern std::string mask_dallas;
	extern std::string mask_pumpkin;
	extern std::string mask_sheep_bloody;
	extern std::string mask_devil_plastic;
	extern std::string mask_boar;
	extern std::string mask_chains;
	extern std::string mask_tiki;
	extern std::string mask_bunny;
	extern std::string mask_sheep_gold;
	extern std::string mask_zombie_plastic;
	extern std::string mask_chicken;
	extern std::string mask_skull_gold;
	extern std::string mask_demon_man;
	extern std::string mask_engineer;
	extern std::string mask_heavy;
	extern std::string mask_medic;
	extern std::string mask_pyro;
	extern std::string mask_scout;
	extern std::string mask_sniper;
	extern std::string mask_solider;
	extern std::string mask_spy;
	extern std::string mask_holiday_light;

	extern std::string sky_tibet;
	extern std::string sky_bagage;
	extern std::string sky_italy;
	extern std::string sky_jungle;
	extern std::string sky_office;
	extern std::string sky_daylight;
	extern std::string sky_daylight2;
	extern std::string sky_vertigo_blue;
	extern std::string sky_vertigo;
	extern std::string sky_day;
	extern std::string sky_nuke_bank;
	extern std::string sky_venice;
	extern std::string sky_daylight3;
	extern std::string sky_daylight4;
	extern std::string sky_cloudy;
	extern std::string sky_night;
	extern std::string sky_nightb;
	extern std::string sky_night_flat;
	extern std::string sky_dust;
	extern std::string sky_vietnam;
	extern std::string sky_lunacy;
	extern std::string sky_embassy;

	extern std::string glove_default;
	extern std::string glove_bloodhound;
	extern std::string glove_sporty;
	extern std::string glove_slick;
	extern std::string glove_leather_wrap;
	extern std::string glove_motorcycle;
	extern std::string glove_specialist;
	extern std::string glove_hydra;

	extern std::string glove_skin_charred;
	extern std::string glove_skin_snakebite;
	extern std::string glove_skin_bronzed;
	extern std::string glove_skin_leather;
	extern std::string glove_skin_spruce;
	extern std::string glove_skin_lunar;
	extern std::string glove_skin_convoy;
	extern std::string glove_skin_crimson;
	extern std::string glove_skin_superconductor;
	extern std::string glove_skin_arid;
	extern std::string glove_skin_slaugher;
	extern std::string glove_skin_eclipse;
	extern std::string glove_skin_spearmint;
	extern std::string glove_skin_boom;
	extern std::string glove_skin_coolmint;
	extern std::string glove_skin_forest;
	extern std::string glove_skin_crimson_kimono;
	extern std::string glove_skin_emerald_web;
	extern std::string glove_skin_foundation;
	extern std::string glove_skin_badlands;
	extern std::string glove_skin_pandora;
	extern std::string glove_skin_hedge;
	extern std::string glove_skin_guerilla;
	extern std::string glove_skin_diamondback;
	extern std::string glove_skin_king;
	extern std::string glove_skin_imperial;
	extern std::string glove_skin_overtake;
	extern std::string glove_skin_racing;
	extern std::string glove_skin_amphibious;
	extern std::string glove_skin_bronze;
	extern std::string glove_skin_omega;
	extern std::string glove_skin_vice;
	extern std::string glove_skin_pow;
	extern std::string glove_skin_turtle;
	extern std::string glove_skin_transport;
	extern std::string glove_skin_polygon;
	extern std::string glove_skin_cobalt;
	extern std::string glove_skin_overprint;
	extern std::string glove_skin_duct;
	extern std::string glove_skin_arboreal;
	extern std::string glove_skin_emerald;
	extern std::string glove_skin_mangrove;
	extern std::string glove_skin_rattler;
	extern std::string glove_skin_case;
	extern std::string glove_skin_crimson_web;
	extern std::string glove_skin_buñkshot;
	extern std::string glove_skin_fade;
	extern std::string glove_skin_mogul;

	extern void init();
}