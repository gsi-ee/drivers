#ifndef _PEX_LIBPEXORNET_H_
#define _PEX_LIBPEXORNET_H_

/**
 * \file
 * C user library to work with pexornet.ko kernel module
 *
 * \author JAM (Joern Adamczewski-Musch, GSI Darmstadt, Germany -- j.adamczewski@gsi.de)
 * \date 26-August-2014 -- started with libmbspex
 * \date 4-November-2015 -- begin with libpexornet transition
 *
 */



#include "pexornet_user.h"

#ifdef PEXORNET_NOMBS
#define printm printf
#else
void printm (char *, ...); /* use mbs logger, or for gosipcmd this will be reimplemented  as printf for link time switching*/
#endif


//////////////

/** these internal registers are used from library instead of implementing dedicated ioctl calls.
 * They are not part of driver user includes!*/
#define PEXORNET_TK_DSIZE_SEL 0x210a0
#define PEXORNET_TK_MEM_SIZE 0x210b0
#define PEXORNET_REP_TK_DSIZE 0x21090


#define pexornet_assert_handle(handle)                                    \
  if(handle < 0)                                  \
    {                                                                   \
      printm("Error: illegal file handle %d \n", \
                handle);                            \
      return -1;                                                   \
    }

/** open file handle of pex device number devnum. Return value is handle*/
int pexornet_open (int devnum);

/** close file handle*/
int pexornet_close (int handle);

/** reset dma and sfp engines */
int pexornet_reset (int handle);

/** read data word *l_dat from sfp, slave and memory offset l_slave_off*/
int pexornet_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat);

/** write data word l_dat to sfp, slave and memory offset l_slave_off*/
int pexornet_slave_wr (int handle, long l_sfp, long l_slave, long l_slave_off, long l_dat);

/** initialize chain at l_sfp with l_s_slaves number of slaves*/
int pexornet_slave_init (int handle, long l_sfp, long l_n_slaves);

/** write block of configuration data to driver*/
int pexornet_slave_config (int handle, struct pex_bus_config* config);

/** retrieve actual slave configuration at sfp chains and put to external structure*/
int pexornet_get_configured_slaves(int handle , struct pex_sfp_links* setup);


#endif
