/*
 * pexor_test.h
 *
 *  Created on: 01.12.2009
 *      Author: J. Adamczewski-Musch, GSI
 *
 *      PEXOR driver internal declarations
 */

#ifndef _PCI_PEXOR_H_
#define _PCI_PEXOR_H_

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

#include <linux/wait.h>
#include <asm/uaccess.h>
#include <asm/atomic.h>
#include <asm/io.h>
#include <asm/system.h>

#include "pexor_user.h"

/*#include "pexor1_defs.h"*/

#include "pexor2_defs.h"

#define PEXOR_SYSFS_ENABLE 1

/*#define PEXOR_DEBUGPRINT 1*/
//
//
#define PEXOR_ENABLE_IRQ 1
//
#define PEXOR_SHARED_IRQ 1


/* modes of interrupt complete handling:*/

/* polling mode in the wait ioctl. if not set, we wait on the event queue
 * of the tasklet which is executed by ir. */
#define DMA_WAITPOLLING 1

/* test: use spinlock to protect dma engine vs buffers. do we need this?*/
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


#ifdef PEXOR_DEBUGPRINT
#define pexor_dbg( args... )                    \
  printk( args );
#else
#define pexor_dbg( args... ) ;
#endif


#define pexor_msg( args... )                    \
  printk( args );


#define pexor_sfp_assert_channel(ch)                                    \
  if(ch < 0 || ch >= PEXOR_SFP_NUMBER)                                  \
    {                                                                   \
      pexor_msg(KERN_WARNING "*** channel %d out of range [%d,%d]! \n", \
                ch, 0 , PEXOR_SFP_NUMBER-1);                            \
      return -EFAULT;                                                   \
    }


#define pexor_sfp_delay()                       \
  mb();                                         \
  ndelay(20);
/*udelay(10);*/


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
 * task to be scheduled in near future, but schedule() will initiate the
 * schedule directly*/
#define PEXOR_DMA_POLL_SCHEDULE 0

/* maximum number of outstandin buffers in receive queue,
   do we still need this?*/
#define PEXOR_MAXOUTSTANDING 50



#ifdef PEXOR_WITH_SFP
/* this structure contains pointers to sfp registers:*/
struct pexor_sfp
{
  u32 *version;                 /* Program date and version */
  u32 *req_comm;                /* Request command */
  u32 *req_addr;                /* Request address */
  u32 *req_data;                /* Request data */
  u32 *rep_stat_clr;            /* Reply status flags and clear for all sfp */
  u32 *rep_stat[PEXOR_SFP_NUMBER];      /* Reply status for sfp 0...3 */
  u32 *rep_addr[PEXOR_SFP_NUMBER];      /* Reply adresses for sfp 0...3 */
  u32 *rep_data[PEXOR_SFP_NUMBER];      /* Reply data for sfp 0...3 */
  u32 *rx_moni;                 /* Receive monitor */
  u32 *tx_stat;                 /* Transmit status */
  u32 *reset;                   /* rx/tx reset */
  u32 *disable;                 /* disable sfps */
  u32 *fault;                   /* fault flags */
  u32 *fifo[PEXOR_SFP_NUMBER];  /* debug access to fifos of sfp 0...3 */
  u32 *tk_stat[PEXOR_SFP_NUMBER];       /* token reply status of sfp 0...3 */
  u32 *tk_head[PEXOR_SFP_NUMBER];       /* token reply header of sfp 0...3 */
  u32 *tk_foot[PEXOR_SFP_NUMBER];       /* token reply footer of sfp 0...3 */
  u32 *tk_dsize[PEXOR_SFP_NUMBER];      /* token datasize(byte) of sfp 0...3 */
  u32 *tk_dsize_sel[PEXOR_SFP_NUMBER];  /* selects slave module ID in sfp 0...3
                                           for reading token datasize */
  u32 *tk_memsize[PEXOR_SFP_NUMBER];    /* memory size filled by token 
                                           data transfer for sfp 0...3 */
  u32 *tk_mem[PEXOR_SFP_NUMBER];        /* memory area filled by token 
                                           data transfer for sfp 0...3 */
  dma_addr_t tk_mem_dma[PEXOR_SFP_NUMBER];  /* token data memory area 
                                               expressed as dma bus address */
};
#endif


struct dev_pexor
{

  u32 *irq_control;             /* irq control register */
  u32 *irq_status;              /* irq status register */
  u32 *dma_control_stat;        /* dma control and statusregister */
  u32 *dma_source;              /* dma source address */
  u32 *dma_dest;                /* dma destination address */
  u32 *dma_len;                 /* dma length */
  u32 *dma_burstsize;           /* dma burstsize, <=0x80 */
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
    unsigned char init_done; /* object is ready flag*/
};





struct pexor_dmabuf
{
  struct list_head queue_list;  /* linked into free or receive queue list */
  dma_addr_t dma_addr;          /* dma engine (pci) address */
  unsigned long kernel_addr;    /* mapped kernel address */
  unsigned long virt_addr;      /* user space virtual address (=buffer id) */
  unsigned long size;           /* buffer size in bytes */
};



