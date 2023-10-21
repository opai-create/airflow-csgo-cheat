#pragma once
#include "client.h"

class c_client_alpha_property;

class c_static_prop
{
public:
  char pad_0000 [ 72 ];         // 0x0000
  c_client_alpha_property* m_pClientAlphaProperty; // 0x0048
  char pad_004C [ 112 ];        // 0x004C
  vector4d m_DiffuseModulation;                    // 0x00BC
};

class c_static_prop_manager
{
public:
  void* N0000010D;                  // 0x0000
  void* N0000010E;                  // 0x0004
  char pad_0008 [ 20 ];             // 0x0008
  c_static_prop* m_StaticPropsBase; // 0x001C
  uint32_t m_StaticPropsCount;      // 0x0020
  uint32_t m_StaticPropsUnk;        // 0x0024
  uint32_t m_StaticPropsCnt2;       // 0x0028
  char pad_002C [ 24 ];             // 0x002C
  bool m_bLevelInitialized;         // 0x0044
  bool m_bClientInitialized;        // 0x0045
  char pad_0046 [ 2 ];              // 0x0046
  vector3d m_vecLastViewOrigin;     // 0x0048
  float m_flLastViewFactor;         // 0x0054
  uint32_t m_nLastCPULevel;         // 0x0058
  uint32_t m_nLastGPULevel;         // 0x005C
};

class c_view_render
{
public:
  char pad [ 4 ];
  c_view_setup view;
};

class c_render_view
{
public:
  void set_blend( float value )
  {
    using fn = void( __thiscall* )( void*, float );
    return g_memory->getvfunc< fn >( this, 4 )( this, value );
  }

  float get_blend( )
  {
    using fn = float( __thiscall* )( void* );
    return g_memory->getvfunc< fn >( this, 5 )( this );
  }

  void set_color_modulation( float* value )
  {
    using fn = void( __thiscall* )( void*, float* );
    return g_memory->getvfunc< fn >( this, 6 )( this, value );
  }

  void get_color_modulation( float* value )
  {
    using fn = void( __thiscall* )( void*, float* );
    return g_memory->getvfunc< fn >( this, 7 )( this, value );
  }
};