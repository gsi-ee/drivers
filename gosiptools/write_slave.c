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
#include <sys/mman.h>
#include <sys/file.h>
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
  long l_sfp, l_slave, l_address,l_comm,l_data;

  int  fd_pex;  // file descriptor for PEXOR device
  long a,b,c;
  long aa;
  long modid;
  
#ifndef GSI__LINUX
  smem_remove(PCI_BAR0_NAME);
#endif  
	// open PEXOR device:

  if (argc!=5){
    printf ("\nusage: read_slave ");
    printf ("sfp[0-3] slave[0-3] address[0-FFFFFC] data[32bit]\n");
    exit (0);
  }  else  {
    sscanf (argv[1], "%x", &l_sfp);
    sscanf (argv[2], "%x", &l_slave);
    sscanf (argv[3], "%x", &l_address);
    sscanf (argv[4], "%x", &l_data);
  }

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

  PEXOR_GetPointer(0x0, (long*) pl_virt_bar0, &sPEXOR_1); 

  l_comm = PEXOR_PT_AD_W_REQ | (0x1<<16+l_sfp);
  l_address = l_address+ (l_slave << 24);

  PEXOR_RX_Clear(&sPEXOR_1); 
  PEXOR_TX(&sPEXOR_1,  l_comm, l_address, l_data) ;


  if(PEXOR_RX(&sPEXOR_1, l_sfp, &a , &b, &c)==-1) {
    printf ("no reply: 0x%x 0x%x 0x%x \n", a,b,c);
  }else{
    printf ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
#ifdef DEBUT
    printf (" 0x%x 0x%x 0x%x \n", a,b,c);
#endif
    if( (a&0xfff) == PEXOR_PT_AD_W_REP){
      printf ("Module: 0x%x Address: 0x%x  \n", ( b&0xff000000) >> 24 , b&0xffffff );
      if(a&0x4000!=0){
	printf ("ERROR: Packet Structure : Command Reply 0x%x \n", a);
      }

    } else {
      printf ("ERROR : Access to empty slave or address: ");
      printf (" 0x%x Address 0x%x  Command Reply  0x%x \n", ( l_address&0xff000000) >> 24 , l_address&0xffffff, a );
    }
  }
  return 0;
}
