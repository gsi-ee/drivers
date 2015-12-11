/*
 * \file
 * pexornet_user.h
 *
 *  Created on: 04.11.2015 -
 *   *      Author: J. Adamczewski-Musch
 *
 *      Contains all common definitions for pexornet kernel driver and user space library
 */

#ifndef PEXORNET_USER_H_
#define PEXORNET_USER_H_

#include <linux/ioctl.h>
#include <linux/sockios.h>


#define PEXORNETVERSION  "0.1"

/** identify name in dev : */
#define PEXORNETNAME 	 "pexornet"
#define PEXORIFNAMEFMT   "pex%d"

#define PEXORNAMEFMT 	"pexor%d"
#define PEXARIANAMEFMT  "pexaria%d"
#define KINPEXNAMEFMT   "kinpex%d"






/** the states:*/
#define PEXORNET_STATE_STOPPED 0 /**< daq stopped*/
#define PEXORNET_STATE_DMA_SINGLE 1 /**< trigger a single dma transfer into next free buf*/
#define PEXORNET_STATE_DMA_FLOW 2   /**< each dma transfer will initiate the next = dataflow or chained DMA*/
#define PEXORNET_STATE_DMA_AUTO 3   /**< TODO: board fpga will automatically initate next DMA when send fifo is full*/
#define PEXORNET_STATE_DMA_SUSPENDED 4   /**< used for backpressure until free buffer list is empty no more */
#define PEXORNET_STATE_IR_TEST 5 /**< this state is used to test raising an ir manually*/
#define PEXORNET_STATE_TRIGGERED_READ 6 /**< trigger interrupt will fill dma buffer automatically with token data*/

#define PEXORNET_TRIGGER_FIRED  0/**< return value from wait trigger to inform that trigger ir was fired reached */
#define PEXORNET_TRIGGER_TIMEOUT 1 /**< return value from wait trigger to inform that wait timeout was reached */



#define PEXORNET_TRIGTYPE_NONE 0 /**< triggerless readout */
#define PEXORNET_TRIGTYPE_DATA 1 /**< regular data trigger */
#define PEXORNET_TRIGTYPE_START 14 /**< start acquisition trigger type like in mbs */
#define PEXORNET_TRIGTYPE_STOP  15 /**< stop acquisition trigger type like in mbs */



#define PEXORNET_TRIX_RES          0   /**< Command for ioctl set trixor to reset trigger - clear dt flag */
#define PEXORNET_TRIX_GO           1   /**< Command for ioctl set trixor to start acquisition  */
#define PEXORNET_TRIX_HALT         2   /**< Command for ioctl set trixor to stop  acquisition  */
#define PEXORNET_TRIX_TIMESET         3   /**< Command for ioctl set trixor to set trigger time windows*/

#define PEXORNET_SFP_NUMBER 4 /**< number of used sfp connections*/
#define PEXORNET_MAXCONFIG_VALS 60 /**< number of configuration commands treated by driver in a single operation*/





struct pexornet_userbuf {
	unsigned long addr;	/**< user space virtual address (=buffer id)*/
	unsigned long size;	/**< allocated size or used size*/
};


struct pexornet_bus_io {
	int sfp;		/**< sfp link id 0..3 (-1 for broadcast to all configured sfps)*/
	long slave;	    /**< slave device id at the sfp (-1 ato broadcast to all slaves)*/
	unsigned long address;	/**< address on the "field bus" connected to the optical links*/
	unsigned long value;	/**< value for read/write at bus address. Contains result status after write*/
};


struct pexornet_bus_config{
  struct pexornet_bus_io param[PEXORNET_MAXCONFIG_VALS]; /**< array of configuration parameters*/
  unsigned int numpars; /**< number of used parameters*/
};





