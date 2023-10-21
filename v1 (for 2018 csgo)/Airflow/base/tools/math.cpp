#include "math.h"

#include "../sdk.h"
#include "../global_context.h"

#include "../other/game_functions.h"
#include "../tools/memory/displacement.h"

namespace math
{
  void sin_cos( float radians, PFLOAT sine, PFLOAT cosine )
  {
    __asm
    {
			fld dword ptr[radians]
			fsincos
			mov edx, dword ptr[cosine]
			mov eax, dword ptr[sine]
			fstp dword ptr[edx]
			fstp dword ptr[eax]
    }
  }

  __forceinline float rad_to_deg( float radian )
  {
    return ( float )( radian ) * ( float )( 180.0f / ( float )( PI ) );
  }

  __forceinline float deg_to_rad( float degree )
  {
    return ( float )( degree ) * ( float )( ( float )( PI ) / 180.0f );
  }

  float get_fov( const vector3d& view_angle, const vector3d& aim_angle )
  {
    vector3d delta = aim_angle - view_angle;
    delta = delta.normalized_angle( );
    return std::min( std::sqrtf( std::powf( delta.x, 2.0f ) + std::powf( delta.y, 2.0f ) ), 180.0f );
  }

  void vector_transform( vector3d in1, const matrix3x4_t& in2, vector3d& out )
  {
    out = { in1.dot( vector3d( in2 [ 0 ][ 0 ], in2 [ 0 ][ 1 ], in2 [ 0 ][ 2 ] ) ) + in2 [ 0 ][ 3 ], in1.dot( vector3d( in2 [ 1 ][ 0 ], in2 [ 1 ][ 1 ], in2 [ 1 ][ 2 ] ) ) + in2 [ 1 ][ 3 ],
      in1.dot( vector3d( in2 [ 2 ][ 0 ], in2 [ 2 ][ 1 ], in2 [ 2 ][ 2 ] ) ) + in2 [ 2 ][ 3 ] };
  }

  vector3d get_vector_transform( vector3d& in1, const matrix3x4_t& in2 )
  {
    return { in1.dot( vector3d( in2 [ 0 ][ 0 ], in2 [ 0 ][ 1 ], in2 [ 0 ][ 2 ] ) ) + in2 [ 0 ][ 3 ], in1.dot( vector3d( in2 [ 1 ][ 0 ], in2 [ 1 ][ 1 ], in2 [ 1 ][ 2 ] ) ) + in2 [ 1 ][ 3 ],
      in1.dot( vector3d( in2 [ 2 ][ 0 ], in2 [ 2 ][ 1 ], in2 [ 2 ][ 2 ] ) ) + in2 [ 2 ][ 3 ] };
  }

  void vector_to_angles( vector3d forward, vector3d& angles )
  {
    float tmp, yaw, pitch;

    if( forward [ 1 ] == 0 && forward [ 0 ] == 0 )
    {
      yaw = 0;
      if( forward [ 2 ] > 0 )
        pitch = 270;
      else
        pitch = 90;
    }
    else
    {
      yaw = ( atan2( forward [ 1 ], forward [ 0 ] ) * 180 / PI );
      if( yaw < 0 )
        yaw += 360;

      tmp = sqrt( forward [ 0 ] * forward [ 0 ] + forward [ 1 ] * forward [ 1 ] );
      pitch = ( atan2( -forward [ 2 ], tmp ) * 180 / PI );
      if( pitch < 0 )
        pitch += 360;
    }

    angles [ 0 ] = pitch;
    angles [ 1 ] = yaw;
    angles [ 2 ] = 0;
  }

  vector3d cross_product( const vector3d& a, const vector3d& b )
  {
    return vector3d( a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x );
  }

  void vector_to_angles( vector3d& forward, vector3d& up, vector3d& angles )
  {
    vector3d left = cross_product( up, forward );
    left = left.normalized( );

    float forwardDist = forward.length( true );

    if( forwardDist > 0.001f )
    {
      angles.x = atan2f( -forward.z, forwardDist ) * 180 / 3.14159265358979323846f;
      angles.y = atan2f( forward.y, forward.x ) * 180 / 3.14159265358979323846f;

      float upZ = ( left.y * forward.x ) - ( left.x * forward.y );
      angles.z = atan2f( left.z, upZ ) * 180 / 3.14159265358979323846f;
    }
    else
    {
      angles.x = atan2f( -forward.z, forwardDist ) * 180 / 3.14159265358979323846f;
      angles.y = atan2f( -left.x, left.y ) * 180 / 3.14159265358979323846f;
      angles.z = 0;
    }
  }

