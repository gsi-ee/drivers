#ifndef __F_MAP_PIPE_H__
#define __F_MAP_PIPE_H__


/**
 *
 * This function implements the actual mapping of the pipe memory
 * parameters: physical address of memory base, length of mapped region
 * return value: virtual address of mapped region base
 * \author J.Adamczewski-Musch (j.adamczewski@gsi.de)
 * \date 28-Oct_2020
 */

int* f_map_pipe(unsigned long physbase, unsigned long maplen);



#endif
