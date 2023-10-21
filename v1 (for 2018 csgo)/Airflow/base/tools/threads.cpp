#include "threads.h"

void* c_thread_pool::work_loop( void* id )
{
  auto& queue = g_thread_pool->queue( );

  while( true )
  {
    std::function< void( ) > task{ };

    queue.watcher.acquire( );

    {
      const std::unique_lock< std::mutex > lock( queue.mutex );

      if( queue.elements.empty( ) )
        continue;

      task = std::move( queue.elements.front( ) );
      queue.elements.pop_front( );
    }

    task( );

    --queue.tasks_count;
  }

  return nullptr;
}

void c_thread_pool::wait( ) const
{
  for( ; current_queue.tasks_count; )
    std::this_thread::yield( );
}

c_thread_pool::queue_t& c_thread_pool::queue( )
{
  return current_queue;
}