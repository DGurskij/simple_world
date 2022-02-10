#include "pch.h"
#include "simple_world.h"
#include "../structures/World.h"
#include "../threads/Threads.h"
#include "../structures/Object/UpdOperation.h"

SW_World* bind_world;

/********************
	DESTROY HELPERS
*********************/

void destroyMainCollections(SW_World* world, char end_index);
void destroyIndependentCollections(SW_World* world, char end_index);
void destroyHelpersPointers(SW_World* world);
void destroyHelperThreads(SW_World* world, char end_index);

SW_World* swCreateWorld(SW_WorldConfig world_config)
{
#ifdef _DEBUG
	printf("SIMPLE_WORLD::Start creating world\n");
#endif
	SW_World* world = (SW_World*)malloc(sizeof(SW_World));

	if (!world)
	{
#ifdef _DEBUG
		printf("SIMPLE_WORLD::ERROR::insufficient memory for STRUCT WORLD\n");
#endif
		return 0;
	}

	char i = 0;
	char count_threads = world_config.type == SW_WTYPE_MULTITHREADED ? world_config.count_threads : 0;

	world->is_exist = 1;

	world->req_iter_time_ms = world_config.req_iter_time_ms;
	world->fps = 1000 / world_config.req_iter_time_ms;

	world->count_threads = count_threads;

	world->main_collections = malloc(sizeof(SW_ObjectCollection*) * (count_threads + 1));

	if (!world->main_collections)
	{
#ifdef _DEBUG
		printf("SIMPLE_WORLD::ERROR::main collection not initialized\n");
#endif
		return 0;
	}

	for (i = 0; i < count_threads + 1; i++)
	{
		world->main_collections[i] = objectCollectionCreate();

		if (!world->main_collections[i])
		{
#ifdef _DEBUG
			printf("SIMPLE_WORLD::ERROR::main collection %d not initialized\n", i);
#endif

			destroyMainCollections(world, i);
			free(world);
			return 0;
		}
	}

	world->independ_collections = 0;

	if (world_config.type == SW_WTYPE_MULTITHREADED)
	{
		if (count_threads > 0)
		{
			// init independ collections
			if (world_config.enable_independ_collections)
			{
				world->independ_collections = malloc(sizeof(SW_ObjectCollection*) * (count_threads + 1));

				if (!world->independ_collections)
				{
#ifdef _DEBUG
					printf("SIMPLE_WORLD::ERROR::independ collections not initialized\n");
#endif
					destroyMainCollections(world, count_threads + 2);
					return 0;
				}

				for (i = 0; i < count_threads + 1; i++)
				{
					world->independ_collections[i] = objectCollectionCreate();

					if (!world->independ_collections[i])
					{
#ifdef _DEBUG
						printf("SIMPLE_WORLD::ERROR::independ collection %d not initialized\n", i);
#endif

						destroyMainCollections(world, count_threads + 2);
						destroyIndependentCollections(world, i);
						free(world);
						return 0;
					}
				}
			}

			world->internal_sync = CreateSemaphore(NULL, 0, count_threads, NULL);

			world->helpers_threads = malloc(sizeof(void*) * count_threads);
			world->helpers_sync = malloc(sizeof(void*) * count_threads);

			world->helpers_threads_p = (SW_ThreadHelper**)malloc(sizeof(SW_ThreadHelper*) * count_threads);

			if (!world->helpers_sync || !world->helpers_threads || !world->helpers_threads_p)
			{
#ifdef _DEBUG
				printf("SIMPLE_WORLD::ERROR::helper threads not initialized\n");
#endif

				destroyHelpersPointers(world);
				destroyMainCollections(world, count_threads + 2);
				destroyIndependentCollections(world, count_threads + 2);
				CloseHandle(world->internal_sync);
				free(world);
				return 0;
			}

			for (i = 0; i < count_threads; i++)
			{
				world->helpers_threads_p[i] = malloc(sizeof(SW_ThreadHelper));

				if (!world->helpers_threads_p[i])
				{
#ifdef _DEBUG
					printf("SIMPLE_WORLD::ERROR::helper thread %d not initialized\n", i);
#endif

					destroyHelperThreads(world, i);
					destroyHelpersPointers(world);
					destroyMainCollections(world, count_threads + 2);
					destroyIndependentCollections(world, count_threads + 2);
					CloseHandle(world->internal_sync);
					free(world);
					return 0;
				}

				world->helpers_threads_p[i]->thread_index = i;
				world->helpers_threads_p[i]->world = world;

				world->helpers_sync[i] = CreateSemaphore(NULL, 0, 1, NULL);

				world->helpers_threads[i] = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)helperThread, world->helpers_threads_p[i], 0, NULL);
			}

			world->main_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mainThread, world, 0, NULL);
		}
		else
		{
			world->main_thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)mainThreadSimple, world, 0, NULL);
		}

		world->global_sync = CreateSemaphore(NULL, 0, 1, NULL);
	}

	world->state = 0;

