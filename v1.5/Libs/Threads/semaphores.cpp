#include "semaphores.h"

Semaphore::Semaphore( bool shared )
{
  // Unnamed shared semaphores do not work on windows
  if( shared )
#if defined( __cpp_exceptions ) || defined( _CPPUNWIND )
	throw;
#else
	return;
#endif
  sm = CreateSemaphoreA( nullptr, 0, 0xffff, nullptr );
}

Semaphore::~Semaphore( )
{
  CloseHandle( sm );
}

void Semaphore::Wait( )
{
  WaitForSingleObject( sm, INFINITE );
}

int Semaphore::TimedWait( size_t milliseconds )
{
  if( WaitForSingleObject( sm, milliseconds ) == WAIT_OBJECT_0 )
	return 0;
  return 1;
}

void Semaphore::Post( )
{
  ReleaseSemaphore( sm, 1, NULL );
}

unsigned long Semaphore::Count( )
{
  long previous;
  switch( WaitForSingleObject( sm, 0 ) )
  {
  case WAIT_OBJECT_0:
	ReleaseSemaphore( sm, 1, &previous );
	return previous + 1;
  case WAIT_TIMEOUT:
	return 0;
  }
  return 0;
}