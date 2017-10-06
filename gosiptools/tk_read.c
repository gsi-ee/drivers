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
  long l_address, l_comm,l_comm_write, l_sfp;
  long LOOPMAX=0x10;
  long l_addr,l_data;

  long header = 0x00000011;
  long footer = 0x0000ED01;
  long data_len = 0x000010;
  long data_size[16];
  long data_size_sum;
  smem_remove(PCI_BAR0_NAME);
	// open PEXOR device:

  if (argc!=2){
    printf ("\nusage: token mode read data ");
    printf ("sfp[0-3] \n");
    exit (0);
  }  else  {
    sscanf (argv[1], "%x", &l_sfp);
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

  data_size_sum= PEXOR_TK_Mem_Size ( &sPEXOR_1, l_sfp);
  printf("stored data size = 32bit wide x  0x%x \n", data_size_sum);

  for(i=0; i<16;i++){
    data_size[i]= PEXOR_TK_Data_Size ( &sPEXOR_1, l_sfp,  i);
    printf(" id %x  data size %x \n",i, data_size[i]);
  }

  loop=(data_size_sum>>2)+1;
  for(i=0;i<loop;i++){
    l_addr = 0x100000+0x40000*l_sfp+i*0x4;
    PEXOR_Read (  &sPEXOR_1, &l_addr, &l_data);
    printf("   addr %x  data size %x \n",l_addr,l_data );
  }



  /*
  l_comm = PEXOR_PT_TK_R_REQ | (0x1<<16+l_sfp);

  if_last_module=0;
  loop=0;

  //  while(1){
  PEXOR_RX_Clear_Ch(&sPEXOR_1, l_sfp); 
  PEXOR_TX(&sPEXOR_1, l_comm, 0x0, 0x0) ;
  //  sleep(2);
  
  if(PEXOR_RX(&sPEXOR_1, l_sfp, &a , &b, &c)==-1) {
    printf ("no reply: 0x%x 0x%x 0x%x \n", a,b,c);
  }else{
    printf ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
    //#ifdef DEBUT
      printf (" 0x%x 0x%x 0x%x \n", a,b,c);
      //#endif
  }
  //  }

  */
}
