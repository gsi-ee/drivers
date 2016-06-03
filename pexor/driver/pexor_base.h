/* \file
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

#include "pexor_gos.h"


#define PEXOR_SHARED_IRQ 1


/** use disable_irq_nosync and enable_irq in isr.*/
#define PEXOR_DISABLE_IRQ_ISR 1


/** if this is set, use trigger status queue.
 * othewise, just atomic variable, since there can be only one trigger processed at same time*/
//#define PEXOR_TRIGSTAT_QUEUE 1


/** test: use spinlock to protect dma engine vs buffers. do we need this?
 * rather bad idea*/
/*#define DMA_SPINLOCK 1 */




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

/** maximum number of devices controlled by this driver*/
#define PEXOR_MAXDEVS 4

/** default timeout for trigger and wait queues, in seconds */
#define PEXOR_WAIT_TIMEOUT 1
//(1*HZ)

/** timeout for trigger wait queue */
//#define PEXOR_TRIG_TIMEOUT (10*HZ)

/** maximum number of timeouts before wait loop terminates*/
//#define PEXOR_WAIT_MAXTIMEOUTS 5

/** maximum number of polling cycles for dma complete bit*/
#define PEXOR_DMA_MAXPOLLS 10000

/** polling delay for each cycle in ns for dma complete bit*/
#define PEXOR_DMA_POLLDELAY 100
//20


/** if set, we use a schedule() in the dma complete polling.
 * Note: according to linux kernel book, yield() will just prepare this
 * task to be scheduled in near future, but schedule() will initiate the
 * schedule directly
 * this must not be enabled if dma completion is polled in interrupt tasklet
 * note that termination of ioctl during schedule might leave pexor inconsistent!*/
//#define PEXOR_DMA_POLL_SCHEDULE 1

/** maximum number of outstandin buffers in receive queue,
   do we still need this?*/
#define PEXOR_MAXOUTSTANDING 50



/** size of interrupt status ringbuffer
 * actually we only need one buffer for mbs like
 * user readout mode! */
#define PEXOR_IRSTATBUFFER_SIZE 10

#define PEXOR_BUS_DELAY 20

/** on some PCs, need maybe waitstates for simple bus io:*/
#define pexor_bus_delay()                       \
  mb();      \
  ndelay(PEXOR_BUS_DELAY);



struct dev_pexor
{

  u32 *irq_control;             /**< irq control register */
  u32 *irq_status;              /**< irq status register */
  u32 *dma_control_stat;        /**< dma control and statusregister */
  u32 *dma_source;              /**< dma source address */
  u32 *dma_dest;                /**< dma destination address */
  u32 *dma_len;                 /**< dma length */
  u32 *dma_burstsize;           /**< dma burstsize, <=0x80 */
  u32 *ram_start;               /**< RAM start */
  u32 *ram_end;                 /**< RAM end */
  dma_addr_t ram_dma_base;      /**< RAM start expressed as dma address */
  dma_addr_t ram_dma_cursor;    /**< cursor for next dma to issue start */
#ifdef PEXOR_WITH_SFP
  struct pexor_sfp sfp;         /**< contains registers of sfp engine */
#endif
#ifdef PEXOR_WITH_TRIXOR
    u32* trix_fcti; /**< fast clear acceptance time register */
    u32* trix_cvti; /**< conversion time register */
#endif

    unsigned char init_done; /**< object is ready flag*/
};





struct pexor_dmabuf
{
  struct list_head queue_list;  /**< linked into free or receive queue list */
  unsigned long virt_addr;      /**< user space virtual address (=buffer id) */
  unsigned long size;           /**< buffer size in bytes */
  unsigned long used_size;      /**< filled payload size*/
  u32 triggerstatus;            /**< optional triggerstatus for automatic readout mode */
  /* the following members are used for kernel buffers only:*/
  dma_addr_t dma_addr;          /**< dma engine (pci) address*/
  unsigned long kernel_addr;    /**< mapped kernel address  */
  /* the following members are used for userspace  sg buffers only:*/
  struct scatterlist* sg;				 /**< optional for sg user buffer*/
  unsigned int sg_ents;			/**< actual entries in the scatter/gatter list (NOT nents for the map function, but the result) */
  struct page **pages;		/**< list of pointers to the pages */
  int num_pages;				 /**< number of pages for this user memory area*/
};


struct pexor_trigger_buf
{
  struct list_head queue_list;    /**< linked into queue list */
  u32 trixorstat;      /**< trixor status register related at trigger interrupt  time*/

};

