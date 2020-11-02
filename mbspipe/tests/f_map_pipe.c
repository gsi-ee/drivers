#include "f_map_pipe.h"

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

int* f_map_pipe (unsigned long physbase, unsigned long maplen)
{
  // this part is directly taken from f_smem.c of MBS, reduced to IFC case
  int* mapbase = 0;
  int fd;
  char *pa;
  size_t len;
  int prot;
  int flags;
  off_t off;

  fd = open ("/dev/mbspipe", O_RDWR);
  if (fd < 0)
  {
    printf ("ERROR>> failed to open /dev/mbspipe for shared segement mmap mapping, exiting.. errno is:%d (%s)\n", errno,
        strerror (errno));
    return 0;
  }
  prot = PROT_READ | PROT_WRITE;
  off = physbase;
  len = maplen;
  flags = MAP_SHARED;
  pa = mmap (NULL, len, prot, flags, fd, off);
  if (pa == (char*) (-1))
  {
    printf ("ERROR>> failed to mmap, errno is:%d (%s) \a\a", errno, strerror (errno));
    return 0;

  }
  mapbase = (int*) pa;
  return mapbase;
}
