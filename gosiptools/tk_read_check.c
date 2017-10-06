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
#include <sys/types.h>
#include <sys/file.h>

#include "pexor_gosip.h"

main(argc,argv)
int argc;
char *argv[];
{
  int i,ii, j;
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

  long mem_size;
  long header_len, fotter_len, data_size_len;
  long header[64], footer[64], data_size[64];
  long mod_id[64]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		   -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
  long submem_id[64]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
		      -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

  long *pl_dat;
  long MEMSIZE=0x100000;
  long l_ad_mode_data, data_tmp;
  long header_cnt, data_size_cnt, footer_cnt, data_cnt, l_slave, footer_len;
  //
  long base_dbuf0, base_dbuf1;
  long num_submem, submem_offset;
  long dbuf_sel;

  struct timeb *tb_s, *tb_e;

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

/* #define REG_BUF0     0xFFFFD0 */
/* #define REG_BUF1     0xFFFFD4 */
/* #define REG_SUBMEM_NUM   0xFFFFD8 */
/* #define REG_SUBMEM_OFF   0xFFFFDC */


  pl_dat = (long*) malloc ( MEMSIZE );

  mem_size= PEXOR_TK_Mem_Read ( &sPEXOR_1, l_sfp, &pl_dat);
  printf("stored data size = 32bit wide x  0x%x \n", mem_size);

  //  loop = (mem_size >>2)+1;
  loop = (mem_size >>2);



  
//  long base_dbuf0, base_dbuf1;
//  long num_submem, submem_offset;
  if  (PEXOR_RX( &sPEXOR_1, l_sfp, &l_comm, &l_addr, &l_data )!=-1){
    //    if((l_comm&0xFFF)==PEXOR_PT_TK_R_REQ){
      printf("Token Loop with COMM = 0x%x Double BUF =0x%x  No. of SFP = 0x%x  \n", l_comm, l_addr, l_data );
      dbuf_sel=l_addr;
      //    }
  }
  
  if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_BUF0 , &base_dbuf0) !=-1){
    printf("Base address for Doulbe Buffer 0  0x%x  \n", base_dbuf0 );
  }
  if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_BUF1 , &base_dbuf1) !=-1){
    printf("Base address for Doulbe Buffer 1  0x%x  \n", base_dbuf1 );
  }
  if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_SUBMEM_NUM , &num_submem) !=-1){
    printf("Number of SubMemories  0x%x  \n", num_submem );
  }
  if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, REG_SUBMEM_OFF , &submem_offset) !=-1){
    printf("Offset of SubMemories to the Base address  0x%x  \n", submem_offset );
  }

  
  //  ftime(tb_s);
  /*
  for(i=0;i<loop;i++){
    printf("   addr %x  data size %x \n",i, *(pl_dat+i) );
  } 
  */
  j=0;
  header_len==0;
  if(loop!=0){
    for(i=0;i<loop;i++){

      for(ii=0;ii<4;ii++){

	if(ii==0){
	  data_tmp=*(pl_dat++);
	} else {
	  data_tmp=(data_tmp>>8);
	}

	if(mod_id[j]==-1 && header_len==0){
	  header_len=((data_tmp&0xf0)>>4);
	  //	  footer_len=((data_tmp&0x30)>>4);
	  data_size_len=(data_tmp&0xf);
	  header_cnt=0;
	  data_cnt=0;
	  data_size_cnt=0;
	  printf("head_len %x  dsize_len %x ", header_len, data_size_len);
	} else if (header_cnt < header_len) {
	  //		  printf("head  %x %x %x \n",j, data_tmp&0xff,header_cnt);
	  if(header_cnt==0)  printf("header  trig_id %x ", data_tmp&0xff);
	  if(header_cnt==1)  printf("mod_id %x ", data_tmp&0xff);
	  if(header_cnt==2)  printf("submem_id  %x ", data_tmp&0xff);
	  if(header_cnt==1) mod_id[j]=data_tmp&0xff;
	  if(header_cnt==2) submem_id[j]=data_tmp&0xff;
	  header_cnt++;

	} else if (data_size_cnt < data_size_len) {
	  //		  printf("data_size  %x %x %x \n",j, data_tmp&0xff,data_size_cnt);
	  data_size[j]=( (data_tmp&0xff)<<(data_size_cnt*8) ) |data_size[j];
	  data_size_cnt++;
	  if(data_size_cnt==3)  printf("data_size  %x \n", data_size[j]);
	} else if ((data_size[j]!=0) && (data_cnt < data_size[j])) {

	  if( (data_cnt&0x3) == 0 ){
	    l_data=data_tmp&0xff;
	  } else if ( (data_cnt&0x3) == 1 ){
	    l_data=(((data_tmp&0xff)<<8)|l_data);
	  } else if ( (data_cnt&0x3) == 2 ){
	    l_data=(((data_tmp&0xff)<<16)|l_data);
	  }  else if ( (data_cnt&0x3) == 3 ){
	    l_data=(((data_tmp&0xff)<<24)|l_data);
	  }

	  if( (data_cnt&0x3) == 0x3  ){
	    l_slave=mod_id[j];
	    if(dbuf_sel==0){
	      l_addr= ( data_cnt & 0xfffffffc)+base_dbuf0+submem_offset*submem_id[j];
	      //	      printf("        submem_offset %x   submem_id[j] %x       addr %x   \n", submem_offset, submem_id[j], l_addr );

	    }else{
	      l_addr= ( data_cnt & 0xfffffffc)+base_dbuf1+submem_offset*submem_id[j];
	    }
	    if ( PEXOR_Slave_Read( &sPEXOR_1, l_sfp, l_slave, l_addr, &l_ad_mode_data) !=-1){
	      //	      printf("Data : slave %x addr %x  TK: %x  AD: %x \n", l_slave, l_addr, l_data ,l_ad_mode_data );
	      if(l_ad_mode_data != l_data)
		printf("Data error: slave %x addr %x  TK: %x  AD: %x \n", l_slave, l_addr, l_data ,l_ad_mode_data );
	    } else {
	      printf("no reply : slave %x addr %x  TK: %x  AD: %x \n", l_slave, l_addr, l_data ,l_ad_mode_data );
	    }
	  }
	  data_cnt++;

	  //	  printf("data_size %x  data_cnt %x \n ", data_cnt, data_size[j]);
	  if(data_cnt==data_size[j]){
	    //	  printf("in !! %x \n");
	    header_len=0;
	    j++;
	  }
	  //	} else if ((footer_len!=0) && (footer_cnt < footer_len) ) {
	  //	  footer_cnt++;

	} else if (data_size[j]==0) {
	  //	    header_len=0;
	  j++;
	  header_len=((data_tmp&0xf0)>>4);
	  //	  footer_len=((data_tmp&0x30)>>4);
	  data_size_len=(data_tmp&0xf);
	  header_cnt=0;
	  data_cnt=0;
	  data_size_cnt=0;
	  printf("data_size[j]==0 %x \n");
	  printf("head_len %x  dsize_len %x ", header_len, data_size_len);
	  //	  printf("size  %x %x %x %x \n", data_tmp, j, header_len, data_size_len);
	} else {
	  printf("why?  \n" );
	}
      }
    }
  }
  printf("data size %x x 4 bytes header %x  \n",i, *(pl_dat+i) );

  //  ftime(tb_e);

  //  printf("start  %d  end %d \n",tb_s->millitm, tb_e->millitm ); 
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
