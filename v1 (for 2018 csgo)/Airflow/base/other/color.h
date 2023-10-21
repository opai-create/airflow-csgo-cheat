#pragma once
#include "../tools/math.h"
#include "../../additionals/imgui/imgui.h"
#include <algorithm>
#include <array>

#undef max
#undef min

class color
{
private:
  union
  {
    uint8_t rgba [ 4 ] = { };
    uint32_t as_u32;
  } m_color;

public:
  __forceinline constexpr color( )
  {
    this->u32( ) = 0xFFFFFFFF;
  }

  __forceinline constexpr color( uint32_t color32 )
  {
    this->u32( ) = color32;
  }

  __forceinline constexpr color( int _r, int _g, int _b )
  {
    this->set( ( uint8_t )_r, ( uint8_t )_g, ( uint8_t )_b, 255 );
  }

  __forceinline constexpr color( int _r, int _g, int _b, int _a )
  {
    this->set( ( uint8_t )_r, ( uint8_t )_g, ( uint8_t )_b, ( uint8_t )_a );
  }

  __forceinline constexpr void set( uint32_t value )
  {
    this->u32( ) = value;
  }

  __forceinline constexpr void set( uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a = 0xFF )
  {
    m_color.rgba [ 0 ] = std::clamp< uint8_t >( _r, 0u, 255u );
    m_color.rgba [ 1 ] = std::clamp< uint8_t >( _g, 0u, 255u );
    m_color.rgba [ 2 ] = std::clamp< uint8_t >( _b, 0u, 255u );
    m_color.rgba [ 3 ] = std::clamp< uint8_t >( _a, 0u, 255u );
  }

  __forceinline constexpr void get( int& _r, int& _g, int& _b, int& _a ) const
  {
    _r = m_color.rgba [ 0 ];
    _g = m_color.rgba [ 1 ];
    _b = m_color.rgba [ 2 ];
    _a = m_color.rgba [ 3 ];
  }

  __forceinline constexpr void get( int& _r, int& _g, int& _b ) const
  {
    _r = m_color.rgba [ 0 ];
    _g = m_color.rgba [ 1 ];
    _b = m_color.rgba [ 2 ];
  }

  __forceinline constexpr uint8_t& r( )
  {
    return m_color.rgba [ 0 ];
  }
  __forceinline constexpr uint8_t& g( )
  {
    return m_color.rgba [ 1 ];
  }
  __forceinline constexpr uint8_t& b( )
  {
    return m_color.rgba [ 2 ];
  }
  __forceinline constexpr uint8_t& a( )
  {
    return m_color.rgba [ 3 ];
  }

  __forceinline constexpr uint8_t r( ) const
  {
    return m_color.rgba [ 0 ];
  }
  __forceinline constexpr uint8_t g( ) const
  {
    return m_color.rgba [ 1 ];
  }
  __forceinline constexpr uint8_t b( ) const
  {
    return m_color.rgba [ 2 ];
  }
  __forceinline constexpr uint8_t a( ) const
  {
    return m_color.rgba [ 3 ];
  }

  __forceinline constexpr uint8_t& operator[]( int index )
  {
    return m_color.rgba [ index ];
  }

  const __forceinline constexpr uint8_t& operator[]( int index ) const
  {
    return m_color.rgba [ index ];
  }

  const __forceinline constexpr bool& operator==( const color& other ) const
  {
    return other.r( ) == this->r( ) && other.g( ) == this->g( ) && other.b( ) == this->b( ) && other.a( ) == this->a( );
  }

  const __forceinline constexpr bool& operator!=( const color& other ) const
  {
    return !( *this == other );
  }

  __forceinline constexpr color& operator=( const color& other )
  {
    this->u32( ) = other.u32( );
    return *this;
  }

  __forceinline ImColor as_imcolor( ) const
  {
    return ImColor( this->r( ), this->g( ), this->b( ), this->a( ) );
  }

  __forceinline constexpr uint32_t& u32( )
  {
    return m_color.as_u32;
  }
  __forceinline constexpr uint32_t u32( ) const
  {
    return m_color.as_u32;
  }

