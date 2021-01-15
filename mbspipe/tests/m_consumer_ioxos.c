#include <sys/timex.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <sys/time.h>
#include "f_map_pipe.h"

#include "tests_ioxos.h"
/* JAM2021: try different length for this test */

#define MBSPIPE_BASE 0x40000000
#define MBSPIPE_LEN  0x30000000

unsigned long PipeBase = MBSPIPE_BASE;
unsigned long PipeLen = MBSPIPE_LEN;
struct timeval ti, to;
struct timezone tz;

int 
check_data_buf( int *buf, int size, int data)
{
  int i;
  int errors=0;
  for( i = 0; i < size/sizeof(int); i++)
  {
    if( buf[i] != data+i)
    {
      //printf("data error: %08x != %08x\n", buf[i], data+i);
      errors++;
      //break;
    }
  }
  return errors;
}

int main (int argc, char *argv[])
{
  volatile int *cnt_A, *cnt_B;
  int i, j, cnt, iex;
  int dt;
  long errorcount=0;
  long totalwords=0;

  printf("entering consumer_ioxos, chunksize=0x%x ...\n", IOXOS_CHUNK_SIZE_INTS);
  int* pipebase = f_map_pipe (PipeBase, PipeLen);
  if (pipebase == 0)
  {
    printf ("ERROR in %s: could not map pipe at base 0x%x with length 0x%x \n",
    basename (argv[0]), PipeBase, PipeLen);
    exit (EXIT_FAILURE);
  }
  printf("pipebase = %p : %x\n", pipebase, *pipebase);
  for (j=0;j<IOXOS_PIPE_REPEATS; ++j)
   {
    cnt_A = pipebase;
    cnt_B = pipebase + 1;
    cnt = 0;
    iex = 1;
    i = 0;
    gettimeofday (&ti, &tz);
    while (iex)
    {
      cnt++;
      while (iex)
      {
        int tmp;
        tmp = *cnt_A;
        if (tmp == cnt)
          break;
        if (tmp == -1)
          iex = 0;
      }
      if (!iex)
      {
        *cnt_B = -1;
      }
      else
      {
        //check_data_buf( pipebase+(0x400*(i+1)), 0x1000, i<<16);
        errorcount += check_data_buf (pipebase + (IOXOS_CHUNK_SIZE_INTS * (i + 1)), IOXOS_CHUNK_SIZE_INTS * sizeof(int),
            i+j);
        *cnt_B = cnt;//++cnt;
        totalwords += IOXOS_CHUNK_SIZE_INTS;
      }
      i++;
    }
    gettimeofday (&to, &tz);
    dt = ((to.tv_sec - ti.tv_sec) * 1000000) + (to.tv_usec - ti.tv_usec);
    printf ("after pipe repeat %d: consumer_ioxos [cnt = %d]...time [usec] = %d rate = = %d MB/s, ERRORS:%ld\n", j, cnt, dt,
        (IOXOS_CHUNK_SIZE_INTS*sizeof(int)* cnt) / dt, errorcount);
    usleep(10000); // do not expect next cycle before producer is ready
  }    //j
  printf(" consumer ends: total words:%E errors:%E, error rate:%E\n",(double) totalwords, (double) errorcount, (double)(errorcount)/(double)(totalwords));
  return(0);
}
