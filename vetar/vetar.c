// Joern Adamczewski-Musch, CSEE, GSI Darmstadt 14-11-2013
// based on example code from N.Kurz and R.Eibauer
//
// #include <linux/kernel.h>
// #include <linux/module.h>
// #include <linux/fs.h>
// #include <linux/uaccess.h>
// #include <linux/io.h>
//#include <linux/byteorder/little_endian.h>

#include "vetar.h"
//#include "vetar_ioctl.h"

#ifdef VETAR_NEW_XPCLIB
struct CesXpcBridge *vme_bridge;
#endif




/* this is for dynamic device numbering*/
static int vetar_major_nr=0;
static dev_t vetar_devt;

/* we support sysfs class only for new kernels to avoid backward incompatibilities here */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* vetar_class;
#endif

/* need to keep track of our privdata structures:*/

static struct vetar_privdata* vetar_devices[VETAR_MAX_DEVICES];


/* Module parameters */
static int  slot[VETAR_MAX_DEVICES];
static unsigned int slot_num;
static unsigned int vmebase[VETAR_MAX_DEVICES];
static unsigned int vmebase_num;
static int  vector[VETAR_MAX_DEVICES];
static unsigned int vector_num;
static int lun[VETAR_MAX_DEVICES] = VETAR_DEFAULT_IDX;
static unsigned int lun_num;


module_param_array(slot, int, &slot_num, S_IRUGO);
MODULE_PARM_DESC(slot, "Slot where VETAR card is installed");
module_param_array(vmebase, uint, &vmebase_num, S_IRUGO);
MODULE_PARM_DESC(vmebase, "VME Base address of the VETAR card registers");
module_param_array(vector, int, &vector_num, S_IRUGO);
MODULE_PARM_DESC(vector, "IRQ vector");
module_param_array(lun, int, &lun_num, S_IRUGO);
MODULE_PARM_DESC(lun, "Index value for VETAR card");



#ifdef VETAR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static DEVICE_ATTR(codeversion, S_IRUGO, vetar_sysfs_codeversion_show, NULL);
#endif
#endif




struct vetar_privdata* get_privdata(struct file *filp)
{
  struct vetar_privdata *privdata;
  privdata= (struct vetar_privdata*) filp->private_data;
  if(privdata->init_done==0)
    {
      vetar_dbg(KERN_ERR "*** PEXOR structure was not initialized!\n");
      return NULL;
    }
  return privdata;
}

#ifdef VETAR_ENABLE_IRQ
 static void vetar_irqhandler(int vec, int prio, void *arg) {
 	struct vetar_privdata *dev = arg;

   debug ((KERN_INFO "BEGIN irq_hand \n"));

   debug ((KERN_INFO "IRQ Level: %d, IRQ vector: 0x%x \n", prio, vec));


 	//return IRQ_HANDLED;
 	dev->irq_count++;
 }

 static int vetar_get_irqcount(struct vetar_privdata *dev, int clear) {
 	int tmp;

 	tmp = dev->irq_count;
 	if (clear)
 		dev->irq_count = 0;
 	return tmp;
 }
#endif

static void vetar_cleanup_dev(struct vetar_privdata *privdata) {

  if(privdata==0) return;
  if(privdata->registers)
    iounmap(privdata->registers);
  if(privdata->regs_phys)
  {
#ifdef VETAR_NEW_XPCLIB
	CesXpcBridge_MasterUnMap64(vme_bridge, privdata->regs_phys, privdata->reglen);
#else  
	xpc_vme_master_unmap(privdata->regs_phys, privdata->reglen);
#endif
  }

  if(privdata->cr_csr)
      iounmap(privdata->cr_csr);
  if(privdata->cr_csr_phys)
    {
  #ifdef VETAR_NEW_XPCLIB
      CesXpcBridge_MasterUnMap64(vme_bridge, privdata->cr_csr_phys, privdata->configlen);
  #else
      xpc_vme_master_unmap(privdata->cr_csr_phys, privdata->configlen);
  #endif
    }

#ifdef VETAR_ENABLE_IRQ
  xpc_vme_free_irq(privdata->vector);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (privdata->class_dev)
    {
#ifdef VETAR_SYSFS_ENABLE
      device_remove_file(privdata->class_dev, &dev_attr_codeversion);
#endif
      device_destroy(vetar_class, privdata->devno);
      privdata->class_dev=0;
    }

#endif

  /* character device cleanup*/
  if(privdata->cdev.owner)
    cdev_del(&privdata->cdev);



  kfree(privdata);

}


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int vetar_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
long vetar_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
	int retval = 0;
