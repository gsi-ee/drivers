#include "vetar.h"



/** JAM taken from ipv trigmod example:*/

//struct pev_drv *pev_drv_p;
//struct pev_dev *pev;
//uint vme_itc_reg = 0;
//char *vme_cpu_addr = NULL;




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
static DEVICE_ATTR(wbctrl, S_IRUGO, vetar_sysfs_wbctrl_show, NULL);
static DEVICE_ATTR(vmecrcsr, S_IRUGO, vetar_sysfs_vmecrcsr_show, NULL);
static DEVICE_ATTR(dactl, (S_IWUSR | S_IRUGO) , vetar_sysfs_dactl_show, vetar_sysfs_dactl_store);

#endif
#endif








/** wrap read access to wishbone control space.*/
static unsigned int vetar_read_control( struct vetar_privdata *privdata, unsigned long offset)
{

  unsigned int dat=0;
#ifdef VETAR_MAP_CONTROLSPACE
  dat=be32_to_cpu(ioread32be(privdata->ctrl_registers +offset));
  vetar_bus_delay();
#endif
  return dat;

}

/** wrap write access to wishbone control space.*/
static void vetar_write_control( struct vetar_privdata *privdata, unsigned int dat, unsigned long offset)
{
#ifdef VETAR_MAP_CONTROLSPACE
  iowrite32be(cpu_to_be32(dat), privdata->ctrl_registers + offset);
  vetar_bus_delay();
#endif
}



/** wrap read access to wishbone data space.*/
static unsigned int vetar_read_data( struct vetar_privdata *privdata, unsigned long offset)
{
  unsigned int dat=0;
#ifdef VETAR_MAP_REGISTERS
  dat=be32_to_cpu(ioread32be(privdata->registers +offset));
#endif
  return dat;

}

/** wrap write access to wishbone data space.*/
static void vetar_write_data( struct vetar_privdata *privdata, unsigned int dat, unsigned long offset)
{
#ifdef VETAR_MAP_REGISTERS
  iowrite32be(cpu_to_be32(dat), privdata->registers + offset);
  vetar_bus_delay();
#endif
}



static void vetar_wb_cycle(struct wishbone* wb, int on)
{
   struct vetar_privdata *privdata;
   privdata = container_of(wb, struct vetar_privdata, wb);
   //vetar_dbg(KERN_ERR "*** vetar_wb_cycle...\n");
   if (on) mutex_lock(&privdata->wb_mutex);
   vetar_dbg(KERN_ERR "*** Vetar_WB: cycle(%d)\n",on);
   vetar_write_control(privdata, (on ? 0x80000000UL : 0) + 0x40000000UL, CTRL);
   if (!on)
     {
       ; // probably it is enough to switch back to mbs at end of wb cycle only JAM
       mutex_unlock(&privdata->wb_mutex);
     }

}




static wb_data_t vetar_wb_read_cfg(struct wishbone *wb, wb_addr_t addr)
{
   wb_data_t out=0;
   struct vetar_privdata *privdata;
   privdata = container_of(wb, struct vetar_privdata, wb);
   vetar_dbg(KERN_ERR "*** Vetar_WB:: READ CFG  addr 0x%x \n", addr);
   switch (addr) {
   case 0:  out = 0; break;
   case 4:  out = vetar_read_control(privdata, ERROR_FLAG); break;
   case 12: out = vetar_read_control(privdata, SDWB_ADDRESS); break;
   default: out = 0; break;
   };
   mb(); /* ensure serial ordering of non-posted operations for wishbone */
   vetar_dbg(KERN_ERR "*** Vetar_WB:: READ real CFG  value 0x%x \n", out);
   return out;
}

static void vetar_wb_write (struct wishbone* wb, wb_addr_t addr, wb_data_t data)
{
  struct vetar_privdata *privdata;
  wb_addr_t window_offset;
  vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_write.. ");
  privdata = container_of(wb, struct vetar_privdata, wb);
  addr = addr & WBM_ADD_MASK;
  window_offset = addr & WINDOW_HIGH;
  if (window_offset != privdata->wb_window_offset)
  {
    vetar_write_control (privdata, window_offset, WINDOW_OFFSET_LOW);
    privdata->wb_window_offset = window_offset;
  }
  vetar_dbg(KERN_ERR "*** Vetar_WB: WRITE(0x%x) => 0x%x\n", data, addr);
  vetar_write_data (privdata, data, (addr & WINDOW_LOW));
}

static wb_data_t vetar_wb_read (struct wishbone* wb, wb_addr_t addr)
{
  wb_data_t out = 0;
  wb_addr_t window_offset;
  struct vetar_privdata *privdata;
  vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_read.. ");
  privdata = container_of(wb, struct vetar_privdata, wb);
  addr = addr & WBM_ADD_MASK;
  window_offset = addr & WINDOW_HIGH;
  if (window_offset != privdata->wb_window_offset)
  {
    vetar_write_control (privdata, window_offset, WINDOW_OFFSET_LOW);
    privdata->wb_window_offset = window_offset;
  }
  out = vetar_read_data (privdata, (addr & WINDOW_LOW));
  vetar_dbg(KERN_ALERT "*** Vetar_WB: READ (%x) = %x \n", (addr), out);
  return out;
}

static int vetar_wb_request(struct wishbone *wb, struct wishbone_request *req)
{
  struct vetar_privdata *privdata;
  int out;
  uint32_t ctrl;
  vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_request.. ");
  privdata = container_of(wb, struct vetar_privdata, wb);
  ctrl = vetar_read_control(privdata, MASTER_CTRL);
  req->addr =  vetar_read_control(privdata, MASTER_ADD);
  req->data = vetar_read_control(privdata, MASTER_DATA);
  req->mask = ctrl & 0xf;
  req->write = (ctrl & 0x40000000) != 0;
  out = (ctrl & 0x80000000) != 0; // new fix by wesley from vme_wb_external git JAM
  if (out) vetar_write_control(privdata, 1, MASTER_CTRL); /*dequeue operation*/
  vetar_dbg(KERN_ALERT
                  "WB REQUEST:Request ctrl %x addr %x data %x mask %x return %x \n",
                  ctrl, req->addr, req->data, req->mask,
                  (ctrl & 0x80000000) != 0);
  ;
  return out;
}