#ifdef _DEBUG
	printf("SIMPLE_WORLD::World initialized successfull\n");
#endif
	return world;
}



/********************
	UTILS PART
*********************/



void swBindWorld(SW_World* world)
{
	bind_world = world;
}

unsigned swGetWorldFPS(SW_World* world)
{
	return (unsigned)world->fps;
}

unsigned swGetWorldFPSB()
{
	return (unsigned)bind_world->fps;
}



/********************
	CONTROL PART
*********************/



void swLaunchWorld(SW_World* world)
{
	if (world->state != 1)
	{
		ReleaseSemaphore(world->global_sync, 1, NULL);
		world->state = 1;
	}
}

void swLaunchWorldB()
{
	if (bind_world->state != 1)
	{
		ReleaseSemaphore(bind_world->global_sync, 1, NULL);
		bind_world->state = 1;
	}
}

void swStopWorld(SW_World* world)
{
	if (world->state == 1)
	{
		WaitForSingleObject(world->global_sync, 0);
		world->state = 2;
	}
}

void swStopWorldB()
{
	if (bind_world->state == 1)
	{
		WaitForSingleObject(bind_world->global_sync, 0);
		bind_world->state = 2;
	}
}

unsigned swSyncMoveWorld(SW_World* world)
{
	struct timeb start;
	struct timeb end;

	ftime(&start);

	SW_ObjectCollection* collection = world->main_collections[0];

	SW_Object* object = collection->first;

	// set initial cycle values&user_changes
	while (object)
	{
		// clear all accumulators
		SW_UpdOperation* update = object->upd_operations->first;

		while (update)
		{
			update->accumalator[0] = update->reset_value;
			update = update->next;
		}

		object = object->next;
	}

	object = collection->first;

	// interaction cycle
	while (object)
	{
		// TODO: add interaction
		object = object->next;
	}

	object = collection->first;

	while (object)
	{
		SW_UpdOperation* update = object->upd_operations->first;

		// updates by accumulators
		while (update)
		{
			update->operationF(update->target_field, update->accumalator[0]);
			update = update->next;
		}

		update = object->upd_const_operations->first;

		// updates by constant
		while (update)
		{
			update->operationF(update->target_field, update->reset_value);
			update = update->next;
		}

		object = object->next;
	}

	ftime(&end);
	return (unsigned)(1000 * (end.time - start.time) + (end.millitm - start.millitm));
}

unsigned swSyncMoveWorldB()
{
	return swSyncMoveWorld(bind_world);
}



/********************
	OBJECT PART
*********************/



SW_Object* swCreateObject(SW_World* world, void* data, char independ, char thread_owner)
{
	SW_Object* object = objectCreate(data, independ);
	
	if (object)
	{
		if (independ && world->type == SW_WTYPE_MULTITHREADED)
		{
			objectCollectionPush(world->independ_collections[thread_owner], object);
		}
		else
		{
			objectCollectionPush(world->main_collections[thread_owner], object);
		}
	}

	return object;
}

SW_Object* swCreateObjectB(void* data, char independ, char thread_owner)
{
	return swCreateObject(bind_world, data, independ, thread_owner);
}

void swDestroyObject(SW_Object* object)
{
	objectDestroy(object);
}

