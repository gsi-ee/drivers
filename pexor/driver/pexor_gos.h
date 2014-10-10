/*
 * \file
 * pexor_gos.h
 *
 *      definitions and functions for pexor with trixor and gosip protocol
 */

#ifndef _PCI_PEXOR_GOS_H_
#define _PCI_PEXOR_GOS_H_


#include "pexor_common.h"


/** enable usage of SFP */
#define PEXOR_WITH_SFP 1

/** enable usage of TRIXOR */
#define PEXOR_WITH_TRIXOR 1


#define PEXOR_DRAM		0x100000 /**< use the first SFP port for DMA testing here*/
//#define PEXOR_RAMSIZE   0xFFFC  /**< test covers first sfp port range here*/
#define PEXOR_RAMSIZE   0x3FFF0 /**< take RAM for all sfps instead */

#define PEXOR_BURST			0x80
#define PEXOR_BURST_MIN		0x10

/** DMA registers and commands:*/
#define PEXOR_DMA_BASE      	0x20000
#define PEXOR_DMA_SRC			0x00
#define PEXOR_DMA_DEST			0x04
#define PEXOR_DMA_LEN			0x08
#define PEXOR_DMA_BURSTSIZE		0x0C
#define PEXOR_DMA_CTRLSTAT		0x10 /**< control, 1:-start*/



 /**OLD REGISTERS pexor 1*/

#define PEXOR_IRQ_CTRL			PEXOR_DMA_BASE + 0x14
#define PEXOR_IRQ_STAT		        PEXOR_DMA_BASE + 0x18


#define PEXOR_TRIXOR_BASE		0x40000

#define PEXOR_TRIX_CTRL 0x04
#define PEXOR_TRIX_STAT 0x00
#define PEXOR_TRIX_FCTI 0x08
#define PEXOR_TRIX_CVTI 0x0C

/** definitions for TRIXOR status and control register
 * taken from mbs driver trig_cam.h*/

/**--- status register bits ---------*/
#define TRIX_DT_CLEAR     0x00000020
#define TRIX_IRQ_CLEAR    0x00001000
#define TRIX_DI_CLEAR     0x00002000
#define TRIX_EV_IRQ_CLEAR 0x00008000
#define TRIX_EON          0x00008000
#define TRIX_FC_PULSE     0x00000010

/**--- control register bits --------*/
#define TRIX_MASTER     0x00000004
#define TRIX_SLAVE      0x00000020
#define TRIX_HALT       0x00000010
#define TRIX_GO         0x00000002
#define TRIX_EN_IRQ     0x00000001
#define TRIX_DIS_IRQ    0x00000008
#define TRIX_CLEAR      0x00000040
#define TRIX_BUS_ENABLE 0x00000800
#define TRIX_BUS_DISABLE 0x00001000




/**
 *  Masks to decode status word of trixor. Taken from mbs sec_mask_def.h
 *
 * N. Kurz
 * definitions to extract special values from the content of the bl_se_ctr_word
 * the following defines are only valid if the trigger module status word
 * (short, 16 bit) is packed in the two highest bytes of an unsigned long,
 * see also the comments at the bl_se_ctr_word declaration in s_sectr.h
 */
#define SEC__MASK_FULL         0x1          /**< indicates if..             */
#define SEC__MASK_SYNC         0x2
#define SEC__MASK_DATA_BUF     0x4
#define SEC__MASK_PIPE_ID      0xff00
#define SEC__MASK_TRG_TYP      0xf0000      /**< trigger type               */
#define SEC__MASK_TDT          0x200000     /**< total dead time on/off     */
#define SEC__MASK_MIS          0x400000     /**< trigger mismatch condition */
#define SEC__MASK_SI           0x800000     /**< subevent invalid           */
#define SEC__MASK_LEC          0x1f000000   /**< local event counter        */
#define SEC__MASK_DI           0x20000000   /**< delay interrupt line       */
#define SEC__MASK_EON          0x80000000   /**< data ready for readout     */





#define PEXOR_DMA_ENABLED_BIT 	0x1

/*#define PEXOR_IRQ_USER_BIT 		0x01*/

#define PEXOR_IRQ_USER_BIT TRIX_EON

