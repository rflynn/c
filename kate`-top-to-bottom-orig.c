/*** snip ***/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
struct fields { bool precision_set; };
/*** snip ***/

/*
 * Read the precision given by a formatting specifier. This is expected to be
 * passed a pointer to the '.' character (if present) which begins the precision
 * specification. It will consume as many further character as it needs, up to
 * but not including the conversion specifier or ength modifiers.
 *
 * The argument ap is expected to point to the current va_list; this may be
 * moved along by way of va_arg() if appropriate (namely for a precision of '*').
 *
 * A pointer to the conversion specifier is returned. (That is, a pointer to
 * the next character after the precision has been dealt with.)
 */
static const char *
readprecision(const char *p, struct fields *fields, int *precision, va_list *ap)
{
    char *ep;
    long int l;
 
    assert(p != NULL);
    assert(*p != '\0');
    assert(fields != NULL);
    assert(precision != NULL);
    assert(ap != NULL);
 
    /* precision omitted */
    if ('.' != *p) {
        return p;
    }
 
    p++;
 
    /* precision passed as parameter */
    if ('*' == *p) {
        *precision = va_arg(*ap, int);
        fields->precision_set = true;
        return p + 1;
    }
 
    /* S7.19.6.1 "if only the period is specified, the precision is taken as
     * zero." */
    if (!isdigit((unsigned char) *p)) {
        fields->precision_set = true;
        *precision = 0;
        return p;
    }
 
    /* TODO: do we need to save and restore errno to leave it untouched? */
    errno = 0;
    l = strtol(p, &ep, 10);
 
    if (ERANGE == errno && (LONG_MAX == l || LONG_MIN == l)) {
        /* TODO: handle error */
        return NULL;
    }
 
    if (l > INT_MAX || l < INT_MIN) {
        /* TODO: handle error */
        return NULL;
    }
 
    assert(ep > p);
 
    *precision = l;
    fields->precision_set = true;
    return ep;
}

/**************** snip ********************/

int main(void)
{
  return 0;
}
