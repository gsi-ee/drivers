/*
 * pexor_base.h
 *
 *  Created on: 08.02.2011
 *      Author: J. Adamczewski-Musch, GSI
 *
 *      PEXOR driver base functionality
 *      contains DMA handling, irq, probe etc.
 */

#ifndef _PCI_PEXOR_BASE_H_
#define _PCI_PEXOR_BASE_H_

#include "pexor_common.h"

#ifdef PEXOR_WITH_TRBNET
#include "pexor_trb.h"
#else
#include "pexor_gos.h"
#endif


#define PEXOR_SHARED_IRQ 1


/* modes of interrupt complete handling:*/

/* polling mode in the wait ioctl. if not set, we wait on the event queue
 * of the tasklet which is executed by ir. */
#define DMA_WAITPOLLING 1

/* test: use spinlock to protect dma engine vs buffers. do we need this?
 * rather bad idea*/
/*#define DMA_SPINLOCK 1 */


/* polling mode in the nextdma function, raising ir when dma is done to
 * test subsequent handlers. if not set, polling mode in
 * nextdma will schedule tasklet directly */
/*#define DMA_EMULATE_IR 1*/

/* set this if pexor board itself raises ir after dma complete (not yet!)*/
/*#define DMA_BOARD_IR 1 */


/* switch on message signalled interrupt mode. Supported by PEXOR? maybe not*/
//#define IRQ_ENABLE_MSI 1


/* use streaming dma mapping, i.e. dma buffers are allocated with kmalloc and
 * then mapped for dma. Otherwise, we use coherent mapping
 * with pci_alloc_consistent
 * which takes coherent dma buffers from preallocated (?)  pci device memory*/
/*#define DMA_MAPPING_STREAMING 1*/






#ifdef DMA_SPINLOCK
#define pexor_dma_lock(lock) \
  spin_lock( lock);


#define pexor_dma_unlock(lock) \
  spin_unlock( lock );


#else
#define pexor_dma_lock(lock) \
  ;

#define pexor_dma_unlock(lock) \
  ;
#endif

/* maximum number of devices controlled by this driver*/
#define PEXOR_MAXDEVS 4

/* timeout for ir wait queue */
#define PEXOR_WAIT_TIMEOUT (1*HZ)
/* maximum number of timeouts before wait loop terminates*/
#define PEXOR_WAIT_MAXTIMEOUTS 20

/* maximum number of polling cycles for dma complete bit*/
#define PEXOR_DMA_MAXPOLLS 10000

/* polling delay for each cycle in ns for dma complete bit*/
#define PEXOR_DMA_POLLDELAY 0

/* if set, we use a schedule() in the dma complete polling.
 * Note: according to linux kernel book, yield() will just prepare this
 * task to be scheduled in near future, but schedpriv->pexor.irq_statusule() will initiate the
 * schedule directly*/
#define PEXOR_DMA_POLL_SCHEDULE 0

/* maximum number of outstandin buffers in receive queue,
   do we still need this?*/
#define PEXOR_MAXOUTSTANDING 50





struct dev_pexor
{

  u32 *irq_control;             /* irq control register */
  u32 *irq_status;              /* irq status register */
  u32 *dma_control_stat;        /* dma control and statusregister */
  u32 *dma_source;              /* dma source address */
  u32 *dma_dest;                /* dma destination address */
  u32 *dma_len;                 /* dma length */
  u32 *dma_burstsize;           /* dma burstsize, <=0x80 */
#ifdef PEXOR_WITH_TRBNET
  u32 *dma_statbits;            /* optional further status bits*/
  u32 *dma_credits;             /* credits*/
  u32 *dma_counts;              /* counter values*/
#endif
  u32 *ram_start;               /* RAM start */
  u32 *ram_end;                 /* RAM end */
  dma_addr_t ram_dma_base;      /* RAM start expressed as dma address */
  dma_addr_t ram_dma_cursor;    /* cursor for next dma to issue start */
#ifdef PEXOR_WITH_SFP
  struct pexor_sfp sfp;         /* contains registers of sfp engine */
#endif
#ifdef PEXOR_WITH_TRIXOR
    u32* trix_fcti; /* fast clear acceptance time register */
    u32* trix_cvti; /* conversion time register */
#endif
#ifdef PEXOR_WITH_TRBNET
  u32* trbnet_sender_err[PEXOR_TRB_CHANS];
  u32* trbnet_sender_data[PEXOR_TRB_CHANS];
  u32* trbnet_sender_ctl[PEXOR_TRB_CHANS];
 /* u32* trbnet_dma_ctl[PEXOR_TRB_CHANS];
  u32* trbnet_dma_add[PEXOR_TRB_CHANS];
  u32* trbnet_dma_len[PEXOR_TRB_CHANS];*/
#endif
    unsigned char init_done; /* object is ready flag*/
};





