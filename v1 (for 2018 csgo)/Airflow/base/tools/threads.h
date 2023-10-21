#pragma once
#include "../../includes.h"
#include "../../windows_includes.h"
#include "../../base/global_context.h"

#include <deque>
#include <mutex>
#include <thread>
#include <future>
#include <semaphore>
#include <algorithm>
#include <memory>
#include <functional>

template < typename _fn_t, typename... _args_t >
  requires std::is_invocable_v< _fn_t, _args_t... >
using invoke_t = std::invoke_result_t< std::decay_t< _fn_t >, std::decay_t< _args_t >... >;

// credits: magma(a).digital
// but it was reworked, now it use in-game threads and you don't need to allocate thread id's manually

class c_thread_pool
{
private:
  struct queue_t
  {
    std::mutex mutex{ };
    std::counting_semaphore<> watcher{ 0u };

    std::deque< std::function< void( ) > > elements{ };
    std::atomic< std::size_t > tasks_count{ };
  } current_queue{ };

  static void* work_loop( void* id );

  struct threads_t
  {
    HANDLE handle;
    DWORD id;
  };

  std::vector< threads_t > threads{ };

  using thread_func_fn = void* ( * )( void* );
  using create_simple_thread_fn = HANDLE( __cdecl* )( thread_func_fn, void*, SIZE_T );
  using release_thread_handle_fn = int( __cdecl* )( HANDLE );

public:
  void create_threads( int cnt )
  {
    static auto create_simple_thread = ( create_simple_thread_fn )GetProcAddress( modules::tier0, xor_c( "CreateSimpleThread" ) );

    if( !create_simple_thread )
      return;

    for( int i = 0; i < cnt; ++i )
    {
      auto& current_thread = threads.emplace_back( );
      current_thread.handle = create_simple_thread( work_loop, ( void* )&current_thread.id, 0 );
    }
  }

  void destroy_threads( )
  {
    static auto release_thread_handle = ( release_thread_handle_fn )GetProcAddress( modules::tier0, xor_c( "ReleaseThreadHandle" ) );

    if( !release_thread_handle )
      return;

    for( auto& i : threads )
      release_thread_handle( i.handle );

    threads.clear( );

    if( current_queue.mutex.try_lock( ) )
      current_queue.mutex.unlock( );

    current_queue.elements.clear( );
    current_queue.tasks_count = 0;
    current_queue.watcher.release( );
  }

  void wait( ) const;

  template < typename _fn_t, typename... _args_t >
    requires std::is_invocable_v< _fn_t, _args_t... >
  std::future< invoke_t< _fn_t, _args_t... > > enqueue( _fn_t&& fn, _args_t&&... args )
  {
    const auto task = std::make_shared< std::packaged_task< invoke_t< _fn_t, _args_t... >( ) > >( std::bind( std::forward< _fn_t >( fn ), std::forward< _args_t >( args )... ) );

    auto ret = task->get_future( );

    {
      const std::unique_lock< std::mutex > lock( current_queue.mutex );
      current_queue.elements.emplace_back( [ = ]( ) { ( *task )( ); } );
    }

    ++current_queue.tasks_count;
    current_queue.watcher.release( );

    return ret;
  }

  queue_t& queue( );
};

inline std::unique_ptr< c_thread_pool > g_thread_pool = std::make_unique< c_thread_pool >( );