#pragma once

#include "../dll_main/simple_world.h"
#include "Collections/ObjectCollection.h"

typedef struct SThreadHelper {
	SW_World* world;
	char thread_index;
} SW_ThreadHelper;

struct SWorld
{
	/* CONFIG PART */

	/*
		World thread type
	*/
	char type;

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
	char state;

	char is_exist;
	
	/* JOB PART */

	char count_threads;

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
	
	SW_ObjectCollection** main_collections;
	SW_ObjectCollection** independ_collections;
};
