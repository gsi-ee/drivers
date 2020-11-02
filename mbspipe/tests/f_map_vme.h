#ifndef __F_MAP_VME_H__
#define __F_MAP_VME_H__


/**
 * map  vme memory to check if this affects the pipe mmu
 * arguments:
 *  s_vme_map_dev - name of driver file handle, e.g. /dev/ioxos/vme0
 *  l_vme_addr - base vme bus address
 *  l_vme_size - size of window to map
 *  l_vme_am - vme address modifier (mostly 0x9)
 *  returns virtual address in calling user space, or  0 if mapping failed.
 * */


int*  f_map_vme (const char *s_vme_map_dev,
                               unsigned int l_vme_addr, unsigned int l_vme_size, unsigned int l_vme_am);

#endif
