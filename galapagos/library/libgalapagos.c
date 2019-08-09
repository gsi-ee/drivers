

#include  "libgalapagos.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>


#define RON  "\x1B[7m"
#define RES  "\x1B[0m"



int galapagos_open(int devnum)
{
  int filehandle,errsv;
  char devname[64];
  char fname[256];

  snprintf(devname,64,GAPGNAMEFMT, devnum);
  snprintf(fname,256,"/dev/%s",devname);
  /*printm("galapagos: opening %s...\n",fname);*/
  filehandle = open(fname, O_RDWR );
  errsv = errno;

  if (filehandle < 0)
  {
    printm("galapagos: error %d (%s) opening device %s...\n",errsv, strerror(errsv), fname);
  }
  return filehandle;
}

int galapagos_close(int handle)
{
  galapagos_assert_handle(handle);
  close(handle);
  /* add all cleanup actions here*/
}


int galapagos_reset (int handle)
{
  int rev, errsv=0;;
  galapagos_assert_handle(handle);
  printm ("galapagos: resetting gapg device...");
  rev = ioctl (handle, GAPG_IOC_RESET);
  errsv = errno;
  if (rev)
    {
      printm ("\n\nError %d reseting gapg device", errsv, strerror (errsv));
    }
  else
    printm(" done!\n");
  return rev;
}

/*****************************************************************************/






/*****************************************************************************/
int galapagos_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat)
{
  int rev = 0, errsv = 0;
  struct gapg_reg_io descriptor;
  galapagos_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.value = l_dat;
  descriptor.bar = s_bar;
  rev = ioctl (handle, GAPG_IOC_WRITE_REGISTER, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on writing value %0xlx to address %0xlx (bar:%d)- %s\n", errsv, l_dat, l_address,
        s_bar, strerror (errsv));
  }
  return rev;
}

int galapagos_register_rd (int handle, unsigned char s_bar, long l_address, long * l_dat)
{
  int rev = 0, errsv = 0;
  struct gapg_reg_io descriptor;
  galapagos_assert_handle(handle);
  descriptor.address = l_address;
  descriptor.bar = s_bar;
  rev = ioctl (handle, GAPG_IOC_READ_REGISTER, &descriptor);
  errsv = errno;
  if (rev)
  {
    printm (RON"ERROR>>"RES"Error %d  on reading from address 0x%lx (bar:%d)- %s\n", errsv, l_address,
        s_bar, strerror (errsv));
  }
  * l_dat=descriptor.value;
  return rev;
}



