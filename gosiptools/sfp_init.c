// N.Kurz, EE, GSI, 9-Mar-2009

// only for the moment...
#define GET_BAR0_BASE 0x1234

#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0"
#define PCI_BAR0_SIZE     0x800000  // in bytes

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
  int l_i, l_j;
  int l_stat;

  int l_bar0_base;
  volatile long *pl_virt_sram;

  long l_bar0_byte_off;
  long l_bar0_long_off;
  long l_bar1_byte_off;
  long l_bar1_long_off;



  long l_wr_dat; 
  long l_rw;
  long l_rd_dat;

  int  fd_pex;  // file descriptor for PEXOR device


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
  if((pl_virt_sram = (long *) smem_create (PCI_BAR0_NAME,
          (char*) l_bar0_base, PCI_BAR0_SIZE, SM_READ | SM_WRITE))==NULL)
  {
    perror("smem_create");
    printf("errno = %d\n",errno);
    exit (-1);
  }
  printf ("pl_virt_sram: 0x%x \n", pl_virt_sram); 


  ////////////////////////////////////////

  ////////////////////////////////////
  // check input paramters
/*   if ((argc < 2) || (argc > 3)) */
/*   { */
/*     printf ("\nusage: pex_bar0_rw \n"); */
/*     printf ("<bar0 offset (hex, in bytes)> \n"); */
/*     printf ("[write data (hex), if present: write, if missing: read> \n"); */
/*     exit (0); */
/*   } */
/*   else */
/*   { */
/*     sscanf (argv[1], "%x", &l_bar0_byte_off); */
/*     l_bar0_long_off = l_bar0_byte_off >> 2; */
/*     if (argc == 3) */
/*     { */
/*       l_rw = 1; // write */
/*       sscanf (argv[2], "%x", &l_wr_dat); */
/*     } */
/*     else */
/*     { */
/*       l_rw = 0; */
/*     } */
/*   } */



/////////////////////////////////
  // do read - or  write access
  //  if (l_rw == 0) // read from bar0
  //  {
  //    l_rd_dat = *(pl_virt_sram + l_bar0_long_off);
  //    printf ("READ: bar0 offset (bytes): 0x%x, data: 0x%x \n",
  //                                                   l_bar0_byte_off, l_rd_dat);
    //  } 
    //  if (l_rw == 1) // write to bar0
    //  {
    //  *(pl_virt_sram + l_bar0_long_off) = l_wr_dat;
    //    printf ("WRITE: bar0 offset (bytes): 0x%x, data: 0x%x \n",
    //                                                   l_bar0_byte_off, l_wr_dat);
    //  }


