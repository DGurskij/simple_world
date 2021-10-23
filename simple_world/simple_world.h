#pragma once

#ifdef  SIMPLE_WORLD_EXPORTS
#define SIMPLE_WORLD_API __declspec(dllexport)
#else
#define SIMPLE_WORLD_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif


	SIMPLE_WORLD_API float distance2D(float x1, float y1, float x2, float y2);
	SIMPLE_WORLD_API float distance3D(float x1, float y1, float z1, float x2, float y2, float z2);

	/*
		Create world with defined threads for process interaction.
		Count thread minimum 1.
	*/
	SIMPLE_WORLD_API void* createWorld(unsigned count_thread);

#ifdef __cplusplus
}
#endif
