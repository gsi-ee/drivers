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
  long l_sfp, l_comm, l_addr, l_data, l_rdata;
  long addr_i, data_i;
  int loop;

  int  fd_pex;  // file descriptor for PEXOR device
  long a,b,c;
  long aa;
  long modid;
  
#ifndef GSI__LINUX
  smem_remove(PCI_BAR0_NAME);
#endif
  // open PEXOR device:

  if (argc!=2){
    printf ("\nusage: write ");
    printf ("ini data \n");
    exit (0);
  }  else  {
    //    sscanf (argv[1], "%x", &l_addr);
    sscanf (argv[2], "%x", &l_data);
    //    sscanf (argv[3], "%x", &loop);
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


  PEXOR_GetPointer(0x0, (long *) pl_virt_bar0, &sPEXOR_1); 
  //  l_comm = 0x844| (0x1<<16+l_sfp);
  addr_i = l_addr;
  data_i = l_data;
  //  printf ("command address data : 0x%x 0x%x 0x%x  \n", l_comm, l_addr, l_data);

  l_addr=0;

  // packet type
  l_data=PEXOR_PT_TK_R_REP; 
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);
  l_addr=l_addr+0x4;

  // header length 2, footer length 2, data size length 2
  l_data=0x2202; // If data length is less than 4096, all size is enough!!
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);
  l_addr=l_addr+0x4;

  // USER DODE
  l_data=0xACDCCAFE;
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);
  l_addr=l_addr+0x4;

  // SLAVE MOD ID
  //  l_addr++;
  l_data=0x3;
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);
  l_addr=l_addr+0x4;

  // DATA SIZE 0 -- 1024 0x400
  //  l_addr++;
  l_data=0x0;
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);
  l_addr=l_addr+0x4;

  // DATA SIZE 1 -- 1024 0x400
  //  l_addr++;
  l_data=0x4;
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);
  l_addr=l_addr+0x4;

  //  loop=0x400;
  loop=0x8;
  //  l_addr= addr_i;
  l_data=data_i;
  for(i=0;i<loop;i++){
    PEXOR_TK_Mem_Write (&sPEXOR_1, &l_addr,&l_data);
    l_addr=l_addr+0x4;
    l_data=l_data+0x1;
  }

  // Footer
  //  l_addr++;
  //  l_addr=l_addr+0x4;
  l_data=0x3;
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);
  l_addr=l_addr+0x4;

  // Footer
  //  l_addr++;
  l_data=0xCAFEACDC;
  PEXOR_TK_Mem_Write ( &sPEXOR_1, &l_addr,&l_data);

  l_addr= 0;
  for(i=0;i<loop+0x10;i++){
    PEXOR_TK_Mem_Read ( &sPEXOR_1, (long) &l_addr,(long**) &l_rdata);
    printf ("address data : 0x%x 0x%x  \n", l_addr, l_rdata);
    l_addr=l_addr+0x04;
    // l_data=l_data+0x1;
  }

  l_sfp=2;
  //  l_addr=1024;
  //  l_addr=2;
  l_addr=10;
  l_comm = 0x844| (0x1<<16+l_sfp);
  //  l_comm = 0x44| l_sfp;

  printf ("command address data : 0x%x 0x%x 0x%x  \n", l_comm, l_addr, l_data);
  PEXOR_TX(&sPEXOR_1,  l_comm, l_addr, l_data) ;

  return 0;
}
