/*
 * pexor_trb.h
 *
 *     *      definitions and functions for pexor with
 *     HADES trbnet protocol
 */

#ifndef _PCI_PEXOR_TRB_H_
#define _PCI_PEXOR_TRB_H_


#include "pexor_common.h"


#ifdef PEXOR_WITH_TRBNET

#define PEXOR_BURST		0x80
#define PEXOR_DMA_ENABLED_BIT 	0x1

#define PEXOR_DRAM		0x100000 /* TODO: specify meaningful RAM position for fops io functions?*/
#define PEXOR_RAMSIZE   0xFFFC  /* test covers first sfp port range here*/

#define PEXOR_DMA_BASE      	0x00 /* base address of DMA engine*/

#define PEXOR_IRQ_CTRL			PEXOR_DMA_BASE  /* DUMMY TODO meaningful register for trbnet*/
#define PEXOR_IRQ_STAT		    PEXOR_DMA_BASE  /* DUMMY TODO meaningful register for trbnet*/
#define PEXOR_IRQ_USER_BIT 		0x01 /* DUMMY TODO meaningful register for trbnet*/


#define PEXOR_TRB_CHANS 4
#define PEXOR_TRB_SENDER_CONTROL         0x0100
#define PEXOR_TRB_TARGET_ADDRESS         0x0101
#define PEXOR_TRB_SENDER_ERROR           0x0102
#define PEXOR_TRB_SENDER_DATA            0x0103
#define PEXOR_TRB_SENDER_STATUS          0x010f
#define PEXOR_TRB_RECEIVER_DATA          0x0203
#define PEXOR_TRB_RECEIVER_FIFO_STATUS   0x0204
#define PEXOR_TRB_API_STATUS             0x0300

#define PEXOR_TRB_DMA_ADD                0x0700   /* DMA Start Address */
#define PEXOR_TRB_DMA_LEN                0x0701   /* (rw) DMA Buffer length in 32 bit words */
#define PEXOR_TRB_DMA_CTL                0x0702   /*(writing to register is sufficient, no need to clear it before writing)
														Bit 0: DMA start
														Bit 1: DMA reset - completely reset the full DMA handler

													 (r):
														Bit 0: DMA active, goes low after TrbNet access has been completed and DMA is finished
														Bit 1: DMA Buffer full. Buffer provided by CPU is full, waiting for new buffer and DMA start
														Bit 31..8: DMA Size. Number of written data words to current dma buffer. Only valid when Bit 0 is low or Bit 1 is high*/


#define PEXOR_TRB_DMA_BST                0x0703  /* Bit 7-0: DMA Burst Length in 32 bit words. Default: 0x1F */
#define PEXOR_TRB_DMA_STA                0x0704  /* Status Register: Various status bits of DMA engine (t.b.d.) */
#define PEXOR_TRB_DMA_CRE                0x0705  /* Status Register: Available credits (t.b.d.) */
#define PEXOR_TRB_DMA_CNT                0x0706  /* Status Register: Counter values (t.b.d.) */





#define PEXOR_TRB_BIT_DMA_ACTIVE          (0x1 << 0)
#define PEXOR_TRB_BIT_DMA_MORE            (0x1 << 1)

#define PEXOR_TRB_CMD_REGISTER_READ                0x08
#define PEXOR_TRB_CMD_REGISTER_READ_MEM            0x0a
#define PEXOR_TRB_CMD_REGISTER_WRITE               0x09
#define PEXOR_TRB_CMD_REGISTER_WRITE_MEM           0x0b
#define PEXOR_TRB_CMD_NETADMINISTRATION            0x0f




/* issue request on trbnet via command/data structure. */
int pexor_ioctl_trbnet_request(struct pexor_privdata *priv, unsigned long arg);


#endif /* ifdef PEXOR_WITH_TRBNET*/





#endif