  void angle_to_vectors( vector3d angles, vector3d& forward )
  {
    float sp, sy, cp, cy;

    sy = sin( math::deg_to_rad( angles [ 1 ] ) );
    cy = cos( math::deg_to_rad( angles [ 1 ] ) );

    sp = sin( math::deg_to_rad( angles [ 0 ] ) );
    cp = cos( math::deg_to_rad( angles [ 0 ] ) );

    forward.x = cp * cy;
    forward.y = cp * sy;
    forward.z = -sp;
  }

  void angle_to_vectors( vector3d angles, vector3d& forward, vector3d& right, vector3d& up )
  {
    float angle;
    static float sp, sy, cp, cy;

    angle = angles [ 0 ] * ( PI / 180.f );
    sp = sin( angle );
    cp = cos( angle );

    angle = angles [ 1 ] * ( PI / 180.f );
    sy = sin( angle );
    cy = cos( angle );

    forward [ 0 ] = cp * cy;
    forward [ 1 ] = cp * sy;
    forward [ 2 ] = -sp;

    static float sr, cr;

    angle = angles [ 2 ] * ( PI / 180.f );
    sr = sin( angle );
    cr = cos( angle );

    right [ 0 ] = -1 * sr * sp * cy + -1 * cr * -sy;
    right [ 1 ] = -1 * sr * sp * sy + -1 * cr * cy;
    right [ 2 ] = -1 * sr * cp;

    up [ 0 ] = cr * sp * cy + -sr * -sy;
    up [ 1 ] = cr * sp * sy + -sr * cy;
    up [ 2 ] = cr * cp;
  }

  bool is_near_equal( float v1, float v2, float toler )
  {
    return std::abs( v1 - v2 ) <= std::abs( toler );
  }

  vector3d angle_from_vectors( vector3d a, vector3d b )
  {
    vector3d angles{ };

    vector3d delta = a - b;
    float hyp = delta.length( true );

    // 57.295f - pi in degrees
    angles.y = std::atan( delta.y / delta.x ) * 57.2957795131f;
    angles.x = std::atan( -delta.z / hyp ) * -57.2957795131f;
    angles.z = 0.0f;

    if( delta.x >= 0.0f )
      angles.y += 180.0f;

    return angles;
  }

  float normalize( float ang )
  {
    while( ang < -180.f )
      ang += 360.f;
    while( ang > 180.f )
      ang -= 360.f;
    return ang;
  }

  vector3d normalize( vector3d ang, bool fix_pitch )
  {
    while( ang.y < -180.0f )
      ang.y += 360.0f;
    while( ang.y > 180.0f )
      ang.y -= 360.0f;

    if( fix_pitch )
    {
      if( ang.x > 89.0f )
        ang.x = 89.0f;
      if( ang.x < -89.0f )
        ang.x = -89.0f;
    }
    return ang;
  }

  float simple_spline( float value )
  {
    float sqr = value * value;
    return ( 3 * sqr - 2 * sqr * value );
  }

  float remap_val( float val, float A, float B, float C, float D )
  {
    if( A == B )
      return val >= B ? D : C;
    float cVal = ( val - A ) / ( B - A );
    cVal = std::clamp( cVal, 0.0f, 1.0f );
    return C + ( D - C ) * simple_spline( cVal );
  }

  float remap_val_clamped( float val, float A, float B, float C, float D )
  {
    if( A == B )
      return val >= B ? D : C;
    float cVal = ( val - A ) / ( B - A );
    cVal = std::clamp( cVal, 0.0f, 1.0f );

    return C + ( D - C ) * cVal;
  }

  float interpolate_inversed( float percent, const float& A, const float& B )
  {
    return A + ( B - A ) * percent;
  }

  float interpolate( const float& from, const float& to, const float& percent )
  {
    return to * percent + from * ( 1.f - percent );
  }

  vector3d interpolate( const vector3d& from, const vector3d& to, const float& percent )
  {
    return to * percent + from * ( 1.f - percent );
  }

  __forceinline int time_to_ticks( float time )
  {
    return ( int )( 0.5f + ( float )( time ) / interfaces::global_vars->interval_per_tick );
  }

  __forceinline float ticks_to_time( int ticks )
  {
    return ( float )ticks * interfaces::global_vars->interval_per_tick;
  }