//	struct vetar_user_data *user_data = filp->private_data;

  debug ((KERN_INFO "BEGIN vetar_ioctl \n"));

	switch (cmd) {
//	  case CMD_VETAR_GET_IRQCOUNT: {
//		  union vmevetar_get_irqcount_arg args;
//
//		  if (copy_from_user((void __user*)arg, &args, sizeof(args.in))) {
//			  retval = -EFAULT;
//			  break;
//		  }
//		  args.out.value = vetar_get_irqcount(user_data->dev, args.in.clear);
//		  if (copy_to_user((void __user*)arg, &args, sizeof(args.out))) {
//			  retval = -EFAULT;
//			  break;
//		  }
//		  break;
//
//    case WAIT_SEM:
//      debug ((KERN_INFO " before WAIT_SEM \n"));
//      down (&triv_sem);
//      triv_val = 0;
//      debug ((KERN_INFO " after  WAIT_SEM \n"));
//      break;
//    case POLL_SEM:
//      debug ((KERN_INFO " before POLL_SEM, triv_val: %d \n", triv_val));
//		  retval = __put_user(triv_val, (int __user *)arg);
//      debug ((KERN_INFO " after POLL_SEM \n"));
//      break;
//    case RESET_SEM:
//      printk (KERN_INFO " before RESET_SEM \n");
//      triv_val = 0;
//      init_MUTEX_LOCKED (&triv_sem);
//		  retval = __put_user(0, (int __user *)arg);
//      printk (KERN_INFO " after  RESET_SEM \n");
//      break;
//	  }
	  default:
		retval = -EINVAL;
	}
  debug ((KERN_INFO "END   vetar_ioctl \n"));
	return retval;
}




int vetar_open(struct inode *inode, struct file *filp)
{
  struct vetar_privdata *privdata;
  vetar_dbg(KERN_NOTICE "** starting vetar_open...\n");
  /* Set the private data area for the file */
  privdata = container_of( inode->i_cdev, struct vetar_privdata, cdev);
  filp->private_data = privdata;
  return 0;
}

int vetar_release(struct inode *inode, struct file *filp)
{
  vetar_dbg(KERN_NOTICE "** starting vetar_release...\n");
   return 0;
}



loff_t vetar_llseek(struct file *filp, loff_t off, int whence)
{
  loff_t newpos;  
  struct vetar_privdata *privdata;
  /* set cursor in mapped board RAM for read/write*/
  vetar_dbg(KERN_NOTICE "** starting vetar_llseek ...\n");
  /* may use struct scull_dev *dev = filp->private_data; */
  privdata= get_privdata(filp);
  if(!privdata) return -EFAULT;

  
  
  switch(whence) {
  case 0: /* SEEK_SET */
    newpos = off;
    break;

  case 1: /* SEEK_CUR */
    newpos = filp->f_pos + off;
    break;

  case 2: /* SEEK_END */
    newpos = privdata->reglen + off;
    break;

  default: /*can't happen */
    return -EINVAL;
  }
  if (newpos < 0) return -EINVAL;
  filp->f_pos = newpos;
  return newpos;



  return 0;
}


