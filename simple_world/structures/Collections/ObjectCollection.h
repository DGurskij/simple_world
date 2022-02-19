#pragma once
#include "../Object/Object.h"

typedef struct SObjectCollection
{
	SW_Object* first;
	SW_Object* last;
} SW_ObjectCollection;

SW_ObjectCollection** createObjectGroup(unsigned count_collections);
SW_ObjectCollection* objectCollectionCreate();

/*
	merge to_merge collection into target
*/
void objectCollectionMerge(SW_ObjectCollection* target, SW_ObjectCollection* to_merge);

/*
	push object object to the end of the collection
*/
void objectCollectionPush(SW_ObjectCollection* collection, SW_Object* obj);

/*
	remove object from collection, with_destroy = 1 - free object memory
*/
void objectCollectionRemoveObject(SW_ObjectCollection* collection, SW_Object* object);

/*
	Destroy dynamic objects data, user data keep save
*/
void objectCollectionDestroy(SW_ObjectCollection* collection);
void destroyObjectGroup(SW_ObjectCollection** group, unsigned end_index);