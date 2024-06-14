

#include  "libmbspex.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#ifdef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS

struct pex_bus_io   bus_descriptor;
struct pex_token_io token_descriptor;
struct pex_reg_io   reg_descriptor;
struct pex_dma_io   dma_descriptor;
struct pex_pipebuf  pipe_descriptor;
struct pex_linkspeed_set _speed_descriptor;

#endif

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


int mbspex_set_linkspeed (int handle, long l_sfp, enum pex_linkspeed specs)
{
  int rev = 0, errsv=0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
  struct pex_linkspeed_set speed_descriptor;
#endif

  mbspex_assert_handle(handle);
  if(specs <=0 || specs>=PEX_MAX_SPEEDSETUP) return -1;
  speed_descriptor.sfp = l_sfp;
  speed_descriptor.specs = specs;
  printm ("mbspex: Change  linkspeed of %s%d) to preset %d (%s Gb)...",
      (l_sfp<0 ? "all SFPs (id:": "SFP chain (%d") , l_sfp, specs,gLinkspeed[specs]);
  rev = ioctl (handle, PEX_IOC_CHANGE_LINKSPEED, &speed_descriptor);
  errsv = errno;
  if (rev)
  {
    printm ("\n\nError %d  on  changing  linkspeed of SFP chain %d to preset %d (%s Gb) - %s\n", errsv, l_sfp, specs, strerror (errsv));
  }
  else
  {
    printm(" done!\n");
  }
  return rev;
}






int  mbspex_slave_init (int handle, long l_sfp, long l_n_slaves)

{

  int rev = 0, errsv=0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
  struct pex_bus_io bus_descriptor;
#endif

  mbspex_assert_handle(handle);
  bus_descriptor.sfp = l_sfp;
  bus_descriptor.slave = l_n_slaves;
  printm ("mbspex: initialize SFP chain %d with %d slaves...", l_sfp, l_n_slaves);
  rev = ioctl (handle, PEX_IOC_INIT_BUS, &bus_descriptor);
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
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
  struct pex_bus_io bus_descriptor;
#endif
  mbspex_assert_handle(handle);
  //PexorInfo("WriteBus writes %x to %x \n",value, address);
  bus_descriptor.address = l_slave_off;
  bus_descriptor.value = l_dat;
  bus_descriptor.sfp = l_sfp;
  bus_descriptor.slave = l_slave;
  rev = ioctl (handle, PEX_IOC_WRITE_BUS, &bus_descriptor);
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
  /* need to copy config structure to stack?*/
//  struct pex_bus_config theConfig;
//  for(i=0;(i<config->numpars) && (i< PEX_MAXCONFIG_VALS);++i){
//    theConfig.param[i]=config->param[i];
//    printf("mbspex_slave_config- s:%d d:%d a:0x%x val:0x%x\n",
//        theConfig.param[i].sfp, theConfig.param[i].slave,theConfig.param[i].address,theConfig.param[i].value);
//  }
//  theConfig.numpars=config->numpars;
  rev = ioctl (handle, PEX_IOC_CONFIG_BUS, config);
   errsv = errno;
    if (rev)
    {
      printm (RON"ERROR>>"RES"Error %d  on writing configuration to bus- %s\n", errsv, strerror (errsv));
    }
    return rev;
}





/*****************************************************************************/
int mbspex_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{
  int rev = 0, errsv=0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
  struct pex_bus_io bus_descriptor;
#endif
  mbspex_assert_handle(handle);
  bus_descriptor.address = l_slave_off;
  bus_descriptor.value = 0;
  bus_descriptor.sfp = l_sfp;
  bus_descriptor.slave = l_slave;
  rev = ioctl (handle, PEX_IOC_READ_BUS, &bus_descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES" Error %d  on reading from address %0xlx (sfp:%d, slave:%d)- %s\n", errsv, l_slave_off, l_sfp, l_slave,
        strerror (errsv));
    return rev;
  }
  *l_dat = bus_descriptor.value;
  return 0;

}


