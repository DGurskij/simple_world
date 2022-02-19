#pragma once

#include "../dll_main/simple_world.h"
#include "Collections/ObjectCollection.h"
#include "Collections/StaticObjectCollection.h"

typedef struct SThreadHelper {
	SW_World* world;
	unsigned thread_index;
} SW_ThreadHelper;

struct SWorld
{
	/* CONFIG PART */

	/*
		World thread type
	*/
	unsigned type;

	/*
		time for iteration in milliseconds
	*/
	unsigned long req_iter_time_ms;

	/* STATE PART */

	/*
		0 - created
		1 - launched
		2 - stoped
	*/
	unsigned state;

	unsigned is_exist;
	
	/* JOB PART */

	unsigned count_threads;

	void* main_thread;
	void** helpers_threads;
	SW_ThreadHelper** helpers_threads_p;

	void* global_sync;
	void* internal_sync;
	void** helpers_sync;

	/* INFO PART */

	/*
		potential frames per seconds, dynamic value for each iteration
	*/
	long long fps;

	/* OBJECTS PART */
	
	// base collections, objects inside be process though full cycle
	SW_ObjectCollection** main_collections;

	/*
		optimization collection, contains objects which do not depend on others objects
		objects processed by specified thread
	*/
	SW_ObjectCollection** independ_collections;

	/*
		optimization collection, contains objects which has only const updates
		object also processed by specified thread, but cycle consists of one action - update without reset and interact
	*/
	SW_ObjectCollection** const_collections;

	// contain disabled objects
	SW_ObjectCollection* disabled_objects;

	// contain disabled objects which must be restored
	SW_ObjectCollection* restore_objects;
};
