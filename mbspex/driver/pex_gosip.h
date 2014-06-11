/*
 * pex_gosip.h
 *
 *      definitions and functions for pex boards with trixor and gosip protocol
 */

#ifndef _PCI_PEX_GOS_H_
#define _PCI_PEX_GOS_H_


#include "pex_common.h"



/* SFP registers and commands:*/
#define PEX_SFP_NUMBER 4 /* number of used sfp connections*/

#define PEX_SFP_BASE 0x21000

#define PEX_SFP_REQ_COMM 0x00
#define PEX_SFP_REQ_ADDR 0x04
#define PEX_SFP_REQ_DATA 0x08
#define PEX_SFP_REP_STAT_CLR  0x0c
#define PEX_SFP_REP_STAT_0 0x10
#define PEX_SFP_REP_STAT_1 0x14
#define PEX_SFP_REP_STAT_2 0x18
#define PEX_SFP_REP_STAT_3 0x1c
#define PEX_SFP_REP_ADDR_0 0x20
#define PEX_SFP_REP_ADDR_1 0x24
#define PEX_SFP_REP_ADDR_2 0x28
#define PEX_SFP_REP_ADDR_3 0x2c
#define PEX_SFP_REP_DATA_0 0x30
#define PEX_SFP_REP_DATA_1 0x34
#define PEX_SFP_REP_DATA_2 0x38
#define PEX_SFP_REP_DATA_3 0x3c
#define PEX_SFP_RX_MONI 0x40
#define PEX_SFP_RX_RST  0x44
#define PEX_SFP_DISA 0x48
#define PEX_SFP_FAULT 0x4c

#define PEX_SFP_FIFO_0 0x50
#define PEX_SFP_FIFO_1 0x54
#define PEX_SFP_FIFO_2 0x58
#define PEX_SFP_FIFO_3 0x5c

#define PEX_SFP_TOKEN_REP_STAT_0 0x60
#define PEX_SFP_TOKEN_REP_STAT_1 0x64
#define PEX_SFP_TOKEN_REP_STAT_2 0x68
#define PEX_SFP_TOKEN_REP_STAT_3 0x6c

#define PEX_SFP_TOKEN_REP_HEAD_0 0x70
#define PEX_SFP_TOKEN_REP_HEAD_1 0x74
#define PEX_SFP_TOKEN_REP_HEAD_2 0x78
#define PEX_SFP_TOKEN_REP_HEAD_3 0x7c

#define PEX_SFP_TOKEN_REP_FOOT_0 0x80
#define PEX_SFP_TOKEN_REP_FOOT_1 0x84
#define PEX_SFP_TOKEN_REP_FOOT_2 0x88
#define PEX_SFP_TOKEN_REP_FOOT_3 0x8c

#define PEX_SFP_TOKEN_DSIZE_0 0x90
#define PEX_SFP_TOKEN_DSIZE_1 0x94
#define PEX_SFP_TOKEN_DSIZE_2 0x98
#define PEX_SFP_TOKEN_DSIZE_3 0x9c

#define PEX_SFP_TOKEN_DSIZE_SEL_0 0xa0
#define PEX_SFP_TOKEN_DSIZE_SEL_1 0xa4
#define PEX_SFP_TOKEN_DSIZE_SEL_2 0xa8
#define PEX_SFP_TOKEN_DSIZE_SEL_3 0xac

#define PEX_SFP_TOKEN_MEM_SIZE_0 0xb0
#define PEX_SFP_TOKEN_MEM_SIZE_1 0xb4
#define PEX_SFP_TOKEN_MEM_SIZE_2 0xb8
#define PEX_SFP_TOKEN_MEM_SIZE_3 0xbc

#define PEX_SFP_TX_STAT 0xc0


#define PEX_SFP_VERSION 0x1fc


#define PEX_SFP_TK_MEM_0 0x100000
#define PEX_SFP_TK_MEM_1 0x140000
#define PEX_SFP_TK_MEM_2 0x180000
#define PEX_SFP_TK_MEM_3 0x1c0000

#define PEX_SFP_TK_MEM_RANGE 0xFFFC /* length of each sfp port mem*/

/* command and reply masks:*/
#define PEX_SFP_PT_AD_R_REQ 0x240
#define PEX_SFP_PT_AD_W_REQ 0x644

#define PEX_SFP_PT_TK_R_REQ 0xA11
#define PEX_SFP_INI_REQ 0x344
#define PEX_SFP_RST_REQ 0x744

#define PEX_SFP_PT_AD_R_REP 0x044
#define PEX_SFP_PT_AD_W_REP 0x440
#define PEX_SFP_PT_TK_R_REP 0xA11 /*0x844*/
#define PEX_SFP_PT_INI_REP 0x244
#define PEX_SFP_PT_ERR_REP 0x544




#define pex_sfp_assert_channel(ch)                                    \
  if(ch < 0 || ch >= PEX_SFP_NUMBER)                                  \
    {                                                                   \
      pex_msg(KERN_WARNING "*** channel %d out of range [%d,%d]! \n", \
                ch, 0 , PEX_SFP_NUMBER-1);                            \
      return -EFAULT;                                                   \
    }


#define pex_sfp_delay()                       \
  mb();      \
  ndelay(200);

/*  ndelay(20); too short for multiprocesses with semaphore wait?*/
/*udelay(10);*/





