/*
* Copyright (C) 2012-2015 GSI (www.gsi.de)
* Authors: Cesar Prados, Joern Adamczewski-Musch
* based partially on code examples by  J.-F.Gilot, IoXos
*
* Released according to the GNU GPL, version 2 or any later version
*
* Driver for VETAR VME board for IPV Linux
* last changed: 20-October-2015 by JAM
*/
#ifndef __VETARIPV_H__
#define __VETARIPV_H__

#include <linux/firmware.h>

#include "wishbone.h"

#include <linux/kernel.h>
#include <linux/version.h>
//#include <linux/pci.h>
#include <linux/sysfs.h>
#include <linux/delay.h>
#include <linux/pagemap.h>
#include <linux/scatterlist.h>


#include <linux/uaccess.h>
#include <linux/io.h>

#include <asm/uaccess.h>         // copy_to_user and copy_from_user
#include <linux/init.h>          // modules
#include <linux/module.h>        // module
#include <linux/types.h>         // dev_t type
#include <linux/fs.h>            // chrdev allocation
#include <linux/slab.h>          // kmalloc and kfree
#include <linux/cdev.h>          // struct cdev
#include <linux/errno.h>         // error codes
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/interrupt.h>

#include "/usr/src/PEV1100/include/pevioctl.h"
#include "/usr/src/PEV1100/include/vmeioctl.h"
#include "/usr/src/PEV1100/drivers/pevdrvr.h"
#include "/usr/src/PEV1100/drivers/pevklib.h"




//#define DEBUG 1

#define VETAR_DUMP_REGISTERS 1

#define VETAR_SYSFS_ENABLE 1
#define VETAR_ENABLE_IRQ 1

//#define VETAR_TRIGMOD_TEST 1
#define VETAR_CTRL_TEST 1

#define VETAR_MAP_REGISTERS 1
#define VETAR_MAP_CONTROLSPACE 1

/** define this to use ELB bus mapping for PEV1100. otherwise standard pci mapping */
#define VETAR_MAP_ELB 1

/** enable this one to modify PEV vme settings*/
//#define VETAR_CONFIGURE_VME 1


#define VETAR_REGS_ADDR   0x1000000 /* this is default*/
//#define VETAR_REGS_ADDR 0x0
#define VETAR_REGS_SIZE   0x1000000;

#define VETAR_CTRLREGS_SIZE 0xA0

//#define TRIGMOD_REGS_ADDR   0x2000000
//#define TRIGMOD_REGS_SIZE   0x100





#define CONTROL_REGISTER 0
#define ERROR_FLAG    0
#define SDWB_ADDRESS  8

/* some VME address modifiers used for the 3 mappings:*/
#define VME_A24_USER_MBLT 0x38
#define VME_A24_USER_DATA_SCT 0x39
#define VME_A24_SUP_DATA_SCT   0x3d
//#define VME_A32_USER_MBLT 0x08
#define VME_A32_USER_DATA_SCT 0x09
#define VME_A32_SUP_DATA_SCT 0x0d
#define VME_CR_CSR 0x2f

/* VME WB Interface*/
#define CTRL 16
#define MASTER_CTRL 24
#define MASTER_ADD 32
#define MASTER_DATA 40
#define EMUL_DAT_WD 48
#define WINDOW_OFFSET_LOW  56
#define WINDOW_OFFSET_HIGH 64

#define WBM_ADD_MASK 0xFFFFFFFC

#define WINDOW_HIGH 0xFFFF0000UL
#define WINDOW_LOW  0x0000FFFCUL





#define VETARVERSION     "1.1"
#define VETARNAME       "vetar"
#define VETARNAMEFMT    "vetar%d"


#define VME_WB 
#define VETAR_MAX_DEVICES        32
#define VETAR_DEFAULT_IDX { [0 ... (VETAR_MAX_DEVICES-1)] = -1 }

#define VETAR_IRQ_LEVEL	2
#define VETAR_VENDOR_ID		0x80031

#define VME_VENDOR_ID_OFFSET	0x24



#define VETAR_CONFIGSIZE 0x80000 /* size of cr/csr space if any*/

