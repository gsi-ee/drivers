// N.Kurz, EE, GSI, 28-Nov-2012: based on template from J.-F.Gilot IOxOS  
// J.Adamczewski-Musch (JAM) - transformed ipv implementation for ifc 4-Sep-2020
// J.Adamczewski-Musch (JAM) - added mmap for triva registers 30-Apr-2021


#define VMETRIGMODVERSION     "0.2.0"
#define VMETRIGMODAUTHORS     "Joern Adamczewski-Musch (JAM), Nikolaus Kurz, GSI Darmstadt (www.gsi.de)"
#define VMETRIGMODDESC        "TRIVA VMEbus trigger module of MBS for IFC Linux"






//#define INTERNAL_TRIG_TEST 1

//#define DBG

#ifdef DBG
#define debugk(x) printk x
#else
#define debugk(x) 
#endif

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#include <asm/uaccess.h>         // copy_to_user and copy_from_user
#include <linux/init.h>          // modules
#include <linux/module.h>        // module
#include <linux/types.h>         // dev_t type
#include <linux/fs.h>            // chrdev allocation
#include <linux/slab.h>          // kmalloc and kfree
#include <linux/cdev.h>          // struct cdev
#include <linux/errno.h>         // error codes
#include <linux/pci.h>
#include <linux/mm.h>
#include <linux/interrupt.h>

#include <linux/device.h>
#include <linux/delay.h>


#include <linux/pagemap.h>
#include <linux/page-flags.h>
#include <linux/version.h>

//#include "/usr/src/PEV1100/include/pevioctl.h"
//#include "/usr/src/PEV1100/include/vmeioctl.h"
//#include "/usr/src/PEV1100/drivers/pevdrvr.h"
//#include "/usr/src/PEV1100/drivers/pevklib.h"

// guess that these things ar alike what we know from ipc:
#include "/mbs/driv/ifc/althea/ALTHEA7910/include/altvme.h"
#include "/mbs/driv/ifc/althea/ALTHEA7910/include/altioctl.h"
#include "/mbs/driv/ifc/althea/ALTHEA7910/driver/altdrvr.h"
#include "/mbs/driv/ifc/althea/ALTHEA7910/driver/altklib.h"

#include "/mbs/driv/ifc/althea/ALTHEA7910/include/altmasioctl.h"
#include "/mbs/driv/ifc/althea/ALTHEA7910/driver/altmasdrvr.h"


#define TRIVA_BUS_DELAY 20

#define triva_bus_delay()                       \
  mb();      \
  ndelay(TRIVA_BUS_DELAY);




/* declare function to register to althea7910 control driver */
extern void *althea7910_register( void);
// JAM - exported kernel symbol without include file

/* declare altklib exported functions */
//extern int alt_map_mas_alloc( void *alt, struct alt_ioctl_map_win *w);
//extern int alt_map_mas_get( void *alt, struct alt_ioctl_map_win *w);
//extern int alt_map_mas_free( void *alt, int sgid, uint pg_idx);
//extern void alt_irq_register( void *alt, int src, void (* func)( void *, int, void *), void *arg);
//extern void alt_irq_unregister( void *alt, int src);
//extern void alt_irq_mask( void *alt, int op, int src);
//extern int alt_wait_wpbusy( void *alt, int tmo);
//

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

#define TRIGMOD_IRQ_VECTOR  0x50 
#define TRIGMOD_IRQ_LEVEL     4
#define TRIGMOD_VME_AM      0x9 
#define TRIGMOD_REGS_ADDR   0x2000000
#define TRIGMOD_REGS_SIZE   0x1000

static struct semaphore triv_sem;
static int              triv_val;  

// trigger module register
static unsigned int *pl_stat;
static unsigned int *pl_ctrl;
static unsigned int *pl_fcti;
static unsigned int *pl_cvti;





//  TODO the following must be exchanged by altea structures
//struct pev_drv *pev_drv_p;
//struct pev_dev *pev;
//uint vme_itc_reg = 0;
//char *vme_cpu_addr = NULL;
//struct vme_board vme_board;

