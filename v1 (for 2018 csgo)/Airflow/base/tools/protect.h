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
  constexpr uint32_t xs32_from_seed( uint32_t seed )
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
    constexpr number_obfuscated( uint32_t num, uint32_t key )
    {
      m_key = xs32_from_seed( key );
      m_obfuscated = bits32( num )._xor( m_key ).get( );
    }
    uint32_t get( ) const
    {
      return bits32( this->m_obfuscated )._xor( this->m_key ).get( );
    }
  };

  template < uint32_t num, uint32_t seed >
  uint32_t obfuscate( )
  {
    constexpr auto x = number_obfuscated( num, seed + __TIME__ [ 0 ] - '0' + __TIME__ [ 1 ] - '0' + __TIME__ [ 3 ] - '0' + __TIME__ [ 4 ] - '0' + __TIME__ [ 6 ] - '0' + __TIME__ [ 7 ] - '0' );

    return x.get( );
  }
}

#define xor_int( n ) numbers::obfuscate< n, __COUNTER__ >( )

template < std::size_t strLen >
class c_xor_string
{
protected:
  static constexpr std::uint64_t hash( std::uint64_t x, std::uint64_t sol )
  {
    x ^= 948274649985346773LLU ^ sol;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return x;
  }
  bool crypt_once{ };
  mutable bool decrypted{ };
  mutable char string [ strLen ]{ };
  std::uint64_t xor_hash{ };

public:
  constexpr c_xor_string( const char ( &str ) [ strLen ], std::uint64_t hashingSol, bool cryptOnce ): decrypted( false ), string{ 0 }, xor_hash( hashingSol ), crypt_once( cryptOnce )
  {
    for( std::size_t i = 0; i < strLen; ++i )
      this->string [ i ] = str [ i ] ^ c_xor_string< strLen >::hash( i, this->xor_hash );
  }
  operator std::string( ) const
  {
    if( crypt_once )
    {
      if( !this->decrypted )
      {
        this->decrypted = true;
        for( std::size_t i = 0; i < strLen; ++i )
          this->string [ i ] ^= c_xor_string< strLen >::hash( i, this->xor_hash );
      }
    }
    else
    {
      for( std::size_t i = 0; i < strLen; ++i )
        this->string [ i ] ^= c_xor_string< strLen >::hash( i, this->xor_hash );
    }
    return { this->string, this->string + strLen - 1 };
  }
};