struct pexor_dmabuf
{
  struct list_head queue_list;  /* linked into free or receive queue list */
   unsigned long virt_addr;      /* user space virtual address (=buffer id) */
  unsigned long size;           /* buffer size in bytes */
  /* the following members are used for kernel buffers only:*/
  dma_addr_t dma_addr;          /* dma engine (pci) address*/
  unsigned long kernel_addr;    /* mapped kernel address  */
  /* the following members are used for userspace  sg buffers only:*/
  struct scatterlist* sg;				 /* optional for sg user buffer*/
  unsigned int sg_ents;			/* actual entries in the scatter/gatter list (NOT nents for the map function, but the result) */
  struct page **pages;		/* list of pointers to the pages */
  int num_pages;				 /* number of pages for this user memory area*/
};



struct pexor_privdata
{
  int magic;                    /* magic number to identify irq */
  atomic_t state;               /* run state of device */
  dev_t devno;                  /* device number (major and minor) */
  int devid;                    /* local id (counter number) */
  u8 board_type;                /* pexor, pexaria, kinpex, ...*/
  char irqname[64];             /* private name for irq */
  struct pci_dev *pdev;         /* PCI device */
  struct device *class_dev;     /* Class device */
  struct cdev cdev;             /* char device struct */
  struct dev_pexor pexor;       /* mapped pexor address pointers */
  unsigned long bases[6];       /* contains pci resource bases */
  unsigned long reglen[6];      /* contains pci resource length */
  void *iomem[6];               /* points to mapped io memory of the bars */
  struct semaphore ramsem;      /* protects read/write access to mapped ram */
  struct list_head free_buffers;        /* list containing the free buffers */
  struct list_head received_buffers;    /* dma receive queue */
  struct list_head used_buffers;        /* list containing the buffers in
                                           use in client application */

  spinlock_t buffers_lock;      /* protect any buffer lists operations */
  spinlock_t dma_lock;		/* protects DMA Buffer */

  atomic_t irq_count;           /* counter for irqs */
  spinlock_t irq_lock;         /* optional lock between top and bottom half? */
  struct tasklet_struct irq_bottomhalf; /* tasklet structure for isr
                                           bottom half */
  wait_queue_head_t irq_dma_queue;      /* wait queue between bottom
                                           half and wait dma ioctl */
  atomic_t dma_outstanding;     /* outstanding dma counter */
  wait_queue_head_t irq_trig_queue;     /* wait queue between bottom half
                                           and user wait trigger ioctl */
  atomic_t trig_outstanding;    /* outstanding triggers counter */
};


/* helper to access private data in file struct. returns 0 if pexor
 * not initialized */
struct pexor_privdata *get_privdata(struct file *fil);

/* create dma buffer of size, allocate it for pci device pdev,
 * returns buffer descriptor structure
 * Optionally, we may use external physical memory as pointed by pgoff*/
struct pexor_dmabuf *new_dmabuffer(struct pci_dev *pdev, size_t size, unsigned long pgoff);


/* remove dmabuffer from pci device */
int delete_dmabuffer(struct pci_dev *pdev, struct pexor_dmabuf *buf);

/* unmap the sglistst for dmabuffer*/
int unmap_sg_dmabuffer(struct pci_dev *pdev, struct pexor_dmabuf *buf);


/* print address and value of a register to dmesg debug output */
void print_register(const char *description, u32 * address);

void print_pexor(struct dev_pexor *pg);
void clear_pexor(struct dev_pexor *pg);
void set_pexor(struct dev_pexor *pg, void *base, unsigned long bar);



int pexor_open(struct inode *inode, struct file *filp);
int pexor_release(struct inode *inode, struct file *filp);
loff_t pexor_llseek(struct file *filp, loff_t off, int whence);
ssize_t pexor_read(struct file *filp, char __user * buf, size_t count,
                   loff_t * f_pos);
ssize_t pexor_write(struct file *filp, const char __user * buf, size_t count,
                    loff_t * f_pos);

int pexor_mmap(struct file *filp, struct vm_area_struct *vma);

/* the general fops ioctl */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int pexor_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else
long pexor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif

/* general reset function */
int pexor_ioctl_reset(struct pexor_privdata *priv, unsigned long arg);



/* map existing user buffer into sglist and register in driver pool*/
int pexor_ioctl_mapbuffer(struct pexor_privdata *priv, unsigned long arg);


/* unmap user buffer sglist and remove from driver pool*/
int pexor_ioctl_unmapbuffer(struct pexor_privdata *priv, unsigned long arg);


/* free dma buffer from usage (put back to free list) */
int pexor_ioctl_freebuffer(struct pexor_privdata *priv, unsigned long arg);