//struct althea7910_device* althea;

// JAM20202- this is copy of mechanisms in altmas driver
struct altmas altmas;

static const char device_name[] = "trigmod";
static struct class *vme_sysfs_class;   /* Sysfs class */

static int altmas_dev_num = 1;
static struct device* class_dev[1]; /**< Class device */
static int altmas_irq_ena[8] = {0,0,0,0,0,0,0,0};
static int altmas_irq_cnt = 0;
static struct altmas_ivec_tbl *altmas_ivec_tbl;

// JAM 4-2021: probably we should put all of  this into private data; as long as we have only one triva, it should also work such...
static struct altmas_device *mas;
static struct alt_ioctl_mas_map *masmap;
static struct alt_ioctl_map_win map_win;

static int triva_open_count=0; /* only mask interrupts when last filehandle is closed */

char *triva_base_addr = NULL;



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
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
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


//void
//altmas_irq( void *p,
//        int iack,
//        void *arg)


// JAM - here we change declaration to be consistent with "official" include
// this was simplified by local declarations in altmasdrv.c
void altmas_irq( struct althea7910_device *p,
        int iack,
        void *arg)
{
  int ivec, src;
  struct altmas *am;
  struct altmas_device *mas;

  am = (struct altmas *)arg;

  ivec = ITC_IACK_VEC(iack);
  src = ITC_IACK_SRC(iack);
  debugk(("ALTHEA master interrupt : %x : %x - %p - %p\n", src, ivec, arg, am->alt));
  /* get pointer to device control structure */
  mas = altmas_ivec_tbl->mas[ivec];
  if( mas)
  {
//    /* increment interrupt pending counter */
    //mas->ip_cnt += 1;
//    /* wake up event queue */
//    wake_up_interruptible( &mas->queue);
// replace generic event queue of altmas driver by the triva semaphore for mbs:
    // clear source of pending interrupts (in triva)
    *pl_stat = (EV_IRQ_CLEAR | IRQ_CLEAR);
    mb ();

    triv_val = 1;
    debugk ((KERN_INFO "Set Semaphore \n"));
    up (&triv_sem);

    #ifdef INTERNAL_TRIG_TEST
    // clear trigger module for test reasons only
    *pl_stat = (EV_IRQ_CLEAR | IRQ_CLEAR);
    *pl_stat = FC_PULSE;
    *pl_stat = DT_CLEAR;
    #endif //INTERNAL_TRIG_TEST

    debugk ((KERN_INFO "END   irq_hand \n"));


  }
  /* unmasq IRQ */
  alt_irq_mask( am->alt, ALT_IOCTL_ITC_MSK_CLEAR, src);

  return;
}







/*
  Ioctl callback
*/
long
trigmod_ioctl( struct file *filp, 
	  unsigned int cmd, 
	  unsigned long arg)
{
  int retval = 0;              // Will contain the result

  debugk(("trigmod_ioctl: %x - %lx\n", cmd, arg));
  switch (cmd) 
  {
    case WAIT_SEM:
      debugk ((KERN_INFO " before WAIT_SEM \n"));
      debugk ((KERN_INFO "Release Semaphore \n"));   
      down (&triv_sem);
      triv_val = 0;
      debugk ((KERN_INFO " after  WAIT_SEM \n"));
      break;
    case POLL_SEM:
      debugk ((KERN_INFO " before POLL_SEM, triv_val: %d \n", triv_val));
		  retval = __put_user(triv_val, (int __user *)arg);
      debugk ((KERN_INFO " after POLL_SEM \n"));
      break;
    case RESET_SEM:
      printk (KERN_INFO " before RESET_SEM \n");
      triv_val = 0;
	    sema_init (&triv_sem, 0);
      //init_MUTEX_LOCKED (&triv_sem);
		  retval = __put_user(0, (int __user *)arg);
      printk (KERN_INFO " after  RESET_SEM \n");
      break;

    default:
    {
      retval = -EINVAL;
    }
  }
  return retval;
}


