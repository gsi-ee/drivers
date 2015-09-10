/** \file
 * pex_user.h
 *
 *  Created on: 01.12.2009, Updated for mbspex 08.04.2014
 *                          Added sg pipe mode 13.03.2015
 *                          Added parallel token dma ioctl 10.09.2015
 *      Author: J. Adamczewski-Musch
 *
 *      Contains all common definitions for kernel driver and user space library
 */

#ifndef PEX_USER_H_
#define PEX_USER_H_

#include <linux/ioctl.h>


#define PEXNAME       "mbspex"
#define PEXORNAMEFMT    "pexor%d"
#define PEXARIANAMEFMT  "pexaria%d"
#define KINPEXNAMEFMT   "kinpex%d"











#define PEX_TRIX_RES          0   /**< Command for ioctl set trixor to reset trigger - clear dt flag */
#define PEX_TRIX_GO           1   /**< Command for ioctl set trixor to start acquisition  */
#define PEX_TRIX_HALT         2   /**< Command for ioctl set trixor to stop  acquisition  */
#define PEX_TRIX_TIMESET         3   /**< Command for ioctl set trixor to set trigger time windows*/

#define PEX_TRIGGER_FIRED  0/**< return value from wait trigger to inform that trigger ir was fired reached */
#define PEX_TRIGGER_TIMEOUT 1 /**< return value from wait trigger to inform that wait timeout was reached */

#define PEX_SFP_NUMBER 4 /**< number of used sfp connections*/
#define PEX_MAXCONFIG_VALS 60 /**< number of configuration commands treated by driver in a single operation*/


struct pex_reg_io {
    unsigned int address;   /**< address of a board register, relative to the specified BAR*/
    unsigned int value;     /**< value for read/write at register address*/
    unsigned char bar;      /**< the BAR where the register is mapped to PCI access. */
};

struct pex_pipebuf {
    unsigned long addr; /**< user space virtual address of mbs pipe*/
    unsigned long size; /**< allocated size or used size*/
};



struct pex_dma_io {
    unsigned int source;     /**< DMA source start address on the *pex* pciEx board*/
    unsigned int target;     /**< physical DMA target start address in host memory*/
    unsigned long virtdest;   /**< virtual target start address in readout process. only for pipe type 4 with sg mapping*/
    unsigned int size;      /**< size of bytes to transfer. returns real transfer size*/
    unsigned int burst;     /**< burst lenght in bytes*/
};

struct pex_bus_io {
	int sfp;		/**< sfp link id 0..3 (-1 for broadcast to all configured sfps)*/
	long slave;	    /**< slave device id at the sfp (-1 ato broadcast to all slaves)*/
	unsigned long address;	/**< address on the "field bus" connected to the optical links*/
	unsigned long value;	/**< value for read/write at bus address. Contains result status after write*/
};


struct pex_bus_config{
  struct pex_bus_io param[PEX_MAXCONFIG_VALS]; /**< array of configuration parameters*/
  unsigned int numpars; /**< number of used parameters*/
};


