#include <stdio.h>
#define VALUE1    0x0001
#define VALUE2    0x0002
#define VALUE3    0x0004
int main(void) {
  int val = VALUE1 | VALUE2; /* initially */
  val |= VALUE3; /* turn VALUE4 on, but keep other flags as well */
  val &= ~VALUE1; /* turn VALUE1 off but keep other flags */
  /* check a single value */
  if (val & VALUE3)
    printf("VALUE3 is definitely set!\n");
  { /* check the status of all flags... */
    int i, flag;
    static const char *Status[2] = { "off", "on" };
    printf("checking status...\n");
    for (i = 1, flag = VALUE1; flag <= VALUE3; flag <<= 1, i++)
      printf("VALUE%d is %s\n", i, Status[!!(val & flag)]);
  }
  return 0;
}