/** SFP registers and commands:*/
#define PEXOR_SFP_NUMBER 4 /**< number of used sfp connections*/

#define PEXOR_SFP_BASE 0x21000

#define PEXOR_SFP_REQ_COMM 0x00
#define PEXOR_SFP_REQ_ADDR 0x04
#define PEXOR_SFP_REQ_DATA 0x08
#define PEXOR_SFP_REP_STAT_CLR  0x0c
#define PEXOR_SFP_REP_STAT_0 0x10
#define PEXOR_SFP_REP_STAT_1 0x14
#define PEXOR_SFP_REP_STAT_2 0x18
#define PEXOR_SFP_REP_STAT_3 0x1c
#define PEXOR_SFP_REP_ADDR_0 0x20
#define PEXOR_SFP_REP_ADDR_1 0x24
#define PEXOR_SFP_REP_ADDR_2 0x28
#define PEXOR_SFP_REP_ADDR_3 0x2c
#define PEXOR_SFP_REP_DATA_0 0x30
#define PEXOR_SFP_REP_DATA_1 0x34
#define PEXOR_SFP_REP_DATA_2 0x38
#define PEXOR_SFP_REP_DATA_3 0x3c
#define PEXOR_SFP_RX_MONI 0x40
#define PEXOR_SFP_RX_RST  0x44
#define PEXOR_SFP_DISA 0x48
#define PEXOR_SFP_FAULT 0x4c

#define PEXOR_SFP_FIFO_0 0x50
#define PEXOR_SFP_FIFO_1 0x54
#define PEXOR_SFP_FIFO_2 0x58
#define PEXOR_SFP_FIFO_3 0x5c

#define PEXOR_SFP_TOKEN_REP_STAT_0 0x60
#define PEXOR_SFP_TOKEN_REP_STAT_1 0x64
#define PEXOR_SFP_TOKEN_REP_STAT_2 0x68
#define PEXOR_SFP_TOKEN_REP_STAT_3 0x6c

#define PEXOR_SFP_TOKEN_REP_HEAD_0 0x70
#define PEXOR_SFP_TOKEN_REP_HEAD_1 0x74
#define PEXOR_SFP_TOKEN_REP_HEAD_2 0x78
#define PEXOR_SFP_TOKEN_REP_HEAD_3 0x7c

#define PEXOR_SFP_TOKEN_REP_FOOT_0 0x80
#define PEXOR_SFP_TOKEN_REP_FOOT_1 0x84
#define PEXOR_SFP_TOKEN_REP_FOOT_2 0x88
#define PEXOR_SFP_TOKEN_REP_FOOT_3 0x8c

#define PEXOR_SFP_TOKEN_DSIZE_0 0x90
#define PEXOR_SFP_TOKEN_DSIZE_1 0x94
#define PEXOR_SFP_TOKEN_DSIZE_2 0x98
#define PEXOR_SFP_TOKEN_DSIZE_3 0x9c

#define PEXOR_SFP_TOKEN_DSIZE_SEL_0 0xa0
#define PEXOR_SFP_TOKEN_DSIZE_SEL_1 0xa4
#define PEXOR_SFP_TOKEN_DSIZE_SEL_2 0xa8
#define PEXOR_SFP_TOKEN_DSIZE_SEL_3 0xac

#define PEXOR_SFP_TOKEN_MEM_SIZE_0 0xb0
#define PEXOR_SFP_TOKEN_MEM_SIZE_1 0xb4
#define PEXOR_SFP_TOKEN_MEM_SIZE_2 0xb8
#define PEXOR_SFP_TOKEN_MEM_SIZE_3 0xbc

#define PEXOR_SFP_TX_STAT 0xc0


#define PEXOR_SFP_VERSION 0x1fc



#define PEXOR_SFP_TK_MEM_0 0x100000
#define PEXOR_SFP_TK_MEM_1 0x140000
#define PEXOR_SFP_TK_MEM_2 0x180000
#define PEXOR_SFP_TK_MEM_3 0x1c0000

#define PEXOR_SFP_TK_MEM_RANGE 0xFFFC /**< length of each sfp port mem*/

