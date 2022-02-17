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

	/*
		for independ objects update will process inside single thread without synchronized
	*/
	unsigned type;

	/*
		Custom user data, do not free
	*/
	void* data;

	void (*after_update_action)(void* data, SW_Object* object);

	unsigned thread_owner;

	unsigned disabled;
};

SW_Object* objectCreate(void* data, unsigned type, unsigned thread_owner);

void objectDestroy(SW_Object* object);