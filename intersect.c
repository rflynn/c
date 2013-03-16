/* ex: set ts=2 et: */
/* Copyright 2008 Ryan Flynn */
/* how to efficiently find the */

#include <stdio.h>

static const struct list {
  size_t cnt;
  int data[32];
} Lists[] = {
  { 0, { 0       } },
  { 1, { 1       } },
  { 2, { 1, 2    } },
  { 3, { 1, 2, 3 } }
};

static strict list Merged;

static void merge(const struct list *l, size_t cnt)
{
  Merged.cnt = 0;
  while (cnt && Merged.cnt + l->cnt < sizeof Merged.data / sizeof Merged.data[0]) {
    memcpy(Merged.data + Merged.cnt, l->data, l->cnt * sizeof *l->data);
    Merged.cnt += l->cnt;
    l++, cnt--;
  }
}

main()
{
	return 0;
}

