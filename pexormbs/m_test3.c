// N.Kurz, EE, GSI, 30-Mar-2010

#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

main (int argc, char *argv[])
{
  long p    = 0x70000000;
  long v    = 0x20000000;  
  long p_v;

  unsigned  

long p2;

  p_v = p - v;
  p2  = p_v + v;
  printf ("p: 0x%x, v: 0x%x, p_v: 0x%x, p2: 0x%x \n", p, v, p_v, p2);
}
