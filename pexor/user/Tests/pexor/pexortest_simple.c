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

#include "timing.h"




#define NUMARGS 5

#define PAGESIZE 0x1000 /* 4096 bytes*/
/* pexor ramsize is 0x6000 */
/* #define TESTBUFSIZE 0x4000/sizeof(int) */
#define TESTBUFSIZE 1024
#define DMABUFNUM 7
#define FLOWSIZE 500



/* will set debug and check Debugmode on
 * with Debugmode=0, debug output is supressed and buffer contents are not checked/cleared
 * -> for performance measurements */
#define VERBOSEMODE 0

static int Debugmode=VERBOSEMODE;
static int Bufsize=TESTBUFSIZE;
static int Bufnum=DMABUFNUM;
static int Flownum=FLOWSIZE;

static int* wbuffer=0;
static int* rbuffer=0;
static int* tbuffer=0;
static void** dmabuffers=0;




void init_buffers()
{
	int i=0;
	printf("  Alloc buffers...\n");
	wbuffer = (int*) malloc(Bufsize*sizeof(int));
	rbuffer = (int*) malloc(Bufsize*sizeof(int));
	tbuffer = (int*) malloc(Bufsize*sizeof(int));
	dmabuffers= (void*)  malloc(Bufnum*sizeof(void*));
	printf("  Filling testbuffer...\n");
	srand(time(0));
	for(i=0; i<Bufsize;++i)
	{
		/* fill testbuffer with some numbers */
		rbuffer[i]=0;
		/* wbuffer[i]=i;*/
		wbuffer[i]=0xFFFFFFFF/RAND_MAX * rand();
		if(Debugmode == 2 )
			{
				if((i%10)==0) printf("\n");
				printf("%d...",wbuffer[i]);
			}
	}
	printf("\n\n");

}


int compare_buffers(int* original, int* different, int buflen)
{
	int i=0, ercnt=0;
	if(!Debugmode) return 0;
	if(Debugmode > 0)
		{
			printf("Comparing buffers...\n");
		}

	for(i=0; i<buflen;++i)
	{

		if(Debugmode >0)
		{
			if(original[i]!=different[i])
				{
					printf("\n### content mismatch for entry %d: %d != %d\n",i,original[i],different[i]);
					ercnt++;
				}

			if(Debugmode >6)
				{
					printf("%d...",different[i]);
					if((i%10)==0) printf("\n");
				}
		}
		else if(Debugmode== -1 )
				{
				  /* memory copy speed test from dma buffer*/
				  tbuffer[i]=different[i];
				}
		else if(Debugmode== -2)
				{
					 /*memory copy speed test from user space buffer*/
					tbuffer[i]=original[i];
				}
		different[i]=0;

	}
	if(Debugmode>0) printf("\n\nFound %d errors (ratio %e) .\n",ercnt, (float) ercnt / (float) buflen);


}


int piotest(int filehandle)
{
	int rev=0;
	struct timespec deltatime;
	double clockdelta, cycledelta;
	double rate=0;
	printf("PEXOR PIOTEST...  \n");
	if(Debugmode) printf("PIOTEST  Writing to PEXOR RAM ...\n");
	Pexortest_ClockStart();
	Pexortest_TimerStart();
	rev=pwrite(filehandle,wbuffer,Bufsize*sizeof(int),0);
	if(rev<0)
	{
		printf("ERROR %d on writing",rev);
		return rev;
	}
	cycledelta=Pexortest_TimerDelta();
	clockdelta=Pexortest_ClockDelta();
	Pexortest_ShowRate("Clock:  PIO write", Bufsize*sizeof(int), clockdelta);
	Pexortest_ShowRate("Cycles: PIO write", Bufsize*sizeof(int), cycledelta);



	if(Debugmode) printf("done\n\n");

	if(Debugmode) printf("  Reading back from PEXOR RAM ...\n");
	Pexortest_ClockStart();
	Pexortest_TimerStart();
		rev=pread(filehandle,rbuffer,Bufsize*sizeof(int),0);
		if(rev<0)
			{
				printf("ERROR %d on reading",rev);
				return rev;
			}
	cycledelta=Pexortest_TimerDelta();
	clockdelta=Pexortest_ClockDelta();
	Pexortest_ShowRate("Clock:  PIO read", Bufsize*sizeof(int), clockdelta);
	Pexortest_ShowRate("Cycles: PIO read", Bufsize*sizeof(int), cycledelta);
	printf("done\n\n");
	compare_buffers(wbuffer, rbuffer, Bufsize);

return 0;
}

