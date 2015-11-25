/*
 * \file
 * pexornet.h
 *
 *  Created on: 08.02.2011 - 23.11.2015
 *      Author: J. Adamczewski-Musch, GSI
 *
 *      PEXORNET driver common includes for all modules
 */

#ifndef _PEXORNET__H_
#define _PEXORNET__H_

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

#include <linux/wait.h>

#include <linux/in.h>
#include <linux/netdevice.h>   /* struct device, and other headers */
#include <linux/etherdevice.h> /* eth_type_trans */
#include <linux/ip.h>          /* struct iphdr */
#include <linux/tcp.h>         /* struct tcphdr */
#include <linux/skbuff.h>


#include "pexornet_user.h"

/* ids for pexornet card:*/
#define PEXOR_VENDOR_ID 0x1204
#define PEXOR_DEVICE_ID 0x5303

#define PEXARIA_VENDOR_ID     0x1172
#define PEXARIA_DEVICE_ID     0x1111

#define KINPEX_VENDOR_ID     0x10EE
#define KINPEX_DEVICE_ID     0x1111

#define BOARDTYPE_PEXOR 0
#define BOARDTYPE_PEXARIA 1
#define BOARDTYPE_KINPEX 2


#define PEXORNET_DEBUGPRINT 1


#define PEXORNET_ENABLE_IRQ 1

#define PEXORNET_SYSFS_ENABLE 1

/** enable usage of SFP */
#define PEXORNET_WITH_SFP 1

/** enable usage of TRIXOR */
#define PEXORNET_WITH_TRIXOR 1



#ifdef PEXORNET_DEBUGPRINT
#define pexornet_dbg( args... )                    \
  printk( args );
#else
#define pexornet_dbg( args... ) ;
#endif


#define pexornet_msg( args... )                    \
  printk( args );


struct pexornet_privdata;

#ifdef PEXORNET_WITH_SFP
struct pexornet_sfp;
/** here include specific optical protocol definitions*/
#include "_pexornet_gosip.h"
#endif




/****************************************************************************************/
/*** pexornet base par :*/


#define PEXORNET_SHARED_IRQ 1


/** use disable_irq_nosync and enable_irq in isr.*/
#define PEXORNET_DISABLE_IRQ_ISR 1



/** test: use spinlock to protect dma engine vs buffers. do we need this?
 * rather bad idea*/
/*#define DMA_SPINLOCK 1 */




/* switch on message signalled interrupt mode. Supported by PEXORNET? maybe not*/
//#define IRQ_ENABLE_MSI 1


/* use streaming dma mapping, i.e. dma buffers are allocated with kmalloc and
 * then mapped for dma. Otherwise, we use coherent mapping
 * with pci_alloc_consistent
 * which takes coherent dma buffers from preallocated (?)  pci device memory*/
/*#define DMA_MAPPING_STREAMING 1*/






#ifdef DMA_SPINLOCK
#define pexornet_dma_lock(lock) \
  spin_lock( lock);


#define pexornet_dma_unlock(lock) \
  spin_unlock( lock );


#else
#define pexornet_dma_lock(lock) \
  ;

#define pexornet_dma_unlock(lock) \
  ;
#endif

/** maximum number of devices controlled by this driver*/
#define PEXORNET_MAXDEVS 4

/** default timeout for trigger and wait queues, in seconds */
#define PEXORNET_WAIT_TIMEOUT 1
//(1*HZ)

/** timeout for trigger wait queue */
//#define PEXORNET_TRIG_TIMEOUT (10*HZ)

/** maximum number of timeouts before wait loop terminates*/
//#define PEXORNET_WAIT_MAXTIMEOUTS 5

/** maximum number of polling cycles for dma complete bit*/
#define PEXORNET_DMA_MAXPOLLS 10000

/** polling delay for each cycle in ns for dma complete bit*/
#define PEXORNET_DMA_POLLDELAY 20

/** if set, we use a schedule() in the dma complete polling.
 * Note: according to linux kernel book, yield() will just prepare this
 * task to be scheduled in near future, but schedpriv->pexornet.irq_statusule() will initiate the
 * schedule directly
 * this must not be enabled if dma completion is polled in interrupt tasklet*/
//#define PEXORNET_DMA_POLL_SCHEDULE 0

/** maximum number of outstandin buffers in receive queue,
   do we still need this?*/
#define PEXORNET_MAXOUTSTANDING 50



/** size of interrupt status ringbuffer
 * actually we only need one buffer for mbs like
 * user readout mode! */
#define PEXORNET_IRSTATBUFFER_SIZE 10

#define PEXORNET_BUS_DELAY 20

