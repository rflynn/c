/* ex: set ts=2 tw=78 et: */
/**
 * Sun Oct  7 03:57:17 EDT 2007
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define VALUE     5
#define DATA_SIZE 1024

#define N_TIMES 1000000 /* test times */

typedef struct {
  size_t len;
  int data[DATA_SIZE+1];
} vector;

/**
 * run function f N_TIMES times and record how long it takes
 */
static double Orig = 0;
static void speed(const char *name, void (*f)(vector *), vector *v)
{
  struct timeval tv[2];
  double d[2], secs;
  int i;
  printf("%20s ", name);
  //test(name, f);
  /* test speed */
  i = -N_TIMES;
  gettimeofday(tv, NULL);
  do
    f(v);
  while (i++);
  gettimeofday(tv + 1, NULL);
  d[0] = (tv[0].tv_sec * 1000000) + tv[0].tv_usec;
  d[1] = (tv[1].tv_sec * 1000000) + tv[1].tv_usec;
  secs = (d[1] - d[0]) / 1000000;
  if (0 == Orig)
    Orig = secs;
  printf("%7.3f %6.0f%%\n", secs, ((Orig / secs) * 100) - 100);
}

static void bound_for(vector *v)
{
  size_t i;
  for (i = 0; i < v->len; i++)
    if (v->data[i] == VALUE)
      break;
}

static void bound_while(vector *v)
{
  size_t i = 0;
  while (i < v->len && v->data[i] != VALUE)
    i++;
}

static void sentinel(vector *v)
{
  int i = -1;
  v->data[DATA_SIZE] = VALUE;
  while (v->data[++i] != VALUE);
}

static void sentinel_backwards(vector *v)
{
  size_t i = DATA_SIZE;
  v->data[0] = VALUE;
  do
    --i;
  while (VALUE != v->data[i]);
}

int main(void)
{
  vector v;
  memset(v.data, 0, sizeof v.data);
  v.data[DATA_SIZE/2] = VALUE;
  printf("%20s %7s %5s\n", "name", "time", "speedup");
  speed("***calibrate***", bound_for, &v);
  speed("bound_for", bound_for, &v);
  speed("bound_while", bound_while, &v);
  speed("sentinel", sentinel, &v);
  speed("sentinel_backwards", sentinel_backwards, &v);
  return 0;
}