int receive_dma_buffer(int filehandle)
{
	int rev;
	struct pexor_userbuf descriptor;
	int* rcvbuffer;
	if(Debugmode>0) printf("Wait for buffer...\n");
	rev=ioctl(filehandle, PEXOR_IOC_WAITBUFFER, &descriptor);
	if(rev)
		{
			printf("\n\nError %d waiting for next receive buffer \n",rev);
			return rev;
		}
	rcvbuffer=(int*) descriptor.addr;
	if(descriptor.size>Bufsize*sizeof(int))
		{
			if(Debugmode>0) printf("receive_dma_buffer Note: mapped buffer len %d is larger than test buffer size %d",descriptor.size, Bufsize*sizeof(int));
		}
	else if(descriptor.size<Bufsize*sizeof(int))
		{
			printf("Error: length mismatch between buf descriptor len %d and test buffer size %d",descriptor.size, Bufsize*sizeof(int));
			return -1;
		}

	compare_buffers(wbuffer, rcvbuffer, Bufsize);

	/* put buffer back*/
	if(Debugmode>0) printf("  Freeing DMA readbuffer...\n");
	rev=ioctl(filehandle, PEXOR_IOC_FREEBUFFER, &descriptor);
	if(rev)
		{
			printf("\n\nError %d freeing  buffer %lx \n",rev, descriptor.addr);
		}
	return rev;

}


int single_dma_test(int filehandle)
{
	int rev=0;
	int state;
	double clockdelta, cycledelta;;
	double rate=0;
	struct pexor_userbuf descriptor;
	int* rcvbuffer;
	printf("PEXOR Test single DMA...\n");
	state=PEXOR_STATE_DMA_SINGLE;
	Pexortest_ClockStart();
	Pexortest_TimerStart();
	rev=ioctl(filehandle, PEXOR_IOC_SETSTATE, &state); /* will initiate DMA*/
	if(rev)
		{
			printf("\n\nError %d setting to state %d\n",rev,state);
			return rev;
		}
	/* sleep(1);  DEBUG: avoid the wait event stuff*/
	rev=receive_dma_buffer(filehandle);
	if(rev)
		{
			printf("\n\nError %d receiving DMA buffer \n",rev);
		}
	cycledelta=Pexortest_TimerDelta();
	clockdelta=Pexortest_ClockDelta();
	Pexortest_ShowRate("Clock: Single DMA read", Bufsize*sizeof(int), clockdelta);
	Pexortest_ShowRate("Cycles: Single DMA read", Bufsize*sizeof(int), cycledelta);


return rev;


}


int flow_dma_test(int filehandle)
{
	int i,rev=0;
	int state;
	double sumsize;
	double clockdelta, cycledelta;;
	double rate=0;
	struct pexor_userbuf descriptor;
	int* rcvbuffer;
	sumsize= (double) (Flownum) * (double) (Bufsize) * sizeof(int);

	printf("PEXOR Test flow DMA for data sum=%e...\n",sumsize);
	state=PEXOR_STATE_DMA_FLOW;
	Pexortest_ClockStart();
	Pexortest_TimerStart();
	rev=ioctl(filehandle, PEXOR_IOC_SETSTATE, &state); /* will initiate DMA*/
	if(rev)
		{
			printf("\n\nError %d setting to state %d\n",rev,state);
			return rev;
		}
	for (i=0; i< Flownum ; ++i)
	{
		rev=receive_dma_buffer(filehandle);
		if(rev)
		{
			printf("\n\nError %d receiving DMA buffer \n",rev);
			return rev;
		}

	}
	cycledelta=Pexortest_TimerDelta();
	clockdelta=Pexortest_ClockDelta();
	Pexortest_ShowRate("Clock: Flow DMA read", sumsize, clockdelta);
	Pexortest_ShowRate("Cycles: Flow DMA read", sumsize, cycledelta);
	printf("PEXOR Test flow DMA finally clearing outstanding receive buffers...\n");
	rev=ioctl(filehandle, PEXOR_IOC_CLEAR_RCV_BUFFERS);
	if(rev)
		{
			printf("\n\nError %d clearing receive buffers\n",rev);
			return rev;
		}

	return 0;
}

