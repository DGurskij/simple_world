#include "UpdCollection.h"
#include <malloc.h>

SW_UpdCollection* updCollectionCreate()
{
	SW_UpdCollection* collection = (SW_UpdCollection*)malloc(sizeof(SW_UpdCollection));

	if (collection)
	{
		collection->count = 0;
		collection->first = 0;
		collection->last = 0;
	}

	return collection;
}

void updCollectionMerge(SW_UpdCollection* target, SW_UpdCollection* to_merge)
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

		target->count += to_merge->count;
	}
}

void updCollectionPush(SW_UpdCollection* collection, SW_UpdOperation* obj)
{
	if (collection->first)
	{
		collection->last->next = obj;
	}
	else
	{
		collection->first = obj;
	}

	collection->last = obj;
	collection->count++;
}

void updCollectionRemoveItem(SW_UpdCollection* collection, SW_UpdOperation* object, unsigned with_destroy)
{
	if (collection->first)
	{
		collection->first = collection->first->next;
	}
	else if (collection->last)
	{
		collection->last = collection->last->prev;
	}
	else
	{
		object->prev->next = object->next;
		object->next->prev = object->prev;
	}

	if (with_destroy)
	{
		updOperationDestroy(object);
	}

	collection->count--;
}

void updCollectionDestroy(SW_UpdCollection* collection)
{
	SW_UpdOperation* obj = collection->first;
	SW_UpdOperation* next;

	while (obj)
	{
		next = obj->next;
		updOperationDestroy(obj);
		obj = next;
	}

	free(collection);
}

void updCollectionFree(SW_UpdCollection* collection)
{
	free(collection);
}