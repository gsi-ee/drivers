// N.Kurz, EE, 13-Oct-2005
#include <typedefs.h>

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>           /* for open()              */
#include <sys/file.h>        /* define flags for open() */
#include <sched.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/stat.h>

#include <sbs_def.h>

#define GSI__LINUX 1

#if defined (GSI__LYNX) && ! defined (V40)
  #include <smem.h>
#endif
#if defined (GSI__LINUX) || defined (V40)
 #include "smem.h"
#endif 
#ifdef GSI__LYNX
 #include <shmmap.h>
#endif

#ifdef GSI_MBS
 #include <error_mac.h>
 #include <errnum_def.h>
 #include <err_mask_def.h>
#endif // GSI_MBS

char *smem_get (char *pc_name, long l_size, int l_perm)
{
  #ifdef GSI_MBS
  CHARS   c_modnam[] = "smem_get";
  #endif // GSI_MBS
  CHARS   c_line[256];
  size_t  len;
  off_t   off;
  mode_t  mode;
  int     shm_fd;
  int     oflags;
  #ifdef GSI__LYNX
  char    path[128]="/";
  #endif
  #ifdef GSI__LINUX
  char    path[128]="/tmp/shm/";
  #endif
  char   *mem_ptr;
  unsigned volatile char *pa, *pe;

  strcat (path, pc_name);
  
  mode   = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
  oflags = O_CREAT | O_RDWR;

  #ifdef DEBUG
  #ifdef GSI_MBS
  sprintf (c_line,"segment name: %s, pid: %d", path, getpid ());
  F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT); 
  #else
  printf ("segment name: %s, pid: %d \n", path, getpid ());
  #endif // GSI_MBS
  #endif // DEBUG

	umask (0);

  #ifdef GSI__LYNX
  if ((shm_fd = shm_open (path, oflags, mode)) == -1)
  #endif
  #ifdef GSI__LINUX
  if ((shm_fd = open (path, oflags, mode)) == -1)
  #endif
  {
    #ifdef GSI_MBS
    sprintf (c_line,"segment: %s failed to open, "RBO"exiting.."RES" \a\a", path);
    F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
    //perror("shm_open");
    exit (0); 
    #else
    #ifdef GSI__LYNX
    perror("shm_open");
    #endif // GSI__LYNX
    #ifdef GSI__LINUX
    perror("open");
    #endif // GSI__LINUX
    exit (EXIT_FAILURE);
    #endif // GSI_MBS 
  }

  // Now allocate RAM to the shared memory
  if (ftruncate (shm_fd, l_size) == -1) 
  {
    #ifdef GSI_MBS
    sprintf (c_line,"segment: %s failed to truncate, "RBO"exiting.."RES" \a\a", path);
    F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
    exit (0); 
    #else
    perror("ftruncate");
    exit (EXIT_FAILURE);
    #endif // GSI_MBS 
  }

  oflags = 0;
  #ifdef GSI__LYNX
  if ( (l_perm & 1) == SM_READ)  { oflags = PROT_READ; }
  if ( (l_perm & 2) == SM_WRITE) { oflags = oflags | PROT_WRITE; }
  #endif // GSI__LYNX
  #ifdef GSI__LINUX
  if ( (l_perm & 1) == SM_READ)  { oflags = PROT_READ | MAP_SHARED; }
  if ( (l_perm & 2) == SM_WRITE) { oflags = oflags | PROT_WRITE | MAP_SHARED; }
  #endif // GSI__LINUX
  if (oflags == 0)
	{
    #ifdef GSI_MBS
    sprintf (c_line,"no permission set for segment: %s, "RBO"exiting.."RES" \a\a", path);
    F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
    exit (0); 
    #else
    printf ("no permission set for segment: %s, "RBO"exiting.."RES" \a\a \n", path);
    exit (EXIT_FAILURE);
    #endif // GSI_MBS 
  }

  len = l_size;
  if ((mem_ptr = (char *) mmap (NULL, len, oflags,
                               MAP_SHARED, shm_fd, (off_t)0)) == MAP_FAILED)
  {
    #ifdef GSI_MBS
    sprintf (c_line,"segment: %s failed to mmap, "RBO"exiting.."RES" \a\a", path);
    F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
    exit (0); 
    #else
    perror("mmap");
    exit (EXIT_FAILURE);
    #endif // GSI_MBS 
  }
  else
  {
    pa = mem_ptr;
    pe = pa + l_size - 1;
    #ifdef DEBUG
    printf ("SHM name:                     %s   \n", path); 
    printf ("size:                         0x%x \n", len);
    printf ("first mapped virtual address: 0x%x \n", pa);
    printf ("last  mapped virtual address: 0x%x \n", pe);
    #endif // DEBUG
  }
  return (mem_ptr);
}