/*
Open callback
*/
int 
trigmod_open( struct inode *inode, 
	  struct file *filp)
{
  debugk(( KERN_ALERT "vme: entering trigmod_open\n"));
  triva_open_count++;
  // JAM2021: if we open file handles also for mmap, this functionality is more likely to be put into ioctl
  // for the moment, try workaroung with used files counter
  if(triva_open_count==1)
  {
    printk ( KERN_ALERT "unmask vme interupt level 4, vector 0x50, open count is %d\n",triva_open_count);
    if( (altmas_irq_ena[0] > 0) && (altmas_irq_ena[0] < 8))
      {
        int src;
        src = ITC_SRC_VME_IRQ1 + altmas_irq_ena[0] - 1;            /* get irq source identifier */
        alt_irq_mask( altmas.alt, ALT_IOCTL_ITC_MSK_CLEAR, src);  /* unmask irq source          */
      }
  }
  return(0);
}


/*
Release callback
*/
int trigmod_release(struct inode *inode, struct file *filp)
{
  debugk(( KERN_ALERT "vme: entering trigmod_release\n"));
  triva_open_count--;
  // JAM2021: if we open file handles also for mmap, this functionality is more likely to be put into ioctl
  // for the moment, try workaroung with used files counter
  if(triva_open_count<=0)
  {
    printk ( KERN_ALERT "mask all vme interupts, open count is %d\n",triva_open_count);
    if( (altmas_irq_ena[0] > 0) && (altmas_irq_ena[0] < 8))
     {
       int src;
       src = ITC_SRC_VME_IRQ1 + altmas_irq_ena[0] - 1;
       alt_irq_mask( altmas.alt, ALT_IOCTL_ITC_MSK_SET, src);
     }
  }
  return 0;
}



int trigmod_mmap (struct file *filp, struct vm_area_struct *vma)
{
  int ret = 0;
  unsigned long bufsize;
  printk(KERN_NOTICE "** starting trigmod_mmap for vm_start=0x%lx\n", vma->vm_start);
//  if (!privdata)
//    return -EFAULT;
  bufsize = (vma->vm_end - vma->vm_start);
  printk(KERN_NOTICE "** starting trigmod_mmap for size=%ld \n", bufsize);

    /* user needs not specify external physical address, we always deliver existing mapping of triva base*/
    debugk((
        KERN_NOTICE "trigmod is Mapping triva base address %x / PFN %x\n", masmap->loc_addr, masmap->loc_addr >> PAGE_SHIFT));
    if (bufsize > TRIGMOD_REGS_SIZE)
    {
      printk(
          KERN_WARNING "Requested length %ld exceeds provided triva map window size, shrinking to %d bytes\n", bufsize, TRIGMOD_REGS_SIZE);
      bufsize = TRIGMOD_REGS_SIZE;
    }

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,7,0)
    vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
#endif

    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot); // taken from altmas driver mmap
    ret = remap_pfn_range (vma, vma->vm_start,  masmap->loc_addr >> PAGE_SHIFT, bufsize, vma->vm_page_prot);
    if (ret)
     {
       printk(
           KERN_ERR "trigmod mmap: remap_pfn_range failed with %d\n", ret);
       return -EFAULT;
     }
     return ret;


  }







// File operations for althea master map device
struct file_operations altmas_fops =
{
  .owner =    THIS_MODULE,
  .unlocked_ioctl =    trigmod_ioctl,
  .mmap = trigmod_mmap,
  .open =     trigmod_open,
  .release =  trigmod_release,
};





