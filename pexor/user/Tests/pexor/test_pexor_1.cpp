/*
 * test_pexor_1.cpp
 *
 *  Created on: 02.02.2010
 *      Author: J.Adamczewski-Musch
 *
 *  Example of usage for pexor library
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pexor/PexorOne.h"
#include "pexor/Benchmark.h"
#include "pexor/DMA_Buffer.h"
#include "pexor/User_Buffer.h"

/* id number of pexor board, can be different from 0
 * if more than one pexor is in same pc*/
#define BOARDID 0


#define NUMARGS 5

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





void usage()
{
	printf("\n**** test_pexor_1 arguments:\n");
	printf("\t test_pexor_1 [mem] [bufs] [pool] [Debugmode] \n");
	printf("\t\t mem - transferbuffer length (integer len) [%d]\n",Bufsize);
	printf("\t\t bufs - number of buffers for test transfer  [%d]\n",Flownum);
	printf("\t\t poolsize - number of allocated dma buffers [%d]\n",Bufnum);
	printf("\t\t Debugmode - \n\t\t\t 0: minimum \n\t\t\t 1: more, check buffers \n\t\t\t 2: print buf contents and lib debug output\n");
	printf("\t\t\t 3: do IRQ test \n\t\t\t 4: bus io test \n\t\t\t 5: register io test [%d] \n",Debugmode);
	printf("**********************************\n");
	exit(0);
}


