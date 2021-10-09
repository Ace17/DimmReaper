#pragma once

using ul = unsigned long;
using ulv = volatile ul;

void runAllTests(ulv* buf1, ulv* buf2, size_t elementCount);
extern void reportError(const char* msg);

