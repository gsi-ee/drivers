

#include  "libpexornet.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>


#define RON  "\x1B[7m"
#define RES  "\x1B[0m"



int pexornet_open(int devnum)
{
  int filehandle,errsv;
  char devname[64];
  char fname[256];

  snprintf(devname,64,PEXORNAMEFMT, devnum);
  snprintf(fname,256,"/dev/%s",devname);
  /*printm("pexornet: opening %s...\n",fname);*/
  filehandle = open(fname, O_RDWR );
  errsv = errno;

  if (filehandle < 0)
  {
    printm("pexornet: error %d (%s) opening device %s...\n",errsv, strerror(errsv), fname);
  }
  return filehandle;
}

int pexornet_close(int handle)
{
  pexornet_assert_handle(handle);
  close(handle);
  /* add all cleanup actions here*/
}


int pexornet_reset (int handle)
{
  int rev, errsv=0;;
  pexornet_assert_handle(handle);
  printm ("pexornet: resetting pex device\n");
  rev = ioctl (handle, PEX_IOC_RESET);
  errsv = errno;
    if (rev)
    {
      printm ("\n\nError %d reseting pex device", errsv, strerror (errsv));
    }
    return rev;
}

/*****************************************************************************/

int  pexornet_slave_init (int handle, long l_sfp, long l_n_slaves)

{

  int rev = 0, errsv=0;
  struct pexornet_bus_io descriptor;
  pexornet_assert_handle(handle);
  descriptor.sfp = l_sfp;
  descriptor.slave = l_n_slaves;
  printm ("pexornet: initialize SFP chain %d with %d slaves\n", l_sfp, l_n_slaves);
  rev = ioctl (handle, PEX_IOC_INIT_BUS, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm ("\n\nError %d  on initializing channel %lx, maxdevices %lx - %s\n", errsv, l_sfp, l_n_slaves, strerror (errsv));
  }
  return rev;
}


/*****************************************************************************/
int pexornet_slave_wr (int handle, long l_sfp, long l_slave, long l_slave_off, long l_dat)
{
  int rev = 0, errsv=0;
  struct pexornet_bus_io descriptor;
  pexornet_assert_handle(handle);
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

int pexornet_slave_config (int handle, struct pexornet_bus_config* config)
{
  int rev = 0, errsv=0, i=0;
  pexornet_assert_handle(handle);
  if(!config) return -EINVAL;
  /* need to copy config structure to stack?*/
//  struct pexornet_bus_config theConfig;
//  for(i=0;(i<config->numpars) && (i< PEX_MAXCONFIG_VALS);++i){
//    theConfig.param[i]=config->param[i];
//    printf("pexornet_slave_config- s:%d d:%d a:0x%x val:0x%x\n",
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

int pexornet_get_configured_slaves(int handle , struct pexornet_sfp_links* setup)
{
  int rev = 0, errsv = 0;
  pexornet_assert_handle(handle);
  rev = ioctl (handle, PEXORNET_IOC_GET_SFP_LINKS, setup);
    errsv = errno;
    if (rev)
    {
      printm (RON"ERROR>>"RES"Error %d  on retrieving slave link configuration- %s\n", errsv, strerror (errsv));
    }
    return rev;
}




/*****************************************************************************/
int pexornet_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{
  int rev = 0, errsv=0;
  struct pexornet_bus_io descriptor;
  pexornet_assert_handle(handle);
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






