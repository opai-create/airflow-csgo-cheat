#ifndef SHARED_MUTEX_H
#define SHARED_MUTEX_H

#include <windows.h>

class SharedMutex
{
public:
  SharedMutex( );
  ~SharedMutex( );
  void rlock( );
  bool tryrlock( );
  void runlock( );
  void wlock( );
  bool trywlock( );
  void wunlock( );

private:
  SRWLOCK lock;
};

#endif
