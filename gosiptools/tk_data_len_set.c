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

  int  fd_pex;  // file descriptor for PEXOR device
  long a,b,c;
  long aa;
  long modid,if_last_module,loop;
  long l_address, l_comm,l_comm_write, l_sfp, l_slave;
  long LOOPMAX=0x10;

  long l_buf, l_submem;
  long data_len[8] ={0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10};

  int dlen_flag;

  smem_remove(PCI_BAR0_NAME);
	// open PEXOR device:

  if (argc >= 6){
    sscanf (argv[1], "%x", &l_sfp);
    sscanf (argv[2], "%x", &l_slave);
    sscanf (argv[3], "%x", &l_buf);
    sscanf (argv[4], "%x", &data_len[0]);
    sscanf (argv[5], "%x", &data_len[1]);
    sscanf (argv[6], "%x", &data_len[2]);
    sscanf (argv[7], "%x", &data_len[3]);
    sscanf (argv[8], "%x", &data_len[4]);
    sscanf (argv[9], "%x", &data_len[5]);
    sscanf (argv[10], "%x", &data_len[6]);
    sscanf (argv[11], "%x", &data_len[7]);
  }else{
    printf ("\nusage: sumemory data length set \n ");
    printf ("sfp[0-3] ");
    printf ("modid ");
    printf ("buffer[0/1] ");
    //    printf ("SubMemoryID ");
    printf ("DataLengh_0 1 2 3 4 5 6 7 \n");
    exit (0);
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

  // get bar0 base:
  l_stat = ioctl (fd_pex, GET_BAR0_BASE, &l_bar0_base);
  if (l_stat == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_BASE failed \n");
	}
  else
  {
#ifdef DEBUG    
    printf ("PEXOR bar0 base: 0x%x \n", l_bar0_base);
#endif
  } 

  // open shared segment
  if((pl_virt_sram = (long *) smem_create (PCI_BAR0_NAME,
          (char*) l_bar0_base, PCI_BAR0_SIZE, SM_READ | SM_WRITE))==NULL)
  {
    perror("smem_create");
    printf("errno = %d\n",errno);
    exit (-1);
  }
#ifdef DEBUG    
  printf ("pl_virt_sram: 0x%x \n", pl_virt_sram); 
#endif


  PEXOR_GetPointer(0x0, pl_virt_sram, &sPEXOR_1); 
  l_comm = PEXOR_PT_AD_W_REQ | (0x1<<16+l_sfp);
  if(l_buf==0){
    l_address = REG_BUF0_DATA_LEN;
  } else {
    l_address = REG_BUF1_DATA_LEN;
  }

  for ( i=0; i < 8; i++){

    if ( PEXOR_Slave_Write( &sPEXOR_1, l_sfp, l_slave, l_address, data_len[i]) ){
      //    printf ("REG_MODID: 0x%x \n", c);
    } else {
      printf ("PEXOR_Slave_READ:errorx \n");
    }
    l_address = l_address+4;
  }

}