struct pexor_privdata
{
  int magic;                    /* magic number to identify irq */
  atomic_t state;               /* run state of device */
  dev_t devno;                  /* device number (major and minor) */
  int devid;                    /* local id (counter number) */
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

/* hold full device number */
static dev_t pexor_devt;

/* counts number of probed pexor devices */
static atomic_t pexor_numdevs;

/* helper to access private data in file struct. returns 0 if pexor
 * not initialized */
struct pexor_privdata *get_privdata(struct file *fil);

/* create dma buffer of size, allocate it for pci device pdev,
 * returns buffer descriptor structure */
struct pexor_dmabuf *new_dmabuffer(struct pci_dev *pdev, size_t size);


/* remove dmabuffer from pci device */
int delete_dmabuffer(struct pci_dev *pdev, struct pexor_dmabuf *buf);

/* print address and value of a register to dmesg debug output */
void print_register(const char *description, u32 * address);

void print_pexor(struct dev_pexor *pg);
void clear_pexor(struct dev_pexor *pg);
void set_pexor(struct dev_pexor *pg, void *base, unsigned long bar);

#ifdef PEXOR_WITH_SFP
void set_sfp(struct pexor_sfp *sfp, void *membase, unsigned long bar);
void print_sfp(struct pexor_sfp *sfp);
void pexor_show_version(struct pexor_sfp *sfp, char *buf);

/* send request command comm to sfp address addr with optional send data.
 * will not wait for response! */
void pexor_sfp_request(struct pexor_privdata *privdata, u32 comm, u32 addr,
                       u32 data);

/* wait for sfp reply on channel ch.
 * return values are put into comm, addr, and data.
 * checkvalue specifies which return type is expected;
 * will return error if not matching */
int pexor_sfp_get_reply(struct pexor_privdata *privdata, int ch, u32 * comm,
                        u32 * addr, u32 * data, u32 checkvalue);

/* wait for sfp token reply on channel ch.
 * return values are put into stat, head, and foot. */
int pexor_sfp_get_token_reply(struct pexor_privdata *privdata, int ch,
                              u32 * stat, u32 * head, u32 * foot);



/* initialize the connected slaves on sfp channel ch */
int pexor_sfp_init_request(struct pexor_privdata *privdata, int ch,
                           int numslaves);


/* clear all sfp connections and wait until complete.
 * return value specifies error if not 0 */
int pexor_sfp_clear_all(struct pexor_privdata *privdata);

/* clear sfp channel ch and wait for success
 * return value specifies error if not 0 */
int pexor_sfp_clear_channel(struct pexor_privdata *privdata, int ch);

#endif

int pexor_open(struct inode *inode, struct file *filp);
int pexor_release(struct inode *inode, struct file *filp);
loff_t pexor_llseek(struct file *filp, loff_t off, int whence);
ssize_t pexor_read(struct file *filp, char __user * buf, size_t count,
                   loff_t * f_pos);
ssize_t pexor_write(struct file *filp, const char __user * buf, size_t count,
                    loff_t * f_pos);

int pexor_mmap(struct file *filp, struct vm_area_struct *vma);

/* the general fops ioctl */
int pexor_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
                unsigned long arg);

/* general reset function */
int pexor_ioctl_reset(struct pexor_privdata *priv, unsigned long arg);



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

/* Initiate reading a token buffer from sfp front end hardware.
 * In synchronous mode, will block until transfer is done and delivers back dma buffer with token data.
 * In asynchronous mode, function returns immediately after token request;
 * user needs to ioctl a wait token afterwards.
 * Setup and data contained in user arg structure */
int pexor_ioctl_request_token(struct pexor_privdata *priv, unsigned long arg);


/* Waits for a token to arrive previously requested by
 * an asynchronous ioctl request token
 * Setup and data contained in user arg structure */
int pexor_ioctl_wait_token(struct pexor_privdata *priv, unsigned long arg);


/* Wait for a trigger interrupt from pexor. Will be raised from trixor board
 * and routed to pci throug pexor driver. */
int pexor_ioctl_wait_trigger(struct pexor_privdata *priv, unsigned long arg);

/* set acquisition state of trixor trigger module extension.
 * used to clear deadtime flag from user program and start/stop acquisition mode, etc.*/
int pexor_ioctl_set_trixor(struct pexor_privdata* priv, unsigned long arg);

irqreturn_t pexor_isr(int irq, void *dev_id);


void pexor_irq_tasklet(unsigned long);

/* set next receive buffer and start dma engine.
 * if source address is 0, we use pexor RAM area as start
 * roffset is dma startpoint relative to source
 * dmasize gives bytes to transfer by dma; 
 * if 0, we use complete size of allocated dma buffer */
int pexor_next_dma(struct pexor_privdata *priv, dma_addr_t source,
                   u32 roffset, u32 dmasize);

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
ssize_t pexor_sysfs_sfpregs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf);

static DEVICE_ATTR(freebufs, S_IRUGO, pexor_sysfs_freebuffers_show, NULL);
static DEVICE_ATTR(usedbufs, S_IRUGO, pexor_sysfs_usedbuffers_show, NULL);
static DEVICE_ATTR(rcvbufs, S_IRUGO, pexor_sysfs_rcvbuffers_show, NULL);
static DEVICE_ATTR(codeversion, S_IRUGO, pexor_sysfs_codeversion_show, NULL);
static DEVICE_ATTR(dmaregs, S_IRUGO, pexor_sysfs_dmaregs_show, NULL);
static DEVICE_ATTR(sfpregs, S_IRUGO, pexor_sysfs_sfpregs_show, NULL);

#endif
#endif

#endif
