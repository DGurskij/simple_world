#include "Object.h"
#include <malloc.h>

SW_Object* objectCreate(void* data, unsigned type, unsigned thread_owner)
{
	SW_Object* object = malloc(sizeof(SW_Object));

	if (object)
	{
		object->prev = 0;
		object->next = 0;

		object->data = data;

		object->upd_operations = updCollectionCreate();
		object->upd_operations_disabled = updCollectionCreate();

		object->upd_const_operations = updCollectionCreate();
		object->upd_const_operations_disabled = updCollectionCreate();

		object->upd_external_operations = updCollectionCreate();
		object->upd_external_operations_disabled = updCollectionCreate();

		object->count_upd_ops = 0;

		object->type = type;

		object->after_update_action = 0;

		object->thread_owner = thread_owner;

		object->disabled = 0;
	}

	return object;
}

/*
	Destroy object and auto delete it from collection if detect
*/
void objectDestroy(SW_Object* object)
{
	if (object->prev)
	{
		object->prev->next = object->next;
	}

	if (object->next)
	{
		object->next->prev = object->prev;
	}

	updCollectionDestroy(object->upd_operations);
	updCollectionDestroy(object->upd_operations_disabled);

	updCollectionDestroy(object->upd_const_operations);
	updCollectionDestroy(object->upd_const_operations_disabled);

	updCollectionDestroy(object->upd_external_operations);
	updCollectionDestroy(object->upd_external_operations_disabled);

	free(object);
}

void objectDestroyViaCollection(SW_Object* object)
{
	updCollectionDestroy(object->upd_operations);
	updCollectionDestroy(object->upd_operations_disabled);

	updCollectionDestroy(object->upd_const_operations);
	updCollectionDestroy(object->upd_const_operations_disabled);

	free(object);
}
