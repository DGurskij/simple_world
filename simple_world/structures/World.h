#pragma once

typedef struct SWorld
{
	char count_threads;
	float global_speed;

	HANDLE global_sync;
	HANDLE internal_sync;
	
} SW_World;