ssize_t vetar_read(struct file *filp, char __user *buf, size_t count,
                          loff_t *f_pos)
{

  /*** WRONG APPROACH: need to interface with wishbone master**/
  /* here we read from mapped vetar memory into user buffer*/
  int i;
  ssize_t retval = 0;
  struct vetar_privdata *privdata;
  void* memstart;
  int lcount=count>>2;
  u32* kbuf=0;
  /*  u32 kbuf[lcount];*/
  vetar_dbg(KERN_NOTICE "** starting vetar_read for f_pos=%d count=%d\n", (int) *f_pos, (int) count);
  privdata= get_privdata(filp);
  if(!privdata) return -EFAULT;

  if (down_interruptible(&privdata->ramsem))
    return -ERESTARTSYS;
  if (*f_pos >= privdata->reglen)
    goto out;
  kbuf= (u32*) kmalloc(count,GFP_KERNEL);
  if(!kbuf)
    {
      vetar_msg(KERN_ERR "vetar_read: could not alloc %d buffer space! \n",lcount);
      retval = -ENOMEM;
      goto out;
    }
  if (*f_pos + count > privdata->reglen)
    {
      /* TODO: better return error to inform user we exceed ram size?*/
      count = privdata->reglen - *f_pos;
      lcount=count>>2;
      vetar_dbg(KERN_NOTICE "** vetar_read truncates count to =%d\n", (int) count);
    }
  memstart=(void*) (privdata->registers) + *f_pos;
  /* try to use intermediate kernel buffer here:*/
  vetar_dbg(KERN_NOTICE "** vetar_read begins io loop at memstart=%lx\n", (long) memstart);
  /*wmb();
    memcpy_fromio(&kbuf, memstart, count);*/
  mb();
  for(i=0;i<lcount;++i)
    {
      vetar_dbg(KERN_NOTICE "%x from %lx..", i,(long unsigned )memstart+(i<<2));
      vetar_dbg(KERN_NOTICE "%d ..", i);
    if((i%10)==0) vetar_msg(KERN_NOTICE "\n");
    mb();
      kbuf[i]=ioread32be(memstart+(i<<2));/*JAM just like pexor, but big endian vme bus*/
      mb();
      ndelay(100);
      /*udelay(1);*/
    }


  vetar_dbg(KERN_NOTICE "** vetar_read begins copy to user from stack buffer=%lx\n", (long) kbuf);
  if (copy_to_user(buf, kbuf, count)) {
    vetar_dbg(KERN_ERR "** vetar_read copytouser error!\n");
    retval = -EFAULT;
    goto out;
  }
  *f_pos += count;
  retval = count;
 out:
  kfree(kbuf);
  up(&privdata->ramsem);
  return retval;

}

ssize_t vetar_write(struct file *filp, const char __user *buf, size_t count,
            loff_t *f_pos)
{
  int i;
  ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
  struct vetar_privdata *privdata;
  void* memstart;
  int lcount=count>>2;
  u32* kbuf=0;
  /*u32 kbuf[lcount];*/
  vetar_dbg(KERN_NOTICE "** starting vetar_write for f_pos=%d count=%d\n", (int) *f_pos, (int) count);
  privdata= get_privdata(filp);
  if(!privdata) return -EFAULT;
  if (down_interruptible(&privdata->ramsem))
    return -ERESTARTSYS;
  if (*f_pos >= privdata->reglen)
    goto out;
  kbuf= (u32*) kmalloc(count,GFP_KERNEL);
  if(!kbuf)
    {
      vetar_msg(KERN_ERR "vetar_write: could not alloc %d buffer space! \n",lcount);
      retval = -ENOMEM;
      goto out;
    }

  if (*f_pos + count >= privdata->reglen )
    {
      /* TODO: better return error to inform user we exceed ram size?*/
      count = privdata->reglen  - *f_pos;
      lcount=count>>2;
      vetar_dbg(KERN_NOTICE "** vetar_write truncates count to =%d\n", (int) count);
    }
  memstart=(void*) (privdata->registers) + *f_pos;
  vetar_dbg(KERN_NOTICE "** vetar_write begins copy from user at stack buffer=%lx\n", (long) kbuf);
  mb();
  if (copy_from_user(kbuf, buf, count))
    {
      retval = -EFAULT;
      goto out;
    }
  vetar_dbg(KERN_NOTICE "** vetar_write begins copy loop at memstart=%lx\n", (long) memstart);
  /*memcpy_toio(memstart, kbuf , count);*/
  mb();
  for(i=0;i<lcount;++i)
    {

      /*vetar_dbg(KERN_NOTICE "kbuf[%d]=%x to %lx..", i,kbuf[i],memstart+(i<<2));*/
      /*vetar_dbg(KERN_NOTICE "%d..", i);
    if((i%10)==0) vetar_msg(KERN_NOTICE "\n");*/
      iowrite32be(kbuf[i], memstart+(i<<2)); /*JAM just like pexor, but big endian vme bus*/
      mb();
      ndelay(20);
    }
  *f_pos += count;
  retval = count;
 out:
  kfree(kbuf);
  up(&privdata->ramsem);
  return retval;

}


ssize_t vetar_sysfs_codeversion_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  char vstring[128];
  ssize_t curs=0;
  struct pexor_privdata *privdata;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  curs=snprintf(vstring, 128, "*** This is VETAR driver for CES VME Linux, version %s build on %s at %s \n\t", VETARVERSION, __DATE__, __TIME__);
  return snprintf(buf, PAGE_SIZE, "%s\n", vstring);
}