/* command and reply masks:*/
#define PEXOR_SFP_PT_AD_R_REQ 0x240
#define PEXOR_SFP_PT_AD_W_REQ 0x644

#define PEXOR_SFP_PT_TK_R_REQ 0xA11
#define PEXOR_SFP_INI_REQ 0x344
#define PEXOR_SFP_RST_REQ 0x744

#define PEXOR_SFP_PT_AD_R_REP 0x044
#define PEXOR_SFP_PT_AD_W_REP 0x440
#define PEXOR_SFP_PT_TK_R_REP 0xA11 /*0x844*/
#define PEXOR_SFP_PT_INI_REP 0x244
#define PEXOR_SFP_PT_ERR_REP 0x544




#define pexor_sfp_assert_channel(ch)                                    \
  if(ch < 0 || ch >= PEXOR_SFP_NUMBER)                                  \
    {                                                                   \
      pexor_msg(KERN_WARNING "*** channel %d out of range [%d,%d]! \n", \
                ch, 0 , PEXOR_SFP_NUMBER-1);                            \
      return -EFAULT;                                                   \
    }


#define pexor_sfp_delay()                       \
  mb();                                         \
  ndelay(200);

/*udelay(10);*/





/* this structure contains pointers to sfp registers:*/
struct pexor_sfp
{
  u32 *version;                 /**< Program date and version */
  u32 *req_comm;                /**< Request command */
  u32 *req_addr;                /**< Request address */
  u32 *req_data;                /**< Request data */
  u32 *rep_stat_clr;            /**< Reply status flags and clear for all sfp */
  u32 *rep_stat[PEXOR_SFP_NUMBER];      /**< Reply status for sfp 0...3 */
  u32 *rep_addr[PEXOR_SFP_NUMBER];      /**< Reply adresses for sfp 0...3 */
  u32 *rep_data[PEXOR_SFP_NUMBER];      /**< Reply data for sfp 0...3 */
  u32 *rx_moni;                 /**< Receive monitor */
  u32 *tx_stat;                 /**< Transmit status */
  u32 *reset;                   /**< rx/tx reset */
  u32 *disable;                 /**< disable sfps */
  u32 *fault;                   /**< fault flags */
  u32 *fifo[PEXOR_SFP_NUMBER];  /**< debug access to fifos of sfp 0...3 */
  u32 *tk_stat[PEXOR_SFP_NUMBER];       /**< token reply status of sfp 0...3 */
  u32 *tk_head[PEXOR_SFP_NUMBER];       /**< token reply header of sfp 0...3 */
  u32 *tk_foot[PEXOR_SFP_NUMBER];       /**< token reply footer of sfp 0...3 */
  u32 *tk_dsize[PEXOR_SFP_NUMBER];      /**< token datasize(byte) of sfp 0...3 */
  u32 *tk_dsize_sel[PEXOR_SFP_NUMBER];  /**< selects slave module ID in sfp 0...3
                                           for reading token datasize */
  u32 *tk_memsize[PEXOR_SFP_NUMBER];    /**< memory size filled by token
                                           data transfer for sfp 0...3 */
  u32 *tk_mem[PEXOR_SFP_NUMBER];        /**< memory area filled by token
                                           data transfer for sfp 0...3 */
  dma_addr_t tk_mem_dma[PEXOR_SFP_NUMBER];  /**< token data memory area
                                               expressed as dma bus address */
  int num_slaves[PEXOR_SFP_NUMBER]; /**< number of initialized slaves, for bus broadcast*/
};


void set_sfp(struct pexor_sfp *sfp, void *membase, unsigned long bar);
void print_sfp(struct pexor_sfp *sfp);
void pexor_show_version(struct pexor_sfp *sfp, char *buf);

/** issue receive reset for all sfps*/
void pexor_sfp_reset( struct pexor_privdata* privdata);

/** send request command comm to sfp address addr with optional send data.
 * will not wait for response! */
void pexor_sfp_request(struct pexor_privdata *privdata, u32 comm, u32 addr,
                       u32 data);

/** wait for sfp reply on channel ch.
 * return values are put into comm, addr, and data.
 * checkvalue specifies which return type is expected;
 * will return error if not matching */
