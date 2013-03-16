/* ex: set ts=8 noet: */

#include <stdio.h>
#include <string.h>

static void remchr(const char *in, char *out, const int c)
{
	while ((*out = *in))
		out += (*in++ != c);
}

main()
{
	static const struct {
		int c;
		char *in,
		     *out;
	} Test[] = {
		{ 'A',	"ABC",	"BC"	},
		{ 'A',	"AAC",	"C"	},
		{ 'A',	"AAA",	""	},
		{ 'A',	"ABA",	"B"	},
		{ 'A',	"BAC",	"BC"	},
		{ 'A',	"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",	""	},
		{ 'D',	"ABC",	"ABC"	}
	}, *T;
	char buf[64];
	size_t i;
	unsigned N = 10000000;
	while (N--) {
	T = Test;
	i = sizeof Test / sizeof Test[0];
	while (i--) {
#if 0
		printf("#%u \"%s\" - '%c' -> \"%s\" (",
			(unsigned)(T - Test), T->in, T->c, T->out);
		fflush(stdout);
#endif
		strcpy(buf, T->in);
		remchr2(buf, buf, T->c);
#if 0
		printf("\"%s\") [%s]\n", buf, 0 == strcmp(buf, T->out) ? "OK" : "!!");
#endif
		T++;
	}
	}
	return 0;
}

