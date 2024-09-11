/*
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2020 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Copyright (C) 2021 - Sebastien Alaiwan
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 * This file contains the functions for the actual tests, called from the
 * main routine in main.cpp.
 *
 */

#include <limits.h>
#include <stdint.h>
#include <stdio.h>

#include "tests.h"

namespace
{
using TestFunc = int (*)(ulv* bufa, ulv* bufb, size_t count);
struct TestDesc
{
  const char* desc;
  TestFunc func;
  TestDesc* next = nullptr;
};

// random generator
uint32_t lcg_parkmiller()
{
  static thread_local uint32_t state = 12345;
  state = (uint64_t)state * 48271 % 0x7fffffff;
  return state;
}

ul rand_ul()
{
  uint64_t result = lcg_parkmiller();

  if(sizeof(ul) > 4)
  {
    result <<= 32;
    result |= lcg_parkmiller();
  }

  return (ul)result;
}

constexpr ul MakeUnsignedLongFromByte(uint8_t x)
{
  ul result = 0;

  for(size_t i = 0; i < sizeof(ul); ++i)
  {
    result <<= 8;
    result |= (uint8_t)x;
  }

  return result;
}

const auto UL_LEN = int(sizeof(ul) * 8);
const auto UL_ONEBITS = MakeUnsignedLongFromByte(0xff);

/* Function definitions. */

int compare_regions(ulv* bufa, ulv* bufb, size_t count)
{
  int r = 0;
  ulv* p1 = bufa;
  ulv* p2 = bufb;

  for(size_t i = 0; i < count; i++, p1++, p2++)
  {
    if(*p1 != *p2)
    {
      char msg[256];
      sprintf(msg, "0x%08llx != 0x%08llx at offset 0x%08llx.",
              (unsigned long long)*p1,
              (unsigned long long)*p2,
              (unsigned long long)(i * sizeof(ul)));
      reportError(msg);

      r = -1;
    }
  }

  return r;
}

int test_stuck_address(ulv* bufa, size_t count)
{
  ulv* p1 = bufa;

  for(int j = 0; j < 16; j++)
  {
    p1 = (ulv*)bufa;

    for(size_t i = 0; i < count; i++)
    {
      *p1 = ((j + i) % 2) == 0 ? (ul)(uintptr_t)p1 : ~((ul)(uintptr_t)p1);
      *p1++;
    }

    p1 = (ulv*)bufa;

    for(size_t i = 0; i < count; i++, p1++)
    {
      if(*p1 != (((j + i) % 2) == 0 ? (ul)(uintptr_t)p1 : ~((ul)(uintptr_t)p1)))
      {
        char msg[256];
        sprintf(msg,
                "possible bad address line at offset "
                "0x%08llx.\n",
                (unsigned long long)(i * sizeof(ul)));
        reportError(msg);

        return -1;
      }
    }
  }

  return 0;
}

TestDesc* g_FirstTest;

int registerTest(TestFunc func, const char* desc, TestDesc& test)
{
  test.func = func;
  test.desc = desc;
  test.next = g_FirstTest;
  g_FirstTest = &test;
  return 0;
}

#define MemTestLine(desc, line) \
  static int test_ ## line(ulv * bufa, ulv * bufb, size_t count); \
  static TestDesc test_ ## line ## _registration; \
  static int test_ ## line ## _registered = registerTest(&test_ ## line, desc, test_ ## line ## _registration); \
  static int test_ ## line(ulv * bufa, ulv * bufb, size_t count)

#define MemTestLine2(desc, line) \
  MemTestLine(desc, line)

#define MemTest(desc) \
  MemTestLine2(desc, __LINE__)

///////////////////////////////////////////////////////////////////////////////

MemTest("Random Value")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;