int pexor_sfp_get_reply(struct pexor_privdata *privdata, int ch, u32 * comm,
                        u32 * addr, u32 * data, u32 checkvalue);

/** wait for sfp token reply on channel ch.
 * return values are put into stat, head, and foot. */
int pexor_sfp_get_token_reply(struct pexor_privdata *privdata, int ch,
                              u32 * stat, u32 * head, u32 * foot);



/** initialize the connected slaves on sfp channel ch */
int pexor_sfp_init_request(struct pexor_privdata *privdata, int ch,
                           int numslaves);


/** clear all sfp connections and wait until complete.
 * return value specifies error if not 0 */
int pexor_sfp_clear_all(struct pexor_privdata *privdata);

/** clear sfp channel ch and wait for success
 * return value specifies error if not 0 */
int pexor_sfp_clear_channel(struct pexor_privdata *privdata, int ch);

/** clear sfp channelpattern ch and wait for success
 * return value specifies error if not 0 */
int pexor_sfp_clear_channelpattern (struct pexor_privdata* privdata, int pat);

/** Initiate reading a token buffer from sfp front end hardware.
 * In synchronous mode, will block until transfer is done and delivers back dma buffer with token data.
 * In asynchronous mode, function returns immediately after token request;
 * user needs to ioctl a wait token afterwards.
 * Setup and data contained in user arg structure */
int pexor_ioctl_request_token(struct pexor_privdata *priv, unsigned long arg);


/** Waits for a token to arrive previously requested by
 * an asynchronous ioctl request token
 * Setup and data contained in user arg structure */
int pexor_ioctl_wait_token(struct pexor_privdata *priv, unsigned long arg);


/** Wait for a trigger interrupt from pexor. Will be raised from trixor board
 * and routed to pci throug pexor driver. */
int pexor_ioctl_wait_trigger(struct pexor_privdata *priv, unsigned long arg);


/** initialize sfp fieldbus of frontends*/
int pexor_ioctl_init_bus(struct pexor_privdata* priv, unsigned long arg);


/** write to sfp fieldbus of frontends*/
int pexor_ioctl_write_bus(struct pexor_privdata* priv, unsigned long arg);

/** read from sfp fieldbus of frontends*/
int pexor_ioctl_read_bus(struct pexor_privdata* priv, unsigned long arg);

/** write list of configuration parameters to frontends*/
int pexor_ioctl_configure_bus(struct pexor_privdata* priv, unsigned long arg);

/** pass information about configured slaves to user*/
int pexor_ioctl_get_sfp_links(struct pexor_privdata* privdata, unsigned long arg);

/** write values as specified in data to frontend and optionally treat broadcast write
 * if sfp or slave numbers are negative*/
int pexor_sfp_broadcast_write_bus(struct pexor_privdata* priv, struct pexor_bus_io* data);

/** write values as specified in data to frontends*/
int pexor_sfp_write_bus(struct pexor_privdata* priv, struct pexor_bus_io* data);

/** read values as specified in data to frontends. results are in data structure*/
int pexor_sfp_read_bus(struct pexor_privdata* priv, struct pexor_bus_io* data);



/** decode trixor status word into trigger status structure*/
void pexor_decode_triggerstatus(u32 trixorstat, struct pexor_trigger_status* result);


/** reset trigger state after reading out corresponding data*/
int pexor_trigger_reset(struct pexor_privdata* priv);

/** initiate start acquisition*/
int pexor_trigger_start_acq(struct pexor_privdata* priv);

/** initiate stop acquisition */
int pexor_trigger_stop_acq(struct pexor_privdata* priv);


/** perform stop acquisition action. This is done after software trigger 15 is received.*/
int pexor_trigger_do_stop(struct pexor_privdata* priv);



#ifdef PEXOR_WITH_TRIXOR
/** set acquisition state of trixor trigger module extension.
 * used to clear deadtime flag from user program and start/stop acquisition mode, etc.*/
int pexor_ioctl_set_trixor(struct pexor_privdata* priv, unsigned long arg);


#endif

#ifdef PEXOR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

ssize_t pexor_sysfs_sfpregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);


#endif
#endif




#endif





