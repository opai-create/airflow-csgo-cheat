#pragma once
#include "../tools/math.h"
#include "../tools/memory/memory.h"

#include "app_system.h"

#include "../../base/sdk/c_material.h"

enum class override_type_t
{
  normal = 0,
  buildshadows,
  depthwrite,
  custommaterial,
  ssaodepthwrite
};

struct draw_model_info_t
{
  void* studio_hdr;
  void* hardware_data;
  void* decals;
  int skin;
  int body;
  int hitbox_set;
  void* client_entity;
  int lod;
  void* color_meshes;
  bool static_lightning;
  void* lightning_state;

  inline draw_model_info_t operator=( const draw_model_info_t& other )
  {
    std::memcpy( this, &other, sizeof( draw_model_info_t ) );
    return *this;
  }
};

class c_studio_render
{
private:
  std::byte pad_0 [ 592 ];
  c_material* override_material;
  std::byte pad_1 [ 12 ];
  override_type_t override_type;

public:
  bool is_forced_material_override( )
  {
    if( !override_material )
      return override_type == override_type_t::depthwrite || override_type == override_type_t::ssaodepthwrite;

    static auto dev_glow = xor_str( "dev/glow" );
    return std::string_view{ override_material->get_name( ) }.starts_with( dev_glow );
  }
};