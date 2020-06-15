/*
 * polandtest.cpp
 *
 *  Created on: 22.06.2014
 *      Author: J.Adamczewski-Musch
 *
 *  Test sfp with connected slaves (poland/qfwboards) at pexor2/3
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

#define REG_RST 0xFFFFF4
#define REG_CTRL       0x200000

//#define NUMARGS 6

#define TESTBUFSIZE 0x10000
#define DMABUFNUM 30

#define NUMSLAVES 1
#define EXPLODERBUF0 0x000000
#define EXPLODERBUF1 0x100000
#define EXPLODERLEN  6100 

//#define WITHTRIGGER 1//
#define MAXTRIGGERS 100
#define TESTMODE 0

#define QFWLOOPS 3
#define QFWCHANS 32
#define QFWNUM 8


// number of polls for triggerless readout with wait for data ready:
#define MAXPOLLS 1000000

// loop wait time per cycle in microseconds:
#define WAITCYCLE 1

static int Debugmode = TESTMODE;
static int Hexmode = 0;

static int Bufsize = TESTBUFSIZE;
static int Bufnum = DMABUFNUM;
static int Maxslaves = NUMSLAVES;

static int Trignum = MAXTRIGGERS;
static bool withtrigger = false;
static bool DoConfig=false;
static int Channel = 0;

static unsigned int ErrorScaler[QFWNUM] = { 0 };

//static int Slave=0;
//static int Address=0;
//static int Data=0;
static int BufID = 0;

/* helper macro for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/
#define  QFWRAW_CHECK_PDATA                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
  printf("############ unexpected end of payload for sfp:%d slave:%d with opticlen:0x%x, skip event\n",sfp_id, device_id, opticlen);\
  return -1; \
}

/* this one just to leave internal loops*/
#define  QFWRAW_CHECK_PDATA_BREAK                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
 break; \
}

void usage ()
{
  printf ("\n**** polandtest v 0.2 24-Jun-2014 by JAM (j.adamczewski@gsi.de)\n");
  printf ("* read out data via token DMA from preconfigured poland frontends and unpack/display\n");
  printf ("Usage:\n");
  printf ("\t polandtest [-i] [-s sfp] [-p slaves ] [-b bufsize] [-n numbufs] [-t numtrigs] [-d level]\n");
  printf ("Options:\n");
  printf ("\t\t -h          : display this help\n");
  printf ("\t\t -i          : initialize qfw before test\n");
  printf ("\t\t -s sfp      : select sfp chain   (default 0)\n");
  printf ("\t\t -p slaves   : number of poland slaves at chain   (default 1)\n");
  printf ("\t\t -b bufsize  : size of dma buffer (default %d integers)\n", TESTBUFSIZE);
  printf ("\t\t -n numbufs  : number of dma buffers in pool (default %d)\n", DMABUFNUM);
  printf ("\t\t -t numtrigs : use triggered readout with number of triggers to read (default polling) \n");
  //printf ("\t\t -x          : hex output mode \n");
  printf ("\t\t -d level    : verbose output (debug) mode with level 1: print traces, 2: traces+ libpexor debug\n");
  printf ("**********************************\n");
  exit (0);
}


