#pragma once

#ifdef  SIMPLEWORLD_EXPORTS
#define SIMPLE_WORLD_API __declspec(dllexport)
#else
#define SIMPLE_WORLD_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
	Use for init world without another threads, fields: req_iter_time_ms, count_threads will be ignored
*/
#define SW_WTYPE_MAINTHREADED 0x0u

/*
	Read comment for field SW_WORLD_CONFIG.count_threads
*/
#define SW_WTYPE_MULTITHREADED 0x1u

	typedef struct SWorld SW_World;
	typedef struct SWObject SW_Object;
	typedef struct SWUpdOperation SW_UpdOperation;
	typedef struct SWInteraction SW_Interaction;

	/*
		Return pointer on World structure or 0 if unsuccess.
		Create world with defined threads for process interaction.

		@param type - Use macro SW_WTYPE_... for this, default SW_WTYPE_MAINTHREADED

		@param req_iter_time_ms - Required time for one world iteration
			calculated time include: dynamic_cfg_time + iteration_time + update_time + [user_synch_time]
			world be sleep if calculated time less than req_iter_time_ms
		
		@param count_helpers_threads - For count_thread = 0 main engine cycle: read commands, engine states -> interactive -> update objects states.
			For count_thread > 0 main engine cycle: read commands, engine states -> interactive -> await another threads -> update objects states
			threads cycle: await main reading cmds, states -> interactive -> update objects states
	*/
	SIMPLE_WORLD_API SW_World* swCreateWorld(unsigned type, unsigned int req_iter_time_ms, unsigned count_helpers_threads);

/*
	count_static_elements - Used for static allocate memory for arrays.
	Array has better performance for iterate through many objects.
	Static arrays work for SW_OBJECT_BASE|INDEPEND|CONST types.
	Alocate count for each collections and threads
	For SW_WTYPE_MAINTHREADED real count = 2 * count_static_elements
	For SW_WTYPE_MULTITHREADED real count = (count_helpers_threads > 0 ? 3 : 2) * count_helpers_threads * count_static_elements
	!!!CAN'T BE REALLOC LATER.
*/

	/*
		Bind world global, after bind can use API with postfix 'B' without WORLD parameter
		!!! API not check if world not bind
		In debug mode print error
	*/
	SIMPLE_WORLD_API void swBindWorld(SW_World* world);

	/*
		Launch world engine, can be used for resume engine after pause
	*/
	SIMPLE_WORLD_API void swLaunchWorld(SW_World* world);
	SIMPLE_WORLD_API void swLaunchWorldB();

	/*
		Stop world engine, can be used for synchronized stop (pause)
	*/
	SIMPLE_WORLD_API void swStopWorld(SW_World* world);
	SIMPLE_WORLD_API void swStopWorldB();

	SIMPLE_WORLD_API void swDestroyWorld(SW_World* world);

	SIMPLE_WORLD_API unsigned swGetWorldFPS(SW_World* world);
	SIMPLE_WORLD_API unsigned swGetWorldFPSB();

	/*
		Only for SW_WTYPE_MAINTHREADED, otherwise ignored
		Return calculation time in milliseconds
	*/
	SIMPLE_WORLD_API unsigned swSyncMoveWorld(SW_World* world);
	SIMPLE_WORLD_API unsigned swSyncMoveWorldB();

/*
	Used for complex calculation with:
	interaction;
	update operations;
	constant update operations;
*/
#define SW_OBJECT_BASE 0
/*
	Used for independ objects which interact with self or all interactions defined inside same thread
*/
#define SW_OBJECT_INDEPEND 1
/*
	Used for simple constant operations with independ objects
	only for const operations, otherwise operations will ignored

	!!!DOESN'T WORK IN - SW_WTYPE_MAINTHREADED
*/
#define SW_OBJECT_CONST 2

	/*
		Create wrapper for object, return 0 if unsuccess
		data is pointer on csutom sctructure be passed into interaction functions

		independ will updates without global synchronize (increase performance for independ objects)
		thread_owner - number of thread which process updates, 0 - main, 1, 2, ... count_threads
		independ doesn't work for SW_WTYPE_MAINTHREADED or count_threads = 0
	*/
	SIMPLE_WORLD_API SW_Object* swCreateObject(SW_World* world, void* data, unsigned type, unsigned thread_owner);
	SIMPLE_WORLD_API SW_Object* swCreateObjectB(void* data, unsigned type, unsigned thread_owner);

	/*
		Destroy object wrapper
	*/
	SIMPLE_WORLD_API void swDestroyObject(SW_Object* object);

