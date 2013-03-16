/* ex: set ts=2 et: */
/*
 *
 * ~/c$ gcc -O3 -lm -o abs abs.c
 * ~/c$ ./abs
 * 500000000 iterations:
 *                      function seconds speedup
 *                    stdlib abs   2.009    0%
 *                     naive_abs   1.758   14%
 *                    sneaky_abs   1.883    7%
 * ~/c$
 *
 * good old naive C plus great optimization by GCC 4.1.1
 * proves to be the fastest
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/time.h>

int naive_abs(int i)
{
	if (i < 0)
		i = -i;
	return i;
}

int mask_abs(int a) {
  int mask = (a >> (sizeof(int) * CHAR_BIT - 1));
  return (a + mask) ^ mask;
}

int sun_abs(int i)
{
  __asm__(
    "movl %0, %%eax\n\t"
    "cdq\n\t"
    "xorl %%edx, %%eax\n\t"
    "subl %%edx, %%eax\n\t"
    : "=m"(i)
    : "m"(i)
    : "%eax","%edx"
  );
	return i;
}

int foo_abs(int i)
{
  __asm__(
    "movl %0, %%eax\n\t"
    "xorl $0xFFFFFFFF, %%eax\n\t"
    "incl %%eax\n\t"
    : "=m"(i)
    : "m"(i)
    : "%eax"
  );
	return i;
}

void test(int (*f)(int))
{
  assert(0 == f(0));
  assert(0 == f(-0));
  assert(1 == f(1));
  printf("f(-1) -> 0x%08x, -1 == 0x%08x\n",
    f(-1), -1);
  assert(1 == f(-1));
  assert(5 == f(-5));
  assert(5 == f(5));
  assert(0x7fffffff == f(-0x7fffffff));
}

#define N_TIMES 1 << 26
//#define N_TIMES 1000

static void speed(const char *name, int (*f)(int))
{
  static double orig = 0;
  struct timeval tv[2];
  double d[2], secs;
  int i;
  printf("%30s ", name);
  /* test for correctness ... */
  test(f);
  /* test speed */
  i = -N_TIMES;
  gettimeofday(tv, NULL);
  do
    f(i);
  while (i++);
  gettimeofday(tv + 1, NULL);
  d[0] = (tv[0].tv_sec * 1000000) + tv[0].tv_usec;
  d[1] = (tv[1].tv_sec * 1000000) + tv[1].tv_usec;
  secs = (d[1] - d[0]) / 1000000;
  if (0 == orig)
    orig = secs;
  printf("%7.3f %4.0f%%\n", secs, ((orig / secs) * 100) - 100);
}

int main(void)
{
  printf("%u iterations:\n", N_TIMES);
  printf("%30s %5s %s\n", "function", "seconds", "speedup");
  speed("foo_abs",  foo_abs);
  //speed("sun_abs",  sun_abs);
  speed("mask_abs",  mask_abs);
  speed("stdlib abs", abs);
  speed("naive_abs",  naive_abs);
  return 0;
}