  __forceinline constexpr double hue( ) const
  {
    double r = m_color.rgba [ 0 ] / 255.f;
    double g = m_color.rgba [ 1 ] / 255.f;
    double b = m_color.rgba [ 2 ] / 255.f;

    double mx = std::max< double >( r, std::max< double >( g, b ) );
    double mn = std::min< double >( r, std::min< double >( g, b ) );
    if( mx == mn )
      return 0.f;

    double delta = mx - mn;

    double hue = 0.f;
    if( mx == r )
      hue = ( g - b ) / delta;
    else if( mx == g )
      hue = 2.f + ( b - r ) / delta;
    else
      hue = 4.f + ( r - g ) / delta;

    hue *= 60.f;
    if( hue < 0.f )
      hue += 360.f;

    return hue / 360.f;
  }

  __forceinline constexpr double saturation( ) const
  {
    double r = m_color.rgba [ 0 ] / 255.f;
    double g = m_color.rgba [ 1 ] / 255.f;
    double b = m_color.rgba [ 2 ] / 255.f;

    double mx = std::max< double >( r, std::max< double >( g, b ) );
    double mn = std::min< double >( r, std::min< double >( g, b ) );

    double delta = mx - mn;

    if( mx == 0.f )
      return delta;

    return delta / mx;
  }

  __forceinline constexpr double brightness( ) const
  {
    double r = m_color.rgba [ 0 ] / 255.f;
    double g = m_color.rgba [ 1 ] / 255.f;
    double b = m_color.rgba [ 2 ] / 255.f;

    return std::max< double >( r, std::max< double >( g, b ) );
  }

  static __forceinline constexpr color hsb( float hue, float saturation, float brightness )
  {
    hue = std::clamp< float >( hue, 0.f, 1.f );
    saturation = std::clamp< float >( saturation, 0.f, 1.f );
    brightness = std::clamp< float >( brightness, 0.f, 1.f );

    float h = ( hue == 1.f ) ? 0.f : ( hue * 6.f );
    float f = h - static_cast< int >( h );
    float p = brightness * ( 1.f - saturation );
    float q = brightness * ( 1.f - saturation * f );
    float t = brightness * ( 1.f - ( saturation * ( 1.f - f ) ) );

    if( h < 1.f )
      return color( ( int )( brightness * 255 ), ( int )( t * 255 ), ( int )( p * 255 ) );
    else if( h < 2.f )
      return color( ( int )( q * 255 ), ( int )( brightness * 255 ), ( int )( p * 255 ) );
    else if( h < 3.f )
      return color( ( int )( p * 255 ), ( int )( brightness * 255 ), ( int )( t * 255 ) );
    else if( h < 4 )
      return color( ( int )( p * 255 ), ( int )( q * 255 ), ( int )( brightness * 255 ) );
    else if( h < 5 )
      return color( ( int )( t * 255 ), ( int )( p * 255 ), ( int )( brightness * 255 ) );
    else
      return color( ( int )( brightness * 255 ), ( int )( p * 255 ), ( int )( q * 255 ) );
  }

  __forceinline constexpr color multiply( const color& other, float strength ) const
  {
    if( *this == other )
      return *this;

    return color( ( int )std::lerp( ( float )m_color.rgba [ 0 ], ( float )other.r( ), strength ), ( int )std::lerp( ( float )m_color.rgba [ 1 ], ( float )other.g( ), strength ),
      ( int )std::lerp( ( float )m_color.rgba [ 2 ], ( float )other.b( ), strength ) );
  }

  __forceinline constexpr color new_alpha( int alpha ) const
  {
    return color( m_color.rgba [ 0 ], m_color.rgba [ 1 ], m_color.rgba [ 2 ], std::clamp( alpha, 0, 255 ) );
  }

  __forceinline constexpr color increase( int value, bool consider_alpha = false ) const
  {
    return color( m_color.rgba [ 0 ] + value, m_color.rgba [ 1 ] + value, m_color.rgba [ 2 ] + value, m_color.rgba [ 3 ] + consider_alpha * value );
  }

  __forceinline constexpr color decrease( int value, bool consider_alpha = false ) const
  {
    return increase( -value, consider_alpha );
  }

  __forceinline ImVec4 as_imvec4( )
  {
    return ImVec4( m_color.rgba [ 0 ] / 255.f, m_color.rgba [ 1 ] / 255.f, m_color.rgba [ 2 ] / 255.f, m_color.rgba [ 3 ] / 255.f );
  }
};