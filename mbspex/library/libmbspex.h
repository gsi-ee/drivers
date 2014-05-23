#ifndef _PEX_LIBMBSPEX_H_
#define _PEX_LIBMBSPEX_H_

#include "../include/pex_user.h"

/* enable this define if you link against mbs libraries */
/*#define MBSPEX_USEMBS 1*/


#ifdef MBSPEX_USEMBS
#include "f_ut_printm.h"
#else
#define printm printf
#endif


//////////////

/* these internal registers are used from library instead of implementing dedicated ioctl calls.
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

/* open file handle of pex device number devnum. Return value is handle*/
int mbspex_open (int devnum);

/* close file handle*/
int mbspex_close (int handle);

/* read data word *l_dat from sfp, slave and memory offset l_slave_off*/
int mbspex_slave_rd (int handle, long l_sfp, long l_slave, long l_slave_off, long *l_dat);

/* write data word l_dat to sfp, slave and memory offset l_slave_off*/
int mbspex_slave_wr (int handle, long l_sfp, long l_slave, long l_slave_off, long l_dat);

/* initialize chain at l_sfp with l_s_slaves number of slaves*/
int mbspex_slave_init (int handle, long l_sfp, long l_n_slaves);

/* send token request to pexor device of handle at chain sfp
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

/* sends token to all SFPs of pexor device handle,
 * marked bitwise in l_sfp_p pattern: 1: sfp 0, 2: sfp 1, 4: sfp 2, 8: sfp 3, 0xf: all four SFPs
 * toggle specifies */
int mbspex_send_tok (int handle, long l_sfp_p, long l_toggle);

/* receive token data from l_sfp after previous request from pexor device handle
 *  l_ldma_target specifies physical address of target buffer for token data DMA
 * returns some result check words:
 * pl_transfersize:  size of transferred dma in bytes
 * pl_check_comm: l_comm
 * pl_check_token: toggle bit
 * pl_check_slaves: nr. of slaves connected to token chain
 */
int mbspex_receive_tok (int handle, long l_sfp, unsigned long l_dma_target, unsigned long *pl_transfersize,
    long *pl_check_comm, long *pl_check_token, long *pl_check_slaves);


/* read token data size of sfp and slave id from internal pex registers*/
long mbspex_get_tok_datasize(int handle, long l_sfp,  long slave_id );

/* read token memory size of sfp from internal pex registers*/
long mbspex_get_tok_memsize(int handle , long l_sfp );



/* write value of l_dat to board l_address on mapped bar*/
int mbspex_register_wr (int handle, unsigned char s_bar, long l_address, long l_dat);

/* read value of &l_dat from board l_address on mapped bar*/
int mbspex_register_rd (int handle, unsigned char s_bar, long l_address, long * l_dat);

/* transfer dma of size bytes from board source to host dest addresses.
 * burst size may be specified, or 0 for automatic burst adjustment in driver
 * returns real number of bytes transferred, or -1 in case of error
 * This function will no sooner return than dma is complete*/
int mbspex_dma_rd (int handle, long source, long dest, long size, int burst);



/*void f_feb_init ();*/



#endif