/////////////////////////////////
//  l_bar1_byte_off = 0x100108;
//  l_bar1_long_off = l_bar1_byte_off >> 2;
//  long l_pack_ad_r_req = 0x240;
//  long l_pack_ad_w_req = 0x644;
//  long l_pack_ini_req = 0x344;


	// open PEXOR device:

  // do read - or  write access
  //  if (l_rw == 0) // read from bar0
  //  {
  while(*(pl_virt_sram + l_rep_clr_long_off)!=0x0){

  *(pl_virt_sram + l_rep_clr_long_off)=0xf;
    printf ("CLEAR: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
  }

    //    printf ("CLEAR: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
    //                                                   l_rep_stat_3_long_off, *(pl_virt_sram + l_rep_stat_3_long_off));

    //    printf ("CLEAR: rep addr 3 (bytes): 0x%x, data: 0x%x \n",
    //                                                   l_rep_addr_3_long_off, *(pl_virt_sram + l_rep_addr_3_long_off));

    //    printf ("CLEAR: rep data 3 (bytes): 0x%x, data: 0x%x \n",
    //                                                   l_rep_data_3_long_off, *(pl_virt_sram + l_rep_data_3_long_off));


  while(*(pl_virt_sram + l_rep_clr_long_off)!=0x0){

  *(pl_virt_sram + l_rep_clr_long_off)=0xf;
    printf ("CLEAR: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
  }

  *(pl_virt_sram + l_req_addr_long_off)=0x1;
  *(pl_virt_sram + l_req_data_long_off)=0x3;
  *(pl_virt_sram + l_req_comm_long_off)=0x80000|l_pack_ini_req;


      while((*(pl_virt_sram + l_rep_stat_long_off) & 0x88 ) != 0x80){ 
	//     while((*(pl_virt_sram + l_rep_stat_long_off) && 0x80) != 0x80){ 
	printf ("STAT: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_long_off, *(pl_virt_sram + l_rep_stat_long_off) );
	
	  }  
    printf ("REP: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
    printf ("REP: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_3_long_off, *(pl_virt_sram + l_rep_stat_3_long_off));

    printf ("REP: rep addr 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_addr_3_long_off, *(pl_virt_sram + l_rep_addr_3_long_off));

    printf ("REP: rep data 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_data_3_long_off, *(pl_virt_sram + l_rep_data_3_long_off));


  while(*(pl_virt_sram + l_rep_clr_long_off)!=0x0){

  *(pl_virt_sram + l_rep_clr_long_off)=0xf;
    printf ("CLEAR: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
  }


  *(pl_virt_sram + l_rep_clr_long_off)=0xf;
  *(pl_virt_sram + l_req_addr_long_off)=0x1004;
  *(pl_virt_sram + l_req_data_long_off)=0x0;
  *(pl_virt_sram + l_req_comm_long_off)=0x80000|l_pack_ad_r_req;


      while((*(pl_virt_sram + l_rep_stat_long_off) & 0x88 ) != 0x80){ 
	//     while((*(pl_virt_sram + l_rep_stat_long_off) && 0x80) != 0x80){ 
	printf ("STAT: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_long_off, *(pl_virt_sram + l_rep_stat_long_off) );
	
	  }  
    printf ("REP: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
    printf ("REP: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_3_long_off, *(pl_virt_sram + l_rep_stat_3_long_off));

    printf ("REP: rep addr 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_addr_3_long_off, *(pl_virt_sram + l_rep_addr_3_long_off));

    printf ("REP: rep data 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_data_3_long_off, *(pl_virt_sram + l_rep_data_3_long_off));


  while(*(pl_virt_sram + l_rep_clr_long_off)!=0x0){

  *(pl_virt_sram + l_rep_clr_long_off)=0xf;
    printf ("CLEAR: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
  }

  *(pl_virt_sram + l_rep_clr_long_off)=0xf;
  *(pl_virt_sram + l_req_addr_long_off)=0x2004;
  *(pl_virt_sram + l_req_data_long_off)=0x0;
  *(pl_virt_sram + l_req_comm_long_off)=0x80000|l_pack_ad_r_req;


      while((*(pl_virt_sram + l_rep_stat_long_off) & 0x88 ) != 0x80){ 
	//     while((*(pl_virt_sram + l_rep_stat_long_off) && 0x80) != 0x80){ 
	printf ("STAT: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_long_off, *(pl_virt_sram + l_rep_stat_long_off) );
	
	  }  
    printf ("REP: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
    printf ("REP: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_3_long_off, *(pl_virt_sram + l_rep_stat_3_long_off));

    printf ("REP: rep addr 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_addr_3_long_off, *(pl_virt_sram + l_rep_addr_3_long_off));

    printf ("REP: rep data 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_data_3_long_off, *(pl_virt_sram + l_rep_data_3_long_off));



  *(pl_virt_sram + l_rep_clr_long_off)=0xf;
  *(pl_virt_sram + l_req_addr_long_off)=0x3004;
  *(pl_virt_sram + l_req_data_long_off)=0x0;
  *(pl_virt_sram + l_req_comm_long_off)=0x80000|l_pack_ad_r_req;


      while((*(pl_virt_sram + l_rep_stat_long_off) & 0x88 ) != 0x80){ 
	//     while((*(pl_virt_sram + l_rep_stat_long_off) && 0x80) != 0x80){ 
	printf ("STAT: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_long_off, *(pl_virt_sram + l_rep_stat_long_off) );	
	  }  
    printf ("REP: rep stat (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_clr_long_off, *(pl_virt_sram + l_rep_clr_long_off));
    printf ("REP: rep stat 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_stat_3_long_off, *(pl_virt_sram + l_rep_stat_3_long_off));

    printf ("REP: rep addr 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_addr_3_long_off, *(pl_virt_sram + l_rep_addr_3_long_off));

    printf ("REP: rep data 3 (bytes): 0x%x, data: 0x%x \n",
                                                   l_rep_data_3_long_off, *(pl_virt_sram + l_rep_data_3_long_off));





/*     while(l_rd_dat !=0xFB){ */
/*       *(pl_virt_sram + l_bar0_long_off) = 0x90;       */
/*       *(pl_virt_sram + l_bar0_long_off) = 0x80;       */
/*       *(pl_virt_sram + l_bar0_long_off) = 0x0;       */
/*       l_rd_dat = *(pl_virt_sram + l_bar1_long_off); */
/*       //      printf ("READ: bar0 offset (bytes): 0x%x, data: 0x%x \n", */
/*       //                                                   l_bar1_byte_off, l_rd_dat); */
/*     } */

/*     printf ("READ: bar0 offset (bytes): 0x%x, data: 0x%x \n", */
//                                                   l_bar0_byte_off, l_rd_dat);
    //  } 
    //  if (l_rw == 1) // write to bar0
    //  {
    //  *(pl_virt_sram + l_bar0_long_off) = l_wr_dat;
    //    printf ("WRITE: bar0 offset (bytes): 0x%x, data: 0x%x \n",
    //                                                   l_bar0_byte_off, l_wr_dat);
    //  }



}
