#include <sys/timex.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include "f_map_pipe.h"

#define MBSPIPE_BASE 0x40000000
#define MBSPIPE_LEN  0x30000000

unsigned long PipeBase = MBSPIPE_BASE;
unsigned long PipeLen = MBSPIPE_LEN;

int 
fill_data_buf( int *buf, int size, int data)
{
  int i;

  for( i = 0; i < size/sizeof(int); i++)
  {
    buf[i] = data+i;
  }
}

int 
main (int argc, char *argv[])
{
  volatile int *cnt_A, *cnt_B;
  int i, cnt;

  printf("entering producer_ioxos...\n");
  int* pipebase = f_map_pipe (PipeBase, PipeLen);
  if (pipebase == 0)
  {
    printf ("ERROR in %s: could not map pipe at base 0x%x with length 0x%x \n",
    basename (argv[0]), PipeBase, PipeLen);
    exit (EXIT_FAILURE);
  }
  printf("pipebase = %p : %x\n", pipebase, *pipebase);
  *pipebase = 0x12345678;

  cnt_A = pipebase;
  cnt_B = pipebase+1;
  cnt = 0;
  *cnt_A = cnt;
  *cnt_B = cnt;
  for( i = 0; i < 0x1000; i++)
  {
    fill_data_buf( pipebase+(0x400*(i+1)), 0x1000, i<<16);
    *cnt_A = ++cnt;
    cnt++;
    while(1)
    {
      int tmp;
      tmp = *cnt_B;
      if( tmp == cnt) break;
    }
  }
  *cnt_A = -1;
  printf("exiting producer_ioxos [cnt=%d]...\n", cnt);
  return(0);
}
