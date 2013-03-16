#include <stdio.h>
main()
{
	char buf[5];
	int n = 123456;
	sprintf(buf, "%0*.*d", (int)sizeof buf, (int)sizeof buf - 1, n);
	puts(buf);
	return 0;
}
