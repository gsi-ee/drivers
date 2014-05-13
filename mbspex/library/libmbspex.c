

#include  "libmbspex.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>


#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#ifdef MBSPEX_USEMBS
#include "f_ut_printm.h"
#else
#define printm printf
#endif

int mbspex_open(int devnum)
{
  int filehandle,errsv;
  char devname[64];
  char fname[256];

  snprintf(devname,64,PEXORNAMEFMT, devnum);
  snprintf(fname,256,"/dev/%s",devname);
  printm("mbspex: opening %s...\n",fname);
  filehandle = open(fname, O_RDWR );
  errsv = errno;

  if (filehandle < 0)
  {
    printm("mbspex: error %d (%s) opening device %s...\n",errsv, strerror(errsv), fname);
  }
  return filehandle;
}

int mbspex_close(int handle)
{
  mbspex_assert_handle(handle);
  close(handle);
  /* add all cleanup actions here*/
}




/*****************************************************************************/

int  mbspex_slave_init (int handle, long l_sfp, long l_n_slaves)

{

  int rev = 0;
  struct pex_bus_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.sfp = l_sfp;
  descriptor.slave = l_n_slaves;
  printm ("initialize SFP chain %d with %d slaves", l_sfp, l_n_slaves);
  rev = ioctl (handle, PEX_IOC_INIT_BUS, &descriptor);
  if (rev)
  {
    printm ("\n\nError %d  on initializing channel %lx, maxdevices %lx - %s\n", rev, l_sfp, l_n_slaves, strerror (rev));
  }
  return rev;
}
//int f_pex_slave_init (long l_sfp, long l_n_slaves)
//  int  l_ret;
//  long l_comm;
//
//  printm ("initialize SFP chain %d ", l_sfp);
//
//  l_comm = PEXOR_INI_REQ | (0x1<<16+l_sfp);
//
//  for (l_j=1; l_j<=10; l_j++)
//  {
//    PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp);
//    PEXOR_TX (&sPEXOR, l_comm, 0, l_n_slaves  - 1) ;
//    //printm ("SFP %d: try nr. %d \n", l_sfp, l_j);
//    l_dat1 = 0; l_dat2 = 0; l_dat3 = 0;
//    l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, &l_dat3);
//    if ( (l_stat != -1) && (l_dat2 > 0) && (l_dat2<=32))
//    {
//      break;
//    }
//    #ifndef Linux
//    yield ();
//    #else
//    sched_yield ();
//    #endif
//  }
//  l_ret = 0;
//  if (l_stat == -1)
//  {
//    l_ret = -1;
//    printm (RON"ERROR>>"RES" initialization of SFP chain %d failed. ", l_sfp);
//    printm ("no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, l_dat3);
//    //printm ("exiting.. \n"); exit (0);
//  }
//  else
//  {
//    if (l_dat2 != 0)
//    {
//      printm ("initialization for SFP chain %d done. \n", l_sfp),
//      printm ("No of slaves : %d \n", l_dat2);
//    }
//    else
//    {
//      l_ret = -1;
//      printm (RON"ERROR>>"RES" initialization of SFP chain %d failed. ", l_sfp);
//      printm ("no slaves found \n");
//      //printm ("exiting.. \n"); exit (0);
//    }
//  }
//  return (l_ret);
//}

/*****************************************************************************/
int mbspex_slave_wr (int handle, long l_sfp, long l_slave, long l_slave_off, long l_dat)
{
  int rev = 0;
  struct pex_bus_io descriptor;
  mbspex_assert_handle(handle);
  //PexorInfo("WriteBus writes %x to %x \n",value, address);
  descriptor.address = l_slave_off;
  descriptor.value = l_dat;
  descriptor.sfp = l_sfp;
  descriptor.slave = l_slave;
  rev = ioctl (handle, PEX_IOC_WRITE_BUS, &descriptor);
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on writing value %0xlx to address %0xlx (sfp:%d, slave:%d)- %s\n", rev, l_dat, l_sfp,
        l_slave, l_slave_off, strerror (rev));
  }
  return rev;
}