struct pexor_privdata
{
  atomic_t state;               /**< run state of device */
  dev_t devno;                  /**< device number (major and minor) */
  int devid;                    /**< local id (counter number) */
  u8 board_type;                /**< pexor, pexaria, kinpex, ...*/
  char irqname[64];             /**< private name for irq */
  struct pci_dev *pdev;         /**< PCI device */
  struct device *class_dev;     /**< Class device */
  struct cdev cdev;             /**< char device struct */
  struct dev_pexor pexor;       /**< mapped pexor address pointers */
  u32 sfp_maxpolls; /**< number of retries when polling for sfp response ready */
  u32 sfp_buswait; /**< sfp bus waitstates in ns for each bus read/write ioctl. To adjust for frontend slaves speed */
  unsigned long bases[6];       /**< contains pci resource bases */
  unsigned long reglen[6];      /**< contains pci resource length */
  void *iomem[6];               /**< points to mapped io memory of the bars */
  struct semaphore ramsem;      /**< protects read/write access to mapped ram */
  struct semaphore ioctl_sem;      /**< protects multi user ioctl access */
  struct list_head free_buffers;        /**< list containing the free buffers */
  struct list_head received_buffers;    /**< dma receive queue */
  struct list_head used_buffers;        /**< list containing the buffers in
                                           use in client application */

  spinlock_t buffers_lock;      /**< protect any buffer lists operations */
  spinlock_t dma_lock;		/**< protects DMA Buffer */


  atomic_t irq_count;           /**< counter for irqs */
  spinlock_t irq_lock;         /**< optional lock between top and bottom half? */
  struct tasklet_struct irq_bottomhalf; /**< tasklet structure for isr
                                           bottom half */
  atomic_t trigstat;           /**< current trixor status for auto readout mode. Complementary to trigger queue! */

  wait_queue_head_t irq_dma_queue;      /**< wait queue between bottom
                                           half and wait dma ioctl */
  atomic_t dma_outstanding;     /**< outstanding dma counter */
  wait_queue_head_t irq_trig_queue;     /**< wait queue between bottom half
                                           and user wait trigger ioctl */
  atomic_t trig_outstanding;    /**< outstanding triggers counter */
#ifdef PEXOR_TRIGSTAT_QUEUE
  struct list_head trig_status; /**< list (queue) of trigger status words corresponding to interrupts*/
  spinlock_t trigstat_lock;       /**< protects trigger status queue */
#endif
  unsigned int wait_timeout; /**< configurable wait timeout for trigger and dma buffer queues. in seconds */
};


/** helper to access private data in file struct. returns 0 if pexor
 * not initialized */
struct pexor_privdata *get_privdata(struct file *fil);

/** create dma buffer of size, allocate it for pci device pdev,
 * returns buffer descriptor structure
 * Optionally, we may use external physical memory as pointed by pgoff*/
struct pexor_dmabuf *new_dmabuffer(struct pci_dev *pdev, size_t size, unsigned long pgoff);


/** remove dmabuffer from pci device */
int delete_dmabuffer(struct pci_dev *pdev, struct pexor_dmabuf *buf);

/** unmap the sglistst for dmabuffer*/
int unmap_sg_dmabuffer(struct pci_dev *pdev, struct pexor_dmabuf *buf);


/** print address and value of a register to dmesg debug output */
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

/** the general fops ioctl */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int pexor_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
#else
long pexor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
#endif

/** general reset function */
int pexor_ioctl_reset(struct pexor_privdata *priv, unsigned long arg);



/** map existing user buffer into sglist and register in driver pool*/
int pexor_ioctl_mapbuffer(struct pexor_privdata *priv, unsigned long arg);


/** unmap user buffer sglist and remove from driver pool*/
int pexor_ioctl_unmapbuffer(struct pexor_privdata *priv, unsigned long arg);


/** free dma buffer from usage (put back to free list) */
int pexor_ioctl_freebuffer(struct pexor_privdata *priv, unsigned long arg);

/** take (acquire) a dma buffer for usage in application space
 * (take from free list)*/
int pexor_ioctl_usebuffer(struct pexor_privdata *priv, unsigned long arg);


/** empty remaining buffers in receive queue and put back to free list */
int pexor_ioctl_clearreceivebuffers(struct pexor_privdata *priv,
                                    unsigned long arg);

/** delete dma buffer from pool */
int pexor_ioctl_deletebuffer(struct pexor_privdata *priv, unsigned long arg);

/** get next filled dma buffer (descriptor pointer) from receive queue.
 * Will wait for dma complete interrupt if receive queue is empty on calling */
int pexor_ioctl_waitreceive(struct pexor_privdata *priv, unsigned long arg);

/** switch internal run state of device (e.g. start/stop daq)
 * when daq is started, driver will receive dma buffers
 * and put them into receive queue, etc */
int pexor_ioctl_setrunstate(struct pexor_privdata *priv, unsigned long arg);

/** this one may be used for different tests with the device on kernel level */
int pexor_ioctl_test(struct pexor_privdata *priv, unsigned long arg);


/** Write a value to an address at the "bus" connected via the optical links
 * address and value are passed via pexor_bus_io structure */
int pexor_ioctl_write_bus(struct pexor_privdata *priv, unsigned long arg);

/** Read a value from an address at the "bus" connected via the optical links
 * address and value are passed via pexor_bus_io structure */
int pexor_ioctl_read_bus(struct pexor_privdata *priv, unsigned long arg);

