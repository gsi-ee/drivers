#include "f_map_vme.h"

#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <vmeioctl.h>

// the latter is the offical name in ifc vmedirect distribution

//#define VME_AM        0x9
//#define VME_DEV_NAME  "/dev/ioxos/vme0"

#define DEBUG

struct usr_mmap
{
  char *base;
  int   size;
};

//  this is  name  in mbs installation. we modify a bit for the tests
//INTS4 f_map_ifc_a32_dimap_vme (INTU4 **pl_vme_virt, CHARX *s_vme_map_dev,
//                               INTU4 l_vme_addr, INTU4 l_vme_size, INTU4 l_vme_am)

int*  f_map_vme (const char *s_vme_map_dev,
                               unsigned int l_vme_addr,
                               unsigned int l_vme_size,
                               unsigned int l_vme_am)
{
  int    fd_vme;
  int    l_i;
  char  *p_virt_vme_base=0;
  struct vme_ioctl_mas_map *ps_vme_trans;
  struct usr_mmap          *ps_vme_user;

  #ifdef DEBUG
  printf ("vme address 0x%x, size: %x \n", l_vme_addr, l_vme_size);
  #endif

  // open VME device
  fd_vme = open(s_vme_map_dev, O_RDWR);
  if( fd_vme < 0)
  {
    printf ("ERROR>> f_map_vme cannot open VME mapping device %s \n", s_vme_map_dev);
    return 0;
  }
  // set VME window parameters
  ps_vme_trans = (struct vme_ioctl_mas_map *)malloc( sizeof( struct vme_ioctl_mas_map));
  if (ps_vme_trans)
  {
    ps_vme_trans->rem_addr = l_vme_addr;
    ps_vme_trans->loc_addr = -1;
    ps_vme_trans->size     = l_vme_size;
    ps_vme_trans->am       = l_vme_am;
    #ifdef DEBUG
    printf ("map request: %llx %llx %x %x\n",
           ps_vme_trans->rem_addr, ps_vme_trans->loc_addr, ps_vme_trans->size, ps_vme_trans->am);
    #endif
    if( ioctl( fd_vme, VME_IOCTL_MAS_MAP_SET, ps_vme_trans) < 0)
    {
      printf ("ERROR>> IOCTL failed, cannot allocate new VME translation. Try to fetch existing one...\n");
      if( ioctl( fd_vme, VME_IOCTL_MAS_MAP_GET, ps_vme_trans) < 0)
        {
        free (ps_vme_trans);
        printf ("ERROR>> f_map_vme IOCTL failed, cannot get any VME translation.\n");
        return 0;
        //exit (-1);
        }
      else
      {
        #ifdef DEBUG
          printf ("Got existing VME window parameters: vme_addr: %llx loc_addr: %llx size: %x am: %x\n",
            ps_vme_trans->rem_addr, ps_vme_trans->loc_addr, ps_vme_trans->size, ps_vme_trans->am);
         #endif
      }
    }
    else
    {
      #ifdef DEBUG
      printf ("VME window parameters: vme_addr: %llx loc_addr: %llx size: %x am: %x\n",
             ps_vme_trans->rem_addr, ps_vme_trans->loc_addr, ps_vme_trans->size, ps_vme_trans->am);
      #endif
    }
  }

  // map VME window into user space
  int vme_base, vme_off;
  int offset;

  ps_vme_user = (struct usr_mmap *)malloc( sizeof( struct usr_mmap));

  vme_base = ps_vme_trans->rem_addr;
  vme_off = l_vme_addr - vme_base;
  if((( vme_off + l_vme_size) > ps_vme_trans->size) || (vme_off < 0))
  {
    printf ("ERROR>> f_map_vme - VME_IOCTL_MAS_MAP_SET with illegal size parameters: base=0x%x, off=0x%x, transsize=0x%x, size=0x%x\n",
        vme_base, vme_off, ps_vme_trans->size, l_vme_size);
    return 0;
    //exit (-1);
  }

  offset = vme_off & 0xfff;
  vme_off &= 0xfffff000;
  ps_vme_user->size = (l_vme_addr + l_vme_size + 0xfff) & 0xfffff000;
  ps_vme_user->size -= (vme_base + vme_off);
  p_virt_vme_base = NULL;
  ps_vme_user->base =  mmap( NULL, ps_vme_user->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_vme, (off_t)vme_off);
  if (ps_vme_user->base == (char*) (-1))
    {
      printf ("ERROR>> failed to mmap   VME, errno is:%d (%s) \a\a", errno, strerror (errno));
      return 0;
    }



  #ifdef DEBUG
  printf ("vme_map_window: %p - %x [%x]\n", ps_vme_user->base, vme_off,  ps_vme_user->size);
  #endif
  if( ps_vme_user->base)
  {
    p_virt_vme_base = ps_vme_user->base + offset;

  }
  return (int*) p_virt_vme_base;
}
