/**
 * @file alloc_mingw32.cpp
 * @brief MS Windows OS-specific implementation
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

#include "alloc.h"

#include <stdio.h>
#include <windows.h>

namespace
{
struct ThreadStartInfo
{
  void (* fp)(void*);
  void* userParam;
};

DWORD WINAPI threadProc(_In_ LPVOID lpParameter)
{
  auto info = *(ThreadStartInfo*)lpParameter;
  free(lpParameter);

  info.fp(info.userParam);
  return 0;
}
}

bool allocate(int blockCount, int64_t blockSize)
{
  // leave some room for the rest of the program
  const int64_t ExtraRoom = 32 * 1024 * 1024;
  const int64_t WorkingSet = blockCount * blockSize + ExtraRoom;

  if(!::SetProcessWorkingSetSize(::GetCurrentProcess(), WorkingSet, WorkingSet))
  {
    fprintf(stderr, "Can't set working set size to %lld Mb : %lu\n", (WorkingSet / 1024 / 1024), GetLastError());
    return false;
  }

  for(int i = 0; i < blockCount; ++i)
  {
    fprintf(stderr, "Allocating %d Mb ... ", int(blockSize / 1024 / 1024));
    fflush(stderr);

    g_Blocks[i] = VirtualAlloc(NULL, blockSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if(!g_Blocks[i])
    {
      fprintf(stderr, "Error: %lu\n", GetLastError());
      return false;
    }

    fprintf(stderr, "locking ...");
    fflush(stderr);

    if(!VirtualLock(g_Blocks[i], blockSize))
    {
      fprintf(stderr, "Error: %lu\n", GetLastError());
      return false;
    }

    fprintf(stderr, "OK\n");
    fflush(stderr);
  }

  if(!SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS))
  {
    fprintf(stderr, "Couldn't set low priority!\n");
    return false;
  }

  return true;
}

void deallocate(int64_t blockSize)
{
  for(auto& buf : g_Blocks)
  {
    if(!buf)
      continue;

    VirtualUnlock(buf, blockSize);
    VirtualFree(buf, 0, MEM_RELEASE);
  }
}

void* beginThread(void (* fp)(void*), void* userParam)
{
  ThreadStartInfo* info = (ThreadStartInfo*)malloc(sizeof(ThreadStartInfo));
  info->fp = fp;
  info->userParam = userParam;

  DWORD id;
  HANDLE h = CreateThread(nullptr, 0, threadProc, info, 0, &id);

  if(!h)
    return nullptr;

  return (void*)h;
}

void joinThread(void* thread)
{
  WaitForSingleObject((HANDLE)thread, INFINITE);
}

