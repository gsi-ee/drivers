//
// N.Kurz, EE, GSI, 24-Oct-2012: based on example code from R.Eibauer
// N.Kurz, EE, GSI, 20-Jul-2015: modified for kernel verion 3.3.10
//                               optional vtrans mapping of triva registers for fast(est) vme access 
// JAM, EEL, GSI, 01-Mar-2023: added sysfs with handle to disable interrupt handling / change ir level mask

#define VMETRIGMODVERSION     "0.3.2"
#define VMETRIGMODAUTHORS     "Joern Adamczewski-Musch (JAM), Nikolaus Kurz, GSI Darmstadt (www.gsi.de)"
#define VMETRIGMODDESC        "TRIVA VMEbus trigger module of MBS for RIO4/Sugarhat Linux"





#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>


#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/sysfs.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/device.h>

//#include <linux/byteorder/little_endian.h>

#ifdef VMETRIGMOD_NEW_XPCLIB
#include <ces/CesXpcBridge.h>
#endif

#include <ces/xpc_vme.h>
#include <ces/xpc.h>

//#define DEBUG 1

#ifdef DEBUG
#define debug(x)        printk x
#else // DEBUG
#define debug(x)
#endif // DEBUG


#ifdef DEBUG

#define debugk( args... )                    \
  printk( args );
#else
#define debugk( args... ) ;
#endif

#define TRIVA_BUS_DELAY 20

#define triva_bus_delay()                       \
  mb();      \
  ndelay(TRIVA_BUS_DELAY);



#define VTRANS 1 
//#define INTERNAL_TRIG_TEST 1

//-----------------------------------------------------------------------------
// only for the moment:
#define WAIT_SEM              12
#define POLL_SEM              16
#define RESET_SEM           0x1236
//-----------------------------------------------------------------------------
//--- status register bits
#define DT_CLEAR     0x00000020
#define IRQ_CLEAR    0x00001000
#define DI_CLEAR     0x00002000
#define EV_IRQ_CLEAR 0x00008000
#define EON          0x00008000
#define FC_PULSE     0x00000010
//--- control register
#define MASTER     0x00000004
#define SLAVE      0x00000020
#define HALT       0x00000010
#define GO         0x00000002
#define EN_IRQ     0x00000001
#define DIS_IRQ    0x00000008
#define CLEAR      0x00000040
#define BUS_ENABLE 0x00000800

#ifndef VMETRIGMOD_NEW_XPCLIB
extern int xpc_vme_request_irq (unsigned int vec, unsigned int lev, void (*handler)(int vec, int prio, void *arg), void *arg, const char *name);
extern void xpc_vme_free_irq (unsigned int vec);
#endif


#include "vmetrigmod_ioctl.h"

#ifdef VMETRIGMOD_NEW_XPCLIB
struct CesXpcBridge *vme_bridge;
#endif

#define TRIGMOD_IRQ_VECTOR  0x50 
#define TRIGMOD_IRQ_MASK    ((1 << 3) | (1 << 4)) /* interrupt happens at level 3 or 4 */
#define TRIGMOD_REGS_ADDR   0x2000000
#define TRIGMOD_REGS_SIZE   0x100

#ifdef VTRANS
volatile u32 __iomem *vtrans_vme_regs;
#endif

static struct semaphore triv_sem;
static int              triv_val;  

// trigger module register
static unsigned int *pl_stat;
static unsigned int *pl_ctrl;
static unsigned int *pl_fcti;
static unsigned int *pl_cvti;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class *vme_sysfs_class;   /* Sysfs class */
static const char device_name[] = "trigmod";
static struct class *vme_sysfs_class;   /* Sysfs class */
static struct device* class_dev[1]; /**< Class device */
#endif

static unsigned int vmetrigmod_irq_mask = TRIGMOD_IRQ_MASK;

static void trigmod_irqhandler (int vec, int prio, void *arg);

// JAM - as usual some info to view at sysfs:
ssize_t vmetrigmod_sysfs_codeversion_show (struct device *dev, struct device_attribute *attr, char *buf)
{

    ssize_t curs=0;
    curs += snprintf (buf + curs, PAGE_SIZE, "*** This is %s, version %s build on %s at %s \n",
        VMETRIGMODDESC, VMETRIGMODVERSION, __DATE__, __TIME__);
    curs += snprintf (buf + curs, PAGE_SIZE, "\tmodule authors: %s \n", VMETRIGMODAUTHORS);
    return curs;
}

