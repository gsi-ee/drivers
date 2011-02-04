/*
 * pexor2_defs.h
 *
 *      PEXOR 2/3 board register definitions
 */

#ifndef _PCI_PEXOR2DEFS_H_
#define _PCI_PEXOR2DEFS_H_


/* ids for pexor card:*/
#define MY_VENDOR_ID 0x1204
#define MY_DEVICE_ID 0x5303

/* enable usage of SFP */
#define PEXOR_WITH_SFP 1

/* enable usage of TRIXOR */
#define PEXOR_WITH_TRIXOR 1

/* define address offsets of pexor */

/* DRAM:*/
//#define PEXOR_DRAM		0x200000
//#define PEXOR_RAMSIZE   0x6000  /* 24 kbyte*/

#define PEXOR_DRAM		0x100000 /* use the first SFP port for DMA testing here*/
#define PEXOR_RAMSIZE   0xFFFC  /* test covers first sfp port range here*/


#define PEXOR_BURST		0x80

/* DMA registers and commands:*/
#define PEXOR_DMA_BASE      	0x20000
#define PEXOR_DMA_SRC			0x00
#define PEXOR_DMA_DEST			0x04
#define PEXOR_DMA_LEN			0x08
#define PEXOR_DMA_BURSTSIZE		0x0C
#define PEXOR_DMA_CTRLSTAT		0x10 /* control, 1:-start*/



 /*OLD REGISTERS pexor 1*/

#define PEXOR_IRQ_CTRL			PEXOR_DMA_BASE + 0x14
#define PEXOR_IRQ_STAT		        PEXOR_DMA_BASE + 0x18


#define PEXOR_TRIXOR_BASE		0x40000

#define PEXOR_TRIX_CTRL 0x04
#define PEXOR_TRIX_STAT 0x00
#define PEXOR_TRIX_FCTI 0x08
#define PEXOR_TRIX_CVTI 0x0C

/* definitions for TRIXOR status and control register
 * taken from mbs driver trig_cam.h*/

/*--- status register bits ---------*/
#define TRIX_DT_CLEAR     0x00000020
#define TRIX_IRQ_CLEAR    0x00001000
#define TRIX_DI_CLEAR     0x00002000
#define TRIX_EV_IRQ_CLEAR 0x00008000
#define TRIX_EON          0x00008000
#define TRIX_FC_PULSE     0x00000010

/*--- control register bits --------*/
#define TRIX_MASTER     0x00000004
#define TRIX_SLAVE      0x00000020
#define TRIX_HALT       0x00000010
#define TRIX_GO         0x00000002
#define TRIX_EN_IRQ     0x00000001
#define TRIX_DIS_IRQ    0x00000008
#define TRIX_CLEAR      0x00000040
#define TRIX_BUS_ENABLE 0x00000800
#define TRIX_BUS_DISABLE 0x00001000






#define PEXOR_DMA_ENABLED_BIT 	0x1

/*#define PEXOR_IRQ_USER_BIT 		0x01*/

#define PEXOR_IRQ_USER_BIT TRIX_EON

/* SFP registers and commands:*/
#define PEXOR_SFP_NUMBER 4 /* number of used sfp connections*/

#define PEXOR_SFP_BASE 0x21000

#define PEXOR_SFP_REQ_COMM 0x00
#define PEXOR_SFP_REQ_ADDR 0x04
#define PEXOR_SFP_REQ_DATA 0x08
#define PEXOR_SFP_REP_STAT_CLR  0x0c
#define PEXOR_SFP_REP_STAT_0 0x10
#define PEXOR_SFP_REP_STAT_1 0x14
#define PEXOR_SFP_REP_STAT_2 0x18
#define PEXOR_SFP_REP_STAT_3 0x1c
#define PEXOR_SFP_REP_ADDR_0 0x20
#define PEXOR_SFP_REP_ADDR_1 0x24
#define PEXOR_SFP_REP_ADDR_2 0x28
#define PEXOR_SFP_REP_ADDR_3 0x2c
#define PEXOR_SFP_REP_DATA_0 0x30
#define PEXOR_SFP_REP_DATA_1 0x34
#define PEXOR_SFP_REP_DATA_2 0x38
#define PEXOR_SFP_REP_DATA_3 0x3c
#define PEXOR_SFP_RX_MONI 0x40
#define PEXOR_SFP_RX_RST  0x44
#define PEXOR_SFP_DISA 0x48
#define PEXOR_SFP_FAULT 0x4c

