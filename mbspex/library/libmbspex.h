#ifndef _PEX_LIBMBSPEX_H_
#define _PEX_LIBMBSPEX_H_

#include "../include/pex_user.h"

/* enable this define if you link against mbs libraries */
/*#define MBSPEX_USEMBS 1*/

#define mbspex_assert_handle(handle)                                    \
  if(handle < 0)                                  \
    {                                                                   \
      printm("Error: illegal file handle %d \n", \
                handle);                            \
      return -1;                                                   \
    }

/* open file handle of pex device number devnum. Return value is handle*/
int mbspex_open (int devnum);

/* close file handle*/
int mbspex_close (int handle);

/* read data word *l_dat from sfp, slave and memory offset l_slave_off*/
int mbspex_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat);

/* write data word l_dat to sfp, slave and memory offset l_slave_off*/
int mbspex_slave_wr (int handle, long l_sfp, long l_slave, long l_slave_off, long l_dat);

/* initialize chain at l_sfp with l_s_slaves number of slaves*/
int mbspex_slave_init (int handle, long l_sfp, long l_n_slaves);

/* send token request to pexor device of handle at chain sfp
 *  with l_toggle word (sets frontend buffer)
 * l_ldma_target specifies physical address of target buffer for token data DMA
 * returns some result check words:
 * pl_transfersize:  size of transferred dma in bytes
 * pl_check_comm: l_comm
 * pl_check_token: toggle and mode bits
 * pl_check_slaves: nr. of slaves connected to token chain
 */
int mbspex_send_and_receive_tok (int handle, long l_sfp, long l_toggle, unsigned long l_dma_target,
    unsigned long* pl_transfersize, long *pl_check_comm, long *pl_check_token, long *pl_check_slaves);

/* sends token to all SFPs of pexor device handle,
 * marked bitwise in l_sfp_p pattern: 1: sfp 0, 2: sfp 1, 4: sfp 2, 8: sfp 3, 0xf: all four SFPs
 * toggle specifies */
int mbspex_send_tok (int handle, long l_sfp_p, long l_toggle);

/* receive token data from l_sfp after previous request from pexor device handle
 *  l_ldma_target specifies physical address of target buffer for token data DMA
 * returns some result check words:
 * pl_transfersize:  size of transferred dma in bytes
 * pl_check_comm: l_comm
 * pl_check_token: toggle bit
 * pl_check_slaves: nr. of slaves connected to token chain
 */
int mbspex_receive_tok (int handle, long l_sfp, unsigned long l_dma_target, unsigned long *pl_transfersize,
    long *pl_check_comm, long *pl_check_token, long *pl_check_slaves);

/*void f_feb_init ();*/

//////////////////////////////////////////////////////////////////////////////////////////////////////

