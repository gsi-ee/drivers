/** \file
 * basic functions of galapagos driver.
 *
 * JAM driver for GSI Arbitrary Logic And Pattern Generator System (galapagos)
 *  started from mbspex driver template 05-Aug-2019
 */

//-----------------------------------------------------------------------------
#ifndef _PCI_GAPG_BASE_H_
#define _PCI_GAPG_BASE_H_

/*#define DEBUG*/

#include "gapg_common.h"



#define GAPG_DRAM        0x100000 /**< use the first SFP port for DMA testing here*/
#define GAPG_RAMSIZE   0xFFFC  /**< test covers first sfp port range here*/


//#define GAPG_BURST           0x80
//#define GAPG_BURST_MIN       0x10

/** DMA registers and commands:*/
#define GAPG_DMA_BASE        0x20000
//#define GAPG_DMA_SRC         0x00
//#define GAPG_DMA_DEST            0x04
//#define GAPG_DMA_LEN         0x08
//#define GAPG_DMA_BURSTSIZE       0x0C
//#define GAPG_DMA_CTRLSTAT        0x10 /**< control, 1:-start*/
//
//
//
// /**OLD REGISTERS gapg 1*/
//
//#define GAPG_IRQ_CTRL            GAPG_DMA_BASE + 0x14
//#define GAPG_IRQ_STAT                GAPG_DMA_BASE + 0x18
//
//
#define GAPG_TRIXOR_BASE     0x40000
//
#define GAPG_TRIX_CTRL 0x04
#define GAPG_TRIX_STAT 0x00
//#define GAPG_TRIX_FCTI 0x08
//#define GAPG_TRIX_CVTI 0x0C

/** definitions for TRIXOR status and control register
 * taken from mbs driver trig_cam.h*/

///**--- status register bits ---------*/
#define TRIX_DT_CLEAR     0x00000020
#define TRIX_IRQ_CLEAR    0x00001000
//#define TRIX_DI_CLEAR     0x00002000
#define TRIX_EV_IRQ_CLEAR 0x00008000
//#define TRIX_EON          0x00008000
//#define TRIX_FC_PULSE     0x00000010
//
///**--- control register bits --------*/
//#define TRIX_MASTER     0x00000004
//#define TRIX_SLAVE      0x00000020
//#define TRIX_HALT       0x00000010
//#define TRIX_GO         0x00000002
//#define TRIX_EN_IRQ     0x00000001
//#define TRIX_DIS_IRQ    0x00000008
//#define TRIX_CLEAR      0x00000040
//#define TRIX_BUS_ENABLE 0x00000800
//#define TRIX_BUS_DISABLE 0x00001000






#define GAPG_DMA_ENABLED_BIT     0x1

/*#define GAPG_IRQ_USER_BIT      0x01*/

#define GAPG_IRQ_USER_BIT TRIX_EON






/** on some PCs, need maybe waitstates for simple bus io:*/

#define GAPG_BUS_DELAY 20

#define gapg_bus_delay()                       \
  mb();      \
  ndelay(GAPG_BUS_DELAY);

 //ndelay(20);








// some register offsets to read out version info:
//#define GAPG_SFP_BASE 0x21000
//#define GAPG_SFP_VERSION 0x1fc






//-----------------------------------------------------------------------------


/** helper structure containing mapped pointers to relevant registers:*/
struct regs_gapg
{
  u32 *irq_control;             /**< irq control register (on trixor) */
  u32 *irq_status;              /**< irq status register  (on trixor) */
//  u32 *dma_control_stat;        /**< dma control and statusregister */
//  u32 *dma_source;              /**< dma source address */
//  u32 *dma_dest;                /**< dma destination address */
//  u32 *dma_len;                 /**< dma length */
//  u32 *dma_burstsize;           /**< dma burstsize, <=0x80 */
  u32 *ram_start;               /**< RAM start */
  u32 *ram_end;                 /**< RAM end */
//  dma_addr_t ram_dma_base;      /**< RAM start expressed as dma address */
//  dma_addr_t ram_dma_cursor;    /**< cursor for next dma to issue start */
//  struct gapg_sfp sfp;         /**< contains registers of sfp engine */
    unsigned char init_done; /**< object is ready flag*/
};

