#include <stdio.h>
#include <stdlib.h>
 
static const char cmac[] = "0:c:6e:52:6:55";
 
void formatMac(const char *mac)
{
  int m[6] = { 0, 0, 0, 0, 0, 0 };
  if (6 != sscanf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", m+0, m+1, m+2, m+3, m+4, m+5)) {
    printf("malformed\n");
  } else {
    printf("%02x:%02x:%02x:%02x:%02x:%02x\n", m[0], m[1], m[2], m[3], m[4], m[5]);
  }
}
 
main(void)
{
  formatMac(cmac);
  return 0;
}
