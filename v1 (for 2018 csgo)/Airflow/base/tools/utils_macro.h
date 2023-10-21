#pragma once
#define concat_impl( x, y ) x##y
#define macro_concat( x, y ) concat_impl( x, y )
#define padding( size )                                                                                                                                                                                                              \
  std::byte macro_concat( _pad, __COUNTER__ ) [ size ]                                                                                                                                                                               \
  {                                                                                                                                                                                                                                  \
  }

#define rnetvar( name, type, netvar )                                                                                                                                                                                                \
  __forceinline type name( )                                                                                                                                                                                                         \
  {                                                                                                                                                                                                                                  \
    static auto var = netvar;                                                                                                                                                                                                        \
    return *( type* )( ( uintptr_t )this + var );                                                                                                                                                                                    \
  }

#define netvar_ref( name, type, netvar )                                                                                                                                                                                             \
  __forceinline type& name( )                                                                                                                                                                                                        \
  {                                                                                                                                                                                                                                  \
    static auto var = netvar;                                                                                                                                                                                                        \
    return *( type* )( ( uintptr_t )this + var );                                                                                                                                                                                    \
  }

#define netvar_ptr( name, type, netvar )                                                                                                                                                                                             \
  __forceinline type* name( )                                                                                                                                                                                                        \
  {                                                                                                                                                                                                                                  \
    static auto var = netvar;                                                                                                                                                                                                        \
    return ( type* )( ( uintptr_t )this + var );                                                                                                                                                                                     \
  }

#define netvar_ptr_ref( name, type, netvar )                                                                                                                                                                                         \
  __forceinline type* name( )                                                                                                                                                                                                        \
  {                                                                                                                                                                                                                                  \
    static auto var = netvar;                                                                                                                                                                                                        \
    return *( type** )( ( uintptr_t )this + var );                                                                                                                                                                                   \
  }

#define declare_feature_ptr( name ) extern std::unique_ptr< c_##name > g_##name;
#define create_feature_ptr( name ) std::unique_ptr< c_##name > g_##name = nullptr;
#define update_feature_ptr( name ) g_##name = std::make_unique< c_##name >( );