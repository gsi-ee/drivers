#ifndef _PEX_LIBMBSPEX_H_
#define _PEX_LIBMBSPEX_H_

/**
 * \file
 * C user library to work with mbspex.ko kernel module
 *
 * \author JAM (Joern Adamczewski-Musch, GSI Darmstadt, Germany -- j.adamczewski@gsi.de)
 * \date 26-August-2014 -- 10-Sep-2015 -- 13-06-2024:
 *
 */

/** JAM 18-09-2023:
 * turn on static structures for ioctl descriptors. May speed up things by avoiding memory allocation in each function
 * */
//#define MBSPEX_IOCTL_GLOBAL_DESCRIPTORS 1

/** JAM 12-06-2024:
 * added functions to change sfp link speed
 * */


#include "../include/pex_user.h"

#ifdef MBSPEX_NOMBS
#define printm printf
#else
void printm (char *, ...); /* use mbs logger, or for gosipcmd this will be reimplemented  as printf for link time switching*/
#endif


//////////////

/** these internal registers are used from library instead of implementing dedicated ioctl calls.
 * They are not part of driver user includes!*/
#define MBSPEX_TK_DSIZE_SEL 0x210a0
#define MBSPEX_TK_MEM_SIZE 0x210b0
#define MBSPEX_REP_TK_DSIZE 0x21090


#define mbspex_assert_handle(handle)                                    \
  if(handle < 0)                                  \
    {                                                                   \
      printm("Error: illegal file handle %d \n", \
                handle);                            \
      return -1;                                                   \
    }

/** open file handle of pex device number devnum. Return value is handle*/
int mbspex_open (int devnum);

/** close file handle*/
int mbspex_close (int handle);

/** reset dma and sfp engines */
int mbspex_reset (int handle);

/** read data word *l_dat from sfp, slave and memory offset l_slave_off*/
int mbspex_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat);

/** write data word l_dat to sfp, slave and memory offset l_slave_off*/
int mbspex_slave_wr (int handle, long l_sfp, long l_slave, long l_slave_off, long l_dat);

/** initialize chain at l_sfp with l_s_slaves number of slaves*/
int mbspex_slave_init (int handle, long l_sfp, long l_n_slaves);

/** set chain at l_sfp to specified linkspeed*/
int mbspex_set_linkspeed (int handle, long l_sfp, enum pex_linkspeed specs);




/** write block of configuration data to driver*/
int mbspex_slave_config (int handle, struct pex_bus_config* config);

/** send token request to pexor device of handle at chain sfp
 *  with l_toggle word (sets frontend buffer)
 * l_ldma_target specifies physical address of target buffer for token data DMA
 * returns some result check words:
 * pl_transfersize:  size of transferred dma in bytes
 * pl_check_comm: l_comm
 * pl_check_token: toggle and mode bits
 * pl_check_slaves: nr. of slaves connected to token chain
 */
int mbspex_send_and_receive_tok (int handle, long l_sfp, long l_toggle, unsigned long l_dma_target,
    unsigned long* pl_transfersize, long *pl_check_comm, long *pl_check_token, long *pl_check_slaves);


/**
 * Sends token request and receive data from all SFPs of pexor device handle,
 * marked bitwise in l_sfp_p pattern: 1: sfp 0, 2: sfp 1, 4: sfp 2, 8: sfp 3, 0xf: all four SFPs.
 *  with l_toggle word (sets frontend buffer).
 * l_ldma_target specifies physical address of target buffer for token data DMA.
 * returns some result check words:
 * pl_transfersize:  size of transferred dma in bytes.
 * pl_check_comm: l_comm.
 * pl_check_token: toggle and mode bits.
 * pl_check_slaves: nr. of slaves connected to token chain.
 * The data written to l_ldma_target contains DMA read token contents of all sfps, separated by the
 * optional DMA padding words 0xaddXXXXX as defined for MBS readout.
 */
int mbspex_send_and_receive_parallel_tok (int handle, long l_sfp_p, long l_toggle, unsigned long l_dma_target,
    unsigned long* pl_transfersize, long *pl_check_comm, long *pl_check_token, long *pl_check_slaves);



/** sends token to all SFPs of pexor device handle,
 * marked bitwise in l_sfp_p pattern: 1: sfp 0, 2: sfp 1, 4: sfp 2, 8: sfp 3, 0xf: all four SFPs
 * toggle specifies */
int mbspex_send_tok (int handle, long l_sfp_p, long l_toggle);

/** receive token data from l_sfp after previous request from pexor device handle
 *  l_ldma_target specifies physical address of target buffer for token data DMA
 * returns some result check words:
 * pl_transfersize:  size of transferred dma in bytes
 * pl_check_comm: l_comm
 * pl_check_token: toggle bit
 * pl_check_slaves: nr. of slaves connected to token chain
 */
int mbspex_receive_tok (int handle, long l_sfp, unsigned long l_dma_target, unsigned long *pl_transfersize,
    long *pl_check_comm, long *pl_check_token, long *pl_check_slaves);


/** read token data size of sfp and slave id from internal pex registers*/
long mbspex_get_tok_datasize(int handle, long l_sfp,  long slave_id );

/** read token memory size of sfp from internal pex registers*/
long mbspex_get_tok_memsize(int handle , long l_sfp );

/** retrieve actual slave configuration at sfp chains and put to external structure*/
int mbspex_get_configured_slaves(int handle , struct pex_sfp_links* setup);



/** write value of l_dat to board l_address on mapped bar*/
int mbspex_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat);

/** read value of &l_dat from board l_address on mapped bar*/
int mbspex_register_rd (int handle, unsigned char s_bar, long l_address, long * l_dat);

/** transfer dma of size bytes from board source to host dest addresses.
 * burst size may be specified, or 0 for automatic burst adjustment in driver
 * returns real number of bytes transferred, or -1 in case of error
 * This function will no sooner return than dma is complete*/
int mbspex_dma_rd (int handle, unsigned int source, unsigned long dest, unsigned int size, unsigned int burst);

/** transfer dma of size bytes from board source to virtual user space dest address.
 * Destination memory must be part of the virtual mbs pipe that has been mapped at iniatializatio to sg list
 * burst size may be specified, or 0 for automatic burst adjustment in driver
 * returns real number of bytes transferred, or -1 in case of error
 * This function will no sooner return than dma is complete*/
int mbspex_dma_rd_virt (int handle, unsigned int source, unsigned long virtdest, unsigned int size, unsigned int burst);



/* map user space mbs pipe for dma into sg list in driver
 * startaddress is virtual adress in mbs process, size is pipe length*/
int mbspex_map_pipe (int handle, unsigned long startaddress, unsigned long size);

/* unmap internal sg list*/
int mbspex_unmap_pipe (int handle);



/*void f_feb_init ();*/



#endif
