/*
 * \file
 * pexornet_gosip.h
 *
 *      definitions and functions for pexornet with trixor and gosip protocol
 */

#ifndef _PEXORNET_GOS_H_
#define _PEXORNET_GOS_H_



#define PEXORNET_DRAM		0x100000 /**< use the first SFP port for DMA testing here*/
//#define PEXORNET_RAMSIZE   0xFFFC  /**< test covers first sfp port range here*/
#define PEXORNET_RAMSIZE   0x3FFF0 /**< take RAM for all sfps instead */

#define PEXORNET_BURST			0x80
#define PEXORNET_BURST_MIN		0x10

/** DMA registers and commands:*/
#define PEXORNET_DMA_BASE      	0x20000
#define PEXORNET_DMA_SRC			0x00
#define PEXORNET_DMA_DEST			0x04
#define PEXORNET_DMA_LEN			0x08
#define PEXORNET_DMA_BURSTSIZE		0x0C
#define PEXORNET_DMA_CTRLSTAT		0x10 /**< control, 1:-start*/



 /**OLD REGISTERS pexornet 1*/

#define PEXORNET_IRQ_CTRL			PEXORNET_DMA_BASE + 0x14
#define PEXORNET_IRQ_STAT		        PEXORNET_DMA_BASE + 0x18


#define PEXORNET_TRIXOR_BASE		0x40000

#define PEXORNET_TRIX_CTRL 0x04
#define PEXORNET_TRIX_STAT 0x00
#define PEXORNET_TRIX_FCTI 0x08
#define PEXORNET_TRIX_CVTI 0x0C

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





#define PEXORNET_DMA_ENABLED_BIT 	0x1

/*#define PEXORNET_IRQ_USER_BIT 		0x01*/

#define PEXORNET_IRQ_USER_BIT TRIX_EON

/** SFP registers and commands:*/
#define PEXORNET_SFP_NUMBER 4 /**< number of used sfp connections*/

#define PEXORNET_SFP_BASE 0x21000

#define PEXORNET_SFP_REQ_COMM 0x00
#define PEXORNET_SFP_REQ_ADDR 0x04
#define PEXORNET_SFP_REQ_DATA 0x08
#define PEXORNET_SFP_REP_STAT_CLR  0x0c
#define PEXORNET_SFP_REP_STAT_0 0x10
#define PEXORNET_SFP_REP_STAT_1 0x14
#define PEXORNET_SFP_REP_STAT_2 0x18
#define PEXORNET_SFP_REP_STAT_3 0x1c
#define PEXORNET_SFP_REP_ADDR_0 0x20
#define PEXORNET_SFP_REP_ADDR_1 0x24
#define PEXORNET_SFP_REP_ADDR_2 0x28
#define PEXORNET_SFP_REP_ADDR_3 0x2c
#define PEXORNET_SFP_REP_DATA_0 0x30
#define PEXORNET_SFP_REP_DATA_1 0x34
#define PEXORNET_SFP_REP_DATA_2 0x38
#define PEXORNET_SFP_REP_DATA_3 0x3c
#define PEXORNET_SFP_RX_MONI 0x40
#define PEXORNET_SFP_RX_RST  0x44
#define PEXORNET_SFP_DISA 0x48
#define PEXORNET_SFP_FAULT 0x4c

#define PEXORNET_SFP_FIFO_0 0x50
#define PEXORNET_SFP_FIFO_1 0x54
#define PEXORNET_SFP_FIFO_2 0x58
#define PEXORNET_SFP_FIFO_3 0x5c

#define PEXORNET_SFP_TOKEN_REP_STAT_0 0x60
#define PEXORNET_SFP_TOKEN_REP_STAT_1 0x64
#define PEXORNET_SFP_TOKEN_REP_STAT_2 0x68
#define PEXORNET_SFP_TOKEN_REP_STAT_3 0x6c

#define PEXORNET_SFP_TOKEN_REP_HEAD_0 0x70
#define PEXORNET_SFP_TOKEN_REP_HEAD_1 0x74
#define PEXORNET_SFP_TOKEN_REP_HEAD_2 0x78
#define PEXORNET_SFP_TOKEN_REP_HEAD_3 0x7c

#define PEXORNET_SFP_TOKEN_REP_FOOT_0 0x80
#define PEXORNET_SFP_TOKEN_REP_FOOT_1 0x84
#define PEXORNET_SFP_TOKEN_REP_FOOT_2 0x88
#define PEXORNET_SFP_TOKEN_REP_FOOT_3 0x8c

#define PEXORNET_SFP_TOKEN_DSIZE_0 0x90
#define PEXORNET_SFP_TOKEN_DSIZE_1 0x94
#define PEXORNET_SFP_TOKEN_DSIZE_2 0x98
#define PEXORNET_SFP_TOKEN_DSIZE_3 0x9c

