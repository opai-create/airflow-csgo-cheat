#pragma once
#include <d3d9.h>
#include <wrl/client.h>

#include "math.h"
#include "../../includes.h"

#include "../interfaces/surface.h"

#include "../../additionals/imgui/imgui.h"
#include "../../additionals/imgui/imgui_internal.h"
#include "../../additionals/imgui/imgui_freetype.h"
#include "../../additionals/imgui/imgui_impl_dx9.h"
#include "../../additionals/imgui/imgui_impl_win32.h"

#include "utils_macro.h"

using Microsoft::WRL::ComPtr;

enum font_flags_t : unsigned int
{
  centered_x = 1 << 0,
  centered_y = 1 << 1,
  dropshadow_ = 1 << 2,
  outline_ = 1 << 3,
  outline_light = 1 << 4,
  dropshadow_light = 1 << 5,
};

struct fonts_t
{
  ImFont* main{ };
  ImFont* misc{ };
  ImFont* esp{ };
  ImFont* bold{ };
  ImFont* bold2{ };
  ImFont* bold_large{ };
  ImFont* dmg{ };
  ImFont* large{ };
  ImFont* eventlog{ };
  ImFont* pixel_menu{ };
  ImFont* pixel{ };
  ImFont* weapon_icons{ };
  ImFont* weapon_icons_large{ };
};

inline fonts_t g_fonts{ };

namespace string_convert
{
  __forceinline std::string to_string( const std::wstring_view str )
  {
    if( str.empty( ) )
      return { };

    const auto len = WideCharToMultiByte( CP_UTF8, 0, str.data( ), str.size( ), 0, 0, 0, 0 );

    std::string ret{ };

    ret.resize( len );

    WideCharToMultiByte( CP_UTF8, 0, str.data( ), str.size( ), ret.data( ), len, 0, 0 );

    return ret;
  }

  __forceinline std::wstring to_wstring( const std::string_view& str )
  {
    if( str.empty( ) )
      return std::wstring( );

    int size_needed = MultiByteToWideChar( CP_UTF8, 0, &str [ 0 ], static_cast< int >( str.size( ) ), NULL, 0 );

    std::wstring out( size_needed, 0 );
    MultiByteToWideChar( CP_UTF8, 0, &str [ 0 ], static_cast< int >( str.size( ) ), &out [ 0 ], size_needed );

    return out;
  }
}

class c_render
{
private:
  DWORD old_color_write{ };
  DWORD old_srgb_write{ };

  DWORD old_antialias{ };
  DWORD old_multisample{ };

  IDirect3DVertexDeclaration9* vertex_dec{ };
  IDirect3DVertexShader9* vertex_shader{ };

public:
  void __stdcall create_objects( );
  void __stdcall invalidate_objects( );
  void __stdcall start_render( IDirect3DDevice9* device );
  void __stdcall end_render( IDirect3DDevice9* device );
  void __stdcall init( IDirect3DDevice9* device );

  void update_screen_size( );
  void line( float x1, float y1, float x2, float y2, color clr, float thickness = 1.f );
  void line_gradient( float x1, float y1, float x2, float y2, color clr1, color cl2, float thickness = 1.f );
  void blur( float x, float y, float w, float h, color clr, float rounding = 0.f );
  void rect( float x, float y, float w, float h, color clr, float rounding = 0.f, float thickness = 1.f );
  void filled_rect( float x, float y, float w, float h, color clr, float rounding = 0.f, ImDrawCornerFlags rounding_corners = 15 );
  void filled_rect_gradient( float x, float y, float w, float h, color col_upr_left, color col_upr_right, color col_bot_right, color col_bot_left );
  void triangle( float x1, float y1, float x2, float y2, float x3, float y3, color clr, float thickness = 1.f );
  void filled_triangle_gradient( float x1, float y1, float x2, float y2, float x3, float y3, color clr, color clr2, color clr3 );
  void filled_triangle( float x1, float y1, float x2, float y2, float x3, float y3, color clr );
  void circle( float x1, float y1, float radius, color col, int segments );
  void filled_circle( float x1, float y1, float radius, color col, int segments );
  const matrix3x4_t& world_to_screen_matrix( );
  bool screen_transform( const vector3d& source, vector2d& output );
  bool world_to_screen( const vector3d& source, vector2d& output, bool skip_screen = false );
  void string( float x, float y, color clr, int flags, ImFont* font, const std::string& message );
  void arc( float x, float y, float radius, float min_angle, float max_angle, color col, float thickness = 1.f );

  rect2d screen_size{ };
  ImDrawList* draw_list{ };
  IDirect3DDevice9* direct_device{ };

  __forceinline void enable_aa( )
  {
    draw_list->Flags |= ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines;
  }

  __forceinline void disable_aa( )
  {
    draw_list->Flags &= ~( ImDrawListFlags_AntiAliasedFill | ImDrawListFlags_AntiAliasedLines );
  }
};

declare_feature_ptr( render );

struct ImDrawList;
struct IDirect3DDevice9;

namespace imgui_blur
{
  void set_device( IDirect3DDevice9* device ) noexcept;
  void clear_blur_textures( ) noexcept;
  void create_textures( ) noexcept;
  void on_device_reset( ) noexcept;
  void new_frame( ) noexcept;
  void create_blur( ImDrawList* drawList, ImVec2 min, ImVec2 max, ImColor col = ImColor( 255, 255, 255, 255 ), float rounding = 0.f, ImDrawCornerFlags round_flags = 15 ) noexcept;
}