int map_dma_buffers(int filehandle)
{
	int i,er;
	struct rlimit locklimits;
	/* analyze the current system limits here:*/
	if(getrlimit(RLIMIT_MEMLOCK,&locklimits))
		{
			er=errno;
			printf("\n\nError getting map locked limits to %d bytes, errno=%d - %s",Bufnum*Bufsize*sizeof(int),er,strerror(er));
			return -1;
		}
	printf("\n\nFound previous locked limits of current=%d bytes, max=%d bytes",locklimits.rlim_cur,locklimits.rlim_max);


	locklimits.rlim_cur=Bufnum*Bufsize*sizeof(int);
	locklimits.rlim_max=Bufnum*Bufsize*sizeof(int);
	/* use setrlimit to increase allowed locked pages size:*/
	if(setrlimit(RLIMIT_MEMLOCK,&locklimits))
	{
		er=errno;
		printf("\n\nError setting map locked limits to %d bytes, errno=%d - %s",Bufnum*Bufsize*sizeof(int),er,strerror(er));
		return -1;
	}
	printf("\nSet new locked limits of current=%d bytes, max=%d bytes \n",locklimits.rlim_cur,locklimits.rlim_max);

	/* Map buffer tests:*/
	for(i=0; i<Bufnum;++i)
	{
		dmabuffers[i]=mmap(0,Bufsize*sizeof(int), PROT_READ | PROT_WRITE , MAP_SHARED | MAP_LOCKED , filehandle, 0); /* | PROT_WRITE , | MAP_LOCKED*/
		if(dmabuffers[i]==MAP_FAILED)
			{
				er=errno;
				printf("\n\nError Mapping buffer %d, errno=%d - %s\n",i,er,strerror(er));
				return -1;
			}
		if(Debugmode>0) printf("\n\nMapped buffer %d to address %lx\n",i,dmabuffers[i]);
	}
	return 0;
}

int delete_dma_buffers(int filehandle)
{
	int i,rev=0;;
	struct pexor_userbuf descriptor;
	/* MUST clean up our buffers before leaving driver (otherwise unvalid virtual adresses as buffer ids for next run!):*/
	printf("  Deleting all DMA buffers...\n");
	for(i=0; i<Bufnum;++i)

		{
			descriptor.addr= (unsigned long) dmabuffers[i];
			rev=ioctl(filehandle, PEXOR_IOC_DELBUFFER , &descriptor);
			if(rev)
				{
					printf("\n\nError %d deleting buffer %d to address %lx\n",rev,i,dmabuffers[i]);
					break;
				}
			if(Debugmode>0) printf("\n\nDeleted buffer %d at address %lx\n",i,dmabuffers[i]);

		}

	return rev;
}

int irqtest(int filehandle)
{
	int state,rev=0;
	struct timespec deltatime;
	double clockdelta, cycledelta;
	double rate=0;
	printf("PEXOR IRQTEST...  \n");
	state=PEXOR_STATE_IR_TEST;
	rev=ioctl(filehandle, PEXOR_IOC_SETSTATE, &state);
	if(rev)
		{
			printf("\n\nError %d setting to state %d\n",rev,state);
			return rev;
		}
	return 0;

}




int fieldbus_test(int filehandle)
{

	/* Test the "fieldbus" io*/
	int rev=0;
	unsigned long address=2000;
	unsigned long value=41;
	struct pexor_bus_io descriptor;
	descriptor.address=address;
	descriptor.value=value;
	printf("PEXOR BUS IO TEST...  \n");
	rev=ioctl(filehandle, PEXOR_IOC_WRITE_BUS, &descriptor);
	if(rev)
		{
			printf("\n\nError %d  on writing value %lx to address %lx - %s\n",rev,value,address, strerror(rev));
		}
	printf("\nWrote value %d to address %d .\n",value, address);
	descriptor.address=address;
	descriptor.value=0;
	rev=ioctl(filehandle, PEXOR_IOC_READ_BUS, &descriptor);
	if(rev)
		{
			printf("\n\nError %d  on reading from address %lx - %s\n",rev, address, strerror(rev));
			return rev;
		}
	value=descriptor.value;
	printf("\nRead value %d from address %d .\n",value, address);
	return rev;
}