static int trigmod_init( void)
{
  int retval;
  dev_t altmas_dev_id;
  int i, vme_major;
//  struct altmas_device *mas;
//  struct alt_ioctl_mas_map *masmap;
//  struct alt_ioctl_map_win map_win;
  debugk(( KERN_ALERT "trigmod: entering altmas_init( void)\n"));
   /*--------------------------------------------------------------------------
   * device number dynamic allocation
   *--------------------------------------------------------------------------*/


  retval = alloc_chrdev_region( &altmas_dev_id, 0, 1, "trigvme");
    if( retval < 0)
    {
      debugk(( KERN_WARNING "trigmod: cannot allocate device number\n"));
      goto altmas_init_err_alloc_chrdev;
    }
    else
    {
      debugk((KERN_WARNING "trigmod: registered with major number:%i\n", MAJOR( altmas_dev_id)));
    }



  altmas.dev_id = altmas_dev_id;
  /*--------------------------------------------------------------------------
   * register driver
   *--------------------------------------------------------------------------*/
  cdev_init( &altmas.cdev, &altmas_fops);
  altmas.cdev.owner = THIS_MODULE;
  altmas.cdev.ops = &altmas_fops;
  retval = cdev_add( &altmas.cdev, altmas.dev_id, altmas_dev_num);
  if(retval) {
    debugk((KERN_NOTICE "trigmod : Error %d adding device\n", retval));
    goto altmas_init_err_cdev_add;
  }
  debugk((KERN_NOTICE "trigmod : device added\n"));

  /*--------------------------------------------------------------------------
   * Create sysfs entries - on udev systems this creates the dev files
   *--------------------------------------------------------------------------*/
  vme_sysfs_class = class_create( THIS_MODULE, device_name);
  if (IS_ERR( vme_sysfs_class))
  {
    retval = PTR_ERR( vme_sysfs_class);
    goto altmas_err_class;
  }
  /*--------------------------------------------------------------------------
   * Create ALTHEA control device in file system
   *--------------------------------------------------------------------------*/
  vme_major = MAJOR( altmas_dev_id);
  altmas.mas = (struct altmas_device **)kzalloc(altmas_dev_num*sizeof(struct altmas_device *), GFP_KERNEL);

  // JAM: for triva, we use only one device:
  for( i = 0; i < altmas_dev_num; i++)
  {
     char name[32];
      altmas.mas[i]  = (struct altmas_device *)kzalloc(altmas_dev_num*sizeof(struct altmas_device), GFP_KERNEL);
//    sprintf(name, "bus/vme/alt_mas%%d");
      sprintf(name, "triva%d",i);
      class_dev[i]=device_create( vme_sysfs_class, NULL, MKDEV( vme_major, i), NULL, name, i);
    mutex_init( &altmas.mas[i]->mutex);

    // JAM2020 new - add sysfs info here
    if (device_create_file (class_dev[i], &dev_attr_codeversion) != 0)
    {
        debugk (KERN_ERR "Could not add device file node for code version.\n");
    }
    if (device_create_file (class_dev[i], &dev_attr_trivaregs) != 0)
    {
        debugk (KERN_ERR "Could not add device file node for trivaregs.\n");
    }

  }




  /*--------------------------------------------------------------------------
   * Get handle to access altklib functions
   *--------------------------------------------------------------------------*/
  altmas.alt = althea7910_register();

  mas=altmas.mas[0]; // JAM just for convenience
  if( !mas->use_cnt)
   {
     mas->alt = altmas.alt;
     debugk(("altmas: ALTHEA control device handle: %p\n", mas->alt));
     mas->map_p = (struct alt_ioctl_mas_map *)kzalloc( sizeof(struct alt_ioctl_mas_map), GFP_KERNEL);
     mas->done = 0;
     mas->pg_idx = MAS_MAP_IDX_INV;
     mas->map_p->ivec = MAS_MAP_NO_IVEC;    /* interrupt vector not yet assigned */
   }
   /* increment device use use_cnt */
   mas->use_cnt += 1;



  ////  vme_board.base = TRIGMOD_REGS_ADDR;
  ////  vme_board.am   = TRIGMOD_VME_AM;
  ////  vme_board.size = TRIGMOD_REGS_SIZE;
  ////  vme_board.irq  = TRIGMOD_IRQ_LEVEL;
  ////  vme_board.vec  = TRIGMOD_IRQ_VECTOR;
  // TODO: here map vme registers

  // JAM from ioctl set map of altmasdrv:

  /* point to device mapping control structure */

   masmap = mas->map_p;
   debugk(("mas = %p - pg_idx = %x\n", altmas.mas, altams.mas->pg_idx));

   masmap->ivec = TRIGMOD_IRQ_VECTOR;
   masmap->size = TRIGMOD_REGS_SIZE;
   masmap->mode = TRIGMOD_VME_AM;


  //
  map_win.req.rem_addr = TRIGMOD_REGS_ADDR;
  map_win.req.loc_addr = MAP_LOC_ADDR_AUTO;
  map_win.req.size = masmap->size;
  map_win.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
  map_win.req.mode.am = (char)(masmap->mode & VME_AM_MASK);
  map_win.req.mode.space = MAP_SPACE_VME;
  map_win.req.mode.swap = (char)((masmap->mode & MAS_MAP_MODE_SWAP_MASK)>>8);


  if( masmap->mode & MAS_MAP_VME_SPLIT)
       {
         map_win.req.mode.swap |= MAP_SPLIT_D32;
       }
  map_win.req.mode.flags = 0;
  mas->pg_idx = alt_map_mas_alloc( mas->alt, &map_win);
  masmap->pg_idx = map_win.pg_idx;
  if( mas->pg_idx < 0)
       {
         retval = -ENOMEM;
             debugk(("cannot allocate map: loc_addr = %lx\n", map_win.req.loc_addr));
             goto altmas_init_err_alloc_map;
       }
  else
  {
         /* set mapping done */
    //mas->done = 1;
    debugk(("pg_idx = %x - loc_addr = %lx\n", mas->pg_idx, m->loc_addr));
    alt_map_mas_get( mas->alt, &map_win);
    masmap->rem_addr = map_win.sts.rem_base;
    masmap->loc_addr = map_win.sts.loc_base;
    masmap->size =  map_win.sts.size;
    debugk(("size = %x - loc_addr = %lx rem_addr = %lx\n", m->size, m->loc_addr, m->rem_addr));


    // map althea loc_address to kernel addresses:
    triva_base_addr = ioremap_nocache (masmap->loc_addr, TRIGMOD_REGS_SIZE);
    if (!triva_base_addr)
      {
        retval = -ENOMEM;
        goto altmas_init_err_map;
      }
  }

  printk (KERN_INFO " map TRIVA registers \n");
  //  // map TRIVA register
    pl_stat = (unsigned int*) ((long)triva_base_addr + 0x0);
    pl_ctrl = (unsigned int*) ((long)triva_base_addr + 0x4);
    pl_fcti = (unsigned int*) ((long)triva_base_addr + 0x8);
    pl_cvti = (unsigned int*) ((long)triva_base_addr + 0xc);

    printk (KERN_INFO " Ptr. TRIVA stat: 0x%lx \n", (unsigned long)pl_stat);
    printk (KERN_INFO " Ptr. TRIVA ctrl: 0x%lx \n", (unsigned long)pl_ctrl);
    printk (KERN_INFO " Ptr. TRIVA fcti: 0x%lx \n", (unsigned long)pl_fcti);
    printk (KERN_INFO " Ptr. TRIVA cvti: 0x%lx \n", (unsigned long)pl_cvti);

    printk (KERN_INFO " TRIVA registers content \n");
    printk (KERN_INFO " TRIVA stat: .... 0x%x \n", *(pl_stat));
    printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", *(pl_ctrl));
    printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0x10000 - *(pl_fcti));
    printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0x10000 - *(pl_cvti));

    sema_init (&triv_sem, 0);

  /*--------------------------------------------------------------------------
   * Register interrupt handlers for ALTHEA master
   *--------------------------------------------------------------------------*/

  // We adjust following most generic form (ruled by kernel parms in altmasdrv) to one device: JAM2020
  altmas_irq_cnt=1;
  altmas_irq_ena[0] = TRIGMOD_IRQ_LEVEL; // ?


  if( altmas_irq_cnt > 8) altmas_irq_cnt = 8;
  for( i = 0; i < altmas_irq_cnt; i++)
  {
    if( (altmas_irq_ena[i] > 0) && (altmas_irq_ena[i] < 8))
    {
      int src;

      src = ITC_SRC_VME_IRQ1 + altmas_irq_ena[i] - 1;            /* get irq source identifier */
      alt_irq_register((struct althea7910_device*) altmas.alt, src, altmas_irq, &altmas);  /* register interrupt handler */
      //alt_irq_mask( altmas.alt, ALT_IOCTL_ITC_MSK_CLEAR, src);  /* unmask irq source          */
      alt_irq_mask( altmas.alt, ALT_IOCTL_ITC_MSK_SET, src); // do not activate ir immediately
      printk (KERN_INFO " registered irq of index %d for src:0x%x \n",altmas_irq_cnt, src);
    }
  }
  altmas_ivec_tbl = (struct altmas_ivec_tbl *)kzalloc(sizeof(struct altmas_ivec_tbl), GFP_KERNEL);


  // need this?
  /* assign interrupt vector */
