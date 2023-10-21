#pragma once
#include "hooker.hpp"

struct draw_model_state_t;
struct model_render_info_t;
struct matrix3x4_t;

namespace hooks
{
	namespace vmt
	{
		void __fastcall cl_create_move_proxy(void* _this, int, int sequence_number, float input_sample_frametime, bool active);
		extern void __fastcall draw_model_execute(void* ecx, void* edx, void* ctx, const draw_model_state_t& state, const model_render_info_t& info, matrix3x4_t* bone_to_world);
		extern bool __fastcall write_usercmd_to_delta_buffer(void* ecx, void* edx, int slot, void* buf, int from, int to, bool isnewcommand);

		extern void init();
	}

	namespace detour
	{
		extern void __vectorcall read_packets(bool final_tick);
		extern void __vectorcall cl_move(float accumulated_extra_samples, bool final_tick);
		extern void init();
	}

	extern void init();
	extern void remove();
}