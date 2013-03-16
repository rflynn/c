/* ex: set ts=2 et: */
/* Copyright 2008 Ryan Flynn */
/* demo program to reverse the order of space-delimited tokens */
/* @ref: http://destraynor.com/serendipity/index.php?/archives/134-Programming-Challenge-4.html */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct word {
  const char *str;
  int len;
};
static struct word *Stack = NULL;
static unsigned StackCnt = 0,
                StackAlloc = 0;

/**
 * push words onto the stack, allocating as we grow
 */
static size_t push(const char *s, size_t len)
{
  if (StackCnt == StackAlloc) {
    size_t bytes;
    void *tmp;
    StackAlloc = (StackAlloc + 1) * 2;
    bytes = StackAlloc * sizeof *Stack;
    tmp = realloc(Stack, bytes);
    if (NULL == tmp) {
      fprintf(stderr, "Failed to allocate %u stack words\n", StackAlloc);
      exit(EXIT_FAILURE);
    }
    Stack = tmp;
  }
  Stack[StackCnt].str = s;
  Stack[StackCnt].len = (int)len;
  StackCnt++;
  return len;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: %s \"string\"\n", argv[0]);
  } else {
    const char *s = argv[1],
              *end = s + strlen(s);
    size_t len;
    s += strspn(s, " "); /* skip initial spaces if any */
    /* push what's between spaces onto the stack in order */
    while (s < end && (len = strcspn(s, " "))) {
      s += push(s, len);
      s += strspn(s, " "); /* skip spaces */
    }
    /* pop off stack and print in reverse order */
    while (StackCnt--)
      printf("%.*s%s", Stack[StackCnt].len, Stack[StackCnt].str, StackCnt ? " " : "");
    printf("\n");
  }
  return 0;
}