///* JAM: TODO check which of these methods we really need ->*/
//
//
//int PEXOR_Read( s_pexor *ps_pexor, long *addr, long *data ){
//
//  *data =  * ( ps_pexor->pexor_base + (*addr >>2) );
//  //  printf ("PEXOR_Read: 0x%x  0x%x \n",(ps_pexor->pexor_base),*(ps_pexor->pexor_base) );
//  //  printf ("PEXOR_Read: 0x%x  0x%x \n",(ps_pexor->pexor_base+(addr>>2)),*(ps_pexor->pexor_base+(addr>>2) ) );
//}
//
//
//int PEXOR_Slave_Read( s_pexor *ps_pexor, long l_sfp, long l_slave, long l_addr, long *data ){
//  long l_comm, l_address;
//  long a,b,c;
//  int status;
//
//  status=1;
//  l_comm = PEXOR_PT_AD_R_REQ | (0x1<<16+l_sfp);
//  l_address = l_addr+ (l_slave << 24);
//
//  PEXOR_RX_Clear_Ch(ps_pexor, l_sfp);
//
//  PEXOR_TX( ps_pexor, l_comm, l_address, 0x0) ;
//  if(PEXOR_RX( ps_pexor, l_sfp, &a , &b, &c)==1) {
//#ifdef DEBUT
//    printf ("PEXOR_Slave_Read: Reply to PEXOR from SFP: 0x%x ", l_sfp);
//    printf (" 0x%x 0x%x 0x%x \n", a,b,c);
//#endif
//    *data = c;
//    if( (a&0xfff) == PEXOR_PT_AD_R_REP){
//      if(a&0x4000!=0){
//	printf ("PEXOR_Slave_Read: ERROR: Packet Structure : Command Reply 0x%x \n", a);
//	status=-1;
//      }
//    } else {
//      printf ("PEXOR_Slave_Read: ERROR : Access to empty slave or address: Module  0x%x Address 0x%x  Command Reply  0x%x \n", ( l_address&0xff000000) >> 24 , l_address&0xffffff, a );
//      status=-1;
//    }
//  }else{
//    status=-1;
//    printf ("PEXOR_Slave_Read: no reply: 0x%x 0x%x 0x%x \n", a,b,c);
//  }
//
//  return status;
//}
//
//
//int PEXOR_Slave_Write( s_pexor *ps_pexor, long l_sfp, long l_slave, long l_addr, long l_data ){
//  long l_comm, l_address;
//  long a,b,c;
//  int status;
//
//  status=1;
//  l_comm = PEXOR_PT_AD_W_REQ | (0x1<<16+l_sfp);
//  l_address = l_addr+ (l_slave << 24);
//
//  PEXOR_RX_Clear(ps_pexor);
//
//  PEXOR_TX( ps_pexor, l_comm, l_address, l_data) ;
//  if(PEXOR_RX( ps_pexor, l_sfp, &a , &b, &c)==1) {
//#ifdef DEBUT
//    printf ("PEXOR_Slave_Write: Reply to PEXOR from SFP: 0x%x ", l_sfp);
//    printf (" 0x%x 0x%x 0x%x \n", a,b,c);
//#endif
//    if( (a&0xfff) == PEXOR_PT_AD_W_REP){
//      if(a&0x4000!=0){
//	printf ("PEXOR_Slave_Write: ERROR: Packet Structure : Command Reply 0x%x \n", a);
//	status=-1;
//      }
//    } else {
//      printf ("PEXOR_Slave_Write: ERROR : Access to empty slave or address: Module  0x%x Address 0x%x  Command Reply  0x%x \n", ( l_address&0xff000000) >> 24 , l_address&0xffffff, a );
//      status=-1;
//    }
//  }else{
//    status=-1;
//    printf ("PEXOR_Slave_Write: no reply: 0x%x 0x%x 0x%x \n", a,b,c);
//  }
//
//  return status;
//}
//
//
//int PEXOR_TK_TX_Mem_Read( s_pexor *ps_pexor, long *addr, long *data ){
//
//  *data =  * ( ps_pexor->tk_mem + (*addr >>2) );
//  //  printf ("PEXOR_Read: 0x%x  0x%x \n",(ps_pexor->tk_mem),*(ps_pexor->tk_mem) );
//  //  printf ("PEXOR_Read: 0x%x  0x%x \n",(ps_pexor->tk_mem+(addr>>2)),*(ps_pexor->tk_mem+(addr>>2) ) );
//}
//
//int PEXOR_TK_Mem_Write( s_pexor *ps_pexor, long *addr, long *data ){
//
//   *( ps_pexor->tk_mem + (*addr >>2) )= *data;
//  //  printf ("PEXOR_Read: 0x%x  0x%x \n",(ps_pexor->pexor_base),*(ps_pexor->pexor_base) );
//  //  printf ("PEXOR_Read: 0x%x  0x%x \n",(ps_pexor->pexor_base+(addr>>2)),*(ps_pexor->pexor_base+(addr>>2) ) );
//}
//
//int PEXOR_RX_Clear( s_pexor *ps_pexor ){
//
//  //  while( (*ps_pexor->rep_stat&0xcccc)!=0x0 ){
//  while( (*ps_pexor->rep_stat)!=0x0 ){
//    *ps_pexor->rep_clr=0xf;
//    //    sleep(1);
//#ifdef DEBUG
//    //    printf ("PEXOR_RX_Clear: rep_stat: 0x%x 0x%x \n",ps_pexor->rep_stat, *ps_pexor->rep_stat );
//#endif
//  }
//  return(1);
//}
//
//
//
//int PEXOR_RX_Clear_Ch( s_pexor *ps_pexor, long ch ){
//  long val;
//  val = 0x1<<ch;
//  while( (*(ps_pexor->rep_stat_0+ch)&0xf000)!=0x0|(*(ps_pexor->sfp_tk_stat+ch)&0xf000)!=0x0 ){
//  //  while( (*ps_pexor->rep_stat)!=0x0 ){
//    //    *ps_pexor->rep_clr=0xf;
//    *ps_pexor->rep_clr=val;
//    //    sleep(1);
//#ifdef DEBUG
//    //    printf ("PEXOR_RX_Clear: rep_stat: ch 0x%x 0x%x  0x%x \n",ch, *(ps_pexor->sfp_tk_stat+ch), *(ps_pexor->rep_stat_0+ch) );
//#endif
//  }
//  return(1);
//}
//
//int PEXOR_RX_Clear_Pattern( s_pexor *ps_pexor, long l_ptn ){
//  long mask;
//  mask=(l_ptn<<8)|(l_ptn<<4)|l_ptn;
//  //  while( (*ps_pexor->rep_stat&0xcccc)!=0x0 ){
//  while( ((*ps_pexor->rep_stat)&mask)!=0x0 ){
//    *ps_pexor->rep_clr=l_ptn;
//    //    sleep(1);
//#ifdef DEBUG
//    //    printf ("PEXOR_RX_Clear: rep_stat: 0x%x 0x%x \n",ps_pexor->rep_stat, *ps_pexor->rep_stat );
//#endif
//  }
//  return(1);
//}
//
//
//
//int PEXOR_TX_Reset_Ch( s_pexor *ps_pexor, long ch ){
//  long val;
//  val = 0x1<<(ch+4);
//  *ps_pexor->rx_rst= val;
//  return(1);
//}
//
//int PEXOR_SERDES_Reset( s_pexor *ps_pexor){
//  long val;
//  *ps_pexor->rx_rst= 0x100;
//  *ps_pexor->rx_rst= 0x0;
//  sleep(1);
//  return(1);
//}
//
//
//
//int PEXOR_TX( s_pexor *ps_pexor,  long comm, long addr, long data ){
//  *ps_pexor->req_addr = addr;
//  *ps_pexor->req_data = data;
//  *ps_pexor->req_comm = comm;
//  return(1);
//}
//
//int PEXOR_RX( s_pexor *ps_pexor, int sfp_id,  long *comm, long *addr, long *data ){
//  int stat;
//  int loop=0;
//  int loop_max=1000000;
//
//  stat=-1;
//  if(sfp_id==0){
//    while( ((*ps_pexor->rep_stat_0 & 0x3000)>>12)!=2 && loop < loop_max){
//      //      sleep(1);
//      loop++;
//      //      printf ("PEXOR_RX: rep_stat: sfp0:0x%x  loop %d \n", *ps_pexor->rep_stat_0, loop);
//    }
//      *comm =  *ps_pexor->rep_stat_0;
//      *addr =  *ps_pexor->rep_addr_0;
//      *data =  *ps_pexor->rep_data_0;
//
//  }  else if(sfp_id==1){
//    while( ((*ps_pexor->rep_stat_1 & 0x3000)>>12)!=2  && loop < loop_max ){
//      //      sleep(1);
//      loop++;
//      //      printf ("PEXOR_RX: rep_stat: sfp1:0x%x \n", *ps_pexor->rep_stat_1);
//    }
//    *comm =  *ps_pexor->rep_stat_1;
//    *addr =  *ps_pexor->rep_addr_1;
//    *data =  *ps_pexor->rep_data_1;
//
//  }  else if(sfp_id==2){
//    while( ((*ps_pexor->rep_stat_2 & 0x3000)>>12)!=2   && loop < loop_max){
//      //      sleep(1);
//      loop++;
//      //      printf ("PEXOR_RX: rep_stat: sfp2:0x%x \n", *ps_pexor->rep_stat_2);
//    }
//    *comm =  *ps_pexor->rep_stat_2;
//    *addr =  *ps_pexor->rep_addr_2;
//    *data =  *ps_pexor->rep_data_2;
//  }  else if(sfp_id==3){
//    while( ((*ps_pexor->rep_stat_3 & 0x3000)>>12)!=2   && loop < loop_max){
//      //      sleep(1);
//      loop++;
//      //      printf ("PEXOR_RX: rep_stat: sfp3:0x%x \n", *ps_pexor->rep_stat_3);
//    }
//    *comm =  *ps_pexor->rep_stat_3;
//    *addr =  *ps_pexor->rep_addr_3;
//    *data =  *ps_pexor->rep_data_3;
//  }
//  if(loop!=loop_max) stat =1;
//  return(stat);
//}
//
//
//long PEXOR_TK_Data_Size( s_pexor *ps_pexor, long l_sfp,  long slave_id ){
//  long comm, addr, data;
//
//  *(ps_pexor->sfp_tk_sel+l_sfp) = slave_id;
//  return(  *(ps_pexor->sfp_tk_dsize + l_sfp) );
//}
//
//long PEXOR_TK_Mem_Size( s_pexor *ps_pexor, long l_sfp ){
//  long comm, addr, data;
//  return ( *(ps_pexor->tk_mem_size+l_sfp)) ;
//}
//
//long PEXOR_TK_Mem_Read( s_pexor *ps_pexor, long l_sfp, long **pl_dat  ){
//  long mem_size, mem_size_32b;
//  long MEM_SIZE_MAX=0x10000;
//  long *pl_dat_start;
//  int i;
//  pl_dat_start = *pl_dat;
//  mem_size=*(ps_pexor->tk_mem_size+l_sfp);
//  printf ("PEXOR_TK_Mem_Read:  %x \n", mem_size);
//
//  mem_size_32b = (mem_size>>2);
//  if(mem_size > MEM_SIZE_MAX) {
//    printf ("PEXOR_TK_Mem_Read: sfp x%x  memory overflow!! (size = %x ) \n", l_sfp, mem_size);
//  } else {
//    if (l_sfp==0) {
//      for (i=0; i < mem_size_32b ; i++)	*((*pl_dat)++)=*(ps_pexor->tk_mem_0 +i);
//    } else if (l_sfp==1) {
//      for (i=0; i < mem_size_32b ; i++)	*((*pl_dat)++)=*(ps_pexor->tk_mem_1 +i);
//    } else if (l_sfp==2) {
//      for (i=0; i < mem_size_32b ; i++)	*((*pl_dat)++)=*(ps_pexor->tk_mem_2 +i);
//    } else if (l_sfp==3) {
//      for (i=0; i < mem_size_32b ; i++) *((*pl_dat)++)=*(ps_pexor->tk_mem_3 +i);
//    }
//  }
//  *pl_dat=pl_dat_start;
//  return (mem_size ) ;
//}
//
//
//int PEXOR_RX_Status( s_pexor *ps_pexor ){
//  //  printf ("0x%x 0x%x \n", ps_pexor->rep_stat,*ps_pexor->rep_stat);
//  printf ("PEXOR_RX_Status: rep_stat: sfp0:0x%x sfp1:0x%x sfp2:0x%x sfp3:0x%x \n",
//	  *ps_pexor->rep_stat_0,*ps_pexor->rep_stat_1 ,
//	  *ps_pexor->rep_stat_2,*ps_pexor->rep_stat_3);
//  return(1);
//}
//
//int PEXOR_Port_Monitor( s_pexor *ps_pexor ){
//  long tmp, fault, moni;
//  int flag=-1;
//  int loop=0;
//  int loopmax=10;
//  //  printf ("0x%x 0x%x \n", ps_pexor->rep_stat,*ps_pexor->rep_stat);
//    while((loop < loopmax)&&(flag==-1)){
//      tmp=~(*ps_pexor->sfp_fault);
//      fault=(tmp&0x1)*0xff+(tmp&0x2)*0xff00/2+(tmp&0x4)*0xff0000/4+(tmp&0x8)*0xff000000/8;
//      moni=(*ps_pexor->rx_moni)&fault;
//
//      if( (  ( (moni&0xff)!=0) && ((moni&0xff)!=0xbc))||( ((moni&0xff00)!=0) && ((moni&0xff00)!=0xbc00) ) ||
//	  ( ((moni&0xff0000)!=0) && ((moni&0xff0000)!=0xbc0000) ) ||( ((moni&0xff000000)!=0) &&( (moni&0xff000000)!=0xbc000000)) )
//	{
//	  PEXOR_SERDES_Reset(ps_pexor);
//
//	}else{
//	  flag=1;
//	}
//    }
//
//    if(flag==-1){
//      printf ("PEXOR_RX_Monitor Error: loop %d 0x%8x 0x%8x 0x%8x  \n",loop, fault, moni ,moni&fault);
//    }
//  return(flag);
//}
//
//
//int PEXOR_Show_Version( s_pexor *ps_pexor ){
//  long tmp, year,month,day,version[2];
//  tmp=(*ps_pexor-> pexor_version);
//  year=((tmp&0xff000000)>>24)+0x2000;
//  month=(tmp&0xff0000)>>16;
//  day=(tmp&0xff00)>>8;
//  version[0]=(tmp&0xf0)>>4;
//  version[1]=(tmp&0xf);
//
//  printf ("PEXOR program compiled at Year=%x Month=%x Date=%x Version=%x.%x \n", year,month,day,version[0],version[1]);
//
//  return(1);
//}
//
//int PEXOR_SFP_Active( s_pexor *ps_pexor, long l_sfp  ){
//  long l_comm;
//  long l_slave=100;
//  long a,b,c;
//  long l_num;
//  long active;
//  long loop;
//  long loopmax=10;
//
//  active=((~*ps_pexor->sfp_fault)>>l_sfp)&0x1;
//  l_num=-1;
//  loop=0;
//  if(active==1&&loop<loopmax){
//    l_comm = PEXOR_INI_REQ| (0x1<<16+l_sfp);
//    PEXOR_RX_Clear_Ch(ps_pexor, l_sfp);
//    while(l_num==-1){
//      PEXOR_TX(ps_pexor, l_comm, 0, l_slave) ;
//      //  printf (" SFP%d: ",l_sfp);
//      if(PEXOR_RX(ps_pexor, l_sfp, &a , &b, &c)==-1) {
//	//    printf ("no reply: 0x%x 0x%x 0x%x \n", a,b,c);
//	l_num=-1;
//      }else{
//	//    printf ("Reply: 0x%x 0x%x 0x%x \n", a,b,c);
//	//    printf ("     : No of slaves  : 0x%x\n", b);
//	l_num=b;
//      }
//      loop++;
//    }
//  }
//  //  printf (" SFP0 %x    : No of slaves  : %i \n", l_sfp, l_num);
//
//  return(l_num);
//}
//
//
//
//int PEXOR_SFP_Disable( s_pexor *ps_pexor,  long disa ){
//  //  *ps_pexor->sfp_disa = disa;
//
//  printf ("PEXOR_SFP_Disable: x%x  x%x  \n", ps_pexor->sfp_disa, *ps_pexor->sfp_disa);
///*   printf ("PEXOR_SFP_-4 x%x  x%x  \n", (ps_pexor->sfp_fifo-4), *(ps_pexor->sfp_fault-4)); */
///*   printf ("PEXOR_SFP_-3: x%x  x%x  \n", (ps_pexor->sfp_fifo-3), *(ps_pexor->sfp_fault-3)); */
///*   printf ("PEXOR_SFP_-2: x%x  x%x  \n", (ps_pexor->sfp_fifo-2), *(ps_pexor->sfp_fault-2)); */
///*   printf ("PEXOR_SFP_-1: x%x  x%x  \n", (ps_pexor->sfp_fifo-1), *(ps_pexor->sfp_fault-1)); */
///*   printf ("PEXOR_SFP_FIFO: x%x  x%x  \n", ps_pexor->sfp_fifo, *ps_pexor->sfp_fifo); */
//  return(1);
//}
//
//int PEXOR_SFP_Show_FIFO( s_pexor *ps_pexor, long ch ){
//  int i;
//  long tmp;
//  i=0;
//  printf ("PEXOR_SFP_Show_FIFO: \n");
//  tmp=*(ps_pexor->sfp_fifo+ch);
//  while((tmp&0xf00)!=0x500){
//    printf ("x%x  x%2x \n", i, 0xfff&tmp);
//    tmp=*(ps_pexor->sfp_fifo+ch);
//    i++;
//  }
//  printf ("x%x  x%2x \n", i, 0xfff&tmp);
//  return(1);
//}
//
//int PEXOR_SFP_Clear_FIFO( s_pexor *ps_pexor ){
//  *ps_pexor->sfp_fifo = 0xf;
//  return(1);
//}
//
//
//
//int PEXOR_Set_LED( s_pexor *ps_pexor, long l_led){
//  int i,j;
//  long a;
//  long l_slave[4];
//
//  for(i=0;i<4;i++){
//    l_slave[i]=PEXOR_SFP_Active(ps_pexor, i);
//    if(l_slave[i]>0){
//      printf ("SFP port: %i\n", i );
//      for(j=0;j<l_slave[i];j++){
//	PEXOR_Slave_Read( ps_pexor, i, j, REG_VERSION, &a );
//	PEXOR_Slave_Write( ps_pexor, i, j, REG_LED, l_led );
//	printf ("  Module ID: 0x%x Program Version: 0x%x LED off\n",j , a );
//      }
//    }
//  }
//}
//
//
//int PEXOR_LED_On( s_pexor *ps_pexor ){
//  PEXOR_Set_LED( ps_pexor, 0x0 );
//}
//int PEXOR_LED_Off( s_pexor *ps_pexor ){
//  PEXOR_Set_LED( ps_pexor, 0x100 );
//}
//
//
//int PEXOR_Set_Data_Reduction( s_pexor *ps_pexor, long l_flag){
//  int i,j;
//  long a;
//  long l_slave[4];
//
//  for(i=0;i<4;i++){
//    l_slave[i]=PEXOR_SFP_Active(ps_pexor, i);
//    if(l_slave[i]>0){
//#ifdef DEBUG
//      printf ("SFP port: %i\n", i );
//#endif
//      for(j=0;j<l_slave[i];j++){
//	PEXOR_Slave_Read( ps_pexor, i, j, REG_VERSION, &a );
//	PEXOR_Slave_Write( ps_pexor, i, j, REG_DATA_REDUCTION, l_flag );
//#ifdef DEBUG
//	printf ("  Module ID: 0x%x Program Version: 0x%x Data Reduction 0x%\n",j , l_flag );
//#endif
//      }
//    }
//  }
//}
//
//
//
//#endif
//

#endif
