
/*
 * I need a fast, custom string trie written in C and available to Python.
 * What we really need is to be able to visit all matching prefixes for a given string
 * Start simple and see what happens...
 * Later: Hmmm, seems to actually work. A terrific way to turn 1 char into 3 pointers...
 * TODO: add remove, add ability to iterate through all full strings that prefix a given string...
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "trie1.h"

static void do_trie_dump(const struct trie *top, const struct trie *t, unsigned level)
{
	static const char Indent[] = "                                             ";
	if (t->c == L'\0')
		printf("%.*s%p \\0\n", level, Indent, (void*)t);
	else
		printf("%.*s%p %lc\n", level, Indent, (void*)t, t->c);
	if (t != top || level == 0) {
		if (t->child)
			do_trie_dump(top, t->child, level+1);
		if (t->next)
			do_trie_dump(top, t->next, level);
	}
}

void trie_dump(const struct trie *t)
{
	do_trie_dump(t, t, 0);
}

static struct trie * trie_alloc(const wchar_t c)
{
	struct trie *t = malloc(sizeof *t);
	t->c = c;
	t->next = NULL;
	t->child = NULL;
	return t;
}

/*
 * convenience function; all nodes should be malloc()ed anyway
 */
struct trie * trie_new(void)
{
	return trie_alloc(L'\0');
}

/*
 * release all memory held by trie and children, depth first
 */
static void do_trie_free(struct trie *t)
{
	if (t->next) do_trie_free(t->next);
	if (t->c) {
		if (t->child) do_trie_free(t->child);
		free(t);
	}
}

void trie_free(struct trie *t)
{
	if (t->next) do_trie_free(t->next);
	if (t->child) do_trie_free(t->child);
	if (!t->c) free(t); /* finally free top node */
}

static struct trie * do_trie_find_or_add(struct trie *parent, const wchar_t c)
{
	struct trie *s = parent->child;
	if (!parent->child)
		return (parent->child = trie_alloc(c));
	while (s->c < c && s->next && s->next->c < c)
		s = s->next;
	if (s->c == c)
		return s;
	else if (s->next && s->next->c == c)
		return s->next;
	{
		struct trie *n = trie_alloc(c);
		if (s->c > c) {
			n->next = s;
			if (s == parent->child)
				parent->child = n;
		} else {
			n->next = s->next;
			s->next = n;
		}
		return n;
	}
}

void trie_add(struct trie *t, const wchar_t *str)
{
	struct trie *top = t;
	while (*str)
		t = do_trie_find_or_add(t, *str++);
	t->child = top; /* point all \0 at the same place. brilliant! */
}

int trie_find(const struct trie *t, const wchar_t *str)
{
	if (*str)
		t = t->child;
	do
		while (t && t->c < *str)
			t = t->next;
	while (t && *str && (t->c == *str++) && (t = t->child));
	return t && !t->c; /* didn't run out of nodes; ended on '\0' */
}

/*
 * if a complete entry for str is found, recursively delete from the bottom up any
 * components not shared by another string
 */
int trie_del(struct trie *t, const wchar_t *str)
{
	/* seek all the way down, delete end... */
	if (*str) {
		struct trie *parent = t;
		t = t->child;
		while (t && t->c < *str)
			t = t->next;
		if (!t || t->c != *str || !trie_del(t, str+1))
			return 0; /* not found */
		/* string found, attempt to delete upwards... */
		if (!t->child || !t->child->c) {
			/* remove references from parent or siblings */
			if (parent->child == t) { /* first child */
				parent->child = t->next;
			} else { /* middle child */
				struct trie *sib = parent->child;
				while (sib->next != t)
					sib = sib->next;
				sib->next = sib->next->next;
			}
			free(t);
		}
	}
	return 1;
}

/*
 * given 'str', iterate through it and t. at each character if we have a complete
 * string in t, print it
 * TODO: what i really want to do is run a callback
 */
void trie_prefix_all_strings(const struct trie *t, const wchar_t *str)
{
	const wchar_t *orig = str;
	if (*str)
		t = t->child;
	do {
		while (t && t->c < *str)
			t = t->next;
		/* full strings will have first child where !c */
		if (t && t->child && !t->child->c)
			printf("%.*ls\n", (int)(str-orig+1), orig);
	} while (t && *str && (t->c == *str++) && (t = t->child));
}

#ifdef DEBUG
int main(void)
{
	struct trie *t = trie_new();
	printf("sizeof(struct trie) = %lu\n", sizeof *t);
	trie_dump(t);
	trie_add(t, L"tea");
	trie_del(t, L"tea");
	trie_dump(t);
	trie_add(t, L"tea");
	trie_add(t, L"ted");
	trie_add(t, L"ten");
	trie_dump(t);
	assert( trie_del(t, L"ten"));
	assert(!trie_del(t, L"ten"));
	trie_dump(t);
	assert( trie_find(t, L""));
	assert(!trie_find(t, L"t"));
	assert(!trie_find(t, L"te"));
	assert( trie_find(t, L"tea"));
	assert( trie_find(t, L"ted"));
	trie_free(t);
	t = trie_new();
	trie_add(t, L"he");
	trie_add(t, L"hell");
	trie_add(t, L"hello");
	trie_prefix_all_strings(t, L"hello");
	trie_prefix_all_strings(t, L"foobar");
	free(t);
	return 0;
}
#endif