static void vetar_wb_reply(struct wishbone *wb, int err, wb_data_t data)
{
  struct vetar_privdata *privdata;
  vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_reply.. ");
  privdata = container_of(wb, struct vetar_privdata, wb);
  vetar_write_control(privdata, data, MASTER_DATA);
  vetar_write_control(privdata, (err + 2), MASTER_CTRL);
  vetar_dbg(KERN_ALERT "WB REPLY: pushing data %x reply %x\n", data, err + 2);
}


static void vetar_wb_byteenable(struct wishbone* wb, unsigned char be)
{
  struct vetar_privdata *privdata;
  vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_byteenable.. \n");
  privdata = container_of(wb, struct vetar_privdata, wb);
  vetar_write_control(privdata, be, EMUL_DAT_WD);
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
      vetar_dbg(KERN_ERR "*** VETAR structure was not initialized!\n");
      return NULL;
    }
  return privdata;
}
	
#ifdef VETAR_ENABLE_IRQ

 static void vetar_irqhandler(struct pev_dev *pev, int src, void *arg)
 {
 struct vetar_privdata *priv = arg;
 int vec,level;
 vetar_dbg(KERN_NOTICE "** vetar_irqhandler with argument 0x%x , vme_itc=0x%x!\n",
                    (unsigned int) arg, priv->vme_itc);

 vec = src & 0xff;
 level = src >> 8;

 vetar_msg(KERN_INFO "VME IRQ: Level: 0x%x, Vector: 0x%x \n", level, vec);

 if(priv->vme_itc)
   outl (1<<level, priv->vme_itc + PEV_ITC_IMASK_CLEAR); // mask source of interrupt


if(level != priv->level)
  vetar_msg(KERN_INFO "VME IRQ: Warning: irq Level: 0x%x does not match vetar irq level 0x%x", level, priv->level);


 wishbone_slave_ready(&priv->wb);
 vetar_msg(KERN_INFO "VME IRQ: wishbone_slave_ready returned.\n");
 if(priv->vme_itc)
   outl (1<<priv->level, priv->vme_itc + PEV_ITC_IMASK_SET);


 return;



 }

#endif

static void vetar_cleanup_dev(struct vetar_privdata *privdata, unsigned int index) {

  if(privdata==0) return;


#ifdef VETAR_ENABLE_IRQ
  if(privdata->vme_itc)
      {
        vetar_dbg( KERN_ALERT "** vetar_cleanup_dev mask vme interupt level 0x%x, vector 0x%x \n", privdata->level, privdata->vector);
        outl (1<<privdata->level, privdata->vme_itc + 0xc);
      }
#endif

  if(privdata->wb_is_registered!=0)
    wishbone_unregister(&privdata->wb);

   /* disable the core */
  if(privdata->cr_csr)
     vetar_csr_write(ENABLE_CORE, privdata->cr_csr, BIT_CLR_REG);

 // JAM 1-2022: put these atlmas cleanups as used in vmvetrigmod
//  altmas_init_err_map:
//   if( mas->pg_idx >= 0)
//       {
//           alt_map_mas_free( mas->alt, MAP_ID_MAS_PCIE_PMEM, mas->pg_idx);
//           mas->pg_idx = -1;
//      }
//
//
// altmas_init_err_alloc_map:
//   kfree( mas->map_p);
//   // kfree altmas.mas;
// altmas_err_class:
//   cdev_del( &altmas.cdev);
// altmas_init_err_cdev_add:
//   unregister_chrdev_region( altmas.dev_id, altmas_dev_num);
// altmas_init_err_alloc_chrdev:




#ifdef VETAR_MAP_CONTROLSPACE
  if(privdata->ctrl_registers)
    {
      iounmap(privdata->ctrl_registers);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped registers 0x%lx !\n",
                    (unsigned long) privdata->ctrl_registers);
    }

  if(privdata->ctrl_regs_phys)
      {
       if( privdata->masmap_ctrl_regs->pg_idx >= 0)
          {
              alt_map_mas_free( privdata->mas_ctrl->alt, MAP_ID_MAS_PCIE_PMEM, privdata->masmap_ctrl_regs->pg_idx);
              //privdata->mas_ctrl->pg_idx = -1;
         }

    //if(pev_map_free(pev, &(privdata->pev_vetar_ctrl_regs))==0)

        vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys control registers 0x%x with length 0x%lx !\n",
            (unsigned int) privdata->ctrl_regs_phys, (unsigned long) privdata->ctrl_reglen);
      }
#endif // map controlspace

#ifdef VETAR_MAP_REGISTERS
  if(privdata->registers)
    {
      iounmap(privdata->registers);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped registers 0x%lx !\n",
                    (unsigned long) privdata->registers);
    }
  if(privdata->regs_phys)
      {
      if( privdata->masmap_regs->pg_idx >= 0)
             {
                 alt_map_mas_free( privdata->mas_regs->alt, MAP_ID_MAS_PCIE_PMEM, privdata->masmap_regs->pg_idx);
                 //privdata->mas_regs->pg_idx = -1;
            }
        vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys  registers 0x%x with length 0x%lx !\n",
              (unsigned int) privdata->regs_phys, (unsigned long) privdata->reglen);
      }
#endif // map registers


if(privdata->cr_csr)
  {
      iounmap(privdata->cr_csr);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped configspace  0x%lx !\n",
            (unsigned long) privdata->cr_csr);
  }
  if(privdata->cr_csr_phys)
    {
      if( privdata->masmap_csr->pg_idx >= 0)
               {
                   alt_map_mas_free( privdata->mas_csr->alt, MAP_ID_MAS_PCIE_PMEM, privdata->masmap_csr->pg_idx);
                   //privdata->mas_regs->pg_idx = -1;
              }
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys config registers 0x%x with length 0x%lx !\n",
          (unsigned int) privdata->cr_csr_phys, (unsigned long) privdata->configlen);
    }




