#pragma once
#include "../tools/memory/memory.h"

class c_localize
{
public:
  wchar_t* find( const char* name )
  {
    using fn = wchar_t*( __thiscall* )( void*, const char* );
    return g_memory->getvfunc< fn >( this, 11 )( this, name );
  }

  const wchar_t* find_safe( const char* name )
  {
    using fn = wchar_t*( __thiscall* )( void*, const char* );
    return g_memory->getvfunc< fn >( this, 12 )( this, name );
  }
};