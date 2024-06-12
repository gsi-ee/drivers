/** \file
 * pex_gosip.h
 *
 *      definitions and functions for pex boards with trixor and gosip protocol
 */

#ifndef _PCI_PEX_GOS_H_
#define _PCI_PEX_GOS_H_


#include "pex_common.h"

/** JAM 18-09-23: switch to version 5 of kinpex gosip engine */
#define PEX_SFP_USE_KINPEX_V5 1

/* SFP registers and commands: this definition was moved to pex_user.h*/
/*#define PEX_SFP_NUMBER 4  number of used sfp connections */

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

// JAM 2024: new for changing gosip link speed
#define PEX_SFP_DRP_PORT 0xc8

#define PEX_SFP_VERSION 0x1fc


#define PEX_SFP_TK_MEM_0 0x100000
#define PEX_SFP_TK_MEM_1 0x140000
#define PEX_SFP_TK_MEM_2 0x180000
#define PEX_SFP_TK_MEM_3 0x1c0000

#define PEX_SFP_TK_MEM_RANGE 0xFFFC /**< length of each sfp port mem*/

/** command and reply masks:*/
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

/** DRP address prefixes JAM2024*/

#define PEX_DRP_GTX_R_0 0x0
#define PEX_DRP_GTX_R_1 0x1
#define PEX_DRP_GTX_R_2 0x2
#define PEX_DRP_GTX_R_3 0x3

#define PEX_DRP_GTX_W_0 0x8
#define PEX_DRP_GTX_W_1 0x9
#define PEX_DRP_GTX_W_2 0xA
#define PEX_DRP_GTX_W_3 0xB

#define PEX_DRP_MMC_R 0x5
#define PEX_DRP_MMC_W 0xD

#define PEX_DRP_GTX_CMN_R 0x4
#define PEX_DRP_GTX_CMN_W 0xC

/** KINPEX DRP addresses JAM2024
 * Please see:
 *
   https://docs.amd.com/v/u/en-US/ug476_7Series_Transceivers

   https://docs.amd.com/v/u/en-US/ug472_7Series_Clocking

   https://docs.amd.com/v/u/en-US/xapp888_7Series_DynamicRecon

 *
 * */



#define PEX_DRP_GTX_CPLL        0x05E
#define PEX_DRP_GTX_TXRX_DIV    0x088
#define PEX_DRP_GTX_RXCDR_CFG   0x0a9

#define PEX_DRP_GTX_QPLL        0x036

#define PEX_DRP_MMC_CLKOUT0_1   0x008
#define PEX_DRP_MMC_CLKOUT0_2   0x009
#define PEX_DRP_MMC_CLKBUFOUT_1 0x014
#define PEX_DRP_MMC_CLKBUFOUT_2 0x015
#define PEX_DRP_MMC_DIVCLK      0x016
#define PEX_DRP_MMC_LOCK_1      0x018
#define PEX_DRP_MMC_LOCK_2      0x019
#define PEX_DRP_MMC_LOCK_3      0x01A
#define PEX_DRP_MMC_FILT_1      0x04E
#define PEX_DRP_MMC_FILT_2      0x04F



/* here bitmask evaluations of register contents:*/

#define PEX_GET_SATA_CPLL_CFG(X)    ((X >>14) & 0x11)
#define PEX_GET_CPLL_REFCLK_DIV(X)  ((X >>8) & 0x1F)
#define PEX_GET_CPLL_FBDIV_45(X)    ((X >>7) & 0x1)
#define PEX_GET_CPLL_FBDIV(X)       ((X >>0) & 0x3F)
#define PEX_GET_TXOUT_DIV(X)       ((X >>4) & 0x7)
#define PEX_GET_RXOUT_DIV(X)       ((X >>0) & 0x7)

#define PEX_GET_QPLL_FBDIV(X)       ((X >>0) & 0x3FF)


#define PEX_CLEAR_SATA_CPLL_CFG(X)    X &= ~(0x11<<14)
#define PEX_CLEAR_CPLL_REFCLK_DIV(X)  X &= ~(0x1F<<8)
#define PEX_CLEAR_CPLL_FBDIV_45(X)    X &= ~(0x1<<7)
#define PEX_CLEAR_CPLL_FBDIV(X)       X &= ~(0x3F)
#define PEX_CLEAR_TXOUT_DIV(X)        X &= ~(0x7<<4)
#define PEX_CLEAR_RXOUT_DIV(X)        X &= ~(0x7)

