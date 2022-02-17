#include "ObjectCollection.h"
#include <malloc.h>
#include <stdio.h>

SW_ObjectCollection* objectCollectionCreate()
{
	SW_ObjectCollection* collection = (SW_ObjectCollection*)malloc(sizeof(SW_ObjectCollection));

	if (collection)
	{
		collection->first = 0;
		collection->last = 0;
	}

	return collection;
}

void objectCollectionMerge(SW_ObjectCollection* target, SW_ObjectCollection* to_merge)
{
	if (to_merge->first)
	{
		if (target->first)
		{
			target->last->next = to_merge->first;
			target->last = to_merge->last;
		}
		else
		{
			target->first = to_merge->first;
			target->last = to_merge->last;
		}
	}
}

void objectCollectionPush(SW_ObjectCollection* collection, SW_Object* obj)
{
	if (collection->first)
	{
		collection->last->next = obj;
		obj->prev = collection->last;
	}
	else
	{
		collection->first = obj;
	}

	collection->last = obj;
}

void objectCollectionRemoveObject(SW_ObjectCollection* collection, SW_Object* object, unsigned with_destroy)
{
	if (collection->first == object)
	{
		collection->first = object->next;
		object->next->prev = 0;
	}
	else if (collection->last == object)
	{
		collection->last = object->prev;
		object->prev->next = 0;
	}
	else
	{
		object->prev->next = object->next;
		object->next->prev = object->prev;
	}

	object->prev = 0;
	object->next = 0;

	if (with_destroy)
	{
		objectDestroy(object);
	}
}

void objectCollectionDestroy(SW_ObjectCollection* collection)
{
	SW_Object* obj = collection->first;
	SW_Object* next;

	while (obj)
	{
		next = obj->next;
		objectDestroy(obj);
		obj = next;
	}

	free(collection);
}

void objectCollectionFree(SW_ObjectCollection* collection)
{
	free(collection);
}