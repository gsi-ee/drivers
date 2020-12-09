#include "f_map_pipe.h"
#include "timing.h"
#include "f_map_pipe.h"
#include "pipe_sync.h"

#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>

#define MBSPIPE_BASE 0x40000000
#define MBSPIPE_LEN  0x30000000

#define MBSPIPE_TEST_MAXLOOP 6247
#define MBSPIPE_TEST_DATALEN 0x10000000

// may enable  benchmark by cpu cycles with this (not working on ifc)
//#define BENCHMARK_USE_CYCLES 1

int Mode = -1;    // test mode
int NumRepeats = 1;    // number of external write loops

unsigned long PipeBase = MBSPIPE_BASE;
unsigned long PipeLen = MBSPIPE_LEN;

unsigned long Loopsize = MBSPIPE_TEST_MAXLOOP;    // optional loopsize parameter
unsigned long Datalength = MBSPIPE_TEST_DATALEN;    // total transferred data length in bytes
int Verbosity = 0;

unsigned long Errcount = 0;

void usage (const char *progname)
{
  printf ("***************************************************************************\n");

  printf (" %s for mbspipe test  \n", progname);
  printf (" v0.4 20-Nov-2020 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf ("  usage: %s [-h][-m <testmode>] [-n <repeat>] [-b <pipebase>] [-p <pipelen>] [-d <debugprint>] \n",
      progname);
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -n        : set number of read tests to repeat (%d)\n", NumRepeats);
  printf ("\t\t -l        : set expected max loop value for sawtooth data (%d)\n", MBSPIPE_TEST_MAXLOOP);
  printf ("\t\t -b        : set physical base address of pipe (0x%x)\n", MBSPIPE_BASE);
  printf ("\t\t -p        : set length of pipe in bytes (0x%x)\n", MBSPIPE_LEN);
  printf ("\t\t -m        : define test mode:\n");
  printf ("\t\t\t  0 - read incrementing counter words\n");
  printf ("\t\t\t  1 - same as 0, but producer is accessing VME  memory\n");
  printf ("\t\t\t  2 - same as 0, but switch back to producer after each data event\n");
  printf ("\t\t\t  3 - same as 2, but producer is accessing VME memory\n");
  printf ("\t\t -d        : set debug verbosity \n");
  exit (0);
}

void assert_wait_for_read(s_pipe_sync* com)
{
  if(Verbosity>3) printf("dddd assert_wait_for_read..\n");
  if (f_wait_read (com) < 0)    // wait until we have data
      {
        printf ("ERROR: timeout of %d s exceeded on waiting for read start. \n",
        PIPESYNC_TIMEOUT);
        exit (EXIT_FAILURE);
      }

}

void switch2write(s_pipe_sync* com)
    {
#ifndef IOXOSSYNC
    f_set_read (com, 0);
#endif
    f_set_write (com, 1);
    if(Verbosity>3) printf("dddd switch2write done.\n");
    }




int f_read_test_data (int* pipe_base)
{
  // Data format:
  // buffer header - total bytes to follow
  // event header -  next event bytes
  // leading data - loop length in event
  // event data   - incrementing numbers up to loop length
  // ...
  // event data   - incrementing numbers up to loop length
  // event header -  next event bytes
  // ...
  // last event data
  // buffer  trailer - total bytes since buffer header

  int firstread=1;
  unsigned long l_n_loop = 0, l_i = 0;
  unsigned long l_n_loopcheck=0;
  int* pl_dat;
  unsigned long l_dat_len = 0, l_ev_len, l_ev_lencheck = 0;
  char* pc_end=0;
  // skip status structure at begin of pipe:
  char* add = (char*) pipe_base;
  s_pipe_sync* com= (s_pipe_sync*) add; // take comm structure to indicate number of round
  add += sizeof(s_pipe_sync);
  pl_dat= (int*) add;

  switch (Mode)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      {
        int altread = (Mode==2 || Mode==3) ? 1 : 0 ; // alternating read/write flagnt altread = (Mode==2 || Mode==3) ? 1 : 0 ; // alternating read/write flag
        // get buffer header:
        l_dat_len = *pl_dat;
        if (Verbosity > 0)
          printf ("** Buffer Len: %d (%E) bytes\n", l_dat_len, (double) l_dat_len);
        // check pointer arithmetics. seems ok
        //pc_end=add+l_dat_len;
        //int* pl_end= (int*) pc_end;
        int* pl_end = pl_dat + (l_dat_len / sizeof(int));

        pl_dat++;
        while (pl_dat < pl_end)
        {
          // here check loop and eventsize to be expected from consumer
          l_n_loopcheck++;
          if (l_n_loopcheck == Loopsize) // note: Loopsize is fixed here
          {
            l_n_loopcheck = 1;
          }
          l_ev_lencheck= (l_n_loopcheck + 1) * sizeof(int);
          // end check section
          if(altread && firstread==0) assert_wait_for_read(com);
          firstread=0; // ioxos mode must not wait for read twice
          l_ev_len = *pl_dat++;
          if (Verbosity > 2)
            printf ("*** Event Len:%d bytes, expected:%d\t", l_ev_len, l_ev_lencheck);
          if(Verbosity>0 && l_ev_len!=l_ev_lencheck) // happens at end of buffer regularily!
            printf ("WARNING:  Event Len:%d does not match expected %d\n", l_ev_len, l_ev_lencheck);
          l_n_loop = *pl_dat++;    // leading data word
          if (Verbosity > 2)
            printf ("Loop:%d , expected:%d\n", l_n_loop, l_n_loopcheck);
          if(l_n_loop!=l_n_loopcheck)
            {
              Errcount++; // accouont as read error
              if (Verbosity > 0) printf ("ERROR:  Loop Len:%d does not match expected %d\n", l_n_loop, l_n_loopcheck);
              if (Verbosity > 1) exit(EXIT_FAILURE);
            }
          for (l_i = 0; l_i < l_n_loop; l_i++)
          {
            if (pl_dat >= pl_end)
              break;
            SERIALIZE_IO;
            int val = *pl_dat++;
            if (Verbosity > 2) printf ("Val(%d)=0x%x bytes\t", l_i,val);
            if (val != l_i + com->counter)    // data payload, simple incrementing with offset
            //if (val != l_i ) // test if it is working anyhow? yes.
            {
              Errcount++;
              if (Verbosity > 0)
              {
                printf ("ERROR f_read_test_data: read value %d does not match pattern value %d\n", val,  l_i+ com->counter);
                if (Verbosity > 1) exit(EXIT_FAILURE);
              }
              }
            if((Verbosity > 2) && (l_i % 10 ==0)) printf("\n");
          }    // for
          if(altread && pl_dat < pl_end) switch2write(com); // avoid double set counter in ioxos mode
        }    // while
        //pl_dat++;
        if(l_dat_len != *pl_dat)
        {
          // check trailer word, should contain buffer length
          printf ("WARNING f_read_test_data: buffer trailer length %d does not match header length %d, loop is %d\n",
              *pl_dat, l_dat_len,l_n_loop);
        }


        return l_dat_len;
      }
      break;
    default:
      {
        printf ("f_read_test_data ERROR: mode %d is not implemented yet! exit.", Mode);
        exit (EXIT_FAILURE);
        break;
      }

  }
  return 0;
}