  vector3d vector_rotate( vector3d in1, matrix3x4_t in2 )
  {
    return vector3d( in1.dot( in2 [ 0 ] ), in1.dot( in2 [ 1 ] ), in1.dot( in2 [ 2 ] ) );
  }

  vector3d vector_rotate( const vector3d& in1, const vector3d& in2 )
  {
    matrix3x4_t matrix = { };
    matrix.angle_matrix( in2 );
    return vector_rotate( in1, matrix );
  }

  void vector_i_transform( const vector3d& in1, const matrix3x4_t& in2, vector3d& out )
  {
    out.x = ( in1.x - in2 [ 0 ][ 3 ] ) * in2 [ 0 ][ 0 ] + ( in1.y - in2 [ 1 ][ 3 ] ) * in2 [ 1 ][ 0 ] + ( in1.z - in2 [ 2 ][ 3 ] ) * in2 [ 2 ][ 0 ];
    out.y = ( in1.x - in2 [ 0 ][ 3 ] ) * in2 [ 0 ][ 1 ] + ( in1.y - in2 [ 1 ][ 3 ] ) * in2 [ 1 ][ 1 ] + ( in1.z - in2 [ 2 ][ 3 ] ) * in2 [ 2 ][ 1 ];
    out.z = ( in1.x - in2 [ 0 ][ 3 ] ) * in2 [ 0 ][ 2 ] + ( in1.y - in2 [ 1 ][ 3 ] ) * in2 [ 1 ][ 2 ] + ( in1.z - in2 [ 2 ][ 3 ] ) * in2 [ 2 ][ 2 ];
  }

  void vector_i_rotate( const vector3d& in1, const matrix3x4_t& in2, vector3d& out )
  {
    out.x = in1.x * in2 [ 0 ][ 0 ] + in1.y * in2 [ 1 ][ 0 ] + in1.z * in2 [ 2 ][ 0 ];
    out.y = in1.x * in2 [ 0 ][ 1 ] + in1.y * in2 [ 1 ][ 1 ] + in1.z * in2 [ 2 ][ 1 ];
    out.z = in1.x * in2 [ 0 ][ 2 ] + in1.y * in2 [ 1 ][ 2 ] + in1.z * in2 [ 2 ][ 2 ];
  }

  bool intersect_line_with_bb( vector3d& start, vector3d& end, vector3d& min, vector3d& max )
  {
    float d1, d2, f;
    auto start_solid = true;
    auto t1 = -1.0f, t2 = 1.0f;

    const float s [ 3 ] = { start.x, start.y, start.z };
    const float e [ 3 ] = { end.x, end.y, end.z };
    const float mi [ 3 ] = { min.x, min.y, min.z };
    const float ma [ 3 ] = { max.x, max.y, max.z };

    for( auto i = 0; i < 6; i++ )
    {
      if( i >= 3 )
      {
        const auto j = i - 3;

        d1 = s [ j ] - ma [ j ];
        d2 = d1 + e [ j ];
      }
      else
      {
        d1 = -s [ i ] + mi [ i ];
        d2 = d1 - e [ i ];
      }

      if( d1 > 0.0f && d2 > 0.0f )
        return false;

      if( d1 <= 0.0f && d2 <= 0.0f )
        continue;

      if( d1 > 0 )
        start_solid = false;

      if( d1 > d2 )
      {
        f = d1;
        if( f < 0.0f )
          f = 0.0f;

        f /= d1 - d2;
        if( f > t1 )
          t1 = f;
      }
      else
      {
        f = d1 / ( d1 - d2 );
        if( f < t2 )
          t2 = f;
      }
    }

    return start_solid || ( t1 < t2 && t1 >= 0.0f );
  }