int mbspex_send_and_receive_parallel_tok (int handle, long l_sfp_p, long l_toggle, unsigned long l_dma_target,
    unsigned long* pl_transfersize, long *pl_check_comm, long *pl_check_token, long *pl_check_slaves)
{
  int rev=0, errsv=0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
       struct pex_token_io token_descriptor;
#endif
       token_descriptor.bufid=l_toggle;
       token_descriptor.sfp=(l_sfp_p << 16); // upper bytes expected as sfp pattern by driver
       token_descriptor.sync=1; // redundant, this call is always synchronous
       token_descriptor.directdma=0; // redundant, parallel mode requires intermediate buffering in pex mem
       token_descriptor.dmatarget= l_dma_target; // begin of pipe memory for writing. initial padding is done without dma though.
       token_descriptor.dmaburst=0; // is adjusted inside driver due to actual chain payload
       rev=ioctl(handle, PEX_IOC_REQUEST_RECEIVE_TOKENS, &token_descriptor);
       errsv = errno;
       if(rev)
           {
               printm(RON"ERROR>>"RES" mbspex_send_and_receive_parallel_tok -Error %d  on token request, sfp 0x%x toggle:0x%x - %s\n",errsv, l_sfp_p, l_toggle, strerror(errsv));
               return -1;
           }
       *pl_check_comm=token_descriptor.check_comm;
       *pl_check_token=token_descriptor.check_token;
       *pl_check_slaves=token_descriptor.check_numslaves;
       *pl_transfersize=token_descriptor.dmasize; // offset to adjust pipe pointer after call

 return rev;

  return 0;
}





/*****************************************************************************/

int  mbspex_send_and_receive_tok (int handle, long l_sfp, long l_toggle, unsigned long l_dma_target, unsigned long* pl_transfersize,
    long *pl_check_comm, long *pl_check_token, long *pl_check_slaves)

{
  int rev=0, errsv=0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
      struct pex_token_io token_descriptor;
#endif
      token_descriptor.bufid=l_toggle;
      token_descriptor.sfp=l_sfp;
      token_descriptor.sync=1;
      token_descriptor.directdma=1;
      token_descriptor.dmatarget= l_dma_target;
      token_descriptor.dmaburst=0;
      rev=ioctl(handle, PEX_IOC_REQUEST_TOKEN, &token_descriptor);
      errsv = errno;
      if(rev)
          {
              printm(RON"ERROR>>"RES" mbspex_send_and_receive_tok -Error %d  on token request, sfp 0x%x toggle:0x%x - %s\n",errsv, l_sfp, l_toggle, strerror(errsv));
              return -1;
          }
      *pl_check_comm=token_descriptor.check_comm;
      *pl_check_token=token_descriptor.check_token;
      *pl_check_slaves=token_descriptor.check_numslaves;
      *pl_transfersize=token_descriptor.dmasize;

return rev;
}


/*****************************************************************************/

 // sends token to all SFPs marked in l_sfp_p pattern: 1: sfp 0, 2: sfp 1,
  //                                                    4: sfp 2, 8: sfp 3,
  //                                                  0xf: all four SFPs
int  mbspex_send_tok (int handle, long l_sfp_p, long l_toggle)
{

  int rev=0, errsv=0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
       struct pex_token_io token_descriptor;
#endif
       token_descriptor.bufid=l_toggle;
       token_descriptor.sfp= (l_sfp_p << 16); // upper bytes expected as sfp pattern by driver
       token_descriptor.sync=0;
       token_descriptor.directdma=0;
       rev=ioctl(handle, PEX_IOC_REQUEST_TOKEN, &token_descriptor);
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
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
      struct pex_token_io token_descriptor;
#endif
      token_descriptor.sfp=l_sfp;
      token_descriptor.dmatarget= l_dma_target;
      token_descriptor.dmaburst=0;
      token_descriptor.directdma=0;
      if(l_dma_target==0) token_descriptor.directdma=1; // we disable the automatic DMA sending after token reception by this
      rev=ioctl(handle, PEX_IOC_WAIT_TOKEN, &token_descriptor);
      errsv = errno;
      if(rev)
          {
              printm(RON"ERROR>>"RES "Error %d  on wait token from channel 0x%x - %s\n",errsv,l_sfp, strerror(errsv));
              return -1;
          }
      *pl_check_comm=token_descriptor.check_comm;
      *pl_check_token=token_descriptor.check_token;
      *pl_check_slaves=token_descriptor.check_numslaves;
      *pl_transfersize=token_descriptor.dmasize;
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
int mbspex_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat)
{
  int rev = 0, errsv = 0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
  struct pex_reg_io reg_descriptor;
#endif
  mbspex_assert_handle(handle);
  reg_descriptor.address = l_address;
  reg_descriptor.value = l_dat;
  reg_descriptor.bar = s_bar;
  rev = ioctl (handle, PEX_IOC_WRITE_REGISTER, &reg_descriptor);
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
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
  struct pex_reg_io reg_descriptor;
#endif
  mbspex_assert_handle(handle);
  reg_descriptor.address = l_address;
  reg_descriptor.bar = s_bar;
  rev = ioctl (handle, PEX_IOC_READ_REGISTER, &reg_descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on reading from address 0x%lx (bar:%d)- %s\n", errsv, l_address,
        s_bar, strerror (errsv));
  }
  * l_dat=reg_descriptor.value;
  return rev;
}


