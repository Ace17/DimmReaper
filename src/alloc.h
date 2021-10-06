#pragma once

#include <stdint.h>

extern void* g_Blocks[256];

bool allocate(int blockCount, int64_t blockSize);
void deallocate(int64_t blockSize);

void* beginThread(void (* fp)(void*), void*);
void joinThread(void* thread);

