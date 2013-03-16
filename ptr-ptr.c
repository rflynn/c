#include <stdlib.h>
int main()
{
	int s = 3;
	char **c;
	*c = malloc(s);
	free(*c);
	return 0;
}
