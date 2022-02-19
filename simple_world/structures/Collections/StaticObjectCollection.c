#include "StaticObjectCollection.h"
#include <malloc.h>

SW_StaticObjectCollection** createStaticGroup(unsigned count_collections, unsigned quantity_elements)
{
	SW_StaticObjectCollection** static_group = malloc(sizeof(SW_StaticObjectCollection*) * count_collections);

	if (static_group != 0)
	{
		unsigned i = 0;

		for (; i < count_collections; i++)
		{
			static_group[i] = createStaticCollection(quantity_elements);

			if (!static_group[i])
			{
				destroyStaticGroup(static_group, i);
				return 0;
			}
		}
	}

	return static_group;
}

SW_StaticObjectCollection* createStaticCollection(unsigned quantity)
{
	SW_StaticObjectCollection* collection = malloc(sizeof(SW_StaticObjectCollection));

	if (collection != 0)
	{
		collection->count = 0;
		collection->objects = malloc(sizeof(SW_Object*) * quantity);

		if (!collection->objects)
		{
			free(collection);
			collection = 0;
		}
	}

	return collection;
}

void doStaticReset(SW_StaticObjectCollection* collection)
{
	/*
	* TODO: implement
	*/
}

void doStaticUpdate(SW_StaticObjectCollection* collection)
{
	{
		/*
		* TODO: implement
		*/
	}
}

void destroyStaticGroup(SW_StaticObjectCollection** group, unsigned end_index)
{
	unsigned i = 0;
	unsigned count = 0;
	SW_Object** objects = 0;

	if (end_index > 0)
	{
		do
		{
			end_index--;

			count = group[end_index]->count;
			objects = group[end_index]->objects;

			for (i = 0; i < count; i++)
			{
				objectDestroy(objects[i]);
			}

			free(group[end_index]->objects);
			free(group[end_index]);
		} while (end_index != 0);
	}

	free(group);
}