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




static void vetar_wb_cycle(struct wishbone* wb, int on)
{
   struct vetar_privdata *privdata;
   privdata = container_of(wb, struct vetar_privdata, wb);
   vetar_dbg(KERN_ERR "*** vetar_wb_cycle...\n");
   //if (on) mutex_lock_interruptible(&privdata->wb_mutex);
   if (on) mutex_lock(&privdata->wb_mutex);
   vetar_dbg(KERN_ERR "*** Vetar_WB: cycle(%d)\n",on);
   if (!on) mutex_unlock(&privdata->wb_mutex);
}




static wb_data_t vetar_wb_read_cfg(struct wishbone *wb, wb_addr_t addr)
{
   wb_data_t out=0;
   struct vetar_privdata *privdata;
   privdata = container_of(wb, struct vetar_privdata, wb);

   //vetar_msg(KERN_ERR "*** Vetar_WB:: READ CFG  addr 0x%x \n", addr);

   switch (addr) {
   case 0:  out = 0; break;
   case 4:  out =0;
   //case 4:  out = be32_to_cpu(ioread32be(privdata->ctrl_registers + ERROR_FLAG));
    break;
   case 8:  out = 0; break;


#ifdef VETAR_MAP_CONTROLSPACE
   case 12: out = ioread32be(privdata->ctrl_registers + SDWB_ADDRESS);
#else
   case 12: out = 0x300000; /* this is expected value*/
#endif


   /* JAM note we have sometimes problems to get correct address here!*/
   break;
   default: out = 0; break;
   }
   mb(); /* ensure serial ordering of non-posted operations for wishbone */
   //ndelay(100);
   //vetar_msg(KERN_ERR "*** Vetar_WB:: READ CFG  value 0x%x \n", out);

   return out;
}

static void vetar_wb_write(struct wishbone* wb, wb_addr_t addr, wb_data_t data)
{
   struct vetar_privdata *privdata;
   vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_write.. ");
   privdata = container_of(wb, struct vetar_privdata, wb);
   addr = addr << 2 ;
   switch (privdata->wb_width) {
   case 4:
      vetar_dbg(KERN_ERR "*** Vetar_WB: iowrite32(0x%x, 0x%x)\n", data, addr);
      iowrite32be(data, privdata->registers  + addr);
      break;
   case 2:
      vetar_dbg(KERN_ERR "*** Vetar_WB: iowrite16(0x%x, 0x%x)\n", data >> privdata->wb_shift, addr);
      iowrite16be(data >> privdata->wb_shift, privdata->registers  + addr);
      break;
   case 1:
      vetar_dbg(KERN_ERR "*** Vetar_WB: : iowrite8(0x%x, 0x%x)\n", data >> privdata->wb_shift, addr);
      iowrite8 (data >> privdata->wb_shift, privdata->registers  + addr);
      break;
   }

   /* printk(KERN_ALERT VME_WB ": WRITE \n"); */
}

static wb_data_t vetar_wb_read(struct wishbone* wb, wb_addr_t addr)
{
    wb_data_t out;
    struct vetar_privdata *privdata;
    vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_read.. ");
    privdata = container_of(wb, struct vetar_privdata, wb);
    addr= addr << 2; /* conversion of the map from VME to WB32 */
    out = be32_to_cpu(ioread32be(privdata->registers +(addr)));
    vetar_dbg(KERN_ERR "*** Vetar_WB: READ (%x) = %x \n", (addr), out);
    mb();
    return out;
}

static int vetar_wb_request(struct wishbone *wb, struct wishbone_request *req)
{
return 0;
}

static void vetar_wb_reply(struct wishbone *wb, int err, wb_data_t dat)
{
}