///* this contains scatter gather information of virtual mbs pipe*/
//struct mbs_pipe
//{
//  unsigned long virt_start;         /**< virtual start address*/
//  unsigned long size;               /**< total size of pipe */
//  struct scatterlist* sg;           /**<  sg list of pipe memory*/
//  unsigned int sg_ents;             /**< actual entries in the scatter/gatter list (NOT nents for the map function, but the result) */
//  struct page **pages;              /**< list of pointers to the pages */
//  int num_pages;                    /**< number of pages for this user memory area*/
//};





/** put all board instance depending things into private data structure:*/
struct gapg_privdata
{

    // JAM this is the generic part for all pci drivers
    dev_t devno; /**< device number (major and minor) */
    int devid; /**< local id (counter number) */
    char irqname[64]; /**< private name for irq */
    struct pci_dev *pdev; /**< PCI device */
    struct device *class_dev; /**< Class device */
    struct cdev cdev; /**< char device struct */
    struct regs_gapg regs;       /**< mapped register address pointers */
   // u32 sfp_maxpolls; /**< number of retries when polling for sfp response ready */
    u32 sfp_buswait; /**< sfp bus waitstates in ns for each bus read/write ioctl. To adjust for frontend slaves speed */
   // struct mbs_pipe pipe;       /**< sg information on mbs pipe, for mode 4*/
    unsigned long bases[6]; /**< contains pci resource bases */
    unsigned long reglen[6]; /**< contains pci resource length */
    void *iomem[6]; /**< points to mapped io memory of the bars */
    u8 irqpin; /**< hardware irq pin */
    u8 irqline; /**< default irq line */
    struct semaphore ioctl_sem;      /**< protects multi user ioctl access */
    // here the special variables as used for gapgor with mbs:
    u32 l_bar0_base; /**< unmapped bus address of bar 0*/
    u32 l_bar0_end;
};





//void gapg_show_version(struct gapg_privdata *privdata, char* buf);

void gapg_clear_pointers(struct  regs_gapg* pg);
void gapg_set_pointers(struct  regs_gapg* pg, void* membase, unsigned long bar);

void print_register(const char* description, u32* address);
//-----------------------------------------------------------------------------
int gapg_open(struct inode *inode, struct file *filp);
int gapg_release(struct inode *inode, struct file *filp);

int gapg_mmap(struct file *filp, struct vm_area_struct *vma);



/** the general fops ioctl */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int gapg_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else
long gapg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif

/** pio read value from a single register on the gapg board*/
int gapg_ioctl_read_register(struct gapg_privdata* priv, unsigned long arg);

/** pio write value from a single register on the gapg board*/
int gapg_ioctl_write_register(struct gapg_privdata* priv, unsigned long arg);

/** dma read from memory on the gapg board to a known physical address in host memory*/
int gapg_ioctl_read_dma(struct gapg_privdata* priv, unsigned long arg);

/** dma read from memory on the gapg board to a known virtual address in pipe*/
int gapg_ioctl_read_dma_pipe (struct gapg_privdata* priv, unsigned long arg);











irqreturn_t irq_hand(int irq, void *dev_id);

void cleanup_device(struct gapg_privdata* priv);




#ifdef GAPG_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
/** export something to sysfs: */
ssize_t gapg_sysfs_codeversion_show(struct device *dev,
        struct device_attribute *attr, char *buf);
ssize_t gapg_sysfs_gapgregs_show(struct device *dev,
        struct device_attribute *attr, char *buf);
ssize_t gapg_sysfs_bar0base_show(struct device *dev,
        struct device_attribute *attr, char *buf);

/* show sfp bus read/write waitstate in microseconds.
 * this will impose such wait time after each frontend address read/write ioctl */
ssize_t gapg_sysfs_buswait_show (struct device *dev, struct device_attribute *attr, char *buf);

/* set sfp bus read/write waitstate in microseconds. */
ssize_t gapg_sysfs_buswait_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);



#endif
#endif

#endif




