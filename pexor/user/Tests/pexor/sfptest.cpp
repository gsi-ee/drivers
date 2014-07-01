/*
 * sfptest.cpp
 *
 *  Created on: 19.03.2010
 *      Author: J.Adamczewski-Musch
 *
 *  Test sfp with connected slaves (exploder boards) at pexor2/3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pexor/PexorTwo.h"
#include "pexor/Benchmark.h"
#include "pexor/DMA_Buffer.h"
#include "pexor/User_Buffer.h"


// address map for slave (exploder): this is user specific data concerning the pexor board, so it is not available from PexorTwo.h
#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC









#define NUMARGS 6

#define TESTBUFSIZE 0x10000
#define DMABUFNUM 30


#define NUMSLAVES 1
#define EXPLODERBUF0 0x000000
#define EXPLODERBUF1 0x100000
#define EXPLODERLEN  6100 
//7200
//0x384

//#define FLOWSIZE 500

//#define WITHTRIGGER 1//
#define MAXTRIGGERS 100

/* will set debug and check Debugmode on
 * with Debugmode=0, debug output is supressed and buffer contents are not checked/cleared
 * -> for performance measurements */
#define TESTMODE 0

static int Mode=TESTMODE;
static int Bufsize=TESTBUFSIZE;
static int Bufnum=DMABUFNUM;
static int Maxslaves=NUMSLAVES;

//static int Flownum=FLOWSIZE;
static int Channel=0;
static int Slave=0;
static int Address=0;
static int Data=0;
static int BufID=0;




void usage()
{
	printf("\n**** sfptest arguments:\n");
	printf("\t sfptest [Mode] [channel] [slave (token mode: bufid)] [address] [data] \n");
	printf("\t\t Mode - \n\t\t\t 0: i/o looptest over all slaves \n\t\t\t 1: write single address \n\t\t\t 2: read single address\n");
	printf("\t\t\t 3: request token read \n\t\t\t 4: read from address relative to pexor board mem [%d]\n",Mode);
	printf("\t\t channel - sfp channel [%d]\n",Channel);
	printf("\t\t slave/bufid - connected device id, or token mode double buffer id  [%d]\n",Slave);
	printf("\t\t address - address on device [%d]\n",Address);
	printf("\t\t data - value to write to address [%d]\n",Data);

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
		Mode=atoi(argv[1]);
	}
	if(argc>2)
	{
		//Channel=atoi(argv[2]);
		sscanf (argv[2], "%x", &Channel);
	}
	if(argc>3)
	{
		sscanf (argv[3], "%x", &Slave);
		//Slave=atoi(argv[3]);
		BufID=Slave;
	}
	if(argc>4)
	{
		sscanf (argv[4], "%x", &Address);
		//Address=atoi(argv[4]);
	}
	if(argc>5)
	{
		sscanf (argv[5], "%x", &Data);
		//Data=atoi(argv[5]);
	}


	printf("\n**** sfptest ****\n");

    //int bytes=Bufsize*sizeof(int);
	int bytes=EXPLODERLEN*sizeof(int); // bytes per submemory

	pexor::User_Buffer readbuffer(bytes);
	pexor::User_Buffer writebuffer(bytes);

	// evaluate the real dma buffer size:
