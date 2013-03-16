/* ex: set ts=2 et: */
#include <assert.h>
#include <stdio.h>

static void full_cycle(unsigned seed, unsigned size, unsigned prime)
{
  unsigned gen = seed % size;
  unsigned increment = prime;
  unsigned i;
  for(i = 0; i < size; i++) {
    printf("[%u] %u\n", i, gen);
    gen = (gen + prime) % size;
  }
}

main()
{
  srand(time(NULL) ^ getpid());
  full_cycle(rand(), 5, 7);
  return 0;
}

