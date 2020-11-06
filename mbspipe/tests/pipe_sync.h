#ifndef __PIPE_SYNC_H__
#define __PIPE_SYNC_H__

#include <unistd.h>

// usecs for wait poll loop
//#define PIPESYNC_SLEEP 10000

#define PIPESYNC_SLEEP 10

// seconds timeout for wait
#define PIPESYNC_TIMEOUT 300

// number of loop cycles for timeout
#define PIPESYNC_MAXLOOP PIPESYNC_TIMEOUT * 1000000 / PIPESYNC_SLEEP

/**
 * communication structure to be put at begin of pipe memory.
 * JAM 30-oct-2020
 */
typedef struct
{
  int canread;  // if 1, producer has provided data to read, otherwise consumer waits
  int canwrite; // if 1, consumer has processed most recent block and producer can send another
  int counter;  // number of pipe cycles processed. can be used to modifiy data content

} s_pipe_sync;


/** wait until reading is allowed. Returns negative value in case of timeout*/
int f_wait_read(s_pipe_sync* com);

/** wait until writing is allowed. Returns negative value in case of timeout*/
int f_wait_write(s_pipe_sync* com);

/** set pipe read state to allow (1) or suspend (0)*/
void f_set_read(s_pipe_sync* com, int on);

/** set pipe write state to allow (1) or suspend (0)*/
void f_set_write(s_pipe_sync* com, int on);


#endif