//int f_pex_slave_wr (long l_sfp, long l_slave, long l_slave_off, long l_dat)
//  int  l_ret;
//  long l_comm;
//  long l_addr;
//
//  l_comm = PEXOR_PT_AD_W_REQ | (0x1<<16+l_sfp);
//  l_addr = l_slave_off + (l_slave << 24);
//  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp);
//  PEXOR_TX (&sPEXOR, l_comm, l_addr, l_dat);
//  l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, &l_dat3);
//
//  l_ret = 0;
//  if (l_stat == -1)
//  {
//    l_ret = -1;
//    l_err_flg++;
//    l_i_err_flg[l_sfp][l_slave]++;
//    #ifdef DEBUG
//    printm (RON"ERROR>>"RES" writing to SFP: %d, slave id: %d, addr 0x%d \n",
//                                                l_sfp, l_slave, l_slave_off);
//    printm ("  no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, l_dat3);
//    #endif // DEBUG
//  }
//  else
//  {
//    // printm ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
//    if( (l_dat1 & 0xfff) == PEXOR_PT_AD_W_REP)
//    {
//      //printm ("SFP: %d, slave id: %d addr: 0x%x  \n",
//      //                l_sfp, (l_dat2 & 0xf0000) >> 24, l_dat2 & 0xfffff);
//      if ( (l_dat1 & 0x4000) != 0)
//      {
//        l_ret = -1;
//        l_err_flg++;
//        l_i_err_flg[l_sfp][l_slave]++;
//        #ifdef DEBUG
//        printm (RON"ERROR>>"RES" packet structure: command reply 0x%x \n", l_dat1);
//        #endif // DEBUG
//      }
//    }
//    else
//    {
//      l_ret = -1;
//      l_err_flg++;
//      l_i_err_flg[l_sfp][l_slave]++;
//      #ifdef DEBUG
//      printm (RON"ERROR>>"RES" writing to empty slave or wrong address: \n");
//      printm ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
//           l_sfp, l_slave, (l_addr & 0xf00000) >> 24 , l_addr & 0xfffff, l_dat1);
//      #endif // DEBUG
//    }
//  }
//  return (l_ret);
//}

/*****************************************************************************/
int mbspex_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{
  int rev = 0;
  struct pex_bus_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.address = l_slave_off;
  descriptor.value = 0;
  descriptor.sfp = l_sfp;
  descriptor.slave = l_slave;
  rev = ioctl (handle, PEX_IOC_READ_BUS, &descriptor);
  if (rev)
  {
    printm (RON"ERROR>>"RES" Error %d  on reading from address %0xlx (sfp:%d, slave:%d)- %s\n", rev, l_slave_off, l_sfp, l_slave,
        strerror (rev));
    return rev;
  }
  *l_dat = descriptor.value;
  return 0;

}

//int f_pex_slave_rd (long l_sfp, long l_slave, long l_slave_off, long *l_dat)
//  int  l_ret;
//  long l_comm;
//  long l_addr;
//
//  l_comm = PEXOR_PT_AD_R_REQ | (0x1<<16+l_sfp);
//  l_addr = l_slave_off + (l_slave << 24);
//  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp);
//  PEXOR_TX (&sPEXOR, l_comm, l_addr, 0);
//  l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, l_dat);
//  //printm ("f_pex_slave_rd, l_dat: 0x%x, *l_dat: 0x%x \n", l_dat, *l_dat);
//
//  l_ret = 0;
//  if (l_stat == -1)
//  {
//    l_ret = -1;
//    l_err_flg++;
//    l_i_err_flg[l_sfp][l_slave]++;
//    #ifdef DEBUG
//    printm (RON"ERROR>>"RES" reading from SFP: %d, slave id: %d, addr 0x%d \n",
//                                  l_sfp, l_slave, l_slave_off);
//    printm ("  no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, *l_dat);
//    #endif // DEBUG
//  }
//  else
//  {
//    // printm ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
//    if( (l_dat1 & 0xfff) == PEXOR_PT_AD_R_REP)
//    {
//      //printm ("SFP: %d, slave id: %d addr: 0x%x  \n",
//      //     l_sfp, (l_dat2 & 0xf00000) >> 24, l_dat2 & 0xfffff);
//      if ( (l_dat1 & 0x4000) != 0)
//      {
//        l_ret = -1;
//        l_err_flg++;
//        l_i_err_flg[l_sfp][l_slave]++;
//        #ifdef DEBUG
//        printm (RON"ERROR>>"RES" packet structure: command reply 0x%x \n", l_dat1);
//        #endif //DEBUG
//      }
//    }
//    else
//    {
//      l_ret = -1;
//      l_err_flg++;
//      l_i_err_flg[l_sfp][l_slave]++;
//      #ifdef DEBUG
//      printm (RON"ERROR>>"RES" Reading from empty slave or wrong address: \n");
//      printm ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
//              l_sfp, l_slave, (l_addr & 0xf0000) >> 24 , l_addr & 0xfffff, l_dat1);
//      #endif // DEBUG
//    }
//  }
//  return (l_ret);
//}

/*****************************************************************************/

int  mbspex_send_and_receive_tok (int handle, long l_sfp, long l_toggle, unsigned long l_dma_target, unsigned long* pl_transfersize,
    long *pl_check_comm, long *pl_check_token, long *pl_check_slaves)