  for(size_t i = 0; i < count; i++)
  {
    *p1++ = *p2++ = rand_ul();
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Compare XOR")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;
  ul q = rand_ul();

  for(size_t i = 0; i < count; i++)
  {
    *p1++ ^= q;
    *p2++ ^= q;
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Compare SUB")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;
  ul q = rand_ul();

  for(size_t i = 0; i < count; i++)
  {
    *p1++ -= q;
    *p2++ -= q;
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Compare MUL")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;
  ul q = rand_ul();

  for(size_t i = 0; i < count; i++)
  {
    *p1++ *= q;
    *p2++ *= q;
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Compare DIV")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;
  ul q = rand_ul();

  for(size_t i = 0; i < count; i++)
  {
    if(!q)
    {
      q++;
    }

    *p1++ /= q;
    *p2++ /= q;
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Compare OR")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;
  ul q = rand_ul();

  for(size_t i = 0; i < count; i++)
  {
    *p1++ |= q;
    *p2++ |= q;
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Compare AND")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;
  const ul q = rand_ul();

  for(size_t i = 0; i < count; i++)
  {
    *p1++ &= q;
    *p2++ &= q;
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Sequential Increment")
{
  ulv* p1 = bufa;
  ulv* p2 = bufb;
  const ul q = rand_ul();

  for(size_t i = 0; i < count; i++)
  {
    *p1++ = *p2++ = (i + q);
  }

  return compare_regions(bufa, bufb, count);
}

MemTest("Solid Bits")
{
  for(int j = 0; j < 64; j++)
  {
    ul q = (j % 2) == 0 ? UL_ONEBITS : 0;

    ulv* p1 = bufa;
    ulv* p2 = bufb;

    for(size_t i = 0; i < count; i++)
    {
      *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
    }

    if(compare_regions(bufa, bufb, count))
    {
      return -1;
    }
  }

  return 0;
}

MemTest("Checkerboard")
{
  static constexpr auto CHECKERBOARD1 = MakeUnsignedLongFromByte(0x55);
  static constexpr auto CHECKERBOARD2 = MakeUnsignedLongFromByte(0xAA);

  for(int j = 0; j < 64; j++)
  {
    const ul q = (j % 2) == 0 ? CHECKERBOARD1 : CHECKERBOARD2;
    ulv* p1 = bufa;
    ulv* p2 = bufb;

    for(size_t i = 0; i < count; i++)
    {
      *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
    }

    if(compare_regions(bufa, bufb, count))
    {
      return -1;
    }
  }

  return 0;
}

MemTest("Block Sequential")
{
  for(int j = 0; j < 256; j++)
  {
    ulv* p1 = bufa;
    ulv* p2 = bufb;

    for(size_t i = 0; i < count; i++)
    {
      *p1++ = *p2++ = (ul)MakeUnsignedLongFromByte(j);
    }

    if(compare_regions(bufa, bufb, count))
    {
      return -1;
    }
  }

  return 0;
}

MemTest("Walking Zeroes")
{
  for(int j = 0; j < UL_LEN * 2; j++)
  {
    ulv* p1 = bufa;
    ulv* p2 = bufb;

    for(size_t i = 0; i < count; i++)
    {
      if(j < UL_LEN)  /* Walk it up. */
      {
        *p1++ = *p2++ = 1UL << j;
      }
      else /* Walk it back down. */
      {
        *p1++ = *p2++ = 1UL << (UL_LEN * 2 - j - 1);
      }
    }

    if(compare_regions(bufa, bufb, count))
    {
      return -1;
    }
  }

  return 0;
}

MemTest("Walking Ones")
{
  for(int j = 0; j < UL_LEN * 2; j++)
  {
    ulv* p1 = bufa;
    ulv* p2 = bufb;

    for(size_t i = 0; i < count; i++)
    {
      if(j < UL_LEN)  /* Walk it up. */
      {
        *p1++ = *p2++ = UL_ONEBITS ^ (1UL << j);
      }
      else /* Walk it back down. */
      {
        *p1++ = *p2++ = UL_ONEBITS ^ (1UL << (UL_LEN * 2 - j - 1));
      }
    }

    if(compare_regions(bufa, bufb, count))
    {
      return -1;
    }
  }

  return 0;
}

MemTest("Bit Spread")
{
  for(int j = 0; j < UL_LEN * 2; j++)
  {
    ulv* p1 = bufa;
    ulv* p2 = bufb;

    for(size_t i = 0; i < count; i++)
    {
      if(j < UL_LEN)  /* Walk it up. */
      {
        *p1++ = *p2++ = (i % 2 == 0)
                    ? (1UL << j) | (1UL << (j + 2))
                    : UL_ONEBITS^ ((1UL << j)
                                   | (1UL << (j + 2)));
      }
      else /* Walk it back down. */
      {
        *p1++ = *p2++ = (i % 2 == 0)
                    ? (1UL << (UL_LEN * 2 - 1 - j)) | (1UL << (UL_LEN * 2 + 1 - j))
                    : UL_ONEBITS^ (1UL << (UL_LEN * 2 - 1 - j)
                                   | (1UL << (UL_LEN * 2 + 1 - j)));
      }
    }

    if(compare_regions(bufa, bufb, count))
    {
      return -1;
    }
  }

  return 0;
}

MemTest("Bit Flip")
{
  for(int k = 0; k < UL_LEN; k++)
  {
    ul q = 1UL << k;

    for(int j = 0; j < 8; j++)
    {
      q = ~q;
      ulv* p1 = bufa;
      ulv* p2 = bufb;

      for(size_t i = 0; i < count; i++)
      {
        *p1++ = *p2++ = (i % 2) == 0 ? q : ~q;
      }

      if(compare_regions(bufa, bufb, count))
      {
        return -1;
      }
    }
  }

  return 0;
}
}

void runAllTests(ulv* buf1, ulv* buf2, size_t elementCount)
{
  test_stuck_address(buf1, elementCount);
  test_stuck_address(buf2, elementCount);

  auto test = g_FirstTest;

  while(test)
  {
    test->func(buf1, buf2, elementCount);
    test = test->next;
  }
}