//      if( m->ivec != MAS_MAP_NO_IVEC)
//      {
//        /* assign ivec in vector table */
//        altmas_ivec_tbl->mas[m->ivec] = mas;
//
//            /* initialize interrupt wait queue */
//            debugk(("altmas: ALTHEA initialize wait queue: %p\n", &mas->queue));
//            init_waitqueue_head( &mas->queue);
//        mas->ip_cnt = 0;   /* reset interrupt pending counter    */
//        mas->done = 1;
//      }

  altmas_ivec_tbl->mas[masmap->ivec] = mas;
  mas->ip_cnt = 0;   /* reset interrupt pending counter   */
  mas->done = 1;


  // here triva specific part JAM:
//   printk (KERN_INFO " map TRIVA registers \n");
//  //  // map TRIVA register
//    pl_stat = (unsigned int*) ((long)triva_base_addr + 0x0);
//    pl_ctrl = (unsigned int*) ((long)triva_base_addr + 0x4);
//    pl_fcti = (unsigned int*) ((long)triva_base_addr + 0x8);
//    pl_cvti = (unsigned int*) ((long)triva_base_addr + 0xc);
//
//    printk (KERN_INFO " Ptr. TRIVA stat: 0x%lx \n", (unsigned long)pl_stat);
//    printk (KERN_INFO " Ptr. TRIVA ctrl: 0x%lx \n", (unsigned long)pl_ctrl);
//    printk (KERN_INFO " Ptr. TRIVA fcti: 0x%lx \n", (unsigned long)pl_fcti);
//    printk (KERN_INFO " Ptr. TRIVA cvti: 0x%lx \n", (unsigned long)pl_cvti);
//
//    printk (KERN_INFO " TRIVA registers content \n");
//    printk (KERN_INFO " TRIVA stat: .... 0x%x \n", *(pl_stat));
//    printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", *(pl_ctrl));
//    printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0x10000 - *(pl_fcti));
//    printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0x10000 - *(pl_cvti));

    #ifdef INTERNAL_TRIG_TEST
    // initalize TRIVA only for internal tests
    printk (KERN_INFO "\n");
    printk (KERN_INFO " Initalize TRIVA \n");

    *pl_ctrl = 0x1000;
    *pl_ctrl = HALT;
    *pl_ctrl = MASTER;
    *pl_ctrl = CLEAR;

    printk (KERN_INFO " TRIVA registers content \n");
    printk (KERN_INFO " TRIVA stat: .... 0x%x \n", *pl_stat);
    printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", *pl_ctrl);
    printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0x10000 - *pl_fcti);
    printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0x10000 - *pl_cvti);

    //*pl_ctrl = BUS_ENABLE;
    //*pl_ctrl = CLEAR;
    //*pl_stat = EV_IRQ_CLEAR;
    *pl_fcti = 0x10000 - 0x10;
    *pl_cvti = 0x10000 - 0x21;

    printk (KERN_INFO " TRIVA registers content \n");
    printk (KERN_INFO " TRIVA stat: .... 0x%x \n", *pl_stat);
    printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", *pl_ctrl);
    printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0x10000 - *pl_fcti);
    printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0x10000 - *pl_cvti);

    *pl_ctrl = HALT;
    *pl_ctrl = CLEAR;
    *pl_stat = 14;
    *pl_ctrl = (EN_IRQ | GO);

    printk (KERN_INFO " TRIVA registers content \n");
    printk (KERN_INFO " TRIVA stat: .... 0x%x \n", *pl_stat);
    printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", *pl_ctrl);
    printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0x10000 - *pl_fcti);
    printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0x10000 - *pl_cvti);
    #endif // INTERNAL_TRIG_TEST

    //outl (1<<TRIGMOD_IRQ_LEVEL, vme_itc_reg+0x8);
    //outl( 0xfe, vme_itc_reg+0x8);

     // sema_init (&triv_sem, 0);




  return( 0);




