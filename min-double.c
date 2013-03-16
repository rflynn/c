/* ex: set ts=2 tw=78 et: */
/**
 * Thu Nov 01 10:53:35 EST
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define N_TIMES 10000000 /* test times */

static void test(const char *name, double (*f)(double, double))
{
  printf("testing %s... ", name);
  fflush(stdout);
  assert(0. == f(0., 1.));
  assert(1. == f(1., 1.00001));
  assert(-1. == f(-1, 0.));
  assert(1e5 == f(1e5, 1.1e5));
  assert(99. == f(99., 2e10));
}

/**
 * run function f N_TIMES times and record how long it takes
 */
static double Orig = 0;
static void speed(const char *name, double (*f)(double, double))
{
  struct timeval tv[2];
  double v[2] = { drand48(), drand48() };
  double d[2], secs;
  int i;
  printf("%20s ", name);
  /* test speed */
  i = -N_TIMES/2;
  gettimeofday(tv, NULL);
  do {
    f(v[0], v[1]);
    f(v[1], v[0]);
  } while (i++);
  gettimeofday(tv + 1, NULL);
  d[0] = (tv[0].tv_sec * 1000000) + tv[0].tv_usec;
  d[1] = (tv[1].tv_sec * 1000000) + tv[1].tv_usec;
  secs = (d[1] - d[0]) / 1000000;
  if (0 == Orig)
    Orig = secs;
  printf("%7.3f %6.0f%%\n", secs, ((Orig / secs) * 100) - 100);
}

static double obvious(double x, double y)
{
  if (x < y)
    return x;
  return y;
}

static double invert(double x, double y)
{
  if (y > x)
    return x;
  return y;
}

static double array(double x, double y)
{
  const double *z[] = { &x, &y };
  return *z[x > y];
}


int main(void)
{
  printf("%20s %7s %5s\n", "name", "time", "speedup");
  speed("***calibrate***", obvious);
  speed("obvious", obvious);
  speed("invert", invert);
  speed("array", array);
  return 0;
}

