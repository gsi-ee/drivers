

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
  /*printm("mbspex: opening %s...\n",fname);*/
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


int mbspex_reset (int handle)
{
  int rev, errsv=0;;
  mbspex_assert_handle(handle);
  printm ("mbspex: resetting pex device...");
  rev = ioctl (handle, PEX_IOC_RESET);
  errsv = errno;
    if (rev)
    {
      printm ("\n\nError %d reseting pex device", errsv, strerror (errsv));
    }
    else
       printm(" done!\n");
    return rev;
}

/*****************************************************************************/

int  mbspex_slave_init (int handle, long l_sfp, long l_n_slaves)

{

  int rev = 0, errsv=0;
  struct pex_bus_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.sfp = l_sfp;
  descriptor.slave = l_n_slaves;
  printm ("mbspex: initialize SFP chain %d with %d slaves...", l_sfp, l_n_slaves);
  rev = ioctl (handle, PEX_IOC_INIT_BUS, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm ("\n\nError %d  on initializing channel %lx, maxdevices %lx - %s\n", errsv, l_sfp, l_n_slaves, strerror (errsv));
  }
  else
   {
     printm(" done!\n");
   }

  return rev;
}


/*****************************************************************************/
int mbspex_slave_wr (int handle, long l_sfp, long l_slave, long l_slave_off, long l_dat)
{
  int rev = 0, errsv=0;
  struct pex_bus_io descriptor;
  mbspex_assert_handle(handle);
  //PexorInfo("WriteBus writes %x to %x \n",value, address);
  descriptor.address = l_slave_off;
  descriptor.value = l_dat;
  descriptor.sfp = l_sfp;
  descriptor.slave = l_slave;
  rev = ioctl (handle, PEX_IOC_WRITE_BUS, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on writing value 0x%lx to address 0x%lx (sfp:%d, slave:%d)- %s\n", errsv, l_dat,  l_slave_off, l_sfp,
        l_slave, strerror (errsv));
  }
  return rev;
}

int mbspex_slave_config (int handle, struct pex_bus_config* config)
{
  int rev = 0, errsv=0, i=0;
  mbspex_assert_handle(handle);
  if(!config) return -EINVAL;
  rev = ioctl (handle, PEX_IOC_CONFIG_BUS, config);
   errsv = errno;
    if (rev)
    {
      printm (RON"ERROR>>"RES"Error %d  on writing configuration to bus- %s\n", errsv, strerror (errsv));
    }
    return rev;
}

int mbspex_get_configured_slaves(int handle , struct pex_sfp_links* setup)
{
  int rev = 0, errsv = 0;
  mbspex_assert_handle(handle);
  rev = ioctl (handle, PEX_IOC_GET_SFP_LINKS, setup);
    errsv = errno;
    if (rev)
    {
      printm (RON"ERROR>>"RES"Error %d  on retrieving slave link configuration- %s\n", errsv, strerror (errsv));
    }
    return rev;
}





/*****************************************************************************/
int mbspex_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{
  int rev = 0, errsv=0;
  struct pex_bus_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.address = l_slave_off;
  descriptor.value = 0;
  descriptor.sfp = l_sfp;
  descriptor.slave = l_slave;
  rev = ioctl (handle, PEX_IOC_READ_BUS, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES" Error %d  on reading from address %0xlx (sfp:%d, slave:%d)- %s\n", errsv, l_slave_off, l_sfp, l_slave,
        strerror (errsv));
    return rev;
  }
  *l_dat = descriptor.value;
  return 0;

}



#ifndef MBSPEX_GOSPCMD_ONLY


/*****************************************************************************/

int  mbspex_send_and_receive_tok (int handle, long l_sfp, long l_toggle, unsigned long l_dma_target, unsigned long* pl_transfersize,
    long *pl_check_comm, long *pl_check_token, long *pl_check_slaves)