int register_io_test(int filehandle)
{

	/* Test the register io*/
	unsigned int address=0x100000; /* begin of mapped memory for PEXOR 2, use for PEXOR1: 0x200000*/
	unsigned int value=0xFFFFFFFF/RAND_MAX * rand();
	int rev = 0, er = 0;
	int localbuf=0;
	struct pexor_reg_io descriptor;
	descriptor.bar=0;
	descriptor.address=address;
	descriptor.value=value;
	rev=ioctl(filehandle, PEXOR_IOC_WRITE_REGISTER, &descriptor);
	if(rev)
		{
			printf("\n\nError %d  on writing register value %lx to address %lx - %s\n",rev,value,address, strerror(rev));
		}
	printf("\nWrote value %d to address %d .\n",value, address);
	descriptor.address=address;
	descriptor.value=0;
	descriptor.bar=0;
	rev=ioctl(filehandle, PEXOR_IOC_READ_REGISTER, &descriptor);
	if(rev)
		{
			printf("\n\nError %d  on reading from register address %lx - %s\n",rev, address, strerror(rev));
			return rev;
		}
	value=descriptor.value;


	printf("\nRead value %d from address %d. \n",value, address);

	rev = pread(filehandle, &localbuf, 1 * sizeof (int), 0);
	if(rev<0)
	{
		er=errno;
		printf("ERROR %d on reading - %s",er, strerror(er));
	}
	printf("\nRead value %d from RAM. Number of read bytes:%d\n",localbuf,rev);



		return 0;

}

int trigger_test(int filehandle)
{
  int state,rev=0,t;
  struct pexor_trixor_set desc;

  printf("PEXOR TRIGGERTEST...  \n");
  desc.command=PEXOR_TRIX_RES;
  rev=ioctl(filehandle, PEXOR_IOC_SET_TRIXOR, &desc);
  if(rev)
      {
              printf("\n\nError %d resetting trixor to %d\n",rev,desc.command);
              return rev;
      }

  desc.command=PEXOR_TRIX_TIMESET;
  desc.cvt=0x200; /* conversion time in 100ns units*/
  desc.fct=0x100; /* fast cleartime in 100ns units*/
    rev=ioctl(filehandle, PEXOR_IOC_SET_TRIXOR, &desc);
    if(rev)
        {
                printf("\n\nError %d resetting trixor to %d\n",rev,desc.command);
                return rev;
        }



  desc.command=PEXOR_TRIX_GO;
  rev=ioctl(filehandle, PEXOR_IOC_SET_TRIXOR, &desc);
  if(rev)
      {
              printf("\n\nError %d set trixor to  %d\n",rev,desc.command);
              return rev;
      }
  printf("     ...set to GO  \n");
  /* now do a loop: wait for trigger, read out some data, reset dt flag */
  for(t=0;t<Flownum; ++t)
    {
        rev=ioctl(filehandle, PEXOR_IOC_WAIT_TRIGGER);
        if(rev)
            {
                  printf("\n\nError %d waiting for trigger at cycle %d\n",rev,t );
                  return rev;
            }
        single_dma_test(filehandle);
        desc.command=PEXOR_TRIX_RES;
        rev=ioctl(filehandle, PEXOR_IOC_SET_TRIXOR, &desc);
         if(rev)
             {
                     printf("\n\nError %d resetting trixor to  %d\n",rev,desc.command);
                     return rev;
             }

    }
   desc.command=PEXOR_TRIX_HALT;
   rev=ioctl(filehandle, PEXOR_IOC_SET_TRIXOR, &desc);
   if(rev)
       {
               printf("\n\nError %d set trixor to  %d\n",rev,desc.command);
               return rev;
       }
   printf("     ...set to HALT  \n");



  return 0;




}


void close_exit(int filehandle)
{
	close(filehandle);
	free (wbuffer);
	free (rbuffer);
	free (tbuffer);
	free (dmabuffers);
	exit(0);
}