//  if( mas->map_p)
//     {
//       /* If interrupt vector has been assigned, release it */
//       if( (mas->map_p->ivec >= MAS_MAP_IVEC_MIN) && (mas->map_p->ivec <= MAS_MAP_IVEC_MAX))
//       {
//         altmas_ivec_tbl->mas[mas->map_p->ivec] = NULL;
//       }
//       if( mas->pg_idx >= 0)
//       {
//         alt_map_mas_free( mas->alt, MAP_ID_MAS_PCIE_PMEM, mas->pg_idx);
//         mas->pg_idx = -1;
//       }
//     }
//     filp->private_data = (void *)NULL;
//     kfree( mas->map_p);
//     mas->map_p = NULL;
//   }


 //altmas_init_err_irq:
 if( (mas->map_p->ivec >= MAS_MAP_IVEC_MIN) && (mas->map_p->ivec <= MAS_MAP_IVEC_MAX))
       {
          altmas_ivec_tbl->mas[mas->map_p->ivec] = NULL;
        }


 altmas_init_err_map:
  if( mas->pg_idx >= 0)
      {
          alt_map_mas_free( mas->alt, MAP_ID_MAS_PCIE_PMEM, mas->pg_idx);
          mas->pg_idx = -1;
     }


altmas_init_err_alloc_map:
  kfree( mas->map_p);
  // kfree altmas.mas;