/** on some PCs, need maybe waitstates for simple bus io:*/
#define pexornet_bus_delay()                       \
  mb();      \
  ndelay(PEXORNET_BUS_DELAY);



#define PEXORNET_DEFAULTBUFFERNUM 100


struct dev_pexornet
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
#ifdef PEXORNET_WITH_SFP
  struct pexornet_sfp sfp;         /**< contains registers of sfp engine */
#endif
#ifdef PEXORNET_WITH_TRIXOR
    u32* trix_fcti; /**< fast clear acceptance time register */
    u32* trix_cvti; /**< conversion time register */
#endif

    unsigned char init_done; /**< object is ready flag*/
};





struct pexornet_dmabuf
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
  struct scatterlist* sg;                /**< optional for sg user buffer*/
  unsigned int sg_ents;         /**< actual entries in the scatter/gatter list (NOT nents for the map function, but the result) */
  struct page **pages;      /**< list of pointers to the pages */
  int num_pages;                 /**< number of pages for this user memory area*/
};


struct pexornet_trigger_buf
{
  struct list_head queue_list;    /**< linked into queue list */
  u32 trixorstat;      /**< trixor status register related at trigger interrupt  time*/

};

struct pexornet_privdata
{
  atomic_t state;               /**< run state of device */
//  int devid;                    /**< local id (counter number) */
  u8 board_type;                /**< pexornet, pexaria, kinpex, ...*/
  char irqname[64];             /**< private name for irq */
  struct pci_dev *pdev;         /**< PCI device */
  struct device *class_dev;     /**< Class device */
  struct net_device *net_dev;   /**< Network device */
  struct net_device_stats stats; /**< network statistics info */
  struct dev_pexornet registers;       /**< mapped pexornet address pointers */
  u32 sfp_maxpolls; /**< number of retries when polling for sfp response ready */
  u32 sfp_buswait; /**< sfp bus waitstates in ns for each bus read/write ioctl. To adjust for frontend slaves speed */
  unsigned long bases[6];       /**< contains pci resource bases */
  unsigned long reglen[6];      /**< contains pci resource length */
  void *iomem[6];               /**< points to mapped io memory of the bars */
  struct semaphore ioctl_sem;      /**< protects multi user ioctl access */
  struct list_head free_buffers;        /**< list containing the free buffers */
  struct list_head received_buffers;    /**< dma receive queue */
  struct list_head used_buffers;        /**< list containing the buffers in
                                           use in client application */

  spinlock_t buffers_lock;      /**< protect any buffer lists operations */
  spinlock_t dma_lock;      /**< protects DMA Buffer */
  spinlock_t trigstat_lock;       /**< protects trigger status queue */

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
  struct list_head trig_status; /**< list (queue) of trigger status words corresponding to interrupts*/
  unsigned int wait_timeout; /**< configurable wait timeout for trigger and dma buffer queues. in seconds */
};

/** the private data of the network device contains reference to privdata of pci device*/
struct pexornet_netdev_privdata
{
  struct pexornet_privdata* pci_privdata; /**< reference to external private data structure */
};


/** helper to access private data in net_device struct. returns 0 if pexornet
 * not initialized */
struct pexornet_privdata *pexornet_get_privdata(struct net_device *netdev);

/** create dma buffer of size, allocate it for pci device pdev,
 * returns buffer descriptor structure
 * Optionally, we may use external physical memory as pointed by pgoff*/
struct pexornet_dmabuf *new_dmabuffer(struct pci_dev *pdev, size_t size, unsigned long pgoff);

/** preliminary helper for isr to give back buffer after copying contents to skb.
 * To be replaced by dma to skb later.*/
int pexornet_freebuffer (struct pexornet_privdata* priv, struct pexornet_dmabuf* tofree);


/** remove dmabuffer from pci device */
int delete_dmabuffer(struct pci_dev *pdev, struct pexornet_dmabuf *buf);

/** unmap the sglistst for dmabuffer*/
int unmap_sg_dmabuffer(struct pci_dev *pdev, struct pexornet_dmabuf *buf);


/** print address and value of a register to dmesg debug output */
void print_register(const char *description, u32 * address);

void print_pexornet(struct dev_pexornet *pg);
void clear_pexornet(struct dev_pexornet *pg);
void set_pexornet(struct dev_pexornet *pg, void *base, unsigned long bar);



//int pexornet_open(struct inode *inode, struct file *filp);
//int pexornet_release(struct inode *inode, struct file *filp);
//loff_t pexornet_llseek(struct file *filp, loff_t off, int whence);
//ssize_t pexornet_read(struct file *filp, char __user * buf, size_t count,
//                   loff_t * f_pos);
//ssize_t pexornet_write(struct file *filp, const char __user * buf, size_t count,
//                    loff_t * f_pos);
//
//int pexornet_mmap(struct file *filp, struct vm_area_struct *vma);

