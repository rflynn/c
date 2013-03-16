/* ex: set ts=2 et: */
/* $Id$ */
/* Mersenne Twister algorithm */

#include <stdio.h>
#include <time.h>
#include "mt.h"

void mt_init(mt *m, int32_t seed)
{
	unsigned i;
	m->index = 0;
	m->mt[0] = seed;
	for (i = 1; i < MT_SIZE; i++)
		m->mt[i] = 0x6c078965L * (m->mt[i-1] ^ (m->mt[i-1] >> 30)) + 1;
}

static void generateNumbers(mt *m)
{
	unsigned i;
	for (i = 0; i < MT_SIZE; i++) {
		int32_t y = (0x80000000L & (uint32_t)m->mt) + (0x7FFFFFFFUL & (m->mt[i+1] % 624));
		m->mt[i] = m->mt[(i + 397) % 624] ^ (y >> 1);
		if (y & 0x1)
			m->mt[i] = m->mt[i] ^ 0x9908b0dfL;
	}
}

static int32_t mt_next(mt *m)
{
	int32_t y;
	if (0 == m->index)
		generateNumbers(m);
	m->index = (m->index + 1) % 624;
	y = m->mt[m->index];
	y = y ^  (y >> 11);
	y = y ^ ((y <<  7) & 0x9d2c5680L);
	y = y ^ ((y << 15) & 0xefc60000L);
	y = y ^  (y >> 18);
	return y;
}

#ifdef TEST
int main(void)
{
	mt m;
	unsigned i;
	mt_init(&m, time(NULL));
	for (i = 0; i < 100000000; i++)
		mt_next(&m);
	return 0;
}
#endif

