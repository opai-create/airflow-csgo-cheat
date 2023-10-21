#pragma once
#include "../tools/memory/memory.h"
#include "../tools/math.h"
#include "../tools/utils_macro.h"

#include "../../base/sdk/c_usercmd.h"

class c_client_entity;
class c_csplayer;

class c_movehelper
{
public:
	virtual void unk_virtual() = 0;
	virtual void set_host(c_client_entity* host) = 0;

private:
	virtual void pad00() = 0;
	virtual void pad01() = 0;

public:
	virtual void process_impacts() = 0;
};

class c_movedata
{
public:
	bool run_funcs : 1;          // 0x0000
	bool game_code_moved_player : 1; // 0x0001
	char pad_0002[2];         // 0x0002
	uint32_t handle;             // 0x0004
	bool N000000F1;              // 0x0008
	char pad_0009[3];         // 0x0009
	vector3d view_angles;
	vector3d old_view_angles;
	int32_t buttons;        // 0x0024
	int32_t old_buttons;    // 0x0028
	float forwardmove;      // 0x002C
	float sidemove;         // 0x0030
	float upmove;           // 0x0034
	float max_speed;        // 0x0038
	float client_max_speed; // 0x003C
	char pad_0040[12];   // 0x0040
	vector3d angles;        // 0x004C
	char pad_0058[84];   // 0x0058
	vector3d abs_origin;       // 0x00AC
};

class c_game_movement
{
public:
	virtual ~c_game_movement(void)
	{
	}

	virtual void process_movement(c_csplayer* player, c_movedata* move_data) = 0;
	virtual void reset(void) = 0;
	virtual void start_track_prediction_errors(c_csplayer* player) = 0;
	virtual void finish_track_prediction_errors(c_csplayer* player) = 0;
	virtual void diff_print(char const* fmt, ...) = 0;

	virtual vector3d const& get_player_mins(bool ducked) const = 0;
	virtual vector3d const& get_player_maxs(bool ducked) const = 0;
	virtual vector3d const& get_player_view_offset(bool ducked) const = 0;

	virtual bool is_moving_player_stuck(void) const = 0;
	virtual c_csplayer* get_moving_player(void) const = 0;
	virtual void ublock_pusher(c_csplayer* player, c_csplayer* pusher) = 0;

	virtual void setup_movement_bounds(c_movedata* move_data) = 0;
};

class c_prediction
{
public:
	uint32_t ground_handle;               // 0x0004
	bool in_prediction;                   // 0x0008
	bool old_in_prediction;               // 0x0009
	char pad_000A[2];                    // 0x000A
	int32_t prev_start_frame;             // 0x000C
	int32_t incoming_packet_number;       // 0x0010
	float time_stamp;                     // 0x0014
	bool is_first_time_predicted;         // 0x0018
	char pad_0019[3];                    // 0x0019
	int32_t commands_predicted;           // 0x001C
	int32_t server_commands_acknowledged; // 0x0020
	int32_t prev_ack_had_errors;          // 0x0024
	float ideal_pitch;                    // 0x0028
	uint32_t last_cmd_acknowledged;       // 0x002C
	bool trigger_latch_reset;             // 0x0030

	void reset_predict()
	{
		prev_ack_had_errors = true;
		commands_predicted = 0;
	}

	void update(int start_frame, bool valid_frame, int inc_ack, int out_cmd)
	{
		using fn = void(__thiscall*)(void*, int, bool, int, int);
		return g_memory->getvfunc< fn >(this, 3)(this, start_frame, valid_frame, inc_ack, out_cmd);
	}

	void postentity_packet_recieved()
	{
		using fn = void(__thiscall*)(void*);
		return g_memory->getvfunc< fn >(this, 5)(this);
	}

	void check_moving_ground(c_csplayer* player, double frame_time)
	{
		using fn = void(__thiscall*)(void*, c_csplayer*, double);
		return g_memory->getvfunc< fn >(this, 18)(this, player, frame_time);
	}

	void run_command(c_csplayer* player, c_usercmd* cmd, c_movehelper* move_helper)
	{
		using fn = void(__thiscall*)(void*, c_csplayer*, c_usercmd*, c_movehelper*);
		return g_memory->getvfunc< fn >(this, 19)(this, player, cmd, move_helper);
	}

	void setup_move(c_csplayer* player, c_usercmd* cmd, c_movehelper* move_helper, void* move_data)
	{
		using fn = void(__thiscall*)(void*, c_csplayer*, c_usercmd*, c_movehelper*, void*);
		return g_memory->getvfunc< fn >(this, 20)(this, player, cmd, move_helper, move_data);
	}

	void finish_move(c_csplayer* player, c_usercmd* cmd, void* move_data)
	{
		using fn = void(__thiscall*)(void*, c_csplayer*, c_usercmd*, void*);
		return g_memory->getvfunc< fn >(this, 21)(this, player, cmd, move_data);
	}

	void set_local_view_angles(vector3d& angles)
	{
		using fn = void(__thiscall*)(void*, vector3d&);
		return g_memory->getvfunc< fn >(this, 13)(this, angles);
	}
};