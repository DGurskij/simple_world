#pragma once

#include "../../dll_main/simple_world.h"
#include "../Collections/UpdCollection.h"

struct SWObject
{
	SW_Object* prev;
	SW_Object* next;

	SW_UpdCollection* upd_operations;
	SW_UpdCollection* upd_operations_disabled;

	SW_UpdCollection* upd_const_operations;
	SW_UpdCollection* upd_const_operations_disabled;

	SW_UpdCollection* upd_external_operations;
	SW_UpdCollection* upd_external_operations_disabled;

	unsigned count_upd_ops;

	/*
		for independ objects update will process inside single thread without synchronized
	*/
	unsigned type;

	/*
		Custom user data, do not free
	*/
	void* data;

	void (*after_update_action)(SW_World* world, SW_Object* object, void* data);

	unsigned thread_owner;

	/*
		0 - not disabled
		1 - disabled asynch and must be place to disable collection
		2 - disabled full
	*/
	unsigned disabled;
};

SW_Object* objectCreate(void* data, unsigned type, unsigned thread_owner);

void objectDestroy(SW_Object* object);