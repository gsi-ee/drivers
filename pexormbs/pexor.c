// N.Kurz, EE, GSI, 8-Apr-2010
// J.Adamczewski-Musch, EE, GSI, added mmap and some small fixes 24-Jan-2013
//-----------------------------------------------------------------------------

#define DEBUG

#ifdef DEBUG
 #define debug(x)        printk x
#else
 #define debug(x)
#endif /* DEBUG */

#ifdef DEBUG
#define pexor_dbg( args... )                    \
  printk( args );
#else
#define pexor_dbg( args... ) ;
#endif

#define pexor_msg( args... )                    \
  printk( args );



//#define INTERNAL_TRIG_TEST 1

#define PEXORVERSION     "1.1"
#define PEXORNAME       "pexor"
#define PEXORNAMEFMT    "pexor%d"
/* maximum number of devices controlled by this driver*/
#define PEXOR_MAXDEVS 4

// some register offsets to read out version info:
#define PEXOR_SFP_BASE 0x21000
#define PEXOR_SFP_VERSION 0x1fc
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
#include <linux/version.h>
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
#include <linux/sysfs.h>
//#include <asm/signal.h>
#include <asm/uaccess.h>
#include <asm/io.h>
//#include <asm/bootparam.h> /* check pyhsical memory regions from bios setup*/





//-----------------------------------------------------------------------------
#define PEXOR_VENDOR_ID     0x1204
#define PEXOR_DEVICE_ID     0x5303

#define BAR0_REG_OFF        0x20000  // register base offset with resp. to BAR0
#define BAR0_TRIX_OFF       0x40000  // TRIXOR base offset with resp. to BAR0
#define BAR0_RAM_OFF        0x100000
#define BAR0_SIZE           0x200000
//-----------------------------------------------------------------------------




// put all board instance depending things into private data structure:

struct pexor_privdata
{

   // JAM this is the generic part for all pci drivers
  dev_t devno;                  /* device number (major and minor) */
  int devid;                    /* local id (counter number) */
  char irqname[64];             /* private name for irq */
  struct pci_dev *pdev;         /* PCI device */
  struct device *class_dev;     /* Class device */
  struct cdev cdev;             /* char device struct */
  unsigned long bases[6];       /* contains pci resource bases */
  unsigned long reglen[6];      /* contains pci resource length */
  void *iomem[6];               /* points to mapped io memory of the bars */
  u8  irqpin;                   /* hardware irq pin */
  u8  irqline;                  /* default irq line */
 // here the special variables as used for pexor with mbs:
  u32 l_bar0_base;
  u32 l_bar0_end;
  u32 l_bar0_trix_base;
  u32 l_map_bar0_trix_base;
  struct semaphore trix_sem;
  long             trix_val;
  // trigger module registers
  unsigned int *pl_stat;
  unsigned int *pl_ctrl;
  unsigned int *pl_fcti;
  unsigned int *pl_cvti;
};

static dev_t pexor_devt;

/* counts number of probed pexor devices */
static atomic_t pexor_numdevs=ATOMIC_INIT(0);


/* export something to sysfs:*/
ssize_t pexor_sysfs_codeversion_show(struct device *dev, struct device_attribute *attr, char *buf);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* pexor_class;
static DEVICE_ATTR(codeversion, S_IRUGO, pexor_sysfs_codeversion_show, NULL);
//static DEVICE_ATTR(dmaregs, S_IRUGO, pexor_sysfs_dmaregs_show, NULL);
//
//static DEVICE_ATTR(sfpregs, S_IRUGO, pexor_sysfs_sfpregs_show, NULL);
#endif





static int my_major_nr=0;




MODULE_AUTHOR ("Nikolaus Kurz, Joern Adamczewski-Musch, EE, GSI, 24-Jan-2013");
MODULE_LICENSE("Dual BSD/GPL");



