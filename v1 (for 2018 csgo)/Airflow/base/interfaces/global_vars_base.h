#pragma once

class c_global_vars_base
{
public:
  float real_time{ };
  int frame_count{ };
  float absolute_frametime{ };
  float absolute_framestarttimestddev{ };
  float cur_time{ };
  float frame_time{ };
  int max_clients{ };
  int tick_count{ };
  float interval_per_tick{ };
  float interpolation_amount{ };
  int sim_ticks_this_frame{ };
  int network_protocol{ };
  void* save_data{ };
  bool client{ };
  bool remote_client{ };
  int time_stamp_networking_base{ };
  int time_stamp_randomize_window{ };
};