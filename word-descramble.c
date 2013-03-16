/* ex: set ts=2 et: */
/*
 * "descramble" single word entries, matching them against a
 * "dictionary" of possible answers
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define DICT_FILE "/usr/share/dict/words"

#undef DEBUG

typedef struct {
  int len, /* total amount of letters */
	    letcnt[128];
  char orig[64];
} word;

static word *Dict = NULL;
static long DictAlloc = 0,
            DictLen = 0;

static void dict_dump(void)
{
  long i = 0;
  while (i < DictLen)
    printf("#%3ld %s\n", ++i, Dict[i].orig);
}

static void word_compile(word *w, char *txt)
{
  /* initialize */
  w->len = 0;
  memset(w->letcnt, 0, sizeof w->letcnt);
  strcpy(w->orig, txt); /* FIXME: will overflow buffer */
  /* build profile */
  while (*txt) {
    assert(*txt < sizeof w->letcnt / sizeof w->letcnt[0]);
    w->letcnt[*txt]++;
    w->len++;
    txt++;
  }
}

static int word_cmp(const void *va, const void *vb)
{
  const word *a = va,
             *b = vb;
  int c = a->len - b->len;
  if (c)
    c = memcmp(a->letcnt, b->letcnt, sizeof a->letcnt);
  return c;
}

static void dict_space(void)
{
  if (DictLen == DictAlloc) {
    size_t num = DictLen ? DictLen * 2 : 64,
           bytes = num * sizeof *Dict;
    void *tmp = realloc(Dict, bytes);
#ifdef DEBUG
    printf("num=%u bytes=%u tmp=%p\n", (unsigned)num, (unsigned)bytes, tmp);
#endif
    assert(tmp);
    Dict = tmp;
    DictAlloc = num;
  }
  assert(DictLen < DictAlloc);
}

static int input_legit(const char *line)
{
  return line[0] != '\r' &&
         line[0] != '\n' &&
         line[0] != '#'  &&
         line[0] != ';';
}

static void input_trim(char *line)
{
  strcspn(line, "\r\n")[line] = '\0';
}

static void dict_add(char *line)
{
  dict_space();
  input_trim(line);
  word_compile(Dict + DictLen++, line);
}

static void dict_load(const char *filename)
{
  char line[256];
  FILE *f = fopen(filename, "r");
  printf("Loading '%s'... ", filename);
  if (NULL == f) {
    perror("fopen");
    exit(EXIT_FAILURE);
  }
  while (fgets(line, sizeof line, f))
    if (input_legit(line))
      dict_add(line);
  fclose(f);
  qsort(Dict, DictLen, sizeof *Dict, word_cmp);
  printf("%ld entries.\n", DictLen);
}

static void dict_check(char *line)
{
  word w,
       *scroll;
  input_trim(line);
  word_compile(&w, line);
#ifdef DEBUG
  printf("dict_check(\"%s\")\n", line);
#endif
  scroll = bsearch(&w, Dict, DictLen, sizeof *Dict, word_cmp);
  if (NULL == scroll) {
    printf("No matches.\n");
  } else {
    printf("Matches:\n");
    /* scroll back to first match */
    while (scroll >= Dict && 0 == word_cmp(scroll - 1, &w))
      scroll--;
    if (scroll < Dict)
      scroll = Dict;
    /* scroll forwards */
    while (scroll < Dict + DictLen && 0 == word_cmp(scroll, &w)) {
      printf("%s\n", scroll->orig);
      scroll++;
    }
  }
}

int main(int argc, char *argv[])
{
  char line[256],
       *rd;
  printf("                  ___                                 \n");
  printf("                 |  ~~--.                             \n");
  printf("                 |%%=@%%%%/                              \n");
  printf("                 |o%%%%%%/  PIZZA'S WORD DESCRAMBLER     \n");
  printf("              __ |%%%%o/    2008-08-04                  \n");
  printf("        _,--~~ | |(_/ ._    USES /usr/share/dict/words\n");
  printf("     ,/'  m%%%%%%%%| |o/ /  `\\.   TYPE WORD, HIT ENTER    \n");
  printf("    /' m%%%%o(_)%%| |/ /o%%%%m `\\    CTRL+D TO QUIT        \n");
  printf("  /' %%%%@=%%o%%%%%%o|   /(_)o%%%%%% `\\                        \n");
  printf(" /  %%o%%%%%%%%%%=@%%%%|  /%%%%o%%%%@=%%%%  \\                       \n");
  printf("|  (_)%%(_)%%%%o%%%%| /%%%%%%=@(_)%%%%%%  |                      \n");
  printf("| %%%%o%%%%%%%%o%%%%%%(_|/%%o%%%%o%%%%%%%%o%%%%%% |                      \n");
  printf("| %%%%o%%(_)%%%%%%%%%%o%%(_)%%%%%%o%%%%o%%o%%%% |                      \n");
  printf("|  (_)%%%%=@%%(_)%%o%%o%%%%(_)%%o(_)%%  |                      \n");
  printf(" \\ ~%%%%o%%%%%%%%%%o%%o%%=@%%%%o%%%%@%%%%o%%~ /                       \n");
  printf("  \\. ~o%%%%(_)%%%%%%o%%(_)%%%%(_)o~ ,/   R.I.P. mod_spox      \n");
  printf("    \\_ ~o%%=@%%(_)%%o%%%%(_)%%~ _/                          \n");
  printf("      `\\_~~o%%%%%%o%%%%%%%%%%~~_/'                            \n");
  printf("         `--..____,,--'  CD                           \n");
  setvbuf(stdout, NULL, _IONBF, 0); /* unbuffer stdout */
  dict_load(DICT_FILE);
  do {
    printf("> ");
    if (NULL != (rd = fgets(line, sizeof line, stdin)))
      if (input_legit(line))
        dict_check(line);
      else
        printf("...");
  } while (rd);
  return 0;
}

