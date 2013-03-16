/* ex: set ts=2 et: */
/*
 * Investigate methods of testing whether an integer is a perfect square.
 *
 * Copyright 2009 Ryan Flynn
 * pizza@parseerror.com
 * Wed Mar 11 20:53:23 EDT 2009
 *
 * Ref:
 *  "Fastest way to determine if an integer's square root is an integer"
 *  <URL: http://stackoverflow.com/questions/295579/fastest-way-to-determine-if-an-integers-square-root-is-an-integer/>
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>

#if defined(_WIN32) || defined(_WIN64)
typedef unsigned __int64 uint64_t;
# define snprintf _snprintf
#else
# include <stdint.h>
#endif

/**
 * canonical way to determine if n is a perfect square
 */
static int perfect(const uint64_t n)
{
  const uint64_t s = (uint64_t)(sqrt(n) + 0.5);
  return s * s == n;
}

/**
 * inline asm uses x86 fpu
 *  - saves call to libm
 *  - gcc 4.3.3 -O3 generates similar code; but includes redundant call to sqrt() anyways...
 */
static int perfect2(const uint64_t n)
{
  int p = 0;
#if defined(_WIN32) || defined(_WIN64)
  fprintf(stderr, "write Visual Studio-friendly inline asm!\n");
  abort();
#else
  __asm__ volatile (
    "fildl   %2             ;" /* st(0)     <- n              */
    "fld     %%st(0)        ;" /* st(1)     <- st(0)          */
    "fsqrt                  ;" /* st(0)     <- sqrt(st(0))    */
    "frndint                ;" /* st(0)     <- round(st(0))   */
    "fmul    %%st(0),%%st(0);" /* st(0)     <- st(0) * st(0)  */
    "fucomip %%st(1),%%st(0);" /* eflags.zf <- st(0) == st(1) */
    "sete    %%al           ;" /* p         <- 1 if eflags.zf */
    : "=a"(p)
    : "a"(p), "m"(n)
  );
#endif
  return p;
}

static uint64_t parse(const char *s)
{
  uint64_t n;
  errno = 0;
  n = strtoull(s, NULL, 10);
  if (errno) {
    perror("strtoul");
    fprintf(stderr, "use -h for help\n");
    exit(1);
  }
  return n;
}

int main(int argc, char *argv[])
{
  uint64_t n,
           limit;
  if (argc < 2 || 0 == strcmp("-h", argv[1])) {
    fprintf(stderr, "Usage: %s [check|speed] <n>\n", argv[0]);
    exit(1);
  }
  if (0 == strcmp(argv[1], "check")) {
    /*
     * check against canonical for correctness
     */
    limit = parse(argv[2]);
    printf("checking [0,%llu]...\n", limit);
    n = 0;
    do
      if (perfect2(n) != perfect(n)) {
        printf("perfect(%llu)=%d perfect2(%llu)=%d (!)\n",
          n, perfect(n), n, perfect2(n));
        exit(1);
      }
    while (n++ != limit);
    printf("checked 0-%llu\n", limit-1);
  } else if (0 == strcmp(argv[1], "speed")) {
    /*
     * check a range of input against our fastest candidate for timing purposes
     */
    limit = parse(argv[2]);
    printf("checking [0,%llu]...\n", limit);
    n = 0;
    do
      (void)perfect2(n);
    while (n++ != limit);
    printf("done.\n");
  } else {
    /*
     * test a specific number
     */
    char test[256];
    n = parse(argv[1]);
    snprintf(test, sizeof test, "%llu", n);
    if (0 != strcmp(test, argv[1])) {
      fprintf(stderr, "Shit, strtoull parsed \"%s\" as \"%s\"...\n", argv[1], test);
      exit(1);
    }
    printf("%llu is %sa perfect square\n",
      n, (perfect2(n) ? "" : "not "));
  }
  return 0;
}

