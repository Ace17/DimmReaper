#include <stdint.h>
#include <stdio.h>
#include <stdlib.h> // atoi

#include "alloc.h"
#include "tests.h"

void* g_Blocks[256] {};
void* g_Threads[256] {};

int64_t BlockSize = 1024 * 1024 * 1024LL; // default: 1Gb

void testOneMemoryBlock(void* block)
{
  const int64_t elementCount = (BlockSize / 2) / sizeof(ulv);
  auto buf1 = (ulv*)block;
  auto buf2 = (ulv*)((char*)(block) + BlockSize / 2);
  runAllTests(buf1, buf2, elementCount);
}

void reportError(const char* msg)
{
  fprintf(stderr, "FAILURE: %s\n", msg);
  fflush(stderr);
  exit(1);
}

int main(int argc, char* argv[])
{
  int N = 2;

  if(argc <= 1)
  {
    fprintf(stderr, "Usage: %s [blockCount] [blockSizeInMb]\n", argv[0]);
    return 1;
  }

  if(argc >= 2)
    N = atoi(argv[1]);

  if(argc >= 3)
    BlockSize = atoi(argv[2]) * 1024LL * 1024LL;

  if(argc >= 4)
  {
    fprintf(stderr, "Too many arguments.\n");
    return 1;
  }

  if(N > 256 || N <= 0)
  {
    fprintf(stderr, "The block count must be in [1;256], not %d.\n", N);
    return 1;
  }

  printf("DimmReaper - A multithreaded userspace memory tester\n");
  printf("Copyright (C) 2021 - Sébastien Alaiwan\n");
  fflush(stdout);

  if(!allocate(N, BlockSize))
  {
    fprintf(stderr, "Allocation failed.");
    return 1;
  }

  for(int k = 0;; ++k)
  {
    fprintf(stderr, "----- PASS %d -----\n", k);
    fflush(stderr);

    for(int i = 0; i < N; ++i)
      g_Threads[i] = beginThread(&testOneMemoryBlock, g_Blocks[i]);

    for(int i = 0; i < N; ++i)
      joinThread(g_Threads[i]);
  }

  deallocate(BlockSize);

  return 0;
}