/* VETAR CSR offsets */

#define FUN0ADER    0x7FF63
#define FUN1ADER    0x7FF73



#define INT_LEVEL   0x7ff5b
#define INTVECTOR   0x7ff5f
#define WB_32_64    0x7ff33
#define BIT_SET_REG 0x7FFFB
#define BIT_CLR_REG 0x7FFF7
#define WB32        1
#define WB64        0
#define RESET_CORE  0x80
#define ENABLE_CORE 0x10








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


#if defined(__BIG_ENDIAN)
#define endian_addr(width, shift) (sizeof(wb_data_t)-width)-shift
#elif defined(__LITTLE_ENDIAN)
#define endian_addr(width, shift) shift
#else
#error "unknown machine byte order (endian)"
#endif



#define VETAR_BUS_DELAY 500

#define vetar_bus_delay()                       \
  mb();      \
  ndelay(VETAR_BUS_DELAY);



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
    struct wishbone wb; /* wishbone structure*/
    struct mutex    wb_mutex; /* wishbone mutex*/
    unsigned char wb_is_registered; /* mark here if wishbone has been registered*/
    unsigned int wb_low_addr; /* wishbone access parameters*/
    unsigned int wb_width;    /* wishbone access parameters*/
    unsigned int wb_shift;    /* wishbone access parameters */
    unsigned int wb_window_offset; /* wishbone access parameters */

    struct semaphore ramsem;      /* protects read/write access to mapped ram */

    uint32_t        configbase; /* base adress in vme address space*/

    //struct vme_board vme_board_config; /* for mapping of board config space ?*/
    struct pev_ioctl_map_pg pev_vetar_cscr; /* for mapping of board config space*/
	void __iomem *cr_csr;    /* kernel mapped address of board configuration/status space*/
    phys_addr_t cr_csr_phys; /* physical bus address of board configuration/status space*/
    unsigned long configlen; /* contains config space length to be mapped */
    uint32_t		vmebase; /* base adress in vme address space for wishbone memory*/
    uint32_t       ctrl_vmebase; /* base adress in vme address space for control memory*/
#ifdef  VETAR_MAP_ELB
    struct vme_board vme_board_registers; /* for mapping of board register space*/
#else
    struct pev_ioctl_map_pg pev_vetar_regs; /* for mapping of board register space*/
    phys_addr_t regs_phys; /* physical bus address of board register space*/
#endif
    void __iomem *registers; /* kernel mapped address of board register space*/
	unsigned long reglen; /* contains register length to be mapped */

#ifdef  VETAR_MAP_ELB
	struct vme_board vme_board_ctrl; /* for mapping of board control register space*/
#else
	 struct pev_ioctl_map_pg pev_vetar_ctrl_regs; /* for mapping of control register space*/
	 phys_addr_t ctrl_regs_phys; /* physical bus address of control register space*/
#endif
	void __iomem *ctrl_registers; /* kernel mapped address of board control register space*/
    unsigned long ctrl_reglen; /* contains control register length to be mapped */


    unsigned int vme_itc; /* ioport for interrupt control register of vetar*/
	unsigned long		irqcount; /* optional irq count*/
	unsigned char sysfs_has_file; /* mark here if sysfs has exported files*/
	unsigned char init_done; /* object is ready flag*/
};


/* helper to access private data in file struct. returns 0 if vetar
 * not initialized */
struct vetar_privdata *get_privdata(struct file *fil);

/* check from configuration space if vendor id matches*/
int vetar_is_present(struct vetar_privdata *privdata);
void vetar_csr_write(u8 value, void *base, u32 offset);

void vetar_setup_csr_fa(struct vetar_privdata *privdata);

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
static void vetar_cleanup_dev(struct vetar_privdata *privdata, unsigned int index);


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
 // OLD JAM static void vetar_irqhandler(int vec, int prio, void *arg);
  static void vetar_irqhandler(struct pev_dev *pev, int src, void *arg);

  #endif

#ifdef VETAR_TRIGMOD_TEST
int vetar_dump_trigmod(struct vetar_privdata *privdata);
#endif
#endif /* __VETAR_H__ */
