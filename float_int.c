#include <stdio.h>
#include <limits.h>
main()
{
	float f = (float)INT_MAX + 1;
	printf("equal? %d\n", (int)f == f);
	return 0;
}

