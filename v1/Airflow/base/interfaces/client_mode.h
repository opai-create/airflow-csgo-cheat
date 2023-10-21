#pragma once
#include "../tools/memory/memory.h"

class c_client_mode
{
public:
	virtual ~c_client_mode()
	{
	}
	virtual int client_mode_cs_normal(void*) = 0;
	virtual void init() = 0;
	virtual void init_view_port() = 0;
	virtual void shut_down() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void layout() = 0;
	virtual c_panel* get_view_port() = 0;
	virtual void* get_viewport_animation_controller() = 0;
	virtual void process_input(bool bActive) = 0;
	virtual bool should_draw_detail_objects() = 0;
	virtual bool should_draw_entity(c_csplayer* e) = 0;
	virtual bool should_draw_local_player(c_csplayer* pPlayer) = 0;
	virtual bool should_draw_particles() = 0;
	virtual bool should_draw_fog(void) = 0;
	virtual void override_view(c_view_setup* pSetup) = 0;
	virtual int key_input(int down, int keynum, const char* pszCurrentBinding) = 0;
	virtual void start_message_mode(int iMessageModeType) = 0;
	virtual c_panel* get_message_panel() = 0;
	virtual void override_mouse_input(float* x, float* y) = 0;
	virtual bool create_move(float flInputSampleTime, void* usercmd) = 0;
	virtual void level_init(const char* newmap) = 0;
	virtual void level_shutdown(void) = 0;
};