static DEVICE_ATTR(codeversion, S_IRUGO, vmetrigmod_sysfs_codeversion_show, NULL);


ssize_t vmetrigmod_sysfs_regs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
//  struct pex_privdata *privdata;
//  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** TRIVA register dump:\n");
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t TRIVA stat: .... 0x%x\n", *pl_stat);
  triva_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t TRIVA ctrl: .... 0x%x\n", *pl_ctrl);
  triva_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t TRIVA fcti: .... 0x%x (FCTI=0x%x)\n", *pl_fcti, 0x10000 - *pl_fcti);
  triva_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t TRIVA cvti: .... 0x%x (CVTI=0x%x)\n", *pl_cvti, 0x10000 - *pl_cvti);
  return curs;
}

static DEVICE_ATTR(trivaregs, S_IRUGO, vmetrigmod_sysfs_regs_show, NULL);







ssize_t vmetrigmod_sysfs_irqmask_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  unsigned int val=0;
  val=vmetrigmod_irq_mask;
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "0x%x\n", val);
   return curs;
}

ssize_t vmetrigmod_sysfs_irqmask_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  unsigned int val=0;
  int result=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif

//  struct pex_privdata *privdata;
//    privdata = (struct pex_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
  if(val==vmetrigmod_irq_mask)
  {
    printk( KERN_NOTICE "VMETRIGMOD: sees unchanged interupt mask 0x%x, do nothing.\n", val);
    return count;
  }
  vmetrigmod_irq_mask=val;
  printk( KERN_NOTICE "VMETRIGMOD: changed interupt mask to value 0x%x\n", val);



  printk( KERN_NOTICE "VMETRIGMOD: vmetrigmod_sysfs_irqmask_store will free interrupt...\n");
  xpc_vme_free_irq (TRIGMOD_IRQ_VECTOR);
  if(val)
  {
    printk( KERN_NOTICE "VMETRIGMOD: vmetrigmod_sysfs_irqmask_will enable new interrupt mask 0x%x...\n",vmetrigmod_irq_mask);
    result = xpc_vme_request_irq (TRIGMOD_IRQ_VECTOR, vmetrigmod_irq_mask, trigmod_irqhandler, dev, "vmetrigmod");
     if (result) {
       printk( KERN_ERR "VMETRIGMOD: vmetrigmod_sysfs_irqmask_error %d when requesting irq !\n",result);
     }
  }
   return count;
}





static DEVICE_ATTR(irqmask, S_IWUGO | S_IRUGO, vmetrigmod_sysfs_irqmask_show, vmetrigmod_sysfs_irqmask_store);







struct trigmod_devdata
{
  int irq_count;
  void __iomem *registers;
  phys_addr_t regs_phys;
};

struct trigmod_user_data
{
  struct trigmod_devdata *dev;
  /* User's allocations must be tracked. */
};


static void trigmod_irqhandler (int vec, int prio, void *arg)
{
  struct trigmod_devdata *dev = arg;

  debug ((KERN_INFO "BEGIN irq_hand \n"));
  
  debug ((KERN_INFO "IRQ Level: %d, IRQ vector: 0x%x \n", prio, vec));

  //disable_irq_nosync (irq);  ???

  // clear source of pending interrupts (in triva)
  #ifndef VTRANS
  out_be32 (pl_stat, (EV_IRQ_CLEAR | IRQ_CLEAR));
  #else
  *pl_stat = EV_IRQ_CLEAR | IRQ_CLEAR;
  #endif

  mb ();

  //enable_irq (irq);   ???

  triv_val = 1;  
  up (&triv_sem);

  #ifdef INTERNAL_TRIG_TEST
  // clear trigger module for test reasons only
  #ifndef VTRANS
  out_be32 (pl_stat, (EV_IRQ_CLEAR | IRQ_CLEAR));
  out_be32 (pl_stat,  FC_PULSE);
  out_be32 (pl_stat,  DT_CLEAR);
  #else
  *pl_stat = EV_IRQ_CLEAR | IRQ_CLEAR;
  *pl_stat = FC_PULSE;
  *pl_stat = DT_CLEAR; 
  #endif 
  #endif //INTERNAL_TRIG_TEST
 
  debug ((KERN_INFO "END   irq_hand \n"));

  //return IRQ_HANDLED;
  dev->irq_count++;
}

