#include "dll_main/pch.h"
#include "simple_world.h"
#include "World.h"

float swDistance2D(float x1, float y1, float x2, float y2)
{
	return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2), 0.5);
}

float swDistance3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2) + powf(z1 - z2, 2), 0.5);
}

PtrWorld swCreateWorld(unsigned count_threads)
{
#ifdef _DEBUG
	printf("Create world with %d threads", count_threads);
#endif
	SW_World* world = (SW_World*)malloc(sizeof(SW_World));

	world->count_threads = count_threads;

	if (count_threads > 0)
	{
		world->global_sync = CreateSemaphore(NULL, 0, count_threads, NULL);
	}

	return (PtrWorld)world;
}

void swLaunchWorld(PtrWorld world)
{

}

void swStopWorld(PtrWorld world)
{

}

void swDestroyWorld(PtrWorld world)
{
	free((SW_World*)world);
}

/*
void swSet(PtrWorld world)
{

}

void* swGet(PtrWorld world)
{

}
*/