/***
 * gapgor_common.h
 *
 *  Created on: 08.04.2014
 *      Author: J. Adamczewski-Musch, GSI
 *
 *      GAPGOR/KINGAPG driver common includes for all modules
 */

#ifndef _PCI_GAPG_COMMON_H_
#define _PCI_GAPG_COMMON_H_

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
#include <linux/vmalloc.h>

#include <linux/wait.h>

/**
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/io.h>

*/

/*
#include <asm/system.h>
*/

#include "gapg_user.h"

/** ids for gapgor card:*/
#define GAPGOR_VENDOR_ID 0x1204
#define GAPGOR_DEVICE_ID 0x5303







#define GAPG_DEBUGPRINT 1


/* debug latencies of trigger/token request*/
//#define GAPG_TRIGGERDEBUG 1


/** this will enable a faster parameter copy between kernel and user space at ioctls
 * but system most likely will crash when daq is interrupted with resl*/
//#define GAPG_COPY_USER_NOCHECK 1


///** this disables that pages of pipe will locked to memory
// * when creating scatter-gather list for virtual pipes, avoiding
// * a deadlock with readout or collector process.
// * The idea is that shared memory pipe will do a memory lock anyway.
// *  */
//#define GAPG_SG_NO_MEMLOCK 1
//
//
///* if defined we merge together physically adjacent pages in sg list as delivered from system
// * Note that this is _not_ done by pci_map_sg by default.
// * May disable merging for testing purposes*/
//#define GAPG_SG_REDUCE_SGLIST 1
//
//
///* if set, complete mbs pipe is synced for device/cpu at each ioctl readdmapipe.
// * if undefined, we will sync only parts of sglist that have been touched*/
//#define GAPG_SG_SYNCFULLPIPE 1

/** maximum number of devices controlled by this driver*/
#define GAPG_MAXDEVS 4

///////////////////////////////////77

///** enable usage of TRIXOR */
//#define GAPG_WITH_TRIXOR 1








/* with this define we can disable the ioctl semaphore
 * for performance reason?*/
//#define GAPG_NO_IOCTL_SEM 1


#define GAPG_ENABLE_IRQ 1

/** use waitqueue after isr instead of semaphore
 * NOTE: this does not make sense with mbs/pipe, because we do
 * not have automatic dma readout to kernel buffer queue as in dabc driver here!*/
/**#define GAPG_IRQ_WAITQUEUE 1*/

#define GAPG_SYSFS_ENABLE 1


#ifdef GAPG_DEBUGPRINT
#define gapg_dbg( args... )                    \
  printk( args );
#else
#define gapg_dbg( args... ) ;
#endif

#ifdef GAPG_TRIGGERDEBUG
#define gapg_tdbg( args... )                    \
  printk( args );
#else
#define gapg_tdbg( args... ) ;
#endif

#define gapg_msg( args... )                    \
  printk( args );


struct gapg_privdata;


// here switch different copy to/from user functions
// the regular copy may sleep, the "__" one not but may crash
// in case of page fault! better do not use this

#ifdef GAPG_COPY_USER_NOCHECK

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#define gapg_copy_to_user __copy_to_user_inatomic
#define gapg_copy_from_user __copy_from_user_inatomic
#else
#define gapg_copy_to_user copy_to_user
#define gapg_copy_from_user copy_from_user
#endif

#else

#define gapg_copy_to_user copy_to_user
#define gapg_copy_from_user copy_from_user

#endif


#endif