static struct file_operations vetar_fops = {
	.open           = vetar_open,
	.release        = vetar_release,
	.llseek =   vetar_llseek,
	.read =           vetar_read,
	.write =          vetar_write,
	#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
	.ioctl        = vetar_ioctl,
	#else
	    .unlocked_ioctl    = vetar_ioctl,
	#endif
};

int vetar_is_present(struct vetar_privdata *privdata)
{
  return 1;
  //struct device *dev = vetar->dev;
    uint32_t idc;
    void* addr;

    /* Check for bootloader */
    //if (vetar_is_bootloader_active(vetar)) {
    //  return 1;
    //}

    /* Ok, maybe there is a vetar, but bootloader is not active.
    In such case, a CR/CSR with a valid manufacturer ID should exist*/

    addr = privdata->cr_csr + VME_VENDOR_ID_OFFSET;
//    mb();
//    idc = ioread32be(addr);
mb();
    idc = be32_to_cpu(ioread32be(addr)) << 16;
    mb();ndelay(100);
    idc += be32_to_cpu(ioread32be(addr + 4))  << 8;
    mb();ndelay(100);
    idc += be32_to_cpu(ioread32be(addr + 8));
     mb(); ndelay(100);
    if (idc == VETAR_VENDOR_ID) {
        vetar_msg(KERN_NOTICE "Found Vetar vendor ID: 0x%08x\n", idc);
        return 1;
    }
    vetar_msg(KERN_ERR "wrong vendor ID. 0x%08x found, 0x%08x expected\n",
            idc, VETAR_VENDOR_ID);
    vetar_msg(KERN_ERR "VETAR not present at slot %d\n", privdata->slot);
    return 0;

}

void vetar_csr_write(u8 value, void *base, u32 offset)
{
    offset -= offset % 4;
    iowrite32be(value, base + offset);
}

void vetar_setup_csr_fa0(struct vetar_privdata *privdata)
{
    u8 fa[4];       /* FUN0 ADER contents */

    /* reset the core */
    vetar_csr_write(RESET_CORE, privdata->cr_csr, BIT_SET_REG);
    msleep(10);

    /* disable the core */
    vetar_csr_write(ENABLE_CORE, privdata->cr_csr, BIT_CLR_REG);

    /* default to 32bit WB interface */
    vetar_csr_write(WB32, privdata->cr_csr, WB_32_64);

#ifdef VETAR_ENABLE_IRQ
    /* set interrupt vector and level */
    vetar_csr_write(privdata->vector, privdata->cr_csr, INTVECTOR);
    vetar_csr_write(privdata->level, privdata->cr_csr, INT_LEVEL);
#endif
    /* do address relocation for FUN0 */
    fa[0] = (privdata->vmebase >> 24) & 0xFF;
    fa[1] = (privdata->vmebase >> 16) & 0xFF;
    fa[2] = (privdata->vmebase >> 8 ) & 0xFF;
    fa[3] = (XPC_VME_A32_STD_USER & 0x3F) << 2;
            /* DFSR and XAM are zero */

    vetar_csr_write(fa[0], privdata->cr_csr, FUN0ADER);
    vetar_csr_write(fa[1], privdata->cr_csr, FUN0ADER + 4);
    vetar_csr_write(fa[2], privdata->cr_csr, FUN0ADER + 8);
    vetar_csr_write(fa[3], privdata->cr_csr, FUN0ADER + 12);

    /* enable module, hence make FUN0 available */
    vetar_csr_write(ENABLE_CORE, privdata->cr_csr, BIT_SET_REG);
}




/*
 * Here we probe vetar device of index in module parameter array*/
static int vetar_probe_vme(unsigned int index)
{
  int result=0;
  int err = 0;
  struct vetar_privdata *privdata;
  vetar_msg(KERN_NOTICE "VETAR vmd driver starts probe for index %d\n",index);

  /* Allocate and initialize the private data for this device */
    privdata = kzalloc(sizeof(struct vetar_privdata), GFP_KERNEL);
    if (privdata == NULL)
      {
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
      }
  vetar_devices[index]=privdata;

  // initialize private device structure:
  privdata->lun=lun[index];
  privdata->vmebase=vmebase[index];
  if(privdata->vmebase==0) privdata->vmebase=VETAR_REGS_ADDR;
  privdata->reglen=VETAR_REGS_SIZE;
  privdata->slot=slot[index];
  privdata->vector=vector[index];
  privdata->level = VETAR_IRQ_LEVEL;

  // first try to map and look up configuration space if any....
  privdata->configbase = privdata->slot * VETAR_CONFIGSIZE;
  privdata->configlen=VETAR_CONFIGSIZE;
#ifdef VETAR_NEW_XPCLIB
    privdata->cr_csr_phys = CesXpcBridge_MasterMap64(vme_bridge, privdata->configbase, privdata->configlen, VME_CR_CSR | XPC_VME_A32_STD_USER);
    if (privdata->cr_csr_phys == 0xffffffffffffffffULL) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not CesXpcBridge_MasterMap64 at configbase 0x%x with length 0x%x !\n",
               privdata->configbase, privdata->configlen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
    }
