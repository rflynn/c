/* ex: set ts=2 et: */
/* abuse a char* and treat it like an array of char**s */

#include <stdio.h>

static void link(char *foo)
{
  char **bar,
       **baz;
  int i;
  /* create a list of pointers */
  baz = (char **)foo;
  for (i = 0; i < 3; i++) {
    bar  = baz;
    *bar = (char *)bar + sizeof bar;
    baz  = (char **)*bar;
  }
  *baz = NULL;
}

static void follow(char *foo)
{
  char **bar = (char **)foo;
  int i = 0;
  while (*bar) {
    printf("bar=%p...\n", (void *)*bar);
    bar = (char **)*bar;
  }
  printf("bar=%p...\n", (void *)*bar);
}

main()
{
  char foo[32];
  link(foo);
  follow(foo);
  return 0;
}

