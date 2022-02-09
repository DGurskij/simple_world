#include "Object.h"
#include <malloc.h>

SW_Object* objectCreate(void* data, char independ)
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

		object->independ = independ;
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

	free(object);
}