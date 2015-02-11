/*
 * Test of gosip io protocol with mbxpex library
 * J.Adamczewski-Musch, gsi, 12-May_2014
 *
 */

/*#include "../driver/pex_user.h"*/

#include "mbspex/libmbspex.h"

#include <string.h>
#include "timing.h"

#define NUMARGS 3

/* logical numer of pexor device*/
#define PEXDEVNO 0

#define NUMSLAVES 1
#define EXPLODERBUF0 0x000000
#define EXPLODERBUF1 0x100000
#define EXPLODERLEN  6100

// address map for slave (exploder1): this is user specific data concering frontends , so it is not available from libmbspex
#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC



#define PEX_MEM_OFF       0x100000
#define PEX_REG_OFF       0x20000
#define PEXOR_TRIXOR_BASE      0x40000
#define PEXOR_TRIX_CTRL 0x04
#define PEXOR_TRIX_STAT 0x00
#define PEXOR_TRIX_FCTI 0x08
#define PEXOR_TRIX_CVTI 0x0C

#define PCI_BAR0_SIZE     0x800000  // in bytes


#define MAX_SFP       4

//nr of slaves on SFP 0   1   2   3
//                     |   |   |   |
#define NR_SLAVES    { 1,  0,  0,  0}




#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>


static int Debugmode=0;
static int* wbuffer=0;
static int* rbuffer=0;
static int Submembytes=EXPLODERLEN*sizeof(int); // bytes per submemory

static   int    sfp_slaves[MAX_SFP] = NR_SLAVES;

void init_buffers(int bytesize)
{
    int i=0;
    printf("  Alloc test buffers for %d (0x%x)bytes\n",bytesize,bytesize);
    wbuffer = (int*) malloc(bytesize);
    rbuffer = (int*) malloc(bytesize);
    printf("  Filling testbuffers...\n");
    srand(time(0));
    for(i=0; i<bytesize/sizeof(int);++i)
    {
        rbuffer[i]=0;
        wbuffer[i]=0xFFFFFFFF/RAND_MAX * rand();
        if(Debugmode == 2 )
            {
                if((i%10)==0) printf("\n");
                printf("%d...",wbuffer[i]);
            }
    }
    printf("\n\n");
}



void close_exit(int filehandle)
{
    close(filehandle);
    free (wbuffer);
    free (rbuffer);
    exit(0);
}





/* helper functions to check validity*/

/* compare original buffer with different buffer of same length buflen.
 * debugmode may switch  output details */
int gosiptest_compare_buffers(int* original, int* different, int buflen, int debugmode)
{
    int i=0, ercnt=0;
    if(!debugmode) return 0;
    if(debugmode > 0)
        {
            printf("Comparing buffers...\n");
        }

    for(i=0; i<buflen;++i)
    {

        if(debugmode >0)
        {
            if(original[i]!=different[i])
                {
                    printf("\n### content mismatch for entry %d: %d != %d\n",i,original[i],different[i]);
                    ercnt++;
                }
            if(debugmode >2)
                {
                    printf("%d...",different[i]);
                    if((i%10)==0) printf("\n");
                }
        }
        different[i]=0;

    }
    if(debugmode>0) printf("\n\nFound %d errors (ratio %e) .\n",ercnt, (float) ercnt / (float) buflen);

return ercnt;
}


