// N.Kurz, EE, GSI, 9-Mar-2009

// only for the moment...
//#define DEBUG
#define GET_BAR0_BASE 0x1234

#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0"
#define PCI_BAR0_SIZE     0x800000  // in bytes
//#define PCI_BASE     0x000000  

#include <stdio.h>
#include <errno.h>
#include <smem.h>
//#include <timeb.h>
#include <sys/file.h>

#include "pexor_gosip.h"

main(argc,argv)
int argc;
char *argv[];
{
  int i,j;
  int l_i, l_j;
  int l_stat;

  int l_bar0_base;
  long *pl_virt_sram;

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
  int flag;
  smem_remove(PCI_BAR0_NAME);
	// open PEXOR device:


  if ((fd_pex = open (PEXDEV, O_RDWR)) == -1)
  {
    printf ("ERROR>> could not open %s device \n", PEXDEV);
    exit (0);
  }
  else
  {
    printf ("opened device: %s, fd = %d \n", PEXDEV, fd_pex);
  }

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
  if((pl_virt_sram = (long *) smem_create (PCI_BAR0_NAME,
          (char*) l_bar0_base, PCI_BAR0_SIZE, SM_READ | SM_WRITE))==NULL)
  {
    perror("smem_create");
    printf("errno = %d\n",errno);
    exit (-1);
  }
  printf ("pl_virt_sram: 0x%x \n", pl_virt_sram); 

  l_comm = PEXOR_INI_REQ| (0x1<<16+l_sfp);

  PEXOR_GetPointer(0x0, pl_virt_sram, &sPEXOR_1); 
  //  for(j=0;j<1;j++){

  PEXOR_Port_Monitor(&sPEXOR_1); 

}