static int trigmod_get_irqcount(struct trigmod_devdata *dev, int clear)
{
  int tmp;

  tmp = dev->irq_count;
  if (clear)
  dev->irq_count = 0;
  return tmp;
}

static int trigmod_init_dev (struct trigmod_devdata *dev)
{
  int result;
  result = xpc_vme_request_irq (TRIGMOD_IRQ_VECTOR, vmetrigmod_irq_mask, trigmod_irqhandler, dev, "vmetrigmod");
  if (result) { return result; }
  #ifdef VMETRIGMOD_NEW_XPCLIB
  dev->regs_phys = CesXpcBridge_MasterMap64 (vme_bridge, TRIGMOD_REGS_ADDR, TRIGMOD_REGS_SIZE, XPC_VME_ATYPE_A32 | XPC_VME_DTYPE_STD);
  if (dev->regs_phys == 0xffffffffffffffffULL)
  {
    result = -ENOMEM;
    goto out_free_irq;
  }
  #else
  dev->regs_phys = xpc_vme_master_map (TRIGMOD_REGS_ADDR, 0, TRIGMOD_REGS_SIZE, XPC_VME_ATYPE_A32 | XPC_VME_DTYPE_STD, 0);
  if (dev->regs_phys == 0xffffffffULL)
  {
    result = -ENOMEM;
    goto out_free_irq;
  }
  #endif

  //dev->registers = ioremap (dev->regs_phys, TRIGMOD_REGS_SIZE);
  dev->registers = ioremap_nocache (dev->regs_phys, TRIGMOD_REGS_SIZE);
  if (!dev->registers)
  {
    result = -ENOMEM;
    goto out_unmap;
  }

  printk (KERN_INFO " map TRIVA registers \n");

  #ifndef VTRANS
  pl_stat = (unsigned int*) ((long)dev->registers + 0x0);
  pl_ctrl = (unsigned int*) ((long)dev->registers + 0x4);
  pl_fcti = (unsigned int*) ((long)dev->registers + 0x8);
  pl_cvti = (unsigned int*) ((long)dev->registers + 0xc);

  printk (KERN_INFO " Ptr. TRIVA stat: 0x%x \n", (unsigned int)pl_stat);
  printk (KERN_INFO " Ptr. TRIVA ctrl: 0x%x \n", (unsigned int)pl_ctrl);
  printk (KERN_INFO " Ptr. TRIVA fcti: 0x%x \n", (unsigned int)pl_fcti);
  printk (KERN_INFO " Ptr. TRIVA cvti: 0x%x \n", (unsigned int)pl_cvti);

  printk (KERN_INFO " TRIVA registers content \n");
  printk (KERN_INFO " TRIVA stat: .... 0x%x \n", in_be32(pl_stat));
  printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", in_be32(pl_ctrl));
  printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0xffff - in_be32(pl_fcti));
  printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0xffff - in_be32(pl_cvti));
  #else
  vtrans_vme_regs = ioremap (0x800000000ULL + 0x2000000ULL, 0x100);
  pl_stat = (unsigned int*) ((u32)vtrans_vme_regs + 0x0);
  pl_ctrl = (unsigned int*) ((u32)vtrans_vme_regs + 0x4);
  pl_fcti = (unsigned int*) ((u32)vtrans_vme_regs + 0x8);
  pl_cvti = (unsigned int*) ((u32)vtrans_vme_regs + 0xc);

  printk (KERN_INFO " VTRANS Ptr. TRIVA stat: 0x%x \n", (unsigned int)pl_stat);
  printk (KERN_INFO " VTRANS Ptr. TRIVA ctrl: 0x%x \n", (unsigned int)pl_ctrl);
  printk (KERN_INFO " VTRANS Ptr. TRIVA fcti: 0x%x \n", (unsigned int)pl_fcti);
  printk (KERN_INFO " VTRANS Ptr. TRIVA cvti: 0x%x \n", (unsigned int)pl_cvti);

  printk (KERN_INFO " VTRANS TRIVA registers content \n");
  printk (KERN_INFO " VTRANS TRIVA stat: .... 0x%x \n", *pl_stat);
  printk (KERN_INFO " VTRANS TRIVA ctrl: .... 0x%x \n", *pl_ctrl);
  printk (KERN_INFO " VTRANS TRIVA fcti: .... 0x%x \n", 0xffff - *pl_fcti);
  printk (KERN_INFO " VTRANS TRIVA cvti: .... 0x%x \n", 0xffff - *pl_cvti);
  #endif

  #ifdef INTERNAL_TRIG_TEST
  // initalize TRIVA only for internal tests
  printk (KERN_INFO "\n");
  printk (KERN_INFO " Initalize TRIVA \n");

  #ifndef VTRANS
  out_be32 (pl_ctrl, 0x1000);
  out_be32 (pl_ctrl, HALT);
  out_be32 (pl_ctrl, MASTER);
  out_be32 (pl_ctrl, CLEAR);

  out_be32 (pl_ctrl, BUS_ENABLE);
  out_be32 (pl_ctrl, CLEAR);
  out_be32 (pl_stat, EV_IRQ_CLEAR);
  out_be32 (pl_fcti, 0xffff - 0x10);
  out_be32 (pl_cvti, 0xffff - 0x20);

  out_be32 (pl_ctrl, HALT);
  out_be32 (pl_ctrl, CLEAR);
  out_be32 (pl_stat, 14);
  out_be32 (pl_ctrl, (EN_IRQ | GO));

  printk (KERN_INFO " TRIVA registers content \n");
  printk (KERN_INFO " TRIVA stat: .... 0x%x \n", in_be32 (pl_stat));
  printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", in_be32 (pl_ctrl));
  printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0xffff - in_be32 (pl_fcti));
  printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0xffff - in_be32 (pl_cvti));
  #else
  *pl_ctrl = 0x1000;
  *pl_ctrl = HALT;
  *pl_ctrl = MASTER;
  *pl_ctrl = CLEAR;

  *pl_ctrl = BUS_ENABLE;
  *pl_ctrl = CLEAR;
  *pl_stat = EV_IRQ_CLEAR;
  *pl_fcti = 0xffff - 0x11;
  *pl_cvti = 0xffff - 0x21;

  *pl_ctrl = HALT;
  *pl_ctrl = CLEAR;
  *pl_stat = 14;
  *pl_ctrl = EN_IRQ | GO;

  printk (KERN_INFO " VTRANS TRIVA registers content \n");
  printk (KERN_INFO " VTRANS TRIVA stat: .... 0x%x \n", *pl_stat);
  printk (KERN_INFO " VTRANS TRIVA ctrl: .... 0x%x \n", *pl_ctrl);
  printk (KERN_INFO " VTRANS TRIVA fcti: .... 0x%x \n", 0xffff - *pl_fcti);
  printk (KERN_INFO " VTRANS TRIVA cvti: .... 0x%x \n", 0xffff - *pl_cvti);
  #endif
  #endif // INTERNAL_TRIG_TEST 

  return 0;

