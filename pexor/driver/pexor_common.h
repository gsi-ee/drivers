/*
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

#include <linux/wait.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/system.h>


#include "pexor_user.h"

/* ids for pexor card:*/
#define MY_VENDOR_ID 0x1204
#define MY_DEVICE_ID 0x5303

//#define PEXOR_DEBUGPRINT 1


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