  float segment_to_segment( const vector3d& s1, const vector3d& s2, vector3d& k1, vector3d& k2 )
  {
    static auto constexpr epsilon = 0.00000001;

    auto u = s2 - s1;
    auto v = k2 - k1;
    const auto w = s1 - k1;

    const auto a = u.dot( u );
    const auto b = u.dot( v );
    const auto c = v.dot( v );
    const auto d = u.dot( w );
    const auto e = v.dot( w );
    const auto D = a * c - b * b;
    float sn, sd = D;
    float tn, td = D;

    if( D < epsilon )
    {
      sn = 0.0;
      sd = 1.0;
      tn = e;
      td = c;
    }
    else
    {
      sn = b * e - c * d;
      tn = a * e - b * d;

      if( sn < 0.0 )
      {
        sn = 0.0;
        tn = e;
        td = c;
      }
      else if( sn > sd )
      {
        sn = sd;
        tn = e + b;
        td = c;
      }
    }

    if( tn < 0.0 )
    {
      tn = 0.0;

      if( -d < 0.0 )
        sn = 0.0;
      else if( -d > a )
        sn = sd;
      else
      {
        sn = -d;
        sd = a;
      }
    }
    else if( tn > td )
    {
      tn = td;

      if( -d + b < 0.0 )
        sn = 0;
      else if( -d + b > a )
        sn = sd;
      else
      {
        sn = -d + b;
        sd = a;
      }
    }

    const float sc = abs( sn ) < epsilon ? 0.0 : sn / sd;
    const float tc = abs( tn ) < epsilon ? 0.0 : tn / td;

    m128 n;
    auto dp = w + u * sc - v * tc;
    n.f [ 0 ] = dp.dot( dp );
    const auto calc = sqrt_ps( n.v );
    return reinterpret_cast< const m128* >( &calc )->f [ 0 ];
  }

  void matrix_copy( const matrix3x4_t& source, matrix3x4_t& target )
  {
    for( int i = 0; i < 3; i++ )
    {
      for( int j = 0; j < 4; j++ )
      {
        target [ i ][ j ] = source [ i ][ j ];
      }
    }
  }

  void contact_transforms( const matrix3x4_t& in1, const matrix3x4_t& in2, matrix3x4_t& out )
  {
    if( &in1 == &out )
    {
      matrix3x4_t in1b;
      matrix_copy( in1, in1b );
      contact_transforms( in1b, in2, out );
      return;
    }

    if( &in2 == &out )
    {
      matrix3x4_t in2b;
      matrix_copy( in2, in2b );
      contact_transforms( in1, in2b, out );
      return;
    }

    out [ 0 ][ 0 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 0 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 0 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 0 ];
    out [ 0 ][ 1 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 1 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 1 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 1 ];
    out [ 0 ][ 2 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 2 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 2 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 2 ];
    out [ 0 ][ 3 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 3 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 3 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 3 ] + in1 [ 0 ][ 3 ];

    out [ 1 ][ 0 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 0 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 0 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 0 ];
    out [ 1 ][ 1 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 1 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 1 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 1 ];
    out [ 1 ][ 2 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 2 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 2 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 2 ];
    out [ 1 ][ 3 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 3 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 3 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 3 ] + in1 [ 1 ][ 3 ];

    out [ 2 ][ 0 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 0 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 0 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 0 ];
    out [ 2 ][ 1 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 1 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 1 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 1 ];
    out [ 2 ][ 2 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 2 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 2 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 2 ];
    out [ 2 ][ 3 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 3 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 3 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 3 ] + in1 [ 2 ][ 3 ];
  }

  typedef void ( *random_seed_fn )( UINT );
  random_seed_fn o_random_seed = 0;

  void random_seed( uint32_t seed )
  {
    if( !o_random_seed )
      o_random_seed = ( random_seed_fn )GetProcAddress( modules::vstdlib, xor_str( "RandomSeed" ).c_str( ) );
    o_random_seed( seed );
  }

  typedef float ( *random_float_fn )( float, float );
  random_float_fn o_random_float;

  float random_float( float min, float max )
  {
    if( !o_random_float )
      o_random_float = ( random_float_fn )GetProcAddress( modules::vstdlib, xor_str( "RandomFloat" ).c_str( ) );

    return o_random_float( min, max );
  }

  typedef int ( *random_int_fn )( int, int );
  random_int_fn o_random_int;

  int random_int( int min, int max )
  {
    if( !o_random_int )
      o_random_int = ( random_int_fn )GetProcAddress( modules::vstdlib, xor_str( "RandomInt" ).c_str( ) );

    return o_random_int( min, max );
  }

  void vector_multiply( const vector3d& start, float scale, const vector3d& direction, vector3d& dest )
  {
    dest.x = start.x + direction.x * scale;
    dest.y = start.y + direction.y * scale;
    dest.z = start.z + direction.z * scale;
  }

