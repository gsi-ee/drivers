// N.Kurz, EE, GSI, 9-Mar-2009
// N.Kurz, EE, GSI, 1-Mar-2013: adoped for linux

#define GET_BAR0_BASE 0x1234

//#define DEBUG
#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0"
#define PCI_BAR0_SIZE     0x800000  // in bytes

#include <stdio.h>
#include <errno.h>
#include <smem.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <stdlib.h>

int main(argc,argv)
int argc;
char *argv[];
{
  int l_i, l_j;
  int l_stat;

  int l_bar0_base;
  volatile long *pl_virt_bar0=0;

  int            prot;
  int            flags;

  long l_bar0_byte_off=0;
  long l_bar0_long_off=0;
  long l_wr_dat; 
  long l_rw;
  long l_rd_dat;

  int  fd_pex;  // file descriptor for PEXOR device

  // check input paramters
  if ((argc < 2) || (argc > 3))
  {
    printf ("\nusage: pex_bar0_rw \n");
    printf ("<bar0 offset (hex, in bytes)> \n");
    printf ("[write data (hex), if present: write, if missing: read> \n");
    exit (0);
  }
  else
  {
    sscanf (argv[1], "%x", &l_bar0_byte_off);
#ifdef DEBUG  
    printf ("Got byte address offset: 0x%x \n",l_bar0_byte_off);
#endif    
    l_bar0_long_off = (l_bar0_byte_off >> 2);
#ifdef DEBUG     
    printf ("Got long address offset: 0x%x \n",l_bar0_long_off);
#endif    
    if (argc == 3)
    {
      l_rw = 1; // write
      sscanf (argv[2], "%x", &l_wr_dat);
    }
    else
    {
      l_rw = 0;
    }
  }

	// open PEXOR device:
  if ((fd_pex = open (PEXDEV, O_RDWR)) == -1)
  {
    printf ("ERROR>> could not open %s device \n", PEXDEV);
    exit (0);
  }
  else
  {
#ifdef DEBUG
    printf ("opened device: %s, fd = %d \n", PEXDEV, fd_pex);
#endif
  }

#ifdef GSI__LINUX
  // map bar0 directly via pexor driver and access trixor base
  prot  = PROT_WRITE | PROT_READ;
  flags = MAP_SHARED | MAP_LOCKED;
  if ((pl_virt_bar0 = (long *) mmap (NULL, PCI_BAR0_SIZE, prot, flags, fd_pex, 0)) == MAP_FAILED)
  {
    printf ("failed to mmap bar0 from pexor, return: 0x%x, %d \n", pl_virt_bar0, pl_virt_bar0);
    perror ("mmap"); 
	  exit (-1);
  } 
#ifdef DEBUG
  printf ("first mapped virtual address of bar0: 0x%p \n", pl_virt_bar0);
#endif // DEBUG

#else // GSI__LINUX
  // get bar0 base:
  l_stat = ioctl (fd_pex, GET_BAR0_BASE, &l_bar0_base);
  if (l_stat == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_BASE failed \n");
  }
  else
  {
    printf ("PEXOR bar0 base: 0x%x \n", l_bar0_base);
  } 
  // open shared segment
  smem_remove(PCI_BAR0_NAME);
  if((pl_virt_bar0 = (long *) smem_create (PCI_BAR0_NAME,
            (char*) l_bar0_base, PCI_BAR0_SIZE, SM_READ | SM_WRITE))==NULL)
  {
      printf ("smem_create for PEXOR BAR0 failed");
      exit (-1);
  }
#endif // GSI__LINUX

  // do read - or  write access
  if (l_rw == 0) // read from bar0
  {
    l_rd_dat = *((int*) pl_virt_bar0 + l_bar0_long_off);
    // JAM2017: below does not give correct address! need cast from volatile
    //l_rd_dat = *(pl_virt_bar0 + l_bar0_long_off);
    
    printf ("READ: bar0 offset (bytes): 0x%lx, data: 0x%lx \n",
                                                   l_bar0_byte_off, l_rd_dat);
  } 
  if (l_rw == 1) // write to bar0
  {
    //*(pl_virt_bar0 + l_bar0_long_off) = l_wr_dat;    
    *((int*) pl_virt_bar0 + l_bar0_long_off) = l_wr_dat;  
    printf ("WRITE: bar0 offset (bytes): 0x%x, data: 0x%x \n",
                                                   l_bar0_byte_off, l_wr_dat);
  }
  
  return 0;
}
