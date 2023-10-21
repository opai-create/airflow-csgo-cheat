#pragma once
#include "../tools/utils_macro.h"

struct verified_cmd_t
{
  c_usercmd cmd{ };
  crc32_t crc{ };
};

class c_input
{
public:
  void* vtable;
  bool trackir;
  bool mouse_init;
  bool mouse_active;
  bool joystick_adv_init;

  padding( 0x2C );

  void* keys;
  padding( 0x6C );

  bool camera_intercepting_mouse;
  bool camera_in_third_person;
  bool camera_moving_with_mouse;
  vector3d camera_offset;
  bool camera_distance_move;
  int camera_old_x;
  int camera_old_y;
  int camera_x;
  int camera_y;
  bool camera_is_orthographic;
  vector3d previous_view_angles;
  vector3d previous_view_angles_tilt;
  float last_forward_move;
  int clear_input_state;
  c_usercmd* commands;
  verified_cmd_t* verified_commands;

  c_usercmd* get_user_cmd( int seq )
  {
    return &commands [ seq % 150 ];
  }

  c_usercmd* get_user_cmd( int slot, int seq )
  {
    return g_memory->getvfunc< c_usercmd*( __thiscall* )( void*, int, int ) >( this, 8 )( this, slot, seq );
  }

  verified_cmd_t* get_verified_user_cmd( int sequence_number )
  {
    return &verified_commands [ sequence_number % 150 ];
  }

  bool write_user_cmd_delta_to_buffer( int slot, void* buf, int from, int to, bool is_new_cmd )
  {
    using fn = bool( __thiscall* )( c_input*, int, void*, int, int, bool );
    return g_memory->getvfunc< fn >( this, 5 )( this, slot, buf, from, to, is_new_cmd );
  }
};