int gosiptest_check_unpack(int* dmabuf, int* testbuf, int intlen, int numsubmem)
{
  int isheaderread=0,isdsizeread=0;
  int datasize=0,t,j=0;
  int hlength=0, dlength=0,trigid=0, modid=0,memid=0;
  int submemcount=0,slavecount=0;
  int cursor,currentdata;
  int buflen=Submembytes/sizeof(int);

  printf("Check unpack of %d submems (%d integers each) contained in %d integer field \n",numsubmem, buflen, intlen);
  for(cursor=0; cursor<intlen;++cursor)
       {

               currentdata=dmabuf[cursor];
               if(!isheaderread)
                   {
                   printf("\nToken header full: 0x%x \n",currentdata);
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
                       isheaderread=1;


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
                       isdsizeread=1;
                   }
               else
                   {
                       // compare submem contents with original send buffer
                       if(j< datasize/sizeof(int) )
                           {
                               if(j>=intlen)
                               {
                                   printf("Error: readbuffer overflow at index 0x%x, len:%x ... \n",j,intlen);
                                   j=datasize+1;
                                   cursor--;
                                   continue;
                               }
                               rbuffer[j++]=currentdata;
                               if(Debugmode>1)
                                   printf("j=%x,data=:0x%x\t",j,currentdata);
                           }
                       else
                           {
                           if(gosiptest_compare_buffers(wbuffer, rbuffer, buflen, Debugmode)==0)
                               printf("Mod %x Submem %x - No errors!\n",modid,memid);


                           // reset bufs and counters:
                           for (t=0; t < buflen; ++t)
                                {
                                   rbuffer[t]=0;
                                }
                           j=0;
                           datasize=0;
                           if(submemcount++ >= numsubmem -1 )
                               {
                                   printf("Read %x Submems from  slave %x, try next slave...\n", numsubmem,slavecount++);
                                   //isheaderread=false;
                                   submemcount=0;
                               }
                           isdsizeread=0;
                           isheaderread=0;
                           cursor--; // rewind to header of next block
                           }
                       }
       } // for cursor
  // after final submem need to check this outside loop:
  if(gosiptest_compare_buffers(wbuffer, rbuffer, buflen, Debugmode)==0)
                                printf("Mod %x Submem %x - No errors!\n",modid,memid);
return 0;
}

void usage()
{
    printf("\n**** gosiptest for mbspex library ***** \n");
    printf(" v0.1 13-May-2014 by JAM (j.adamczewski@gsi.de)\n");
    printf("\t usage: gosiptest [sfp] [debug] \n");
    printf("\t\t sfp  - \n\t\t\t sfp channel for token request \n");
    printf("\t\t debug - \n\t\t\t 0:minimum \n\t\t\t 1:more, check buffers \n\t\t\t 2: also print buf contents \n");
    printf("******************************************\n");
    exit(0);
}