struct pexornet_sfp_links{
    int numslaves[PEXORNET_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
};





struct pexornet_token_io {
	unsigned char sync;		/**< 1:synchronous mode, 0: asynchronous mode*/
    unsigned char directdma;  /**< 1: direct DMA to host on token receive, 0 explicit DMA from pex ram required*/
	unsigned long sfp;		/**< sfp link id 0..3 */
	unsigned long bufid;	/**< switch double buffer id on slave (1 or 0)*/
	struct pexornet_userbuf tkbuf; /**< dma buffer with received token data (on synchronous reply);
								    on token request, may specify id of dma buffer to fill*/
	unsigned long offset;		/**< offset of token payload relative to tkbuf start. To skip user header for zero-copy mode*/
};


//struct pexornet_reg_io {
//	unsigned int address;	/**< address of a board register, relative to the specified BAR*/
//	unsigned int value;		/**< value for read/write at register address*/
//	unsigned char bar;		/**< the BAR where the register is mapped to PCI access. */
//};

struct pexornet_trixor_set {
        unsigned int command;     /**< command id to be issued for trixor*/
        unsigned int fct;     /**< optional argument for trixor settings (fast clear time)*/
        unsigned int cvt;     /**< optional argument for trixor settings (conversion time)*/
};


/** contains decoded information of trixor status register
 * at corresponding trigger interrupt.
 * These are compatible with mbs definitions */
struct pexornet_trigger_status {
        unsigned char typ;      /**<  trigger type */
        unsigned char lec;      /**<  local event counter */
        unsigned char si;       /**< sub event invalid  */
        unsigned char mis;      /**< trigger mismatch condition */
        unsigned char di;       /**< delay interrupt line  */
        unsigned char tdt;      /**< total dead time on/off */
        unsigned char eon;      /**< data ready for readout*/
};


/** Data releavant for automatic trigger readout.
 * contains token data  and corresponding trigger status */
struct pexornet_trigger_readout{
  struct pexornet_trigger_status triggerstatus;
  struct pexornet_userbuf data;
};




#define PEXORNET_IOC_RESET          SIOCDEVPRIVATE+0
#define PEXORNET_IOC_INIT_BUS       SIOCDEVPRIVATE+1
#define PEXORNET_IOC_WRITE_BUS      SIOCDEVPRIVATE+2
#define PEXORNET_IOC_READ_BUS       SIOCDEVPRIVATE+3
#define PEXORNET_IOC_CONFIG_BUS     SIOCDEVPRIVATE+4
#define PEXORNET_IOC_SET_TRIXOR     SIOCDEVPRIVATE+5
#define PEXORNET_IOC_GET_SFP_LINKS  SIOCDEVPRIVATE+6

///* the ioctl stuff here:*/
//#define PEXORNET_IOC_MAGIC  0xE0
//
//#define PEXORNET_IOC_RESET  _IO(  PEXORNET_IOC_MAGIC, 0)
////#define PEXORNET_IOC_FREEBUFFER  _IOW(  PEXORNET_IOC_MAGIC, 1, struct pexornet_userbuf)
////#define PEXORNET_IOC_DELBUFFER   _IOW(  PEXORNET_IOC_MAGIC, 2, struct pexornet_userbuf)
////#define PEXORNET_IOC_WAITBUFFER  _IOR(  PEXORNET_IOC_MAGIC, 3, struct pexornet_trigger_readout)
////#define PEXORNET_IOC_USEBUFFER   _IOR(  PEXORNET_IOC_MAGIC, 4, struct pexornet_userbuf)
////#define PEXORNET_IOC_SETSTATE    _IOR(  PEXORNET_IOC_MAGIC, 5, int)
////#define PEXORNET_IOC_TEST        _IOR(  PEXORNET_IOC_MAGIC, 6, int)
//#define PEXORNET_IOC_CLEAR_RCV_BUFFERS  _IO(  PEXORNET_IOC_MAGIC, 7)
//#define PEXORNET_IOC_WRITE_BUS   _IOWR(  PEXORNET_IOC_MAGIC, 8, struct pexornet_bus_io)
//#define PEXORNET_IOC_READ_BUS    _IOWR(  PEXORNET_IOC_MAGIC, 9, struct pexornet_bus_io)
//#define PEXORNET_IOC_INIT_BUS    _IOW(  PEXORNET_IOC_MAGIC, 10, struct pexornet_bus_io)
////#define PEXORNET_IOC_WRITE_REGISTER   _IOW(  PEXORNET_IOC_MAGIC, 11, struct pexornet_reg_io)
////#define PEXORNET_IOC_READ_REGISTER    _IOWR(  PEXORNET_IOC_MAGIC, 12, struct pexornet_reg_io)
////#define PEXORNET_IOC_REQUEST_TOKEN    _IOWR(  PEXORNET_IOC_MAGIC, 13, struct pexornet_token_io)
////#define PEXORNET_IOC_WAIT_TOKEN    _IOWR(  PEXORNET_IOC_MAGIC, 14, struct pexornet_token_io)
////#define PEXORNET_IOC_WAIT_TRIGGER    _IOR(  PEXORNET_IOC_MAGIC, 15, struct pexornet_trigger_status)
//#define PEXORNET_IOC_SET_TRIXOR    _IOW(  PEXORNET_IOC_MAGIC, 16, struct pexornet_trixor_set)
////#define PEXORNET_IOC_MAPBUFFER   _IOW(  PEXORNET_IOC_MAGIC, 17, struct pexornet_userbuf)
////#define PEXORNET_IOC_UNMAPBUFFER   _IOW(  PEXORNET_IOC_MAGIC, 18, struct pexornet_userbuf)
//#define PEXORNET_IOC_CONFIG_BUS       _IOWR(  PEXORNET_IOC_MAGIC, 19, struct pexornet_bus_config)
//#define PEXORNET_IOC_GET_SFP_LINKS    _IOR(  PEXORNET_IOC_MAGIC, 20, struct pexornet_sfp_links)
//#define PEXORNET_IOC_SET_WAIT_TIMEOUT    _IOW(  PEXORNET_IOC_MAGIC, 21, int)
//
//
//#define PEXORNET_IOC_MAXNR 22


/** some alias ioctl definitions for goiscmd/mbspex lib:*/
#define PEX_IOC_RESET PEXORNET_IOC_RESET
#define PEX_IOC_WRITE_BUS PEXORNET_IOC_WRITE_BUS
#define PEX_IOC_READ_BUS PEXORNET_IOC_READ_BUS
#define PEX_IOC_INIT_BUS PEXORNET_IOC_INIT_BUS
#define PEX_IOC_CONFIG_BUS PEXORNET_IOC_CONFIG_BUS
#define PEX_IOC_GET_SFP_LINKS PEXORNET_IOC_GET_SFP_LINKS

/**alias structure names for mbspex lib:*/
#define pex_bus_io pexornet_bus_io
#define pex_bus_config pexornet_bus_config
#define pex_sfp_links pexornet_sfp_links

/** other mbspex aliases*/
#define PEX_MAXCONFIG_VALS PEXORNET_MAXCONFIG_VALS
#define PEX_SFP_NUMBER PEXORNET_SFP_NUMBER


#endif /* PEXORNET_USER_H_ */
