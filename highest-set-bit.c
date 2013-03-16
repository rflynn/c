#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv)
{
	int i;
	for (i = 0; i < 32; i++) {
		int j = i;
		j |= (j >> 1);
		j |= (j >> 2);
		j |= (j >> 4);
		j |= (j >> 8);
		j |= (j >> 16);
		printf("%2d: %d\n", i, i ^ (i >> 1));
	}
	return 0;
}