/* take (acquire) a dma buffer for usage in application space
 * (take from free list)*/
int pexor_ioctl_usebuffer(struct pexor_privdata *priv, unsigned long arg);


/* empty remaining buffers in receive queue and put back to free list */
int pexor_ioctl_clearreceivebuffers(struct pexor_privdata *priv,
                                    unsigned long arg);

/* delete dma buffer from pool */
int pexor_ioctl_deletebuffer(struct pexor_privdata *priv, unsigned long arg);

/* get next filled dma buffer (descriptor pointer) from receive queue.
 * Will wait for dma complete interrupt if receive queue is empty on calling */
int pexor_ioctl_waitreceive(struct pexor_privdata *priv, unsigned long arg);

/* switch internal run state of device (e.g. start/stop daq)
 * when daq is started, driver will receive dma buffers
 * and put them into receive queue, etc */
int pexor_ioctl_setrunstate(struct pexor_privdata *priv, unsigned long arg);

/* this one may be used for different tests with the device on kernel level */
int pexor_ioctl_test(struct pexor_privdata *priv, unsigned long arg);


/* Write a value to an address at the "bus" connected via the optical links
 * address and value are passed via pexor_bus_io structure */
int pexor_ioctl_write_bus(struct pexor_privdata *priv, unsigned long arg);

/* Read a value from an address at the "bus" connected via the optical links
 * address and value are passed via pexor_bus_io structure */
int pexor_ioctl_read_bus(struct pexor_privdata *priv, unsigned long arg);

/* Initialize devices on the "bus" connected via the optical links
 * pexor_bus_io structure may specify which channel and device to init */
int pexor_ioctl_init_bus(struct pexor_privdata *priv, unsigned long arg);

/* Write a value to a register on the board, mapped to a PCI BAR.
 * address, value and optionally the BAR are passed
 * via pexor_reg_io structure */
int pexor_ioctl_write_register(struct pexor_privdata *priv,
                               unsigned long arg);

/* Read a value from a register on the board, mapped to a PCI BAR.
 * address, value and optionally the BAR are passed
 * via pexor_reg_io structure */
int pexor_ioctl_read_register(struct pexor_privdata *priv, unsigned long arg);





irqreturn_t pexor_isr(int irq, void *dev_id);


void pexor_irq_tasklet(unsigned long);

/* set next receive buffer and start dma engine.
 * if source address is 0, we use pexor RAM area as start
 * roffset is dma startpoint relative to source (read offset)
 * woffset is optional write offset relative to target buffer
 * dmasize gives bytes to transfer by dma;
 * bufid is optional buffer id (i.e. user virtual address pointer) of buffer to fill
 * if 0, we use complete size of allocated dma buffer
 * Function may decide upon buffer type if we use plain dma or sg dma to user buffer
 * channelmask is used to initiate direct dma while reading token requested sfp data
 * bit i of channelmask (1...4) will decide sfp (i-1) will send data to dma buffer*/
int pexor_next_dma(struct pexor_privdata *priv, dma_addr_t source,
                   u32 roffset, u32 woffset, u32 dmasize, unsigned long bufid, u32 channelmask);

/* start dma engine to transfer dmasize bytes from source to dest.
 * Will not block until transfer is complete
 * if firstchunk is true, we may correct parameters internally to match burstsize>=8  modulos
 * if channelmask >0, instead of starting dma immediately the mask defines which sfp channel may write
 *         data on next token request to the dest address with direct dma*/
int pexor_start_dma(struct pexor_privdata *priv, dma_addr_t source, dma_addr_t dest,
					u32 dmasize, int firstchunk, u32 channelmask);

/* poll the dma register complete bit.
   returns error if loop exceeds certain cycle number */
int pexor_poll_dma_complete(struct pexor_privdata *priv);


/* wait for next received dma read buffer.
 * result is pointer to structure on the buffer
 * after return, buffer is already in user buffer list
 * Return value 0 on success; it may pass on error number */
int pexor_wait_dma_buffer(struct pexor_privdata *priv,
                          struct pexor_dmabuf *result);

/* general cleanup function*/
void cleanup_device(struct pexor_privdata *priv);

/* remove all dma buffers etc.*/
void cleanup_buffers(struct pexor_privdata *priv);


/* we support sysfs class only for new kernels to avoid
   backward incompatibilities here */
#ifdef PEXOR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

ssize_t pexor_sysfs_freebuffers_show(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf);
ssize_t pexor_sysfs_usedbuffers_show(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf);
ssize_t pexor_sysfs_rcvbuffers_show(struct device *dev,
                                    struct device_attribute *attr, char *buf);

ssize_t pexor_sysfs_codeversion_show(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf);
ssize_t pexor_sysfs_dmaregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);



#endif
#endif

#endif