#else
    privdata->cr_csr_phys = xpc_vme_master_map(privdata->configbase, 0, privdata->configlen, XPC_VME_ATYPE_CRCSR , 0);
    if (privdata->regs_phys == 0xffffffffULL) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not xpc_vme_master_map at configbase 0x%x with length 0x%lx !\n",
          privdata->configbase, privdata->configlen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
    }
#endif
    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme mapped configbase 0x%x with length 0x%lx to physical address 0x%x!\n",
            privdata->configbase, privdata->configlen, (unsigned int) privdata->cr_csr_phys);
    privdata->cr_csr = ioremap_nocache(privdata->cr_csr_phys, privdata->configlen);
    if (!privdata->cr_csr) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at config physical address 0x%x with length 0x%lx !\n",
          (unsigned int) privdata->cr_csr_phys, privdata->configlen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
     }

    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme remapped physical config address 0x%x to kernel address 0x%lx\n",
             (unsigned int) privdata->cr_csr_phys,  (unsigned long) privdata->cr_csr);

  //  may check for vendor id etc...
if(!vetar_is_present(privdata))
  {
    vetar_cleanup_dev(privdata);
    return -EFAULT;
  }

vetar_setup_csr_fa0(privdata);


  // setup interrupts:
#ifdef VETAR_ENABLE_IRQ
snprintf(privdata->irqname, 64, VETARNAMEFMT,privdata->lun);
  result = xpc_vme_request_irq(privdata->vector, privdata->level, vetar_irqhandler, dev, privdata->irqname);
    if (result)
    {
    }
#endif



    // map register space:
#ifdef VETAR_NEW_XPCLIB
    privdata->regs_phys = CesXpcBridge_MasterMap64(vme_bridge, privdata->vmebase, privdata->reglen, XPC_VME_A32_STD_USER);
    if (privdata->regs_phys == 0xffffffffffffffffULL) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not CesXpcBridge_MasterMap64 at vmebase 0x%x with length 0x%x !\n",
               privdata->vmebase, privdata->reglen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
    }
#else
    privdata->regs_phys = xpc_vme_master_map(privdata->vmebase, 0, privdata->reglen, XPC_VME_A32_STD_USER, 0);
    if (privdata->regs_phys == 0xffffffffULL) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not xpc_vme_master_map at vmebase 0x%x with length 0x%lx !\n",
          privdata->vmebase, privdata->reglen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
    }
#endif
    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme mapped vmebase 0x%x with length 0x%lx to physical address 0x%x!\n",
            privdata->vmebase, privdata->reglen, (unsigned int) privdata->regs_phys);
    privdata->registers = ioremap_nocache(privdata->regs_phys, privdata->reglen);
    if (!privdata->registers) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at physical address 0x%x with length 0x%lx !\n",
          (unsigned int) privdata->regs_phys, privdata->reglen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
     }
    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme remapped physical address 0x%x to kernel address 0x%lx\n",
          (unsigned int) privdata->regs_phys,  (unsigned long) privdata->registers);


#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
  init_MUTEX(&(privdata->ramsem));
#else
  sema_init(&(privdata->ramsem),1);