struct pex_sfp_links{
    int numslaves[PEX_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
};

struct pex_token_io {
	unsigned char sync;		/**< 1:synchronous mode, 0: asynchronous mode*/
	unsigned char directdma;  /**< 1: direct DMA to host on token receive, 0 explicit DMA from pex ram required*/
	unsigned long sfp;		/**< sfp link id 0..3 */
	unsigned long bufid;	/**< switch double buffer id on slave (1 or 0)*/
	unsigned long dmatarget; /**< target address (physical) for DMA transfer*/
	unsigned long dmasize;   /**< length of transferred data (bytes) after DMA*/
	unsigned long dmaburst;   /**< burst size (bytes) for DMA*/
	unsigned long check_comm;   /**< optional check returned command*/
	unsigned long check_token;  /**< optional check returned token status bits*/
	unsigned long check_numslaves; /**< optional check returned number of slaves*/
};



struct pex_trixor_set {
        unsigned int command;     /**< command id to be issued for trixor*/
        unsigned int fct;     /**< optional argument for trixor settings (fast clear time)*/
        unsigned int cvt;     /**< optional argument for trixor settings (conversion time)*/
};


/** the ioctl stuff here:*/
#define PEX_IOC_MAGIC  0xE0



#define PEX_IOC_RESET             _IO(  PEX_IOC_MAGIC, 0)           /**<Reset pexor/kinpex board (gosip engine). This will also clear registered number of configured slaves for each sfp.*/
#define PEX_IOC_TEST            _IOR(  PEX_IOC_MAGIC, 1, int)       /**< do not use, internal test mode*/
#define PEX_IOC_WAIT_SEM          _IO(  PEX_IOC_MAGIC, 2)           /**<  wait for trigger semaphore. same as =WAIT_SEM=*/
#define PEX_IOC_POLL_SEM          _IOR(  PEX_IOC_MAGIC, 3, int)     /**< poll trigger state. same as =POLL_SEM=*/
#define PEX_IOC_RESET_SEM         _IOR(  PEX_IOC_MAGIC, 4, int)     /**< reset trigger semaphore. Same as =RESET_SEM=*/
#define PEX_IOC_GET_BAR0_BASE      _IOR(  PEX_IOC_MAGIC, 5, int)    /**< deliver physical address of pexor bar 0. Same as =GET_BAR0_BASE=*/
#define PEX_IOC_GET_BAR0_TRIX_BASE      _IOR(  PEX_IOC_MAGIC, 6, int)       /**< deliver physical address   of trixor registers base. Same as   =GET_BAR0_TRIX_BASE=*/
#define PEX_IOC_WRITE_BUS   _IOWR(  PEX_IOC_MAGIC, 7, struct pex_bus_io)    /**< write on slave via gosip. Data is exchanged via  =struct pex_bus_io=. Is capable of slave broadcast if sfp or slave id is -1.*/
#define PEX_IOC_READ_BUS    _IOWR(  PEX_IOC_MAGIC, 8, struct pex_bus_io)    /**< read  from slave via gosip. Data is exchanged via  =struct pex_bus_io=*/
#define PEX_IOC_INIT_BUS    _IOW(  PEX_IOC_MAGIC, 9, struct pex_bus_io)     /**< Initialize sfp chain with selected number of slaves. Data exchanged via =struct pex_bus_io=*/
#define PEX_IOC_SET_TRIXOR    _IOR(  PEX_IOC_MAGIC, 10, struct pex_trixor_set)      /**< Send commands and values to trixor (also start/stop acquisition). Data exchange via  =struct pex_trixor_set=*/
#define PEX_IOC_REQUEST_TOKEN    _IOWR(  PEX_IOC_MAGIC, 11, struct pex_token_io)    /**<  Request data from slaves via token and optionally directly initiate DMA to destination address in PC memory. This is controlled via =struct pex_token_io=*/
#define PEX_IOC_WAIT_TOKEN    _IOWR(  PEX_IOC_MAGIC, 12, struct pex_token_io)       /**< For non-synchronos token request without direct DMA -  wait until token transfer on chain has been finished. Controlled via =struct pex_token_io=*/
#define PEX_IOC_WAIT_TRIGGER    _IO(  PEX_IOC_MAGIC, 13)                            /**< Wait until next trigger interrupt arrives. Usually the same as =WAIT_SEM=, but other mode of operation may be tested later.*/
#define PEX_IOC_WRITE_REGISTER   _IOW(  PEX_IOC_MAGIC, 14, struct pex_reg_io)       /**< Write value to register on pexor/kinpex. Data exchange via =struct pex_reg_io=*/
#define PEX_IOC_READ_REGISTER    _IOWR(  PEX_IOC_MAGIC, 15, struct pex_reg_io)      /**< Read value from register on pexor/kinpex. Data exchange via =struct pex_reg_io=*/
#define PEX_IOC_READ_DMA         _IOWR(  PEX_IOC_MAGIC, 16, struct pex_dma_io)      /**< directly initiate DMA transfer from pexor/kinpex board memory to host destination address. Controlled by =struct pex_dma_io=. Required in MBS for backward compatibility in parallel readout mode. since burstsize is computed in user readout function from actual token payload before DMA is started.*/
#define PEX_IOC_CONFIG_BUS       _IOWR(  PEX_IOC_MAGIC, 17, struct pex_bus_config)  /**< Send package of configuration values to one or several slaves at once via gosip. Defined by array of  =struct pex_bus_io= that is contained in =struct pex_bus_config=. Is capable of slave broadcast if sfp or slave id is -1.*/
#define PEX_IOC_GET_SFP_LINKS    _IOR(  PEX_IOC_MAGIC, 18, struct pex_sfp_links)    /**< Retrieve actual number of configured slaves for each sfp chain, as it was done by  =PEX_IOC_INIT_BUS=. This is delivered in =struct pex_sfp_links=. Required from =gosipcmd= to perform a safe _broadcast read_ of register values.*/
#define PEX_IOC_MAP_PIPE         _IOW(  PEX_IOC_MAGIC, 19, struct pex_pipebuf)          /**< map virtual mbs pipe to driver sg list*/
#define PEX_IOC_UNMAP_PIPE       _IO(  PEX_IOC_MAGIC, 20)        /**< unmap virtual mbs pipe and clear driver sg list*/
#define PEX_IOC_READ_DMA_PIPE    _IOWR(  PEX_IOC_MAGIC, 21, struct pex_dma_io)      /**< directly initiate DMA transfer from pexor/kinpex board memory to _virtual_ destination address in pipe. Pipe must be mapped before into internal sg list by PEX_IOC_MAP_PIPE*/
#define PEX_IOC_REQUEST_RECEIVE_TOKENS    _IOWR(  PEX_IOC_MAGIC, 22, struct pex_token_io)    /**<  Request data from parallel slaves via token and initiate DMA to destination address in PC memory. MBS padding words are provided between slave DMA data sections.*/



/** we keep old ioctl definitions for backward compatibility and patch it in ioctl function*/
#define WAIT_SEM              12
#define POLL_SEM              16
#define GET_BAR0_BASE       0x1234
#define GET_BAR0_TRIX_BASE  0x1235
#define RESET_SEM           0x1236
#define PEX_IOC_MAXNR 28


/* note: we do not redefine ioctls existing in mbs user code!*/
//#define WAIT_SEM              PEX_IOC_WAIT_SEM
//#define POLL_SEM              PEX_IOC_POLL_SEM
//#define GET_BAR0_BASE       PEX_IOC_GET_BAR0_BASE
//#define GET_BAR0_TRIX_BASE  PEX_IOC_GET_BAR0_TRIX_BASE
//#define RESET_SEM           PEX_IOC_RESET_SEM


#endif /* PEX_USER_H_ */
