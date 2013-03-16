#include <stdio.h>

typedef char footype[1];

void f(footype *foo)
{
  printf("(%c)\n", (*foo)[0]);
}

int main(void)
{
  footype foo = "1";
  f(&foo);
  return 0;
}

