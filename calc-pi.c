/* pi^2 = 6*(1/(1^2) + 1/(2^2) + 1/(3^2) ...) */
#include <stdio.h>
#include <math.h>
int main(void)
{
	long terms = 50000, i = 1;
	long double pi = 1.L,
	            add;
	while (i++ < terms) {
		add = 1.L / (i * i);
		pi += add;
	}
	pi *= 6.L;
	printf("pi=%.30Lf\n", sqrtl(pi));
	return 0;
}