altmas_err_class:
  cdev_del( &altmas.cdev);
altmas_init_err_cdev_add:
  unregister_chrdev_region( altmas.dev_id, altmas_dev_num);
altmas_init_err_alloc_chrdev:

  return( retval);
}




static void trigmod_exit(void)
{
  int i, vme_major;
  struct altmas_device *mas;
  mas=altmas.mas[0]; // JAM just for convenience
  debugk(( KERN_ALERT "trigmod: entering trigmod_exit( void)\n"));
  if( triva_base_addr)
   {
     iounmap(triva_base_addr);
   }
  for( i = 0; i < altmas_irq_cnt; i++)
  {
    if( (altmas_irq_ena[i] > 0) && (altmas_irq_ena[i] < 8))
    {
      int src;

      src = ITC_SRC_VME_IRQ1 + altmas_irq_ena[i] - 1;
      alt_irq_mask( altmas.alt, ALT_IOCTL_ITC_MSK_SET, src);
      alt_irq_unregister( altmas.alt, src);
    }
  }

  kfree(altmas_ivec_tbl);

  if( mas->pg_idx >= 0)
       {
           alt_map_mas_free( mas->alt, MAP_ID_MAS_PCIE_PMEM, mas->pg_idx);
           mas->pg_idx = -1;
      }

  kfree( mas->map_p);



  vme_major = MAJOR( altmas.dev_id);
  for( i = 0; i < altmas_dev_num; i++)
  {
    device_destroy( vme_sysfs_class, MKDEV( vme_major, i));
    mutex_destroy( &altmas.mas[i]->mutex);
  }
  class_destroy( vme_sysfs_class);
  cdev_del( &altmas.cdev);
  unregister_chrdev_region( altmas.dev_id, altmas_dev_num);
}


module_init( trigmod_init);
module_exit( trigmod_exit);


MODULE_AUTHOR(VMETRIGMODAUTHORS);
MODULE_DESCRIPTION(VMETRIGMODDESC);
MODULE_LICENSE("GPL");
MODULE_VERSION(VMETRIGMODVERSION);

/*================================< end file >================================*/

