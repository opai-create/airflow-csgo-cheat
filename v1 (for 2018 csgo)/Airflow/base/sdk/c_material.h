#pragma once
#include "../tools/memory/memory.h"
#include "../tools/math.h"
#include "../tools/utils_macro.h"

enum material_var_flags_t
{
  material_var_debug = ( 1 << 0 ),
  material_var_no_debug_override = ( 1 << 1 ),
  material_var_no_draw = ( 1 << 2 ),
  material_var_use_in_fillrate_mode = ( 1 << 3 ),
  material_var_vertexcolor = ( 1 << 4 ),
  material_var_vertexalpha = ( 1 << 5 ),
  material_var_selfillum = ( 1 << 6 ),
  material_var_additive = ( 1 << 7 ),
  material_var_alphatest = ( 1 << 8 ),
  material_var_multipass = ( 1 << 9 ),
  material_var_znearer = ( 1 << 10 ),
  material_var_model = ( 1 << 11 ),
  material_var_flat = ( 1 << 12 ),
  material_var_nocull = ( 1 << 13 ),
  material_var_nofog = ( 1 << 14 ),
  material_var_ignorez = ( 1 << 15 ),
  material_var_decal = ( 1 << 16 ),
  material_var_envmapsphere = ( 1 << 17 ),
  material_var_noalphamod = ( 1 << 18 ),
  material_var_envmapcameraspace = ( 1 << 19 ),
  material_var_basealphaenvmapmask = ( 1 << 20 ),
  material_var_translucent = ( 1 << 21 ),
  material_var_normalmapalphaenvmapmask = ( 1 << 22 ),
  material_var_needs_software_skinning = ( 1 << 23 ),
  material_var_opaquetexture = ( 1 << 24 ),
  material_var_envmapmode = ( 1 << 25 ),
  material_var_suppress_decals = ( 1 << 26 ),
  material_var_halflambert = ( 1 << 27 ),
  material_var_wireframe = ( 1 << 28 ),
  material_var_allowalphatocoverage = ( 1 << 29 ),
  material_var_ignore_alpha_modulation = ( 1 << 30 ),
  material_var_vertexfog = ( 1 << 31 ),
};

class c_key_values
{
public:
  padding( 40 );
};

class c_material_var
{
public:
  void set_vec_value( float value )
  {
    using fn = void( __thiscall* )( void*, float );
    g_memory->getvfunc< fn >( this, 4 )( this, value );
  }

  void set_vec_value( int value )
  {
    using fn = void( __thiscall* )( void*, int );
    g_memory->getvfunc< fn >( this, 5 )( this, value );
  }

  void set_vec_value( const char* value )
  {
    using fn = void( __thiscall* )( void*, const char* );
    g_memory->getvfunc< fn >( this, 6 )( this, value );
  }

  void set_vec_value( float x, float y, float z )
  {
    using fn = void( __thiscall* )( void*, float, float, float );
    g_memory->getvfunc< fn >( this, 11 )( this, x, y, z );
  }
};

class c_material
{
public:
  virtual const char* get_name( ) const = 0;
  virtual const char* get_texture_group_name( ) const = 0;

  virtual int get_preview_image_properties( int* width, int* height, void* image_format, bool* translucent ) const = 0;
  virtual int get_preview_image( unsigned char* data, int width, int height, void* image_format ) const = 0;

  virtual int get_mapping_width( ) = 0;
  virtual int get_mapping_height( ) = 0;

  virtual int get_num_animation_frames( ) = 0;

  virtual bool in_material_page( ) = 0;

  virtual void get_material_offset( float* offset ) = 0;
  virtual void get_material_scale( float* scale ) = 0;

  virtual c_material* get_material_page( ) = 0;
  virtual c_material_var* find_var( const char* name, bool* found, bool complain = true ) = 0;

  virtual void increment_reference_count( ) = 0;
  virtual void decrement_reference_count( ) = 0;

  inline void add_ref( )
  {
    increment_reference_count( );
  }
  inline void release( )
  {
    decrement_reference_count( );
  }

  virtual int get_enum_id( ) const = 0;

  virtual void get_low_res_clr_sample( float s, float t, float* color ) const = 0;

  virtual void recompute_state_snapshots( ) = 0;

  virtual bool is_translucent( ) = 0;
  virtual bool is_alpha_tested( ) = 0;
  virtual bool is_vertex_lit( ) = 0;

  virtual int get_vertex_format( ) const = 0;

  virtual bool has_proxy( ) const = 0;
  virtual bool use_cube_map( ) = 0;

  virtual bool needs_tangent_space( ) = 0;
  virtual bool needs_power_of_two_frame_buffer_texture( bool check_specific_to_this_frame = true ) = 0;
  virtual bool needs_full_frame_buffer_texture( bool check_specific_to_this_frame = true ) = 0;
  virtual bool needs_software_skinning( ) = 0;

  virtual void alpha_modulate( float alpha ) = 0;
  virtual void color_modulate( float r, float g, float b ) = 0;

  virtual void set_material_var_flag( material_var_flags_t flag, bool on ) = 0;
  virtual bool get_material_var_flag( material_var_flags_t flag ) const = 0;

  virtual void get_reflectivity( vector3d& reflect ) = 0;

  virtual bool get_property_flag( int type ) = 0;

  virtual bool is_two_sided( ) = 0;

  virtual void set_shader( const char* shader_name ) = 0;

  virtual int get_num_passes( ) = 0;
  virtual int get_texture_memory_bytes( ) = 0;

  virtual void refresh( ) = 0;

  virtual bool needs_lightmap_blend_alpha( ) = 0;
  virtual bool needs_software_lightning( ) = 0;

  virtual int shader_param_count( ) const = 0;
  virtual c_material_var** get_shader_params( ) = 0;

  virtual bool is_error_material( ) const = 0;
  virtual void unused( ) = 0;

  virtual float get_alpha_modulation( ) = 0;
  virtual void get_color_modulation( float* r, float* g, float* b ) = 0;
  virtual bool is_translucent_under_modulation( float alpha_modulation = 1.0f ) const = 0;

  virtual c_material_var* find_var_fast( char const* name, unsigned int* token ) = 0;

  virtual void set_shader_and_params( c_key_values* pKeyValues ) = 0;
  virtual const char* get_shader_name( ) const = 0;

  virtual void delete_if_unreferenced( ) = 0;

  virtual bool is_spite_card( ) = 0;

  virtual void call_bind_proxy( void* data ) = 0;

  virtual void refresh_preversing_material_vars( ) = 0;

  virtual bool was_reloaded_from_whitelist( ) = 0;

  virtual bool set_tem_excluded( bool set, int excluded_dimension_limit ) = 0;

  virtual int get_reference_count( ) const = 0;

  void set_material_var_flag_custom( material_var_flags_t flag, bool on )
  {
    if( !*( uintptr_t* )( ( uintptr_t )this + 0x24 ) )
      return;

    g_memory->getvfunc< void( __thiscall* )( void*, material_var_flags_t, bool ) >( this, 29 )( this, flag, on );
  }
};