  // thanks to https://www.unknowncheats.me/forum/counterstrike-global-offensive/282111-offscreen-esp.html
  void rotate_triangle_points( vector2d points [ 3 ], float rotation )
  {
    const auto points_center = ( points [ 0 ] + points [ 1 ] + points [ 2 ] ) / vector2d( 3.f, 3.f );
    for( int i = 0; i < 3; i++ )
    {
      vector2d& point = points [ i ];

      point -= points_center;

      const auto temp_x = point.x;
      const auto temp_y = point.y;

      const auto theta = rotation;
      const auto c = std::cos( theta );
      const auto s = std::sin( theta );

      point.x = temp_x * c - temp_y * s;
      point.y = temp_x * s + temp_y * c;

      point += points_center;
    }
  }

  void matrix_multiply( matrix3x4_t& in1, const matrix3x4_t& in2 )
  {
    matrix3x4_t out;
    if( &in1 == &out )
    {
      matrix3x4_t in1b;
      matrix_copy( in1, in1b );
      matrix_multiply( in1b, in2 );
      return;
    }
    if( &in2 == &out )
    {
      matrix3x4_t in2b;
      matrix_copy( in2, in2b );
      matrix_multiply( in1, in2b );
      return;
    }
    out [ 0 ][ 0 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 0 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 0 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 0 ];
    out [ 0 ][ 1 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 1 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 1 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 1 ];
    out [ 0 ][ 2 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 2 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 2 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 2 ];
    out [ 0 ][ 3 ] = in1 [ 0 ][ 0 ] * in2 [ 0 ][ 3 ] + in1 [ 0 ][ 1 ] * in2 [ 1 ][ 3 ] + in1 [ 0 ][ 2 ] * in2 [ 2 ][ 3 ] + in1 [ 0 ][ 3 ];
    out [ 1 ][ 0 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 0 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 0 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 0 ];
    out [ 1 ][ 1 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 1 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 1 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 1 ];
    out [ 1 ][ 2 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 2 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 2 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 2 ];
    out [ 1 ][ 3 ] = in1 [ 1 ][ 0 ] * in2 [ 0 ][ 3 ] + in1 [ 1 ][ 1 ] * in2 [ 1 ][ 3 ] + in1 [ 1 ][ 2 ] * in2 [ 2 ][ 3 ] + in1 [ 1 ][ 3 ];
    out [ 2 ][ 0 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 0 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 0 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 0 ];
    out [ 2 ][ 1 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 1 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 1 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 1 ];
    out [ 2 ][ 2 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 2 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 2 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 2 ];
    out [ 2 ][ 3 ] = in1 [ 2 ][ 0 ] * in2 [ 0 ][ 3 ] + in1 [ 2 ][ 1 ] * in2 [ 1 ][ 3 ] + in1 [ 2 ][ 2 ] * in2 [ 2 ][ 3 ] + in1 [ 2 ][ 3 ];

    in1 = out;
  }

  void rotate_matrix( matrix3x4_t in [ 128 ], matrix3x4_t out [ 128 ], float delta, vector3d render_origin )
  {
    auto vDelta = vector3d( 0, delta, 0 );
    vector3d vOutPos;
    for( int i = 0; i < 128; i++ )
    {
      out [ i ].angle_matrix( vDelta );
      matrix_multiply( out [ i ], in [ i ] );
      auto vBonePos = vector3d( in [ i ][ 0 ][ 3 ], in [ i ][ 1 ][ 3 ], in [ i ][ 2 ][ 3 ] ) - render_origin;
      vOutPos = vector_rotate( vBonePos, vDelta );
      vOutPos += render_origin;
      out [ i ][ 0 ][ 3 ] = vOutPos.x;
      out [ i ][ 1 ][ 3 ] = vOutPos.y;
      out [ i ][ 2 ][ 3 ] = vOutPos.z;
    }
  }

  void change_matrix_position( matrix3x4_t* bones, size_t msize, vector3d current_position, vector3d new_position )
  {
    for( size_t i = 0; i < msize; ++i )
    {
      bones [ i ][ 0 ][ 3 ] += new_position.x - current_position.x;
      bones [ i ][ 1 ][ 3 ] += new_position.y - current_position.y;
      bones [ i ][ 2 ][ 3 ] += new_position.z - current_position.z;
    }
  }
}

float easings::east_out_cubic( float x )
{
  return 1.f - std::pow( 1.f - x, 3.f );
}

float easings::ease_out_quint( float x )
{
  return 1.f - std::pow( 1.f - x, 5.f );
}

float easings::ease_out_expo( float x )
{
  return x == 1.f ? 1.f : 1.f - std::pow( 2.f, -10.f * x );
}

float easings::ease_out_quad( float x )
{
  return 1.f - ( 1.f - x ) * ( 1.f - x );
}