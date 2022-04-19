#include "pch.h"
#include "simple_world.h"
#include "../structures/World.h"
#include "../threads/Threads.h"
#include "../structures/Object/UpdOperation.h"

SW_World* bind_world;

/********************
	DESTROY HELPERS
*********************/

void destroyHelpersPointers(SW_World* world);
void destroyHelperThreads(SW_World* world, unsigned end_index, unsigned threads_ended);

SW_World* swCreateWorld(unsigned type, unsigned int req_iter_time_ms, unsigned count_helpers_threads)
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

	unsigned i = 0;
	unsigned count_threads = type == SW_WTYPE_MULTITHREADED ? count_helpers_threads : 0;

	world->type = type;
	world->is_exist = 1;

	world->req_iter_time_ms = req_iter_time_ms;
	world->fps = 1000 / req_iter_time_ms;

	world->count_threads = count_threads;

	// START::Init maint collection
	world->main_collections = createObjectGroup(count_threads + 1);

	world->disabled_objects = objectCollectionCreate();
	world->restore_objects = objectCollectionCreate();

	if (!world->main_collections || !world->disabled_objects || !world->restore_objects)
	{
		// free memory if some allocated
		if (world->main_collections)
		{
			destroyObjectGroup(world->main_collections, count_threads + 1);
		}

		if (world->disabled_objects)
		{
			free(world->disabled_objects);
		}

		if (world->restore_objects)
		{
			free(world->restore_objects);
		}

#ifdef _DEBUG
		printf("SIMPLE_WORLD::ERROR::main collection not initialized\n");
#endif

		free(world);
		return 0;
	}
	// END::Init main collection

	// START::Init const collection
	world->const_collections = createObjectGroup(count_threads + 1);

	if (!world->const_collections)
	{
#ifdef _DEBUG
		printf("SIMPLE_WORLD::ERROR::const collection not initialized\n");
#endif
		destroyObjectGroup(world->main_collections, count_threads + 1);

		free(world->disabled_objects);
		free(world->restore_objects);
		free(world);
		return 0;
	}
	// END::Init const collection

	// START::Init interactions
	world->intreaction_group = createInteractionGroup(count_threads + 1, 50000);

	if (!world->intreaction_group)
	{
#ifdef _DEBUG
		printf("SIMPLE_WORLD::ERROR::interactions buffer not initialized\n");
#endif
		destroyObjectGroup(world->main_collections, count_threads + 1);

		free(world->disabled_objects);
		free(world->restore_objects);
		free(world);
		return 0;
	}
	// END::Init interactions

	world->independ_collections = 0;

	if (type == SW_WTYPE_MULTITHREADED)
	{
		if (count_threads > 0)
		{
			// START::Init independ collection
			world->independ_collections = createObjectGroup(count_threads + 1);

			if (!world->independ_collections)
			{
#ifdef _DEBUG
				printf("SIMPLE_WORLD::ERROR::independ collections not initialized\n");
#endif
				destroyObjectGroup(world->main_collections, count_threads + 1);
				destroyObjectGroup(world->const_collections, count_threads + 1);

				destroyInteractionGroup(world->intreaction_group, count_threads + 1);

				free(world->disabled_objects);
				free(world->restore_objects);
				free(world);
				return 0;
			}
			// END::Init independ collection

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
				destroyObjectGroup(world->main_collections, count_threads + 1);
				destroyObjectGroup(world->const_collections, count_threads + 1);
				destroyObjectGroup(world->independ_collections, count_threads + 1);

				destroyInteractionGroup(world->intreaction_group, count_threads + 1);

				free(world->disabled_objects);
				free(world->restore_objects);
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

					destroyHelperThreads(world, i, 0);
					destroyHelpersPointers(world);
					destroyObjectGroup(world->main_collections, count_threads + 1);
					destroyObjectGroup(world->const_collections, count_threads + 1);
					destroyObjectGroup(world->independ_collections, count_threads + 1);

					destroyInteractionGroup(world->intreaction_group, count_threads + 1);

					free(world->disabled_objects);
					free(world->restore_objects);
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
		WaitForSingleObject(world->global_sync, INFINITE);
		world->state = 2;
	}
}

void swStopWorldB()
{
	if (bind_world->state == 1)
	{
		WaitForSingleObject(bind_world->global_sync, INFINITE);
		bind_world->state = 2;
	}
}

