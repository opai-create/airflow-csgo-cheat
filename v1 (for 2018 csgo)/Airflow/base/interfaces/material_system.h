#pragma once
#include "../tools/math.h"

#include "../sdk/studio_hdr.h"

enum studiorender_flags_t
{
  studiorender_draw_entire_model = 0,
  studiorender_draw_opaque_only = 0x01,
  studiorender_draw_translucent_only = 0x02,
  studiorender_draw_group_mask = 0x03,
  studiorender_draw_no_flexes = 0x04,
  studiorender_draw_static_lighting = 0x08,
  studiorender_draw_accuratetime = 0x10,
  studiorender_draw_no_shadows = 0x20,
  studiorender_draw_get_perf_stats = 0x40,
  studiorender_draw_wireframe = 0x80,
  studiorender_draw_item_blink = 0x100,
  studiorender_shadowdepthtexture = 0x200,
  studiorender_unused = 0x400,
  studiorender_skip_decals = 0x800,
  studiorender_model_is_cacheable = 0x1000,
  studiorender_shadowdepthtexture_include_translucent_materials = 0x2000,
  studiorender_no_primary_draw = 0x4000,
  studiorender_ssaodepthtexture = 0x8000,
};

enum shader_stencil_op_t
{
  shader_stencilop_keep = 1,
  shader_stencilop_zero = 2,
  shader_stencilop_set_to_reference = 3,
  shader_stencilop_increment_clamp = 4,
  shader_stencilop_decrement_clamp = 5,
  shader_stencilop_invert = 6,
  shader_stencilop_increment_wrap = 7,
  shader_stencilop_decrement_wrap = 8,
  shader_stencilop_force_dword = 0x7FFFFFFF
};

enum shader_stencil_func_t
{
  shader_stencilfunc_never = 1,
  shader_stencilfunc_less = 2,
  shader_stencilfunc_equal = 3,
  shader_stencilfunc_lequal = 4,
  shader_stencilfunc_greater = 5,
  shader_stencilfunc_notequal = 6,
  shader_stencilfunc_gequal = 7,
  shader_stencilfunc_always = 8,
  shader_stencilfunc_force_dword = 0x7FFFFFFF
};

enum renderable_lightning_model_t
{
  lighting_model_none = -1,
  lighting_model_standard = 0,
  lighting_model_static_prop,
  lighting_model_physics_prop,

  lighting_model_count,
};

struct light_desc_t
{
  int type;
  vector3d color;
  vector3d position;
  vector3d direction;
  float range;
  float falloff;
  float attenuation0;
  float attenuation1;
  float attenuation2;
  float theta;
  float phi;
  float theta_dot;
  float phi_dot;
  float over_theta_dot_minus_dot;
  std::uint32_t flags;

protected:
  float range_squared;
};

struct render_instance_t
{
  uint8_t alpha;
};

struct flash_light_instance_t
{
  c_material* debug_material;
  char padding [ 248 ];
  matrix3x4_t world_to_texture;
  void* flash_depth_light_texture;
};

struct studio_array_data_t
{
  studio_hdr_t* studio_hdr;
  studio_hw_data_t* hardware_data;
  void* instance_data;
  int count;
};

struct studio_model_array_info2_t
{
  int flash_count;
  flash_light_instance_t* flashlights;
};

struct studio_model_array_info_t: public studio_model_array_info2_t
{
  studio_hdr_t* studio_hdr;
  studio_hw_data_t* hardware_data;
};

struct model_render_system_data_t
{
  void* renderable;
  void* renderable_model;
  render_instance_t instance_data;
};

struct shader_stencil_state_t
{
  bool enable;
  shader_stencil_op_t fail_op;
  shader_stencil_op_t z_fail_op;
  shader_stencil_op_t pass_op;
  shader_stencil_func_t compare_func;
  int reference_value;
  uint32_t test_mask;
  uint32_t write_masl;

  shader_stencil_state_t( )
  {
    enable = false;
    fail_op = z_fail_op = pass_op = shader_stencilop_keep;
    compare_func = shader_stencilfunc_always;
    reference_value = 0;
    test_mask = write_masl = 0xFFFFFFFF;
  }
};

struct studio_shadow_array_instance_data_t
{
  int lod;
  int body;
  int skin;
  matrix3x4_t* pose_to_world;
  float* flex_weights;
  float* delayed_flex_weights;
};

struct color_mesh_info_t
{
  void* mesh;
  void* all_allocator;
  int vert_offset;
  int num_verts;
};