#define PEX_CLEAR_QPLL_FBDIV(X)       X &= ~(0x3FF)


#define PEX_SET_SATA_CPLL_CFG(X, V)    X|= ((V & 0x11)<<14)
#define PEX_SET_CPLL_REFCLK_DIV(X, V)  X |= ((V & 0x1F) << 8)
#define PEX_SET_CPLL_FBDIV_45(X, V)    X |= ((V & 0x1)<<7)
#define PEX_SET_CPLL_FBDIV(X, V)       X |= (V & 0x3F)
#define PEX_SET_TXOUT_DIV(X, V)        X |= ((V & 0x7)<<4)
#define PEX_SET_RXOUT_DIV(X, V)        X |= (V & 0x7)

#define PEX_SET_QPLL_FBDIV(X, V)       X |= (V & 0x3FF)


#define PEX_GET_MMCM_HIGH_TIME(X)       ((X >>6) & 0x3F)
#define PEX_GET_MMCM_LOW_TIME(X)        ((X >>0) & 0x3F)
#define PEX_GET_MMCM_CLK_EDGE(X)        ((X >>7) & 0x1)
#define PEX_GET_MMCM_DIVCLK_EDGE(X)     ((X >>13) & 0x1)

#define PEX_CLEAR_MMCM_HIGH_TIME(X)     X &=  ~(0x3F<<6)
#define PEX_CLEAR_MMCM_LOW_TIME(X)      X &=  ~(0x3F)
#define PEX_CLEAR_MMCM_CLK_EDGE(X)      X &=  ~(0x1<< 7)
#define PEX_CLEAR_MMCM_DIVCLK_EDGE(X)   X &=  ~(0x1<<13)

#define PEX_SET_MMCM_HIGH_TIME(X, V)     X |=  ((V & 0x3F)<<6)
#define PEX_SET_MMCM_LOW_TIME(X, V)      X |= (V & 0x3F)
#define PEX_SET_MMCM_CLK_EDGE(X, V)      X |= ((V & 0x1) << 7)
#define PEX_SET_MMCM_DIVCLK_EDGE(X, V)   X |= ((V & 0x1)<<13)

/* X:lock register 1, Y:lock register 2. Z: lock register 3*/
#define PEX_GET_MMCM_LOCK_TABLE(X,Y,Z) \
  ((X & 0x3FF) << 20) | (Y & 0x3FF) | (((Y >>10) & 0x1F) << 30)  | ((Z & 0x3FF) << 10) | (((Z>>10) & 0x1F) << 35)

/* X:filter register 1, Y:filter register 2 */
#define PEX_GET_MMCM_FILT_TABLE(X,Y) \
  (((X >> 15) & 0x1) << 9) | (((X >> 11) & 0x3) << 7) | (( (X>>8) & 0x1)<<6) | (((Y>>15) & 0x1) <<5) | (((Y >>11) & 0x3)<<3) | (((Y>>7) & 0x3) <<1) | ((Y << 4) & 0x1)





#define PEX_FBDIV_DRPDECODE(X,V) \
    for(i=0;i<16;++i)\
      if(gFbdivCode[i].drp==X) V=gFbdivCode[i].attr;


// JAM24: helpers for inspecting the setups:
#define PEX_GTX_SETUPDUMP(X) \
  pex_msg(KERN_NOTICE "** GTX setup for speed %d (%s);\n", X, gLinkspeed[X]);\
  pex_gtx_register_dump(theSetup, txt, 1024);\
  pex_msg(KERN_NOTICE "%s", txt);\
  pex_gtx_parameter_dump(thePars, txt, 1024);\
  pex_msg(KERN_NOTICE "%s", txt);

#define PEX_MMCM_SETUPDUMP(X) \
  pex_msg(KERN_NOTICE "** MMCM setup for speed %d (%s);\n", X,  gLinkspeed[X]);\
  pex_mmcm_register_dump(theSetup, txt, 1024);\
  pex_msg(KERN_NOTICE "%s", txt);\
  pex_mmcm_parameter_dump(thePars, txt, 1024);\
  pex_msg(KERN_NOTICE "%s", txt);