SW_UpdOperation* swAddUpdOpToObject(SW_World* world, SW_Object* object, float* target_field, float reset_value, unsigned math_operation)
{
	char count = 1;

	if (object->independ != 0)
	{
		count = world->count_threads + 1;
	}

	float* accumulator = malloc(sizeof(float) * count);

	SW_UpdOperation* upd_operation = updOperationCreate(target_field, accumulator, reset_value, math_operation);

	if (upd_operation)
	{
		updCollectionPush(object->upd_operations, upd_operation);
	}

	return upd_operation;
}

SW_UpdOperation* swAddUpdOpToObjectB(SW_Object* object, float* target_field, float reset_value, unsigned math_operation)
{
	return swAddUpdOpToObject(bind_world, object, target_field, reset_value, math_operation);
}

SW_UpdOperation* swAddConstUpdOpToObject(SW_Object* object, float* target_field, float const_value, unsigned math_operation)
{
	SW_UpdOperation* upd_operation = updConstOperationCreate(target_field, const_value, math_operation);

	if (upd_operation)
	{
		updCollectionPush(object->upd_const_operations, upd_operation);
	}

	return upd_operation;
}

void swDisableUpdOp(SW_Object* object, SW_UpdOperation* operation)
{
	if (operation->accumalator)
	{
		updCollectionRemoveItem(object->upd_operations, operation, 0);
		updCollectionPush(object->upd_operations_disabled, operation);
	}
	else
	{
		updCollectionRemoveItem(object->upd_const_operations, operation, 0);
		updCollectionPush(object->upd_const_operations_disabled, operation);
	}
}

void swSetInitCycleValueToObject(SW_Object* object, float* target_field, float* source)
{

}



/********************
	DESTROY PART
*********************/



void destroyMainCollections(SW_World* world, char end_index)
{
	if (end_index > 0)
	{
		end_index--;

		do
		{
			objectCollectionDestroy(world->main_collections[end_index]);
		} while (end_index != 0);
	}

	free(world->main_collections);
}

void destroyIndependentCollections(SW_World* world, char end_index)
{
	if (world->independ_collections)
	{
		if (end_index > 0)
		{
			end_index--;

			do
			{
				objectCollectionDestroy(world->independ_collections[end_index]);
			} while (end_index != 0);
		}

		free(world->independ_collections);
	}
}

void destroyHelpersPointers(SW_World* world)
{
	// free if some allocated
	if (world->helpers_sync)
	{
		free(world->helpers_sync);
	}

	if (world->helpers_threads)
	{
		free(world->helpers_threads);
	}

	if (world->helpers_threads_p)
	{
		free(world->helpers_threads_p);
	}
}

void destroyHelperThreads(SW_World* world, char end_index)
{
	if (end_index > 0)
	{
		end_index--;

		do
		{
			TerminateThread(world->helpers_threads[end_index], 0);

			CloseHandle(world->helpers_threads[end_index]);
			CloseHandle(world->helpers_sync[end_index]);

			free(world->helpers_threads_p[end_index]);
		} while (end_index != 0);
	}
}

void swDestroyWorld(SW_World* world)
{
#ifdef _DEBUG
	printf("SIMPLE_WORLD::Start destroy\n");
#endif

	world->is_exist = 0;

	if (world->type == SW_WTYPE_MULTITHREADED)
	{
		char i;

		// relesae main thread if stopped or not launched
		ReleaseSemaphore(world->global_sync, 1, NULL);
		// wait main thread
		WaitForSingleObject(world->main_thread, INFINITE);

		if (world->count_threads != 0)
		{
			for (i = 0; i < world->count_threads; i++)
			{
				printf("SIMPLE_WORLD::Close handle %d\n", i);
				CloseHandle(world->helpers_threads[i]);
				CloseHandle(world->helpers_sync[i]);
			}

			free(world->helpers_sync);
			free(world->helpers_threads);
			free(world->helpers_threads_p);

			CloseHandle(world->internal_sync);
		}

		CloseHandle(world->global_sync);
		CloseHandle(world->main_thread);

		destroyIndependentCollections(world, world->count_threads + 2);
	}

	destroyMainCollections(world, world->count_threads + 2);

	free(world);

#ifdef _DEBUG
	printf("SIMPLE_WORLD::World destroyed successfull\n");
#endif
}