out_unmap:
  #ifdef VMETRIGMOD_NEW_XPCLIB
  CesXpcBridge_MasterUnMap64 (vme_bridge, dev->regs_phys, TRIGMOD_REGS_SIZE);
  #else
  xpc_vme_master_unmap (dev->regs_phys, TRIGMOD_REGS_SIZE);
  #endif
  out_free_irq:
  xpc_vme_free_irq (TRIGMOD_IRQ_VECTOR);
  return result;
}

static void trigmod_cleanup_dev (struct trigmod_devdata *dev)
{
  iounmap(dev->registers);
  #ifdef VMETRIGMOD_NEW_XPCLIB
  CesXpcBridge_MasterUnMap64 (vme_bridge, dev->regs_phys, TRIGMOD_REGS_SIZE);
  #else
  xpc_vme_master_unmap (dev->regs_phys, TRIGMOD_REGS_SIZE);
  #endif
  xpc_vme_free_irq (TRIGMOD_IRQ_VECTOR);
}

static long trigmod_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
  int retval = 0;
  struct trigmod_user_data *user_data = filp->private_data;

  debug ((KERN_INFO "BEGIN pexor_ioctl \n"));

  switch (cmd)
  {
    case CMD_TRIGMOD_GET_IRQCOUNT:
    {
      union vmetrigmod_get_irqcount_arg args;

      if (copy_from_user((void __user*)arg, &args, sizeof(args.in)))
      {
        retval = -EFAULT;
        break;
      }
      args.out.value = trigmod_get_irqcount (user_data->dev, args.in.clear);
      if (copy_to_user((void __user*)arg, &args, sizeof(args.out)))
      {
        retval = -EFAULT;
        break;
      }
      break;
    }
    case WAIT_SEM:
      debug ((KERN_INFO " before WAIT_SEM \n"));
      down (&triv_sem);
      triv_val = 0;
      debug ((KERN_INFO " after  WAIT_SEM \n"));
      break;
    case POLL_SEM:
      debug ((KERN_INFO " before POLL_SEM, triv_val: %d \n", triv_val));
      retval = __put_user(triv_val, (int __user *)arg);
      debug ((KERN_INFO " after POLL_SEM \n"));
      break;
    case RESET_SEM:
      printk (KERN_INFO " before RESET_SEM \n");
      triv_val = 0;
      init_MUTEX_LOCKED (&triv_sem);
      retval = __put_user(0, (int __user *)arg);
      printk (KERN_INFO " after  RESET_SEM \n");
      break;
    default:
      retval = -EINVAL;
      break;
  }
  debug ((KERN_INFO "END   pexor_ioctl \n"));
  return retval;
}

