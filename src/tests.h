/**
 * @file tests.h
 * @brief List of registered RAM tests
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

using ul = uint64_t;
using ulv = volatile ul;

void runAllTests(ulv* buf1, ulv* buf2, size_t elementCount);
extern void reportError(const char* msg);