int init_qfw(pexor::PexorTwo* theBoard)
{

  BufID=0;

  int rev=theBoard->InitBus(Channel,Maxslaves);
  if(rev)
          {
              printf("\n\nError %d in InitBus\n",rev);
              return 1;
          }


  // evaluate the submemory structures:
    unsigned long base_dbuf0 = 0, base_dbuf1 = 0;
    unsigned long num_submem = 0, submem_offset = 0;
    unsigned long qfw_control=0;
    //            unsigned long datadepth=EXPLODERLEN*sizeof(int); // bytes per submemory

    for (int sl = 0; sl < Maxslaves; ++sl)
    {
      int werrors = 0;
      rev = theBoard->ReadBus (REG_BUF0, base_dbuf0, Channel, sl);
      if (rev == 0)
        printf ("Slave %x: Base address for Double Buffer 0  0x%x  \n", sl, base_dbuf0);
      else
        printf ("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 0 address)\n", rev, sl, REG_BUF0);

      rev = theBoard->ReadBus (REG_BUF1, base_dbuf1, Channel, sl);
      if (rev == 0)
        printf ("Slave %x: Base address for Double Buffer 1  0x%x  \n", sl, base_dbuf1);
      else
        printf ("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 1 address)\n", rev, sl, REG_BUF1);

      rev = theBoard->ReadBus (REG_SUBMEM_NUM, num_submem, Channel, sl);
      if (rev == 0)
        printf ("Slave %x: Number of SubMemories  0x%x  \n", sl, num_submem);
      else
        printf ("\n\ntoken Error %d in ReadBus: slave %x addr %x (num submem)\n", rev, sl, REG_SUBMEM_NUM);

      rev = theBoard->ReadBus (REG_SUBMEM_OFF, submem_offset, Channel, sl);
      if (rev == 0)
        printf ("Slave %x: Offset of SubMemories to the Base address  0x%x  \n", sl, submem_offset);
      else
        printf ("\n\ncheck_token Error %d in ReadBus: slave %x addr %x (submem offset)\n", rev, sl, REG_SUBMEM_OFF);

// now configuration as in mbs:
      // disable test data length

      theBoard->WriteBus (REG_DATA_LEN, 0x10000000, Channel, sl);
      theBoard->WriteBus (REG_CTRL, 0, Channel, sl);
      theBoard->ReadBus (REG_CTRL, qfw_control, Channel, sl);
      if ( (qfw_control & 0x1) != 0)
          {
          printf ("ERROR disabling trigger acceptance in qfw failed, qfw control=0x%x!\n",qfw_control);
          return 1;
          }
      theBoard->WriteBus (REG_CTRL, 1, Channel, sl);
      theBoard->ReadBus (REG_CTRL, qfw_control, Channel, sl);
      if ( (qfw_control & 0x1) != 1)
      {
        printf ("ERROR enabling trigger acceptance in qfw failed, qfw control=0x%x!\n",qfw_control);
        return 1;
      }

      // write SFP id for channel header
      rev = theBoard->WriteBus (REG_HEADER, Channel, Channel, sl);
      if (rev)
        {
                         printf ("ERROR>> PEXOR slave write REG_HEADER  failed\n");
                         return 1;
        }
      rev = theBoard->WriteBus (REG_RST, 1 , Channel, sl);
      if (rev)
      {
        printf ("ERROR>> PEXOR slave write REG_RST  failed\n");
        return 1;
      }


      // now put setup for stairs here:

//
//#Disable Trigger (ON=1 OFF=0)
      theBoard->WriteBus (0x20004c, 0, Channel, sl);

//0 0 20004c 0
//
//#Start DAC program
      theBoard->WriteBus (0x200030, 1, Channel, sl);
      theBoard->WriteBus (0x200030, 0, Channel, sl);

//
//#set number of TS loop1
//#max x"fd" slices in total
      theBoard->WriteBus (0x200014, 2, Channel, sl);
//#set number of TS loop2
      theBoard->WriteBus (0x200018, 3, Channel, sl);
//
//#set number of TS loop3
      theBoard->WriteBus (0x20001c, 4, Channel, sl);
//
//#::::::::::::::::::::::::::::::::::::
//#all numbers = hex / unit = 20ns
//#::::::::::::::::::::::::::::::::::::
//#set time of TS loop1
//#1F4 #10us
//#c350 #1ms
//#7a120 #10ms
//#1e8480 #40ms
//#4c4b40 #100ms
//#2faf080 #1s
      theBoard->WriteBus (0x200020, 0xc350, Channel, sl);
//
//#set time of TS loop2
      theBoard->WriteBus (0x200024, 0xc350, Channel, sl);
//
//#set time of TS loop3
      theBoard->WriteBus (0x200028, 0xc350, Channel, sl);

      //#reset counter on FPGA
      theBoard->WriteBus (0x20003c, 1, Channel, sl);
      theBoard->WriteBus (0x20003c, 0, Channel, sl);

      //#only one Module should be Master:
      if(Channel==0)
      {
        theBoard->WriteBus (0x200040, 1, Channel, sl); //#internal trigger is on (1) or off (0)
        theBoard->WriteBus (0x200048, 2, Channel, sl); //#FESA = Bit 1 (ON=1 OFF=0) MASTER or Slave = BIT 2 (ON=1 OFF=0)
      }
      else
      {
        theBoard->WriteBus (0x200040, 0, Channel, sl); //#internal trigger is on (1) or off (0)
        theBoard->WriteBus (0x200048, 0, Channel, sl); //#FESA = Bit 1 (ON=1 OFF=0) MASTER or Slave = BIT 2 (ON=1 OFF=0)
          }

//#Offset Measurment (1s mess - time)
      theBoard->WriteBus (0x200044, 0, Channel, sl);
//
//################
//# s2_ini_qfw:
//#reset qfw
      theBoard->WriteBus (0x200010, 0, Channel, sl);
      theBoard->WriteBus (0x200010, 1, Channel, sl);

//#QFW mode 2
 theBoard->WriteBus (0x200004, 2, Channel, sl);
//#program qfw
 theBoard->WriteBus (0x200008, 1, Channel, sl);
 theBoard->WriteBus (0x200008, 0, Channel, sl);

//#Enable Trigger (ON=1 OFF=0)
 theBoard->WriteBus (0x20004c, 1, Channel, sl);

    }    // for slaves



    if (withtrigger)
      {
        /* case of trixor interrupt mode: set up readout loop*/
        theBoard->ResetTrigger ();
        theBoard->SetTriggerTimes (10, 300);    // fcti, cvti
        theBoard->StartAcquisition ();
      }


    return 0;
}


