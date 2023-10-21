#include "mutex.h"
#include <windows.h>

Mutex::Mutex( )
{
  ::InitializeCriticalSection( &lck );
}

Mutex::~Mutex( )
{
  ::DeleteCriticalSection( &lck );
}

void Mutex::lock( )
{
  ::EnterCriticalSection( &lck );
}

void Mutex::unlock( )
{
  ::LeaveCriticalSection( &lck );
}