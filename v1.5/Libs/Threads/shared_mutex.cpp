#include "shared_mutex.h"

#include <windows.h>

SharedMutex::SharedMutex( )
{
  ::InitializeSRWLock( &lock );
}

SharedMutex::~SharedMutex( )
{
  // No release function
}

void SharedMutex::rlock( )
{
  ::AcquireSRWLockShared( &lock );
}

bool SharedMutex::tryrlock( )
{
  return ::TryAcquireSRWLockShared( &lock );
}

void SharedMutex::runlock( )
{
  ::ReleaseSRWLockShared( &lock );
}

void SharedMutex::wlock( )
{
  ::AcquireSRWLockExclusive( &lock );
}

bool SharedMutex::trywlock( )
{
  return ::TryAcquireSRWLockExclusive( &lock );
}

void SharedMutex::wunlock( )
{
  ::ReleaseSRWLockExclusive( &lock );
}