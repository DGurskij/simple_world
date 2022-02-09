#include "UpdOperation.h"
#include <malloc.h>

void add(float* target_field, float accumalate_value);
void sub(float* target_field, float accumalate_value);
void mul(float* target_field, float accumalate_value);
void div(float* target_field, float accumalate_value);

void (*operationF[4])(float* target_field, float accumalate_value) = { add, sub, mul, div };

SW_UpdOperation* updOperationCreate(float* target_field, float* accumulator, float reset_value, unsigned operation)
{
	SW_UpdOperation* upd_operation = malloc(sizeof(SW_UpdOperation));

	if (upd_operation)
	{
		upd_operation->prev = 0;
		upd_operation->next = 0;

		upd_operation->target_field = target_field;
		upd_operation->accumalator = accumulator;
		upd_operation->reset_value = reset_value;

		upd_operation->operation = operation;
		upd_operation->operationF = operationF[operation];
	}

	return upd_operation;
}

SW_UpdOperation* updConstOperationCreate(float* target_field, float const_value, unsigned operation)
{
	SW_UpdOperation* upd_operation = malloc(sizeof(SW_UpdOperation));

	if (upd_operation)
	{
		upd_operation->prev = 0;
		upd_operation->next = 0;

		upd_operation->target_field = target_field;
		upd_operation->accumalator = 0;
		upd_operation->reset_value = const_value;

		upd_operation->operation = operation;
		upd_operation->operationF = operationF[operation];
	}

	return upd_operation;
}

/*SW_UpdOperation* updConstOperationCreate(float* target_field, float const_value, unsigned operation)
{
	SW_UpdOperation* upd_operation = malloc(sizeof(SW_UpdOperation));

	if (upd_operation)
	{
		upd_operation->prev = 0;
		upd_operation->next = 0;

		upd_operation->target_field = target_field;
		upd_operation->accumalator = accumulator;
		upd_operation->const_value = const_value;

		upd_operation->operation = operation;
		upd_operation->operationF = operationF[operation];
	}

	return upd_operation;
}*/

void updOperationDestroy(SW_UpdOperation* upd_operation)
{
	if (upd_operation->prev)
	{
		upd_operation->prev->next = upd_operation->next;
	}

	if (upd_operation->next)
	{
		upd_operation->next->prev = upd_operation->prev;
	}

	if (upd_operation->accumalator)
	{
		free(upd_operation->accumalator);
	}

	free(upd_operation);
}

void add(float* target_field, float accumalate_value)
{
	*target_field += accumalate_value;
}

void sub(float* target_field, float accumalate_value)
{
	*target_field -= accumalate_value;
}

void mul(float* target_field, float accumalate_value)
{
	*target_field *= accumalate_value;
}

void div(float* target_field, float accumalate_value)
{
	*target_field /= accumalate_value;
}