template < std::size_t strLen >
class c_xor_wstring
{
protected:
  static constexpr std::uint64_t hash( std::uint64_t x, std::uint64_t sol )
  {
    x ^= 948274649985346773LLU ^ sol;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    return x;
  }
  bool crypt_once{ };
  mutable bool decrypted{ };
  mutable wchar_t string [ strLen ]{ };
  std::uint64_t xor_hash{ };

public:
  constexpr c_xor_wstring( const wchar_t ( &str ) [ strLen ], std::uint64_t hashingSol, bool cryptOnce ): decrypted( false ), string{ 0 }, xor_hash( hashingSol ), crypt_once( cryptOnce )
  {
    for( std::size_t i = 0; i < strLen; ++i )
      this->string [ i ] = str [ i ] ^ c_xor_wstring< strLen >::hash( i, this->xor_hash );
  }
  operator std::wstring( ) const
  {
    if( crypt_once )
    {
      if( !this->decrypted )
      {
        this->decrypted = true;
        for( std::size_t i = 0; i < strLen; ++i )
          this->string [ i ] ^= c_xor_wstring< strLen >::hash( i, this->xor_hash );
      }
    }
    else
    {
      for( std::size_t i = 0; i < strLen; ++i )
        this->string [ i ] ^= c_xor_wstring< strLen >::hash( i, this->xor_hash );
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
  unsigned long long rt( const _ty* txt )
  {
    auto hash = fnv_basis;

    std::size_t length = 0;
    while( txt [ length ] )
      ++length;

    for( auto i = 0u; i < length; i++ )
    {
      hash ^= txt [ i ];
      hash *= fnv_prime;
    }

    return hash;
  }

  template < typename _ty >
  constexpr unsigned long long ct( const _ty* txt, unsigned long long value = fnv_basis )
  {
    return !*txt ? value : ct( txt + 1, static_cast< unsigned long long >( 1ull * ( value ^ static_cast< unsigned short >( *txt ) ) * fnv_prime ) );
  }
}

#define HASH( s ) CONSTANT( fnv1a::ct( s ) )
#define HASH_RT( s ) fnv1a::rt( s )

namespace character
{
  template < typename Type >
  constexpr bool is_upper( const Type character )
  {
    return ( character >= static_cast< const Type >( 65 ) && character <= static_cast< const Type >( 90 ) );
  }

  template < typename Type >
  constexpr Type to_lower( const Type character )
  {
    if( is_upper( character ) )
    {
      return ( character + static_cast< const Type >( 32 ) );
    }

    return character;
  }

  template < typename Type >
  constexpr bool is_terminator( const Type character )
  {
    return ( character == static_cast< const Type >( 0 ) );
  }

  template < typename Type >
  constexpr bool is_question( const Type character )
  {
    return ( character == static_cast< const Type >( 63 ) );
  }

  template < typename Type >
  constexpr std::size_t get_length( const Type* const data )
  {
    std::size_t length = 0;

    while( true )
    {
      if( is_terminator( data [ length ] ) )
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
  constexpr hash_t hash_compute( hash_t hash_basis, const Type* const data, std::size_t size, bool ignore_case )
  {
    const auto element = static_cast< hash_t >( ignore_case ? character::to_lower( data [ 0 ] ) : data [ 0 ] );
    return ( size == 0 ) ? hash_basis : hash_compute( ( hash_basis * hash_prime ) ^ element, data + 1, size - 1, ignore_case );
  }

  template < typename Type >
  constexpr hash_t fnva1_hash( const Type* const data, std::size_t size, bool ignore_case )
  {
    return hash_compute( hash_basis, data, size, ignore_case );
  }

  constexpr hash_t fnva1_hash( const char* const data, bool ignore_case )
  {
    const auto length = character::get_length( data );
    return fnva1_hash( data, length, ignore_case );
  }

  constexpr hash_t fnva1_hash( const wchar_t* const data, bool ignore_case )
  {
    const auto length = character::get_length( data );
    return fnva1_hash( data, length, ignore_case );
  }

  template < typename Type >
  constexpr hash_t fnva1_hash( const std::basic_string< Type >& data, bool ignore_case )
  {
    return fnva1_hash( data.c_str( ), data.size( ), ignore_case );
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

  extern std::string aa_stand;
  extern std::string aa_move;
  extern std::string aa_air;

  extern std::string aa_pitch_down;
  extern std::string aa_pitch_up;
  extern std::string aa_pitch_minimal;

  extern std::string aa_yaw_back;
  extern std::string aa_yaw_spin;
  extern std::string aa_yaw_crooked;

  extern std::string aa_jitter_center;
  extern std::string aa_jitter_offset;
  extern std::string aa_jitter_random;

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
  extern std::string chams_fakelag;

  extern std::string sound_metallic;
  extern std::string sound_tap;

  extern std::string box_default;
  extern std::string box_thin;

  extern std::string ragdoll_away;
  extern std::string ragdoll_fly;

  extern std::string target_damage;
  extern std::string target_distance;

  extern std::string tracer_beam;
  extern std::string tracer_line;

  extern void init( );
}

#ifndef _DEBUG

enum eauth_t
{
  auth_success = 0,
  auth_fail,
};

class c_condition_callback
{
private:
  std::function< void( ) > first_callback{ };
  std::function< void( ) > second_callback{ };

  uint32_t condition{ };

public:
  c_condition_callback( )
  {
  }

  c_condition_callback( bool cond, const std::function< void( ) >& first, const std::function< void( ) >& second ): first_callback( first ), second_callback( second )
  {
    condition = ( int )cond;
    if( condition )
      first_callback( );
    else
      second_callback( );
  }
};

#endif