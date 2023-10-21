#pragma once
#include <vector>
#include <mutex>

#include "../../additionals/minhook/MinHook.h"
#include "../../includes.h"

namespace hooks
{
  namespace detour
  {
    struct hook_t
    {
      bool enabled = false;
      void* target = nullptr;
      void* original = nullptr;
      void* custom = nullptr;

      void enable( )
      {
        MH_EnableHook( target );
        enabled = true;
      }

      void disable( )
      {
        MH_DisableHook( target );
        enabled = false;
      }
    };

    class c_hooks
    {
      std::vector< hook_t > m_hooks;

    public:
      c_hooks( )
      {
        MH_Initialize( );
      }
      ~c_hooks( )
      {
        MH_Uninitialize( );
      }

      template < typename fn = uintptr_t >
      bool create_hook( fn custom_func, void* o_func )
      {
        hook_t hook = { };
        hook.target = o_func;
        hook.custom = custom_func;
        if( MH_CreateHook( o_func, custom_func, &hook.original ) == MH_OK )
        {
          m_hooks.emplace_back( hook );
          return true;
        }
        return false;
      }

      void enable( )
      {
        for( auto& h : m_hooks )
          h.enable( );
      }

      void restore( )
      {
        for( auto& h : m_hooks )
          h.disable( );
      }

      template < typename fn = uintptr_t, class ret = fn >
      ret original( fn custom_func )
      {
        auto found = std::find_if( m_hooks.begin( ), m_hooks.end( ), [ & ]( hook_t hook ) { return hook.custom == custom_func; } );
        if( found != m_hooks.end( ) )
          return ( ret )found->original;

        return nullptr;
      }
    };
  }

  namespace vmt
  {
    class c_protect_guard
    {
    public:
      c_protect_guard( void* base, uint32_t len, uint32_t protect )
      {
        this->base = base;
        this->len = len;

        VirtualProtect( base, len, protect, ( PDWORD )( &old_protect ) );
      }

      ~c_protect_guard( )
      {
        VirtualProtect( base, len, old_protect, ( PDWORD )( &old_protect ) );
      }

    private:
      void* base;
      uint32_t len;
      uint32_t old_protect;
    };

    class c_hooks
    {
    public:
      c_hooks( ): class_base( nullptr ), method_count( 0 ), shadow_vtable( nullptr ), original_vtable( nullptr ), indexes( { } ), unhooked( false )
      {
      }

      c_hooks( void* base ): class_base( base ), method_count( 0 ), shadow_vtable( nullptr ), original_vtable( nullptr ), indexes( { } ), unhooked( false )
      {
      }

      ~c_hooks( )
      {
        restore_vtable( );

        indexes.clear( );
      }

      void setup( void* base = nullptr )
      {
        if( base != nullptr )
          class_base = base;

        if( !class_base )
          return;

        original_vtable = *( uintptr_t** )( class_base );
        method_count = get_vtable_methods( original_vtable );

        if( method_count == 0 )
          return;

        shadow_vtable = new uintptr_t [ method_count + 1 ]( );

        shadow_vtable [ 0 ] = original_vtable [ -1 ];
        std::memcpy( &shadow_vtable [ 1 ], original_vtable, method_count * sizeof( uintptr_t ) );

        c_protect_guard guard = c_protect_guard{ class_base, sizeof( uintptr_t ), PAGE_READWRITE };
        *( uintptr_t** )( class_base ) = &shadow_vtable [ 1 ];
      }

      template < typename T >
      void hook( uint32_t index, T method )
      {
        auto idx = index + 1;
        shadow_vtable [ idx ] = ( uintptr_t )( method );

#ifdef _DEBUG
        indexes.emplace_back( index );
#endif
        unhooked = false;
      }

      void unhook( uint32_t index )
      {
        auto idx = index + 1;
        shadow_vtable [ idx ] = original_vtable [ index ];
        unhooked = true;
      }

      void unhook_all( )
      {
        if( indexes.empty( ) )
          return;

        for( const auto& i : indexes )
          unhook( i );
      }

      template < typename T >
      T original( uint32_t index )
      {
        return ( T )original_vtable [ index ];
      }

      void restore_vtable( )
      {
        if( original_vtable != nullptr )
        {
          c_protect_guard guard = c_protect_guard{ class_base, sizeof( uintptr_t ), PAGE_READWRITE };
          *( uintptr_t** )( class_base ) = original_vtable;
          original_vtable = nullptr;
        }
      }

      bool was_unhooked( )
      {
        return unhooked;
      }

      uintptr_t* shadow_vtable{ nullptr };

    private:
      inline uint32_t get_vtable_methods( uintptr_t* vtable_start )
      {
        uint32_t len = -1;

        while( vtable_start [ len ] )
          len++;

        return len;
      }

      void* class_base{ nullptr };
      uint32_t method_count{ 0 };
      uintptr_t* original_vtable{ nullptr };
      bool unhooked{ false };

      std::vector< int > indexes{ };
    };
  }
}