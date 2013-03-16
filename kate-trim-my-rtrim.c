/* $Id: trim.c 304 2008-08-03 15:02:17Z kate $ */

/*
 * originally: http://elide.org/snippets/trim.c
 * Trim whitespace.
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Trim leading whitespace from a string. Whitespace is defined by isspace().
 *
 * Returns a pointer into s, at the address of the first non-whitespace
 * character. This should not be passed to free().
 */
const char *
ltrim(const char *s) {
	/* I would use strspn(), but we want to use isspace() here */
	while (isspace((unsigned char) *s)) {
		s++;
	}

	return s;
}

/*
 * Trim trailing whitespace from a string. Whitespace is defined by isspace().
 *
 * Returns s, modified to be terminated at the start of its trailing whitespace.
 */
char *
rtrim(char *s) {
	char *p = s + strlen(s) - 1;
	const char *end = p;
	while (p >= s && isspace((unsigned char) *p)) {
		p--;
	}
	if (p != end)
		*p = '\0';
	return s;
}

/*
 * Trim leading and trailing whitespace from a string. Whitespace is defined by
 * isspace().
 *
 * Returns a pointer into s, modified to be terminated. This should not be
 * passed to free().
 */
const char *
trim(char *s) {
	char *p;

	p = rtrim(s);
	return ltrim(p);
}

static void testltrim(const char *s, const char *e) {
	const char *o;

	o = ltrim(s);
	printf("ltrim(\"%s\") => \"%s\" %s\n", s, o, strcmp(e, o) == 0 ? "" : "XXX");
}

static void testrtrim(const char *s, const char *e) {
	const char *o;
	char *p;

	p = strdup(s);
	if (p == NULL) {
		perror("strdup");
		exit(EXIT_FAILURE);
	}

	o = rtrim(p);
	printf("rtrim(\"%s\") => \"%s\" %s\n", s, o, strcmp(e, o) == 0 ? "" : "XXX");

	free(p);
}

static void testtrim(const char *s, const char *e) {
	const char *o;
	char *p;

	p = strdup(s);
	if (p == NULL) {
		perror("strdup");
		exit(EXIT_FAILURE);
	}

	o = trim(p);
	printf("trim(\"%s\") => \"%s\" %s\n", s, o, strcmp(e, o) == 0 ? "" : "XXX");

	free(p);
}

int main(void) {
	/* ltrim */
	testltrim("", "");
	testltrim(" ", "");

	testltrim("a", "a");
	testltrim(" a", "a");
	testltrim("  a", "a");
	testltrim("   a", "a");
	testltrim("a ", "a ");
	testltrim("a  ", "a  ");
	testltrim("a   ", "a   ");
	testltrim(" a ", "a ");
	testltrim("  a ", "a ");
	testltrim(" a  ", "a  ");
	testltrim("  a  ", "a  ");
	testltrim(" a   ", "a   ");
	testltrim("a    ", "a    ");
	testltrim(" a ", "a ");
	testltrim("  a ", "a ");
	testltrim("   a ", "a ");

	testltrim("aa", "aa");
	testltrim(" aa", "aa");
	testltrim("aa ", "aa ");
	testltrim(" aa ", "aa ");

	testltrim("a a", "a a");
	testltrim(" a a", "a a");
	testltrim("a a ", "a a ");
	testltrim(" a a ", "a a ");

	testltrim("ab  ab", "ab  ab");
	testltrim("  ab  ab", "ab  ab");
	testltrim("ab  ab  ", "ab  ab  ");
	testltrim("  ab  ab  ", "ab  ab  ");

	/* rtrim */
	testrtrim("", "");
	testrtrim(" ", "");

	testrtrim("a", "a");
	testrtrim(" a", " a");
	testrtrim("  a", "  a");
	testrtrim("   a", "   a");
	testrtrim("a ", "a");
	testrtrim("a  ", "a");
	testrtrim("a   ", "a");
	testrtrim(" a ", " a");
	testrtrim("  a ", "  a");
	testrtrim(" a  ", " a");
	testrtrim("  a  ", "  a");
	testrtrim(" a   ", " a");
	testrtrim("a    ", "a");
	testrtrim(" a ", " a");
	testrtrim("  a ", "  a");
	testrtrim("   a ", "   a");

	testrtrim("aa", "aa");
	testrtrim(" aa", " aa");
	testrtrim("aa ", "aa");
	testrtrim(" aa ", " aa");

	testrtrim("a a", "a a");
	testrtrim(" a a", " a a");
	testrtrim("a a ", "a a");
	testrtrim(" a a ", " a a");

	testrtrim("ab  ab", "ab  ab");
	testrtrim("  ab  ab", "  ab  ab");
	testrtrim("ab  ab  ", "ab  ab");
	testrtrim("  ab  ab  ", "  ab  ab");

	/* trim */
	testtrim("", "");
	testtrim(" ", "");

	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");
	testtrim("a", "a");

	testtrim("aa", "aa");
	testtrim("aa", "aa");
	testtrim("aa", "aa");
	testtrim("aa", "aa");

	testtrim("a a", "a a");
	testtrim("a a", "a a");
	testtrim("a a", "a a");
	testtrim("a a", "a a");

	testtrim("ab  ab", "ab  ab");
	testtrim("ab  ab", "ab  ab");
	testtrim("ab  ab", "ab  ab");
	testtrim("ab  ab", "ab  ab");

	return 0;
}

