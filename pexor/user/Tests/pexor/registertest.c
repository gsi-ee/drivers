/*
 * pexortest_simple.c
 *
 *  Created on: 25.11.2009
 *      Author: J.Adamczewski-Musch
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <errno.h>
#include <time.h>

//#include "../../driver/pexor_user.h"
#include "pexor_user.h"

//#include "timing.h"




#define NUMARGS 5





/* will set debug and check Debugmode on
 * with Debugmode=0, debug output is supressed and buffer contents are not checked/cleared
 * -> for performance measurements */
#define PCIBAR 0

#define VERBOSEMODE 0
#define READMODE 0
#define WRITEMODE 1

static int Debugmode=VERBOSEMODE;
static int Writemode=READMODE;
static unsigned int Address=0;
static unsigned int Content=0;







int register_read(int filehandle)
{

	int rev = 0;
	int localbuf=0;
	struct pexor_reg_io descriptor;
	descriptor.bar=PCIBAR;
	descriptor.address=Address;
	descriptor.value=0;
  rev=ioctl(filehandle, PEXOR_IOC_READ_REGISTER, &descriptor);
	if(rev)
		{
			printf("\n\nError %d  on reading from register address %lx - %s\n",rev, Address, strerror(rev));
			return rev;
		}
	Content=descriptor.value;
	printf("\nRead value %x from address %lx. \n",Content, Address);
return 0;

}

int register_write(int filehandle)
{
  int rev = 0;
  struct pexor_reg_io descriptor;
  descriptor.bar=PCIBAR;
  descriptor.address=Address;
  descriptor.value=Content;
  rev=ioctl(filehandle, PEXOR_IOC_WRITE_REGISTER, &descriptor);
  if(rev)
    {
      printf("\n\nError %d  on writing register value %lx to address %lx - %s\n",rev,Content,Address, strerror(rev));
    }
  printf("\nWrote value %x to address %x .\n",Content, Address);
  return 0;

}

void close_exit(int filehandle)
{
	close(filehandle);
	exit(0);
}


void usage()
{
	printf("\n**** PEXOR registertest v0.5 1/2011, JAM GSI:\n");
  printf("\n**** arguments:\n");
	printf("\t registertest [mode] [address] [value] [Debugmode] \n");
	printf("\t\t mode - read(0) or write(1) [%d]\n",Writemode);
	printf("\t\t address - address on bar 0 [%x]\n",Address);
	printf("\t\t value - value to write [%x]\n",Content);
	printf("\t\t Debugmode - \n\t\t\t 0:no  \n\t\t\t 1:on [%d] \n",Debugmode);
	printf("**********************************\n");
	exit(0);
}


int main(int argc, char **argv)
{

	int filehandle;
	int rev;
	int rest;
	int bytes;
	char fname[256];




	if(argc > NUMARGS)
		usage();
	if(argc>1)
	{
		if(strstr("-h",argv[1])) usage();
		/*Writemode=atoi(argv[1]);*/
      sscanf(argv[1],"%x",&Writemode);
	}
	if(argc>2)
	{
// 		Address=atoi(argv[2]);
	  sscanf(argv[2],"%x",&Address);
  }
	if(argc>3)
	{
// 		Content=atoi(argv[3]);
    sscanf(argv[3],"%x",&Content);
	}
	if(argc>4)
	{
		Debugmode=atoi(argv[4]);
	}




  if(Debugmode>0)
    {
      printf("\n**** pexor register test ****\n");
      printf(" - set Mode:%d \n",Writemode);
      printf(" - set Address:0x%x\n",Address);
      printf(" - set Content:0x:%x\n",Content);
      printf(" - set Debugmode:%d\n",Debugmode);
    }





    /* open filehandle /dev/pexor*/
	snprintf(fname,256,"/dev/%s-%d",PEXORNAME,0);
	if(Debugmode) printf("PEXORtest is opening %s...\n",fname);
	filehandle = open(fname, O_RDWR );
	if (filehandle < 0)
			return filehandle;

  switch(Writemode)
  {
		  case READMODE:
      rev=register_read(filehandle);
      break;
      case WRITEMODE:
      default:
      rev=register_write(filehandle);
      break;
}
if(rev)
        {
      printf("\n\nError %d in register io\n",rev);
      //close_exit(filehandle);
        }
close_exit(filehandle);
return 0;
}
