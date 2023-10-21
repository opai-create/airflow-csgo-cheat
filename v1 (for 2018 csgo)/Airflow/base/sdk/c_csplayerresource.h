#pragma once
#include "../tools/netvar_parser.h"
#include "entity.h"
#include <memory>

class c_player_resource
{
public:
  rnetvar( player_c4, int, offsets::m_iPlayerC4 ) int ping( c_csplayer* player )
  {
    if( !player )
      return 0;

    return *( int* )( ( uintptr_t )this + offsets::m_iPing + ( player->index( ) * 4 ) );
  }
};