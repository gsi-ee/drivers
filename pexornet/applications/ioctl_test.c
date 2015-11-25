/**
 * Test how ioctl to socket of network driver works
 * JAM 27-Oct-2015
 * */


#include "pexornet_user.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <libgen.h>
#include <stdlib.h>


void my_usage (const char *progname)
{
  printf ("***************************************************************************\n");
  printf (" %s for pexornet network driver example  \n", progname);
  printf (" v0.02 25-Nov-2015 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf (
      "  usage: %s [-h] [-z] [-i sfp numslaves] [[-r|-w] sfp slave address [value]]] \n",
      progname);
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -z        : reset (zero) pexor device \n");
  printf ("\t\t -i        : init  sfp chain\n");
  printf ("\t\t -r        : read from register at sfp, slave address \n");
  printf ("\t\t -w        : write value to register at sfp, slave, address\n");
}

int main (int argc, char *argv[])
{

int errsav=0;
int ioctlcode=0;
unsigned int cmdlen=0, i=0, opt=0;
struct ifreq ifr;

struct pexornet_bus_io busdat;
memset(&busdat,0,sizeof(busdat));

ifr.ifr_data= (void*) &busdat;

struct sockaddr_in* ipaddr; // for basic test

char if_name[128];
snprintf(if_name,128,"pex0");
optind = 1;
while ((opt = getopt (argc, argv, "hirwz")) != -1)
{
  switch (opt)
  {
    case '?':
      my_usage (basename (argv[0]));
      exit (1);
    case 'h':
      my_usage (basename (argv[0]));
      exit (0);
    case 'i':
      ioctlcode=PEXORNET_IOC_INIT_BUS;
      break;
    case 'w':
      ioctlcode=PEXORNET_IOC_WRITE_BUS;
      break;
    case 'r':
      ioctlcode=PEXORNET_IOC_READ_BUS;
      break;
    case 'z':
      ioctlcode=PEXORNET_IOC_RESET;
      break;

  };

} // while

cmdlen = argc - optind;
for (i = 0; i < cmdlen;  i++)
  {
    if (argv[optind + i])
    {
      if(i==0)
        busdat.sfp = strtol (argv[optind + i], NULL, 0);
      if(i==1)
        busdat.slave = strtol (argv[optind + i], NULL, 0);
      if(i==2)
         busdat.address = strtol (argv[optind + i], NULL, 0);
      if(i==3)
        busdat.value =  strtol (argv[optind + i], NULL, 0);
    }


    else
      printf ("warning: argument at position %d is empty!", optind + i);
  }


printf("Got arguments sfp=%d slave=%d add=0x%x val=0x%x \n",
    busdat.sfp, busdat.slave,busdat.address, busdat.value);


int fd;
printf("Testing ioctl on interface %s\n",if_name);
size_t if_name_len=strlen(if_name);
if (if_name_len<sizeof(ifr.ifr_name)) {
    memcpy(ifr.ifr_name,if_name,if_name_len);
    ifr.ifr_name[if_name_len]=0;
} else {
    printf("interface name is too long\n");
    exit(0);
}

fd=socket(AF_INET,SOCK_DGRAM,0);
if (fd==-1) {
    errsav=errno;
    printf("Error %d opening socket - %s \n",errsav, strerror(errsav));
    exit(0);
}



if (ioctl(fd,ioctlcode,&ifr)==-1) {
    errsav=errno;
    close(fd);
    printf("Error %d on ioctl 0x%x - %s \n",errsav, ioctlcode, strerror(errsav));
}
close(fd);



// evaluate return values:

switch(ioctlcode)
{
  case PEXORNET_IOC_READ_BUS:
    printf("Read from sfp %d slave %d - val(0x%x)=0x%x\n",busdat.sfp, busdat.slave, busdat.address, busdat.value);
    break;

  case PEXORNET_IOC_WRITE_BUS:
    printf("Wrote to sfp %d slave %d - val(0x%x)=0x%x\n",busdat.sfp, busdat.slave, busdat.address, busdat.value);
    break;

  case PEXORNET_IOC_INIT_BUS:
      printf("Initialized sfp %d with %d slaves.\n",busdat.sfp, busdat.slave);
      break;
  case PEXORNET_IOC_RESET:
    if(busdat.value==42)
      printf("Reset was successful!\n");
    else
      printf("Reset failed!\n");
    break;


  default:
//    ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
//    if(ipaddr)
//      printf("IP address: %s\n",inet_ntoa(ipaddr->sin_addr));
//    else
//      printf("zero IP address! \n");
    printf("Unknown ioctl code, never come here! \n");
    break;

};




return 0;
}