#define SW_MATH_ADD 0x0u
#define SW_MATH_SUB 0x1u
#define SW_MATH_MUL 0x2u
#define SW_MATH_DIV 0x3u

	/*
		Bind field with 'accelerate' which would apply to object each iteration, for operations use macro SW_MATH_...
		For SW_MATH_ADD: target_field += accelerate_field;
		Math operations process after all iterations
		accumulator reset to reset_value on cycle start
	*/
	SIMPLE_WORLD_API SW_UpdOperation* swAddUpdOpToObject(SW_World* world, SW_Object* object, float* target_field, float reset_value, unsigned math_operation);
	SIMPLE_WORLD_API SW_UpdOperation* swAddUpdOpToObjectB(SW_Object* object, float* target_field, float reset_value, unsigned math_operation);

	/*
		Work like  swAddUpdOpToObject but instead of accumulate data just update field by const value
	*/
	SIMPLE_WORLD_API SW_UpdOperation* swAddConstUpdOpToObject(SW_Object* object, float* target_field, float const_value, unsigned math_operation);

	/*
		Work like  swAddConstUpdOpToObject but instead of const bind value for update with external ptr
	*/
	SIMPLE_WORLD_API SW_UpdOperation* swAddExternalUpdOpToObject(SW_Object* object, float* target_field, float* source, unsigned math_operation);

	/*
		Add handler which would be called after complete all updates for object
	*/
	SIMPLE_WORLD_API void swAddUpdHandler(SW_Object* object, void(*handler)(SW_World* world, SW_Object* object, void* data));

	typedef void (*interactFunc)(
		SW_World* world,
		SW_Object* object1,
		SW_Object* object2,
		float* accumulators1,
		float* accumulators2,
		void* data1,
		void* data2
	);

	/*
		Add interaction between to objects
	*/
	SIMPLE_WORLD_API void swAddInteraction(SW_World* world, SW_Object* object1, SW_Object* object2, unsigned thread_owner, interactFunc interactF);
	SIMPLE_WORLD_API void swAddInteractionB(SW_Object* object1, SW_Object* object2, unsigned thread_owner, interactFunc interactF);

	/*
		Disable object, do it permanent if engine not active, otherwise work on next iteration
	*/
	SIMPLE_WORLD_API void swDisableObject(SW_World* world, SW_Object* object);
	SIMPLE_WORLD_API void swDisableObjectB(SW_Object* object);

	/*
		Enable object, do it permanent if engine not active, otherwise work on next iteration
	*/
	SIMPLE_WORLD_API void swEnableObject(SW_World* world, SW_Object* object);
	SIMPLE_WORLD_API void swEnableObjectB(SW_Object* object);

	// TODO: add disable custom operations
	/*
		Disable operation, do it permanent for SW_WTYPE_MAINTHREADED or if world not launched, otherwise work on next iteration
	*/
	//SIMPLE_WORLD_API void swDisableUpdOp(SW_World* world, SW_Object* object, SW_UpdOperation* operation);
	//SIMPLE_WORLD_API void swDisableUpdOpB(SW_Object* object, SW_UpdOperation* operation);

	// TODO: add init value for custom field from custom source or const
	/*
		Bind field with source which value be asigned on field each time when cycle start
	*/
	//SIMPLE_WORLD_API void swSetInitCycleValueToObject(SW_Object* object, float* target_field, float* source);

	/*
		Bind field with const value be asigned on field each time when cycle start
	*/
	//SIMPLE_WORLD_API void swSetInitCycleConstValueToObject(SW_Object* object, float* target_field, float const_value);

#ifdef __cplusplus
}
#endif
