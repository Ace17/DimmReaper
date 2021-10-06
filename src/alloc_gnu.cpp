#include "alloc.h"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/resource.h>

namespace
{
struct ThreadStartInfo
{
  pthread_t handle;
  void (* fp)(void*);
  void* userParam;
};

void* threadProc(void* userParam)
{
  auto t = (ThreadStartInfo*)userParam;
  t->fp(t->userParam);

  return nullptr;
}
}

bool allocate(int blockCount, int64_t blockSize)
{
  for(int i = 0; i < blockCount; ++i)
  {
    fprintf(stderr, "Allocating %ld Mb ... ", blockSize / 1024 / 1024);
    fflush(stderr);

    g_Blocks[i] = mmap(NULL, blockSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    if(!g_Blocks[i])
    {
      fprintf(stderr, "Error: %d\n", errno);
      return false;
    }

    fprintf(stderr, "locking ...");
    fflush(stderr);

    if(mlock(g_Blocks[i], blockSize) < 0)
    {
      fprintf(stderr, "Error: %d\n", errno);
      return false;
    }

    fprintf(stderr, "OK\n");
    fflush(stderr);
  }

  // run at low priority
  setpriority(PRIO_PROCESS, 0, 19);

  return true;
}

void deallocate(int64_t blockSize)
{
  for(auto& buf : g_Blocks)
    munmap(buf, blockSize);
}

void* beginThread(void (* fp)(void*), void* userParam)
{
  auto t = (ThreadStartInfo*)malloc(sizeof(ThreadStartInfo));
  t->fp = fp;
  t->userParam = userParam;
  pthread_create(&t->handle, nullptr, &threadProc, t);
  return t;
}

void joinThread(void* thread)
{
  auto t = (ThreadStartInfo*)thread;
  pthread_join(t->handle, nullptr);
  free(t);
}

