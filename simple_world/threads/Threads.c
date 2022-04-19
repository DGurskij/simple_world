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

	// wait first launch or destroy before launch
	WaitForSingleObject(world->global_sync, INFINITE);
	ReleaseSemaphore(world->global_sync, 1, 0); // sem = 1

	SW_ObjectCollection* collection;
	SW_Object* object;
	SW_UpdOperation* update;

	SW_ObjectCollection* restore_col = 0;

	while (world->is_exist == 1)
	{
		WaitForSingleObject(world->global_sync, INFINITE); // sem = 0, block swStopWorld caller for synchronize

		ftime(&start);

		collection = world->restore_objects;
		object = collection->first;

		// restore enabled objects
		while (object)
		{
			objectCollectionRemoveObject(world->restore_objects, object);

			if (object->type == SW_OBJECT_BASE)
			{
				restore_col = world->main_collections[0];
			}

			if (object->type == SW_OBJECT_INDEPEND)
			{
				restore_col = world->independ_collections[0];
			}

			if (object->type == SW_OBJECT_CONST)
			{
				restore_col = world->const_collections[0];
			}

			objectCollectionPush(restore_col, object);

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

		object = collection->first;

		world->state = 1;

		// interaction cycle
		while (object)
		{
			// TODO: add interaction
			object = object->next;
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

		ReleaseSemaphore(world->global_sync, 1, 0); // sem = 1, unlock swStopWorld caller if exist

		ftime(&end);

		dtime = 1000 * (end.time - start.time) + (end.millitm - start.millitm);
		sleep_time_ms = world->req_iter_time_ms - dtime;
		world->fps = 1000 / dtime;

		if (sleep_time_ms >= 0)
		{
			Sleep(sleep_time_ms);
		}

		// world did stoped, wait release caller
		if (world->state != 1)
		{
			WaitForSingleObject(world->global_sync, INFINITE);
			ReleaseSemaphore(world->global_sync, 1, 0); // sem = 1, for next iteration
		}
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

	// wait first launch or destroy before launch
	WaitForSingleObject(world->global_sync, INFINITE);
	ReleaseSemaphore(world->global_sync, 1, 0); // sem = 1

	SW_ObjectCollection* collection;
	SW_Object* object;
	SW_UpdOperation* update;

	SW_ObjectCollection* restore_col = 0;

	unsigned count_collections = world->count_threads + 1;

	unsigned i = 0;
	unsigned j = 0;

	float accumulate_value = 0.f;

	while (world->is_exist == 1)
	{
		WaitForSingleObject(world->global_sync, INFINITE); // sem = 0, block swStopWorld caller for synchronize

		// if destoryed while wait
		if (world->is_exist != 1)
		{
			break;
		}

		ftime(&start);

		/* START::RESTORE OBJECTS */
		collection = world->restore_objects;
		object = collection->first;

		// restore enabled objects
		while (object)
		{
			objectCollectionRemoveObject(world->restore_objects, object);

			if (object->type == SW_OBJECT_BASE)
			{
				restore_col = world->main_collections[object->thread_owner];
			}

			if (object->type == SW_OBJECT_INDEPEND)
			{
				restore_col = world->independ_collections[object->thread_owner];
			}

			if (object->type == SW_OBJECT_CONST)
			{
				restore_col = world->const_collections[object->thread_owner];
			}

			objectCollectionPush(restore_col, object);

			object = object->next;
		}
		/* END::RESET DEPENDENT COLLECTIONS */

		/* START::RESET DEPENDENT COLLECTIONS */
		for (i = 0; i < count_collections; i++)
		{
			collection = world->main_collections[i];
			object = collection->first;

			// set initial cycle values&user_changes
			while (object)
			{
				// clear all accumulators
				update = object->upd_operations->first;

				while (update)
				{
					for (j = 0; j < count_collections; j++)
					{
						update->accumalator[j] = update->reset_value;
					}
					update = update->next;
				}

				object = object->next;
			}
		}
		/* END::RESET DEPENDENT COLLECTIONS */

		// TODO: add config poll logic

		/****************************************************
			RELEASE HELPERS THREAD AND START MULTITHREAD WORK
		*****************************************************/

		ReleaseSemaphore(world->internal_sync, world->count_threads, NULL);

		/* START::RESET INDEPENDENT COLLECTION */
		collection = world->independ_collections[0];
		object = collection->first;

		while (object)
		{
			update = object->upd_operations->first;

			while (update)
			{
				update->accumalator[0] = update->reset_value;
				update = update->next;
			}

			object = object->next;
		}
		/* END::RESET INDEPENDENT COLLECTION */

		/* START::INTERACTION */
		collection = world->main_collections[0];
		object = collection->first;

		while (object)
		{
			// TODO: add interaction
			object = object->next;
		}
		/* END::INTERACTION */

		/* START::UPDATE CONST OBJECTS */
		collection = world->const_collections[0];
		object = collection->first;

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
		/* END::UPDATE CONST OBJECTS */

		/* START::UPDATE INDEPENDENT OBJECTS */
		collection = world->independ_collections[0];
		object = collection->first;

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
		/* END::UPDATE INDEPENDENT OBJECTS */

		// wait for helpers
		WaitForMultipleObjects(world->count_threads, world->helpers_sync, 1, INFINITE);

		/* START::UPDATE DEPENDENT COLLECTIONS */
		for (i = 0; i < count_collections; i++)
		{
			collection = world->main_collections[i];
			object = collection->first;

			// iterate through objects and updates them
			while (object)
			{
				update = object->upd_operations->first;

				// updates by accumulators
				while (update)
				{
					accumulate_value = 0.f;

					for (j = 0; j < count_collections; j++)
					{
						accumulate_value += update->accumalator[j];
					}

					update->operationF(update->target_field, accumulate_value);
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
		}
		/* END::UPDATE DEPENDENT COLLECTIONS */

		ftime(&end);

		dtime = 1000 * (end.time - start.time) + (end.millitm - start.millitm);
		sleep_time_ms = world->req_iter_time_ms - dtime;
		world->fps = dtime > 0 ? 1000 / dtime : 1000;

		ReleaseSemaphore(world->global_sync, 1, 0); // sem = 1, unlock swStopWorld caller if exist

		if (sleep_time_ms >= 0)
		{
			Sleep(sleep_time_ms);
		}

		// world did stoped, wait release caller
		if (world->state != 1)
		{
			WaitForSingleObject(world->global_sync, INFINITE);
			ReleaseSemaphore(world->global_sync, 1, 0); // sem = 1, for next iteration
		}
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

#ifdef _DEBUG
	printf("SIMPLE_WORLD::Helper thread start: %d\n", thread_index);
#endif
	// wait first launch
	WaitForSingleObject(world->internal_sync, INFINITE);

	SW_ObjectCollection* collection;
	SW_Object* object;
	SW_UpdOperation* update;

	// number for access to collections
	unsigned collection_index = thread_index + 1;

	while (world->is_exist == 1)
	{
		/* START::RESET INDEPENDENT COLLECTION */
		collection = world->independ_collections[collection_index];
		object = collection->first;

		while (object)
		{
			update = object->upd_operations->first;

			while (update)
			{
				update->accumalator[0] = update->reset_value;
				update = update->next;
			}

			object = object->next;
		}
		/* END::RESET INDEPENDENT COLLECTION */

		/* START::INTERACTION */
		collection = world->main_collections[collection_index];
		object = collection->first;

		while (object)
		{
			// TODO: add interaction
			object = object->next;
		}
		/* END::INTERACTION */

		/* START::UPDATE CONST OBJECTS */
		collection = world->const_collections[collection_index];
		object = collection->first;

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
		/* END::UPDATE CONST OBJECTS */

		/* START::UPDATE INDEPENDENT OBJECTS */
		collection = world->independ_collections[collection_index];
		object = collection->first;

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
		/* END::UPDATE INDEPENDENT OBJECTS */

		ReleaseSemaphore(world->helpers_sync[thread_index], 1, NULL);
		WaitForSingleObject(world->internal_sync, INFINITE);
	}

	free(_sw_thread_helper);
#ifdef _DEBUG
	printf("SIMPLE_WORLD::Helper thread end: %d\n", thread_index);
#endif
}