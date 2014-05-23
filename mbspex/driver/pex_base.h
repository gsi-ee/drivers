// N.Kurz, EE, GSI, 8-Apr-2010
// J.Adamczewski-Musch, EE, GSI, added mmap and some small fixes 24-Jan-2013
// JAM added generic probe/cleanup, privdata structure, sysfs, etc.  28-Jan-2013
// JAM refactoring of driver for mbs adding gosip ioctls etc. 3-Apr-2014 started...
//-----------------------------------------------------------------------------
#ifndef _PCI_PEX_BASE_H_
#define _PCI_PEX_BASE_H_

/*#define DEBUG*/

#include "pex_common.h"
#include "pex_gosip.h"





#define PEX_DRAM        0x100000 /* use the first SFP port for DMA testing here*/
#define PEX_RAMSIZE   0xFFFC  /* test covers first sfp port range here*/


#define PEX_BURST           0x80
#define PEX_BURST_MIN       0x10

/* DMA registers and commands:*/
#define PEX_DMA_BASE        0x20000
#define PEX_DMA_SRC         0x00
#define PEX_DMA_DEST            0x04
#define PEX_DMA_LEN         0x08
#define PEX_DMA_BURSTSIZE       0x0C
#define PEX_DMA_CTRLSTAT        0x10 /* control, 1:-start*/



 /*OLD REGISTERS pex 1*/

#define PEX_IRQ_CTRL            PEX_DMA_BASE + 0x14
#define PEX_IRQ_STAT                PEX_DMA_BASE + 0x18


#define PEX_TRIXOR_BASE     0x40000

#define PEX_TRIX_CTRL 0x04
#define PEX_TRIX_STAT 0x00
#define PEX_TRIX_FCTI 0x08
#define PEX_TRIX_CVTI 0x0C

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






#define PEX_DMA_ENABLED_BIT     0x1

/*#define PEX_IRQ_USER_BIT      0x01*/

#define PEX_IRQ_USER_BIT TRIX_EON
















/* workaround: kinpex does show too big bar0 size, use this instead*/
#define PEX_KINPEX_BARSIZE 0x400000

// some register offsets to read out version info:
//#define PEX_SFP_BASE 0x21000
//#define PEX_SFP_VERSION 0x1fc






//-----------------------------------------------------------------------------


/* helper structure containing mapped pointers to relevant regsiters:*/
struct regs_pex
{
  u32 *irq_control;             /* irq control register (on trixor) */
  u32 *irq_status;              /* irq status register  (on trixor) */
  u32 *dma_control_stat;        /* dma control and statusregister */
  u32 *dma_source;              /* dma source address */
  u32 *dma_dest;                /* dma destination address */
  u32 *dma_len;                 /* dma length */
  u32 *dma_burstsize;           /* dma burstsize, <=0x80 */
  u32 *ram_start;               /* RAM start */
  u32 *ram_end;                 /* RAM end */
  dma_addr_t ram_dma_base;      /* RAM start expressed as dma address */
  dma_addr_t ram_dma_cursor;    /* cursor for next dma to issue start */
  struct pex_sfp sfp;         /* contains registers of sfp engine */
#ifdef PEX_WITH_TRIXOR
    u32* trix_fcti; /* fast clear acceptance time register */
    u32* trix_cvti; /* conversion time register */
#endif
    unsigned char init_done; /* object is ready flag*/
};










/* put all board instance depending things into private data structure:*/
struct pex_privdata
{

    // JAM this is the generic part for all pci drivers
    dev_t devno; /* device number (major and minor) */
    int devid; /* local id (counter number) */
    char irqname[64]; /* private name for irq */
    struct pci_dev *pdev; /* PCI device */
    struct device *class_dev; /* Class device */
    struct cdev cdev; /* char device struct */
    struct regs_pex regs;       /* mapped register address pointers */
    unsigned long bases[6]; /* contains pci resource bases */
    unsigned long reglen[6]; /* contains pci resource length */
    void *iomem[6]; /* points to mapped io memory of the bars */
    u8 irqpin; /* hardware irq pin */
    u8 irqline; /* default irq line */
    struct semaphore ioctl_sem;      /* protects multi user ioctl access */
#ifdef  PEX_IRQ_WAITQUEUE
    wait_queue_head_t irq_trig_queue;     /* wait queue between top half ir_handler
                                                   and user wait trigger ioctl */
    atomic_t trig_outstanding;    /* outstanding triggers counter */
#else
    struct semaphore trix_sem;
    long trix_val;
#endif
    // here the special variables as used for pexor with mbs:
    u8 board_type; /* pexor, pexaria, kinpex, ...*/


    u32 l_bar0_base; /* unmapped bus address of bar 0*/
    u32 l_bar0_end;
    u32 l_bar0_trix_base; /* unmapped bus address of trixor register part*/