void usage()
{
	printf("\n**** pexortest_simple arguments:\n");
	printf("\t pexortest_simple [mem] [bufs] [pool] [Debugmode] \n");
	printf("\t\t mem - transferbuffer length (integer len) [%d]\n",Bufsize);
	printf("\t\t bufs - number of buffers for test transfer  [%d]\n",Flownum);
	printf("\t\t poolsize - number of allocated dma buffers [%d]\n",Bufnum);
	printf("\t\t Debugmode - \n\t\t\t 0:minimum \n\t\t\t 1:more, check buffers \n\t\t\t 2: also print buf contents \n");
	printf("\t\t\t 3: do IRQ test  \n\t\t\t 4: bus io test \n\t\t\t 5: register io test \n\t\t\t 6: triggered io test\n\t\t\t-1: With copy of DMAbuf->userbuf \n\t\t\t-2: With dummy copy userbuf->userbuf   [%d] \n",Debugmode);
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
		Bufsize=atoi(argv[1]);
	}
	if(argc>2)
	{
		Flownum=atoi(argv[2]);
	}
	if(argc>3)
	{
		Bufnum=atoi(argv[3]);
	}
	if(argc>4)
	{
		Debugmode=atoi(argv[4]);
	}

	printf("\n**** pexortest_simple ****\n");
	bytes=Bufsize*sizeof(int);
	rest=bytes % PAGESIZE;
	if(rest)
		{
			bytes=bytes-rest + PAGESIZE;
			Bufsize=bytes/sizeof(int);
			printf(" - resetting Bufsize as next PAGESIZE multiple...\n");
		}
	printf(" - set Bufsize:%d integers (%d bytes)\n",Bufsize,bytes);
	printf(" - set Flownum:%d\n",Flownum);
	printf(" - set Bufnum:%d\n",Bufnum);
	printf(" - set Debugmode:%d\n",Debugmode);



	init_buffers();

	Pexortest_TimerInit();


    /* open filehandle /dev/pexor*/
	snprintf(fname,256,"/dev/%s-%d",PEXORNAME,0);
	printf("PEXORtest is opening %s...\n",fname);
	filehandle = open(fname, O_RDWR );
	if (filehandle < 0)
			return filehandle;
	printf("PEXOR Test resetting driver...\n");
	rev=ioctl(filehandle, PEXOR_IOC_RESET); /* cleanup previous buffers*/
	if(rev)
		{
			printf("\n\nError %d resetting driver \n",rev);
			close_exit(filehandle);
		}
	if(Debugmode==3)
		{
			rev=irqtest(filehandle);
			if(rev)
				{
					printf("\n\nError %d in irq test\n",rev);
					close_exit(filehandle);
				}
			close_exit(filehandle);
		}

	if(Debugmode==4)
		{
			rev=fieldbus_test(filehandle);
			if(rev)
				{
					printf("\n\nError %d in fieldbus test\n",rev);
					close_exit(filehandle);
				}
			close_exit(filehandle);
			return 0;
		}


	if(Debugmode==5)
		{
			rev=register_io_test(filehandle);
			if(rev)
				{
					printf("\n\nError %d in register io test\n",rev);
					close_exit(filehandle);
				}
			close_exit(filehandle);
		}



	/*printf("PEXOR Test executing test...\n");
	rev=ioctl(filehandle, PEXOR_IOC_TEST, &Bufsize);  Test to access ram from within driver
		if(rev)
			{
				printf("\n\nError %d testing driver\n",rev);
				close_exit(filehandle);
			}*/
	/*close_exit(filehandle);*/

	rev=piotest(filehandle);
	if(rev)
		{
			printf("\n\nError %d in RAM pio test\n",rev);
			close_exit(filehandle);
		}
	/*close_exit(filehandle);*/
	/* RAMSIZE test *************************/


	rev=map_dma_buffers(filehandle);
	if(rev)
		{
			printf("\n\nError %d on mapping dma buffers\n",rev);
			close_exit(filehandle);
		}

	if(Debugmode==6)
              {
                rev=trigger_test(filehandle);
                if(rev)
                        {
                                printf("\n\nError %d in trigger test\n",rev);
                                close_exit(filehandle);
                        }
                close_exit(filehandle);
              }



	rev=single_dma_test(filehandle);
	if(rev)
		{
			printf("\n\nError %d in single dma test\n",rev);
			close_exit(filehandle);
		}

	rev=flow_dma_test(filehandle);
	if(rev)
		{
			printf("\n\nError %d in flow dma test\n",rev);
			close_exit(filehandle);
		}

	rev=delete_dma_buffers(filehandle);
	if(rev)
		{
			printf("\n\nError %d on deleting dma buffers\n",rev);
			close_exit(filehandle);
		}

close_exit(filehandle);
return 0;
}
