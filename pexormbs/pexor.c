// N.Kurz, EE, GSI, 8-Apr-2010
//-----------------------------------------------------------------------------

//#define DEBUG

#ifdef DEBUG
 #define debug(x)        printk x
#else DEBUG
 #define debug(x)
#endif DEBUG

//#define INTERNAL_TRIG_TEST 1
//-----------------------------------------------------------------------------
// only for the moment:
#define WAIT_SEM              12
#define POLL_SEM              16
#define GET_BAR0_BASE       0x1234
#define GET_BAR0_TRIX_BASE  0x1235
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
//-----------------------------------------------------------------------------
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/interrupt.h>
//#include <linux/config.h>
//#include <linux/module.h>
//#include <linux/moduleparam.h>
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
//#include <linux/errno.h>	/* error codes */
//#include <linux/types.h>	/* size_t */
//#include <linux/proc_fs.h>
//#include <linux/fcntl.h>	/* O_ACCMODE */
//#include <linux/seq_file.h>
#include <linux/cdev.h>
//#include <asm/signal.h>
#include <asm/uaccess.h>
#include <asm/io.h>
//-----------------------------------------------------------------------------
#define PEXOR_VENDOR_ID     0x1204
#define PEXOR_DEVICE_ID     0x5303

#define BAR0_REG_OFF        0x20000  // register base offset with resp. to BAR0
#define BAR0_TRIX_OFF       0x40000  // TRIXOR base offset with resp. to BAR0
#define BAR0_RAM_OFF        0x100000
#define BAR0_SIZE           0x200000
//-----------------------------------------------------------------------------
static u16 l_ve_id=0;
static u16 l_de_id=0; 
static u8  l_rev  =0;
static u32 l_bar0 =0;
static u8  l_irq_pin=0;
static u8  l_irq_line=0;
static u32 l_bar0_trix_base=0;
static u32 l_bar0_base=0;
static u32 l_bar0_end=0;
static u32 l_bar0_flags=0;
static u32 l_map_bar0_trix_base=0;
//static u32 l_map_bar0_base=0;

static struct semaphore trix_sem;
static long             trix_val;  

//static long *pl_bar0_base;
//static long *pl_bar0_reg;
//static long *pl_bar0_trix;
//static long *pl_bar0_ram;

// trigger module register
static unsigned int *pl_stat;
static unsigned int *pl_ctrl;
static unsigned int *pl_fcti;
static unsigned int *pl_cvti;

static long  l_irq_ct=0;
//static long  l_stat;

static unsigned long *l_addr;

#ifndef PEXOR_MAJOR
#define PEXOR_MAJOR 0     // dynamic major by default
#endif

#ifndef PEXOR_NR_DEVS
#define PEXOR_NR_DEVS 1   // pexor
#endif
//-----------------------------------------------------------------------------
struct pexor_dev
{
	//struct semaphore sem;   // mutual exclusion semaphore
	struct cdev      cdev;	// Char device structure
};
//-----------------------------------------------------------------------------
int  pexor_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg);

int pexor_major =   PEXOR_MAJOR;
int pexor_minor =   0;
int pexor_nr_devs = PEXOR_NR_DEVS;	// number of bare pexor devices

MODULE_AUTHOR ("Nikolaus Kurz, EE, GSI, 30-Mar-2010");
MODULE_LICENSE("Dual BSD/GPL");

struct pexor_dev *pexor_devices;	// allocated in pexor_init_module
//-----------------------------------------------------------------------------
int pexor_open(struct inode *inode, struct file *filp)
{
	struct pexor_dev *dev;      // device information
  printk (KERN_INFO "\nBEGIN pexor_open \n");

	dev = container_of(inode->i_cdev, struct pexor_dev, cdev);
	filp->private_data = dev;   // for other methods
  printk (KERN_INFO "END   pexor_open \n");
	return 0;                   // success
}
//-----------------------------------------------------------------------------
int pexor_release(struct inode *inode, struct file *filp)
{
  printk (KERN_INFO "BEGIN pexor_release \n");
  printk (KERN_INFO "END   pexor_release \n");
	return 0;
}