/* reference clock frequency, MHz*/
#define PEX_GTX_REF_CLOCK 125
// no floats in the kernel please 125.0E+6 :-))
#define PEX_GTX_REF_CLOCK_TEXT "125 MHz"

#define PEX_DRP_CHECK(X,Y) \
    if(X){ \
          pex_msg(KERN_ERR "** pex read/write linkspeed_registers problem: return value %d when reading/writing address 0x%x, never come here!",X,Y);\
          return X;\
        }\

#define PEX_MAX_SPEEDSETUP 6


///////////////////////////////////////////////////77

#define pex_sfp_assert_channel(ch)                                    \
  if(ch < 0 || ch >= PEX_SFP_NUMBER)                                  \
    {                                                                   \
      pex_msg(KERN_WARNING "*** channel %d out of range [%d,%d]! \n", \
                ch, 0 , PEX_SFP_NUMBER-1);                            \
      return -EFAULT;                                                   \
    }




/* default maximum number of polls for sfp request response.
 * Initializes privdata->sfp_maxpolls
 * This causes a timeout value =(PEX_SFP_MAXPOLLS * PEX_SFP_DELAY ns)
 * note that this default can be tuned by setting privdata->sfp_maxpolls
 * via sysfs handle*/
#define PEX_SFP_MAXPOLLS 10000

/* delay in nanoseconds (ns) for any operation on gosip sfp protocol*/
#define PEX_SFP_DELAY 20
//20

/* pseudo function to work with delay:*/
#define pex_sfp_delay()                       \
  mb();      \
  if(PEX_SFP_DELAY>0) ndelay(PEX_SFP_DELAY);


/*  ndelay(20); too short for multiprocesses with semaphore wait?*/
/*udelay(10);*/


 ///////////
// JAM2024: new for changing link speeds: ///////////////////////////////////

/* this has plain register contents*/
 struct pex_gtx_set
 {
    int sfp;                 /**< gtx link id 0..3 (-1 for all configured sfps) = sfp chain id*/
    unsigned short pll;      /**< value of drp pll register PEX_DRP_GTX_CPLL */
    unsigned short div;      /**< value of drp div register PEX_DRP_GTX_TXRX_DIV */
    unsigned short rxcdr;    /**< value of drp rxclock data recovery register PEX_DRP_GTX_RXCDR_CFG */
    unsigned short qpll;      /**< value of drp quad pll register PEX_DRP_GTX_QPLL (same for all link lines) */
 };

 /* this has plain register contents*/
 struct pex_mmcm_set
 {
   unsigned short clkout0[2];    /**< values of drp mmcm registers PEX_DRP_MMC_CLKOUT0_1 and  PEX_DRP_MMC_CLKOUT0_2*/
   unsigned short clkbufout[2];  /**< values of drp mmcm registers PEX_DRP_MMC_CLKBUFOUT_1 and  PEX_DRP_MMC_CLKBUFOUT_2*/
   unsigned short divclk;        /**< value of drp mmcm register PEX_DRP_MMC_DIVCLK */
   unsigned short lock[3];       /**< values of drp mmcm registers PEX_DRP_MMC_LOCK_1 _2 _3*/
   unsigned short filter[2];     /**< values of drp mmcm registers PEX_DRP_MMC_FILT_1 _2 */
 };


/** these are the primitive  parameters for GTX link speed definitions*/
 struct pex_gtx_params
 {
   unsigned char qpll_fdiv;
   unsigned char cpll_refclk_div;
   unsigned char cpll_fbdiv_45;
   unsigned char cpll_fbdiv;
   unsigned char txout_div;
   unsigned char rxout_div;
 };

 struct pex_mmcm_params
  {
    unsigned char clkout0_hitime;
    unsigned char clkout0_lotime;
    unsigned char clkout0_edge;
    unsigned char clkbufout_hitime;
    unsigned char clkbufout_lotime;
    unsigned char clkbufout_edge;
    unsigned char divclk_hitime;
    unsigned char divclk_lotime;
    unsigned char divclk_edge;
    unsigned short lock[3];       /**< plain register values here, or reduced to 40 bit words*/
    unsigned short filter[2];

  };



