#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <string.h>

/*
 * original float encoding preserves all possible digits all the time.
 */
char * float_encode(const double f, char *buf, size_t len)
{
    if (0 > sprintf(buf, "%0.20g", f))
        return NULL;
    return buf;
}

/*
 * encode honoring DBL_DIG to reduce superfluous digits
 */
char * float_encode2(const double f, char *buf, size_t len)
{
    if (f <= 9999999.9)
    {
        if (0 > snprintf(buf, len, "%.*g", DBL_DIG+1, f))
            return NULL;
    } else {
        if (0 > snprintf(buf, len, "%0.20g", f))
            return NULL;
    }
    return buf;
}

double float_decode(const char *buf)
{
    double f = strtod(buf, NULL);
    return f;
}

static void test_encode(char * (*encode_func)(const double, char *, size_t))
{
    const static double F[] = {
        0,
        1,
        2,
        0.1,
        0.11,
        0.111,
        1.2345,
        1.23456,
        1.234567,
        1.2345678,
        1.23456789,
        DBL_EPSILON,
        3.141592653589793238462643383279502884197169399375105820974944592307816406286,
        123456789012345.141592653589793238462643383279502884197169399375105820974944592307816406286,
        DBL_MAX,
        DBL_MAX*2
    };
    size_t i, sumlen = 0;
    double sumdiff = 0;
    for (i = 0; i < sizeof F / sizeof F[0]; i++)
    {
        static char buf[32];
        double dec, diff;
        int eq;
        (void)encode_func(F[i], buf, sizeof buf);
        dec = float_decode(buf);
        sumlen += strlen(buf);
        diff = fabs(dec - F[i]);
        if (!isnan(diff))
            sumdiff += diff;
        eq = diff <= DBL_EPSILON;
        printf("%s %-26.20g -> %-26s -> %-12g +/- %g\n",
                eq ? "ok" : "!!", F[i], buf, dec, diff);
    }
    printf("sum:{diff:%g, len:%zu}\n", sumdiff, sumlen);
}

int main(void)
{
    printf("FLT_EPSILON=%g DBL_EPSILON=%g\n", FLT_EPSILON, DBL_EPSILON);
    test_encode(float_encode);
    test_encode(float_encode2);
    return 0;
}
