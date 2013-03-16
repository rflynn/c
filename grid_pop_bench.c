/* ex: set ts=2 tw=78 et: */

/**
 * Wed Oct 10 10:39:46 EDT 2007
 *
 * This code explores what the fastest method for populating a square matrix
 * "grid" of psuedo-random values between 0-25 (think A-Z). Speed is our goal
 * here, at the possible sacrifice of quality, sanity and portability. GCC on
 * Linux is used here.
 *
 * This was undertaken at the behest of "amazon101" in freenode ##c.
 *
 * "grid_pop_XXX" populate the grid with psuedo-random values.
 *
 * The rest are functions to test the speed of the computation and the
 * validity of the results.
 *
 *
 * Results currently look something like this:
 *
 * RAND_MAX=2147483647
 * N_TIMES=1000000
 *              name    time speedup
 *          grid_pop   4.621      0%
 * grid_pop_lessrand   1.553    197%
 *     grid_pop_bits   0.994    365%
 *
 * With the hideous grid_pop_bits taking the lead...
 *
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define GRID_SIZE 16

#define N_TIMES 1000000 /* test times */

typedef struct {
  unsigned char c[GRID_SIZE][GRID_SIZE];
} grid;

/**
 * the obvious way
 */
void grid_pop(grid *g)
{
  int i, j;
  for (i = 0; i < GRID_SIZE; i++)
    for (j = 0; j < GRID_SIZE; j++)
      g->c[i][j] = rand() % 26;
}

/**
 * \li use rand()'s 31 bits for multiple values at a time
 * \li while instead of for
 * \li count down towards zero instead of up
 */
void grid_pop_lessrand(grid *g)
{
  int i = GRID_SIZE;
#ifdef DEBUG
  assert(RAND_MAX >= (1 << 4 * 5));
#endif
  while (i--) {
    int j = GRID_SIZE;
    do {
      int k = 4,
          r = rand() >> 1;
      while (k--)
        g->c[i][--j] = r % 26, r >>= 5;
    } while (j);
  }
}

/**
 * there is no spoon
 * treat everything as an int and use bitmasks to control the maximum value
 */
void grid_pop_bits(grid *g)
{
  int i = GRID_SIZE * GRID_SIZE;
  int *x = (int *)g->c;
#ifdef DEBUG
  assert(GRID_SIZE % sizeof *x == 0);
#endif
  do {
    int r = rand();
    *x++ = (r & 0x0f0f0f0f) + ((r >> 4) & 0x07070707) + ((r >> 3) & 0x03030303);
  } while (i -= sizeof *x);
}

#if 0
/**
 * try using doubles
 */
void grid_pop_dbl(grid *g)
{
  int i = GRID_SIZE * GRID_SIZE;
  double *x = (double *)g->c;
#ifdef DEBUG
  assert(GRID_SIZE % sizeof *x == 0);
#endif
  do {
    double r = drand48();
    *x++ = (r & 0x0f0f0f0f) + ((r >> 4) & 0x07070707) + ((r >> 3) & 0x03030303);
  } while (i -= sizeof *x);
}
#endif

/**
 * ...
 */
void grid_pop_zen(grid *g)
{
  
}

void grid_print(const grid *g)
{
  int i = GRID_SIZE;
  while (i--) {
    int j = GRID_SIZE;
    while (j--)
      printf("%3d ", g->c[i][j]);
    putc('\n', stdout);
  }
}

void grid_hist(const grid *g)
{
  int i = GRID_SIZE,
      j, cnt[26] = { 0 };
  while (i--) {
    j = GRID_SIZE;
    while (j--)
      cnt[g->c[i][j]]++;
  }
  i = 0;
  while (i < sizeof cnt / sizeof cnt[0]) {
    j = cnt[i];
    printf("%c ", 'A' + i);
    while (j--)
      putc('*', stdout);
    printf(" %d\n", cnt[i++]);
  }
}

/**
 * Test the results of a function for correctness
 */
static void test(const char *name, void (*f)(grid *))
{
  static grid g;
  int i, j;
  memset(g.c, 0, sizeof g.c);
  f(&g);
  for (i = 0; i < GRID_SIZE; i++) {
    for (j = 0; j < GRID_SIZE; j++) {
      if (g.c[i][j] > 25) {
        fprintf(stderr, "%s: Invalid value (%d)!\n", name, g.c[i][j]);
        exit(EXIT_FAILURE);
      }
    }
  }
  //grid_hist(&g);
}

/**
 * run function f N_TIMES times and record how long it takes
 */
static void speed(const char *name, void (*f)(grid *))
{
  static double orig = 0;
  static grid g;
  struct timeval tv[2];
  double d[2], secs;
  int i;
  printf("%20s ", name);
  test(name, f);
  /* test speed */
  i = -N_TIMES;
  gettimeofday(tv, NULL);
  do
    f(&g);
  while (i++);
  gettimeofday(tv + 1, NULL);
  d[0] = (tv[0].tv_sec * 1000000) + tv[0].tv_usec;
  d[1] = (tv[1].tv_sec * 1000000) + tv[1].tv_usec;
  secs = (d[1] - d[0]) / 1000000;
  if (0 == orig)
    orig = secs;
  printf("%7.3f %6.0f%%\n", secs, ((orig / secs) * 100) - 100);
}

int main(void)
{
  printf("RAND_MAX=%d\n", RAND_MAX);
  printf("N_TIMES=%d\n", N_TIMES);
  srand(time(NULL));
  /* do it... */
  printf("%20s %7s %5s\n", "name", "time", "speedup");
  speed("grid_pop", grid_pop);
  speed("grid_pop_lessrand", grid_pop_lessrand);
  speed("grid_pop_bits", grid_pop_bits);
  //speed("grid_pop_dbl", grid_pop_dbl);
  //speed("grid_pop_zen", grid_pop_zen);
  return 0;
}

