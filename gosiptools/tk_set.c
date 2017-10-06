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


  long header = 0x11; // header length 1 , header is mod id
  long footer = 0x00; // footer length 1 , footer is FF

   //   long data_len[4] = {0x4000-0x20, 0x19, 0x20, 0x3}; 
   long data_len[16] = {0x10, 0x10, 0x10, 0x10,0x10, 0x10, 0x10, 0x10,0x10, 0x10, 0x10, 0x10,0x10, 0x10, 0x10, 0x10}; 
   int dlen_flag;

  smem_remove(PCI_BAR0_NAME);
	// open PEXOR device:

  if (argc==1||argc>18){
    printf ("\nusage: token mode setting ");
    printf ("sfp[0-3] \n");
    printf ("data length set [on=1/off=0] for mod_id_0, 1,2,3 [0x0 - 0x3fe0] \n");
    exit (0);
  }  else if(argc==2)  {
    sscanf (argv[1], "%x", &l_sfp);
    dlen_flag=0;
  }  else if(argc>2)  {
    sscanf (argv[1], "%x", &l_sfp);
    sscanf (argv[2], "%x", &dlen_flag);
    sscanf (argv[3], "%x", &data_len[0]);
    sscanf (argv[4], "%x", &data_len[1]);
    sscanf (argv[5], "%x", &data_len[2]);
    sscanf (argv[6], "%x", &data_len[3]);
    sscanf (argv[7], "%x", &data_len[4]);
    sscanf (argv[8], "%x", &data_len[5]);
    sscanf (argv[9], "%x", &data_len[6]);
    sscanf (argv[10], "%x", &data_len[7]);
    sscanf (argv[11], "%x", &data_len[8]);
    sscanf (argv[12], "%x", &data_len[9]);
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
  l_comm = PEXOR_PT_AD_R_REQ | (0x1<<16+l_sfp);
  l_comm_write = PEXOR_PT_AD_W_REQ | (0x1<<16+l_sfp);

  if_last_module=0;
  loop=0;

  while(if_last_module==0&&(loop<LOOPMAX)){
    l_slave=loop;
    if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_MODID, &c) ==-1){
      printf ("PEXOR_Slave_READ:errorx \n");
    }
    if_last_module=c&0x100;

/*     if ( PEXOR_Slave_Write( &sPEXOR_1, l_sfp, l_slave, REG_HEADER, header)==-1 ){ */
/*       printf ("PEXOR_Slave_READ:errorx \n"); */
/*     } */
/*     if ( PEXOR_Slave_Write( &sPEXOR_1, l_sfp, l_slave, REG_FOOTER, footer)==-1 ){ */
/*       printf ("PEXOR_Slave_READ:errorx \n"); */
/*     } */

    if(dlen_flag==1){
      if ( PEXOR_Slave_Write( &sPEXOR_1, l_sfp, l_slave, REG_DATA_LEN, data_len[loop])==-1 ){
	printf ("PEXOR_Slave_READ:errorx \n");
      }
    }
    loop++;
  }    

  if_last_module=0;
  loop=0;

  while(if_last_module==0&&(loop<LOOPMAX)){

    l_slave=loop;
    if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_MODID, &c) ){
      printf ("REG_MODID: 0x%x \n", c);
    } else {
      printf ("PEXOR_Slave_READ:errorx \n");
    }
    if_last_module=c&0x100;
    

/*     if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_HEADER, &c) ){ */
/*       printf ("REG_HEADER: 0x%x \n", c); */
/*     } else { */
/*       printf ("PEXOR_Slave_READ:errorx \n"); */
/*     } */
/*     if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_FOOTER, &c) ){ */
/*       printf ("REG_FOOTER: 0x%x \n", c); */
/*     } else { */
/*       printf ("PEXOR_Slave_READ:errorx \n"); */
/*     } */

    if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_DATA_LEN, &c) ){
      printf ("REG_DATA_LEN: 0x%x \n", c);
    } else {
      printf ("PEXOR_Slave_READ:errorx \n");
    }

    loop++;
  }    


}