//	pexor::D_Buffer dummybuffer(Bufsize*sizeof(int));// pexor buffers will expand their size to n*PAGESIZE automatically
//	if(Bufsize*sizeof(int)<dummybuffer.Size())
//		{
//			printf("Bufsize is reset to next pagesize multiple...\n");
//			bytes=dummybuffer.Size();
//			Bufsize=dummybuffer.Length(); // integers
//		}

	bytes=Bufsize*sizeof(int);
	bytes=pexor::Buffer::NextPageSize(bytes);
	Bufsize=bytes/sizeof(int);

	printf(" - set Mode:%d\n",Mode);
	printf(" - set DMA Bufsize:%d integers (%d bytes)\n",Bufsize,bytes);
	printf(" - set sfp Channel:0x%x\n",Channel);
	if(Mode==3)
		printf(" - set token mode double buffer:%d \n",BufID);
	else
		printf(" - set slave device:0x%x \n",Slave);
    printf(" - set address:0x%x\n",Address);
    printf(" - set data value:0x%x\n",Data);

	//if(Debugmode == 2 ) 
	pexor::Logger::Instance()->SetMessageLevel(pexor::MSG_DEBUG);

	pexor::Benchmark bench;
	bench.TimerInit();


	printf("  Filling testbuffer of length 0x%x ints\n",readbuffer.Length());
	srand(time(0));
	for (int i=0; i < readbuffer.Length(); ++i)
		{
			 //fill testbuffer with some numbers
			*readbuffer.Cursor(i)=0;
			*writebuffer.Cursor(i)=0xFFFFFFFF/RAND_MAX * rand();
//			if(Debugmode == 2)
//				{
//					if((i%10)==0) printf("\n");
//					printf("%d...",writebuffer[i]);
//				}
		}


	pexor::PexorTwo board;
	if(!board.IsOpen())
		{
			printf("**** Could not open pexor board!\n");
			return 1;
		}
	int iret=board.Reset();
	if(iret)
	{
	  printf("\n\nError %d in Board reset",iret);
	  return 1;
	}

	iret=board.InitBus(Channel,Maxslaves);
	if(iret)
		{
			printf("\n\nError %d in InitBus\n",iret);
			return 1;
		}
	sleep(1);


	if(Mode==0)
			{
				/* Loop io over all slaves*/
				printf("**** loop over all slaves and automate test \n");
				for(int sl=0; sl< Maxslaves; ++sl)
				{
					int werrors=0;




				// first write contents of writebuffer to the slave memory:
					bench.ClockStart();
				    bench.TimerStart();

				    // write bus here:
				    for(int i=0; i<EXPLODERLEN ;++i)
						{
				    	    int rev= board.WriteBus( EXPLODERBUF0 + i*4 , writebuffer[i] , Channel, sl);
							if(rev)
								{
									printf("\n\nError %d in WriteBus\n",rev);
									werrors++;
									continue;
									//break;
								}
						}
					double cycledelta=bench.TimerDelta();
					double clockdelta=bench.ClockDelta();
					bench.ShowRate("Clock:  SFP write ", writebuffer.Size(), clockdelta); // bytes
					bench.ShowRate("Cycles: SFP write", writebuffer.Size() , cycledelta);
					printf("\nSlave %d has %d write errors\n",sl,werrors);
				}


				// read back and compare it:

				printf("  Reading back from PEXOR Slaves...\n");

				long unsigned int val=0;
				for(int sl=0; sl< Maxslaves; ++sl)
					{
						int rerrors=0;
						printf("**** Slave %d ...\n",sl);
						for (int i=0; i < readbuffer.Length(); ++i)
								{
									*readbuffer.Cursor(i)=0; // clear read buffer for each slave
								}

						bench.ClockStart();
						bench.TimerStart();
						 for(int i=0; i<EXPLODERLEN;++i)
								{
									int rev=board.ReadBus(EXPLODERBUF0+i*4,val,Channel,sl);
									*(readbuffer.Cursor(i))=val;
									if(rev)
										{
											printf("\n\nError %d in ReadBus\n",rev);
											rerrors++;
											continue;
											//break;
										}

								}
						double cycledelta=bench.TimerDelta();
						double clockdelta=bench.ClockDelta();
						bench.ShowRate("Clock:  SFP read", readbuffer.Size(), clockdelta);
						bench.ShowRate("Cycles: SFP read", readbuffer.Size(), cycledelta);
						printf("\nSlave %d has %d read errors\n",sl,rerrors);
						if(readbuffer!=writebuffer) // will use equal operator== checking for errors
								readbuffer.ShowCompareErrors();
						else
								printf("No errors!\n");
					} // for


				return 1;
			}




	if(Mode==1)
		{
			 /* Test the "fieldbus" io*/
		    printf("**** Writing data 0x%x to sfp 0x%x, slave 0x%x, address 0x%x ...\n",Data,Channel,Slave,Address);

			int rev=board.WriteBus(Address,Data,Channel,Slave);
			if(rev)
				{
					printf("\n\nError %d in WriteBus\n",rev);
					return 1;
				}
			printf("\t\t Done.\n",rev);
			return 0;
		}	

	if(Mode==2)
		{
			 /* Test the "fieldbus" io*/
		    printf("**** Reading from  sfp 0x%x, slave 0x%x, address 0x%x ...\n", Channel,Slave,Address);
		    unsigned long value=0;
		    int rev=board.ReadBus(Address,value,Channel,Slave);
			if(rev)
				{
					printf("\n\nError %d in ReadBus\n",rev);
					return 1;
				}
		    printf("\nRead value = 0x%x .\n",value);
		    return 0;
		}
	if(Mode==3)
			{
				int rev=board.Add_DMA_Buffers(bytes,Bufnum);
				if(rev)
					{
						printf("\n\nError %d on mapping dma buffers\n",rev);
						return rev;
					}

				 /* write test data to the token buffers:*/

				// evaluate the submemory structures:
				unsigned long base_dbuf0=0, base_dbuf1=0;
    			unsigned long num_submem=0, submem_offset=0;
    			unsigned long datadepth=EXPLODERLEN*sizeof(int); // bytes per submemory

    			for(int sl=0; sl< Maxslaves; ++sl)
					{
    				  int werrors=0;
					  rev=board.ReadBus(REG_BUF0, base_dbuf0, Channel,sl);
					  if(rev==0)
						  printf("Slave %x: Base address for Double Buffer 0  0x%x  \n", sl,base_dbuf0 );
					  else
						  printf("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 0 address)\n", rev, sl, REG_BUF0);

					  rev=board.ReadBus(REG_BUF1,base_dbuf1,Channel,sl);
					  if(rev==0)
						  printf("Slave %x: Base address for Double Buffer 1  0x%x  \n", sl,base_dbuf1 );
					  else
						  printf("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 1 address)\n", rev, sl, REG_BUF1);

					  rev=board.ReadBus(REG_SUBMEM_NUM,num_submem,Channel,sl);
					  if(rev==0)
						  printf("Slave %x: Number of SubMemories  0x%x  \n", sl,num_submem );
					  else
						  printf("\n\ntoken Error %d in ReadBus: slave %x addr %x (num submem)\n", rev, sl, REG_SUBMEM_NUM);

					  rev=board.ReadBus(REG_SUBMEM_OFF,submem_offset,Channel,sl);
					  if(rev==0)
						  printf("Slave %x: Offset of SubMemories to the Base address  0x%x  \n", sl,submem_offset );
					  else
						  printf("\n\ncheck_token Error %d in ReadBus: slave %x addr %x (submem offset)\n", rev, sl, REG_SUBMEM_OFF);


						rev=board.WriteBus(REG_DATA_LEN,datadepth,Channel,sl);
						if(rev)
								{
									printf("\n\nError %d in WriteBus setting datadepth\n",rev);
									return 1;
								}
				// write predefined random numbers to the submemories. both double buffers have same contents.

					bench.ClockStart();
				    bench.TimerStart();
					for(int submem=0;submem<num_submem;++submem)
					 {
						 unsigned long submembase0=base_dbuf0+ submem*submem_offset;
						 unsigned long submembase1=base_dbuf1+ submem*submem_offset;
						 for(int i=0; i<EXPLODERLEN ;++i)
							{
								int rev= board.WriteBus( submembase0 + i*4 , writebuffer[i] , Channel, sl);
								if(rev)
									{
										printf("\n\nError %d in WriteBus for submem %d of buffer 0, wordcount %d\n",rev,submem,i);
										werrors++;
										continue;
										//break;
									}

								rev= board.WriteBus( submembase1 + i*4 , writebuffer[i] , Channel, sl);
								if(rev)
									{
										printf("\n\nError %d in WriteBus for submem %d of buffer 1, wordcount %d\n",rev,submem,i);
										werrors++;
										continue;
										//break;
									}
							} // datadepth

					 } // submem

					double cycledelta=bench.TimerDelta();
					double clockdelta=bench.ClockDelta();
					int totalsize=datadepth*num_submem*2;
					bench.ShowRate("Clock:  SFP write submems", totalsize, clockdelta); // bytes
					bench.ShowRate("Cycles: SFP write submems", totalsize , cycledelta);
					printf("\nSlave %d has %d write errors\n",sl,werrors);

				} // for slaves






				 /* Test the token io*/
#ifdef WITHTRIGGER
                /* case of trixor interrupt mode: set up readout loop*/
    			board.ResetTrigger();
    			board.SetTriggerTimes(0x10,0x20); // fcti, cvti
    			board.StartAcquisition();
    			for(int i=0; i< MAXTRIGGERS; ++i)
    			  {

    			 printf("**** Waiting for TRIGGER %d...\n", i);

                              if(!board.WaitForTrigger())
                                {
                                  printf("\n\nError Waiting for trigger!\n");
                                  return 1;
                                }

#endif
			    printf("**** Requesting token from  sfp 0x%x, bufid 0x%x ...\n", Channel, BufID);
			    int value=0;
			    bench.ClockStart();
			    bench.TimerStart();
			    pexor::DMA_Buffer* tokbuf= board.RequestToken(Channel, BufID, true); // synchronous dma mode here
#ifdef WITHTRIGGER
			    board.ResetTrigger();
#endif
			    if(tokbuf==0)
					{
						printf("\n\nError in Token Request\n");
						return 1;
					}
				double cycledelta=bench.TimerDelta();
				double clockdelta=bench.ClockDelta();
				printf("\nGot token buffer of length %d ints.\n",tokbuf->Length());
//				bench.ShowRate("Clock:  DMA buffer read:", tokbuf->Size(), clockdelta); // bytes
//				bench.ShowRate("Cycles: DMA buffer read:", tokbuf->Size() , cycledelta);
				int tokensize=Maxslaves*datadepth*num_submem; //
				bench.ShowRate("Clock:  token read:", tokensize, clockdelta); // bytes
				bench.ShowRate("Cycles: token read:", tokensize , cycledelta);



				// here check token memory contents:
				bool isheaderread=false,isdsizeread=false;
				int datasize=0,j=0;
				int hlength=0, dlength=0,trigid=0, modid=0,memid=0;
				int submemcount=0,slavecount=0;
				for(int cursor=0; cursor<tokbuf->Length();++cursor)
					{

							int currentdata=*(tokbuf->Cursor(cursor));
							if(!isheaderread)
								{
								printf("Token header full: 0x%x \n",currentdata);
								    // get next submemory header:
									hlength=((currentdata )  & 0xf0) >> 4; // just for check should be 3(byte)
									dlength=((currentdata )  & 0x0f); // should be 4 (byte)
									trigid = ((currentdata >> 8 ) & 0xff);
				 					modid =  ((currentdata >> 16 ) & 0xff);
									memid =  ((currentdata >> 24 ) & 0xff);
									printf("Token header: hlen:0x%x dlen:0x%x trigid:0x%x modid:0x%x memid:0x%x \n",hlength,dlength,trigid,modid,memid);
									if(hlength!=3 ||  dlength!=4)
										{
											printf("Invalid header data, assume we reached end of token, stop it!\n");
											break;
										}
									isheaderread=true;


								}
							else if(!isdsizeread)
								{
									// read data size of next submem chunk
									datasize=currentdata;
									printf("Reading datasize:0x%x bytes... \n",datasize);
									if(datasize==0)
									{
										printf("Zero datasize, assume we reached end of token, stop it!\n");
										break;
									}
									isdsizeread=true;
								}
							else
								{
									// compare submem contents with original send buffer
									if(j< datasize/sizeof(int) )
										{
											if(j>=readbuffer.Length())
											{
												printf("Error: readbuffer overflow at index 0x%x, len:%x ... \n",j,readbuffer.Length());
												j=datasize+1;
												cursor--;
												continue;
											}
											*readbuffer.Cursor(j++)=currentdata; // fill pexor buffer to use compare functionality
											//printf("j=%x,data=:0x%x\t",j,currentdata);
										}
									else
										{
										if(readbuffer!=writebuffer) // will use equal operator== checking for errors
												readbuffer.ShowCompareErrors();
										else
											printf("Mod %x Submem %x - No errors!\n",modid,memid);


										// reset bufs and counters:
										for (int t=0; t < readbuffer.Length(); ++t)
											 {
												*readbuffer.Cursor(t)=0;
											 }
										j=0;
										datasize=0;
										if(submemcount++ >= num_submem -1 )
											{
											    printf("Read %x Submems from  slave %x, try next slave...\n", num_submem,slavecount++);
												//isheaderread=false;
												submemcount=0;
											}
										isdsizeread=false;
										isheaderread=false;
										cursor--; // rewind to header of next block

										}
									}





					}

                                  board.Free_DMA_Buffer(tokbuf);
#ifdef WITHTRIGGER

    			  } // for loop
    			board.StopAcquisition();
#endif


				return 0;
			}

	  if(Mode==4)
				{
				  int val=0;
				  int rev=board.Read(&val,1,Address);
				  printf("  Reading %x from address %x\n",val,Address);

				  return 0;
				}


return 0;
}
