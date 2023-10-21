#pragma once
#include "../../base/tools/math.h"
#include "../../base/tools/render.h"

#include "../../base/sdk/c_utlvector.h"

#include <vector>

struct notice_text_t
{
  wchar_t text [ 512 ];
  int unk0;
  float unk1;
  float unk2;
  int unk3;
  float time;
  int unk4;
  float fade;
  int unk5;
};

struct kill_feed_t
{
  padding( 0x7C );

  c_utlvector< notice_text_t > notices{ };
};

class c_view_setup;

class c_local_visuals
{
private:
  void thirdperson( );
  void modulate_bloom( );
  void remove_post_processing( );
  void remove_smoke( );
  void remove_flash( );
  void remove_viewmodel_sway( );
  void remove_visual_recoil( bool restore = false );
  void filter_console( );
  void fullbright( );
  void spoof_cvars( );
  void preverse_killfeed( );
  void force_ragdoll_gravity( );

  vector3d old_punch{ };
  vector3d old_view_punch{ };

  bool thirdperson_enabled{ };
  vector2d peek_w2s{ };

  std::vector< ImVec2 > peek_positions{ };

  float old_zoom_sensitivity{ };

public:
  void on_paint_traverse( );
  void on_directx( );
  void on_calc_view( );

  void on_render_start( int stage );
  void on_render_start_after( int stage );

  void on_render_view( c_view_setup* setup );
  void on_render_view_after( c_view_setup* setup );

  float last_duck_time{ };
};