//-----------------------------------------------------------------------------
int pexor_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	int retval = 0;
  debug ((KERN_INFO "BEGIN pexor_ioctl \n"));
  //printk (KERN_INFO "BEGIN pexor_ioctl \n");

  switch (cmd)
  {
    case WAIT_SEM:
      debug ((KERN_INFO " before WAIT_SEM \n"));
      //printk (KERN_INFO " before WAIT_SEM \n");
      down (&trix_sem);
      trix_val = 0;
      //sema_init (&trix_sem, 0); 
      //init_MUTEX_LOCKED (&trix_sem);
      debug ((KERN_INFO " after  WAIT_SEM \n"));
      //printk (KERN_INFO " after  WAIT_SEM \n");
      break;
    case POLL_SEM:
      debug ((KERN_INFO " before POLL_SEM, trix_val: %d \n", trix_val));
      //printk (KERN_INFO " before POLL_SEM, trix_val: %d \n", trix_val);
		  retval = __put_user(trix_val, (int __user *)arg);
      debug ((KERN_INFO " after POLL_SEM \n"));
				//printk (KERN_INFO " after POLL_SEM \n");
      break;
    case GET_BAR0_BASE:
      printk (KERN_INFO " before GET_BAR0_BASE \n");
		  retval = __put_user(l_bar0, (int __user *)arg);
      printk (KERN_INFO " after  GET_BAR0_BASE \n");
      break;
    case GET_BAR0_TRIX_BASE:
      printk (KERN_INFO " before GET_TRIX_BASE \n");
		  retval = __put_user(l_bar0_trix_base, (int __user *)arg);
      printk (KERN_INFO " after  GET_TRIX_BASE \n");
      break;
    case RESET_SEM:
      printk (KERN_INFO " before RESET_SEM \n");
      trix_val = 0;
      //sema_init (&trix_sem, 0); 
      init_MUTEX_LOCKED (&trix_sem);
		  retval = __put_user(0, (int __user *)arg);
      printk (KERN_INFO " after  RESET_SEM \n");
      break;
    default:
      break;
  }
  debug ((KERN_INFO "END   pexor_ioctl \n"));
  //printk (KERN_INFO "END   pexor_ioctl \n");
  return retval;
}
//-----------------------------------------------------------------------------
struct file_operations pexor_fops = {
	.owner =    THIS_MODULE,
	.ioctl =    pexor_ioctl,
	.open =     pexor_open,
	.release =  pexor_release,
};
//-----------------------------------------------------------------------------
static struct pci_device_id ids[] =
{
	{ PCI_DEVICE(PEXOR_VENDOR_ID, PEXOR_DEVICE_ID), },  // PEXOR
	{ 0, }
};
//-----------------------------------------------------------------------------
MODULE_DEVICE_TABLE(pci, ids);
//-----------------------------------------------------------------------------
static unsigned char pexor_get_pci_config(struct pci_dev *pdev)
{
	u8 revision;

  printk (KERN_INFO " BEGIN pexor_get_pci_config \n");
  // read some config parameters

  pci_read_config_word (pdev, PCI_VENDOR_ID, &l_ve_id);
  printk (KERN_INFO "  vendor id:........0x%x \n", l_ve_id);
  pci_read_config_word (pdev, PCI_DEVICE_ID, &l_de_id);
  printk (KERN_INFO "  device id:........0x%x \n", l_de_id);
	pci_read_config_byte (pdev, PCI_REVISION_ID, &l_rev);
  printk (KERN_INFO "  revison id:.......0x%x \n", l_rev);
	pci_read_config_dword (pdev, PCI_BASE_ADDRESS_0, &l_bar0);
  printk (KERN_INFO "  BAR0 base:........0x%x \n", l_bar0);
	pci_read_config_byte (pdev, PCI_INTERRUPT_LINE, &l_irq_line);
  printk (KERN_INFO "  IRQ_LINE:...........%d \n", l_irq_line);
	pci_read_config_byte (pdev, PCI_INTERRUPT_PIN, &l_irq_pin);
  printk (KERN_INFO "  IRQ_PIN:............%d \n", l_irq_pin);
  l_bar0_trix_base = l_bar0 + BAR0_TRIX_OFF;
  printk (KERN_INFO "  l_bar0_trix_base: 0x%x \n", l_bar0_trix_base);

	pci_read_config_byte(pdev, PCI_REVISION_ID, &revision);
  printk (KERN_INFO " END pexor_get_pci_config \n");
	return revision;
}
//-----------------------------------------------------------------------------