int main (int argc, char *argv[])
{
  //int l_status;
  int i;
  int opt;
  double cycledelta = 0;
  double clockdelta = 0;
  double totalsize = 0;
  long transfersum = 0;

  readcounter=0;
  writecounter=0;
  /* get arguments*/
  //optind = 1;
  while ((opt = getopt (argc, argv, "hl:m:n:l:p:b:d:")) != -1)
  {
    switch (opt)
    {
      case '?':
        usage (basename (argv[0]));
        exit (EXIT_FAILURE);
      case 'h':
        usage (basename (argv[0]));
        exit (EXIT_SUCCESS);
      case 'b':
        PipeBase = strtol (optarg, NULL, 0);
        break;
      case 'p':
        PipeLen = strtol (optarg, NULL, 0);
        break;
      case 'm':
        Mode = strtol (optarg, NULL, 0);
        break;
      case 'n':
        NumRepeats = strtol (optarg, NULL, 0);
        break;
      case 'l':
        Loopsize = strtol (optarg, NULL, 0);
        break;
      case 'd':
        Verbosity = strtol (optarg, NULL, 0);
        break;
      default:
        break;
    }
  }

  if (Mode < 0)
  {
    usage (basename (argv[0]));
    exit (EXIT_SUCCESS);
  }

  if (Datalength > PipeLen)
    Datalength = PipeLen;

// map pipe
  int* pipebase = f_map_pipe (PipeBase, PipeLen);
  if (pipebase == 0)
  {
    printf ("ERROR in %s: could not map pipe at base 0x%x with length 0x%x \n",
    basename (argv[0]), PipeBase, PipeLen);
    exit (EXIT_FAILURE);
  }
  printf ("%s: wait before reading from pipe in mode %d, loopsize=%d\n",
  basename (argv[0]), Mode, Loopsize);
  s_pipe_sync* com = (s_pipe_sync*) pipebase;
#ifdef BENCHMARK_USE_CYCLES
  Pexortest_TimerInit ();
#endif
  for (i = 0; i < NumRepeats; ++i)
  {
    switch2write(com);
    assert_wait_for_read(com);
    printf ("after wait read, let's go for round %d\n",i);
    Pexortest_ClockStart ();
#ifdef BENCHMARK_USE_CYCLES
    Pexortest_TimerStart ();
#endif
    int len = f_read_test_data (pipebase);
#ifdef BENCHMARK_USE_CYCLES
    cycledelta = Pexortest_TimerDelta ();
#endif
    clockdelta = Pexortest_ClockDelta ();
    totalsize = len;
    transfersum += len;
    Pexortest_ShowRate ("Clock:  pipe read ", totalsize, clockdelta);    // bytes
#ifdef BENCHMARK_USE_CYCLES
    Pexortest_ShowRate ("Cycles: pipe read ", totalsize, cycledelta);
#endif
    //f_set_read (com, 0);
    printf ("After repeat %d we see %ld (%E) errors for %ld (%E) bytes read from pipe \n",i, Errcount, (double) Errcount, transfersum, (double) transfersum);
  }    // for
  printf ("%s: finished reading %ld bytes from pipe \n",
  basename (argv[0]), transfersum);

  return 0;
}
