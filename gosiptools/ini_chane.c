// N.Kurz, EE, GSI, 9-Mar-2009
// N.Kurz, EE, GSI, 1-Mar-2013: adoped for linux

//#define DEBUG
#define GET_BAR0_BASE 0x1234

#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0"
#define PCI_BAR0_SIZE     0x800000  // in bytes

#include <stdio.h>
#include <errno.h>
#include <smem.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "pexor_gosip.h"

int main(argc,argv)
int argc;
char *argv[];
{
  int i,j;
  int l_i, l_j;
  int l_stat;

  int l_bar0_base;
  volatile long *pl_virt_bar0;

  int            prot;
  int            flags;

  long l_bar0_byte_off;
  long l_bar0_long_off;
  long l_bar1_byte_off;
  long l_bar1_long_off;

  s_pexor sPEXOR_1;

  long l_wr_dat; 
  long l_rw;
  long l_rd_dat;
  long l_sfp, l_slave, l_last,l_comm;
  int  fd_pex;  // file descriptor for PEXOR device
  long a,b,c;
  long aa;
  
#ifndef GSI__LINUX  
  smem_remove(PCI_BAR0_NAME);
#endif  
	// open PEXOR device:


  if (argc!=3){
    printf ("\nusage: ini_chane ");
    printf ("sfp [0-3] No. of slave modules [1-16] \n");
    exit (0);
  }  else  {
    sscanf (argv[1], "%x", &l_sfp);
    sscanf (argv[2], "%x", &l_slave);
  }


  if ((fd_pex = open (PEXDEV, O_RDWR)) == -1)
  {
    printf ("ERROR>> could not open %s device \n", PEXDEV);
    exit (0);
  }
  else
  {
    printf ("opened device: %s, fd = %d \n", PEXDEV, fd_pex);
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
   // get bar0 base:#include <stdlib.h>
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

  l_comm = PEXOR_INI_REQ| (0x1<<16+l_sfp);

  PEXOR_GetPointer(0x0, (long*) pl_virt_bar0, &sPEXOR_1); 
  //  for(j=0;j<1;j++){

  l_slave = l_slave - 1;
  PEXOR_RX_Clear_Ch(&sPEXOR_1, l_sfp); 
  //  PEXOR_RX_Clear(&sPEXOR_1); 
  PEXOR_TX(&sPEXOR_1, l_comm, 0, l_slave) ;
  printf (" SFP%d: ",l_sfp);
  if(PEXOR_RX(&sPEXOR_1, l_sfp, &a , &b, &c)==-1) {
    printf ("no reply: 0x%x 0x%x 0x%x \n", a,b,c);
  }else{
    printf ("Reply: 0x%x 0x%x 0x%x \n", a,b,c);
    printf ("     : No of slaves  : 0x%x\n", b);
  }

  return 0;
}
