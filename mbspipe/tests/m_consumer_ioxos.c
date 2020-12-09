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

  for( i = 0; i < size/sizeof(int); i++)
  {
    if( buf[i] != data+i)
    {
      printf("data error: %08x != %08x\n", buf[i], data+i);
      break; 
    }
  }
}

int main (int argc, char *argv[])
{
  volatile int *cnt_A, *cnt_B;
  int i, cnt, iex;
  int dt;

  printf("entering consumer_ioxos...\n");
  int* pipebase = f_map_pipe (PipeBase, PipeLen);
  if (pipebase == 0)
  {
    printf ("ERROR in %s: could not map pipe at base 0x%x with length 0x%x \n",
    basename (argv[0]), PipeBase, PipeLen);
    exit (EXIT_FAILURE);
  }
  printf("pipebase = %p : %x\n", pipebase, *pipebase);

  cnt_A = pipebase;
  cnt_B = pipebase+1;
  cnt = 0;
  iex = 1;
  i = 0;
  gettimeofday( &ti, &tz);
  while(iex)
  {
    cnt++;
    while(iex)
    {
      int tmp;
      tmp = *cnt_A;
      if( tmp == cnt) break;
      if( tmp == -1) iex = 0;
    }
    if( !iex)
    {
      *cnt_B = -1;
    }
    else
    {
      check_data_buf( pipebase+(0x400*(i+1)), 0x1000, i<<16);
      *cnt_B = ++cnt;
    }
    i++;
  }
  gettimeofday( &to, &tz);
  dt = ((to.tv_sec -  ti.tv_sec) * 1000000) + ( to.tv_usec - ti.tv_usec);
  printf("exiting producer_ioxos [cnt = %d]...time [usec] = %d rate = = %d MB/s\n", cnt, dt, (0x1000*cnt)/dt);
  return(0);
}