/** this structure contains pointers to sfp registers:*/
struct pex_sfp
{
  u32 *version;                 /**< Program date and version */
  u32 *req_comm;                /**< Request command */
  u32 *req_addr;                /**< Request address */
  u32 *req_data;                /**< Request data */
  u32 *rep_stat_clr;            /**< Reply status flags and clear for all sfp */
  u32 *rep_stat[PEX_SFP_NUMBER];      /**< Reply status for sfp 0...3 */
  u32 *rep_addr[PEX_SFP_NUMBER];      /**< Reply adresses for sfp 0...3 */
  u32 *rep_data[PEX_SFP_NUMBER];      /**< Reply data for sfp 0...3 */
  u32 *rx_moni;                 /**< Receive monitor */
  u32 *tx_stat;                 /**< Transmit status */
  u32 *reset;                   /**< rx/tx reset */
  u32 *disable;                 /**< disable sfps */
  u32 *fault;                   /**< fault flags */
  u32 *fifo[PEX_SFP_NUMBER];  /**< debug access to fifos of sfp 0...3 */
  u32 *tk_stat[PEX_SFP_NUMBER];       /**< token reply status of sfp 0...3 */
  u32 *tk_head[PEX_SFP_NUMBER];       /**< token reply header of sfp 0...3 */
  u32 *tk_foot[PEX_SFP_NUMBER];       /**< token reply footer of sfp 0...3 */
  u32 *tk_dsize[PEX_SFP_NUMBER];      /**< token datasize(byte) of sfp 0...3 */
  u32 *tk_dsize_sel[PEX_SFP_NUMBER];  /**< selects slave module ID in sfp 0...3
                                           for reading token datasize */
  u32 *tk_memsize[PEX_SFP_NUMBER];    /**< memory size filled by token
                                           data transfer for sfp 0...3 */
  u32 *tk_mem[PEX_SFP_NUMBER];        /**< memory area filled by token
                                           data transfer for sfp 0...3 */
  dma_addr_t tk_mem_dma[PEX_SFP_NUMBER];  /**< token data memory area
                                               expressed as dma bus address */
  int num_slaves[PEX_SFP_NUMBER]; /**< number of initialized slaves, for bus broadcast*/
  // JAM2022 below:
  u32 *drp_port;                 /**< Dynamic reconfiguration port */
  struct pex_gtx_set gtx_setup[PEX_SFP_NUMBER]; /**< gtx tranceiver speed setup of sfp 0...3 */
  struct pex_mmcm_set mmcm_setup;    /**< mixed mode clock manager speed setup */
  enum pex_linkspeed lspeed[PEX_SFP_NUMBER];  /**< link speed presets identifier, per channel */
};


void set_sfp(struct pex_sfp *sfp, void *membase, unsigned long bar);
void print_sfp(struct pex_sfp *sfp);
void pex_show_version(struct pex_sfp *sfp, char *buf);

/** issue receive reset for all sfps*/
void pex_sfp_reset( struct pex_privdata* privdata);

/** send request command comm to sfp address addr with optional send data.
 * will not wait for response! */
void pex_sfp_request(struct pex_privdata *privdata, u32 comm, u32 addr,
                       u32 data);

/** wait for sfp reply on channel ch.
 * return values are put into comm, addr, and data.
 * checkvalue specifies which return type is expected;
 * will return error if not matching */
int pex_sfp_get_reply(struct pex_privdata *privdata, int ch, u32 * comm,
                        u32 * addr, u32 * data, u32 checkvalue);

/** wait for sfp token reply on channel ch.
 * return values are put into stat, head, and foot. */
//int pex_sfp_get_token_reply(struct pex_privdata *privdata, int ch,
//                              u32 * stat, u32 * head, u32 * foot);
//


/** initialize the connected slaves on sfp channel ch */
int pex_sfp_init_request(struct pex_privdata *privdata, int ch,
                           int numslaves);


/** clear all sfp connections and wait until complete.
 * return value specifies error if not 0 */
int pex_sfp_clear_all(struct pex_privdata *privdata);

/** clear sfp channel ch and wait for success.
 * return value specifies error if not 0 */
int pex_sfp_clear_channel(struct pex_privdata *privdata, int ch);

/** clear sfp channel pattern pat before broadcast and wait for success.
 * return value specifies error if not 0 */
int pex_sfp_clear_channelpattern(struct pex_privdata *privdata, int pat);


/** Initiate reading a token buffer from sfp front end hardware.
 * In synchronous mode, will block until transfer is done and delivers back dma buffer with token data.
 * In asynchronous mode, function returns immediately after token request;
 * user needs to ioctl a wait token afterwards.
 * Setup and data contained in user arg structure */
