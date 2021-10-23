#include "pch.h"
#include "simple_world.h"
#include "World.h"

float distance2D(float x1, float y1, float x2, float y2)
{
	return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2), 0.5);
}

float distance3D(float x1, float y1, float z1, float x2, float y2, float z2)
{
	return powf(powf(x1 - x2, 2) + powf(y1 - y2, 2) + powf(z1 - z2, 2), 0.5);
}

void* createWorld(unsigned count_thread)
{
#ifdef _DEBUG
	printf("Create world with %d threads", count_thread);
#endif
	return malloc(sizeof(World));
}
