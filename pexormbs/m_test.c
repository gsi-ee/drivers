// N.Kurz, EE, GSI, 30-Mar-2010

// only for the moment:
#define WAIT_SEM              12
#define POLL_SEM              16
#define GET_BAR0_BASE       0x1234
#define GET_BAR0_TRIX_BASE  0x1235
#define RESET_SEM           0x1236

#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

main (int argc, char *argv[])
{
  int          l_status;
  int          fd_pex; 
  int          fd_mem; 
  int          flags;
  mode_t       mode;
  long         l_bar0; 
  long         l_trix_base;
  long         l_trix_val=0; 
  uid_t        uid;


  if ((fd_pex = open ("/dev/pexor", O_RDWR)) == -1)
  {
    printf ("ERROR>> open /dev/pexor \n");
  }

  // get BAR0:
  l_status = ioctl (fd_pex, GET_BAR0_BASE, &l_bar0);
  if (l_status == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_BASE failed \n");
	}
  else
  {
    printf ("BAR0:        0x%x \n", l_bar0); 
  }

  // get TRIXOR base:
  l_status = ioctl (fd_pex, GET_BAR0_TRIX_BASE, &l_trix_base);
  if (l_status == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_TRIX_BASE failed \n");
	}
  else
  {
    printf ("TRIXOR base: 0x%x \n", l_trix_base); 
  }

  // POLL_SEM:
  l_status = ioctl (fd_pex, POLL_SEM, &l_trix_val);
  if (l_status == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_TRIX_BASE failed \n");
	}
  else
  {
    printf ("trix_val: %d \n", l_trix_val); 
  }

  flags = O_RDWR; 
  mode  = S_IRWXO;
  fd_mem = open ("/dev/mem", flags);
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
}