int pex_ioctl_request_token(struct pex_privdata *priv, unsigned long arg);


/** Waits for a token to arrive previously requested by
 * an asynchronous ioctl request token.
 * Setup and data contained in user arg structure */
int pex_ioctl_wait_token(struct pex_privdata *priv, unsigned long arg);


/** Initiate reading token buffers from sfp front end hardware in parallel.
 *  After token data has been received on *PEX* board memory, DMA is performed to user specified physical host memory.
 *  Memory between SFP sections is padded with 0xaddXXXXX words for MBS readout unpackers.
 *  Function is synchronous, when it returns the data is ready in user buffer, i.e. MBS Pipe
 */
int pex_ioctl_request_receive_token_parallel(struct pex_privdata *priv, unsigned long arg);


/** initialize sfp fieldbus of frontends*/
int pex_ioctl_init_bus(struct pex_privdata* priv, unsigned long arg);


/** write to sfp fieldbus of frontends*/
int pex_ioctl_write_bus(struct pex_privdata* priv, unsigned long arg);

/** read from sfp fieldbus of frontends*/
int pex_ioctl_read_bus(struct pex_privdata* priv, unsigned long arg);

/** write list of configuration parameters to frontends*/
int pex_ioctl_configure_bus(struct pex_privdata* priv, unsigned long arg);

/** pass information about configured slaves to user*/
int pex_ioctl_get_sfp_links(struct pex_privdata* privdata, unsigned long arg);

/** configure gtx and mmcm depending on desired setup*/
int pex_ioctl_set_linkspeed(struct pex_privdata* privdata,  unsigned long arg);


/** write values as specified in data to frontend and optionally treat broadcast write
 * if sfp or slave numbers are negative*/
int pex_sfp_broadcast_write_bus(struct pex_privdata* priv, struct pex_bus_io* data);

/** write values as specified in data to frontends*/
int pex_sfp_write_bus(struct pex_privdata* priv, struct pex_bus_io* data);

/** read values as specified in data to frontends. results are in data structure*/
int pex_sfp_read_bus(struct pex_privdata* priv, struct pex_bus_io* data);


///////////////////////////////////////////////////////// new 2024:

/* read value from address via drp port*/
int pex_drp_read(struct pex_privdata* priv, unsigned short address, unsigned short* value);

/* write value via drp port*/
int pex_drp_write(struct pex_privdata* priv, unsigned short address, unsigned short value);


/** configure speed for given sfp channels. configure all channels if ch<0 (broadcast mode)*/
int pex_configure_linkspeed(struct pex_privdata* privdata, int ch, enum pex_linkspeed setup);


/** refresh information of linkspeed from drp registers into privdata cache*/
int pex_read_linkspeed_registers(struct pex_privdata* privdata);


/** apply linkspeed setup from privdata to the drp registers*/
int pex_write_linkspeed_registers(struct pex_privdata* privdata);



/** configure speed of GTX for given channel*/
int pex_gtx_configure(struct pex_privdata* priv, struct pex_gtx_set* conf);

/** configure MMCM for new channel speed*/
int pex_mmcm_configure(struct pex_privdata* priv, struct pex_mmcm_set* conf);

/** Define the default values for the possible gtx setup*/
void pex_gtx_init_defaults(void);

/** Define the default values for the possible mmcm setup*/
void pex_mmcm_init_defaults(void);


/* convert kinpex gtx register contents to appropriate paramter structure **/
void pex_gtx_register2pars(struct pex_gtx_set* conf, struct pex_gtx_params* pars);

/* convert kinpex gtx parameters to appropriate kinpex register contents **/
void pex_gtx_pars2register(struct pex_gtx_params* pars, struct pex_gtx_set* conf);

/* convert kinpex gtx register contents to appropriate paramter structure **/
void pex_mmcm_register2pars(struct pex_mmcm_set* conf, struct pex_mmcm_params* pars);

/* convert kinpex gtx parameters to appropriate kinpex register contents **/
void pex_mmcm_pars2register(struct pex_mmcm_params* pars, struct pex_mmcm_set* conf);


#ifdef PEX_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

ssize_t pex_sysfs_sfpregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);


ssize_t pex_sysfs_linkspeed_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);

ssize_t  pex_sysfs_linkspeed_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

#endif
#endif




#endif





