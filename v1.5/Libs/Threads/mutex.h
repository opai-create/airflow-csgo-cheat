#ifndef MUTEX_H
#define MUTEX_H

#include <windows.h>

class Mutex
{
public:
  Mutex( );
  ~Mutex( );
  void lock( );
  bool trylock( );
  void unlock( );
  // private:

  CRITICAL_SECTION lck;
};
#endif