/** the general fops ioctl */
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
//int pexornet_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg);
//#else
//long pexornet_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
//#endif

/** ioctl hook via network device:*/
int pexornet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);


/** general reset function */
int pexornet_ioctl_reset(struct pexornet_privdata *priv, unsigned long arg);



///** map existing user buffer into sglist and register in driver pool*/
//int pexornet_ioctl_mapbuffer(struct pexornet_privdata *priv, unsigned long arg);
//
//
///** unmap user buffer sglist and remove from driver pool*/
//int pexornet_ioctl_unmapbuffer(struct pexornet_privdata *priv, unsigned long arg);
//
//
///** free dma buffer from usage (put back to free list) */
//int pexornet_ioctl_freebuffer(struct pexornet_privdata *priv, unsigned long arg);
//
///** take (acquire) a dma buffer for usage in application space
// * (take from free list)*/
//int pexornet_ioctl_usebuffer(struct pexornet_privdata *priv, unsigned long arg);
//
//

///** empty remaining buffers in receive queue and put back to free list */
//int pexornet_ioctl_clearreceivebuffers(struct pexornet_privdata *priv,
//                                    unsigned long arg);

//
///** delete dma buffer from pool */
//int pexornet_ioctl_deletebuffer(struct pexornet_privdata *priv, unsigned long arg);
//
///** get next filled dma buffer (descriptor pointer) from receive queue.
// * Will wait for dma complete interrupt if receive queue is empty on calling */
//int pexornet_ioctl_waitreceive(struct pexornet_privdata *priv, unsigned long arg);

///** switch internal run state of device (e.g. start/stop daq)
// * when daq is started, driver will receive dma buffers
// * and put them into receive queue, etc */
//int pexornet_ioctl_setrunstate(struct pexornet_privdata *priv, unsigned long arg);
//
///** this one may be used for different tests with the device on kernel level */
//int pexornet_ioctl_test(struct pexornet_privdata *priv, unsigned long arg);


/** Write a value to an address at the "bus" connected via the optical links
 * address and value are passed via pexornet_bus_io structure */
int pexornet_ioctl_write_bus(struct pexornet_privdata *priv, unsigned long arg);

/** Read a value from an address at the "bus" connected via the optical links
 * address and value are passed via pexornet_bus_io structure */
int pexornet_ioctl_read_bus(struct pexornet_privdata *priv, unsigned long arg);

/** Initialize devices on the "bus" connected via the optical links
 * pexornet_bus_io structure may specify which channel and device to init */
int pexornet_ioctl_init_bus(struct pexornet_privdata *priv, unsigned long arg);

///** Write a value to a register on the board, mapped to a PCI BAR.
// * address, value and optionally the BAR are passed
// * via pexornet_reg_io structure */
//int pexornet_ioctl_write_register(struct pexornet_privdata *priv,
//                               unsigned long arg);
//
///** Read a value from a register on the board, mapped to a PCI BAR.
// * address, value and optionally the BAR are passed
// * via pexornet_reg_io structure */
//int pexornet_ioctl_read_register(struct pexornet_privdata *priv, unsigned long arg);


///** change timeout in waitqueues for trigger or dma buffers.
// * argument specifies timeout in seconds. */
//int pexornet_ioctl_set_wait_timeout(struct pexornet_privdata* priv, unsigned long arg);


#ifdef PEXORNET_WITH_TRIXOR
/** set acquisition state of trixor trigger module extension.
 * used to clear deadtime flag from user program and start/stop acquisition mode, etc.*/
int pexornet_ioctl_set_trixor(struct pexornet_privdata* priv, unsigned long arg);


/** decode trixor status word into trigger status structure*/
void pexornet_decode_triggerstatus(u32 trixorstat, struct pexornet_trigger_status* result);


/** reset trigger state after reading out corresponding data*/
int pexornet_trigger_reset(struct pexornet_privdata* priv);

/** initiate start acquisition*/
int pexornet_trigger_start_acq(struct pexornet_privdata* priv);

/** initiate stop acquisition */
int pexornet_trigger_stop_acq(struct pexornet_privdata* priv);

/** perform stop acquisition action. This is done after software trigger 15 is received.*/
int pexornet_trigger_do_stop(struct pexornet_privdata* priv);

#endif //PEXORNET_WITH_TRIXOR



/** the top half interrupt service routine.
 * This is invoked by trigger interrupts from trixor
 * Depending on daq mode, will either schedule bottom half tasklet for automatic readout,
 * or just put trigger status to queue and wakes up consumer in userland for explicit readout by ioctl calls.*/