int main (int argc, char *argv[])
{
  int          l_status;
  int          fd_pex;
  int          sfp_channel=0;
  int sl;
  int bufid=0;

  unsigned long base_dbuf0=0, base_dbuf1=0;
  unsigned long num_submem=0, submem_offset=0;
  unsigned long dmasize=0;
  long check_comm=0, check_token=0, check_slaves=0;
  double cycledelta=0;
  double clockdelta=0;
  double totalsize;
  mode_t       mode;
  long         l_bar0; 
  long         l_trix_base;
  long         l_trix_val=0; 
  uid_t        uid;

  int            rev;



  // this one for pipe memory:
  size_t         len   = 0x6c784000;//the limit is 0x6d800000 on lx86l-10 (DELL p3600)<- reserved area starts at 0xad800000
  off_t          off   = (off_t)0x40000000;
  int            prot  = PROT_WRITE | PROT_READ;
  int            flags = MAP_SHARED | MAP_LOCKED;
  char           *pa, *pe;

  int* pdat;
  int i;

  // try to read some pexor registers here:
//  long  volatile *pl_virt_bar0;
//
//  long  volatile *pl_dma_source_base;
//  long  volatile *pl_dma_target_base;
//  long  volatile *pl_dma_trans_size;
//  long  volatile *pl_dma_burst_size;
//  long  volatile *pl_dma_stat;
//  long  volatile *pl_irq_control;
//  long  volatile *pl_irq_status;
//  long  volatile *pl_trix_fcti;
//  long  volatile *pl_trix_cvti;



  if(argc > NUMARGS)
          usage();
      if(argc>1)
      {
          if(strstr("-h",argv[1])) usage();
          sfp_channel=atoi(argv[1]);
      }
      if(argc>2)
      {
        Debugmode=atoi(argv[2]);
      }

 MbsPextest_TimerInit();

 if(Debugmode==42)
 {
    printf ("Special test of benchmarking!\n");
    MbsPextest_ClockStart();
    //MbsPextest_TimerStart();
    sleep(3);
    cycledelta=MbsPextest_TimerDelta();
    clockdelta=MbsPextest_ClockDelta();
    printf ("\t Clockdelta = %e (%f) s\n",clockdelta,clockdelta);
    MbsPextest_ShowRate("Clock:  deltaT test", 100.0, clockdelta); // bytes
    MbsPextest_ShowRate("Cycles: deltaT test", 100.0, cycledelta);
     exit(0);
 }


/* OPEN DEVICE handle*/
fd_pex=mbspex_open(PEXDEVNO);
if (fd_pex < 0) {
  printf ("ERROR>> open /dev/pexor%d \n", PEXDEVNO);
  exit(1);
}

/* map pipe memory*/

printf ("Test to map pipe at 0x%x, size: 0x%x, ...\n", off, len);
 if ((pa = (char *) mmap (NULL, len, prot, flags, fd_pex, off)) == MAP_FAILED)
  {
    printf ("failed to mmap pipe at pex, return: 0x%x, %d \n", pa, pa);
    perror ("mmap");
  }
  else
  {
    pe = pa + len - 1;
    printf ("physical address:             0x%x \n", off);
    printf ("size:                         0x%x \n", len);
    printf ("first mapped virtual address: 0x%x \n", pa);
    printf ("last  mapped virtual address: 0x%x \n", pe);
    pdat= (int*) pa;
  }
/* allocate and fill test buffer with random values*/

 init_buffers(Submembytes);

 rev=mbspex_reset(fd_pex);
 if(rev!=0)
 {
   printf("\n\nError %d in mbspex_reset . exiting...\n", rev);
   close_exit(fd_pex);
 }
/* copy random test values to exploder frontend memory*/

 rev=mbspex_slave_init (fd_pex, sfp_channel, NUMSLAVES);
 if(rev!=0)
 {
   printf("\n\nError %d in mbspex_slave_init for channel %x . exiting...\n", rev, sfp_channel);
   close_exit(fd_pex);
 }

 /* write test data to the token buffers:*/
 for(sl=0; sl< NUMSLAVES; ++sl)
          {
            int werrors=0;
            int submem=0;

            rev=mbspex_slave_rd (fd_pex, sfp_channel, sl, REG_BUF0, &base_dbuf0);
            if(rev==0)
                printf("Slave %x: Base address for Double Buffer 0  0x%x  \n", sl,base_dbuf0 );
            else
                printf("\n\nError %d in mbspex_slave_rd: slave %x addr %x (double buffer 0 address)\n", rev, sl, REG_BUF0);

            rev=mbspex_slave_rd (fd_pex, sfp_channel, sl, REG_BUF1, &base_dbuf1);
            if(rev==0)
                printf("Slave %x: Base address for Double Buffer 1  0x%x  \n", sl,base_dbuf1 );
            else
                printf("\n\nError %d in mbspex_slave_rd: slave %x addr %x (double buffer 1 address)\n", rev, sl, REG_BUF1);

            rev=mbspex_slave_rd (fd_pex, sfp_channel, sl, REG_SUBMEM_NUM, &num_submem);
            if(rev==0)
                printf("Slave %x: Number of SubMemories  0x%x  \n", sl,num_submem );
            else
                printf("\n\nError %d in mbspex_slave_rd: slave %x addr %x (num submem)\n", rev, sl, REG_SUBMEM_NUM);

            rev=mbspex_slave_rd (fd_pex, sfp_channel, sl, REG_SUBMEM_OFF, &submem_offset);
            if(rev==0)
                printf("Slave %x: Offset of SubMemories to the Base address  0x%x  \n", sl,submem_offset );
            else
                printf("\n\nError %d in mbspex_slave_rd: slave %x addr %x (submem offset)\n", rev, sl, REG_SUBMEM_OFF);

            rev=mbspex_slave_wr  (fd_pex, sfp_channel, sl, REG_DATA_LEN, Submembytes);
            if(rev)
                      {
                          printf("\n\nError %d in mbspex_slave_wr setting datadepth\n",rev);
                          close_exit(fd_pex);
                      }
            MbsPextest_ClockStart();
            MbsPextest_TimerStart();

                  for(submem=0;submem<num_submem;++submem)
                   {
                       unsigned long submembase0=base_dbuf0+ submem*submem_offset;
                       unsigned long submembase1=base_dbuf1+ submem*submem_offset;
                       for(i=0; i<EXPLODERLEN ;++i)
                          {
                             rev=mbspex_slave_wr  (fd_pex, sfp_channel, sl, submembase0 + i*4 , wbuffer[i]);
                              if(rev)
                                  {
                                      printf("\n\nError %d in mbspex_slave_wr for submem %d of buffer 0, wordcount %d\n",rev,submem,i);
                                      werrors++;
                                      continue;
                                      //break;
                                  }
                              rev=mbspex_slave_wr  (fd_pex, sfp_channel, sl, submembase1 + i*4 , wbuffer[i]);
                              if(rev)
                                  {
                                      printf("\n\nError %d in WriteBus for submem %d of buffer 1, wordcount %d\n",rev,submem,i);
                                      werrors++;
                                      continue;
                                      //break;
                                  }
                          }

                   } // submem
                  cycledelta=MbsPextest_TimerDelta();
                  clockdelta=MbsPextest_ClockDelta();
                  totalsize=Submembytes*num_submem*2;
                  MbsPextest_ShowRate("Clock:  SFP write submems", totalsize, clockdelta); // bytes
                  MbsPextest_ShowRate("Cycles: SFP write submems", totalsize , cycledelta);
                  printf("\nSlave %d has %d write errors\n",sl,werrors);
          } // slaves





/* do synchronous token request with direct dma*/


 printf("**** Requesting token from  sfp 0x%x, bufid 0x%x ...\n", sfp_channel, bufid);
 MbsPextest_ClockStart();
 MbsPextest_TimerStart();
 rev=mbspex_send_and_receive_tok (fd_pex, sfp_channel, bufid, off,&dmasize,
     &check_comm, &check_token, &check_slaves);/* set target to begin of pipe memory*/
 if(rev!=0)
  {
    printf("\n\nError %d in mbspex_send_and_receive_tok for channel %x . exiting...\n", sfp_channel);
    close_exit(fd_pex);
  }


 cycledelta=MbsPextest_TimerDelta();
 clockdelta=MbsPextest_ClockDelta();

   // TODO: test check words
    rev = 0;
    if ((check_token & 0x1) != bufid)
    {
      printf ("ERROR double buffer toggle bit differs from token return toggle bit \n");
      rev++;
    }
    /* note: mode can not be specified so far, always direct dma*/
    if (check_slaves != sfp_slaves[sfp_channel])
    {
      printf ("ERROR nr. of slaves specified: %d differ from token return: %d \n", sfp_slaves[sfp_channel],
          check_slaves);
      rev++;
    }
    if(dmasize==0)
       {
         printf ("ERROR zero transfersize! \n");
         rev++;
       }
    if(rev!=0) close_exit (fd_pex);


   // TODO: get real dma transfer size and use this for rate calculation and unpack boundary
    totalsize=NUMSLAVES*Submembytes*num_submem;
    printf ("Received %d DMA bytes, token memory payload:%d bytes\n",dmasize, totalsize);

    MbsPextest_ShowRate("Clock:  dma read:", (double) dmasize, clockdelta); // bytes
    MbsPextest_ShowRate("Cycles: dma read:", (double) dmasize , cycledelta);
    MbsPextest_ShowRate("Clock:  payload read:", totalsize, clockdelta); // bytes
    MbsPextest_ShowRate("Cycles: payload read:", totalsize , cycledelta);

    gosiptest_check_unpack(pdat, wbuffer, dmasize/sizeof(int), num_submem);

    close_exit(fd_pex);

return 0;
}


#ifndef MBSPEX_NOMBS
/*****************************************************************/
/* here separate definition of printm required by mbspex lib at link time:*/

#include <stdarg.h>

void printm (CHARX *fmt, ...)
{
  CHARX c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
  printf ("%s", c_str);
  va_end(args);
}
#endif

