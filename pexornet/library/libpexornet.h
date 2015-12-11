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

#include <net/if.h>
#include <netinet/in.h>

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
  if((handle==0) || (handle->fSockhandle <=0))                                  \
    {                                                                   \
      printm("Error: illegal pexornet handle %d, socket:%d \n", \
                handle, (handle ? handle->fSockhandle : 0));                            \
      return -1;                                                   \
    }

/** This is general ioctle handle structure.
 * It is created at pexornet_open() time and destroyed at pexornet_close()
 * Any process/thread using this library has individual handle*/
typedef struct
{
  int fSockhandle;        /**< socket file handle*/
  struct ifreq fIfreq;    /**< socket file handle*/
} pexornet_handle_t;

/** open file handle of pex device number devnum. Return value is handle*/
pexornet_handle_t* pexornet_open (int devnum);

/** close file handle*/
int pexornet_close (pexornet_handle_t* handle);

/** reset dma and sfp engines */
int pexornet_reset (pexornet_handle_t* handle);

/** read data word *l_dat from sfp, slave and memory offset l_slave_off*/
int pexornet_slave_rd (pexornet_handle_t*, long l_sfp, long l_slave, long l_slave_off, long *l_dat);

/** write data word l_dat to sfp, slave and memory offset l_slave_off*/
int pexornet_slave_wr (pexornet_handle_t* handle, long l_sfp, long l_slave, long l_slave_off, long l_dat);

/** initialize chain at l_sfp with l_s_slaves number of slaves*/
int pexornet_slave_init (pexornet_handle_t* handle, long l_sfp, long l_n_slaves);

/** write block of configuration data to driver*/
int pexornet_slave_config (pexornet_handle_t* handle, struct pex_bus_config* config);

/** retrieve actual slave configuration at sfp chains and put to external structure*/
int pexornet_get_configured_slaves(pexornet_handle_t* handle , struct pex_sfp_links* setup);


#endif