/* this structure contains pointers to sfp registers:*/
struct pex_sfp
{
  u32 *version;                 /* Program date and version */
  u32 *req_comm;                /* Request command */
  u32 *req_addr;                /* Request address */
  u32 *req_data;                /* Request data */
  u32 *rep_stat_clr;            /* Reply status flags and clear for all sfp */
  u32 *rep_stat[PEX_SFP_NUMBER];      /* Reply status for sfp 0...3 */
  u32 *rep_addr[PEX_SFP_NUMBER];      /* Reply adresses for sfp 0...3 */
  u32 *rep_data[PEX_SFP_NUMBER];      /* Reply data for sfp 0...3 */
  u32 *rx_moni;                 /* Receive monitor */
  u32 *tx_stat;                 /* Transmit status */
  u32 *reset;                   /* rx/tx reset */
  u32 *disable;                 /* disable sfps */
  u32 *fault;                   /* fault flags */
  u32 *fifo[PEX_SFP_NUMBER];  /* debug access to fifos of sfp 0...3 */
  u32 *tk_stat[PEX_SFP_NUMBER];       /* token reply status of sfp 0...3 */
  u32 *tk_head[PEX_SFP_NUMBER];       /* token reply header of sfp 0...3 */
  u32 *tk_foot[PEX_SFP_NUMBER];       /* token reply footer of sfp 0...3 */
  u32 *tk_dsize[PEX_SFP_NUMBER];      /* token datasize(byte) of sfp 0...3 */
  u32 *tk_dsize_sel[PEX_SFP_NUMBER];  /* selects slave module ID in sfp 0...3
                                           for reading token datasize */
  u32 *tk_memsize[PEX_SFP_NUMBER];    /* memory size filled by token
                                           data transfer for sfp 0...3 */
  u32 *tk_mem[PEX_SFP_NUMBER];        /* memory area filled by token
                                           data transfer for sfp 0...3 */
  dma_addr_t tk_mem_dma[PEX_SFP_NUMBER];  /* token data memory area
                                               expressed as dma bus address */
  int num_slaves[PEX_SFP_NUMBER]; /* number of initialized slaves, for bus broadcast*/
};


void set_sfp(struct pex_sfp *sfp, void *membase, unsigned long bar);
void print_sfp(struct pex_sfp *sfp);
void pex_show_version(struct pex_sfp *sfp, char *buf);

/* issue receive reset for all sfps*/
void pex_sfp_reset( struct pex_privdata* privdata);

/* send request command comm to sfp address addr with optional send data.
 * will not wait for response! */
void pex_sfp_request(struct pex_privdata *privdata, u32 comm, u32 addr,
                       u32 data);

/* wait for sfp reply on channel ch.
 * return values are put into comm, addr, and data.
 * checkvalue specifies which return type is expected;
 * will return error if not matching */
int pex_sfp_get_reply(struct pex_privdata *privdata, int ch, u32 * comm,
                        u32 * addr, u32 * data, u32 checkvalue);

/* wait for sfp token reply on channel ch.
 * return values are put into stat, head, and foot. */
int pex_sfp_get_token_reply(struct pex_privdata *privdata, int ch,
                              u32 * stat, u32 * head, u32 * foot);



/* initialize the connected slaves on sfp channel ch */
int pex_sfp_init_request(struct pex_privdata *privdata, int ch,
                           int numslaves);


/* clear all sfp connections and wait until complete.
 * return value specifies error if not 0 */
int pex_sfp_clear_all(struct pex_privdata *privdata);

/* clear sfp channel ch and wait for success
 * return value specifies error if not 0 */
int pex_sfp_clear_channel(struct pex_privdata *privdata, int ch);

/* clear sfp channel pattern pat before broadcast and wait for success
 * return value specifies error if not 0 */
int pex_sfp_clear_channelpattern(struct pex_privdata *privdata, int pat);


/* Initiate reading a token buffer from sfp front end hardware.
 * In synchronous mode, will block until transfer is done and delivers back dma buffer with token data.
 * In asynchronous mode, function returns immediately after token request;
 * user needs to ioctl a wait token afterwards.
 * Setup and data contained in user arg structure */
int pex_ioctl_request_token(struct pex_privdata *priv, unsigned long arg);


/* Waits for a token to arrive previously requested by
 * an asynchronous ioctl request token
 * Setup and data contained in user arg structure */
int pex_ioctl_wait_token(struct pex_privdata *priv, unsigned long arg);


/* initialize sfp fieldbus of frontends*/
int pex_ioctl_init_bus(struct pex_privdata* priv, unsigned long arg);


/* write to sfp fieldbus of frontends*/
int pex_ioctl_write_bus(struct pex_privdata* priv, unsigned long arg);

/* read from sfp fieldbus of frontends*/
int pex_ioctl_read_bus(struct pex_privdata* priv, unsigned long arg);

/* write list of configuration parameters to frontends*/
int pex_ioctl_configure_bus(struct pex_privdata* priv, unsigned long arg);

/* write values as specified in data to frontend and optionally treat broadcast write
 * if sfp or slave numbers are negative*/
int pex_sfp_broadcast_write_bus(struct pex_privdata* priv, struct pex_bus_io* data);

/* write values as specified in data to frontends*/
int pex_sfp_write_bus(struct pex_privdata* priv, struct pex_bus_io* data);

/* read values as specified in data to frontends. results are in data structure*/
int pex_sfp_read_bus(struct pex_privdata* priv, struct pex_bus_io* data);



#ifdef PEX_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

ssize_t pex_sysfs_sfpregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);


#endif
#endif




#endif