 // below is redundat, everything is already in regs_pex
//    u32 l_map_bar0_trix_base;
//
//    // trigger module registers
//    unsigned int *pl_stat;
//    unsigned int *pl_ctrl;
//    unsigned int *pl_fcti;
//    unsigned int *pl_cvti;

};


// from full driver for comparison:
//struct pexor_privdata
//{
//  int magic;                    /* magic number to identify irq */
//  atomic_t state;               /* run state of device */
//  dev_t devno;                  /* device number (major and minor) */
//  int devid;                    /* local id (counter number) */
//  char irqname[64];             /* private name for irq */
//  struct pci_dev *pdev;         /* PCI device */
//  struct device *class_dev;     /* Class device */
//  struct cdev cdev;             /* char device struct */
//  struct dev_pexor pexor;       /* mapped pexor address pointers */
//  unsigned long bases[6];       /* contains pci resource bases */
//  unsigned long reglen[6];      /* contains pci resource length */
//  void *iomem[6];               /* points to mapped io memory of the bars */
//  struct semaphore ramsem;      /* protects read/write access to mapped ram */
//  struct list_head free_buffers;        /* list containing the free buffers */
//  struct list_head received_buffers;    /* dma receive queue */
//  struct list_head used_buffers;        /* list containing the buffers in
//                                           use in client application */
//
//  spinlock_t buffers_lock;      /* protect any buffer lists operations */
//  spinlock_t dma_lock;      /* protects DMA Buffer */
//
//  atomic_t irq_count;           /* counter for irqs */
//  spinlock_t irq_lock;         /* optional lock between top and bottom half? */
//  struct tasklet_struct irq_bottomhalf; /* tasklet structure for isr
//                                           bottom half */
//  wait_queue_head_t irq_dma_queue;      /* wait queue between bottom
//                                           half and wait dma ioctl */
//  atomic_t dma_outstanding;     /* outstanding dma counter */
//  wait_queue_head_t irq_trig_queue;     /* wait queue between bottom half
//                                           and user wait trigger ioctl */
//  atomic_t trig_outstanding;    /* outstanding triggers counter */
//};





//void pex_show_version(struct pex_privdata *privdata, char* buf);

void pex_clear_pointers(struct  regs_pex* pg);
void pex_set_pointers(struct  regs_pex* pg, void* membase, unsigned long bar);

void print_register(const char* description, u32* address);
//-----------------------------------------------------------------------------
int pex_open(struct inode *inode, struct file *filp);
int pex_release(struct inode *inode, struct file *filp);

int pex_mmap(struct file *filp, struct vm_area_struct *vma);



/* the general fops ioctl */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int pex_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else
long pex_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif

/* pio read value from a single register on the pex board*/
int pex_ioctl_read_register(struct pex_privdata* priv, unsigned long arg);

/* pio write value from a single register on the pex board*/
int pex_ioctl_write_register(struct pex_privdata* priv, unsigned long arg);

/* dma read from memory on the pex board to a known physical address in host memory*/
int pex_ioctl_read_dma(struct pex_privdata* priv, unsigned long arg);

#ifdef PEX_WITH_TRIXOR
/* set acquisition state of trixor trigger module extension.
 * used to clear deadtime flag from user program and start/stop acquisition mode, etc.*/
int pex_ioctl_set_trixor(struct pex_privdata* priv, unsigned long arg);

/* Wait for a trigger interrupt from pex. Will be raised from trixor board
 * and routed to pci throug pex driver. */
int pex_ioctl_wait_trigger(struct pex_privdata *priv, unsigned long arg);

#endif


/* initiate dma from source to dest. depending on channelmask, dma transfer will start immediately (channelmask=1), or will be
 * triggered after token mode data request from indicated sfp channel (channelmask = 1 << (sfp+1))
 * burst size may be defined*/
int pex_start_dma(struct pex_privdata *priv, dma_addr_t source, dma_addr_t dest, u32 dmasize, u32 channelmask, u32 burst);

/* poll on dma status word until dma is complete*/
int pex_poll_dma_complete(struct pex_privdata* priv);



irqreturn_t irq_hand(int irq, void *dev_id);

void cleanup_device(struct pex_privdata* priv);




#ifdef PEX_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
/* export something to sysfs:*/
ssize_t pex_sysfs_codeversion_show(struct device *dev,
        struct device_attribute *attr, char *buf);
ssize_t pex_sysfs_trixorregs_show(struct device *dev,
        struct device_attribute *attr, char *buf);
ssize_t pex_sysfs_bar0base_show(struct device *dev,
        struct device_attribute *attr, char *buf);
ssize_t pex_sysfs_trixorbase_show(struct device *dev,
        struct device_attribute *attr, char *buf);
ssize_t pex_sysfs_dmaregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);

#endif
#endif

#endif