static struct trigmod_devdata *trigmod_dev;

static int trigmod_open (struct inode *inode, struct file *filp)
{
  struct trigmod_user_data *user_data;

  user_data = kzalloc (sizeof(struct trigmod_user_data), GFP_KERNEL);
  user_data->dev = trigmod_dev;
  filp->private_data = user_data;

  return 0;
}

static int trigmod_release (struct inode *inode, struct file *filp)
{
  struct trigmod_user_data *user_data = filp->private_data;

  kfree (user_data);
  return 0;
}

static struct file_operations trigmod_fops =
{
  .open           = trigmod_open,
  .release        = trigmod_release,
  .unlocked_ioctl = trigmod_ioctl,
};

static int trigmod_major = 0;

int __init trigmod_init (void)
{
  int result;

  result = register_chrdev (0, "vmetrigmod", &trigmod_fops);
  if (result < 0)
  {
    printk (KERN_ERR "%s: unable to get a major number\n", __func__);
    return -EIO;
  }
  trigmod_major = result;
  trigmod_dev = kmalloc (sizeof(*trigmod_dev), GFP_KERNEL);
  if (!trigmod_dev)
  {
    result = -ENOMEM;
    goto out_unregister;
  }

  /*--------------------------------------------------------------------------
     * Create sysfs entries - on udev systems this creates the dev files
     *--------------------------------------------------------------------------*/
    vme_sysfs_class = class_create( THIS_MODULE, device_name);
    if (IS_ERR( vme_sysfs_class))
    {
      result = PTR_ERR( vme_sysfs_class);
      goto out_unregister;
    }

    class_dev[0]=device_create( vme_sysfs_class, NULL, MKDEV( trigmod_major, 0), NULL, "triva0");


    // JAM2023 new - add sysfs info here
      if (device_create_file (class_dev[0], &dev_attr_codeversion) != 0)
      {
          printk (KERN_ERR "Could not add device file node for code version.\n");
      }
      if (device_create_file (class_dev[0], &dev_attr_trivaregs) != 0)
      {
          printk (KERN_ERR "Could not add device file node for trivaregs.\n");
      }

      if (device_create_file (class_dev[0], &dev_attr_irqmask) != 0)
          {
              printk (KERN_ERR "Could not add device file node for irqmask.\n");
          }



  #ifdef VMETRIGMOD_NEW_XPCLIB
  vme_bridge = CesXpcBridge_GetByName ("VME Bridge");
  if (!vme_bridge)
  {
    goto out_free;
  }
  #endif
  result = trigmod_init_dev (trigmod_dev);
  if (result)
  goto out_free;

  printk (KERN_INFO "VME TRIVA driver installed @ major %d\n", trigmod_major);
  return 0;

out_free:
  kfree (trigmod_dev);
out_unregister:
  unregister_chrdev (trigmod_major, "vmetrigmod");
  return result;
}

void trigmod_exit (void)
{
  trigmod_cleanup_dev (trigmod_dev);
  kfree (trigmod_dev);
   device_destroy( vme_sysfs_class, MKDEV( trigmod_major, 0));
   class_destroy( vme_sysfs_class);


  unregister_chrdev (trigmod_major, "vmetrigmod");

  printk (KERN_INFO "VME TRIVA driver  removed\n");
}

module_init (trigmod_init);
module_exit (trigmod_exit);


MODULE_AUTHOR(VMETRIGMODAUTHORS);
MODULE_DESCRIPTION(VMETRIGMODDESC);
MODULE_LICENSE("GPL");
MODULE_VERSION(VMETRIGMODVERSION);



