#include <stdio.h>
int f(int n){ return n / 3; }
double d(double n){ return n / 3.0; }
int two(int a, int b){ int c = a * b; return c; }
int main(void){ int a = 1, b = 2; printf("%d\n", two(a,b)); return 0; }
