#pragma once

#include "../../dll_main/simple_world.h"

struct SWInteraction
{
	float** accumulators1;
	float** accumulators2;

	unsigned thread_owner;

	SW_Object* object1;
	SW_Object* object2;

	interactFunc interactF;
};

typedef struct SWInteractionGorup
{
	SW_Interaction*** interactions;

	// count interactions of each threads, used for loop
	unsigned* counts_interactions;

	unsigned count_threads;
	unsigned count_interactions;
} SW_InteractionGroup;

SW_InteractionGroup* createInteractionGroup(unsigned count_threads, unsigned count_interactions);
SW_Interaction* createInteraction(SW_Object* object1, SW_Object* object2, unsigned thread_owner, interactFunc interactF);

void destroyInteraction(SW_Interaction* interaction);
void destroyInteractionGroup(SW_InteractionGroup* intercation_group, unsigned end_index);