{
  int rev=0, errsv=0;
      struct pex_token_io descriptor;
      descriptor.bufid=l_toggle;
      descriptor.sfp=l_sfp;
      descriptor.sync=1;
      descriptor.directdma=1;
      descriptor.dmatarget= l_dma_target;
      descriptor.dmaburst=0;
      rev=ioctl(handle, PEX_IOC_REQUEST_TOKEN, &descriptor);
      errsv = errno;
      if(rev)
          {
              printm(RON"ERROR>>"RES" mbspex_send_and_receive_tok -Error %d  on token request, sfp 0x%x toggle:0x%x - %s\n",errsv, l_sfp, l_toggle, strerror(errsv));
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

  int rev=0, errsv=0;
       struct pex_token_io descriptor;
       descriptor.bufid=l_toggle;
       descriptor.sfp= (l_sfp_p << 16); // upper bytes expected as sfp pattern by driver
       descriptor.sync=0;
       descriptor.directdma=0;
       rev=ioctl(handle, PEX_IOC_REQUEST_TOKEN, &descriptor);
       errsv = errno;
       if(rev)
           {
               printm(RON"ERROR>>"RES" mbspex_send_tok -Error %d  on token request, sfp pattern 0x%x toggle:0x%x - %s\n",errsv, l_sfp_p, l_toggle, strerror(errsv));
               return -1;
           }
return rev;
}




/*****************************************************************************/
int  mbspex_receive_tok (int handle, long l_sfp, unsigned long l_dma_target, unsigned long* pl_transfersize, long *pl_check_comm, long *pl_check_token, long *pl_check_slaves)
{
  int rev=0, errsv=0;
      struct pex_token_io descriptor;
      descriptor.sfp=l_sfp;
      descriptor.dmatarget= l_dma_target;
      descriptor.dmaburst=0;
      descriptor.directdma=0;
      if(l_dma_target==0) descriptor.directdma=1; // we disable the automatic DMA sending after token reception by this
      rev=ioctl(handle, PEX_IOC_WAIT_TOKEN, &descriptor);
      errsv = errno;
      if(rev)
          {
              printm(RON"ERROR>>"RES "Error %d  on wait token from channel 0x%x - %s\n",errsv,l_sfp, strerror(errsv));
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
  mbspex_assert_handle(handle);
  if(mbspex_register_wr(handle,0, MBSPEX_TK_DSIZE_SEL + 4*l_sfp ,slave_id)) return -1;
  if(mbspex_register_rd (handle, 0, MBSPEX_REP_TK_DSIZE + 4*l_sfp , &rev)) return -1;
  return rev;
// from original gosip interface:
  //  *(ps_pexor->sfp_tk_sel+l_sfp) = slave_id;
//  return(  *(ps_pexor->sfp_tk_dsize + l_sfp) );

}

long mbspex_get_tok_memsize(int handle , long l_sfp ){
  long rev=0;
  mbspex_assert_handle(handle);
  if(mbspex_register_rd (handle, 0, MBSPEX_TK_MEM_SIZE + 4*l_sfp , &rev)) return -1;
  return rev;
  //return ( *(ps_pexor->tk_mem_size+l_sfp)) ;
}


/*****************************************************************************/
int mbspex_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat)
{
  int rev = 0, errsv = 0;
  struct pex_reg_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.value = l_dat;
  descriptor.bar = s_bar;
  rev = ioctl (handle, PEX_IOC_WRITE_REGISTER, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on writing value %0xlx to address %0xlx (bar:%d)- %s\n", errsv, l_dat, l_address,
        s_bar, strerror (errsv));
  }
  return rev;
}

int mbspex_register_rd (int handle, unsigned char s_bar, long l_address, long * l_dat)
{
  int rev = 0, errsv = 0;
  struct pex_reg_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.bar = s_bar;
  rev = ioctl (handle, PEX_IOC_READ_REGISTER, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on reading from address 0x%lx (bar:%d)- %s\n", errsv, l_address,
        s_bar, strerror (errsv));
  }
  * l_dat=descriptor.value;
  return rev;
}


int mbspex_dma_rd (int handle, long source, long dest, long size, int burst)
{
  int rev = 0, errsv = 0;;
  struct pex_dma_io descriptor;
  mbspex_assert_handle(handle);
  descriptor.source = source;
  descriptor.target = dest;
  descriptor.size = size;
  descriptor.burst=burst;
  rev = ioctl (handle, PEX_IOC_READ_DMA, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on DMA reading 0x%x bytes from address 0x%lx  to 0x%lx (%s)\n", errsv, size, source,dest, strerror (errsv));
    return -1;
  }
  return descriptor.size;
}

#endif /* gosipcmd only mode*/


