#include <stdio.h>
main()
{
	void *a = (void *)1,
	     *b = (void *)2;
	printf("a vs b -> %d\n", b > a);
	return 0;
}
