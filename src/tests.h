#pragma once

using ulv = unsigned long volatile;
void runAllTests(ulv* buf1, ulv* buf2, size_t elementCount);
extern void reportError(const char* msg);