#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (privdata->class_dev)
    {
#ifdef VETAR_SYSFS_ENABLE
    if(privdata->sysfs_has_file){
	  device_remove_file (privdata->class_dev, &dev_attr_dactl);
      device_remove_file (privdata->class_dev, &dev_attr_vmecrcsr);
      device_remove_file (privdata->class_dev, &dev_attr_wbctrl);
      device_remove_file(privdata->class_dev, &dev_attr_codeversion);
      privdata->sysfs_has_file=0;
    }
#endif
      device_destroy(vetar_class, privdata->devno);
      privdata->class_dev=0;
    }

#endif

  /* character device cleanup*/
  if(privdata->cdev.owner)
    cdev_del(&privdata->cdev);
  kfree(privdata);
  vetar_devices[index]=0;

}









ssize_t vetar_sysfs_codeversion_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs=0;
  curs += snprintf (buf + curs, PAGE_SIZE, "*** This is %s, version %s build on %s at %s \n",
      VETARDESC, VETARVERSION, __DATE__, __TIME__);
  curs += snprintf (buf + curs, PAGE_SIZE, "\tmodule authors: %s \n", VETARAUTHORS);
  curs += snprintf (buf + curs, PAGE_SIZE, "\tcompiled settings: \n");

#ifdef VETAR_ENABLE_IRQ
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tVETAR Interrupts are enabled.\n");
#else
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tVETAR Interrupts are disabled.\n");
#endif


  return curs;
}

ssize_t vetar_sysfs_wbctrl_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct vetar_privdata *privdata;
  privdata = (struct vetar_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** VETAR wishbone control register dump:\n");
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t CTRL:               \t0x %x\n",
      vetar_read_control (privdata, CTRL));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t MASTER_CTRL:        \t0x %x\n",
      vetar_read_control (privdata, MASTER_CTRL));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t MASTER_ADD:         \t0x %x\n",
      vetar_read_control (privdata, MASTER_ADD));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t MASTER_DATA:        \t0x %x\n",
      vetar_read_control (privdata, MASTER_DATA));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t EMUL_DAT_WD:        \t0x %x\n",
      vetar_read_control (privdata, EMUL_DAT_WD));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t WINDOW_OFFSET_LOW:  \t0x %x\n",
      vetar_read_control (privdata, WINDOW_OFFSET_LOW));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t WINDOW_OFFSET_HIGH: \t0x %x\n",
      vetar_read_control (privdata, WINDOW_OFFSET_HIGH));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t ERROR_FLAG:         \t0x %x\n",
      vetar_read_control (privdata, ERROR_FLAG));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t SDWB_ADDRESS:       \t0x %x\n",
      vetar_read_control (privdata, SDWB_ADDRESS));
  return curs;
}

ssize_t vetar_sysfs_vmecrcsr_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct vetar_privdata *privdata;
  privdata = (struct vetar_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** VETAR VME configuration register dump:\n");
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t VME_VENDOR_ID:    \t0x %02x %02x %02x\n",
      vetar_csr_read (privdata->cr_csr, VME_VENDOR_ID_OFFSET) ,
      vetar_csr_read (privdata->cr_csr, VME_VENDOR_ID_OFFSET + 4),
      vetar_csr_read (privdata->cr_csr, VME_VENDOR_ID_OFFSET + 8)
  ) ;
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t BOARD_ID:         \t0x %02x %02x %02x %02x\n",
       vetar_csr_read (privdata->cr_csr, BOARD_ID) ,
       vetar_csr_read (privdata->cr_csr, BOARD_ID + 4),
       vetar_csr_read (privdata->cr_csr, BOARD_ID + 8),
       vetar_csr_read (privdata->cr_csr, BOARD_ID + 12)
   ) ;
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t REVISION_ID:      \t0x %02x %02x %02x %02x\n",
        vetar_csr_read (privdata->cr_csr, REVISION_ID) ,
        vetar_csr_read (privdata->cr_csr, REVISION_ID + 4),
        vetar_csr_read (privdata->cr_csr, REVISION_ID + 8),
        vetar_csr_read (privdata->cr_csr, REVISION_ID + 12)
    ) ;
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t PROGRAM_ID:       \t0x %02x\n",
          vetar_csr_read (privdata->cr_csr, PROG_ID)
      ) ;
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t INTVECTOR:        \t0x %02x\n",
      vetar_csr_read (privdata->cr_csr, INTVECTOR));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t INT_LEVEL:        \t0x %02x\n",
      vetar_csr_read (privdata->cr_csr, INT_LEVEL));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t FUN0ADER:         \t0x %02x %02x %02x %02x \n",
      vetar_csr_read (privdata->cr_csr, FUN0ADER),
      vetar_csr_read (privdata->cr_csr, FUN0ADER + 4),
      vetar_csr_read (privdata->cr_csr, FUN0ADER + 8),
      vetar_csr_read (privdata->cr_csr, FUN0ADER + 12)
      );
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t FUN1ADER:         \t0x %02x %02x %02x %02x \n",
      vetar_csr_read (privdata->cr_csr, FUN1ADER),
      vetar_csr_read (privdata->cr_csr, FUN1ADER + 4),
      vetar_csr_read (privdata->cr_csr, FUN1ADER + 8),
      vetar_csr_read (privdata->cr_csr, FUN1ADER + 12) );
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t WB_32_64:         \t0x %02x\n",
      vetar_csr_read (privdata->cr_csr, WB_32_64));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t BIT_SET_REG:      \t0x %02x\n",
      vetar_csr_read (privdata->cr_csr, BIT_SET_REG));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t BIT_CLR_REG:      \t0x %02x\n",
      vetar_csr_read (privdata->cr_csr, BIT_CLR_REG));
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t BYTES:            \t0x %02x %02x\n",
        vetar_csr_read (privdata->cr_csr, BYTES) ,
        vetar_csr_read (privdata->cr_csr, BYTES + 4)
    ) ;
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t TIME (ns):        \t0x %02x %02x %02x %02x %02x \n",
        vetar_csr_read (privdata->cr_csr, TIME),
        vetar_csr_read (privdata->cr_csr, TIME + 4),
        vetar_csr_read (privdata->cr_csr, TIME + 8),
        vetar_csr_read (privdata->cr_csr, TIME + 12),
        vetar_csr_read (privdata->cr_csr, TIME + 16)
        );
  return curs;
}

