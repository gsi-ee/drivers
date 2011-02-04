/*
 * pexor1_defs.h
 *
 *      PEXOR 1 board register definitions
 */

#ifndef _PCI_PEXOR1DEFS_H_
#define _PCI_PEXOR1DEFS_H_


/* ids for pexor card:*/
#define MY_VENDOR_ID 0x1204
#define MY_DEVICE_ID 0x5303

/* define address offsets of pexor */

#define PEXOR_DRAM		0x200000
#define PEXOR_RAMSIZE   0x6000  /* 24 kbyte*/
#define PEXOR_BURST		0x80


#define PEXOR_DMA_BASE      	0x20000
#define PEXOR_DMA_SRC			0x00
#define PEXOR_DMA_DEST			0x04
#define PEXOR_DMA_LEN			0x08
#define PEXOR_DMA_BURSTSIZE		0x0C
#define PEXOR_DMA_CTRLSTAT		0x10 /* control, 1:-start*/
#define PEXOR_IRQ_CTRL			0x14
#define PEXOR_IRQ_STAT		    0x18

#define PEXOR_DMA_ENABLED_BIT 	0x1
#define PEXOR_IRQ_USER_BIT 		0x01


#endif





