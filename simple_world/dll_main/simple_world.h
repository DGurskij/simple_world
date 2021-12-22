#pragma once

#ifdef  SIMPLEWORLD_EXPORTS
#define SIMPLE_WORLD_API __declspec(dllexport)
#else
#define SIMPLE_WORLD_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

	typedef int PtrWorld;
	#define SW_GLOBAL_SPEED 0;

	SIMPLE_WORLD_API float swDistance2D(float x1, float y1, float x2, float y2);
	SIMPLE_WORLD_API float swDistance3D(float x1, float y1, float z1, float x2, float y2, float z2);

	/*
		Return pointer on World structure.
		Create world with defined threads for process interaction.
		For count_thread = 0 main engine cycle: read commands, engine states -> interactive -> update objects states.
		For count_thread > 0 main engine cycle: read commands, engine states -> interactive -> await another threads -> update objects states
		threads cycle: await main reading cmds, states -> interactive -> update objects states
	*/
	SIMPLE_WORLD_API PtrWorld swCreateWorld(unsigned count_thread);

	SIMPLE_WORLD_API void swLaunchWorld(PtrWorld world);
	SIMPLE_WORLD_API void swStopWorld(PtrWorld world);
	SIMPLE_WORLD_API void swDestroyWorld(PtrWorld world);

	//SIMPLE_WORLD_API void  swSet(PtrWorld world);
	//SIMPLE_WORLD_API void* swGet(PtrWorld world);

#ifdef __cplusplus
}
#endif
