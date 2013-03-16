/**
 * ex: set ts=2 et:
 *
 * Copyright 2008 Ryan Flynn
 *
 * www.parseerror.com
 *
 * Calculate the min and max values in an array of integers without branching
 *
 */

#include <stdio.h>
#include <limits.h>

void obvious(const int n[], size_t len, int *min, int *max)
{
  if (len) {
    int in = n[0],
        ax = n[0];
    unsigned i;
    for (i = 1; i < len; i++) {
      if (n[i] < in)
        in = n[i];
      if (n[i] > ax)
        ax = n[i];
    }
    *min = in;
    *max = ax;
  }
}

void obvious_else(const int n[], size_t len, int *min, int *max)
{
  if (len) {
    int in = n[0],
        ax = n[0];
    unsigned i;
    for (i = 1; i < len; i++) {
      if (n[i] < in)
        in = n[i];
      else if (n[i] > ax)
        ax = n[i];
    }
    *min = in;
    *max = ax;
  }
}

/**
 * calculate the minimum and maximum values in an array without branching
 */
void nonbranching(const int n[], size_t len, int *min, int *max)
{
  if (len) {
    int x[4] = { n[0], n[0], n[0], n[0] }; /* set all vals to the first item */
    unsigned i;
    for (i = 1; i < len; i++) {
      x[2 + (n[i] > x[3])] = n[i]; /* min */
      x[1 - (n[i] < x[0])] = n[i]; /* max */
    }
    *min = x[0]; /* copy results */
    *max = x[3];
  }
}

main()
{
  static const int N[] = { -1, 2, 6, 0 }; /* list we're searching */
  int min = INT_MIN,
      max = INT_MIN;
  minmax(N, sizeof N / sizeof N[0], &min, &max);
  printf("min=%d max=%d\n", min, max);
  return 0;
}

