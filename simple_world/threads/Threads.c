#include "Threads.h"
#include <Windows.h>

#ifdef _DEBUG
#include <stdio.h>
#endif // _DEBUG

void mainThreadSimple(void* _world)
{
#ifdef _DEBUG
	printf("SIMPLE_WORLD::Main thread start\n");
#endif

	SW_World* world = (SW_World*)_world;

	struct timeb start;
	struct timeb end;

	long long sleep_time_ms = 0;
	long long dtime;

	// wait first launch
	WaitForSingleObject(world->global_sync, INFINITE);

	while (world->is_exist == 1)
	{
		if (sleep_time_ms > 0)
		{
			Sleep(sleep_time_ms);

			// check if has some changes
			if (world->is_exist == 0)
			{
				break;
			}

			if (world->state != 1)
			{
				WaitForSingleObject(world->global_sync, INFINITE);
			}
		}

		ftime(&start);
		// TODO: add config poll logic

		printf("MAIN:DO %d\n", world->count_threads);
		// TODO: add interction logic

		// TODO: add update logic

		if (world->state != 1)
		{
			WaitForSingleObject(world->global_sync, INFINITE);
		}

		ftime(&end);

		dtime = 1000 * (end.time - start.time) + (end.millitm - start.millitm);
		sleep_time_ms = world->req_iter_time_ms - dtime;
		world->fps = 1000 / dtime;
	}

#ifdef _DEBUG
	printf("SIMPLE_WORLD::Main thread end\n");
#endif
}

void mainThread(void* _world)
{
#ifdef _DEBUG
	printf("SIMPLE_WORLD::Main thread start\n");
#endif

	SW_World* world = (SW_World*)_world;

	struct timeb start;
	struct timeb end;

	long long sleep_time_ms = 0;
	long long dtime;

	unsigned thread_number = 0;

	// wait first launch
	WaitForSingleObject(world->global_sync, INFINITE);

	while (world->is_exist == 1)
	{
		if (sleep_time_ms > 0)
		{
			Sleep(sleep_time_ms);

			// check if has some changes
			if (world->is_exist == 0)
			{
				break;
			}

			if (world->state != 1)
			{
				WaitForSingleObject(world->global_sync, INFINITE);
			}
		}

		ftime(&start);

		// TODO: add config poll logic

		ReleaseSemaphore(world->internal_sync, world->count_threads, NULL);

		printf("MAIN:DO\n");
		// TODO: add interction logic

		WaitForMultipleObjects(world->count_threads, world->helpers_sync, 1, INFINITE);

		// TODO: add update logic

		// if on end cycle game is stoped or destroyed then not release helpers and synch for destroy/launch
		if (world->state != 1)
		{
			WaitForSingleObject(world->global_sync, INFINITE);
		}

		ftime(&end);

		dtime = 1000 * (end.time - start.time) + (end.millitm - start.millitm);
		sleep_time_ms = world->req_iter_time_ms - dtime;
		world->fps = 1000 / dtime;
	}

	// unlock helpers after leave cycle
	ReleaseSemaphore(world->internal_sync, world->count_threads, NULL);

#ifdef _DEBUG
	printf("SIMPLE_WORLD::Main thread end\n");
#endif
}

void helperThread(void* _sw_thread_helper)
{
	SW_ThreadHelper* helper = (SW_ThreadHelper*)_sw_thread_helper;

	SW_World* world = helper->world;
	unsigned thread_index = helper->thread_index;

	// number for access to collections
	unsigned thread_number = helper->thread_index + 1;

#ifdef _DEBUG
	printf("SIMPLE_WORLD::Helper thread start: %d\n", thread_index);
#endif
	// wait first launch
	WaitForSingleObject(world->internal_sync, INFINITE);

	while (world->is_exist == 1)
	{
		printf("HELPER[%d]:DO\n", thread_index);

		ReleaseSemaphore(world->helpers_sync[thread_index], 1, NULL);

		printf("HELPER[%d]:WAIT INTERNAL\n", thread_index);
		WaitForSingleObject(world->internal_sync, INFINITE);
	}

	free(_sw_thread_helper);
#ifdef _DEBUG
	printf("SIMPLE_WORLD::Helper thread end: %d\n", thread_index);
#endif
}