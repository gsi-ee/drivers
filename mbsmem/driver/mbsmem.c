

#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/backing-dev.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>

#include <linux/uaccess.h>

#include <linux/module.h>
#include <linux/version.h>



/**
 * mbsmem driver for x86 Linux - V0.10 on 5-November-2021 by JAM
 * intended for PCs without kinpex/trixor boards to be used as readout for mvlc via usb
 * This is just an extract of mbspex driver to mmap pipe memory,
 * since /dev/mem would require special priviliges
 */

#define MBSMEM_DEBUGPRINT 1

#define MBSMEMVERSION     "0.1.0"
#define MBSMEMAUTHORS     "Joern Adamczewski-Musch (JAM), GSI Darmstadt (www.gsi.de)"
#define MBSMEMDESC        "MBS pipe host memory mapping module for X86 Linux without pexor.ko"



#ifdef MBSMEM_DEBUGPRINT
#define mbsmem_dbg( args... )                    \
  printk( args );
#else
#define mbsmem_dbg( args... ) ;
#endif

/** maximum number of devices controlled by this driver*/
#define MBSMEM_MAXDEVS 4

#define mbsmem_msg( args... )                    \
  printk( args );


/** put all board instance depending things into private data structure:*/
struct mbsmem_privdata
{
    // JAM this is the generic part for all pci drivers
    dev_t devno; /**< device number (major and minor) */
    int devid; /**< local id (counter number) */
    struct device *class_dev; /**< Class device */
    struct cdev cdev; /**< char device struct */
};




/** from mbspex driver to simpplify init exit for generic dev id*/
static dev_t mbsmem_devt;
static atomic_t mbsmem_numdevs = ATOMIC_INIT(0);
static int my_major_nr = 0;


/* these were in privdata, for the moment put it singleton*/
//static struct device *class_dev; /**< Class device */
//static struct cdev cdev; /**< char device struct */
//static dev_t devno; /**< device number (major and minor) */
//static int devid; /**< local id (counter number) */


#define MBSMEMNAME       "mbsmem"
#define MBSMEMNAMEFMT    "mbsmem%d"



#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* mbsmem_class;

// need to keep privdata here for proper exit...
static struct mbsmem_privdata *privdata=0;

ssize_t mbsmem_sysfs_codeversion_show (struct device *dev, struct device_attribute *attr, char *buf)
{

    ssize_t curs=0;
    curs += snprintf (buf + curs, PAGE_SIZE, "*** This is %s, version %s build on %s at %s \n",
        MBSMEMDESC, MBSMEMVERSION, __DATE__, __TIME__);
    curs += snprintf (buf + curs, PAGE_SIZE, "\tmodule authors: %s \n", MBSMEMAUTHORS);

    return curs;
}

static DEVICE_ATTR(codeversion, S_IRUGO, mbsmem_sysfs_codeversion_show, NULL);


#endif









void cleanup_device (struct mbsmem_privdata* priv)
{
  //int j = 0;
  //unsigned long arg=0;
  if (!priv)
    return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (priv->class_dev)
  {
    device_remove_file (priv->class_dev, &dev_attr_codeversion);
    device_destroy (mbsmem_class, priv->devno);
    priv->class_dev = 0;
  }

#endif

  /* character device cleanup*/
  if (priv->cdev.owner)
    cdev_del (&priv->cdev);
  if (priv->devid)
    atomic_dec (&mbsmem_numdevs);
  kfree (priv);
}



//-----------------------------------------------------------------------------
int mbsmem_open (struct inode *inode, struct file *filp)
{
  struct mbsmem_privdata *priv;      // device information
  mbsmem_dbg(KERN_INFO "\nBEGIN mbsmem_open \n");
  priv = container_of(inode->i_cdev, struct mbsmem_privdata, cdev);
  filp->private_data = priv;    // for other methods
  mbsmem_dbg(KERN_INFO "END   mbsmem_open \n");
  return 0;                   // success
}
//-----------------------------------------------------------------------------
int mbsmem_release (struct inode *inode, struct file *filp)
{
  mbsmem_dbg(KERN_INFO "BEGIN mbsmem_release \n");
  mbsmem_dbg(KERN_INFO "END   mbsmem_release \n");
  return 0;
}



int mbsmem_mmap (struct file *filp, struct vm_area_struct *vma)
{
  struct mbsmem_privdata *priv;
  int ret = 0;
  unsigned long bufsize;
  priv = (struct mbsmem_privdata*) filp->private_data;
  mbsmem_dbg(KERN_NOTICE "** starting mbsmem_mmap for vm_start=0x%lx\n", vma->vm_start);
  if (!privdata)
    return -EFAULT;
  bufsize = (vma->vm_end - vma->vm_start);
  mbsmem_dbg(KERN_NOTICE "** starting mbsmem_mmap for size=%ld \n", bufsize);

  if (vma->vm_pgoff == 0)
  {
    /* user does not specify external physical address, we deliver mapping of bar 0:*/
    mbsmem_msg(KERN_NOTICE "** mbsmem_mmap does not support mapping of zero address\n");

  }
  else
  {
    /* for external phys memory, use directly pfn*/
    mbsmem_dbg(
        KERN_NOTICE "Pexor is Mapping external address %lx / PFN %lx\n", (vma->vm_pgoff << PAGE_SHIFT ), vma->vm_pgoff);

    /* JAM tried to check via bios map if the requested region is usable or reserved
     * This will not work, since the e820map as present in Linux kernel was already cut above mem=1024M
     * So we would need to rescan the original bios entries, probably too much effort if standard MBS hardware is known
     * */
    /* phstart=  (u64) vma->vm_pgoff << PAGE_SHIFT;
     phend = phstart +  (u64) bufsize;
     phtype = E820_RAM;
     if(e820_any_mapped(phstart, phend, phtype)==0)
     {
     printk(KERN_ERR "Pexor mmap: requested physical memory region  from %lx to %lx is not completely usable!\n", (long) phstart, (long) phend);
     return -EFAULT;
     }
     NOTE that e820_any_mapped only checks if _any_ memory inside region is mapped
     So it is the wrong method anyway?*/

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,7,0)
    vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
#endif
    ret = remap_pfn_range (vma, vma->vm_start, vma->vm_pgoff, bufsize, vma->vm_page_prot);

  }

  if (ret)
  {
    mbsmem_msg(
        KERN_ERR "Pexor mmap: remap_pfn_range failed with %d\n", ret);
    return -EFAULT;
  }
  return ret;
}



