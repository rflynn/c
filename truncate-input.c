/* ex: set ts=2 et: */
/* $Id$ */
/* capture output from the mod_spox php invocation, causing php to end should it exceed the maximum output bytes */
/* NOTE: that this doesn't enforce any maximum number of lines, so the bot still needs to do that */
#include <stdio.h>
#include <string.h>
#define MAX_OUTPUT_BYTES 400
int main(void)
{
	unsigned long bytes,
	              total = 0;
	char cap[MAX_OUTPUT_BYTES];
	while (total < MAX_OUTPUT_BYTES && 0 != (bytes = fread(cap + total, 1, sizeof cap - total, stdin)))
		total += bytes;
	fclose(stdin);
	fwrite(cap, 1, total, stdout);
	return 0;
}