int unpack_qfw (pexor::DMA_Buffer* tokbuf)
{

///////////////// this code is mostly taken from Go4 unpacker at https://subversion.gsi.de/go4/app/qfw/pexor

  int loopsize[QFWLOOPS];
  int looptime[QFWLOOPS];

  int *pdata = tokbuf->Data ();
  int *pdatabegin = pdata;
  int lwords = tokbuf->UsedSize ()/sizeof(int); // this is true filled size from DMA, not total buffer lenght
  // loop over single subevent data:
  while (pdata - pdatabegin < lwords)
  {

    if ((*pdata & 0xff) != 0x34)    // regular channel data
    {
      printf ("**** unpack_qfw: Skipping Non-header format 0x%x - (0x34 are expected) ...\n",
          (*pdata & 0xff));
      pdata++;
      continue; // we have to skip it, since the dedicated padding pattern is added by mbs and not available here!
    }

    unsigned trig_type = (*pdata & 0xf00) >> 8;
    unsigned sfp_id = (*pdata & 0xf000) >> 12;
    unsigned device_id = (*pdata & 0xff0000) >> 16;
    unsigned channel_id = (*pdata & 0xff000000) >> 24;
    int* pdatastart = pdata;
    pdata++;

    int opticlen = *pdata++;
    printf ("Token header: trigid:0x%x sfp:0x%x modid:0x%x memid:0x%x opticlen:0x%x\n", trig_type, sfp_id, device_id,
        channel_id, opticlen);
    //
    if (opticlen > lwords * 4)
    {
      printf ("**** unpack_qfw: Mismatch with subevent len %d and optic len %d", lwords * 4, opticlen);
      // avoid that we run second step on invalid raw event!
      return -1;
    }
    QFWRAW_CHECK_PDATA;
    int eventcounter = *pdata;
    printf (" - Internal Event number 0x%x\n", eventcounter);
    // board id calculated from SFP and device id:

    pdata += 1;
    QFWRAW_CHECK_PDATA;
    int QfwSetup = *pdata;
    printf (" - QFW SEtup %d\n", QfwSetup);
    for (int j = 0; j < 4; ++j)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      pdata++;

    }
    QFWRAW_CHECK_PDATA;
    for (int l = 0; l < QFWLOOPS; l++)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      loopsize[l] = *pdata++;
      printf (" - Loopsize[%d] = 0x%x\n", l, loopsize[l]);
    }    // first loop loop

    QFWRAW_CHECK_PDATA;
    for (int loop = 0; loop < QFWLOOPS; loop++)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      looptime[loop] = *pdata++;
      printf (" - Looptime[%d] = 0x%x\n", loop, looptime[loop]);
    }    // second loop loop

    for (int j = 0; j < 21; ++j)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      pdata++;

    }
    QFWRAW_CHECK_PDATA;
    /** All loops X slices/loop X channels */
    for (int loop = 0; loop < QFWLOOPS; loop++)
    {
      for (int sl = 0; sl < loopsize[loop]; ++sl)
        for (int ch = 0; ch < QFWCHANS; ++ch)
        {
          QFWRAW_CHECK_PDATA_BREAK;
          int value = *pdata++;
          //loopData->fQfwTrace[ch].push_back(value);
          // TODO: pseudo trace graphics on terminal
          if(Debugmode)
            printf (" -- loop %d slice %d ch %d = 0x%x\n", loop, sl, ch, value);
        }
    }    //loop

    QFWRAW_CHECK_PDATA;
    /* errorcount values: - per QFW CHIPS*/
    for (int qfw = 0; qfw < QFWNUM; ++qfw)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      ErrorScaler[qfw] = (unsigned int) (*pdata++);
      printf (" - ErrorScaler[%d] = 0x%x\n", qfw, ErrorScaler[qfw]);
    }
    QFWRAW_CHECK_PDATA;

    // skip filler words at the end of gosip payload:
    while (pdata - pdatastart <= (opticlen / 4))    // note that trailer is outside opticlen!
    {
      if(Debugmode) printf("######### skipping word 0x%x\n ",*pdata);
      pdata++;
    }

    // crosscheck if trailer word matches eventcounter header
    if (*pdata != eventcounter)
    {
      printf ("!!!!! Eventcounter 0x%x does not match trailing word 0x%x at position 0x%x!\n", eventcounter, *pdata,
          (opticlen / 4));
    }
    else
    {
        printf ("Found trailing Eventcounter 0x%x \n",*pdata);
    }
    pdata++;
  }    // while pdata - pdatastart < lwords

