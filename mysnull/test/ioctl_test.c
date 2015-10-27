/**
 * Test how ioctl to socket of network driver works
 * JAM 27-Oct-2015
 * */


#include "mysnull_ioctl.h"
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
  printf (" %s for snull network driver example  \n", progname);
  printf (" v0.01 27-Oct-2015 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf (
      "  usage: %s [-h] [-n ifname] [-s] [-z] [[-r|-w] sfp slave address [value]]] \n",
      progname);
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -z        : reset (zero) network device \n");
  printf ("\t\t -s        : show statistics of network device \n");
  printf ("\t\t -n        : specify interface name \n");
  printf ("\t\t -r        : read from register \n");
  printf ("\t\t -w        : write to  register\n");
}

int main (int argc, char *argv[])
{

int errsav=0;
int ioctlcode=0;
unsigned int cmdlen=0, i=0, opt=0;
struct ifreq ifr;

struct pex_bus_io snulldat;
memset(&snulldat,0,sizeof(snulldat));

ifr.ifr_data= (void*) &snulldat;

struct sockaddr_in* ipaddr; // for basic test

char if_name[128];
snprintf(if_name,128,"sn0");
optind = 1;
while ((opt = getopt (argc, argv, "hn:srwz")) != -1)
{
  switch (opt)
  {
    case '?':
      my_usage (basename (argv[0]));
      exit (1);
    case 'h':
      my_usage (basename (argv[0]));
      exit (0);
    case 'n':
      snprintf(if_name,128,"%s",optarg);
      printf ("parsed interface name = %s \n",optarg);
      break;
    case 's':
      ioctlcode=SIOCSNULLSTATS;
      break;
    case 'w':
      ioctlcode=SIOCSNULLWRITE;
      break;
    case 'r':
      ioctlcode=SIOCSNULLREAD;
      break;
    case 'z':
      ioctlcode=SIOCSNULLRESET;
      break;

  };

} // while

cmdlen = argc - optind;
for (i = 0; i < cmdlen;  i++)
  {
    if (argv[optind + i])
    {
      if(i==0)
        snulldat.sfp = strtol (argv[optind + i], NULL, 0);
      if(i==1)
        snulldat.slave = strtol (argv[optind + i], NULL, 0);
      if(i==2)
         snulldat.address = strtol (argv[optind + i], NULL, 0);
      if(i==3)
        snulldat.value =  strtol (argv[optind + i], NULL, 0);
    }


    else
      printf ("warning: argument at position %d is empty!", optind + i);
  }


printf("Got arguments sfp=%d slave=%d add=0x%x val=0x%x \n",
    snulldat.sfp, snulldat.slave,snulldat.address, snulldat.value);


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
  case SIOCSNULLREAD:
    printf("Read val(0x%x)=0x%x\n",snulldat.address, snulldat.value);
    break;

  case SIOCSNULLWRITE:
    printf("Wrote val(0x%x)=0x%x\n",snulldat.address, snulldat.value);
    break;

  case SIOCSNULLRESET:
    if(snulldat.value==42)
      printf("Reset was successful!\n");
    else
      printf("Reset failed!\n");
    break;


  case SIOCSNULLSTATS:
  default:
//    ipaddr = (struct sockaddr_in*)&ifr.ifr_addr;
//    if(ipaddr)
//      printf("IP address: %s\n",inet_ntoa(ipaddr->sin_addr));
//    else
//      printf("zero IP address! \n");
    printf("No stats yet! \n");
    break;

};




return 0;
}