void pexor_show_version(struct pexor_privdata *privdata, char* buf)
{
  /* stolen from pexor_gosip.h*/
  u32 tmp, year,month, day, version[2];
  char txt[1024];
  u32* ptversion=(u32*)(privdata->iomem[0]+PEXOR_SFP_BASE + PEXOR_SFP_VERSION);
  tmp=ioread32(ptversion);
  mb();
  ndelay(20);
  year=((tmp&0xff000000)>>24)+0x2000;
  month=(tmp&0xff0000)>>16;
  day=(tmp&0xff00)>>8;
  version[0]=(tmp&0xf0)>>4;
  version[1]=(tmp&0xf);
  snprintf(txt, 1024,"PEXOR FPGA code compiled at Year=%x Month=%x Date=%x Version=%x.%x \n", year,month,day,version[0],version[1]);
  pexor_dbg(KERN_NOTICE "%s", txt);
  if(buf) snprintf(buf, 1024, "%s",txt);
}





ssize_t pexor_sysfs_codeversion_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  char vstring[1024];
  ssize_t curs=0;
  struct pexor_privdata *privdata;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  curs=snprintf(vstring, 1024, "*** This is PEXOR driver for MBS, Version %s build on %s at %s \n\t", PEXORVERSION, __DATE__, __TIME__);
  pexor_show_version(privdata,vstring+curs);
  return snprintf(buf, PAGE_SIZE, "%s\n", vstring);
}





void test_pci(struct pci_dev *dev)
{
  int bar=0;
  u32 originalvalue=0;
  u32 base=0;
  u16 comstat=0;
  u8 typ=0;
  u8 revision=0;
  u16 vid=0;
  u16 did=0;

  pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
  pexor_dbg(KERN_NOTICE "\n PEXOR: test_pci found PCI revision number 0x%x \n",revision);
  pci_read_config_word (dev, PCI_VENDOR_ID, &vid);
  pexor_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
  pci_read_config_word (dev, PCI_DEVICE_ID, &did);
  pexor_dbg(KERN_NOTICE "  device id:........0x%x \n", did);

  /*********** test the address regions*/
  for(bar=0; bar<6; ++bar){
    pexor_dbg(KERN_NOTICE "Resource %d start=%x\n",bar, (unsigned) pci_resource_start( dev,bar ));
    pexor_dbg(KERN_NOTICE "Resource %d end=%x\n",bar,(unsigned) pci_resource_end( dev,bar ));
    pexor_dbg(KERN_NOTICE "Resource %d len=%x\n",bar,(unsigned) pci_resource_len( dev,bar ));
    pexor_dbg(KERN_NOTICE "Resource %d flags=%x\n",bar,(unsigned) pci_resource_flags( dev,bar ));
    if( (pci_resource_flags(dev,bar) & IORESOURCE_IO) ) {
      // Ressource im IO-Adressraum
      pexor_dbg(KERN_NOTICE " - resource is IO\n");
    }
    if( (pci_resource_flags(dev,bar) & IORESOURCE_MEM) ) {
      pexor_dbg(KERN_NOTICE " - resource is MEM\n");
    }
    if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_SPACE_IO) ) {
      pexor_dbg(KERN_NOTICE " - resource is PCI IO\n");
    }
    if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_SPACE_MEMORY) ) {
      pexor_dbg(KERN_NOTICE " - resource is PCI MEM\n");
    }
    if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_PREFETCH) ) {
      pexor_dbg(KERN_NOTICE " - resource prefetch bit is set \n");
    }
    if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_TYPE_64) ) {
      pexor_dbg(KERN_NOTICE " - resource is 64bit address \n");
    }
    if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_TYPE_32) ) {
      pexor_dbg(KERN_NOTICE " - resource is 32bit address \n");
    }
    if( (pci_resource_flags(dev,bar) & IORESOURCE_PREFETCH) ) {
      pexor_dbg(KERN_NOTICE " - resource is prefetchable \n");
    }
    if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_PREFETCH) ) {
      pexor_dbg(KERN_NOTICE " - resource is PCI mem prefetchable \n");
    }
    if( (pci_resource_flags(dev,bar) & PCI_BASE_ADDRESS_MEM_TYPE_1M) ) {
      pexor_dbg(KERN_NOTICE " - resource is PCI memtype below 1M \n");
    }



  }
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_0, &originalvalue );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_0, 0xffffffff );
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_0, &base );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_0, originalvalue );
  pexor_dbg("size of base address 0: %i\n", ~base+1 );
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_1, &originalvalue );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_1, 0xffffffff );
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_1, &base );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_1, originalvalue );
  pexor_dbg("size of base address 1: %i\n", ~base+1 );
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_2, &originalvalue );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_2, 0xffffffff );
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_2, &base );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_2, originalvalue );
  pexor_dbg("size of base address 2: %i\n", ~base+1 );
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_3, &originalvalue );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_3, 0xffffffff );
  pci_read_config_dword( dev, PCI_BASE_ADDRESS_3, &base );
  pci_write_config_dword( dev, PCI_BASE_ADDRESS_3, originalvalue );
  pexor_dbg("size of base address 3: %i\n", ~base+1 );

  /***** here tests of configuration/status register:******/
  pci_read_config_word(dev, PCI_COMMAND , &comstat);
  pexor_dbg("\n****  Command register is: %d\n", comstat );
  pci_read_config_word(dev, PCI_STATUS , &comstat);
  pexor_dbg("\n****  Status register is: %d\n", comstat );
  pci_read_config_byte(dev, PCI_HEADER_TYPE , &typ);
  pexor_dbg("\n****  Header type is: %d\n", typ );





}



