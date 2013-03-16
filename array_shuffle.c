/* ex: set ts=2 et: */
#include <stdio.h>
#include <stdlib.h>
int cmp(const void *a, const void *b) { static const int i[3] = { -1, 0, 1 }; return i[rand() % 3]; }
main()
{
	const char c[5] = "abcde";
	int i[5] = { 0, 1, 2, 3, 4 };
  srand(time(NULL) ^ getpid());
  qsort(i, sizeof i / sizeof i[0], sizeof i[0], cmp);
  {
    int j;
    for (j = 0; j < sizeof i / sizeof i[0]; j++)
      printf("[%d] [%d] %c\n", j, i[j], c[i[j]]);
  }
  return 0;
}