irqreturn_t irq_hand (int irq, void *dev_id, struct pt_regs *regs)
{
  debug ((KERN_INFO "BEGIN irq_hand \n"));
  //printk (KERN_INFO "BEGIN irq_hand \n");

  disable_irq_nosync (irq);  

  ndelay (1000);

  // clear source of pending interrupts (in trixor)
  iowrite32 ((EV_IRQ_CLEAR | IRQ_CLEAR), pl_stat);
  //wmb ();
  mb ();

  ndelay (1000);

  enable_irq (irq);

  //ndelay (200);

  trix_val = 1;
  up (&trix_sem);

  #ifdef INTERNAL_TRIG_TEST
	// clear trigger module for test reasons only
  iowrite32 ((EV_IRQ_CLEAR | IRQ_CLEAR), pl_stat);
  iowrite32 (FC_PULSE,                   pl_stat);
  iowrite32 (DT_CLEAR,                   pl_stat);
  #endif //INTERNAL_TRIG_TEST
 
  debug ((KERN_INFO "END   irq_hand \n"));
  //printk (KERN_INFO "END   irq_hand \n"); 
	return IRQ_HANDLED;
}

//-----------------------------------------------------------------------------
static int probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
  printk (KERN_INFO "BEGIN probe function \n");

  printk (KERN_INFO " IRQ: %d\n", pdev->irq);

	pci_enable_device(pdev);
	if (pexor_get_pci_config(pdev) == 0x42)
		return -ENODEV;

  printk (KERN_INFO " check bar0 resources \n");
  l_bar0_base = pci_resource_start (pdev, 0);
  printk (KERN_INFO " l_bar0_base:  0x%x \n", l_bar0_base);
  l_bar0_end  = pci_resource_end (pdev, 0);
  printk (KERN_INFO " l_bar0_end:   0x%x \n", l_bar0_end);
  l_bar0_flags  = pci_resource_flags (pdev, 0);
  printk (KERN_INFO " l_bar0_flags: 0x%x \n", l_bar0_flags);

  printk (KERN_INFO " map TRIXOR registers \n");

	if (! request_mem_region(l_bar0_trix_base, 16, "pexor"))
  {
		printk(KERN_INFO "ERROR>> requesting TRIXOR 0x%x\n", l_bar0_trix_base);
		return -ENODEV;
	}
	l_map_bar0_trix_base = (unsigned long) ioremap_nocache (l_bar0_trix_base, 16);
  printk (KERN_INFO " l_map_bar0_base:  0x%x \n", l_map_bar0_trix_base);

  printk (KERN_INFO " map TRIXOR registers \n");
  // map TRIXOR register 
  pl_stat = (unsigned int*) ((long)l_map_bar0_trix_base + 0x0);
  pl_ctrl = (unsigned int*) ((long)l_map_bar0_trix_base + 0x4);
  pl_fcti = (unsigned int*) ((long)l_map_bar0_trix_base + 0x8);
  pl_cvti = (unsigned int*) ((long)l_map_bar0_trix_base + 0xc);

  printk (KERN_INFO " Ptr. TRIXOR stat: 0x%x \n", (unsigned int)pl_stat);
  printk (KERN_INFO " Ptr. TRIXOR ctrl: 0x%x \n", (unsigned int)pl_ctrl);
  printk (KERN_INFO " Ptr. TRIXOR fcti: 0x%x \n", (unsigned int)pl_fcti);
  printk (KERN_INFO " Ptr. TRIXOR cvti: 0x%x \n", (unsigned int)pl_cvti);

  printk (KERN_INFO " TRIXOR registers content \n");
  printk (KERN_INFO " TRIXOR stat: .... 0x%x \n", ioread32(pl_stat));
  printk (KERN_INFO " TRIXOR ctrl: .... 0x%x \n", ioread32(pl_ctrl));
  printk (KERN_INFO " TRIXOR fcti: .... 0x%x \n", 0x10000 - ioread32(pl_fcti));
  printk (KERN_INFO " TRIXOR cvti: .... 0x%x \n", 0x10000 - ioread32(pl_cvti));

  printk (KERN_INFO " Initialize mutex in locked state \n");
  //sema_init (&trix_sem, 0); 
  init_MUTEX_LOCKED (&trix_sem);
  trix_val = 0;

	/*
  // tests not necessary fo functionality
	l_addr = ioremap (0x90000000, 0x2000);
  printk (KERN_INFO "ioreamp addr:     0x%x \n", l_addr);
  l_addr = virt_to_phys (l_addr);
  printk (KERN_INFO "ioreamp bus addr: 0x%x \n", l_addr); 

  l_addr = kmalloc (0x2000, GFP_KERNEL);
  printk (KERN_INFO "kmalloc addr:     0x%x \n", l_addr);
  l_addr = virt_to_phys (l_addr);
  printk (KERN_INFO "kmalloc bus addr: 0x%x \n", l_addr); 

  l_addr = vmalloc (0x2000);
  printk (KERN_INFO "vmalloc addr:     0x%x \n", l_addr);
  l_addr = virt_to_phys (l_addr);
  printk (KERN_INFO "vmalloc bus addr: 0x%x \n", l_addr); 
	*/

  //if ((request_irq (pdev->irq, irq_hand, SA_INTERRUPT, "pexor", irq_hand)) == 0)
  if ((request_irq (pdev->irq, irq_hand, 0, "pexor", irq_hand)) == 0)
  {
		printk (KERN_INFO " IRQ %d request accepted \n", pdev->irq);
  }
  else
  {
		printk (KERN_INFO " ERROR>> IRQ %d request refused \n", l_irq_line);
  }

  #ifdef INTERNAL_TRIG_TEST
  // initalize TRIXOR only for internal tests
  printk (KERN_INFO "\n");
  printk (KERN_INFO " Initalize TRIXOR \n");

  iowrite32 (0x1000,        pl_ctrl);
  iowrite32 (HALT,          pl_ctrl);
  iowrite32 (MASTER,        pl_ctrl);
  iowrite32 (CLEAR,         pl_ctrl);

  iowrite32 (BUS_ENABLE,    pl_ctrl);
  iowrite32 (CLEAR,         pl_ctrl);
  iowrite32 (EV_IRQ_CLEAR,  pl_stat);
  iowrite32 (0x10000 - 14,  pl_fcti);
  iowrite32 (0x10000 - 22,  pl_cvti);

  iowrite32 (HALT,          pl_ctrl);
  iowrite32 (CLEAR,         pl_ctrl);
  iowrite32 (14,            pl_stat);
  iowrite32 ((EN_IRQ | GO), pl_ctrl);

  printk (KERN_INFO " TRIXOR registers content \n");
  printk (KERN_INFO " TRIXOR stat: .... 0x%x \n", ioread32(pl_stat));
  printk (KERN_INFO " TRIXOR ctrl: .... 0x%x \n", ioread32(pl_ctrl));
  printk (KERN_INFO " TRIXOR fcti: .... 0x%x \n", 0x10000 - ioread32(pl_fcti));
  printk (KERN_INFO " TRIXOR cvti: .... 0x%x \n", 0x10000 - ioread32(pl_cvti));
  #endif // INTERNAL_TRIG_TEST 

  printk (KERN_INFO "END   probe function \n");
	return 0;
}
//-----------------------------------------------------------------------------
static void remove(struct pci_dev *pdev)
{
  printk (KERN_INFO " BEGIN remove function \n");

  free_irq (pdev->irq, irq_hand);
  iounmap (l_map_bar0_trix_base);
	release_mem_region (l_bar0_trix_base, 16);

  printk (KERN_INFO " END   remove function \n");
}
//-----------------------------------------------------------------------------
static struct pci_driver pci_driver = {
	.name = "pexor",
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};
//-----------------------------------------------------------------------------
// Set up the char_dev structure for this device.
static void pexor_setup_cdev(struct pexor_dev *dev, int index)
{
  printk (KERN_INFO " BEGIN pexor_setup_cdev function \n");

	int err,  devno = MKDEV(pexor_major, pexor_minor + index);
    
	cdev_init(&dev->cdev, &pexor_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &pexor_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	// Fail gracefully
	if (err)
		printk(KERN_INFO "Error %d adding pexor%d", err, index);
  printk (KERN_INFO " END   pexor_setup_cdev function \n");  
}
//-----------------------------------------------------------------------------
static void __exit pexor_exit(void)
{
  printk (KERN_INFO "\nBEGIN pexor_exit\n");

	int i;
	dev_t devno = MKDEV(pexor_major, pexor_minor);
  
	// Get rid of our char dev entries
	if (pexor_devices)
  {
		for (i = 0; i < pexor_nr_devs; i++)
    {
			cdev_del(&pexor_devices[i].cdev);
		}
		kfree(pexor_devices);
  }

	// cleanup_module is never called if registering failed
	unregister_chrdev_region(devno, pexor_nr_devs);

	pci_unregister_driver(&pci_driver);
  printk (KERN_INFO "END   pexor_exit\n");
}
//-----------------------------------------------------------------------------
static int __init pexor_init(void)
{
	int result, i;
	dev_t dev = 0;

  printk (KERN_INFO "\nBEGIN pexor_init \n");

  // Get a range of minor numbers to work with, asking for a dynamic
  // major unless directed otherwise at load time.

	if (pexor_major)
  {
		dev = MKDEV(pexor_major, pexor_minor);
		result = register_chrdev_region(dev, pexor_nr_devs, "pexor");
	}
  else
  {
		result = alloc_chrdev_region(&dev, pexor_minor, pexor_nr_devs, "pexor");
		pexor_major = MAJOR(dev);
	}
	if (result < 0)
  {
		printk(KERN_INFO "pexor: can't get major %d\n", pexor_major);
		return result;
	}
  printk (KERN_INFO " register / major / minor \n");

	// allocate the devices -- we can't have them static, as the number
	// can be specified at load time
	pexor_devices = kmalloc(pexor_nr_devs * sizeof(struct pexor_dev), GFP_KERNEL);
	if (!pexor_devices)
  {
		result = -ENOMEM;
		goto fail;
	}
	memset(pexor_devices, 0, pexor_nr_devs * sizeof(struct pexor_dev));

  printk (KERN_INFO " initialize pexor device \n");
  // Initialize each device.
	for (i = 0; i < pexor_nr_devs; i++)
  {
		//init_MUTEX(&pexor_devices[i].sem);
		pexor_setup_cdev(&pexor_devices[i], i);
	}

	// At this point call the init function for any friend device
  printk (KERN_INFO " major: %d, minor %d \n", pexor_major, pexor_minor);
	dev = MKDEV(pexor_major, pexor_minor + pexor_nr_devs);

  printk (KERN_INFO " register PCI driver \n");
  printk (KERN_INFO "END pexor_init (no fail)\n");
	return pci_register_driver(&pci_driver); // succeed

  fail:
	pexor_exit();
  return (result);
}
//-----------------------------------------------------------------------------
module_init(pexor_init);
module_exit(pexor_exit);
//-----------------------------------------------------------------------------
