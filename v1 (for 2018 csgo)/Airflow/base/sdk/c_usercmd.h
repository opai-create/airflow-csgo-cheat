#pragma once
#include "../tools/math.h"
#include "../other/checksum_crc.h"

enum cmd_buttons_t
{
  in_attack = ( 1 << 0 ),
  in_jump = ( 1 << 1 ),
  in_duck = ( 1 << 2 ),
  in_forward = ( 1 << 3 ),
  in_back = ( 1 << 4 ),
  in_use = ( 1 << 5 ),
  in_cancel = ( 1 << 6 ),
  in_left = ( 1 << 7 ),
  in_right = ( 1 << 8 ),
  in_moveleft = ( 1 << 9 ),
  in_moveright = ( 1 << 10 ),
  in_attack2 = ( 1 << 11 ),
  in_run = ( 1 << 12 ),
  in_reload = ( 1 << 13 ),
  in_alt1 = ( 1 << 14 ),
  in_alt2 = ( 1 << 15 ),
  in_score = ( 1 << 16 ),
  in_speed = ( 1 << 17 ),
  in_walk = ( 1 << 18 ),
  in_zoom = ( 1 << 19 ),
  in_weapon1 = ( 1 << 20 ),
  in_weapon2 = ( 1 << 21 ),
  in_bullrush = ( 1 << 22 ),
  in_grenade1 = ( 1 << 23 ),
  in_grenade2 = ( 1 << 24 ),
  in_lookspin = ( 1 << 25 ),
};

class c_usercmd
{
public:
  c_usercmd( )
  {
    std::memset( this, 0, sizeof( *this ) );
  };
  virtual ~c_usercmd( ){ };

  crc32_t get_check_sum( void ) const
  {
    crc32_t crc;
    crc32_Init( &crc );

    crc32_process_buffer( &crc, &command_number, sizeof( command_number ) );
    crc32_process_buffer( &crc, &tickcount, sizeof( tickcount ) );
    crc32_process_buffer( &crc, &viewangles, sizeof( viewangles ) );
    crc32_process_buffer( &crc, &aimdirection, sizeof( aimdirection ) );
    crc32_process_buffer( &crc, &forwardmove, sizeof( forwardmove ) );
    crc32_process_buffer( &crc, &sidemove, sizeof( sidemove ) );
    crc32_process_buffer( &crc, &upmove, sizeof( upmove ) );
    crc32_process_buffer( &crc, &buttons, sizeof( buttons ) );
    crc32_process_buffer( &crc, &impulse, sizeof( impulse ) );
    crc32_process_buffer( &crc, &weaponselect, sizeof( weaponselect ) );
    crc32_process_buffer( &crc, &weaponsubtype, sizeof( weaponsubtype ) );
    crc32_process_buffer( &crc, &random_seed, sizeof( random_seed ) );
    crc32_process_buffer( &crc, &mousedx, sizeof( mousedx ) );
    crc32_process_buffer( &crc, &mousedy, sizeof( mousedy ) );

    crc32_final( &crc );

    return crc;
  }

  int command_number{ };
  int tickcount{ };

  vector3d viewangles{ };
  vector3d aimdirection{ };

  float forwardmove{ };
  float sidemove{ };
  float upmove{ };

  int buttons{ };
  char impulse{ };

  int weaponselect{ };
  int weaponsubtype{ };
  int random_seed{ };

  short mousedx{ };
  short mousedy{ };

  bool predicted{ };

  padding( 0x18 );

  __forceinline void invalidate_packets( )
  {
    this->tickcount = INT_MAX;
    this->buttons &= ~( in_attack | in_attack2 );

    this->forwardmove = this->sidemove = this->upmove = 0.f;
  }
};