static void vetar_wb_byteenable(struct wishbone* wb, unsigned char be)
{


  struct vetar_privdata *privdata;
  vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_byteenable.. ");
  privdata = container_of(wb, struct vetar_privdata, wb);

   switch (be) {
   case 0x1:
      privdata->wb_width = 1;
      privdata->wb_shift = 0;
      privdata->wb_low_addr = endian_addr(1, 0);
      break;
   case 0x2:
      privdata->wb_width = 1;
      privdata->wb_shift = 8;
      privdata->wb_low_addr = endian_addr(1, 1);
      break;
   case 0x4:
      privdata->wb_width = 1;
      privdata->wb_shift = 16;
      privdata->wb_low_addr = endian_addr(1, 2);
      break;
   case 0x8:
      privdata->wb_width = 1;
      privdata->wb_shift = 24;
      privdata->wb_low_addr = endian_addr(1, 3);
      break;
   case 0x3:
      privdata->wb_width = 2;
      privdata->wb_shift = 0;
      privdata->wb_low_addr = endian_addr(2, 0);
      break;
   case 0xC:
      privdata->wb_width = 2;
      privdata->wb_shift = 16;
      privdata->wb_low_addr = endian_addr(2, 2);
      break;
   case 0xF:
      privdata->wb_width = 4;
      privdata->wb_shift = 0;
      privdata->wb_low_addr = endian_addr(4, 0);
      break;
   default:
      /* noop -- ignore the strange bitmask */
      break;
   }

}

static const struct wishbone_operations vetar_wb_ops = {
        .cycle      = vetar_wb_cycle,
        .byteenable = vetar_wb_byteenable,
        .write      = vetar_wb_write,
        .read       = vetar_wb_read,
        .read_cfg   = vetar_wb_read_cfg,
        .request    = vetar_wb_request,
        .reply      = vetar_wb_reply,
};







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


   wishbone_unregister(&privdata->wb);

   /* disable the core */
   vetar_csr_write(ENABLE_CORE, privdata->cr_csr, BIT_CLR_REG);

#ifdef VETAR_MAP_CONTROLSPACE
  if(privdata->ctrl_registers)
    {
      iounmap(privdata->ctrl_registers);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped registers 0x%x !\n",
                    (unsigned int) privdata->ctrl_registers);
    }
  if(privdata->ctrl_regs_phys)
  {
#ifdef VETAR_NEW_XPCLIB
    CesXpcBridge_MasterUnMap64(vme_bridge, privdata->ctrl_regs_phys, privdata->ctrl_reglen);
#else
    xpc_vme_master_unmap(privdata->ctrl_regs_phys, privdata->ctrl_reglen);
#endif
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys control registers 0x%x with length 0x%lx !\n",
          (unsigned int) privdata->ctrl_regs_phys, privdata->ctrl_reglen);
  }
#endif


  if(privdata->registers)
    {
      iounmap(privdata->registers);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped registers 0x%x !\n",
                    (unsigned int) privdata->registers);
    }
  if(privdata->regs_phys)
  {
#ifdef VETAR_NEW_XPCLIB
	CesXpcBridge_MasterUnMap64(vme_bridge, privdata->regs_phys, privdata->reglen);
#else  
	xpc_vme_master_unmap(privdata->regs_phys, privdata->reglen);
#endif
	  vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys registers 0x%x with length 0x%lx !\n",
	      (unsigned int) privdata->regs_phys, privdata->reglen);

  }

  if(privdata->cr_csr)
  {
      iounmap(privdata->cr_csr);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped configspace  0x%x !\n",
            (unsigned int) privdata->cr_csr);
  }
  if(privdata->cr_csr_phys)
    {
  #ifdef VETAR_NEW_XPCLIB
      CesXpcBridge_MasterUnMap64(vme_bridge, privdata->cr_csr_phys, privdata->configlen);
  #else
      xpc_vme_master_unmap(privdata->cr_csr_phys, privdata->configlen);
  #endif
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys config registers 0x%x with length 0x%lx !\n",
          (unsigned int) privdata->cr_csr_phys, (unsigned long) privdata->configlen);
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
#ifdef VETAR_CTRL_TEST
  u32 out=0;
#endif

  /*  u32 kbuf[lcount];*/
  vetar_dbg(KERN_NOTICE "** starting vetar_read for f_pos=%d count=%d\n", (int) *f_pos, (int) count);
  privdata= get_privdata(filp);
  if(!privdata) return -EFAULT;