{
  int rev=0;
      struct pex_token_io descriptor;
      descriptor.bufid=l_toggle;
      descriptor.sfp=l_sfp;
      descriptor.sync=1;
      descriptor.dmatarget= l_dma_target;
      rev=ioctl(handle, PEX_IOC_REQUEST_TOKEN, &descriptor);
      if(rev)
          {
              printm(RON"ERROR>>"RES" mbspex_send_and_receive_tok -Error %d  on token request, sfp 0x%x toggle:0x%x - %s\n",rev, l_sfp, l_toggle, strerror(rev));
              return -1;
          }
      *pl_check_comm=descriptor.check_comm;
      *pl_check_token=descriptor.check_token;
      *pl_check_slaves=descriptor.check_numslaves;
      *pl_transfersize=descriptor.dmasize;

return rev;
}
//int f_pex_send_and_receive_tok (long l_sfp, long l_toggle,
//                    long *pl_check1, long *pl_check2, long *pl_check3)
//  int  l_ret;
//  long l_comm;
//
//  l_comm = PEXOR_PT_TK_R_REQ | (0x1<<16+l_sfp);
//  PEXOR_RX_Clear_Ch(&sPEXOR, l_sfp);
//  PEXOR_TX (&sPEXOR, l_comm, l_toggle, 0);
//  l_stat = PEXOR_RX (&sPEXOR, l_sfp, pl_check1, pl_check2, pl_check3);
//  // return values:
//  // l_check1: l_comm
//  // l_check2: toggle bit
//  // l_check3: nr. of slaves connected to token chain
//
//  l_ret = 0;
//  if (l_stat == -1)
//  {
//    l_ret = -1;
//    #ifdef DEBUG
//    printm (RON"ERROR>>"RES" sending token to SFP: %d \n", l_sfp);
//    printm ("  no reply: 0x%x 0x%x 0x%x \n", *pl_check1, *pl_check2, *pl_check3);
//    #endif // DEBUG
//  }
//
//  return (l_ret);
//}

/*****************************************************************************/

 // sends token to all SFPs marked in l_sfp_p pattern: 1: sfp 0, 2: sfp 1,
  //                                                    4: sfp 2, 8: sfp 3,
  //                                                  0xf: all four SFPs
int  mbspex_send_tok (int handle, long l_sfp_p, long l_toggle)
{

  int rev=0;
       struct pex_token_io descriptor;
       descriptor.bufid=l_toggle;
       descriptor.sfp= (l_sfp_p << 16); // upper bytes expected as sfp pattern by driver
       descriptor.sync=0;
       rev=ioctl(handle, PEX_IOC_REQUEST_TOKEN, &descriptor);
       if(rev)
           {
               printm(RON"ERROR>>"RES" mbspex_send_tok -Error %d  on token request, sfp pattern 0x%x toggle:0x%x - %s\n",rev, l_sfp_p, l_toggle, strerror(rev));
               return -1;
           }
return rev;
}


//int f_pex_send_tok (long l_sfp_p, long l_toggle) {
//  long l_comm;
//
//  l_comm = PEXOR_PT_TK_R_REQ | (l_sfp_p << 16);
//  PEXOR_RX_Clear_Pattern(&sPEXOR, l_sfp_p);
//  PEXOR_TX (&sPEXOR, l_comm, l_toggle, 0);
//
//  return (0);
//}

/*****************************************************************************/
int  mbspex_receive_tok (int handle, long l_sfp, unsigned long l_dma_target, unsigned long* pl_transfersize, long *pl_check_comm, long *pl_check_token, long *pl_check_slaves)
{
  int rev=0;
      struct pex_token_io descriptor;
      descriptor.sfp=l_sfp;
      descriptor.dmatarget= l_dma_target;
      rev=ioctl(handle, PEX_IOC_WAIT_TOKEN, &descriptor);
      if(rev)
          {
              printm(RON"ERROR>>"RES "Error %d  on wait token from channel 0x%x - %s\n",rev,l_sfp, strerror(rev));
              return -1;
          }
      *pl_check_comm=descriptor.check_comm;
      *pl_check_token=descriptor.check_token;
      *pl_check_slaves=descriptor.check_numslaves;
      *pl_transfersize=descriptor.dmasize;
      return 0;
}
//int f_pex_receive_tok (long l_sfp, long *pl_check1, long *pl_check2, long *pl_check3)
//{
//  // checks token return for a single, individual SFPS
//  int  l_ret;
//
//  l_stat = PEXOR_RX (&sPEXOR, l_sfp, pl_check1, pl_check2, pl_check3);
//  // return values:
//  // l_check1: l_comm
//  // l_check2: toggle bit
//  // l_check3: nr. of slaves connected to token chain
//
//  l_ret = 0;
//  if (l_stat == -1)
//  {
//    l_ret = -1;
//    #ifdef DEBUG
//    printm (RON"ERROR>>"RES" receiving token from SFP: %d \n", l_sfp);
//    printm ("  no reply: 0x%x 0x%x 0x%x \n", *pl_check1, *pl_check2, *pl_check3);
//    #endif // DEBUG
//  }
//
//  return (l_ret);
//}

/*****************************************************************************/
