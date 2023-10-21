#pragma once
#include "../../base/other/color.h"
#include <string>
#include <array>
#include <memory>

class c_world_modulation
{
private:
  void skybox_changer( );
  void fog_changer( );
  void sunset_mode( );
  void light_props_modulation( );

public:
  std::string old_sky_name{ };
  void on_render_start( int stage );
};