/** Initialize devices on the "bus" connected via the optical links
 * pexor_bus_io structure may specify which channel and device to init */
int pexor_ioctl_init_bus(struct pexor_privdata *priv, unsigned long arg);

/** Write a value to a register on the board, mapped to a PCI BAR.
 * address, value and optionally the BAR are passed
 * via pexor_reg_io structure */
int pexor_ioctl_write_register(struct pexor_privdata *priv,
                               unsigned long arg);

/** Read a value from a register on the board, mapped to a PCI BAR.
 * address, value and optionally the BAR are passed
 * via pexor_reg_io structure */
int pexor_ioctl_read_register(struct pexor_privdata *priv, unsigned long arg);


/** change timeout in waitqueues for trigger or dma buffers.
 * argument specifies timeout in seconds. */
int pexor_ioctl_set_wait_timeout(struct pexor_privdata* priv, unsigned long arg);

/** the top half interrupt service routine.
 * This is invoked by trigger interrupts from trixor
 * Depending on daq mode, will either schedule bottom half tasklet for automatic readout,
 * or just put trigger status to queue and wakes up consumer in userland for explicit readout by ioctl calls.*/
irqreturn_t pexor_isr(int irq, void *dev_id);

/** The bottom half interrupt service routine.
 * Implements automatic direct dma token request from all configured sfps.
 * Data DMA buffer is put in the receive queue. Waiting consumer in userland is woken up
 * to receive it. */
void pexor_irq_tasklet(unsigned long);

/** set next receive buffer and start dma engine.
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
                   u32 roffset, u32 woffset, u32 dmasize, unsigned long *bufid, u32 channelmask, u32 burstsize);

/** start dma engine to transfer dmasize bytes from source to dest.
 * Will not block until transfer is complete
 * if firstchunk is true, we may correct parameters internally to match burstsize>=8  modulos
 * if channelmask >0, instead of starting dma immediately the mask defines which sfp channel may write
 *         data on next token request to the dest address with direct dma*/
int pexor_start_dma(struct pexor_privdata *priv, dma_addr_t source, dma_addr_t dest,
					u32 dmasize, int firstchunk, u32 channelmask, u32 burstsize);

/** poll the dma register complete bit.
   returns error if loop exceeds certain cycle number
   flag doschedule will switch if polling shall be done with schedule()
   which is forbiden for automatic readout in ir tasklet*/
int pexor_poll_dma_complete(struct pexor_privdata *priv, int doschedule);


/** wait for next received dma read buffer.
 * result is pointer to structure on the buffer
 * after return, buffer is already in user buffer list
 * Return value 0 on success; it may pass on error number */
int pexor_wait_dma_buffer(struct pexor_privdata *priv,
                          struct pexor_dmabuf *result);


/** poll for dma completion and move received buffer into receive queue.
 * Wake up consuming process (that should wait in call of pexor_wait_dma_buffer)
 * Optionally after direct dma the used size may be set in receive buffer.
 * for automatic readout mode, triggerstatus can be appended to dma buffer structure*/
int pexor_receive_dma_buffer(struct pexor_privdata *priv, unsigned long used_size, u32 triggerstatus);



/** evaluate optimum burst size for dma for given dma size dmasize.
 * dmasize might be increased to fulfill the burst alignment.
 * New values will be set in pointer argument contents*/
void pexor_eval_dma_size(u32* dmasize, u32* dmaburst);


/** general cleanup function*/
void cleanup_device(struct pexor_privdata *priv);

/** remove all dma buffers etc.*/
void cleanup_buffers(struct pexor_privdata *priv);


/** we support sysfs class only for new kernels to avoid
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




/** following functions can be used to tune the trixor fast clear and conversion time registers without
 * using ioctl calls:*/

ssize_t pexor_sysfs_trixorregs_show (struct device *dev, struct device_attribute *attr, char *buf);


ssize_t pexor_sysfs_trixor_fctime_show (struct device *dev, struct device_attribute *attr, char *buf);

ssize_t pexor_sysfs_trixor_fctime_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);


ssize_t pexor_sysfs_trixor_cvtime_show (struct device *dev, struct device_attribute *attr, char *buf);

ssize_t pexor_sysfs_trixor_cvtime_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);








/* show number of retries Nr for sfp request until error is recognized.
 * this will cause request timeout = Nr * (20 ns + arbitrary schedule() switch time)  */
ssize_t pexor_sysfs_sfp_retries_show (struct device *dev, struct device_attribute *attr, char *buf);

/* set number of retries for sfp request until error is recognized:*/
ssize_t pexor_sysfs_sfp_retries_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

/* show sfp bus read/write waitstate in microseconds.
 * this will impose such wait time after each frontend address read/write ioctl */
ssize_t pexor_sysfs_buswait_show (struct device *dev, struct device_attribute *attr, char *buf);

/* set sfp bus read/write waitstate in microseconds. */
ssize_t pexor_sysfs_buswait_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);


#endif
#endif

#endif
