#pragma once
#include "../structures/World.h"
#include <sys/timeb.h>

/* thread without helpers supporting code */
void mainThreadSimple(void* world);

/* main thread with synchronize calls */
void mainThread(void* world);

void helperThread(void* world);