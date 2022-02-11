#pragma once

#include "../Object/UpdOperation.h"

typedef struct SUpdCollection
{
	SW_UpdOperation* first;
	SW_UpdOperation* last;

	unsigned count;
} SW_UpdCollection;

SW_UpdCollection* updCollectionCreate();

/*
	merge to_merge collection into target
*/
void updCollectionMerge(SW_UpdCollection* target, SW_UpdCollection* to_merge);

void updCollectionPush(SW_UpdCollection* collection, SW_UpdOperation* item);
/*
	remove object from collection, with_destroy = 1 - free object memory
*/
void updCollectionRemoveItem(SW_UpdCollection* collection, SW_UpdOperation* item, unsigned with_destroy);

/*
	Destroy dynamic items data
*/
void updCollectionDestroy(SW_UpdCollection* collection);

/*
	Just free collection memory without free items data, can be used after merge collections
*/
void updCollectionFree(SW_UpdCollection* collection);