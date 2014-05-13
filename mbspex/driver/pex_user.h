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



/* the ioctl stuff here:*/
#define PEX_IOC_MAGIC  0xE0

#define PEX_IOC_RESET             _IO(  PEX_IOC_MAGIC, 0)
#define PEX_IOC_TEST  	        _IOR(  PEX_IOC_MAGIC, 1, int)
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

#define PEX_IOC_MAXNR 14


/* for mbs backward compatibility:*/
#define WAIT_SEM              PEX_IOC_WAIT_SEM
#define POLL_SEM              PEX_IOC_POLL_SEM
#define GET_BAR0_BASE       PEX_IOC_GET_BAR0_BASE
#define GET_BAR0_TRIX_BASE  PEX_IOC_GET_BAR0_TRIX_BASE
#define RESET_SEM           PEX_IOC_RESET_SEM


#define PEX_TRIX_RES          0   /* Command for ioctl set trixor to reset trigger - clear dt flag */
#define PEX_TRIX_GO           1   /* Command for ioctl set trixor to start acquisition  */
#define PEX_TRIX_HALT         2   /* Command for ioctl set trixor to stop  acquisition  */
#define PEX_TRIX_TIMESET         3   /* Command for ioctl set trixor to set trigger time windows*/

#define PEX_TRIGGER_FIRED  0/* return value from wait trigger to inform that trigger ir was fired reached */
#define PEX_TRIGGER_TIMEOUT 1 /* return value from wait trigger to inform that wait timeout was reached */

struct pex_bus_io {
	unsigned long sfp;		/* sfp link id 0..3 (optional)*/
	unsigned long slave;	/* slave device id at the sfp (optional)*/
	unsigned long address;	/* address on the "field bus" connected to the optical links*/
	unsigned long value;	/* value for read/write at bus address. Contains result status after write*/
};


struct pex_token_io {
	unsigned char sync;		/* 1:synchronous mode, 0: asynchronous mode*/
	unsigned long sfp;		/* sfp link id 0..3 */
	unsigned long bufid;	/* switch double buffer id on slave (1 or 0)*/
	unsigned long dmatarget; /* target address (physical) for dma transfer*/
	unsigned long dmasize;   /* length of transferred data (bytes) after DMA*/
	unsigned long check_comm;   /* optional check returned command*/
	unsigned long check_token;  /* optional check returned token status bits*/
	unsigned long check_numslaves; /* optional check returned number of slaves*/
};



struct pex_trixor_set {
        unsigned int command;     /* command id to be issued for trixor*/
        unsigned int fct;     /* optional argument for trixor settings (fast clear time)*/
        unsigned int cvt;     /* optional argument for trixor settings (conversion time)*/
};




#endif /* PEX_USER_H_ */
