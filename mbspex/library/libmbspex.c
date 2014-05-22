

#include  "libmbspex.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>


#define RON  "\x1B[7m"
#define RES  "\x1B[0m"



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


/*****************************************************************************/


long mbspex_get_tok_datasize(int handle, long l_sfp,  long slave_id ){

  long rev=0;
  if(mbspex_register_wr(handle,0, MBSPEX_TK_DSIZE_SEL + l_sfp ,slave_id)) return -1;
  if(mbspex_register_rd (handle, 0, MBSPEX_REP_TK_DSIZE + l_sfp , &rev)) return -1;
  return rev;
// from original gosip interface:
  //  *(ps_pexor->sfp_tk_sel+l_sfp) = slave_id;
//  return(  *(ps_pexor->sfp_tk_dsize + l_sfp) );

}

long mbspex_get_tok_memsize(int handle , long l_sfp ){
  long rev=0;
  if(mbspex_register_rd (handle, 0, MBSPEX_TK_MEM_SIZE + l_sfp , &rev)) return -1;
  return rev;
  //return ( *(ps_pexor->tk_mem_size+l_sfp)) ;
}



/*****************************************************************************/
int mbspex_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat)
{
  int rev = 0;
  struct pex_reg_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.value = l_dat;
  descriptor.bar = s_bar;
  rev = ioctl (handle, PEX_IOC_WRITE_REGISTER, &descriptor);
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on writing value %0xlx to address %0xlx (bar:%d)- %s\n", rev, l_dat, l_address,
        s_bar, strerror (rev));
  }
  return rev;
}

int mbspex_register_rd (int handle, unsigned char s_bar, long l_address, long * l_dat)
{
  int rev = 0;
  struct pex_reg_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.bar = s_bar;
  rev = ioctl (handle, PEX_IOC_READ_REGISTER, &descriptor);
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on reading from address 0x%lx (bar:%d)- %s\n", rev, l_address,
        s_bar, strerror (rev));
  }
  * l_dat=descriptor.value;
  return rev;
}


int mbspex_dma_rd (int handle, long source, long dest, long size)
{
  int rev = 0;
  struct pex_dma_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.source = source;
  descriptor.target = dest;
  descriptor.size = size;
  rev = ioctl (handle, PEX_IOC_READ_DMA, &descriptor);
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on DMA reading 0x%x bytes from address 0x%lx  to 0x%lx (%s)\n", rev, size, source,dest, strerror (rev));
    return -1;
  }
  return descriptor.size;
}




