/**
 * @file alloc.h
 * @brief OS abstraction layer
 * @author Sebastien Alaiwan
 * @date 2021-10-08
 */

/*
 * Copyright (C) 2021 - Sebastien Alaiwan
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 */

#pragma once

#include <stdint.h>

extern void* g_Blocks[256];

bool allocate(int blockCount, int64_t blockSize);
void deallocate(int64_t blockSize);

void* beginThread(void (* fp)(void*), void*);
void joinThread(void* thread);

