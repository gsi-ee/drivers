/*
 * pexor_user.h
 *
 *  Created on: 01.12.2009
 *      Author: J. Adamczewski-Musch
 *
 *      Contains all common definitions for kernel driver and user space library
 */

#ifndef PEXOR_USER_H_
#define PEXOR_USER_H_

#include <linux/ioctl.h>


#define PEXORVERSION  "0.99"

/* identify name in dev : */
#define PEXORNAME 		"pexor"
#define PEXORNAMEFMT 	"pexor-%d"



/* the ioctl stuff here:*/
#define PEXOR_IOC_MAGIC  0xE0

#define PEXOR_IOC_RESET  _IO(  PEXOR_IOC_MAGIC, 0)
#define PEXOR_IOC_FREEBUFFER  _IOW(  PEXOR_IOC_MAGIC, 1, struct pexor_userbuf)
#define PEXOR_IOC_DELBUFFER   _IOW(  PEXOR_IOC_MAGIC, 2, struct pexor_userbuf)
#define PEXOR_IOC_WAITBUFFER  _IOR(  PEXOR_IOC_MAGIC, 3, struct pexor_userbuf)
#define PEXOR_IOC_USEBUFFER   _IOR(  PEXOR_IOC_MAGIC, 4, struct pexor_userbuf)
#define PEXOR_IOC_SETSTATE    _IOR(  PEXOR_IOC_MAGIC, 5, int)
#define PEXOR_IOC_TEST  	  _IOR(  PEXOR_IOC_MAGIC, 6, int)
#define PEXOR_IOC_CLEAR_RCV_BUFFERS  _IO(  PEXOR_IOC_MAGIC, 7)
#define PEXOR_IOC_WRITE_BUS   _IOWR(  PEXOR_IOC_MAGIC, 8, struct pexor_bus_io)
#define PEXOR_IOC_READ_BUS    _IOWR(  PEXOR_IOC_MAGIC, 9, struct pexor_bus_io)
#define PEXOR_IOC_INIT_BUS    _IOW(  PEXOR_IOC_MAGIC, 10, struct pexor_bus_io)
#define PEXOR_IOC_WRITE_REGISTER   _IOW(  PEXOR_IOC_MAGIC, 11, struct pexor_reg_io)
#define PEXOR_IOC_READ_REGISTER    _IOWR(  PEXOR_IOC_MAGIC, 12, struct pexor_reg_io)
#define PEXOR_IOC_REQUEST_TOKEN    _IOWR(  PEXOR_IOC_MAGIC, 13, struct pexor_token_io)
#define PEXOR_IOC_WAIT_TOKEN    _IOWR(  PEXOR_IOC_MAGIC, 14, struct pexor_token_io)
#define PEXOR_IOC_WAIT_TRIGGER    _IO(  PEXOR_IOC_MAGIC, 15)
#define PEXOR_IOC_SET_TRIXOR    _IOR(  PEXOR_IOC_MAGIC, 16, struct pexor_trixor_set)
#define PEXOR_IOC_TRBNET_REQUEST _IOWR(  PEXOR_IOC_MAGIC, 17, struct pexor_trbnet_io)
#define PEXOR_IOC_MAXNR 18

/* the states:*/
#define PEXOR_STATE_STOPPED 0 /* daq stopped*/
#define PEXOR_STATE_DMA_SINGLE 1 /* trigger a single dma transfer into next free buf*/
#define PEXOR_STATE_DMA_FLOW 2   /* each dma transfer will initiate the next = dataflow or chained DMA*/
#define PEXOR_STATE_DMA_AUTO 3   /* TODO: board fpga will automatically initate next DMA when send fifo is full*/
#define PEXOR_STATE_DMA_SUSPENDED 4   /* used for backpressure until free buffer list is empty no more */
#define PEXOR_STATE_IR_TEST 5 /* this state is used to test raising an ir manually*/

#define PEXOR_TRIGGER_FIRED  0/* return value from wait trigger to inform that trigger ir was fired reached */
#define PEXOR_TRIGGER_TIMEOUT 1 /* return value from wait trigger to inform that wait timeout was reached */

#define PEXOR_TRIX_RES          0   /* Command for ioctl set trixor to reset trigger - clear dt flag */
#define PEXOR_TRIX_GO           1   /* Command for ioctl set trixor to start acquisition  */
#define PEXOR_TRIX_HALT         2   /* Command for ioctl set trixor to stop  acquisition  */
#define PEXOR_TRIX_TIMESET         3   /* Command for ioctl set trixor to set trigger time windows*/


#define PEXOR_TRBNETCOM_REG_WRITE             0   /* Command for ioctl trbnet request */
#define PEXOR_TRBNETCOM_REG_WRITE_MEM          1   /* Command for ioctl trbnet request */
#define PEXOR_TRBNETCOM_REG_READ          2   /* Command for ioctl trbnet request */
#define PEXOR_TRBNETCOM_REG_READ_MEM          3   /* Command for ioctl trbnet request */


struct pexor_userbuf {
	unsigned long addr;	/* user space virtual address (=buffer id)*/
	unsigned long size;	/* allocated size or used size*/
};


struct pexor_bus_io {
	unsigned long sfp;		/* sfp link id 0..3 (optional)*/
	unsigned long slave;	/* slave device id at the sfp (optional)*/
	unsigned long address;	/* address on the "field bus" connected to the optical links*/
	unsigned long value;	/* value for read/write at bus address. Contains result status after write*/
};

struct pexor_token_io {
	unsigned char sync;		/* 1:synchronous mode, 0: asynchronous mode*/
	unsigned long sfp;		/* sfp link id 0..3 */
	unsigned long bufid;	/* switch double buffer id on slave (1 or 0)*/
	struct pexor_userbuf tkbuf; /* dma buffer with received token data (on synchronous reply)*/
};


struct pexor_reg_io {
	unsigned int address;	/* address of a board register, relative to the specified BAR*/
	unsigned int value;		/* value for read/write at register address*/
	unsigned char bar;		/* the BAR where the register is mapped to PCI access. */
};

struct pexor_trixor_set {
        unsigned int command;     /* command id to be issued for trixor*/
        unsigned int fct;     /* optional argument for trixor settings (fast clear time)*/
        unsigned int cvt;     /* optional argument for trixor settings (conversion time)*/
};

#define TRBNET_MAX_BUFS 64

struct pexor_trbnet_io {
  unsigned int command;     /* command to issue on trbnet*/
  unsigned short trb_address;     /* address of board in trbnet*/
  unsigned int reg_address;    /* address on board*/
  unsigned char channel;          /* trb channel id (0...3) */
 struct pexor_userbuf tkbuf[TRBNET_MAX_BUFS]; /* dma buffers with received result data*/
};



#endif /* PEXOR_USER_H_ */
