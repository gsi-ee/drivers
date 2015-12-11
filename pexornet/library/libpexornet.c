

#include  "libpexornet.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdlib.h>


#define RON  "\x1B[7m"
#define RES  "\x1B[0m"


pexornet_handle_t* pexornet_open(int devnum)
{
  int sockhandle,errsv;
  sockhandle=socket(AF_INET,SOCK_DGRAM,0);
  errsv=errno;
  if (sockhandle < 0)
  {
    printm("pexornet: error %d (%s) opening socket handle for interface %d...\n",errsv, strerror(errsv), devnum);
    return 0;
  }
  pexornet_handle_t* handle= (pexornet_handle_t*) malloc(sizeof(pexornet_handle_t));
  errsv=errno;
  if(handle==0)
  {
    printm("pexornet: error %d (%s) allocating handle memory for interface %d...\n",errsv, strerror(errsv), devnum);
    return 0;
  }
  memset(handle,0,sizeof(pexornet_handle_t));
  handle->fSockhandle=sockhandle;
  snprintf(handle->fIfreq.ifr_name,sizeof(handle->fIfreq.ifr_name),PEXORIFNAMEFMT, devnum);
  //printm("pexornet: opening socket for interface %s...\n",handle->fIfreq.ifr_name);


  return handle;
}

int pexornet_close(pexornet_handle_t* handle)
{
  int res=0, errsv=0;
  pexornet_assert_handle(handle);
  res=close(handle->fSockhandle);
  errsv=errno;
  if(res<0)
    {
      printm("pexornet: error %d (%s) closing socket handle for %s...\n",errsv, strerror(errsv), handle->fIfreq.ifr_name);
      return res;
    }
  free(handle);
  /* add other cleanup actions here*/
  return res;
}


int pexornet_reset (pexornet_handle_t* handle)
{
  int rev, errsv=0;;
  pexornet_assert_handle(handle);

  printm ("pexornet: resetting pex device\n");
  rev = ioctl (handle->fSockhandle, PEXORNET_IOC_RESET, &(handle->fIfreq));
  errsv = errno;
    if (rev)
    {
      printm ("\n\nError %d reseting pex device", errsv, strerror (errsv));
    }
    return rev;
}

/*****************************************************************************/

int  pexornet_slave_init (pexornet_handle_t* handle, long l_sfp, long l_n_slaves)

{

  int rev = 0, errsv=0;
  struct pexornet_bus_io descriptor;

  pexornet_assert_handle(handle);
  descriptor.sfp = l_sfp;
  descriptor.slave = l_n_slaves;
  handle->fIfreq.ifr_data= (void*) &descriptor;
  printm ("pexornet: initialize SFP chain %d with %d slaves\n", l_sfp, l_n_slaves);
  rev = ioctl (handle->fSockhandle, PEXORNET_IOC_INIT_BUS, &(handle->fIfreq));
  errsv = errno;
  if (rev)
  {
    printm ("\n\nError %d  on initializing channel %lx, maxdevices %lx - %s\n", errsv, l_sfp, l_n_slaves, strerror (errsv));
  }
  handle->fIfreq.ifr_data=0;
  return rev;
}


/*****************************************************************************/
int pexornet_slave_wr (pexornet_handle_t* handle, long l_sfp, long l_slave, long l_slave_off, long l_dat)
{
  int rev = 0, errsv=0;
  struct pexornet_bus_io descriptor;
  pexornet_assert_handle(handle);
  //PexorInfo("WriteBus writes %x to %x \n",value, address);
  descriptor.address = l_slave_off;
  descriptor.value = l_dat;
  descriptor.sfp = l_sfp;
  descriptor.slave = l_slave;
  handle->fIfreq.ifr_data= (void*) &descriptor;
  rev = ioctl (handle->fSockhandle, PEXORNET_IOC_WRITE_BUS, &(handle->fIfreq));
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on writing value 0x%lx to address 0x%lx (sfp:%d, slave:%d)- %s\n", errsv, l_dat,  l_slave_off, l_sfp,
        l_slave, strerror (errsv));
  }
  handle->fIfreq.ifr_data=0;
  return rev;
}

int pexornet_slave_config (pexornet_handle_t* handle, struct pexornet_bus_config* config)
{
  int rev = 0, errsv=0, i=0;
  pexornet_assert_handle(handle);
  if(!config) return -EINVAL;

  handle->fIfreq.ifr_data= (void*) config;
  rev = ioctl (handle->fSockhandle, PEXORNET_IOC_CONFIG_BUS, &(handle->fIfreq));
   errsv = errno;
    if (rev)
    {
      printm (RON"ERROR>>"RES"Error %d  on writing configuration to bus- %s\n", errsv, strerror (errsv));
    }
    handle->fIfreq.ifr_data=0;
    return rev;
}

int pexornet_get_configured_slaves(pexornet_handle_t* handle , struct pexornet_sfp_links* setup)
{
  int rev = 0, errsv = 0;
  pexornet_assert_handle(handle);
  handle->fIfreq.ifr_data= (void*) setup;
  rev = ioctl (handle->fSockhandle, PEXORNET_IOC_GET_SFP_LINKS,  &(handle->fIfreq));

  errsv = errno;
  if (rev)
    {
      printm (RON"ERROR>>"RES"Error %d  on retrieving slave link configuration- %s\n", errsv, strerror (errsv));
    }
  handle->fIfreq.ifr_data=0;
  return rev;
}




/*****************************************************************************/
int pexornet_slave_rd (pexornet_handle_t* handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{
  int rev = 0, errsv=0;
  struct pexornet_bus_io descriptor;
  pexornet_assert_handle(handle);
  descriptor.address = l_slave_off;
  descriptor.value = 0;
  descriptor.sfp = l_sfp;
  descriptor.slave = l_slave;
  handle->fIfreq.ifr_data= (void*) &descriptor;
  rev = ioctl (handle->fSockhandle, PEXORNET_IOC_READ_BUS, &(handle->fIfreq));
  handle->fIfreq.ifr_data=0;
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES" Error %d  on reading from address 0x%lx (sfp:%d, slave:%d)- %s\n", errsv, l_slave_off, l_sfp, l_slave,
        strerror (errsv));
    return rev;
  }
  *l_dat = descriptor.value;
  //printm("pexornet_slave_rd on sfp:%d sl:%d has value(0x%x)=0x%x\n",l_sfp,l_slave, l_slave_off, *l_dat);
  return 0;

}






