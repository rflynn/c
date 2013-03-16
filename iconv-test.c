/* ex: set ts=2 et: */

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>

static void dump(const char *c, size_t len)
{
  while (len--)
    printf(isalnum((int)*c) ? " %c " : "%02hhX ", *c), c++;
  printf("\n");
}

main()
{
  char from[] = "HELLO THERE";
  static char to[256];
  char *fromp = from,
       *top = to;
  size_t fromlen = strlen(from),
         toleft = sizeof to,
         conv;
  iconv_t icon = iconv_open("UCS-2", "ASCII");
  if (icon == (void *)(size_t)-1) {
    perror("iconv_open");
    abort();
  }
  printf("iconv(%p, %p, %zu, %p, %zu) -> ",
    (void *)icon, (void *)&fromp, fromlen, (void *)&top, toleft);
  conv = iconv(icon, &fromp, &fromlen, &top, &toleft);
  printf("%zu\n", conv);
  printf("toleft=%zu\n", toleft);
  if (conv == (size_t)-1) {
    perror("iconv");
    abort();
  }
  dump(to, sizeof to - toleft);
  iconv_close(icon);
	return 0;
}

