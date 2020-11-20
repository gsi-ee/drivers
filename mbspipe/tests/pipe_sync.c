#include "pipe_sync.h"



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
