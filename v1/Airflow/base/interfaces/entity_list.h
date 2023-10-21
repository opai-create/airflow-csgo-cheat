#pragma once
#include "../tools/memory/memory.h"
#include "../sdk/c_utlvector.h"

class c_baseentity;

class c_entity_listener
{
public:
	virtual void on_entity_created(c_baseentity* entity) {};
	virtual void on_entity_deleted(c_baseentity* entity) {};
};

class c_entity_list
{
private:
	c_utlvector< c_entity_listener* > listeners = {};

public:
	virtual void* get_client_networkable(int idx) = 0;

	virtual void* vtablepad0x1() = 0;
	virtual void* vtablepad0x2() = 0;

	virtual void* get_entity(int idx) = 0;
	virtual void* get_entity_handle(uint32_t handle) = 0;

	virtual int number_of_entities(bool include_non_networkable) = 0;

	virtual int get_highest_ent_index() = 0;

	virtual void set_max_entities(int max_entities) = 0;
	virtual int get_max_entities() = 0;

	void add_listener_entity(c_entity_listener* listener)
	{
		this->listeners.add_to_tail(listener);
	}

	void remove_listener_entity(c_entity_listener* listener)
	{
		this->listeners.find_and_remove(listener);
	}
};