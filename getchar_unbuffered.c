#include <stdio.h>

main()
{
	int c;
	setvbuf(stdin, (char *)NULL, _IONBF, 0);
	while (EOF != (c = getchar()))
		putc(c, stdout);
	return 0;
}

