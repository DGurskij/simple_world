#pragma once

#include "../../dll_main/simple_world.h"

struct SWUpdOperation
{
	SW_UpdOperation* prev;
	SW_UpdOperation* next;

	float* target_field;

	/*
		contains accumalate values or ptr on source value for external update
	*/
	float* accumalator;

	/*
		update accumulator values or used for const update
	*/
	float reset_value;

	unsigned operation;
	void (*operationF)(float* target_field, float value);

	unsigned is_external;
};

SW_UpdOperation* updOperationCreate(float* target_field, float* accumulator, float reset_value, unsigned operation);
SW_UpdOperation* updConstOperationCreate(float* target_field, float const_value, unsigned operation);
SW_UpdOperation* updExternalOperationCreate(float* target_field, float* source, unsigned operation);

void updOperationDestroy(SW_UpdOperation* upd_operation);