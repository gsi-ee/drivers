/*
 * pex_user.h
 *
 *  Created on: 01.12.2009, Updated for mbspex 08.04.2014
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











#define PEX_TRIX_RES          0   /* Command for ioctl set trixor to reset trigger - clear dt flag */
#define PEX_TRIX_GO           1   /* Command for ioctl set trixor to start acquisition  */
#define PEX_TRIX_HALT         2   /* Command for ioctl set trixor to stop  acquisition  */
#define PEX_TRIX_TIMESET         3   /* Command for ioctl set trixor to set trigger time windows*/

#define PEX_TRIGGER_FIRED  0/* return value from wait trigger to inform that trigger ir was fired reached */
#define PEX_TRIGGER_TIMEOUT 1 /* return value from wait trigger to inform that wait timeout was reached */

#define PEX_MAXCONFIG_VALS 60 /* number of configuration commands treated by driver in a single operation*/


struct pex_reg_io {
    unsigned int address;   /* address of a board register, relative to the specified BAR*/
    unsigned int value;     /* value for read/write at register address*/
    unsigned char bar;      /* the BAR where the register is mapped to PCI access. */
};

struct pex_dma_io {
    unsigned int source;     /* DMA source start address on the *pex* pciEx board*/
    unsigned int target;     /* physical DMA target start address in host memory*/
    unsigned int size;      /* size of bytes to transfer. returns real transfer size*/
    unsigned int burst;     /* burst lenght in bytes*/
};

struct pex_bus_io {
	int sfp;		/* sfp link id 0..3 (-1 for broadcast to all configured sfps)*/
	long slave;	    /* slave device id at the sfp (-1 ato broadcast to all slaves)*/
	unsigned long address;	/* address on the "field bus" connected to the optical links*/
	unsigned long value;	/* value for read/write at bus address. Contains result status after write*/
};


struct pex_bus_config{
  struct pex_bus_io param[PEX_MAXCONFIG_VALS]; /* array of configuration parameters*/
  unsigned int numpars; /* number of used parameters*/
};

struct pex_token_io {
	unsigned char sync;		/* 1:synchronous mode, 0: asynchronous mode*/
	unsigned char directdma;  /* 1: direct DMA to host on token receive, 0 explicit DMA from pex ram required*/
	unsigned long sfp;		/* sfp link id 0..3 */
	unsigned long bufid;	/* switch double buffer id on slave (1 or 0)*/
	unsigned long dmatarget; /* target address (physical) for DMA transfer*/
	unsigned long dmasize;   /* length of transferred data (bytes) after DMA*/
	unsigned long dmaburst;   /* burst size (bytes) for DMA*/
	unsigned long check_comm;   /* optional check returned command*/
	unsigned long check_token;  /* optional check returned token status bits*/
	unsigned long check_numslaves; /* optional check returned number of slaves*/
};



struct pex_trixor_set {
        unsigned int command;     /* command id to be issued for trixor*/
        unsigned int fct;     /* optional argument for trixor settings (fast clear time)*/
        unsigned int cvt;     /* optional argument for trixor settings (conversion time)*/
};


/* the ioctl stuff here:*/
#define PEX_IOC_MAGIC  0xE0

#define PEX_IOC_RESET             _IO(  PEX_IOC_MAGIC, 0)
#define PEX_IOC_TEST            _IOR(  PEX_IOC_MAGIC, 1, int)
#define PEX_IOC_WAIT_SEM          _IO(  PEX_IOC_MAGIC, 2)
#define PEX_IOC_POLL_SEM          _IOR(  PEX_IOC_MAGIC, 3, int)
#define PEX_IOC_RESET_SEM         _IOR(  PEX_IOC_MAGIC, 4, int)
#define PEX_IOC_GET_BAR0_BASE      _IOR(  PEX_IOC_MAGIC, 5, int)
#define PEX_IOC_GET_BAR0_TRIX_BASE      _IOR(  PEX_IOC_MAGIC, 6, int)
#define PEX_IOC_WRITE_BUS   _IOWR(  PEX_IOC_MAGIC, 7, struct pex_bus_io)
#define PEX_IOC_READ_BUS    _IOWR(  PEX_IOC_MAGIC, 8, struct pex_bus_io)
#define PEX_IOC_INIT_BUS    _IOW(  PEX_IOC_MAGIC, 9, struct pex_bus_io)
#define PEX_IOC_SET_TRIXOR    _IOR(  PEX_IOC_MAGIC, 10, struct pex_trixor_set)
#define PEX_IOC_REQUEST_TOKEN    _IOWR(  PEX_IOC_MAGIC, 11, struct pex_token_io)
#define PEX_IOC_WAIT_TOKEN    _IOWR(  PEX_IOC_MAGIC, 12, struct pex_token_io)
#define PEX_IOC_WAIT_TRIGGER    _IO(  PEX_IOC_MAGIC, 13)
#define PEX_IOC_WRITE_REGISTER   _IOW(  PEX_IOC_MAGIC, 14, struct pex_reg_io)
#define PEX_IOC_READ_REGISTER    _IOWR(  PEX_IOC_MAGIC, 15, struct pex_reg_io)
#define PEX_IOC_READ_DMA         _IOWR(  PEX_IOC_MAGIC, 16, struct pex_dma_io)
#define PEX_IOC_CONFIG_BUS       _IOWR(  PEX_IOC_MAGIC, 17, struct pex_bus_config)

/* we keep old ioctl definitions for backward compatibility and patch it in ioctl function*/
#define WAIT_SEM              12
#define POLL_SEM              16
#define GET_BAR0_BASE       0x1234
#define GET_BAR0_TRIX_BASE  0x1235
#define RESET_SEM           0x1236
#define PEX_IOC_MAXNR 23


/* note: we do not redefine ioctls existing in mbs user code!*/
//#define WAIT_SEM              PEX_IOC_WAIT_SEM
//#define POLL_SEM              PEX_IOC_POLL_SEM
//#define GET_BAR0_BASE       PEX_IOC_GET_BAR0_BASE
//#define GET_BAR0_TRIX_BASE  PEX_IOC_GET_BAR0_TRIX_BASE
//#define RESET_SEM           PEX_IOC_RESET_SEM


#endif /* PEX_USER_H_ */
