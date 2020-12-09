#include "pipe_sync.h"


#ifdef IOXOSSYNC

//#define IOXOS_COUNTER_DEBUG 1

#ifdef IOXOS_COUNTER_DEBUG
#define printd( args... ) printf(args);
#else
#define printd( args... ) ;
#endif


int f_wait_read(s_pipe_sync* com)
{
  readcounter++;
  //long timeout=0;
  printd("f_wait_read with readcounter %ld\n", readcounter);

  while(com->canread !=readcounter)
  {
//    usleep(PIPESYNC_SLEEP);
//    if(++timeout > PIPESYNC_MAXLOOP) return -1;
  }
  return 0;
}


int f_wait_write(s_pipe_sync* com)
{
  writecounter++;
  //long timeout=0;
  printd("f_wait_write with writecounter %ld \n", writecounter);
  while(com->canwrite != writecounter)
  {
//    usleep(PIPESYNC_SLEEP);
//    if(++timeout > PIPESYNC_MAXLOOP) return -1;
  }
  return 0;
}

void f_set_read(s_pipe_sync* com, int on)
{
  printd("f_set_read increments canread to %ld\n", readcounter+=on);
  com->canread+=on;

}

void f_set_write(s_pipe_sync* com, int on)
{
  printd("f_set_write increments canwrite to %ld\n", writecounter+=on);
  com->canwrite+=on;
}


#else

int f_wait_read(s_pipe_sync* com)
{
  long timeout=0;
  while(com->canread == 0)
  {
    usleep(PIPESYNC_SLEEP);
    if(++timeout > PIPESYNC_MAXLOOP) return -1;
    SERIALIZE_COM;
  }
  return 0;
}


int f_wait_write(s_pipe_sync* com)
{
  long timeout=0;
  while(com->canwrite == 0)
  {
    usleep(PIPESYNC_SLEEP);
    if(++timeout > PIPESYNC_MAXLOOP) return -1;
    SERIALIZE_COM;
  }
  return 0;
}

void f_set_read(s_pipe_sync* com, int on)
{
  com->canread=on;
  SERIALIZE_COM;
}

void f_set_write(s_pipe_sync* com, int on)
{
  com->canwrite=on;
  SERIALIZE_COM;
}

#endif