char *smem_create (char *pc_name, char *pc_physaddr, long l_size, int l_perm)
{
  #ifdef GSI_MBS
  CHARS   c_modnam[] = "smem_create";
  #endif // GSI_MBS
  CHARS          c_line[256];
  int            fd;
  unsigned char *pa, *pe;
  size_t         len;
  int            prot;
  int            flags;
  off_t          off;

  if ( (l_perm & 16) == SM_DETACH)
  {
    //printm ("DETACH: shall not be called !! \n");
    return 0;
  } 

  #ifdef DEBUG
  printf ("name: %s, permission: %d ", pc_name, l_perm);
  #endif // DEBUG

  prot = 0;
  if ( (l_perm & 1) == SM_READ) { prot = PROT_READ; }
  if ( (l_perm & 2) == SM_WRITE){ prot = prot | PROT_WRITE; }
	if ( prot == 0 )
	{
    #ifdef GSI_MBS
    sprintf (c_line,"no permission set for: %s, "RBO"exiting.."RES" \a\a", pc_name);
    F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
    exit (0); 
    #else
    printf ("no permission set for: %s, "RBO"exiting.."RES" \a\a \n", pc_name);
    exit (EXIT_FAILURE);
    #endif // GSI_MBS 
  }

  if ((fd = open ("/dev/mem", O_RDWR)) < 0)
  {
    #ifdef GSI_MBS
    sprintf (c_line,"failed to open /dev/mem for %s, "RBO"exiting.."RES" \a\a", pc_name);
    F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
    exit (0); 
    #else
    perror("open");
    exit (EXIT_FAILURE);
    #endif // GSI_MBS 
  }

  len   = l_size;
  flags = MAP_SHARED;
  off   = (off_t) pc_physaddr;
  #ifdef GSI__LYNX
  #ifdef RIO4
  if (pc_physaddr >= 0x40000000) // start of static vme
  { 
    prot = prot | PROT_UNCACHE; // static vme needs PROT_UNCACHE
  }
  #endif // RIO4  
  #endif // endif GSI__LYNX

  if ((pa = mmap (NULL, len, prot, flags, fd, off)) < 0)
  {
    #ifdef GSI_MBS
    sprintf (c_line,"failed to mmap for %s, "RBO"exiting.."RES" \a\a", pc_name);
    F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
    exit (0); 
    #else
    perror("mmap");
    exit (EXIT_FAILURE);
    #endif // GSI_MBS 
  }
  else
  {
    #ifdef DEBUG
    pe = pa + len - 1;
    printf ("name:                           %s \n", pc_name);
    printf ("physical address:             0x%x \n", off);
    printf ("size:                         0x%x \n", len);
    printf ("first mapped virtual address: 0x%x \n", pa);
    printf ("last  mapped virtual address: 0x%x \n", pe);
    #endif // DEBUG
  }
  return (pa);
}


char smem_remove (char *pc_name)
{
  #ifdef GSI_MBS
  CHARS   c_modnam[] = "smem_remove";
  #endif // GSI_MBS
  CHARS   c_line[256];  
  CHARS   path[128] = "/";

  strcat (path, pc_name);  

  #ifdef DEBUG
  printf ("unlink shared segment %s \n", path);
  #endif // DEBUG 
  
  #ifdef GSI__LYNX
  if (shm_unlink (path) == -1)
  #endif
  #ifdef GSI__LINUX
  if (unlink (path) == -1)
  #endif
  {
    if (errno == ENOENT)
		{ 
      return (0);
    }
    else
		{
      #ifdef GSI_MBS
      sprintf (c_line,"failed to delete %s \a\a", path);
      F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
      return (0); 
      #else
      #ifdef GSI__LYNX
      perror("shm_unlink");
      #endif
      #ifdef GSI__LINUX
      perror("unlink");
      #endif
      return (-1);
      #endif // GSI_MBS 
    }
  }
  return (0);
}
