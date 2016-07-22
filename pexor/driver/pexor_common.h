/*
 * \file
 * pexor_common.h
 *
 *  Created on: 08.02.2011 - 06.10.2014
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
#include <linux/list.h>
#include <linux/jiffies.h>
#include <linux/wait.h>


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


//#define PEXOR_DEBUGPRINT 1


#define PEXOR_ENABLE_IRQ 1

#define PEXOR_SYSFS_ENABLE 1

/** if set, triggerless readout may use autonomous workqueue task. For development*/
#define PEXOR_TRIGGERLESS_WORKER 1

/** define this to use a channelwise spinlock for triggerless acquisiton*/
//#define PEXOR_TRIGGERLESS_SPINLOCK 1

/** next try: semaphores instead of spinlocks, since we may sleep in worker.*/
#define PEXOR_TRIGGERLESS_SEMAPHORE 1

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
