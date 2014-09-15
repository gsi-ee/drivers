/*
 * \file
 * pexor_common.h
 *
 *  Created on: 08.02.2011
 *      Author: J. Adamczewski-Musch, GSI
 *
 *      PEXOR driver common includes for all modules
 */

#ifndef _PCI_PEXOR_COMMON_H_
#define _PCI_PEXOR_COMMON_H_

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/pagemap.h>
#include <linux/scatterlist.h>
#include <linux/page-flags.h>
#include <linux/sched.h>

#include <linux/wait.h>

/*
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/io.h>

*/

/*
#include <asm/system.h>
*/

#include "pexor_user.h"

/* ids for pexor card:*/
#define PEXOR_VENDOR_ID 0x1204
#define PEXOR_DEVICE_ID 0x5303

#define PEXARIA_VENDOR_ID     0x1172
#define PEXARIA_DEVICE_ID     0x1111

#define KINPEX_VENDOR_ID     0x10EE
#define KINPEX_DEVICE_ID     0x1111

#define BOARDTYPE_PEXOR 0
#define BOARDTYPE_PEXARIA 1
#define BOARDTYPE_KINPEX 2


#define PEXOR_DEBUGPRINT 1

// this will enable mode where pexor on board memory is not used as buffer
// instead, data from sfp token request will be directly streamed to host dma buffer.
#define PEXOR_DIRECT_DMA 1


/* this define will switch at compiletime between trbnet and gosip protocols*/
//#define PEXOR_WITH_TRBNET

#define PEXOR_ENABLE_IRQ 1

#define PEXOR_SYSFS_ENABLE 1


#ifdef PEXOR_DEBUGPRINT
#define pexor_dbg( args... )                    \
  printk( args );
#else
#define pexor_dbg( args... ) ;
#endif


#define pexor_msg( args... )                    \
  printk( args );


struct pexor_privdata;




#endif
