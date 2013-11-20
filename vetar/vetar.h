/*
* Copyright (C) 2012-2013 GSI (www.gsi.de)
* Author: Cesar Prados, Joern Adamczewski-Musch
*
* Released according to the GNU GPL, version 2 or any later version
*
* Driver for VETAR VME board for CES SugarHat Linux
*/
#ifndef __VETAR_H__
#define __VETAR_H__

#include <linux/firmware.h>
//#include <vmebus.h>

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


#define DEBUG 1
#define VETAR_SYSFS_ENABLE 1
//#define VETAR_ENABLE_IRQ 1

#define VETAR_TRIGMOD_TEST 1


//#define VETAR_IRQ_VECTOR  0x50
//#define VETAR_IRQ_MASK    ((1 << 3) | (1 << 4)) /* interrupt happens at level 3 or 4 */
#define VETAR_REGS_ADDR   0x1000000 /* this is default*/
#define VETAR_REGS_SIZE   0x100000
//

#define TRIGMOD_REGS_ADDR   0x2000000
#define TRIGMOD_REGS_SIZE   0x100


#define VETARVERSION     "1.0"
#define VETARNAME       "vetar"
#define VETARNAMEFMT    "vetar%d"


#define VME_WB 
#define VETAR_MAX_DEVICES        32
#define VETAR_DEFAULT_IDX { [0 ... (VETAR_MAX_DEVICES-1)] = -1 }

#define VETAR_IRQ_LEVEL	2
#define VETAR_VENDOR_ID		0x80031

#define VME_VENDOR_ID_OFFSET	0x24


//#define VME_CR_CSR       0x2f /* 0x2f */

#define VETAR_CONFIGSIZE 0x80000 /* size of cr/csr space if any*/

/* VETAR CSR offsets */
#define FUN0ADER	0x7FF63
#define INT_LEVEL	0x7ff5b
#define INTVECTOR	0x7ff5f
#define WB_32_64	0x7ff33
#define BIT_SET_REG	0x7FFFB
#define BIT_CLR_REG	0x7FFF7
#define WB32		1
#define WB64		0
#define RESET_CORE	0x80
#define ENABLE_CORE	0x10




#ifdef VETAR_NEW_XPCLIB
#include <ces/CesXpcBridge.h>
#endif

#include <ces/xpc_vme.h>
#include <ces/xpc.h>

enum vme_address_modifier {
    VME_A64_MBLT        = 0,    /* 0x00 */
    VME_A64_SCT,            /* 0x01 */
    VME_A64_BLT     = 3,    /* 0x03 */
    VME_A64_LCK,            /* 0x04 */
    VME_A32_LCK,            /* 0x05 */
    VME_A32_USER_MBLT   = 8,    /* 0x08 */
    VME_A32_USER_DATA_SCT,      /* 0x09 */
    VME_A32_USER_PRG_SCT,       /* 0x0a */
    VME_A32_USER_BLT,       /* 0x0b */
    VME_A32_SUP_MBLT,       /* 0x0c */
    VME_A32_SUP_DATA_SCT,       /* 0x0d */
    VME_A32_SUP_PRG_SCT,        /* 0x0e */
    VME_A32_SUP_BLT,        /* 0x0f */
    VME_2e6U        = 0x20, /* 0x20 */
    VME_2e3U,           /* 0x21 */
    VME_A16_USER        = 0x29, /* 0x29 */
    VME_A16_LCK     = 0x2c, /* 0x2c */
    VME_A16_SUP     = 0x2d, /* 0x2d */
    VME_CR_CSR      = 0x2f, /* 0x2f */
    VME_A40_SCT     = 0x34, /* 0x34 */
    VME_A40_LCK,            /* 0x35 */
    VME_A40_BLT     = 0x37, /* 0x37 */
    VME_A24_USER_MBLT,      /* 0x38 */
    VME_A24_USER_DATA_SCT,      /* 0x39 */
    VME_A24_USER_PRG_SCT,       /* 0x3a */
    VME_A24_USER_BLT,       /* 0x3b */
    VME_A24_SUP_MBLT,       /* 0x3c */
    VME_A24_SUP_DATA_SCT,       /* 0x3d */
    VME_A24_SUP_PRG_SCT,        /* 0x3e */
    VME_A24_SUP_BLT,        /* 0x3f */
};


