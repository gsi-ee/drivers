/*
 * data producer test executable to check mbpsipe validity
 *  \ \author J.Adamczewski-Musch (j.adamczewski@gsi.de)
 *  \date 22-Oct_2020
 * */


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


 int Mode = -1; // test mode
 int NumRepeats=1; // number of external write loops

 unsigned long PipeBase = MBSPIPE_BASE;
 unsigned long PipeLen =  MBSPIPE_LEN;

 unsigned long Loopsize= MBSPIPE_TEST_MAXLOOP; // optional loopsize parameter
 unsigned long Datalength= MBSPIPE_TEST_DATALEN; // total transferred data length in bytes
 int Verbosity=0;


void usage (const char *progname)
{
  printf ("***************************************************************************\n");

  printf (" %s for mbspipe test  \n", progname);
  printf (" v0.1 30-Oct-2020 by JAM (j.adamczewski@gsi.de)\n");
  printf ("***************************************************************************\n");
  printf (
      "  usage: %s [-h][-m <testmode>] [-n <repeat>][-l <maxloop>] [-s <sendbytes>] [-b <pipebase>] [-p <pipelen>] [-d <debugprint>] \n",
      progname);
  printf ("\t Options:\n");
  printf ("\t\t -h        : display this help\n");
  printf ("\t\t -n        : set number of write tests to repeat (%d)\n",NumRepeats);
  printf ("\t\t -l        : set max loop value for sawtooth writing (%d)\n",MBSPIPE_TEST_MAXLOOP);
  printf ("\t\t -s        : set number of bytes to send (%d)\n",MBSPIPE_TEST_DATALEN);
  printf ("\t\t -b        : set physical base address of pipe (0x%x)\n", MBSPIPE_BASE);
  printf ("\t\t -p        : set length of pipe in bytes (0x%x)\n", MBSPIPE_LEN);
  printf ("\t\t -m        : define test mode:\n");
  printf ("\t\t\t  0 - write incrementing counter words up to max loop value\n");
  printf ("\t\t\t  1 - mbs formatted with header (?)\n");
  printf ("\t\t -d        : set debug verbosity \n");
  exit(0);
}


int f_write_test_data(int* pipe_base)
{
  // Data format:
  // buffer header - total bytes to follow
  // event header -  next event bytes
  // leading data - loop lenght in event
  // event data   - incrementing numbers up to loop length
  // ...
  // event data   - incrementing numbers up to loop length
  // event header -  next event bytes
  // ...
  // last event data
  // buffer  trailer - total bytes since buffer header


  unsigned long l_n_loop=0, l_i=0;
  int* pl_dat;
  int l_dat_len=0, l_ev_len=0;
  // skip status structure at begin of pipe:
  char* add = (char*) pipe_base;
  s_pipe_sync* com= (s_pipe_sync*) add; // take comm structure to indicate number of round
  add += sizeof(s_pipe_sync);
  pl_dat = (int*) add;
  switch(Mode)
  {
    case 0:
      {
        // todo: sync producer and consumer by first word in pipe

        int* pl_end = pl_dat + Datalength / sizeof(int);
        int* pl_buf_header = pl_dat++;
        while (pl_dat < pl_end)
        {
          l_ev_len=0;
          int* pl_header = pl_dat++;
          l_n_loop++;
          if (l_n_loop == Loopsize)
          {
            l_n_loop = 1;
          }
          *pl_dat++ = l_n_loop; // leading data word
          l_ev_len += sizeof(int);
          for (l_i = 0; l_i < l_n_loop; l_i++)
          {
            if (pl_dat >= pl_end)
              break;
            *pl_dat++ = l_i + com->counter; // data payload, simple incrementing with offset of round number
            l_ev_len += sizeof(int);
          }    // for
          *pl_header = l_ev_len; // event header
          l_dat_len+=l_ev_len;

        }    // while
        *pl_buf_header=l_dat_len; // buffer header
        *pl_dat=l_dat_len; // buffer trailer
        return l_dat_len;
      }
      break;
    default:
    {
      printf ("f_write_test_data ERROR: mode %d is not implemented yet! exit.",Mode);
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
  double cycledelta=0;
  double clockdelta=0;
  double totalsize=0;
  long transfersum=0;


  /* get arguments*/
  //optind = 1;
  while ((opt = getopt (argc, argv, "hl:m:n:s:p:b:d:")) != -1)
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
      case 'l':
        Loopsize = strtol (optarg, NULL, 0);
        break;
      case 's':
        Datalength = strtol (optarg, NULL, 0);
        break;
      case 'm':
        Mode = strtol (optarg, NULL, 0);
        break;
      case 'n':
        NumRepeats= strtol (optarg, NULL, 0);
        break;
      case 'd':
        Verbosity = strtol (optarg, NULL, 0);
        break;
      default:
        break;
    }
  }

  if (Mode<0)
  {
    usage (basename (argv[0]));
    exit (EXIT_SUCCESS);
  }

  if(Datalength>PipeLen) Datalength=PipeLen;

// map pipe
  int* pipebase=f_map_pipe(PipeBase, PipeLen);
  if(pipebase == 0)
  {
    printf ("ERROR in %s: could not map pipe at base 0x%x with length 0x%x \n",
        basename (argv[0]), PipeBase, PipeLen);
    exit (EXIT_FAILURE);
  }

  printf ("%s: start writing %d bytes to pipe in mode %d, loopsize=%d\n",
           basename (argv[0]), Datalength, Mode, Loopsize);
  Pexortest_TimerInit();
  s_pipe_sync* com = (s_pipe_sync*) pipebase;
  f_set_read(com, 0); // init
  f_set_write(com, 0); // do not write until the consumer is ready


  for(i=0; i< NumRepeats; ++i)
  {
    // here sync mechansim with consumer - simple approach- read and write not simultaneous:
  if(f_wait_write(com)<0)
    {
      printf ("ERROR in %s: timeout of %d s exceeded on waiting for write start. \n",
              basename (argv[0]), PIPESYNC_TIMEOUT);
          exit (EXIT_FAILURE);
    }
  printf ("after wait write, let's go for round %d\n",i);
  f_set_read(com, 0);
  com->counter=i;
  Pexortest_ClockStart();
  Pexortest_TimerStart();

  int len=f_write_test_data(pipebase);
  cycledelta=Pexortest_TimerDelta();
  clockdelta=Pexortest_ClockDelta();
  totalsize=len;
  transfersum+=len;
  Pexortest_ShowRate("Clock:  pipe write ", totalsize, clockdelta); // bytes
  Pexortest_ShowRate("Cycles: pipe write ", totalsize , cycledelta);
  f_set_write(com,0);
  f_set_read(com,1);
  } // for
  printf ("%s: finished writing %ld bytes to pipe \n",
         basename (argv[0]), transfersum);

 return 0;
}