int mbspex_dma_rd (int handle, unsigned int source, unsigned long dest, unsigned int size, unsigned int burst)
{
  int rev = 0, errsv = 0;;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
  struct pex_dma_io dma_descriptor;
#endif
  mbspex_assert_handle(handle);
  dma_descriptor.source = source;
  dma_descriptor.target = dest;
  dma_descriptor.size = size;
  dma_descriptor.burst=burst;
#if 0
  // JAM24 debug
  printm("libmbspex: mbspex_dma_rd: source:0x%x, target:0x%lx, size:0x%lx, burst:0x%x", dma_descriptor.source,  dma_descriptor.target,  dma_descriptor.size, dma_descriptor.burst);
  //
#endif
  rev = ioctl (handle, PEX_IOC_READ_DMA, &dma_descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on DMA reading 0x%x bytes from address 0x%lx  to 0x%lx (%s)\n", errsv, size, source,dest, strerror (errsv));
    return -1;
  }
  return dma_descriptor.size;
}


int mbspex_dma_rd_virt (int handle, unsigned int source, unsigned long virtdest, unsigned int size, unsigned int burst)
{
  int rev = 0, errsv = 0;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
    struct pex_dma_io dma_descriptor;
#endif
    mbspex_assert_handle(handle);
    dma_descriptor.source = source;
    dma_descriptor.virtdest = virtdest;
    dma_descriptor.size = size;
    dma_descriptor.burst=burst;
    rev = ioctl (handle, PEX_IOC_READ_DMA_PIPE, &dma_descriptor);
    errsv = errno;
    if (rev)
    {
      printm (RON"ERROR>>"RES"Error %d  on DMA reading 0x%x bytes from address 0x%lx  to 0x%lx (%s)\n", errsv, size, source, virtdest, strerror (errsv));
      return -1;
    }
    return dma_descriptor.size;
}

int mbspex_map_pipe (int handle, unsigned long startaddress, unsigned long size)
{
      int rev = 0, errsv = 0;;
#ifndef MBSPEX_IOCTL_GLOBAL_DESCRIPTORS
     struct pex_pipebuf pipe_descriptor;
#endif
      mbspex_assert_handle(handle);
      pipe_descriptor.addr = startaddress;
      pipe_descriptor.size = size;
      rev = ioctl (handle, PEX_IOC_MAP_PIPE, &pipe_descriptor);
      errsv = errno;
      if (rev)
      {
        printm (RON"ERROR>>"RES"Error %d  on mapping PIPE of 0x%x bytes from address 0x%lx  (%s)\n", errsv, size, startaddress,  strerror (errsv));
        return -1;
      }
      return pipe_descriptor.size;
}

int mbspex_unmap_pipe (int handle){
  int rev = 0, errsv = 0;;
       rev = ioctl (handle, PEX_IOC_UNMAP_PIPE);
       errsv = errno;
       if (rev)
       {
         printm (RON"ERROR>>"RES"Error %d  on unmapping PIPE (%s)\n", errsv, strerror (errsv));
         return -1;
       }
       return 0;
}