int main(int argc, char **argv)
{




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

	printf("\n**** pexortest_lib ****\n");
	int bytes=Bufsize*sizeof(int);

	pexor::User_Buffer readbuffer(bytes);
	pexor::User_Buffer writebuffer(bytes); // pexor buffers will expand their size to n*PAGESIZE automatically
	if(bytes<writebuffer.Size())
		{
			printf("Bufsize is reset to next pagesize multiple...\n");
			bytes=writebuffer.Size();
			Bufsize=writebuffer.Length(); // integers
		}


	printf(" - set Bufsize:%d integers (%d bytes)\n",Bufsize,bytes);
	printf(" - set Flownum:%d\n",Flownum);
	printf(" - set Bufnum:%d\n",Bufnum);
	printf(" - set Debugmode:%d\n",Debugmode);

	if(Debugmode == 2 ) pexor::Logger::Instance()->SetMessageLevel(pexor::MSG_DEBUG);

	pexor::Benchmark bench;
	bench.TimerInit();


	printf("  Filling testbuffer...\n");
	srand(time(0));
	for (int i=0; i < readbuffer.Length(); ++i)
		{
			/* fill testbuffer with some numbers */
			*readbuffer.Cursor(i)=0;
			*writebuffer.Cursor(i)=0xFFFFFFFF/RAND_MAX * rand();
			if(Debugmode == 2)
				{
					if((i%10)==0) printf("\n");
					printf("%d...",writebuffer[i]);
				}
		}

	pexor::PexorOne board(BOARDID);
	if(!board.IsOpen())
		{
			printf("**** Could not open pexor board!\n");
			return 1;
		}

	if(Debugmode==3)
		{
			/* Test the IRQ*/
			int rev=board.Test_IRQ();
			if(rev)
				{
					printf("\n\nError %d in irq test\n",rev);
				}
			return 0;
		}
	if(Debugmode==4)
		{
			 /* Test the "fieldbus" io*/
			unsigned long address=2000;
			unsigned long value=41;
			unsigned long channel=0;
			unsigned long device=0;
			int rev=board.WriteBus(address,value,channel,device);
			if(rev)
				{
					printf("\n\nError %d in WriteBus\n",rev);
				}
			printf("\nWrote value %d to address %d in device %d of channel %d.\n",value, address, device, channel);
			rev=board.ReadBus(address,value,channel,device);
			if(rev)
				{
					printf("\n\nError %d in ReadBus\n",rev);
				}
			printf("\nRead value %d from address %d .\n",value, address, device, channel);


			return 0;
		}	
		
		
	if(Debugmode==5)
		{
			 /* Test the register io*/
			unsigned int address=0x100000; /* begin of mapped memory for pexor 2!*/
			unsigned int value=0xFFFFFFFF/RAND_MAX * rand();
			int rev=board.WriteRegister(0,address,value);
			if(rev)
				{
					printf("\n\nError %d in WriteRegister\n",rev);
				}
			printf("\nWrote value %d to address %d .\n",value, address);
			value=0;
			rev=board.ReadRegister(0,address,value);
			if(rev)
				{
					printf("\n\nError %d in ReadBus\n",rev);
				}
			printf("\nRead value %d from address %d. \n",value, address);

			rev=board.Read(&readbuffer,1);
			if(rev<0)
				{
					printf("\n\nError %d in Read()\n",rev);
				}
			printf("\nRead value %d from RAM. Number of read bytes:%d\n",readbuffer[0],rev);
			return 0;
		}


	////////////////////////////////////////////////////////
	// PIO test here:
	printf("PEXOR PIOTEST...  \n");
	if(Debugmode) printf("PIOTEST  Writing to PEXOR RAM ...\n");
	bench.ClockStart();
	bench.TimerStart();
	board.Write(&writebuffer,writebuffer.Length()); // integers
	double cycledelta=bench.TimerDelta();
	double clockdelta=bench.ClockDelta();
	bench.ShowRate("Clock:  PIO write", writebuffer.Size(), clockdelta); // bytes
	bench.ShowRate("Cycles: PIO write", writebuffer.Size() , cycledelta);


	if(Debugmode) printf("  Reading back from PEXOR RAM ...\n");
	bench.ClockStart();
	bench.TimerStart();
	board.Read(&readbuffer,readbuffer.Length());
	cycledelta=bench.TimerDelta();
	clockdelta=bench.ClockDelta();
	bench.ShowRate("Clock:  PIO read", readbuffer.Size(), clockdelta);
	bench.ShowRate("Cycles: PIO read", readbuffer.Size(), cycledelta);
	if(Debugmode)
	{
		if(readbuffer!=writebuffer) // will use equal operator== checking for errors
			readbuffer.ShowCompareErrors();
		else
			printf("No errors!\n");

	}


	////////////////////////////////////////////////////////
	// map DMA buffers here:
	int rev=board.Add_DMA_Buffers(bytes,Bufnum);
	if(rev)
		{
			printf("\n\nError %d on mapping dma buffers\n",rev);
			return rev;
		}
	///////////////////////////////////////////////////////
	// single DMA TEST
	printf("PEXOR Test single DMA...\n");
	bench.ClockStart();
	bench.TimerStart();
	board.SetDMA(pexor::DMA_SINGLE); // start the DMA by this
	pexor::DMA_Buffer* rcv=board.ReceiveDMA();
	if(!rcv)
		{
			printf("\n\nError receiving DMA buffer \n");
			return -1;
		}
	cycledelta=bench.TimerDelta();
	clockdelta=bench.ClockDelta();
	bench.ShowRate("Clock:  Single DMA read", rcv->Size(), clockdelta);
	bench.ShowRate("Cycles: Single DMA read", rcv->Size(), cycledelta);
	if(Debugmode)
		{
			if(*rcv != writebuffer) // will use equal operator== checking for errors
				rcv->ShowCompareErrors();
			else
				printf("No errors!\n");
		}

	///////////////////////////////////////////////////7
	// chained DMA test:
	double sumsize= (double) (Flownum) * (double) (Bufsize) * sizeof(int);
	printf("PEXOR Test flow DMA for data sum=%e...\n",sumsize);
	bench.ClockStart();
	bench.TimerStart();
	board.SetDMA(pexor::DMA_CHAINED); // start the DMA by this
	pexor::DMA_Buffer* nextrcv=0;
	for (int i=0; i< Flownum ; ++i)
		{
			nextrcv=board.ReceiveDMA();
			if(!nextrcv)
				{
					printf("\nError receiving DMA buffer %d\n",i);
					return -1;
				}
			if(Debugmode<0)
				{
					// do performance copy test:
					pexor::Buffer* buf1= &readbuffer;
					pexor::Buffer* buf2= nextrcv;
					*buf1 = *buf2;

				}
			else if(Debugmode)
				{
					if(*nextrcv != writebuffer)// will use equal operator== checking for errors
						{
							printf("\n Flow DMA Test has errors in received buffer %d \n",i);
							nextrcv->ShowCompareErrors();
						}
				}

			// need to free the buffer again:
			if(board.Free_DMA_Buffer(nextrcv))
				{
					printf("\nError Freeing DMA buffer %d\n", i);
					return -1;
				}

		} // for
	cycledelta=bench.TimerDelta();
	clockdelta=bench.ClockDelta();
	bench.ShowRate("Clock:  Flow DMA read", sumsize, clockdelta);
	bench.ShowRate("Cycles: Flow DMA read", sumsize, cycledelta);
	board.SetDMA(pexor::DMA_STOPPED);

return 0;
}