struct material_lightning_state_t
{
  vector3d aimbient_cube [ 6 ];
  vector3d lightning_origin;
  int light_count;
  light_desc_t light_desc [ 4 ];
};

struct studio_array_instance_data_t: public studio_shadow_array_instance_data_t
{
  material_lightning_state_t* light_state;
  material_lightning_state_t* decal_light_state;
  void* env_cubemap_textuer;
  void* decals;
  uint32_t flash_usage;
  shader_stencil_state_t* stencil_state;
  color_mesh_info_t* color_mesh_info;
  bool mesh_has_light_only;
  vector4d diffuse_modulation;
};

struct model_list_mode_t
{
  model_render_system_data_t entry;
  int32_t initial_list_index : 24;
  uint32_t bone_merge : 1;
  int32_t lod : 7;
  shader_stencil_state_t* stencil_state;
  model_list_mode_t* next;
};

struct render_model_info_t: public studio_array_instance_data_t
{
  model_render_system_data_t entry;
  unsigned short instance;
  matrix3x4_t* bone_to_world;
  uint32_t list_idx : 24;
  uint32_t setup_bones_only : 1;
  uint32_t bone_merge : 1;
};

struct model_list_by_type_t: public studio_model_array_info_t
{
  renderable_lightning_model_t light_model;
  const model_t* model;
  model_list_mode_t* first_node;
  int count;
  int setup_bone_count;
  uint32_t parent_depth : 31;
  uint32_t wants_stencil : 1;
  render_model_info_t* render_models;
  model_list_by_type_t* next_lightning_model;

  model_list_by_type_t& operator=( const model_list_by_type_t& rhs )
  {
    memcpy( this, &rhs, sizeof( model_list_by_type_t ) );
    return *this;
  }

  model_list_by_type_t( )
  {
  }

  model_list_by_type_t( const model_list_by_type_t& rhs )
  {
    std::memcpy( this, &rhs, sizeof( model_list_by_type_t ) );
  }
};

class c_material_system
{
public:
  c_material* create_material( const char* name, c_key_values* key )
  {
    using fn = c_material*( __thiscall* )( void*, const char*, c_key_values* );
    return g_memory->getvfunc< fn >( this, 83 )( this, name, key );
  }

  c_material* find_material( const char* material_name, const char* group_name, bool complain = true, const char* complain_prefix = NULL )
  {
    using fn = c_material*( __thiscall* )( void*, const char*, const char*, bool, const char* );
    return g_memory->getvfunc< fn >( this, 84 )( this, material_name, group_name, complain, complain_prefix );
  }

  unsigned short first_material( )
  {
    using fn = unsigned short( __thiscall* )( void* );
    return g_memory->getvfunc< fn >( this, 86 )( this );
  }

  unsigned short next_material( unsigned short h )
  {
    using fn = unsigned short( __thiscall* )( void*, unsigned short h );
    return g_memory->getvfunc< fn >( this, 87 )( this, h );
  }

  unsigned short invalid_material( )
  {
    using fn = unsigned short( __thiscall* )( void* );
    return g_memory->getvfunc< fn >( this, 88 )( this );
  }

  c_material* get_material( unsigned short h )
  {
    using fn = c_material*( __thiscall* )( void*, unsigned short );
    return g_memory->getvfunc< fn >( this, 89 )( this, h );
  }

  void* get_render_context( )
  {
    using fn = void*( __thiscall* )( void* );
    return g_memory->getvfunc< fn >( this, 115 )( this );
  }
};

class c_key_values_system
{
public:
  virtual void unk( ) = 0;
  virtual void RegisterSizeofKeyValues( int iSize ) = 0;
  virtual void* AllocKeyValuesMemory( int iSize ) = 0;
  virtual void FreeKeyValuesMemory( void* pMemory ) = 0;
  virtual int GetSymbolForString( const char* szName, bool bCreate = true ) = 0;
  virtual const char* GetStringForSymbol( int hSymbol ) = 0;
  virtual void AddKeyValuesToMemoryLeakList( void* pMemory, int hSymbolName ) = 0;
  virtual void RemoveKeyValuesFromMemoryLeakList( void* pMemory ) = 0;
  virtual void SetKeyValuesExpressionSymbol( const char* szName, bool bValue ) = 0;
  virtual bool GetKeyValuesExpressionSymbol( const char* szName ) = 0;
  virtual int GetSymbolForStringCaseSensitive( int& hCaseInsensitiveSymbol, const char* szName, bool bCreate = true ) = 0;
};

using key_values_system_fn = c_key_values_system*( __cdecl* )( );