//-----------------------------------------------------------------------------
int pexor_open(struct inode *inode, struct file *filp)
{
	struct pexor_privdata *dev;      // device information
  printk (KERN_INFO "\nBEGIN pexor_open \n");

	dev = container_of(inode->i_cdev, struct pexor_privdata, cdev);
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








//--------------------------

int pexor_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct pexor_privdata *privdata;
//    u64 phstart, phend;
//    unsigned phtype;
    int ret = 0;
    unsigned long bufsize, barsize;
    privdata= (struct pexor_privdata*) filp->private_data;

    printk(KERN_NOTICE "** starting pexor_mmap...\n");

    if(!privdata) return -EFAULT;

    bufsize = (vma->vm_end - vma->vm_start);
    printk(KERN_NOTICE "** starting pexor_mmap for size=%ld \n", bufsize);

    if (vma->vm_pgoff == 0)
        {
            /* user does not specify external physical address, we deliver mapping of bar 0:*/
            printk(KERN_NOTICE "Pexor is Mapping bar0 base address %x / PFN %x\n",
                    privdata->l_bar0_base, privdata->l_bar0_base >> PAGE_SHIFT);
            barsize = privdata->l_bar0_end - privdata->l_bar0_base;
            if (bufsize > barsize)
                {
                    printk(KERN_WARNING "Requested length %ld exceeds bar0 size, shrinking to %ld bytes\n",bufsize,barsize);
                    bufsize = barsize;
                }

            vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
            ret = remap_pfn_range(vma, vma->vm_start, privdata->l_bar0_base >> PAGE_SHIFT,
                    bufsize, vma->vm_page_prot);
        }
    else
        {
            /* for external phys memory, use directly pfn*/
            printk(KERN_NOTICE "Pexor is Mapping external address %lx / PFN %lx\n",
                    (vma->vm_pgoff << PAGE_SHIFT ),vma->vm_pgoff);

            /* tried to check via bios map if the requested region is usable or reserved
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
                So it is the wrong method anyway*/


            vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
            ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, bufsize,
                    vma->vm_page_prot);

        }

    if (ret)
        {
            printk(KERN_ERR "Pexor mmap: remap_pfn_range failed with %d\n", ret);
            //delete_dmabuffer(privdata->pdev, buf);
            return -EFAULT;
        }
    return ret;
}



