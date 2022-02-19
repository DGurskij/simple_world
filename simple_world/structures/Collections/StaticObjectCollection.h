#pragma once

#include "../Object/Object.h"

typedef struct SStaticObjectCollection
{
	unsigned count;
	SW_Object** objects;
} SW_StaticObjectCollection;

SW_StaticObjectCollection** createStaticGroup(unsigned count_collections, unsigned quantity_elements);
SW_StaticObjectCollection* createStaticCollection(unsigned quantity);

void doStaticReset(SW_StaticObjectCollection* collection);
void doStaticUpdate(SW_StaticObjectCollection* collection);
// void doStaticIterate(SW_StaticObjectCollection* collection);

void destroyStaticGroup(SW_StaticObjectCollection** group, unsigned end_index);