#ifdef VETAR_TRIGMOD_TEST
  /* this is to debug access to trigger module*/
  vetar_dump_trigmod(privdata);
  /* this is to debug the config spaced setup*/
  vetar_is_present(privdata);
  return -EFAULT;
  /* end debug cscsr*/
#endif

#ifdef VETAR_CTRL_TEST

  out = be32_to_cpu(ioread32(privdata->ctrl_registers + ERROR_FLAG));
  mb(); ndelay(100);
  vetar_msg(KERN_INFO "vetar_read: be32_to_cpu(ioread32 control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + ERROR_FLAG, out);
  out = be32_to_cpu(ioread32(privdata->ctrl_registers + SDWB_ADDRESS));
  mb(); ndelay(100); mdelay(10);
  vetar_msg(KERN_INFO "vetar_read: be32_to_cpu(ioread32 control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + SDWB_ADDRESS, out);

   out = be32_to_cpu(ioread32be(privdata->ctrl_registers + ERROR_FLAG));
   mb(); ndelay(100); mdelay(10);
   vetar_msg(KERN_INFO "vetar_read: be32_to_cpu(ioread32be control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + ERROR_FLAG, out);
   out = be32_to_cpu(ioread32be(privdata->ctrl_registers + SDWB_ADDRESS));
   mb(); ndelay(100); mdelay(10);
   vetar_msg(KERN_INFO "vetar_read: be32_to_cpu(ioread32be control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + SDWB_ADDRESS, out);

   out = ioread32be(privdata->ctrl_registers + ERROR_FLAG);
   mb(); ndelay(100); mdelay(10);
   vetar_msg(KERN_INFO "vetar_read: ioread32be control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + ERROR_FLAG, out);
   out = ioread32be(privdata->ctrl_registers + SDWB_ADDRESS);
   mb(); ndelay(100); mdelay(10);
   vetar_msg(KERN_INFO "vetar_read: ioread32be control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + SDWB_ADDRESS, out);

   out = ioread32(privdata->ctrl_registers + ERROR_FLAG);
   mb(); ndelay(100); mdelay(10);
   vetar_msg(KERN_INFO "vetar_read: ioread32 control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + ERROR_FLAG, out);
   out = ioread32(privdata->ctrl_registers + SDWB_ADDRESS);
   mb(); ndelay(100); mdelay(10);
   vetar_msg(KERN_INFO "vetar_read: ioread32 control register va 0x%0x = 0x%x \n",privdata->ctrl_registers + SDWB_ADDRESS, out);




   return -EFAULT;
#endif

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

      vetar_dbg(KERN_NOTICE "%d ..", i);
    if((i%10)==0) vetar_msg(KERN_NOTICE "\n");
    mb();
      kbuf[i]=ioread32be(memstart+(i<<2));/*JAM just like pexor, but big endian vme bus*/
      mb();
      ndelay(100);
      vetar_dbg(KERN_NOTICE "0x%x from %lx  \n", kbuf[i],(long unsigned )memstart+(i<<2));
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


#ifdef VETAR_TRIGMOD_TEST
int vetar_dump_trigmod(struct vetar_privdata *privdata)
{
  // trigger module register
 unsigned int *pl_stat;
 unsigned int *pl_ctrl;
 unsigned int *pl_fcti;
 unsigned int *pl_cvti;
 void* addr_stat;
 void* addr_ctrl;


  vetar_msg(KERN_NOTICE "Check trigmod registers at base address 0x%x\n",privdata->vmebase);
  // map TRIVA register
      pl_stat = (unsigned int*) ((long)privdata->registers + 0x0);
      pl_ctrl = (unsigned int*) ((long)privdata->registers + 0x4);
      pl_fcti = (unsigned int*) ((long)privdata->registers + 0x8);
      pl_cvti = (unsigned int*) ((long)privdata->registers + 0xc);

      printk (KERN_INFO " Ptr. TRIVA stat: 0x%x \n", (unsigned int)pl_stat);
      printk (KERN_INFO " Ptr. TRIVA ctrl: 0x%x \n", (unsigned int)pl_ctrl);
      printk (KERN_INFO " Ptr. TRIVA fcti: 0x%x \n", (unsigned int)pl_fcti);
      printk (KERN_INFO " Ptr. TRIVA cvti: 0x%x \n", (unsigned int)pl_cvti);

      //out_be32 (pl_cvti, 0x50);

      printk (KERN_INFO " TRIVA registers content \n");
      printk (KERN_INFO " TRIVA stat: .... 0x%x \n", in_be32(pl_stat));
      printk (KERN_INFO " TRIVA ctrl: .... 0x%x \n", in_be32(pl_ctrl));
      printk (KERN_INFO " TRIVA fcti: .... 0x%x \n", 0x10000 - in_be32(pl_fcti));
      printk (KERN_INFO " TRIVA cvti: .... 0x%x \n", 0x10000 - in_be32(pl_cvti));

      /* above is same as in vmetrigmod.c, below use other read function:*/

      addr_stat=privdata->registers;
      addr_ctrl=privdata->registers + 0x04;

      printk (KERN_INFO " ioread32be -      TRIVA stat: .... 0x%x \n", ioread32be(addr_stat));
      printk (KERN_INFO " ioread32be -      TRIVA ctrl: .... 0x%x \n",  ioread32be(addr_ctrl));
return 0;
}
#endif


int vetar_is_present(struct vetar_privdata *privdata)
{
    uint32_t idc;
    void* addr;
    vetar_msg(KERN_ERR "Check if VETAR is present at slot %d, config base address 0x%x\n", privdata->slot, privdata->configbase);
    addr = privdata->cr_csr + VME_VENDOR_ID_OFFSET;
    vetar_msg(KERN_NOTICE "Reading Vendor id from address 0x%x ...\n", (unsigned) addr);
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
    vetar_dbg(KERN_NOTICE "vetar_csr_write value 0x%x to base 0x%x + offset 0x%x \n",
        value, (unsigned) base, offset);
}

void vetar_setup_csr_fa(struct vetar_privdata *privdata)
{
    int i;
    u32 offset;
    u8 fa[4];       /* FUN0 ADER contents */
    u8 am=0;
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


/* JAM test: do we need to disable all ADERs before defining the mapping?
 * try to initialize it with address mode that will never used by mbs*/
    //am=0x29;  /* A16=0x29, this will*/
    am=0;
    for(i=0;i<8;++i)
    {
      offset=FUN0ADER + i* 0x10;
      vetar_msg(KERN_NOTICE "vetar_setup_csr_fa initializes ADER %d at register 0x%x with AM:0x%x\n",i,offset,am);
      fa[0] = 0;
      fa[1] = 0;
      fa[2] = 0;
      fa[3] = (am & 0x3F) << 2;
      vetar_csr_write(fa[0], privdata->cr_csr, offset);
      vetar_csr_write(fa[1], privdata->cr_csr, offset + 4);
      vetar_csr_write(fa[2], privdata->cr_csr, offset + 8);
      vetar_csr_write(fa[3], privdata->cr_csr, offset + 12);
    }



#ifdef VETAR_MAP_REGISTERS

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
#endif

#ifdef VETAR_MAP_CONTROLSPACE
    /*do address relocation for FUN1, WB control mapping*/
    //am=0x39; /* JAM This is what we actually see on the vmebus monitor for (XPC_VME_ATYPE_A24 | XPC_VME_DTYPE_BLT | XPC_VME_PTYPE_USER)*/
    am = VME_A24_USER_MBLT; /*0x38*/
    //am= (XPC_VME_ATYPE_A24 | XPC_VME_DTYPE_MBLT | XPC_VME_PTYPE_USER); /* 0x44*/
    //am= (XPC_VME_ATYPE_A24 | XPC_VME_DTYPE_BLT | XPC_VME_PTYPE_USER); /* 0x42*/
    //am= XPC_VME_A24_STD_USER;

    vetar_msg(KERN_NOTICE "vetar_setup_csr_fa sets address modifier 0x%x\n",am);

     fa[0] = 0x00;
     fa[1] = 0x00;
     fa[2] = (privdata->vmebase >> 24 ) & 0xFF;
     //fa[3] = (am & 0x3F) << 2;
     fa[3] = am  << 2;

//     fa[0] = (privdata->vmebase >> 24) & 0xFF;
//     fa[1] = (privdata->vmebase >> 16) & 0xFF;
//     fa[2] = (privdata->vmebase >> 8 ) & 0xFF;
//     fa[3] = (am & 0x3F) << 2;

     vetar_csr_write(fa[0], privdata->cr_csr, FUN1ADER);
     vetar_csr_write(fa[1], privdata->cr_csr, FUN1ADER + 4);
     vetar_csr_write(fa[2], privdata->cr_csr, FUN1ADER + 8);
     vetar_csr_write(fa[3], privdata->cr_csr, FUN1ADER + 12);

#endif


    /* enable module, hence make FUN0/FUN1 available */
    vetar_csr_write(ENABLE_CORE, privdata->cr_csr, BIT_SET_REG);
    msleep(100);
}




/*
 * Here we probe vetar device of index in module parameter array*/
static int vetar_probe_vme(unsigned int index)
{
  int result=0;
  int err = 0;
  u32 am=0;
  struct vetar_privdata *privdata;
  vetar_msg(KERN_NOTICE "VETAR vme driver starts probe for index %d\n",index);
  vetar_msg(KERN_NOTICE "Use parameters address 0x%x, slot number 0x%x, lun 0x%x vector 0x%x\n",
                     vmebase[index],slot[index], lun[index],vector[index]);
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
  am=XPC_VME_ATYPE_CRCSR;
      /*JAM: Important: we _must_ use this address modifier from CES xpc lib ( defined as 0x0 !)
       * it will be translated on accessing the vmebus to the correct CS_CSR modifier 0x2f */
    /*am=VME_CR_CSR; this one will not work for xpc*/
#ifdef VETAR_NEW_XPCLIB
    privdata->cr_csr_phys = CesXpcBridge_MasterMap64(vme_bridge, privdata->configbase, privdata->configlen, am);
    if (privdata->cr_csr_phys == 0xffffffffffffffffULL) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not CesXpcBridge_MasterMap64 at configbase 0x%x with length 0x%lx !\n",
               privdata->configbase, privdata->configlen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
    }
#else

    privdata->cr_csr_phys = xpc_vme_master_map(privdata->configbase, 0, privdata->configlen, am , 0);
    if (privdata->cr_csr_phys == 0xffffffffULL) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not xpc_vme_master_map at configbase 0x%x with length 0x%lx !\n",
          privdata->configbase, privdata->configlen);
        vetar_cleanup_dev(privdata);
        return -ENOMEM;
    }
#endif
    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme mapped configbase 0x%x with length 0x%lx to physical address 0x%x, am=0x%x!\n",
            privdata->configbase, privdata->configlen, (unsigned int) privdata->cr_csr_phys,am);
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



  // setup interrupts:
#ifdef VETAR_ENABLE_IRQ
snprintf(privdata->irqname, 64, VETARNAMEFMT,privdata->lun);
  result = xpc_vme_request_irq(privdata->vector, privdata->level, vetar_irqhandler, dev, privdata->irqname);
    if (result)
    {
    }
#endif


#ifdef VETAR_TRIGMOD_TEST

    privdata->vmebase=TRIGMOD_REGS_ADDR;
    privdata->reglen=TRIGMOD_REGS_SIZE;
    vetar_msg(KERN_ERR "** vetar_probe_vme TEST: mapping triva vmebase 0x%x with length 0x%lx !\n",
                privdata->vmebase, privdata->reglen);

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
#endif
/********** END access to triggermodule test */


vetar_setup_csr_fa(privdata);

#ifdef VETAR_MAP_REGISTERS




    // map register space:
#ifdef VETAR_NEW_XPCLIB
    privdata->regs_phys = CesXpcBridge_MasterMap64(vme_bridge, privdata->vmebase, privdata->reglen, XPC_VME_A32_STD_USER);
    if (privdata->regs_phys == 0xffffffffffffffffULL) {
      vetar_msg(KERN_ERR "** vetar_probe_vme could not CesXpcBridge_MasterMap64 at vmebase 0x%x with length 0x%lx !\n",
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

#endif

#ifdef VETAR_MAP_CONTROLSPACE
//    else if (map_type == MAP_CTRL) {
//            am = VME_A24_USER_MBLT;
//            dw = VME_D32;
//            base = vme_dev->vme_res.vmebase;
//            size = 0xA0;
//            map_type_c = "WB MAP CTRL";
//
    privdata->ctrl_reglen=VETAR_CTRLREGS_SIZE;
    am= (XPC_VME_ATYPE_A24 | XPC_VME_DTYPE_MBLT | XPC_VME_PTYPE_USER); /* 0x44*/
    //am= (XPC_VME_ATYPE_A24 | XPC_VME_DTYPE_BLT | XPC_VME_PTYPE_USER); /* 0x42*/
    //am= XPC_VME_A24_STD_USER ;
    //am = VME_A24_USER_MBLT;
    vetar_msg(KERN_NOTICE "vetar_probe_vme maps with  controlspace address modifier 0x%x\n",am);

 #ifdef VETAR_NEW_XPCLIB
     privdata->ctrl_regs_phys = CesXpcBridge_MasterMap64(vme_bridge, privdata->vmebase, privdata->reglen, am);
     if (privdata->regs_phys == 0xffffffffffffffffULL) {
       vetar_msg(KERN_ERR "** vetar_probe_vme could not CesXpcBridge_MasterMap64 at vmebase 0x%x with length 0x%x !\n",
                privdata->vmebase, privdata->reglen);
         vetar_cleanup_dev(privdata);
         return -ENOMEM;
     }
 #else
     privdata->ctrl_regs_phys = xpc_vme_master_map(privdata->vmebase, 0, privdata->ctrl_reglen, am , 0);
     if (privdata->ctrl_regs_phys == 0xffffffffULL) {
       vetar_msg(KERN_ERR "** vetar_probe_vme could not xpc_vme_master_map at vmebase 0x%x with length 0x%lx !\n",
           privdata->vmebase, privdata->reglen);
         vetar_cleanup_dev(privdata);
         return -ENOMEM;
     }
 #endif
     mb();
     vetar_dbg(KERN_NOTICE "** vetar_probe_vme mapped control register vmebase 0x%x with length 0x%lx to physical address 0x%x, am:0x%x!\n",
             privdata->vmebase, privdata->ctrl_reglen, (unsigned int) privdata->ctrl_regs_phys,(unsigned) am);
     privdata->ctrl_registers = ioremap_nocache(privdata->ctrl_regs_phys, privdata->reglen);
     if (!privdata->registers) {
       vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at physical address 0x%x with length 0x%lx !\n",
           (unsigned int) privdata->regs_phys, privdata->reglen);
         vetar_cleanup_dev(privdata);
         return -ENOMEM;
      }
     mb();
     vetar_dbg(KERN_NOTICE "** vetar_probe_vme remapped physical address 0x%x to kernel address 0x%lx\n",
           (unsigned int) privdata->ctrl_regs_phys,  (unsigned long) privdata->ctrl_registers);





#endif

    /* mutex for read/write access of mapped memory (JAM: redundant!)*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
  init_MUTEX(&(privdata->ramsem));
#else
  sema_init(&(privdata->ramsem),1);
#endif

/* this is wishbone mutex:*/
  mutex_init(&privdata->wb_mutex);

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

   /* wishbone registration */
   privdata->wb.wops = &vetar_wb_ops;
   privdata->wb.parent = privdata->class_dev;




   err=wishbone_register(&privdata->wb);
   if (err< 0) {
        vetar_msg(KERN_ERR "Could not register wishbone bus, error %d\n",err);
        vetar_cleanup_dev(privdata);
        return err;
      }




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
	      vector[0]=0x60;
	      vetar_msg(KERN_NOTICE "No module parameters - use default address 0x%x, slot number 0x%x, lun 0x%x vector 0x%x\n",
	            vmebase[0],slot[0], lun[0],vector[0]);
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

MODULE_AUTHOR("Cesar Prados, Joern Adamczewski-Musch, GSI");
MODULE_DESCRIPTION("VETAR2 VME driver for CES xpc Linux");
MODULE_LICENSE("GPL");
