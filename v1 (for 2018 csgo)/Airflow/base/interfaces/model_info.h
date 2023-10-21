#pragma once
#include "../tools/memory/memory.h"

struct studio_hdr_t;
struct studio_hw_data_t;
struct model_t;

class c_model_info
{
public:
  studio_hdr_t* get_studio_model( const model_t* mod )
  {
    using fn = studio_hdr_t*( __thiscall* )( void*, const model_t* );
    return g_memory->getvfunc< fn >( this, 30 )( this, mod );
  }

  int get_model_index( const char* name )
  {
    using fn = int( __thiscall* )( void*, const const char* );
    return g_memory->getvfunc< fn >( this, 2 )( this, name );
  }
};

class c_mdl_cache: public c_app_system
{
public:
  virtual void set_cache_notify( void* notify ) = 0;

  virtual unsigned short find_mdl( const char* mdl_path ) = 0;

  virtual int add_ref( unsigned short handle ) = 0;
  virtual int release( unsigned short handle ) = 0;
  virtual int get_ref( unsigned short handle ) = 0;

  virtual studio_hdr_t* get_studio_hdr( unsigned short handle ) = 0;
  virtual studio_hw_data_t* get_hardware_data( unsigned short handle ) = 0;
};