/* ex: set ts=2 et: */
/*  */

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define WORKERS 4

#ifndef O_LARGEFILE
# define O_LARGEFILE 0
#endif

static int          Fd;
static struct stat  Stat;
static void        *Map;
static int          ShmId;
static unsigned     Workers = WORKERS;

static void logfile_close(void)
{
  if (-1 == shmctl(ShmId, IPC_RMID, NULL))
    perror("logfile_close shmctl");
  if (0 != close(Fd))
    perror("logfile_close close");
}

static void logfile_open(const char *filename)
{
  Fd = open(filename, O_LARGFILE);
  if (0 > Fd) {
    perror("open");
    exit(EXIT_FAILURE);
  }
  if (0 > stat(filename, &Stat)) {
    perror("stat");
    exit(EXIT_FAILURE);
  }
  Map = mmap(NULL, (size_t)Stat.st_size, 0, PROT_READ, MAP_SHARED, Fd, 0);
  if (NULL == Map) {
    perror("mmap");
    exit(EXIT_FAILURE);
  }
  ShmId = shmget((key_t)0, ShmBytes, IPC_CREAT);
  if (-1 == ShmId) {
    perror("shmget");
    exit(EXIT_FAILURE);
  }
  ShmPtr = shmat(ShmId, ptr, 0);
  if (NULL == ShmPtr) {
    perror("shmat");
    mem_close();
    exit(EXIT_FAILURE);
  }
}

static void workers_launch(unsigned cnt)
{
  
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "Usage: %s filename [#workers]\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  Workers = atoi(argv[2]);
  assert(Workers 
	return 0;
}