irqreturn_t pexornet_isr(int irq, void *dev_id);

/** The bottom half interrupt service routine.
 * Implements automatic direct dma token request from all configured sfps.
 * Data DMA buffer is put in the receive queue. Waiting consumer in userland is woken up
 * to receive it. */
void pexornet_irq_tasklet(unsigned long);

/** set next receive buffer and start dma engine.
 * if source address is 0, we use pexornet RAM area as start
 * roffset is dma startpoint relative to source (read offset)
 * woffset is optional write offset relative to target buffer
 * dmasize gives bytes to transfer by dma;
 * bufid is optional buffer id (i.e. user virtual address pointer) of buffer to fill
 * if 0, we use complete size of allocated dma buffer
 * Function may decide upon buffer type if we use plain dma or sg dma to user buffer
 * channelmask is used to initiate direct dma while reading token requested sfp data
 * bit i of channelmask (1...4) will decide sfp (i-1) will send data to dma buffer*/
int pexornet_next_dma(struct pexornet_privdata *priv, dma_addr_t source,
                   u32 roffset, u32 woffset, u32 dmasize, unsigned long bufid, u32 channelmask);

/** start dma engine to transfer dmasize bytes from source to dest.
 * Will not block until transfer is complete
 * if firstchunk is true, we may correct parameters internally to match burstsize>=8  modulos
 * if channelmask >0, instead of starting dma immediately the mask defines which sfp channel may write
 *         data on next token request to the dest address with direct dma*/
int pexornet_start_dma(struct pexornet_privdata *priv, dma_addr_t source, dma_addr_t dest,
                    u32 dmasize, int firstchunk, u32 channelmask);

/** poll the dma register complete bit.
   returns error if loop exceeds certain cycle number */
int pexornet_poll_dma_complete(struct pexornet_privdata *priv);


/** wait for next received dma read buffer.
 * result is pointer to structure on the buffer
 * after return, buffer is already in user buffer list
 * Return value 0 on success; it may pass on error number */
int pexornet_wait_dma_buffer(struct pexornet_privdata *priv,
                          struct pexornet_dmabuf *result);


/** poll for dma completion and move received buffer into receive queue.
 * Wake up consuming process (that should wait in call of pexornet_wait_dma_buffer)
 * Optionally after direct dma the used size may be set in receive buffer.
 * for automatic readout mode, triggerstatus can be appended to dma buffer structure*/
int pexornet_receive_dma_buffer(struct pexornet_privdata *priv, unsigned long used_size, u32 triggerstatus);


/** general cleanup function*/
void pexornet_cleanup_device(struct pexornet_privdata *priv);

/** remove all dma buffers etc.*/
void pexornet_cleanup_buffers(struct pexornet_privdata *priv);

/** construct _numbufs_ new dma buffers of size _bufsize_ and put them into the free list*/
void pexornet_build_buffers(struct pexornet_privdata *priv,size_t bufsize, unsigned int numbufs);

/*
 * Receive a packet: retrieve, encapsulate and pass over to upper levels
 * JAM this one is called from receive interrupt
 * prelliminary, later dmabuf will directly be pre-allocated skb
 */
void pexornet_rx(struct net_device *dev,  struct pexornet_dmabuf *pkt);


/** we support sysfs class only for new kernels to avoid
   backward incompatibilities here */
#ifdef PEXORNET_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

ssize_t pexornet_sysfs_freebuffers_show(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf);
ssize_t pexornet_sysfs_usedbuffers_show(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf);
ssize_t pexornet_sysfs_rcvbuffers_show(struct device *dev,
                                    struct device_attribute *attr, char *buf);

ssize_t pexornet_sysfs_codeversion_show(struct device *dev,
                                     struct device_attribute *attr,
                                     char *buf);
ssize_t pexornet_sysfs_dmaregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);

/* show number of retries Nr for sfp request until error is recognized.
 * this will cause request timeout = Nr * (20 ns + arbitrary schedule() switch time)  */
ssize_t pexornet_sysfs_sfp_retries_show (struct device *dev, struct device_attribute *attr, char *buf);

/* set number of retries for sfp request until error is recognized:*/
ssize_t pexornet_sysfs_sfp_retries_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);

/* show sfp bus read/write waitstate in microseconds.
 * this will impose such wait time after each frontend address read/write ioctl */
ssize_t pexornet_sysfs_buswait_show (struct device *dev, struct device_attribute *attr, char *buf);

/* set sfp bus read/write waitstate in microseconds. */
ssize_t pexornet_sysfs_buswait_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count);


#endif
#endif

#endif