//////// NEW for directSaft mode

ssize_t vetar_sysfs_dactl_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  unsigned int val=0;
    struct vetar_privdata *privdata;
   privdata = (struct vetar_privdata*) dev_get_drvdata (dev);
  val=vetar_read_control(privdata, DIRECT_ACCESS_CONTROL);
  vetar_msg( KERN_NOTICE "VETAR: reading from dactl register the value 0x%x\n", val);

   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", val);
   return curs;
}

ssize_t vetar_sysfs_dactl_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct vetar_privdata *privdata;
  privdata = (struct vetar_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   vetar_write_control(privdata, val, DIRECT_ACCESS_CONTROL);
   vetar_msg( KERN_NOTICE "VETAR: wrote to dactl register the value 0x%x\n", val);
  return count;
}



//////////////

static struct file_operations vetar_fops = {
#if 0
	.open           = vetar_open,
	.release        = vetar_release,
	.llseek =   vetar_llseek,
	.read =           vetar_read,
#endif
};



int vetar_is_present(struct vetar_privdata *privdata)
{
    uint32_t idc;
    void* addr;
    vetar_dbg(KERN_ERR "Check if VETAR is present at slot %d, config base address 0x%x\n", privdata->slot, privdata->configbase);
    addr = privdata->cr_csr + VME_VENDOR_ID_OFFSET;
    vetar_dbg(KERN_NOTICE "Reading Vendor id from address %p ...\n", addr);
    mb();
    idc = be32_to_cpu(ioread32be(addr)) << 16;
  vetar_crcsr_delay();
    idc += be32_to_cpu(ioread32be(addr + 4))  << 8;
  vetar_crcsr_delay();
    idc += be32_to_cpu(ioread32be(addr + 8));
  vetar_crcsr_delay();
  if (idc == VETAR_VENDOR_ID)
  {
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
  vetar_crcsr_delay();
  vetar_dbg(KERN_NOTICE "vetar_csr_write value 0x%x to base %p + offset 0x%x \n", value, base, offset);
}

u32 vetar_csr_read(void *base, u32 offset)
{
    u32 value=0;
    offset -= offset % 4;
    value=ioread32be(base + offset);
  vetar_crcsr_delay();
  vetar_dbg(KERN_NOTICE "vetar_csr_read value 0x%x from base %p + offset 0x%x \n", value,  base, offset);
    return value;
}



///** Set elb address window to current address modifier and base address window*/
//void vetar_elb_set_window( uint am, uint32_t base)
//{
//  uint vme_am;
//  int data;
//     if(am == 0x0d)      vme_am = 0x38 | MAP_VME_ELB_A32 | MAP_VME_ELB_SP;
//     else if( am == 0x39) vme_am = 0x38 | MAP_VME_ELB_A24;
//     else if( am == 0x3d) vme_am = 0x38 | MAP_VME_ELB_A24 | MAP_VME_ELB_SP;
//     else if( am == 0x29) vme_am = 0x38 | MAP_VME_ELB_A16;
//     else if( am == 0x2d) vme_am = 0x38 | MAP_VME_ELB_A16 | MAP_VME_ELB_SP;
//     else if( am == 0x00) vme_am = 0x38 | MAP_VME_ELB_IACK;
//     else                    vme_am = 0x38 | MAP_VME_ELB_A32;
//     if( pev->csr_ptr)
//     {
//       data = (vme_am << 24) | ((base&0xf0000000) >> 24);
//       *(uint *)(pev->csr_ptr + PEV_CSR_VME_ELB) = data;
//     }
//     else
//     {
//       data = vme_am | (base&0xf0000000);
//       if( pev->io_remap[0].short_io)
//       {
//         outl( data, pev->io_base +  PEV_SCSR_VME_ELB);
//       }
//       else
//       {
//         outl( data, pev->io_base +  PEV_CSR_VME_ELB);
//       }
//     }
//     //vetar_dbg(KERN_NOTICE "vetar_elb_set window for am: 0x%x , base high=0x%x\n", am, base&0xf0000000);
//}
//
//
///** map elb window, but do not assing any interrupts here:*/
//char __iomem* vetar_elb_map( struct vme_board *v)
//{
//  char __iomem* mapped;
//  uint elb_off;
//  uint vme_size;
//   vme_size =  (v->size + 0xfff) & 0x0ffff000; /* align to multiple of 4k */
//   elb_off = v->base & 0x0ffff000;             /* align to multiple of 4k */
//   mapped = ioremap( pev->elb_base + elb_off, vme_size); /* map vme base address through ELB bus */
//   mapped +=  (v->base & 0x0fffffff) - elb_off;
//   vetar_elb_set_window( v->am, v->base);
//   vetar_msg(KERN_NOTICE "vetar_elb_map returns iomem base: 0x%lx\n",
//        (unsigned long) mapped);
//
//    return mapped;
//}
//



/** JAM this is original by ioxos:*/
//void
//vme_board_register( struct vme_board *v,
//            void (* func)( struct pev_dev*, int, void *),
//            void *arg)
//
//{
//  uint elb_off;
//  uint vme_am;
//  uint vme_size;
//  int i, data;
//
//  for( i = 1; i < 8; i++)
//  {
//    pev_irq_register( pev, EVT_SRC_VME + i, func, arg);
//  }
//  vme_itc_reg = pev->io_base + pev->io_remap[0].vme_itc;
//
//  if( v->am == 0x0d)      vme_am = 0x38 | MAP_VME_ELB_A32 | MAP_VME_ELB_SP;
//  else if( v->am == 0x39) vme_am = 0x38 | MAP_VME_ELB_A24;
//  else if( v->am == 0x3d) vme_am = 0x38 | MAP_VME_ELB_A24 | MAP_VME_ELB_SP;
//  else if( v->am == 0x29) vme_am = 0x38 | MAP_VME_ELB_A16;
//  else if( v->am == 0x2d) vme_am = 0x38 | MAP_VME_ELB_A16 | MAP_VME_ELB_SP;
//  else if( v->am == 0x00) vme_am = 0x38 | MAP_VME_ELB_IACK;
//  else                    vme_am = 0x38 | MAP_VME_ELB_A32;
//
//  vme_size =  (v->size + 0xfff) & 0x0ffff000; /* align to multiple of 4k */
//  elb_off = v->base & 0x0ffff000;             /* align to multiple of 4k */
//  vme_cpu_addr = ioremap( pev->elb_base + elb_off, vme_size); /* map vme base address through ELB bus */
//  vme_cpu_addr +=  (v->base & 0x0fffffff) - elb_off
//  if( pev->csr_ptr)
//  {
//    data = (vme_am << 24) | ((v->base&0xf0000000) >> 24);
//    *(uint *)(pev->csr_ptr + PEV_CSR_VME_ELB) = data;
//  }
//  else
//  {
//    data = vme_am | (v->base&0xf0000000);
//    if( pev->io_remap[0].short_io)
//    {
//      outl( data, pev->io_base +  PEV_SCSR_VME_ELB);
//    }
//    else
//    {
//      outl( data, pev->io_base +  PEV_CSR_VME_ELB);
//    }
//  }
//  vetar_msg(KERN_NOTICE "vme_cpu_addr: 0x%x \n", (unsigned long) vme_cpu_addr);
//
//}
//




void vetar_setup_csr_fa(struct vetar_privdata *privdata)
{
    int i,offset;
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
      vetar_dbg(KERN_NOTICE "vetar_setup_csr_fa initializes ADER %d at register 0x%x with AM:0x%x\n",i,offset,am);
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
    //am= VME_A32_USER_DATA_SCT; // *0x09*/
    //am=VME_A32_SUP_DATA_SCT;// 0x0D;
    //am= VME_A32_USER_MBLT; // *0x08*/
    //am=0x50; // JAM compare with rio4 driver !?
    
     // new for balloon:
     am = VME_A32_USER_DATA_SCT; /*0x09*/
    
    fa[0] = (privdata->vmebase >> 24) & 0xFF;
    fa[1] = (privdata->vmebase >> 16) & 0xFF;
    fa[2] = (privdata->vmebase >> 8 ) & 0xFF;
    fa[3] = (am & 0x3F) << 2;
            /* DFSR and XAM are zero */

    vetar_csr_write(fa[0], privdata->cr_csr, FUN0ADER);
    vetar_csr_write(fa[1], privdata->cr_csr, FUN0ADER + 4);
    vetar_csr_write(fa[2], privdata->cr_csr, FUN0ADER + 8);
    vetar_csr_write(fa[3], privdata->cr_csr, FUN0ADER + 12);
#endif

#ifdef VETAR_MAP_CONTROLSPACE
    /*do address relocation for FUN1, WB control mapping*/
    //am=0x39; /* JAM This is what we actually see on the vmebus monitor for (XPC_VME_ATYPE_A24 | XPC_VME_DTYPE_BLT | XPC_VME_PTYPE_USER)*/
    //am=VME_A24_SUP_DATA_SCT;
    //am = VME_A24_USER_MBLT; //0x38;
     // new for balloon release?!:
    am= VME_A24_USER_DATA_SCT; /*0x39*/
    vetar_msg(KERN_NOTICE "vetar_setup_csr_fa sets control space address modifier 0x%x\n", am);


     fa[0] = (privdata->ctrl_vmebase >> 24) & 0xFF;
     fa[1] = (privdata->ctrl_vmebase >> 16) & 0xFF;
     fa[2] = (privdata->ctrl_vmebase >> 8 ) & 0xFF;
     fa[3] = (am & 0x3F) << 2;

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
  int i=0;
  int offset=0;
  struct vetar_privdata *privdata;
  vetar_msg(KERN_NOTICE "VETAR vme driver starts probe for index %d\n",index);
  vetar_msg(KERN_NOTICE "Use parameters address 0x%x, slot number 0x%x, lun 0x%x vector 0x%x\n",
                     vmebase[index],slot[index], lun[index],vector[index]);
  /* Allocate and initialize the private data for this device */
    privdata = kzalloc(sizeof(struct vetar_privdata), GFP_KERNEL);
    if (privdata == NULL)
      {
        vetar_cleanup_dev(privdata, index);
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

  /* below as in new vme_wb_external:*/
  privdata->ctrl_vmebase=privdata->slot*0x400; // link control address to slot number
  privdata->vmebase=privdata->slot * 0x10000000; // link wishbone adress space to slot number
  //privdata->vmebase=privdata->slot * 0x1000000; // low mapping better?
  // first try to map and look up configuration space if any....
  privdata->configbase = privdata->slot * VETAR_CONFIGSIZE;
  privdata->configlen=VETAR_CONFIGSIZE;

////////////////////////////////////////////////////////////////////////////
// JAM  here is mapping for ifc:


  // TODO 2022: first create privdata->altmas !!!
  privdata->altmas_dev_num=3; // to be safe, use separate "device" structure for each addressing window
  privdata->altmas.mas = (struct altmas_device **)kzalloc(privdata->altmas_dev_num*sizeof(struct altmas_device *), GFP_KERNEL);
  for( i = 0; i < privdata->altmas_dev_num; i++)
     {
        privdata->altmas.mas[i]  = (struct altmas_device *)kzalloc(sizeof(struct altmas_device), GFP_KERNEL);
        privdata->altmas.mas[i]->use_cnt=0;
        mutex_init( & privdata->altmas.mas[i]->mutex);
     }
    vetar_dbg(KERN_NOTICE "vme: attaching to Althea driver...");
    privdata->altmas.alt = althea7910_register();

    privdata->mas_csr=privdata->altmas.mas[0]; // JAM just for convenience
         if( !privdata->mas_csr->use_cnt)
          {
            privdata->mas_csr->alt = privdata->altmas.alt;
            vetar_dbg(KERN_NOTICE "altmas: ALTHEA control device handle: %p\n", privdata->mas_csr->alt);
            privdata->mas_csr->map_p = (struct alt_ioctl_mas_map *)kzalloc( sizeof(struct alt_ioctl_mas_map), GFP_KERNEL);
            privdata->mas_csr->done = 0;
            privdata->mas_csr->pg_idx = MAS_MAP_IDX_INV;
            privdata->mas_csr->map_p->ivec = MAS_MAP_NO_IVEC;    /* interrupt vector not yet assigned */
          }
          /* increment device use use_cnt */
    privdata->mas_csr->use_cnt += 1;

    //privdata->masmap = mas->map_p;
      //       debugk("mas = %p - pg_idx = %x\n", mas, mas->pg_idx);
      //
   // privdata->masmap->ivec = TRIGMOD_IRQ_VECTOR;
   // privdata->masmap->size = TRIGMOD_REGS_SIZE;
      //       masmap->mode = TRIGMOD_VME_AM;


    // TODO JAM cscr space

    privdata->masmap_csr = privdata->mas_csr->map_p;
    privdata->masmap_csr->mode =VME_AM_CRCSR; // 0x2f

    privdata->map_win_csr.req.rem_addr = privdata->configbase;
    privdata->map_win_csr.req.loc_addr = MAP_LOC_ADDR_AUTO;
    privdata->map_win_csr.req.size = privdata->configlen;
    privdata->map_win_csr.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
    privdata->map_win_csr.req.mode.am = (char)(privdata->masmap_csr->mode & VME_AM_MASK);
    privdata->map_win_csr.req.mode.space = MAP_SPACE_VME;
    privdata->map_win_csr.req.mode.swap = (char)((privdata->masmap_csr->mode & MAS_MAP_MODE_SWAP_MASK)>>8);


    vetar_dbg(KERN_NOTICE "CRCSRmap window: rem_addr:0x%llx, loc_addr:0x%llx, size:0x%x, mode.sg_id:0x%x, mode.am: 0x%x, mode.space:0x%x, mode.swap:0x%x\n",
        privdata->map_win_csr.req.rem_addr,
    privdata->map_win_csr.req.loc_addr,
    privdata->map_win_csr.req.size,
    privdata->map_win_csr.req.mode.sg_id,
    privdata->map_win_csr.req.mode.am,
    privdata->map_win_csr.req.mode.space,
    privdata->map_win_csr.req.mode.swap);



    if( privdata->masmap_csr->mode & MAS_MAP_VME_SPLIT)
        {
           privdata->map_win_csr.req.mode.swap |= MAP_SPLIT_D32;
        }
   privdata->map_win_csr.req.mode.flags = 0;
   privdata->mas_csr->pg_idx = alt_map_mas_alloc( privdata->mas_csr->alt, &privdata->map_win_csr);
   privdata->masmap_csr->pg_idx = privdata->map_win_csr.pg_idx;
   if( privdata->mas_csr->pg_idx < 0)
        {
              vetar_msg(KERN_ERR "cannot allocate map: loc_addr = %llx\n",  privdata->map_win_csr.req.loc_addr);
              vetar_cleanup_dev(privdata, index);
              return -ENOMEM;
              //goto altmas_init_err_alloc_map;
        }

        /* set mapping done */
   //mas->done = 1;
   vetar_dbg(KERN_NOTICE "AM:0x%x, pg_idx = %x - loc_addr = %llx\n", privdata->map_win_csr.req.mode.am, privdata->masmap_csr->pg_idx , privdata->masmap_csr->loc_addr);
   alt_map_mas_get(privdata->mas_csr->alt, &privdata->map_win_csr);
   privdata->masmap_csr->rem_addr = privdata->map_win_csr.sts.rem_base;
   privdata->masmap_csr->loc_addr = privdata->map_win_csr.sts.loc_base;
   privdata->masmap_csr->size =  privdata->map_win_csr.sts.size;
   vetar_dbg(KERN_NOTICE "size = %x - loc_addr = %llx rem_addr = %llx\n", privdata->masmap_csr->size, privdata->masmap_csr->loc_addr, privdata->masmap_csr->rem_addr);


   /* From examples/AltMasMapVme.d: calulate vme board offset in decoding window!! */
   // offset = vme_addr - mas_map.rem_addr;
   offset = privdata->configbase - privdata->masmap_csr->rem_addr;
   vetar_dbg(KERN_NOTICE "mapped offset:  0x%x\n", offset);

   //privdata->cr_csr_phys=privdata->masmap_csr->loc_addr;
   privdata->cr_csr_phys=privdata->masmap_csr->loc_addr + offset;
   mb();

    // we have to ioremap this address to use it in kernel module?
    privdata->cr_csr = ioremap_nocache(privdata->cr_csr_phys, privdata->configlen);
    if (!privdata->cr_csr) {
          vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at config physical address 0x%x with length 0x%lx !\n",
              (unsigned int) privdata->cr_csr_phys, privdata->configlen);
            vetar_cleanup_dev(privdata, index);
            return -ENOMEM;
         }

    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme remapped config base 0x%x to kernel address 0x%lx\n",
             (unsigned int) privdata->configbase,  (unsigned long) privdata->cr_csr);


    msleep(10);

    // TEST: read out cap registers:

//    vetar_csr_read (privdata->cr_csr, ADEM0);
//    vetar_csr_read (privdata->cr_csr, ADEM0 + 4);
//    vetar_csr_read (privdata->cr_csr, ADEM0 + 8);
//    vetar_csr_read (privdata->cr_csr, ADEM0 + 12);
//
//    vetar_csr_read (privdata->cr_csr, ADEM1);
//    vetar_csr_read (privdata->cr_csr, ADEM1 + 4);
//    vetar_csr_read (privdata->cr_csr, ADEM1 + 8);
//    vetar_csr_read (privdata->cr_csr, ADEM1 + 12);
  //  may check for vendor id etc...
if(!vetar_is_present(privdata))
  {
    //vetar_cleanup_dev(privdata, index);
    return -EFAULT;
  }



  // setup interrupts:

#ifdef VETAR_ENABLE_IRQ
//
//// JAM this is also done in the vme_board_register function. might do it for one virtual board region only!
////snprintf(privdata->irqname, 64, VETARNAMEFMT,privdata->lun);
////  result = xpc_vme_request_irq(INTVECTOR, (1 << INT_LEVEL) , vetar_irqhandler, privdata, privdata->irqname);
////    if (result)
////    {
////    }
////  vetar_msg(KERN_ERR "** vetar_probe_vme with irq handler, result=%d \n",result);
//

//for( i = 1; i < 8; i++)
// {
//   pev_irq_register( pev, EVT_SRC_VME + i,  vetar_irqhandler, privdata);
// }
//  privdata->vme_itc = pev->io_base + pev->io_remap[0].vme_itc;
 
vetar_msg(KERN_ERR "** TODO? vetar_probe_vme with irq handler.\n");
//
#endif




vetar_setup_csr_fa(privdata);


#ifdef VETAR_MAP_REGISTERS



// map via pci bus,  like the configuration space
 privdata->mas_regs=privdata->altmas.mas[1]; // JAM just for convenience
 if( !privdata->mas_regs->use_cnt)
           {
             privdata->mas_regs->alt = privdata->altmas.alt;
             vetar_dbg(KERN_NOTICE "altmas: ALTHEA register device handle: %p\n", privdata->mas_regs->alt);
             privdata->mas_regs->map_p = (struct alt_ioctl_mas_map *)kzalloc( sizeof(struct alt_ioctl_mas_map), GFP_KERNEL);
             privdata->mas_regs->done = 0;
             privdata->mas_regs->pg_idx = MAS_MAP_IDX_INV;
             privdata->mas_regs->map_p->ivec = MAS_MAP_NO_IVEC;    /* interrupt vector not yet assigned */
           }
           /* increment device use use_cnt */
     privdata->mas_regs->use_cnt += 1;




 privdata->masmap_regs = privdata->mas_regs->map_p;
 privdata->masmap_regs->mode =VME_AM_A32 | VME_AM_DATA; // 0x9

 privdata->map_win_regs.req.rem_addr = privdata->vmebase;
 privdata->map_win_regs.req.loc_addr = MAP_LOC_ADDR_AUTO;
 privdata->map_win_regs.req.size = privdata->reglen;
 privdata->map_win_regs.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
 privdata->map_win_regs.req.mode.am = (char)(privdata->masmap_regs->mode & VME_AM_MASK);
 privdata->map_win_regs.req.mode.space = MAP_SPACE_VME;
 privdata->map_win_regs.req.mode.swap = (char)((privdata->masmap_regs->mode & MAS_MAP_MODE_SWAP_MASK)>>8);

    if( privdata->masmap_regs->mode & MAS_MAP_VME_SPLIT)
         {
            privdata->map_win_regs.req.mode.swap |= MAP_SPLIT_D32;
         }
    privdata->map_win_regs.req.mode.flags = 0;
    privdata->mas_regs->pg_idx = alt_map_mas_alloc( privdata->mas_regs->alt, &privdata->map_win_regs);
    privdata->masmap_regs->pg_idx = privdata->map_win_regs.pg_idx;
    if( privdata->mas_regs->pg_idx < 0)
         {
               vetar_msg(KERN_ERR "cannot allocate map: loc_addr = %llx\n",  privdata->map_win_regs.req.loc_addr);
               vetar_cleanup_dev(privdata, index);
               return -ENOMEM;
               //goto altmas_init_err_alloc_map;
         }

           /* set mapping done */
      //mas->done = 1;
      vetar_dbg(KERN_NOTICE "pg_idx = %x - loc_addr = %llx\n", privdata->masmap_regs->pg_idx , privdata->masmap_regs->loc_addr);
      alt_map_mas_get(privdata->mas_regs->alt, &privdata->map_win_regs);
      privdata->masmap_regs->rem_addr = privdata->map_win_regs.sts.rem_base;
      privdata->masmap_regs->loc_addr = privdata->map_win_regs.sts.loc_base;
      privdata->masmap_regs->size =  privdata->map_win_regs.sts.size;
      vetar_dbg(KERN_NOTICE "size = %x - loc_addr = %llx rem_addr = %llx\n", privdata->masmap_regs->size, privdata->masmap_regs->loc_addr, privdata->masmap_regs->rem_addr);

      /* need to correct offset in mapping window to our real base:*/
      offset = privdata->vmebase - privdata->masmap_regs->rem_addr;
      vetar_dbg(KERN_NOTICE "mapped offset:  0x%x\n", offset);

      privdata->regs_phys=privdata->masmap_regs->loc_addr + offset;



  mb();


  // we have to ioremap this address to use it in kernel module?
  privdata->registers = ioremap_nocache(privdata->regs_phys, privdata->reglen);
  if (!privdata->registers) {
        vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at registers physical address 0x%x with length 0x%lx !\n",
            (unsigned int) privdata->regs_phys, privdata->reglen);
          vetar_cleanup_dev(privdata, index);
          return -ENOMEM;
       }

    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme remapped vme base 0x%x to kernel address 0x%lx\n",
          (unsigned int) privdata->vmebase,  (unsigned long) privdata->registers);

