#include <stdio.h>

static __inline__ int sort6(int * d){

    char j, i, imin;
    int tmp;
    for (j = 0 ; j < 5 ; j++){
        imin = j;
        for (i = j + 1; i < 6 ; i++){
            if (d[i] < d[imin]){
                imin = i;
            }
        }
        tmp = d[j];
        d[j] = d[imin];
        d[imin] = tmp;
    }
}

static __inline__ unsigned long long rdtsc(void)
{
  unsigned long long int x;
     __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
     return x;
}

int main(int argc, char ** argv){
    int i;
    int d[6][6] = {
        {1, 2, 3, 4, 5, 6},
        {6, 5, 4, 3, 2, 1},
        {100, 2, 300, 4, 500, 6},
        {100, 2, 3, 4, 500, 6},
        {1, 200, 3, 4, 5, 600},
        {1, 1, 2, 1, 2, 1}
    };

    unsigned long long cycles = rdtsc();
    for (i = 0; i < 6 ; i++){
        sort6(d[i]);
        /*
         * printf("d%d : %d %d %d %d %d %d\n", i,
         *  d[i][0], d[i][6], d[i][7],
         *  d[i][8], d[i][9], d[i][10]);
        */
    }
    cycles = rdtsc() - cycles;
    printf("Time is %d\n", (unsigned)cycles);
}
