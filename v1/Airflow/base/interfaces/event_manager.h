#pragma once
#include "../tools/memory/memory.h"

class c_game_event
{
public:
	virtual ~c_game_event() {};
	virtual const char* get_name() const = 0;

	virtual bool is_reliable() const = 0;
	virtual bool is_local() const = 0;
	virtual bool is_empty(const char* key_name = NULL) const = 0;

	virtual bool get_bool(const char* key_name = NULL, bool default_value = false) const = 0;
	virtual int get_int(const char* key_name = NULL, int default_value = 0) const = 0;
	virtual uint64_t get_uint64(const char* key_name = NULL, uint64_t default_value = 0) const = 0;
	virtual float get_float(const char* key_name = NULL, float default_value = 0.0f) const = 0;
	virtual const char* get_string(const char* key_name = NULL, const char* default_value = "") const = 0;
	virtual const wchar_t* get_wstring(const char* key_name = NULL, const wchar_t* default_value = L"") const = 0;
	virtual const void* get_ptr(const char* key_name = NULL) const = 0;

	virtual void set_bool(const char* key_name, bool value) = 0;
	virtual void set_int(const char* key_name, int value) = 0;
	virtual void set_uint64(const char* key_name, uint64_t value) = 0;
	virtual void set_float(const char* key_name, float value) = 0;
	virtual void set_string(const char* key_name, const char* value) = 0;
	virtual void set_wstring(const char* key_name, const wchar_t* value) = 0;
	virtual void set_ptr(const char* key_name, const void* value) = 0;
};

class c_game_event_listener2
{
public:
	virtual ~c_game_event_listener2()
	{
	}
	virtual void fire_game_event(c_game_event* event) = 0;
	virtual int get_event_debug_id() = 0;
};

class c_game_event_manager2
{
public:
	virtual ~c_game_event_manager2() {};
	virtual int load_events_from_file(const char* file_name) = 0;
	virtual void reset() = 0;
	virtual bool add_listener(c_game_event_listener2* listener, const char* name, bool server_side) = 0;
	virtual bool find_listener(c_game_event_listener2* listener, const char* name) = 0;
	virtual void remove_listener(c_game_event_listener2* listener) = 0;
	virtual bool add_listener_global(c_game_event_listener2* listener, bool server_side) = 0;
	virtual c_game_event* create_event(const char* name, bool force = false, int* cookie = NULL) = 0;
	virtual bool fire_event(c_game_event* event, bool dont_broadcast = false) = 0;
	virtual bool fire_event_client_side(c_game_event* event) = 0;
};