#endif



  // add character device and sysfs class:
   privdata->devno
     = MKDEV(MAJOR(vetar_devt), MINOR(vetar_devt) + privdata->lun);

   /* Register character device */
   cdev_init(&(privdata->cdev), &vetar_fops);
   privdata->cdev.owner = THIS_MODULE;
   privdata->cdev.ops = &vetar_fops;
   err = cdev_add(&privdata->cdev, privdata->devno, 1);
   if (err)
     {
       vetar_msg( "Vetar couldn't add character device.\n" );
       vetar_cleanup_dev(privdata);
       return err;
     }



 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
   if (!IS_ERR(vetar_class))
     {
       /* driver init had successfully created class, now we create device:*/
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
       privdata->class_dev = device_create(vetar_class, NULL, privdata->devno,
                       privdata, VETARNAMEFMT, MINOR(vetar_devt) + privdata->lun);
 #else
       privdata->class_dev = device_create(vetar_class, NULL, privdata->devno,
                       VETARNAMEFMT, MINOR(vetar_devt) + privdata->lun);
 #endif
       dev_set_drvdata(privdata->class_dev, privdata);
       vetar_msg(KERN_NOTICE "Added VETAR device: ");
       vetar_msg(KERN_NOTICE VETARNAMEFMT, MINOR(vetar_devt) + privdata->lun);

 #ifdef VETAR_SYSFS_ENABLE
  if(device_create_file(privdata->class_dev, &dev_attr_codeversion) != 0)
     {
       vetar_msg(KERN_ERR "Could not add device file node for code version.\n");
     }

 #endif
     }
   else
     {
       /* something was wrong at class creation, we skip sysfs device support here:*/
       vetar_msg(KERN_ERR "Could not add VETAR device node to /dev !");
     }

 #endif


   privdata->init_done=1;
   vetar_msg(KERN_NOTICE "vetar_probe_vme has finished for index %d.\n",index);
   return result;


}






int __init vetar_init(void)
{
	int result,i;
	vetar_msg(KERN_NOTICE "vetar driver init...\n");
	/* Check that all insmod argument vectors are the same length */
	    if (lun_num != vmebase_num || lun_num!= vector_num) {
	        pr_err("%s: The number of parameters doesn't match\n",
	               __func__);
	        return -EINVAL;
	    }
	  if(vmebase_num>VETAR_MAX_DEVICES)   vmebase_num=VETAR_MAX_DEVICES;
	  if(vmebase_num==0)
	    {
	      /* no module parameter given: set to defaults*/
	      vmebase_num=1;
	      vmebase[0]=VETAR_REGS_ADDR;
	      lun[0]=0;
	      slot[0]=8; /* first test case*/
	      vetar_msg(KERN_NOTICE "No module parameters - use default address 0x%x, slot number 0x%x\n",vmebase[0],slot[0]);
	    }

/*  JAM: here we need to probe all vetar devices in bus that are specified by module parameters
 *  and initialize different devices for each of them
 */


/* register chardev region:*/
	      vetar_devt  = MKDEV(vetar_major_nr, 0);

	      /*
	       * Register your major, and accept a dynamic number.
	       */
	      if (vetar_major_nr)
	        result = register_chrdev_region(vetar_devt, VETAR_MAX_DEVICES, VETARNAME);
	      else {
	        result = alloc_chrdev_region(&vetar_devt, 0, VETAR_MAX_DEVICES, VETARNAME);
	        vetar_major_nr = MAJOR(vetar_devt);
	      }
	      if (result < 0)
	        return result;

	    #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
	      vetar_class = class_create(THIS_MODULE, VETARNAME);
	      if (IS_ERR(vetar_class))
	        {
	          vetar_msg(KERN_ALERT "Could not create class for sysfs support!\n");
	        }

	    #endif


//	      vetar_devices= (struct vetar_privdata**) kzalloc(VETAR_MAX_DEVICES*sizeof(struct vetar_privdata*), GFP_KERNEL);
//	      if (vetar_devices == NULL)
//	            {
//	              vetar_msg(KERN_ALERT "Could not allocate memory for device list!\n");
//	              return -ENOMEM;
//	            }

	    for (i=0;i<vmebase_num;++i)
	    {
	       vetar_probe_vme(i);
	    }

	    return 0;
}

void vetar_exit(void)
{
  int i;
  vetar_msg(KERN_NOTICE "vetar driver exit...\n");

  /* since we have no remove, we need to cleanup all devices here:
   *
   * TODO: can we get list of devices from class object instead? JAM
   * */
  for(i=0;i<VETAR_MAX_DEVICES;++i)
  {
    if(vetar_devices[i]==0) continue;
    vetar_cleanup_dev(vetar_devices[i]);
    vetar_devices[i]=0;
  }
  unregister_chrdev_region(vetar_devt, VETAR_MAX_DEVICES);


  #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (vetar_class != NULL)
        class_destroy(vetar_class);
  #endif
  //kfree(vetar_devices);
  vetar_msg(KERN_NOTICE "\t\tdriver exit done.\n");

}

module_init(vetar_init);
module_exit(vetar_exit);

MODULE_AUTHOR("J.Adamczewski-Musch EE, GSI");
MODULE_DESCRIPTION("VETAR2 VME driver for CES PPC Linux");
MODULE_LICENSE("GPL");