unsigned swSyncMoveWorld(SW_World* world)
{
	struct timeb start;
	struct timeb end;

	ftime(&start);

	world->state = 1;

	SW_ObjectCollection* collection = world->restore_objects;
	SW_Object* object = collection->first;
	SW_UpdOperation* update;

	SW_ObjectCollection* restore_col = 0;

	SW_Interaction** interactions = world->intreaction_group->interactions[0];
	unsigned count_interactions = world->intreaction_group->counts_interactions[0];
	unsigned i = 0;

	// restore enabled objects
	while (object)
	{
		objectCollectionRemoveObject(world->restore_objects, object);

		if (object->type == SW_OBJECT_BASE)
		{
			restore_col = world->main_collections[0];
		}

		if (object->type == SW_OBJECT_CONST)
		{
			restore_col = world->const_collections[0];
		}

		objectCollectionPush(restore_col, object);

		object->disabled = 0;

		object = object->next;
	}

	collection = world->main_collections[0];
	object = collection->first;

	// set initial cycle values&user_changes
	while (object)
	{
		// clear all accumulators
		update = object->upd_operations->first;

		while (update)
		{
			update->accumalator[0] = update->reset_value;
			update = update->next;
		}

		object = object->next;
	}

	i = 0;

	for (; i < count_interactions; i++)
	{
		if (!(interactions[i]->object1->disabled || interactions[i]->object2->disabled))
		{
			interactions[i]->interactF(
				world,
				interactions[i]->object1,
				interactions[i]->object2,
				interactions[i]->accumulators1,
				interactions[i]->accumulators2,
				interactions[i]->object1->data,
				interactions[i]->object2->data
			);
		}
	}

	object = collection->first;

	// iterate through objects and updates them
	while (object)
	{
		update = object->upd_operations->first;

		// updates by accumulators
		while (update)
		{
			update->operationF(update->target_field, update->accumalator[0]);
			update = update->next;
		}

		// updates by external data
		update = object->upd_external_operations->first;

		while (update)
		{
			update->operationF(update->target_field, *update->accumalator);
			update = update->next;
		}

		update = object->upd_const_operations->first;

		// updates by constant
		while (update)
		{
			update->operationF(update->target_field, update->reset_value);
			update = update->next;
		}

		if (object->after_update_action)
		{
			object->after_update_action(world, object, object->data);
		}

		if (object->disabled)
		{
			SW_Object* next = object->next;

			objectCollectionRemoveObject(collection, object);
			objectCollectionPush(world->disabled_objects, object);

			object = next;
		}
		else
		{
			object = object->next;
		}
	}

	collection = world->const_collections[0];
	object = collection->first;

	// iterate through const collection
	while (object)
	{
		update = object->upd_const_operations->first;

		while (update)
		{
			update->operationF(update->target_field, update->reset_value);
			update = update->next;
		}

		if (object->after_update_action)
		{
			object->after_update_action(world, object, object->data);
		}

		if (object->disabled == 1)
		{
			SW_Object* next = object->next;

			objectCollectionRemoveObject(collection, object);
			objectCollectionPush(world->disabled_objects, object);

			object = next;
		}
		else
		{
			object = object->next;
		}
	}

	world->state = 0;

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



SW_Object* swCreateObject(SW_World* world, void* data, unsigned type, unsigned thread_owner)
{
	SW_Object* object = objectCreate(data, type, thread_owner);
	
	if (object && type != SW_OBJECT_EMPTY)
	{
		if (type == SW_OBJECT_CONST)
		{
			objectCollectionPush(world->const_collections[thread_owner], object);
		}
		else if (type == SW_OBJECT_INDEPEND && world->type == SW_WTYPE_MULTITHREADED && world->count_threads != 0)
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

SW_Object* swCreateObjectB(void* data, unsigned type, unsigned thread_owner)
{
	return swCreateObject(bind_world, data, type, thread_owner);
}

void swDestroyObject(SW_Object* object)
{
	objectDestroy(object);
}

SW_UpdOperation* swAddUpdOpToObject(SW_World* world, SW_Object* object, float* target_field, float reset_value, unsigned math_operation)
{
	unsigned count = 1;

	if (object->type == SW_OBJECT_BASE)
	{
		count = world->count_threads + 1;
	}

	float* accumulator = malloc(sizeof(float) * count);

	SW_UpdOperation* upd_operation = updOperationCreate(target_field, accumulator, reset_value, math_operation);

	if (upd_operation)
	{
		updCollectionPush(object->upd_operations, upd_operation);
		object->count_upd_ops++;
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

SW_UpdOperation* swAddExternalUpdOpToObject(SW_Object* object, float* target_field, float* source, unsigned math_operation)
{
	SW_UpdOperation* upd_operation = updExternalOperationCreate(target_field, source, math_operation);

	if (upd_operation)
	{
		updCollectionPush(object->upd_external_operations, upd_operation);
	}

	return upd_operation;
}

void swAddUpdHandler(SW_Object* object, void(*handler)(SW_World* world, SW_Object* object, void* data))
{
	object->after_update_action = handler;
}

void swAddInteraction(SW_World* world, SW_Object* object1, SW_Object* object2, unsigned thread_owner, interactFunc interaction)
{
	unsigned* counts_interactions = world->intreaction_group->counts_interactions;

	world->intreaction_group->interactions[thread_owner][counts_interactions[thread_owner]] = createInteraction(
		object1,
		object2,
		thread_owner,
		interaction
	);

	counts_interactions[thread_owner]++;
}

void swAddInteractionB(SW_Object* object1, SW_Object* object2, unsigned thread_owner, interactFunc interaction)
{
	swAddInteraction(bind_world, object1, object2, thread_owner, interaction);
}

void swDisableObject(SW_World* world, SW_Object* object)
{
	if (world->state != 1)
	{
		if (object->type != SW_OBJECT_EMPTY)
		{
			if (object->type == SW_OBJECT_CONST)
			{
				objectCollectionRemoveObject(world->const_collections[object->thread_owner], object);
			}
			else if (object->type == SW_OBJECT_INDEPEND && world->count_threads > 0)
			{
				objectCollectionRemoveObject(world->independ_collections[object->thread_owner], object);
			}
			else
			{
				objectCollectionRemoveObject(world->main_collections[object->thread_owner], object);
			}

			objectCollectionPush(world->disabled_objects, object);
			object->disabled = 2;
		}
	}
	else
	{
		object->disabled = 1;
	}
}

void swDisableObjectB(SW_Object* object)
{
	swDisableObject(bind_world, object);
}

void swEnableObject(SW_World* world, SW_Object* object)
{
	if (object->type != SW_OBJECT_EMPTY)
	{
		objectCollectionRemoveObject(world->disabled_objects, object);
		objectCollectionPush(world->restore_objects, object);
	}
}

void swEnableObjectB(SW_Object* object)
{
	swEnableObject(bind_world, object);
}



/********************
	DESTROY PART
*********************/



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

	if (world->internal_sync)
	{
		CloseHandle(world->internal_sync);
	}
}

void destroyHelperThreads(SW_World* world, unsigned end_index, unsigned threads_ended)
{
	if (end_index > 0)
	{
		do
		{
			end_index--;
			if (!threads_ended)
			{
				TerminateThread(world->helpers_threads[end_index], 0);
			}

			CloseHandle(world->helpers_threads[end_index]);
			CloseHandle(world->helpers_sync[end_index]);
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
		// wait if run now
		if (world->state == 1)
		{
			WaitForSingleObject(world->global_sync, INFINITE);
		}
		
		// relesae main thread if stopped or not launched
		ReleaseSemaphore(world->global_sync, 1, 0);

		// wait main thread
		WaitForSingleObject(world->main_thread, INFINITE);

		if (world->count_threads != 0)
		{
			destroyHelperThreads(world, world->count_threads, 1);
			destroyHelpersPointers(world);

			CloseHandle(world->internal_sync);

			destroyObjectGroup(world->independ_collections, world->count_threads + 1);
		}

		CloseHandle(world->global_sync);
		CloseHandle(world->main_thread);
	}

	destroyInteractionGroup(world->intreaction_group, world->count_threads + 1);

	destroyObjectGroup(world->main_collections, world->count_threads + 1);
	destroyObjectGroup(world->const_collections, world->count_threads + 1);


	objectCollectionDestroy(world->disabled_objects);
	objectCollectionDestroy(world->restore_objects);

	free(world);

#ifdef _DEBUG
	printf("SIMPLE_WORLD::World destroyed successfull\n");
#endif
}



/********************
	UTILS METHODS
*********************/



float swDistanceP1P2(float x1, float y1, float x2, float y2)
{
	return powf(powf(x1 - x2, 2.f) + powf(y1 - y2, 2.f), 0.5f);
}
float swDistanceP1P2WithOut(float x1, float y1, float x2, float y2, float* dx_out, float* dy_out)
{
	float dx = x1 - x2;
	float dy = y1 - y2;

	*dx_out = dx;
	*dy_out = dy;

	return powf(dx * dx + dy * dy, 0.5f);
}

float swDistanceDxDy(float dx, float dy)
{
	return powf(powf(dx, 2.f) + powf(dy, 2.f), 0.5f);
}