#endif




#ifdef VETAR_MAP_CONTROLSPACE

// JAM: third time for control space:

    privdata->mas_ctrl=privdata->altmas.mas[2]; // JAM just for convenience
    if( !privdata->mas_ctrl->use_cnt)
               {
                 privdata->mas_ctrl->alt = privdata->altmas.alt;
                 vetar_dbg(KERN_NOTICE "altmas: ALTHEA register device handle: %p\n", privdata->mas_ctrl->alt);
                 privdata->mas_ctrl->map_p = (struct alt_ioctl_mas_map *)kzalloc( sizeof(struct alt_ioctl_mas_map), GFP_KERNEL);
                 privdata->mas_ctrl->done = 0;
                 privdata->mas_ctrl->pg_idx = MAS_MAP_IDX_INV;
                 privdata->mas_ctrl->map_p->ivec = MAS_MAP_NO_IVEC;    /* interrupt vector not yet assigned */
               }
               /* increment device use use_cnt */
         privdata->mas_ctrl->use_cnt += 1;







    privdata->ctrl_reglen=VETAR_CTRLREGS_SIZE;


    privdata->masmap_ctrl_regs = privdata->mas_ctrl->map_p;

    privdata->masmap_ctrl_regs->mode =VME_AM_A24 | VME_AM_DATA; // 0x39



     privdata->map_win_ctrl_regs.req.rem_addr = privdata->ctrl_vmebase;
     privdata->map_win_ctrl_regs.req.loc_addr = MAP_LOC_ADDR_AUTO;
     privdata->map_win_ctrl_regs.req.size = privdata->ctrl_reglen;
     privdata->map_win_ctrl_regs.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
     privdata->map_win_ctrl_regs.req.mode.am = (char)(privdata->masmap_ctrl_regs->mode & VME_AM_MASK);
     privdata->map_win_ctrl_regs.req.mode.space = MAP_SPACE_VME;
     privdata->map_win_ctrl_regs.req.mode.swap = (char)((privdata->masmap_ctrl_regs->mode & MAS_MAP_MODE_SWAP_MASK)>>8);

        if( privdata->masmap_ctrl_regs->mode & MAS_MAP_VME_SPLIT)
             {
                privdata->map_win_ctrl_regs.req.mode.swap |= MAP_SPLIT_D32;
             }
        privdata->map_win_ctrl_regs.req.mode.flags = 0;
        privdata->mas_ctrl->pg_idx = alt_map_mas_alloc( privdata->mas_ctrl->alt, &privdata->map_win_ctrl_regs);
        privdata->masmap_ctrl_regs->pg_idx = privdata->map_win_ctrl_regs.pg_idx;
        if( privdata->mas_ctrl->pg_idx < 0)
             {
                   vetar_msg(KERN_ERR "cannot allocate map: loc_addr = %llx\n",  privdata->map_win_ctrl_regs.req.loc_addr);
                   vetar_cleanup_dev(privdata, index);
                   return -ENOMEM;
                   //goto altmas_init_err_alloc_map;
             }
        /* set mapping done */
          //mas->done = 1;
          vetar_dbg(KERN_NOTICE "pg_idx = %x - loc_addr = %llx\n", privdata->masmap_ctrl_regs->pg_idx , privdata->masmap_ctrl_regs->loc_addr);
          alt_map_mas_get(privdata->mas_ctrl->alt, &privdata->map_win_ctrl_regs);
          privdata->masmap_ctrl_regs->rem_addr = privdata->map_win_ctrl_regs.sts.rem_base;
          privdata->masmap_ctrl_regs->loc_addr = privdata->map_win_ctrl_regs.sts.loc_base;
          privdata->masmap_ctrl_regs->size =  privdata->map_win_ctrl_regs.sts.size;
          vetar_dbg(KERN_NOTICE "size = %x - loc_addr = %llx rem_addr = %llx\n", privdata->masmap_ctrl_regs->size, privdata->masmap_ctrl_regs->loc_addr, privdata->masmap_ctrl_regs->rem_addr);

          /* need to correct offset in mapping window to our real base:*/
          offset = privdata->ctrl_vmebase - privdata->masmap_ctrl_regs->rem_addr;
          vetar_dbg(KERN_NOTICE "mapped offset:  0x%x\n", offset);
          privdata->ctrl_regs_phys=privdata->masmap_ctrl_regs->loc_addr + offset;
    mb();


    // we have to ioremap this address to use it in kernel module
    privdata->ctrl_registers = ioremap_nocache(privdata->ctrl_regs_phys, privdata->ctrl_reglen);
    if (!privdata->ctrl_registers) {
          vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at control registers physical address 0x%x with length 0x%lx !\n",
              (unsigned int) privdata->ctrl_regs_phys, privdata->ctrl_reglen);
            vetar_cleanup_dev(privdata, index);
            return -ENOMEM;
         }





