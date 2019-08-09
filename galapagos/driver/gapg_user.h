/** \file
 * gapg_user.h
 *
 *  Created on: 05.08.2019, from mbspex example template
 *      Author: J. Adamczewski-Musch
 *
 *      Contains all common definitions for kernel driver and user space library
 */

#ifndef GAPG_USER_H_
#define GAPG_USER_H_

#include <linux/ioctl.h>


#define GAPGNAME       "galapagos"
#define GAPGNAMEFMT    "galapagos%d"

#define GAPG_VERSION     "0.10"

#define GAPG_AUTHORS     "JAM <j.adamczewski@gsi.de>"
#define GAPG_DESC        "GSI Arbitrary Logic And Pattern Generator System device driver"








//#define GAPG_TRIX_RES          0   /**< Command for ioctl set trixor to reset trigger - clear dt flag */
//#define GAPG_TRIX_GO           1   /**< Command for ioctl set trixor to start acquisition  */
//#define GAPG_TRIX_HALT         2   /**< Command for ioctl set trixor to stop  acquisition  */
//#define GAPG_TRIX_TIMESET         3   /**< Command for ioctl set trixor to set trigger time windows*/

//#define GAPG_TRIGGER_FIRED  0/**< return value from wait trigger to inform that trigger ir was fired reached */
//#define GAPG_TRIGGER_TIMEOUT 1 /**< return value from wait trigger to inform that wait timeout was reached */

//#define GAPG_SFP_NUMBER 4 /**< number of used sfp connections*/
#define GAPG_MAXCONFIG_VALS 60 /**< number of configuration commands treated by driver in a single operation*/

#define GAPG_REGISTERS_BAR 0 /**< base address region that contains the control register space */


struct gapg_reg_io {
    unsigned int address;   /**< address of a board register, relative to the specified BAR*/
    unsigned int value;     /**< value for read/write at register address*/
    unsigned char bar;      /**< the BAR where the register is mapped to PCI access. */
};

//struct gapg_pipebuf {
//    unsigned long addr; /**< user space virtual address of mbs pipe*/
//    unsigned long size; /**< allocated size or used size*/
//};
//
//
//
//struct gapg_dma_io {
//    unsigned int source;     /**< DMA source start address on the *gapg* pciEx board*/
//    unsigned int target;     /**< physical DMA target start address in host memory*/
//    unsigned long virtdest;   /**< virtual target start address in readout process. only for pipe type 4 with sg mapping*/
//    unsigned int size;      /**< size of bytes to transfer. returns real transfer size*/
//    unsigned int burst;     /**< burst lenght in bytes*/
//};
//
//struct gapg_bus_io {
//	int sfp;		/**< sfp link id 0..3 (-1 for broadcast to all configured sfps)*/
//	long slave;	    /**< slave device id at the sfp (-1 ato broadcast to all slaves)*/
//	unsigned long address;	/**< address on the "field bus" connected to the optical links*/
//	unsigned long value;	/**< value for read/write at bus address. Contains result status after write*/
//};
//
//
//struct gapg_bus_config{
//  struct gapg_bus_io param[GAPG_MAXCONFIG_VALS]; /**< array of configuration parameters*/
//  unsigned int numpars; /**< number of used parameters*/
//};
//
//
//struct gapg_sfp_links{
//    int numslaves[GAPG_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
//};
//
//struct gapg_token_io {
//	unsigned char sync;		/**< 1:synchronous mode, 0: asynchronous mode*/
//	unsigned char directdma;  /**< 1: direct DMA to host on token receive, 0 explicit DMA from gapg ram required*/
//	unsigned long sfp;		/**< sfp link id 0..3 */
//	unsigned long bufid;	/**< switch double buffer id on slave (1 or 0)*/
//	unsigned long dmatarget; /**< target address (physical) for DMA transfer*/
//	unsigned long dmasize;   /**< length of transferred data (bytes) after DMA*/
//	unsigned long dmaburst;   /**< burst size (bytes) for DMA*/
//	unsigned long check_comm;   /**< optional check returned command*/
//	unsigned long check_token;  /**< optional check returned token status bits*/
//	unsigned long check_numslaves; /**< optional check returned number of slaves*/
//};
//
//
//
//struct gapg_trixor_set {
//        unsigned int command;     /**< command id to be issued for trixor*/
//        unsigned int fct;     /**< optional argument for trixor settings (fast clear time)*/
//        unsigned int cvt;     /**< optional argument for trixor settings (conversion time)*/
//};


