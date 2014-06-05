/*
 * pexor_common.h
 *
 *  Created on: 08.04.2014
 *      Author: J. Adamczewski-Musch, GSI
 *
 *      PEXOR/KINPEX driver common includes for all modules
 */

#ifndef _PCI_PEX_COMMON_H_
#define _PCI_PEX_COMMON_H_

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

#include "pex_user.h"

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


#define PEXVERSION     "1.5"


/*#define PEX_DEBUGPRINT 1*/


/* maximum number of devices controlled by this driver*/
#define PEX_MAXDEVS 4

///////////////////////////////////77

/* enable usage of TRIXOR */
#define PEX_WITH_TRIXOR 1


//#define INTERNAL_TRIG_TEST 1





/* timeout for ir wait queue, set to 5 s for slow poland tests */
#define PEX_WAIT_TIMEOUT (5*HZ)
/* maximum number of timeouts before wait loop terminates*/
#define PEX_WAIT_MAXTIMEOUTS 20

/* maximum number of polling cycles for dma complete bit*/
#define PEX_DMA_MAXPOLLS 10000

/* polling delay for each cycle in ns for dma complete bit*/
#define PEX_DMA_POLLDELAY 20

/* if set, we use a schedule() in the dma complete polling.
 * Note: according to linux kernel book, yield() will just prepare this
 * task to be scheduled in near future, but schedule() will initiate the
 * schedule directly*/
#define PEX_DMA_POLL_SCHEDULE 1




#define PEX_ENABLE_IRQ 1

/* use waitqueue after isr instead of semaphore
 * NOTE: this does not make sense with mbs/pipe, because we do
 * not have automatic dma readout to kernel buffer queue as in dabc driver here!*/
/*#define PEX_IRQ_WAITQUEUE 1*/

#define PEX_SYSFS_ENABLE 1


#ifdef PEX_DEBUGPRINT
#define pex_dbg( args... )                    \
  printk( args );
#else
#define pex_dbg( args... ) ;
#endif


#define pex_msg( args... )                    \
  printk( args );


struct pex_privdata;




#endif
