// N.Kurz, EE, GSI, 30-Mar-2010
#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

main (int argc, char *argv[])
{
  int            l_ret;
  int            fd_mem; 

  size_t         len   = 0x40000000;
  off_t          off   = (off_t)0x80000000;
  int            prot  = PROT_WRITE | PROT_READ;
  int            flags = MAP_SHARED;
  char           *pa, *pe;


  fd_mem = open ("/dev/mem", O_RDWR);
  if (fd_mem < 0)
	{
    printf ("failed to open /dev/mem, exiting..\n");
    perror ("open");
    printf ("fd_mem: %d \n", fd_mem);
  }
  else
	{
    printf ("open /dev/mem ok \n");
  }

  //if ((pa = (char *) mmap (NULL, len, prot, flags, fd_mem, off)) < 0)
  if ((pa = (char *) mmap (NULL, len, prot, flags, fd_mem, off)) == MAP_FAILED)
  {
    printf ("failed to mmap, return: 0x%x, %d \n", pa, pa);
    perror ("mmap");
  }
  else
  {
    pe = pa + len - 1;
    printf ("physical address:             0x%x \n", off);
    printf ("size:                         0x%x \n", len);
    printf ("first mapped virtual address: 0x%x \n", pa);
    printf ("last  mapped virtual address: 0x%x \n", pe);
  }
}