#define PEXORNET_SFP_TOKEN_DSIZE_SEL_0 0xa0
#define PEXORNET_SFP_TOKEN_DSIZE_SEL_1 0xa4
#define PEXORNET_SFP_TOKEN_DSIZE_SEL_2 0xa8
#define PEXORNET_SFP_TOKEN_DSIZE_SEL_3 0xac

#define PEXORNET_SFP_TOKEN_MEM_SIZE_0 0xb0
#define PEXORNET_SFP_TOKEN_MEM_SIZE_1 0xb4
#define PEXORNET_SFP_TOKEN_MEM_SIZE_2 0xb8
#define PEXORNET_SFP_TOKEN_MEM_SIZE_3 0xbc

#define PEXORNET_SFP_TX_STAT 0xc0


#define PEXORNET_SFP_VERSION 0x1fc



#define PEXORNET_SFP_TK_MEM_0 0x100000
#define PEXORNET_SFP_TK_MEM_1 0x140000
#define PEXORNET_SFP_TK_MEM_2 0x180000
#define PEXORNET_SFP_TK_MEM_3 0x1c0000

#define PEXORNET_SFP_TK_MEM_RANGE 0xFFFC /**< length of each sfp port mem*/

/* command and reply masks:*/
#define PEXORNET_SFP_PT_AD_R_REQ 0x240
#define PEXORNET_SFP_PT_AD_W_REQ 0x644

#define PEXORNET_SFP_PT_TK_R_REQ 0xA11
#define PEXORNET_SFP_INI_REQ 0x344
#define PEXORNET_SFP_RST_REQ 0x744

#define PEXORNET_SFP_PT_AD_R_REP 0x044
#define PEXORNET_SFP_PT_AD_W_REP 0x440
#define PEXORNET_SFP_PT_TK_R_REP 0xA11 /*0x844*/
#define PEXORNET_SFP_PT_INI_REP 0x244
#define PEXORNET_SFP_PT_ERR_REP 0x544


/* default maximum number of polls for sfp request response.
 * Initializes privdata->sfp_maxpolls
 * This causes a timeout value =(PEXORNET_SFP_MAXPOLLS * PEXORNET_SFP_DELAY ns)
 * note that this default can be tuned by setting privdata->sfp_maxpolls
 * via sysfs handle*/
#define PEXORNET_SFP_MAXPOLLS 10000

/* delay in nanoseconds (ns) for any operation on gosip sfp protocol*/
#define PEXORNET_SFP_DELAY 20

#define pexornet_sfp_assert_channel(ch)                                    \
  if(ch < 0 || ch >= PEXORNET_SFP_NUMBER)                                  \
    {                                                                   \
      pexornet_msg(KERN_WARNING "*** channel %d out of range [%d,%d]! \n", \
                ch, 0 , PEXORNET_SFP_NUMBER-1);                            \
      return -EFAULT;                                                   \
    }

/* pseudo function to work with delay:*/
#define pexornet_sfp_delay()                       \
  mb();      \
  ndelay(PEXORNET_SFP_DELAY);


/*udelay(10);*/





/* this structure contains pointers to sfp registers:*/
struct pexornet_sfp
{
  u32 *version;                 /**< Program date and version */
  u32 *req_comm;                /**< Request command */
  u32 *req_addr;                /**< Request address */
  u32 *req_data;                /**< Request data */
  u32 *rep_stat_clr;            /**< Reply status flags and clear for all sfp */
  u32 *rep_stat[PEXORNET_SFP_NUMBER];      /**< Reply status for sfp 0...3 */
  u32 *rep_addr[PEXORNET_SFP_NUMBER];      /**< Reply adresses for sfp 0...3 */
  u32 *rep_data[PEXORNET_SFP_NUMBER];      /**< Reply data for sfp 0...3 */
  u32 *rx_moni;                 /**< Receive monitor */
  u32 *tx_stat;                 /**< Transmit status */
  u32 *reset;                   /**< rx/tx reset */
  u32 *disable;                 /**< disable sfps */
  u32 *fault;                   /**< fault flags */
  u32 *fifo[PEXORNET_SFP_NUMBER];  /**< debug access to fifos of sfp 0...3 */
  u32 *tk_stat[PEXORNET_SFP_NUMBER];       /**< token reply status of sfp 0...3 */
  u32 *tk_head[PEXORNET_SFP_NUMBER];       /**< token reply header of sfp 0...3 */
  u32 *tk_foot[PEXORNET_SFP_NUMBER];       /**< token reply footer of sfp 0...3 */
  u32 *tk_dsize[PEXORNET_SFP_NUMBER];      /**< token datasize(byte) of sfp 0...3 */
  u32 *tk_dsize_sel[PEXORNET_SFP_NUMBER];  /**< selects slave module ID in sfp 0...3
                                           for reading token datasize */
  u32 *tk_memsize[PEXORNET_SFP_NUMBER];    /**< memory size filled by token
                                           data transfer for sfp 0...3 */
  u32 *tk_mem[PEXORNET_SFP_NUMBER];        /**< memory area filled by token
                                           data transfer for sfp 0...3 */
  dma_addr_t tk_mem_dma[PEXORNET_SFP_NUMBER];  /**< token data memory area
                                               expressed as dma bus address */
  int num_slaves[PEXORNET_SFP_NUMBER]; /**< number of initialized slaves, for bus broadcast*/
};