#define PEXOR_SFP_FIFO_0 0x50
#define PEXOR_SFP_FIFO_1 0x54
#define PEXOR_SFP_FIFO_2 0x58
#define PEXOR_SFP_FIFO_3 0x5c

#define PEXOR_SFP_TOKEN_REP_STAT_0 0x60
#define PEXOR_SFP_TOKEN_REP_STAT_1 0x64
#define PEXOR_SFP_TOKEN_REP_STAT_2 0x68
#define PEXOR_SFP_TOKEN_REP_STAT_3 0x6c

#define PEXOR_SFP_TOKEN_REP_HEAD_0 0x70
#define PEXOR_SFP_TOKEN_REP_HEAD_1 0x74
#define PEXOR_SFP_TOKEN_REP_HEAD_2 0x78
#define PEXOR_SFP_TOKEN_REP_HEAD_3 0x7c

#define PEXOR_SFP_TOKEN_REP_FOOT_0 0x80
#define PEXOR_SFP_TOKEN_REP_FOOT_1 0x84
#define PEXOR_SFP_TOKEN_REP_FOOT_2 0x88
#define PEXOR_SFP_TOKEN_REP_FOOT_3 0x8c

#define PEXOR_SFP_TOKEN_DSIZE_0 0x90
#define PEXOR_SFP_TOKEN_DSIZE_1 0x94
#define PEXOR_SFP_TOKEN_DSIZE_2 0x98
#define PEXOR_SFP_TOKEN_DSIZE_3 0x9c

#define PEXOR_SFP_TOKEN_DSIZE_SEL_0 0xa0
#define PEXOR_SFP_TOKEN_DSIZE_SEL_1 0xa4
#define PEXOR_SFP_TOKEN_DSIZE_SEL_2 0xa8
#define PEXOR_SFP_TOKEN_DSIZE_SEL_3 0xac

#define PEXOR_SFP_TOKEN_MEM_SIZE_0 0xb0
#define PEXOR_SFP_TOKEN_MEM_SIZE_1 0xb4
#define PEXOR_SFP_TOKEN_MEM_SIZE_2 0xb8
#define PEXOR_SFP_TOKEN_MEM_SIZE_3 0xbc

#define PEXOR_SFP_TX_STAT 0xc0


#define PEXOR_SFP_VERSION 0x1fc



#define PEXOR_SFP_TK_MEM_0 0x100000
#define PEXOR_SFP_TK_MEM_1 0x140000
#define PEXOR_SFP_TK_MEM_2 0x180000
#define PEXOR_SFP_TK_MEM_3 0x1c0000

#define PEXOR_SFP_TK_MEM_RANGE 0xFFFC /* length of each sfp port mem*/

/* command and reply masks:*/
#define PEXOR_SFP_PT_AD_R_REQ 0x240
#define PEXOR_SFP_PT_AD_W_REQ 0x644

#define PEXOR_SFP_PT_TK_R_REQ 0xA11
#define PEXOR_SFP_INI_REQ 0x344
#define PEXOR_SFP_RST_REQ 0x744

#define PEXOR_SFP_PT_AD_R_REP 0x044
#define PEXOR_SFP_PT_AD_W_REP 0x440
#define PEXOR_SFP_PT_TK_R_REP 0xA11 /*0x844*/
#define PEXOR_SFP_PT_INI_REP 0x244
#define PEXOR_SFP_PT_ERR_REP 0x544




#endif