////////////////////////////// end go4 unpacker

  return 0;
}

int main (int argc, char **argv)
{

  /* get arguments*/
  int opt, optind = 1;
  while ((opt = getopt (argc, argv, "hs:p:b:n:t:xd:i")) != -1)
  {
    switch (opt)
    {
      case '?':
        usage ();
        exit (EXIT_SUCCESS);
      case 'h':
        usage ();
        exit (EXIT_SUCCESS);
      case 's':
        Channel = strtol (optarg, NULL, 0);
        break;
      case 'p':
        Maxslaves = strtol (optarg, NULL, 0);
        break;
      case 'b':
        Bufsize = strtol (optarg, NULL, 0);
        break;
      case 'n':
        Bufnum = strtol (optarg, NULL, 0);
        break;
      case 't':
        withtrigger = true;
        Trignum = strtol (optarg, NULL, 0);
        break;
      case 'i':
        DoConfig = true;
        break;

//      case 'x':
//        Hexmode = strtol (optarg, NULL, 0);
//        break;
      case 'd':
        Debugmode = strtol (optarg, NULL, 0);
        break;
      default:
        break;
    }
  }

  printf ("\n**** polandtest ****\n");

  int bytes = Bufsize * sizeof(int);
  bytes = pexor::Buffer::NextPageSize (bytes);
  Bufsize = bytes / sizeof(int);

  printf (" - set Mode:%d\n", Debugmode);
  printf (" - set DMA Bufsize:%d integers (%d bytes)\n", Bufsize, bytes);
  printf (" - set DMA Poolsize:%d buffers\n", Bufnum);
  printf (" - set sfp Channel:0x%x\n", Channel);
   printf(" - set number of slave devices:0x%x \n",Maxslaves);
  printf (" - set token mode double buffer:%d \n", BufID);
  printf (" - set do configure:%d \n",DoConfig);
  printf (" - set Triggermode:%d with 0x%x triggers \n",withtrigger,Trignum);

//    printf(" - set address:0x%x\n",Address);
//    printf(" - set data value:0x%x\n",Data);

  if (Debugmode == 2)
    pexor::Logger::Instance ()->SetMessageLevel (pexor::MSG_DEBUG);

  pexor::Benchmark bench;
  bench.TimerInit ();

  pexor::PexorTwo board;
  if (!board.IsOpen ())
  {
    printf ("**** Could not open pexor board!\n");
    return 1;
  }

  if(DoConfig)
  {
    if (board.Reset ()!=0)
     {
       printf ("**** Could not reset pexor board!\n");
       return 1;
     }


    if(init_qfw(&board)!=0)
      {
        printf ("**** Could not configure qfw!\n");
        return 1;
      }
    printf("* Wait a while after config...\n\n");
    sleep(1);
  }

  int rev = board.Add_DMA_Buffers (bytes, Bufnum);
  if (rev)
  {
    printf ("\n\nError %d on mapping dma buffers\n", rev);
    return rev;
  }



  /* Test the token io*/
  if (!withtrigger)
  {
    Trignum = 10;
  }

  for (int i = 0; i < Trignum; ++i)
  {
    pexor::DMA_Buffer* tokbuf=0;
    int pollcounter=0;
    if (withtrigger)
    {
      printf ("**** Waiting for TRIGGER %d...\n", i);

      if (!board.WaitForTrigger ())
      {
        printf ("\n\nError Waiting for trigger!\n");
        return 1;
      }
    }

    printf ("**** Requesting token from  sfp 0x%x, bufid 0x%x ...\n", Channel, BufID);
    int value = 0;
    bench.ClockStart ();
    bench.TimerStart ();

    if (withtrigger)
       {
        tokbuf = board.RequestToken (Channel, BufID , true);    // synchronous dma mode here,
        board.ResetTrigger ();
       }
    else
    {

      board.RequestToken (Channel, BufID | 2 , false, false); // asynchronous mode here, wait for data ready |2

      BufID = (BufID == 0 ? 1 : 0);

      do
      {
      tokbuf= board.WaitForToken (Channel, false, 0, 0, false);
      pollcounter++;
      usleep(WAITCYCLE);
      } while (tokbuf == (pexor::DMA_Buffer*)(-1) && pollcounter<MAXPOLLS);

      printf ("Polled for %d cycles of %d us, tokbuf=0x%x\n", pollcounter, WAITCYCLE, tokbuf);


    }

    if (tokbuf <=  0 || pollcounter>=MAXPOLLS)
    {
      printf ("\n\nError in Token Request\n");
      return 1;
    }





    double cycledelta = bench.TimerDelta ();
    double clockdelta = bench.ClockDelta ();
    printf ("\nGot token buffer of length %d ints.\n", tokbuf->Length ());
    bench.ShowRate ("Clock:  DMA buffer read:", tokbuf->UsedSize (), clockdelta);    // bytes
    bench.ShowRate ("Cycles: DMA buffer read:", tokbuf->UsedSize (), cycledelta);

//				int tokensize=Maxslaves*datadepth*num_submem; //
//				bench.ShowRate("Clock:  token read:", tokensize, clockdelta); // bytes
//				bench.ShowRate("Cycles: token read:", tokensize , cycledelta);





    if (unpack_qfw (tokbuf) != 0)
    {
      printf ("Error in unpacker at event %d\n",i);
      continue;
    }



  board.Free_DMA_Buffer (tokbuf);

}    // for loop


if(withtrigger && DoConfig)
  board.StopAcquisition();

return 0;

}
