#include <stdio.h>
#include <float.h>

main()
{
	double d = FLT_MAX + 1.;
	float f = (float)d;
	printf("FLT_MAX=%f d=%f f=%f d-f=%f\n", FLT_MAX, d, f, d - f);
	d = -FLT_MAX;
	d -= 1;
	f = (float)d;
	printf("-FLT_MAX=%f d=%f f=%f d-f=%f\n", -FLT_MAX, d, f, d - f);
	return 0;
}