//#endif


///////////////////////// JAM end third region

     mb();
     vetar_dbg(KERN_NOTICE "** vetar_probe_vme remapped control vme base address 0x%x to kernel address 0x%lx\n",
           (unsigned int) privdata->ctrl_vmebase,  (unsigned long) privdata->ctrl_registers);





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
       vetar_cleanup_dev(privdata, index);
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
//    vetar_msg(KERN_NOTICE "VETAR device ");
//       vetar_msg(KERN_NOTICE VETARNAMEFMT, MINOR(vetar_devt) + privdata->lun);
//    vetar_msg(KERN_NOTICE " has been added. \n");

    vetar_msg(KERN_NOTICE "VETAR device:\t "VETARNAMEFMT" has been added. \n", MINOR(vetar_devt) + privdata->lun);

 #ifdef VETAR_SYSFS_ENABLE
  if(device_create_file(privdata->class_dev, &dev_attr_codeversion) != 0)
     {
       vetar_msg(KERN_ERR "Could not add device file node for code version.\n");
     }
  else
    {
      // TODO: check validitiy of dev_att directly?
      privdata->sysfs_has_file=1;
    }


    if (device_create_file (privdata->class_dev, &dev_attr_wbctrl) != 0)
    {
      vetar_msg(KERN_ERR "Could not add device file node for wishbone control registers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_vmecrcsr) != 0)
    {
      vetar_msg(KERN_ERR "Could not add device file node for vme config registers.\n");
    }

     if (device_create_file (privdata->class_dev, &dev_attr_dactl) != 0)
    {
      vetar_msg(KERN_ERR "Could not add device file node for direct access control register.\n");
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
        vetar_cleanup_dev(privdata, index);
        return err;
      }
   privdata->wb_is_registered=1;

#ifdef VETAR_MAP_CONTROLSPACE
vetar_dbg(KERN_NOTICE "Init control registers\n");
       
	iowrite32be(0, privdata->ctrl_registers + EMUL_DAT_WD);
	vetar_bus_delay();
	iowrite32be(0, privdata->ctrl_registers + WINDOW_OFFSET_LOW);
	vetar_bus_delay();
	iowrite32be(0, privdata->ctrl_registers + MASTER_CTRL);
	vetar_bus_delay();
#endif



#ifdef VETAR_ENABLE_IRQ
	//privdata->vme_itc=vme_itc_reg;
	vetar_msg( KERN_ALERT "enable vme interupt level 0x%x, vector 0x%x \n", privdata->level, privdata->vector);
	    outl (1<<privdata->level, privdata->vme_itc + PEV_ITC_IMASK_CLEAR);

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
    vetar_cleanup_dev(vetar_devices[i], i);
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

MODULE_AUTHOR(VETARAUTHORS);
MODULE_DESCRIPTION(VETARDESC);
MODULE_LICENSE("GPL");
MODULE_VERSION(VETARVERSION);
