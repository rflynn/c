#include <stdio.h>
#include <math.h>
digits(int i) {
int n = pow(10, 1+(int)(log(i)/log(10)));
do printf("%d ", n % (i % (n/100)?(n/100):1) );
while (n /= 10); return 0; }
main(void){ return digits(123); }