void set_sfp(struct pexornet_sfp *sfp, void *membase, unsigned long bar);
void print_sfp(struct pexornet_sfp *sfp);
void pexornet_show_version(struct pexornet_sfp *sfp, char *buf);

/** issue receive reset for all sfps*/
void pexornet_sfp_reset( struct pexornet_privdata* privdata);

/** send request command comm to sfp address addr with optional send data.
 * will not wait for response! */
void pexornet_sfp_request(struct pexornet_privdata *privdata, u32 comm, u32 addr,
                       u32 data);

/** wait for sfp reply on channel ch.
 * return values are put into comm, addr, and data.
 * checkvalue specifies which return type is expected;
 * will return error if not matching */
int pexornet_sfp_get_reply(struct pexornet_privdata *privdata, int ch, u32 * comm,
                        u32 * addr, u32 * data, u32 checkvalue);

/** wait for sfp token reply on channel ch.
 * return values are put into stat, head, and foot. */
int pexornet_sfp_get_token_reply(struct pexornet_privdata *privdata, int ch,
                              u32 * stat, u32 * head, u32 * foot);



/** initialize the connected slaves on sfp channel ch */
int pexornet_sfp_init_request(struct pexornet_privdata *privdata, int ch,
                           int numslaves);


/** clear all sfp connections and wait until complete.
 * return value specifies error if not 0 */
int pexornet_sfp_clear_all(struct pexornet_privdata *privdata);

/** clear sfp channel ch and wait for success
 * return value specifies error if not 0 */
int pexornet_sfp_clear_channel(struct pexornet_privdata *privdata, int ch);

/** clear sfp channelpattern ch and wait for success
 * return value specifies error if not 0 */
int pexornet_sfp_clear_channelpattern (struct pexornet_privdata* privdata, int pat);

///** Initiate reading a token buffer from sfp front end hardware.
// * In synchronous mode, will block until transfer is done and delivers back dma buffer with token data.
// * In asynchronous mode, function returns immediately after token request;
// * user needs to ioctl a wait token afterwards.
// * Setup and data contained in user arg structure */
//int pexornet_ioctl_request_token(struct pexornet_privdata *priv, unsigned long arg);
//
//
///** Waits for a token to arrive previously requested by
// * an asynchronous ioctl request token
// * Setup and data contained in user arg structure */
//int pexornet_ioctl_wait_token(struct pexornet_privdata *priv, unsigned long arg);
//
//
///** Wait for a trigger interrupt from pexornet. Will be raised from trixor board
// * and routed to pci throug pexornet driver. */
//int pexornet_ioctl_wait_trigger(struct pexornet_privdata *priv, unsigned long arg);


/** initialize sfp fieldbus of frontends*/
int pexornet_ioctl_init_bus(struct pexornet_privdata* priv, unsigned long arg);


/** write to sfp fieldbus of frontends*/
int pexornet_ioctl_write_bus(struct pexornet_privdata* priv, unsigned long arg);

/** read from sfp fieldbus of frontends*/
int pexornet_ioctl_read_bus(struct pexornet_privdata* priv, unsigned long arg);

/** write list of configuration parameters to frontends*/
int pexornet_ioctl_configure_bus(struct pexornet_privdata* priv, unsigned long arg);

/** pass information about configured slaves to user*/
int pexornet_ioctl_get_sfp_links(struct pexornet_privdata* privdata, unsigned long arg);

/** write values as specified in data to frontend and optionally treat broadcast write
 * if sfp or slave numbers are negative*/
int pexornet_sfp_broadcast_write_bus(struct pexornet_privdata* priv, struct pexornet_bus_io* data);

/** write values as specified in data to frontends*/
int pexornet_sfp_write_bus(struct pexornet_privdata* priv, struct pexornet_bus_io* data);

/** read values as specified in data to frontends. results are in data structure*/
int pexornet_sfp_read_bus(struct pexornet_privdata* priv, struct pexornet_bus_io* data);






#ifdef PEXORNET_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

ssize_t pexornet_sysfs_sfpregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);


#endif
#endif




#endif





