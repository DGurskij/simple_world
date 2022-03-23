#include "Interaction.h"
#include "Object.h"
#include <malloc.h>

SW_InteractionGroup* createInteractionGroup(unsigned count_threads, unsigned count_interactions)
{
	SW_InteractionGroup* interaction_group = malloc(sizeof(SW_InteractionGroup));

	if (interaction_group != 0)
	{
		unsigned i = 0;

		SW_Interaction*** interactions = malloc(sizeof(SW_Interaction**) * count_threads);
		unsigned* counts_interactions = malloc(sizeof(unsigned) * count_threads);

		if (!interactions || !counts_interactions)
		{
			if (interactions)
			{
				free(interactions);
			}

			if (counts_interactions)
			{
				free(counts_interactions);
			}

			free(interaction_group);

			return 0;
		}

		interaction_group->interactions = interactions;
		interaction_group->counts_interactions = counts_interactions;

		interaction_group->count_threads = count_threads;
		interaction_group->count_interactions = count_interactions;

		for (; i < count_threads; i++)
		{
			interactions[i] = malloc(sizeof(SW_Interaction*) * count_interactions);
			counts_interactions[i] = 0;

			if (interactions[i] == 0)
			{
				destroyInteractionGroup(interaction_group, i);
				return 0;
			}
		}
	}

	return interaction_group;
}

SW_Interaction* createInteraction(SW_Object* object1, SW_Object* object2, unsigned thread_owner, interactFunc interactF)
{
	SW_Interaction* interaction = malloc(sizeof(SW_Interaction));

	if (interaction != 0)
	{
		interaction->thread_owner = thread_owner;

		interaction->accumulators1 = malloc(sizeof(float*) * object1->count_upd_ops);
		interaction->accumulators2 = malloc(sizeof(float*) * object2->count_upd_ops);

		// bind interaction with accumulators for object1
		SW_UpdCollection* upd_ops = object1->upd_operations;
		SW_UpdOperation* upd_operation = upd_ops->first;
		unsigned i = 0;
		
		while (upd_operation)
		{
			interaction->accumulators1[i++] = &upd_operation->accumalator[thread_owner];

			upd_operation = upd_operation->next;
		}

		upd_ops = object1->upd_operations_disabled;
		upd_operation = upd_ops->first;
		i = 0;

		while (upd_operation)
		{
			interaction->accumulators1[i++] = &upd_operation->accumalator[thread_owner];

			upd_operation = upd_operation->next;
		}

		// bind interaction with accumulators for object2
		upd_ops = object2->upd_operations;
		upd_operation = upd_ops->first;
		i = 0;

		while (upd_operation)
		{
			interaction->accumulators2[i++] = &upd_operation->accumalator[thread_owner];

			upd_operation = upd_operation->next;
		}

		upd_ops = object2->upd_operations_disabled;
		upd_operation = upd_ops->first;
		i = 0;

		while (upd_operation)
		{
			interaction->accumulators2[i++] = &upd_operation->accumalator[thread_owner];

			upd_operation = upd_operation->next;
		}

		interaction->interactF = interactF;
	}

	return interaction;
}

void destroyInteraction(SW_Interaction* interaction)
{
	free(interaction->accumulators1);
	free(interaction->accumulators2);

	free(interaction);
}

void destroyInteractionGroup(SW_InteractionGroup* interaction_group, unsigned end_index)
{
	if (end_index > 0)
	{
		SW_Interaction*** interactions = interaction_group->interactions;
		unsigned count_interactions = 0;
		unsigned i;

		do
		{
			end_index--;

			count_interactions = interaction_group->counts_interactions[end_index];
			
			i = 0;

			for (; i < count_interactions; i++)
			{
				destroyInteraction(interactions[end_index][i]);
			}

			free(interactions[end_index]);
		} while (end_index != 0);
	}

	free(interaction_group->interactions);
	free(interaction_group->counts_interactions);

	free(interaction_group);
}