//-----------------------------------------------------------------------------
int pexor_ioctl(struct inode *inode, struct file *filp,
                 unsigned int cmd, unsigned long arg)
{
	int retval = 0;
	struct pexor_privdata *privdata;
  debug ((KERN_INFO "BEGIN pexor_ioctl \n"));
  privdata= (struct pexor_privdata*) filp->private_data;
  switch (cmd)
  {
    case WAIT_SEM:
      debug ((KERN_INFO " before WAIT_SEM \n"));
      //printk (KERN_INFO " before WAIT_SEM \n");
      down (&(privdata->trix_sem));
      privdata->trix_val = 0;
      //sema_init (&(privdata->trix_sem), 0);
      //init_MUTEX_LOCKED (&(privdata->trix_sem));
      debug ((KERN_INFO " after  WAIT_SEM \n"));
      //printk (KERN_INFO " after  WAIT_SEM \n");
      break;
    case POLL_SEM:
      debug ((KERN_INFO " before POLL_SEM, trix_val: %ld \n", privdata->trix_val));
      //printk (KERN_INFO " before POLL_SEM, trix_val: %d \n", privdata->trix_val);
		  retval = __put_user(privdata->trix_val, (int __user *)arg);
      debug ((KERN_INFO " after POLL_SEM \n"));
				//printk (KERN_INFO " after POLL_SEM \n");
      break;
    case GET_BAR0_BASE:
      printk (KERN_INFO " before GET_BAR0_BASE \n");
		  retval = __put_user(privdata->l_bar0_base, (int __user *)arg);
      printk (KERN_INFO " after  GET_BAR0_BASE \n");
      break;
    case GET_BAR0_TRIX_BASE:
      printk (KERN_INFO " before GET_TRIX_BASE \n");
		  retval = __put_user(privdata->l_bar0_trix_base, (int __user *)arg);
      printk (KERN_INFO " after  GET_TRIX_BASE \n");
      break;
    case RESET_SEM:
      printk (KERN_INFO " before RESET_SEM \n");
      privdata->trix_val = 0;
      //sema_init (&trix_sem, 0); 
      init_MUTEX_LOCKED (&(privdata->trix_sem));
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
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
	.ioctl        = pexor_ioctl,
#else
    .unlocked_ioctl    = pexor_ioctl,
#endif
    .mmap  = 		pexor_mmap,
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
//static unsigned char pexor_get_pci_config(struct pci_dev *pdev)
//{
//	u8 revision;
//
//  printk (KERN_INFO " BEGIN pexor_get_pci_config \n");
//  // read some config parameters
//
//  pci_read_config_word (pdev, PCI_VENDOR_ID, &l_ve_id);
//  printk (KERN_INFO "  vendor id:........0x%x \n", l_ve_id);
//  pci_read_config_word (pdev, PCI_DEVICE_ID, &l_de_id);
//  printk (KERN_INFO "  device id:........0x%x \n", l_de_id);
//	pci_read_config_byte (pdev, PCI_REVISION_ID, &l_rev);
//  printk (KERN_INFO "  revison id:.......0x%x \n", l_rev);
//	pci_read_config_dword (pdev, PCI_BASE_ADDRESS_0, &l_bar0);
//  printk (KERN_INFO "  BAR0 base:........0x%x \n", l_bar0);
//	pci_read_config_byte (pdev, PCI_INTERRUPT_LINE, &l_irq_line);
//  printk (KERN_INFO "  IRQ_LINE:...........%d \n", l_irq_line);
//	pci_read_config_byte (pdev, PCI_INTERRUPT_PIN, &l_irq_pin);
//  printk (KERN_INFO "  IRQ_PIN:............%d \n", l_irq_pin);
//  l_bar0_trix_base = l_bar0 + BAR0_TRIX_OFF;
//  printk (KERN_INFO "  l_bar0_trix_base: 0x%x \n", l_bar0_trix_base);
//
//	pci_read_config_byte(pdev, PCI_REVISION_ID, &revision);
//  printk (KERN_INFO " END pexor_get_pci_config \n");
//	return revision;
//}
//-----------------------------------------------------------------------------
irqreturn_t irq_hand( int irq, void *dev_id)
{

  struct pexor_privdata *privdata;
  debug ((KERN_INFO "BEGIN irq_hand \n"));

  privdata=(struct pexor_privdata *) dev_id;
  disable_irq_nosync (irq);  

  ndelay (1000);

  // clear source of pending interrupts (in trixor)
  iowrite32 ((EV_IRQ_CLEAR | IRQ_CLEAR), privdata->pl_stat);
  //wmb ();
  mb ();

  ndelay (1000);

  enable_irq (irq);

  //ndelay (200);

  privdata->trix_val = 1;
  up (&(privdata->trix_sem));

  #ifdef INTERNAL_TRIG_TEST
	// clear trigger module for test reasons only
  iowrite32 ((EV_IRQ_CLEAR | IRQ_CLEAR), privdata->pl_stat);
  iowrite32 (FC_PULSE,                   privdata->pl_stat);
  iowrite32 (DT_CLEAR,                   privdata->pl_stat);
  #endif //INTERNAL_TRIG_TEST
 
  debug ((KERN_INFO "END   irq_hand \n"));
  //printk (KERN_INFO "END   irq_hand \n"); 
	return IRQ_HANDLED;
}


void cleanup_device(struct pexor_privdata* priv)
{
  int j=0;
  struct pci_dev* pcidev;
  if(!priv) return;



#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (priv->class_dev)
    {
//      device_remove_file(priv->class_dev, &dev_attr_sfpregs);
//      device_remove_file(priv->class_dev, &dev_attr_dmaregs);
      device_remove_file(priv->class_dev, &dev_attr_codeversion);
//      device_remove_file(priv->class_dev, &dev_attr_rcvbufs);
//      device_remove_file(priv->class_dev, &dev_attr_usedbufs);
//      device_remove_file(priv->class_dev, &dev_attr_freebufs);


      device_destroy(pexor_class, priv->devno);
      priv->class_dev=0;
    }

#endif

  /* character device cleanup*/
  if(priv->cdev.owner)
    cdev_del(&priv->cdev);
  if(priv->devid)
    atomic_dec(&pexor_numdevs);

  pcidev = priv->pdev;
  if(!pcidev) return;

  free_irq( pcidev->irq, priv );
  for (j = 0; j < 6; ++j)
    {
      if (priv->bases[j] == 0)
        continue;
      if ((pci_resource_flags(pcidev, j) & IORESOURCE_IO))
        {
          pexor_dbg(KERN_NOTICE " releasing IO region at:%lx -len:%lx \n",priv->bases[j],priv->reglen[j]);
          release_region(priv->bases[j], priv->reglen[j]);
        }
      else
        {
          if (priv->iomem[j] != 0)
            {
              pexor_dbg(KERN_NOTICE " unmapping virtual MEM region at:%lx -len:%lx \n",(unsigned long) priv->iomem[j],priv->reglen[j]);
              iounmap(priv->iomem[j]);
            }
          pexor_dbg(KERN_NOTICE " releasing MEM region at:%lx -len:%lx \n",priv->bases[j],priv->reglen[j]);
          release_mem_region(priv->bases[j], priv->reglen[j]);
        }
      priv->bases[j] = 0;
      priv->reglen[j] = 0;
    }
  kfree(priv);
  pci_disable_device(pcidev);
}




static int probe(struct pci_dev *dev, const struct pci_device_id *id)
{
  int err = 0, ix = 0;
  unsigned char irpin = 0, irline = 0;
  struct pexor_privdata *privdata;
  pexor_msg(KERN_NOTICE "PEXOR pci driver starts probe...\n");
  if ((err = pci_enable_device(dev)) != 0)
    {
      pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d enabling PCI device! \n",err);
      return -ENODEV;
    }
  pexor_dbg(KERN_NOTICE "PEXOR Device is enabled.\n");

  /* Set Memory-Write-Invalidate support */
  if (!pci_set_mwi(dev))
    {
      pexor_dbg(KERN_NOTICE "MWI enabled.\n");
    }
  else
    {
      pexor_dbg(KERN_NOTICE "MWI not supported.\n");
    }
  pci_set_master(dev); /* NNOTE: DMA worked without, but maybe depends on bios...*/


  test_pci(dev);

  /* Allocate and initialize the private data for this device */
  privdata = kmalloc(sizeof(struct pexor_privdata), GFP_KERNEL);
  if (privdata == NULL)
    {
      cleanup_device(privdata);
      return -ENOMEM;
    }
  memset(privdata, 0, sizeof(struct pexor_privdata));
  pci_set_drvdata(dev, privdata);
  privdata->pdev = dev;


  for (ix = 0; ix < 6; ++ix)
    {
      privdata->bases[ix] = pci_resource_start(dev, ix);
      privdata->reglen[ix] = pci_resource_len(dev, ix);
      if (privdata->bases[ix] == 0)
        continue;
      if ((pci_resource_flags(dev, ix) & IORESOURCE_IO))
        {

          pexor_dbg(KERN_NOTICE " - Requesting io ports for bar %d\n",ix);
          if (request_region(privdata->bases[ix], privdata->reglen[ix],
                 kobject_name(&dev->dev.kobj)) == NULL)
            {
              pexor_dbg(KERN_ERR "I/O address conflict at bar %d for device \"%s\"\n",
            ix, kobject_name(&dev->dev.kobj));
              cleanup_device(privdata);
              return -EIO;
            }pexor_dbg("requested ioport at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
        }
      else if ((pci_resource_flags(dev, ix) & IORESOURCE_MEM))
        {
          pexor_dbg(KERN_NOTICE " - Requesting memory region for bar %d\n",ix);
          if (request_mem_region(privdata->bases[ix], privdata->reglen[ix],
                 kobject_name(&dev->dev.kobj)) == NULL)
            {
              pexor_dbg(KERN_ERR "Memory address conflict at bar %d for device \"%s\"\n",
            ix, kobject_name(&dev->dev.kobj));
              cleanup_device(privdata);
              return -EIO;
            }pexor_dbg("requested memory at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
          privdata->iomem[ix] = ioremap_nocache(privdata->bases[ix],
                        privdata->reglen[ix]);
          if (privdata->iomem[ix] == NULL)
            {
              pexor_dbg(KERN_ERR "Could not remap memory  at bar %d for device \"%s\"\n",
            ix, kobject_name(&dev->dev.kobj));
              cleanup_device(privdata);
              return -EIO;
            }pexor_dbg("remapped memory to %lx with length %lx\n", (unsigned long) privdata->iomem[ix], privdata->reglen[ix]);
        }
    } //for

// here set custom registers for mbs implementation:
  privdata->l_bar0_base=privdata->bases[0];
  privdata->l_bar0_end=privdata->bases[0]+privdata->reglen[0];

  printk (KERN_INFO " Assigning TRIXOR registers \n");
  privdata->l_bar0_trix_base=privdata->bases[0]+BAR0_TRIX_OFF;
  privdata->l_map_bar0_trix_base= (u32) privdata->iomem[0]+BAR0_TRIX_OFF;
      // map TRIXOR register
  privdata->pl_stat = (unsigned int*) ((long) privdata->l_map_bar0_trix_base + 0x0);
  privdata->pl_ctrl = (unsigned int*) ((long) privdata->l_map_bar0_trix_base + 0x4);
  privdata->pl_fcti = (unsigned int*) ((long) privdata->l_map_bar0_trix_base + 0x8);
  privdata->pl_cvti = (unsigned int*) ((long) privdata->l_map_bar0_trix_base + 0xc);

  printk (KERN_INFO " Ptr. TRIXOR stat: 0x%x \n", (unsigned int)privdata->pl_stat);
  printk (KERN_INFO " Ptr. TRIXOR ctrl: 0x%x \n", (unsigned int)privdata->pl_ctrl);
  printk (KERN_INFO " Ptr. TRIXOR fcti: 0x%x \n", (unsigned int)privdata->pl_fcti);
  printk (KERN_INFO " Ptr. TRIXOR cvti: 0x%x \n", (unsigned int)privdata->pl_cvti);

  printk (KERN_INFO " TRIXOR registers content \n");
  printk (KERN_INFO " TRIXOR stat: .... 0x%x \n", ioread32(privdata->pl_stat));
  printk (KERN_INFO " TRIXOR ctrl: .... 0x%x \n", ioread32(privdata->pl_ctrl));
  printk (KERN_INFO " TRIXOR fcti: .... 0x%x \n", 0x10000 - ioread32(privdata->pl_fcti));
  printk (KERN_INFO " TRIXOR cvti: .... 0x%x \n", 0x10000 - ioread32(privdata->pl_cvti));

  printk (KERN_INFO " Initialize mutex in locked state \n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
      init_MUTEX_LOCKED(&(privdata->trix_sem));
#else
      sema_init (&(privdata->trix_sem), 0);
#endif
      privdata->trix_val = 0;




  /* debug: do we have valid ir pins/lines here?*/
  if ((err = pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &(privdata->irqpin))) != 0)
    {
      pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d getting the PCI interrupt pin \n",err);
    }
  if ((err = pci_read_config_byte(dev, PCI_INTERRUPT_LINE, &(privdata->irqline))) != 0)
    {
      pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d getting the PCI interrupt line.\n",err);
    }
  snprintf(privdata->irqname, 64, PEXORNAMEFMT,atomic_read(&pexor_numdevs));
  if(request_irq(dev->irq, irq_hand , IRQF_SHARED, privdata->irqname, privdata))
    {
      pexor_msg( KERN_ERR "PEXOR pci_drv: IRQ %d not free.\n", dev->irq );
      cleanup_device(privdata);
      return -EIO;
    }
  pexor_msg(KERN_NOTICE " assigned IRQ %d for name %s, pin:%d, line:%d \n",dev->irq, privdata->irqname,irpin,irline);


  ////////////////// here chardev registering
  privdata->devid = atomic_inc_return(&pexor_numdevs) - 1;
  if (privdata->devid >= PEXOR_MAXDEVS)
    {
      pexor_msg(KERN_ERR "Maximum number of devices reached! Increase MAXDEVICES.\n");
      cleanup_device(privdata);
      return -ENOMSG;
    }

  privdata->devno
    = MKDEV(MAJOR(pexor_devt), MINOR(pexor_devt) + privdata->devid);

  /* Register character device */
  cdev_init(&(privdata->cdev), &pexor_fops);
  privdata->cdev.owner = THIS_MODULE;
  privdata->cdev.ops = &pexor_fops;
  err = cdev_add(&privdata->cdev, privdata->devno, 1);
  if (err)
    {
      pexor_msg( "Couldn't add character device.\n" );
      cleanup_device(privdata);
      return err;
    }

  /* export special things to class in sysfs: */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (!IS_ERR(pexor_class))
    {
      /* driver init had successfully created class, now we create device:*/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
      privdata->class_dev = device_create(pexor_class, NULL, privdata->devno,
                      privdata, PEXORNAMEFMT, MINOR(pexor_devt) + privdata->devid);
#else
      privdata->class_dev = device_create(pexor_class, NULL, privdata->devno,
                      PEXORNAMEFMT, MINOR(pexor_devt) + privdata->devid);
#endif
      dev_set_drvdata(privdata->class_dev, privdata);
      pexor_msg(KERN_NOTICE "Added PEXOR device: ");
      pexor_msg(KERN_NOTICE PEXORNAMEFMT, MINOR(pexor_devt) + privdata->devid);

//#ifdef PEXOR_SYSFS_ENABLE
//
//      if(device_create_file(privdata->class_dev, &dev_attr_freebufs) != 0)
//        {
//          pexor_msg(KERN_ERR "Could not add device file node for free buffers.\n");
//        }
//      if(device_create_file(privdata->class_dev, &dev_attr_usedbufs) != 0)
//        {
//          pexor_msg(KERN_ERR "Could not add device file node for used buffers.\n");
//        }
//      if(device_create_file(privdata->class_dev, &dev_attr_rcvbufs) != 0)
//    {
//      pexor_msg(KERN_ERR "Could not add device file node for receive buffers.\n");
//    }
//
      if(device_create_file(privdata->class_dev, &dev_attr_codeversion) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for code version.\n");
    }
//
//      if(device_create_file(privdata->class_dev, &dev_attr_dmaregs) != 0)
//        {
//          pexor_msg(KERN_ERR "Could not add device file node for dma registers.\n");
//        }
//#ifdef PEXOR_WITH_SFP
//      if(device_create_file(privdata->class_dev, &dev_attr_sfpregs) != 0)
//    {
//      pexor_msg(KERN_ERR "Could not add device file node for sfp registers.\n");
//    }
//#endif
//
//#endif



#ifdef INTERNAL_TRIG_TEST
        // initalize TRIXOR only for internal tests
      printk (KERN_INFO "\n");
      printk (KERN_INFO " Initalize TRIXOR \n");

      iowrite32 (0x1000, privdata->pl_ctrl);
      iowrite32 (HALT, privdata->pl_ctrl);
      iowrite32 (MASTER, privdata->pl_ctrl);
      iowrite32 (CLEAR, privdata->pl_ctrl);

      iowrite32 (BUS_ENABLE, privdata->pl_ctrl);
      iowrite32 (CLEAR, privdata->pl_ctrl);
      iowrite32 (EV_IRQ_CLEAR, privdata->pl_stat);
      iowrite32 (0x10000 - 14, privdata->pl_fcti);
      iowrite32 (0x10000 - 22, privdata->pl_cvti);

      iowrite32 (HALT, privdata->pl_ctrl);
      iowrite32 (CLEAR, privdata->pl_ctrl);
      iowrite32 (14, privdata->pl_stat);
      iowrite32 ((EN_IRQ | GO), privdata->pl_ctrl);

      printk (KERN_INFO " TRIXOR registers content \n");
      printk (KERN_INFO " TRIXOR stat: .... 0x%x \n", ioread32(privdata->pl_stat));
      printk (KERN_INFO " TRIXOR ctrl: .... 0x%x \n", ioread32(privdata->pl_ctrl));
      printk (KERN_INFO " TRIXOR fcti: .... 0x%x \n", 0x10000 - ioread32(privdata->pl_fcti));
      printk (KERN_INFO " TRIXOR cvti: .... 0x%x \n", 0x10000 - ioread32(privdata->pl_cvti));
#endif // INTERNAL_TRIG_TEST


    }
  else
    {
      /* something was wrong at class creation, we skip sysfs device support here:*/
      pexor_msg(KERN_ERR "Could not add PEXOR device node to /dev !");
    }

#endif

  pexor_msg(KERN_NOTICE "probe has finished.\n");
  return 0;



}

static void remove(struct pci_dev *dev)
{
  struct pexor_privdata* priv = (struct pexor_privdata*) pci_get_drvdata(dev);
  cleanup_device(priv);

  pexor_msg(KERN_NOTICE "PEXOR pci driver end remove.\n");
}









//-----------------------------------------------------------------------------
static struct pci_driver pci_driver = {
	.name = PEXORNAME,
	.id_table = ids,
	.probe = probe,
	.remove = remove,
};
//-----------------------------------------------------------------------------


static int __init pexor_init(void)
{

  int result;
  pexor_msg(KERN_NOTICE "pexor driver init...\n");

  pexor_devt  = MKDEV(my_major_nr, 0);

  /*
   * Register your major, and accept a dynamic number.
   */
  if (my_major_nr)
    result = register_chrdev_region(pexor_devt, PEXOR_MAXDEVS, PEXORNAME);
  else {
    result = alloc_chrdev_region(&pexor_devt, 0, PEXOR_MAXDEVS, PEXORNAME);
    my_major_nr = MAJOR(pexor_devt);
  }
  if (result < 0)
      {
          pexor_msg(KERN_ALERT "Could not alloc chrdev region for major: %d !\n",my_major_nr);
          return result;
      }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  pexor_class = class_create(THIS_MODULE, PEXORNAME);
  if (IS_ERR(pexor_class))
    {
      pexor_msg(KERN_ALERT "Could not create class for sysfs support!\n");
    }

#endif

  if(pci_register_driver(&pci_driver) < 0 )
    {
      pexor_msg(KERN_ALERT  "pci driver could not register!\n");
      unregister_chrdev_region(pexor_devt, PEXOR_MAXDEVS);
      return -EIO;
    }

  pexor_msg(KERN_NOTICE "\t\tdriver init with registration for major no %d done.\n",my_major_nr);
  return 0;

  /* note: actual assignment will be done on probe time*/


}

static void __exit pexor_exit(void)
{
  pexor_msg(KERN_NOTICE "pexor driver exit...\n");

  unregister_chrdev_region(pexor_devt, PEXOR_MAXDEVS);
  pci_unregister_driver(&pci_driver);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
if (pexor_class != NULL)
      class_destroy(pexor_class);
#endif

  pexor_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}




//-----------------------------------------------------------------------------
module_init(pexor_init);
module_exit(pexor_exit);
//-----------------------------------------------------------------------------