/** the ioctl stuff here:*/
#define GAPG_IOC_MAGIC  0xE0



#define GAPG_IOC_RESET             _IO(  GAPG_IOC_MAGIC, 0)           /**<Reset gapgor/kingapg board (gosip engine). This will also clear registered number of configured slaves for each sfp.*/
#define GAPG_IOC_TEST            _IOR(  GAPG_IOC_MAGIC, 1, int)       /**< do not use, internal test mode*/
#define GAPG_IOC_WRITE_REGISTER   _IOW(  GAPG_IOC_MAGIC, 2, struct gapg_reg_io)       /**< Write value to register on gapgor/kingapg. Data exchange via =struct gapg_reg_io=*/
#define GAPG_IOC_READ_REGISTER    _IOWR(  GAPG_IOC_MAGIC, 3, struct gapg_reg_io)      /**< Read value from register on gapgor/kingapg. Data exchange via =struct gapg_reg_io=*/

#define GAPG_IOC_MAXNR 3

//#define GAPG_IOC_WAIT_SEM          _IO(  GAPG_IOC_MAGIC, 2)           /**<  wait for trigger semaphore. same as =WAIT_SEM=*/
//#define GAPG_IOC_POLL_SEM          _IOR(  GAPG_IOC_MAGIC, 3, int)     /**< poll trigger state. same as =POLL_SEM=*/
//#define GAPG_IOC_RESET_SEM         _IOR(  GAPG_IOC_MAGIC, 4, int)     /**< reset trigger semaphore. Same as =RESET_SEM=*/
//#define GAPG_IOC_GET_BAR0_BASE      _IOR(  GAPG_IOC_MAGIC, 5, int)    /**< deliver physical address of gapgor bar 0. Same as =GET_BAR0_BASE=*/
//#define GAPG_IOC_GET_BAR0_TRIX_BASE      _IOR(  GAPG_IOC_MAGIC, 6, int)       /**< deliver physical address   of trixor registers base. Same as   =GET_BAR0_TRIX_BASE=*/
//#define GAPG_IOC_WRITE_BUS   _IOWR(  GAPG_IOC_MAGIC, 7, struct gapg_bus_io)    /**< write on slave via gosip. Data is exchanged via  =struct gapg_bus_io=. Is capable of slave broadcast if sfp or slave id is -1.*/
//#define GAPG_IOC_READ_BUS    _IOWR(  GAPG_IOC_MAGIC, 8, struct gapg_bus_io)    /**< read  from slave via gosip. Data is exchanged via  =struct gapg_bus_io=*/
//#define GAPG_IOC_INIT_BUS    _IOW(  GAPG_IOC_MAGIC, 9, struct gapg_bus_io)     /**< Initialize sfp chain with selected number of slaves. Data exchanged via =struct gapg_bus_io=*/
//#define GAPG_IOC_SET_TRIXOR    _IOR(  GAPG_IOC_MAGIC, 10, struct gapg_trixor_set)      /**< Send commands and values to trixor (also start/stop acquisition). Data exchange via  =struct gapg_trixor_set=*/
//#define GAPG_IOC_REQUEST_TOKEN    _IOWR(  GAPG_IOC_MAGIC, 11, struct gapg_token_io)    /**<  Request data from slaves via token and optionally directly initiate DMA to destination address in PC memory. This is controlled via =struct gapg_token_io=*/
//#define GAPG_IOC_WAIT_TOKEN    _IOWR(  GAPG_IOC_MAGIC, 12, struct gapg_token_io)       /**< For non-synchronos token request without direct DMA -  wait until token transfer on chain has been finished. Controlled via =struct gapg_token_io=*/
//#define GAPG_IOC_WAIT_TRIGGER    _IO(  GAPG_IOC_MAGIC, 13)                            /**< Wait until next trigger interrupt arrives. Usually the same as =WAIT_SEM=, but other mode of operation may be tested later.*/
//#define GAPG_IOC_WRITE_REGISTER   _IOW(  GAPG_IOC_MAGIC, 14, struct gapg_reg_io)       /**< Write value to register on gapgor/kingapg. Data exchange via =struct gapg_reg_io=*/
//#define GAPG_IOC_READ_REGISTER    _IOWR(  GAPG_IOC_MAGIC, 15, struct gapg_reg_io)      /**< Read value from register on gapgor/kingapg. Data exchange via =struct gapg_reg_io=*/
//#define GAPG_IOC_READ_DMA         _IOWR(  GAPG_IOC_MAGIC, 16, struct gapg_dma_io)      /**< directly initiate DMA transfer from gapgor/kingapg board memory to host destination address. Controlled by =struct gapg_dma_io=. Required in MBS for backward compatibility in parallel readout mode. since burstsize is computed in user readout function from actual token payload before DMA is started.*/
//#define GAPG_IOC_CONFIG_BUS       _IOWR(  GAPG_IOC_MAGIC, 17, struct gapg_bus_config)  /**< Send package of configuration values to one or several slaves at once via gosip. Defined by array of  =struct gapg_bus_io= that is contained in =struct gapg_bus_config=. Is capable of slave broadcast if sfp or slave id is -1.*/
//#define GAPG_IOC_GET_SFP_LINKS    _IOR(  GAPG_IOC_MAGIC, 18, struct gapg_sfp_links)    /**< Retrieve actual number of configured slaves for each sfp chain, as it was done by  =GAPG_IOC_INIT_BUS=. This is delivered in =struct gapg_sfp_links=. Required from =gosipcmd= to perform a safe _broadcast read_ of register values.*/
//#define GAPG_IOC_MAP_PIPE         _IOW(  GAPG_IOC_MAGIC, 19, struct gapg_pipebuf)          /**< map virtual mbs pipe to driver sg list*/
//#define GAPG_IOC_UNMAP_PIPE       _IO(  GAPG_IOC_MAGIC, 20)        /**< unmap virtual mbs pipe and clear driver sg list*/
//#define GAPG_IOC_READ_DMA_PIPE    _IOWR(  GAPG_IOC_MAGIC, 21, struct gapg_dma_io)      /**< directly initiate DMA transfer from gapgor/kingapg board memory to _virtual_ destination address in pipe. Pipe must be mapped before into internal sg list by GAPG_IOC_MAP_PIPE*/
//#define GAPG_IOC_REQUEST_RECEIVE_TOKENS    _IOWR(  GAPG_IOC_MAGIC, 22, struct gapg_token_io)    /**<  Request data from parallel slaves via token and initiate DMA to destination address in PC memory. MBS padding words are provided between slave DMA data sections.*/
//


/** we keep old ioctl definitions for backward compatibility and patch it in ioctl function*/
//#define WAIT_SEM              12
//#define POLL_SEM              16
//#define GET_BAR0_BASE       0x1234
//#define GET_BAR0_TRIX_BASE  0x1235
//#define RESET_SEM           0x1236
//#define GAPG_IOC_MAXNR 28


/* note: we do not redefine ioctls existing in mbs user code!*/
//#define WAIT_SEM              GAPG_IOC_WAIT_SEM
//#define POLL_SEM              GAPG_IOC_POLL_SEM
//#define GET_BAR0_BASE       GAPG_IOC_GET_BAR0_BASE
//#define GET_BAR0_TRIX_BASE  GAPG_IOC_GET_BAR0_TRIX_BASE
//#define RESET_SEM           GAPG_IOC_RESET_SEM


#endif /* GAPG_USER_H_ */