static const struct file_operations mbsmem_fops = {
    .owner = THIS_MODULE,
    .mmap       = mbsmem_mmap,
    .open       = mbsmem_open,
    .release    = mbsmem_release,
};




static int __init mbsmem_init (void)
{

  int result;
  int err = 0;
  char devname[64];

  mbsmem_msg(KERN_NOTICE "mbsmem driver init...\n");
  mbsmem_devt = MKDEV(my_major_nr, 0);

  /*
   * Register your major, and accept a dynamic number.
   */
  if (my_major_nr)
  {
    result = register_chrdev_region (mbsmem_devt, MBSMEM_MAXDEVS, MBSMEMNAME);
  }
  else
  {
    result = alloc_chrdev_region (&mbsmem_devt, 0, MBSMEM_MAXDEVS, MBSMEMNAME);
    my_major_nr = MAJOR(mbsmem_devt);
  }
  if (result < 0)
  {
    mbsmem_msg(
        KERN_ALERT "Could not alloc chrdev region for major: %d !\n", my_major_nr);
    return result;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

  mbsmem_class = class_create (THIS_MODULE, MBSMEMNAME);
  if (IS_ERR (mbsmem_class))
  {
    mbsmem_msg(KERN_ALERT "Could not create class for sysfs support!\n");
  }

#endif


  // following things were in probe of mbspex, but we know that memory is plugged in...
  /* Allocate and initialize the private data for this device */
   privdata = kmalloc (sizeof(struct mbsmem_privdata), GFP_KERNEL);
   if (privdata == NULL )
   {
     cleanup_device (privdata);
     return -ENOMEM;
   }
   memset (privdata, 0, sizeof(struct mbsmem_privdata));


   ////////////////// here chardev registering
    privdata->devid = atomic_inc_return(&mbsmem_numdevs) - 1;
    if (privdata->devid >= MBSMEM_MAXDEVS)
    {
      mbsmem_msg(
          KERN_ERR "Maximum number of devices reached! Increase MAXDEVICES.\n");
      cleanup_device (privdata);
      return -ENOMSG;
    }

    privdata->devno = MKDEV(MAJOR(mbsmem_devt), MINOR(mbsmem_devt) + privdata->devid);

    /* Register character device */
    cdev_init (&(privdata->cdev), &mbsmem_fops);
    privdata->cdev.owner = THIS_MODULE;
    privdata->cdev.ops = &mbsmem_fops;
    err = cdev_add (&privdata->cdev, privdata->devno, 1);
    if (err)
    {
      mbsmem_msg( "Couldn't add character device.\n");
      cleanup_device (privdata);
      return err;
    }


   /* export special things to class in sysfs: */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
   if (!IS_ERR (mbsmem_class))
   {
     /* driver init had successfully created class, now we create device:*/
     snprintf (devname, 64, "mbsmem%d", MINOR(mbsmem_devt) + privdata->devid);
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
     privdata->class_dev =device_create (mbsmem_class, NULL, privdata->devno, NULL, devname);
 #else
     privdata->class_dev ==device_create(mbsmem_class, NULL,
         devno, devname);
 #endif
     dev_set_drvdata (privdata->class_dev, privdata);

     mbsmem_msg (KERN_NOTICE "Added MBSMEM device: %s", devname);

     if (device_create_file (privdata->class_dev, &dev_attr_codeversion) != 0)
     {
       mbsmem_msg (KERN_ERR "Could not add device file node for code version.\n");
     }
   }
     else
       {
         /* something was wrong at class creation, we skip sysfs device support here:*/
         mbsmem_msg(KERN_ERR "Could not add MBSMEM device node to sysfs !");
       }

#endif


  mbsmem_msg(
      KERN_NOTICE "\t\tdriver init with registration for major no %d done. Device name is %s\n",
      my_major_nr, devname);
  return 0;

}

static void __exit mbsmem_exit (void)
{
  mbsmem_msg(KERN_NOTICE "mbsmem driver exit...\n");

// taken from remove of mbspex:
  cleanup_device(privdata); // note that we use global privdata here JAM

// from exit:
  unregister_chrdev_region (mbsmem_devt, MBSMEM_MAXDEVS);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (mbsmem_class != NULL )
    class_destroy (mbsmem_class);
#endif

  mbsmem_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}



//-----------------------------------------------------------------------------
module_init(mbsmem_init);
module_exit(mbsmem_exit);
//-----------------------------------------------------------------------------


MODULE_AUTHOR(MBSMEMAUTHORS);
MODULE_DESCRIPTION(MBSMEMDESC);
MODULE_LICENSE("GPL");
MODULE_VERSION(MBSMEMVERSION);