#ifdef DEBUG
#define debug(x)        printk x
#else // DEBUG
#define debug(x)
#endif // DEBUG


#ifdef DEBUG
#define vetar_dbg( args... )                    \
  printk( args );
#else
#define vetar_dbg( args... ) ;
#endif

#define vetar_msg( args... )                    \
  printk( args );


#ifndef VETAR_NEW_XPCLIB
extern int xpc_vme_request_irq(unsigned int vec, unsigned int lev, void (*handler)(int vec, int prio, void *arg), void *arg, const char *name);
extern void xpc_vme_free_irq(unsigned int vec);
#endif






/* Our device's private structure */
struct vetar_privdata {
	int			lun;   /* logical device unit */
	int			slot;  /* slot number (do we have configuration space from this?) */
	int			vector;
	int			level;
	//char			*fw_name;
	//struct device		*dev; /* kernel device reference*/
	//char			driver[16];
	//char			description[80];
    dev_t devno; /* device number (major and minor) */
    char irqname[64]; /* private name for irq */
    struct device *class_dev; /* Class device */
    struct cdev cdev; /* char device struct */
    struct semaphore ramsem;      /* protects read/write access to mapped ram */
    uint32_t        configbase; /* base adress in vme address space*/
	void __iomem *cr_csr;    /* kernel mapped address of board configuration/status space*/
    phys_addr_t cr_csr_phys; /* physical bus address of board configuration/status space*/
    unsigned long configlen; /* contains config space length to be mapped */
    uint32_t		vmebase; /* base adress in vme address space*/
	void __iomem *registers; /* kernel mapped address of board register space*/
	unsigned long reglen; /* contains register length to be mapped */
	phys_addr_t regs_phys; /* physical bus address of board register space*/
	unsigned long		irqcount; /* optional irq count*/
	unsigned char init_done; /* object is ready flag*/
};


/* helper to access private data in file struct. returns 0 if vetar
 * not initialized */
struct vetar_privdata *get_privdata(struct file *fil);

/* check from configuration space if vendor id matches*/
int vetar_is_present(struct vetar_privdata *privdata);
void vetar_csr_write(u8 value, void *base, u32 offset);

void vetar_setup_csr_fa0(struct vetar_privdata *privdata);

/* File operations:*/
int vetar_open(struct inode *inode, struct file *filp);
int vetar_release(struct inode *inode, struct file *filp);
loff_t vetar_llseek(struct file *filp, loff_t off, int whence);
ssize_t vetar_read(struct file *filp, char __user * buf, size_t count,
                   loff_t * f_pos);
ssize_t vetar_write(struct file *filp, const char __user * buf, size_t count,
                    loff_t * f_pos);

/*
 * Here we probe vetar device of index in module parameter array*/
static int vetar_probe_vme(unsigned int index);

/* cleanup device with private device data*/
static void vetar_cleanup_dev(struct vetar_privdata *privdata);


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int vetar_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else
long vetar_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif



#ifdef VETAR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

ssize_t vetar_sysfs_codeversion_show(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf);



#endif
#endif

#ifdef VETAR_ENABLE_IRQ
 static void vetar_irqhandler(int vec, int prio, void *arg);
 static int vetar_get_irqcount(struct vetar_privdata *dev, int clear);
#endif

#ifdef VETAR_TRIGMOD_TEST
int vetar_dump_trigmod(struct vetar_privdata *privdata);
#endif
#endif /* __VETAR_H__ */
