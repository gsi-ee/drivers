#include "vetar.h"



/** JAM taken from ipv trigmod example:*/

struct pev_drv *pev_drv_p;
struct pev_dev *pev;
uint vme_itc_reg = 0;
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
#endif
#endif

#ifdef VETAR_PEV_DUMP
///////////////////////////////////////////////////////////////////////////
/////////// JAM ripped from pev driver /usr/src/PEV1100/drivers/rdwrlib.c

int
rdwr_swap_32( int data)
{
  char ci[4];
  char co[4];

  *(int *)ci = data;
  co[0] = ci[3];
  co[1] = ci[2];
  co[2] = ci[1];
  co[3] = ci[0];

  return( *(int *)co);
}



#ifdef PPC
short rdwr_swap_16( short);
int rdwr_swap_32( int);
#define SWAP16(x) rdwr_swap_16(x)
#define SWAP32(x) rdwr_swap_32(x)
#else
#define SWAP16(x) x
#define SWAP32(x) x
#endif



#if defined(PPC)

int
pev_inl( struct pev_dev *pev, uint addr)
{
  if( pev->csr_remap)
  {
    int data;
    data = *(int *)(pev->csr_ptr + addr);
    data = SWAP32( data);
    return( data);
  }
  else
  {
    return( inl( pev->io_base + addr));
  }
}
void
pev_outl( struct pev_dev *pev, uint data, uint addr)
{
  //volatile uint tmp;

  if( pev->csr_remap)
  {
    data = SWAP32( data);
    *(uint *)(pev->csr_ptr + addr) = data;
    //tmp = *(uint *)(pev->csr_ptr + addr);
  }
  else
  {
    outl( data, pev->io_base + addr);
    //tmp = inl( pev->io_base + addr);
  }
  return;
}
#else
int
pev_inl( struct pev_dev *pev, uint addr)
{
  return( inl( pev->io_base + addr));
}
void
pev_outl( struct pev_dev *pev, uint data, uint addr)
{
  outl( data, pev->io_base + addr);
  return;
}
#endif


#endif


// end test code IOxOS
////////////////////////////////////////////////////////////
///////////////////////////////////////////////////

#ifdef VETAR_PEV_DUMP
static void vetar_dump_error_regs()
{
#ifdef DEBUG
//#if 0
  int add=0,stat=0;
  add= pev_inl( pev, pev->io_remap[pev->csr_remap].vme_base + 0x18);
  stat= pev_inl( pev, pev->io_remap[pev->csr_remap].vme_base + 0x1c);
  mb();
  vetar_msg(KERN_NOTICE "Dump address err: 0x%x status err : 0x%x \n",add,stat);
#endif
}

#else

#define vetar_dump_error_regs() ;

#endif

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
#ifdef VETAR_MAP_ELB_CTRL
/* switch elb mapping to wb control space*/
void vetar_elb_switch_control(struct vetar_privdata *privdata)
{
  if(privdata->elb_am_mode== VETAR_ELB_CONTROL) return;
  //ndelay(50);
  vetar_elb_set_window(VETAR_ELB_CONTROL, privdata->ctrl_vmebase);
  privdata->elb_am_mode= VETAR_ELB_CONTROL;
  vetar_elb_delay();
}
#else
#define vetar_elb_switch_control(x) ;
#endif

#ifdef VETAR_MAP_ELB_REG
/* switch elb mapping to wb data space*/
void vetar_elb_switch_data(struct vetar_privdata *privdata)
{
  if(privdata->elb_am_mode== VETAR_ELB_DATA) return;
  //ndelay(50);
  vetar_elb_set_window(VETAR_ELB_DATA, privdata->vmebase);
  privdata->elb_am_mode= VETAR_ELB_DATA;
  vetar_elb_delay();
}
#else
#define vetar_elb_switch_data(x) ;
#endif




#if defined(VETAR_MAP_ELB_REG) || defined(VETAR_MAP_ELB_CTRL)
/* switch elb mapping to wb data space*/
/* switch elb mapping to wb data space*/
void vetar_elb_switch_triva(struct vetar_privdata *privdata)
{
  if(privdata->elb_am_mode== VETAR_ELB_TRIVA) return;
  //ndelay(50);
  vetar_elb_set_window(TRIGMOD_VME_AM, TRIGMOD_REGS_ADDR);
  //JAM note: this address TRIGMOD_REGS_ADDR=0x2000000 has high address nibble 0 which is in conflict with ours=5
  // then mbs might see no vme addressing problems unless all read out modules stay below ad=0xF000000 with A32
  privdata->elb_am_mode= VETAR_ELB_TRIVA;
  vetar_elb_delay();
}
#else
#define vetar_elb_switch_triva(x) ;
#endif




/** wrap read access to wishbone control space.*/
static unsigned int vetar_read_control( struct vetar_privdata *privdata, unsigned long offset)
{

  unsigned int dat=0;
#ifdef VETAR_MAP_CONTROLSPACE
  vetar_elb_switch_control(privdata) ;
  dat=be32_to_cpu(ioread32be(privdata->ctrl_registers +offset));
  vetar_bus_delay();
  vetar_dump_error_regs();
#endif
  return dat;

}

/** wrap write access to wishbone control space.*/
static void vetar_write_control( struct vetar_privdata *privdata, unsigned int dat, unsigned long offset)
{
#ifdef VETAR_MAP_CONTROLSPACE
  vetar_elb_switch_control(privdata) ;
  iowrite32be(cpu_to_be32(dat), privdata->ctrl_registers + offset);
  vetar_bus_delay();
  vetar_dump_error_regs();
#endif
}



/** wrap read access to wishbone data space.*/
static unsigned int vetar_read_data( struct vetar_privdata *privdata, unsigned long offset)
{
  unsigned int dat=0;
#ifdef VETAR_MAP_REGISTERS
  vetar_elb_switch_data(privdata) ;
  dat=be32_to_cpu(ioread32be(privdata->registers +offset));
  vetar_dump_error_regs();
#endif
  return dat;

}

/** wrap write access to wishbone data space.*/
static void vetar_write_data( struct vetar_privdata *privdata, unsigned int dat, unsigned long offset)
{
#ifdef VETAR_MAP_REGISTERS
  vetar_elb_switch_data(privdata) ;
  iowrite32be(cpu_to_be32(dat), privdata->registers + offset);
  vetar_bus_delay();
  vetar_dump_error_regs();
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
       vetar_elb_switch_triva(privdata); // probably it is enough to switch back to mbs at end of wb cycle only JAM
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
   vetar_elb_switch_triva(privdata);
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
  vetar_elb_switch_triva(privdata);
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
  vetar_elb_switch_triva(privdata);
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
  vetar_elb_switch_triva(privdata);
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
  vetar_elb_switch_triva(privdata);
}


static void vetar_wb_byteenable(struct wishbone* wb, unsigned char be)
{
  struct vetar_privdata *privdata;
  vetar_dbg(KERN_ERR "*** Vetar_WB: vetar_wb_byteenable.. \n");
  privdata = container_of(wb, struct vetar_privdata, wb);
  vetar_write_control(privdata, be, EMUL_DAT_WD);
  vetar_elb_switch_triva(privdata);
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

 vetar_dbg(KERN_INFO "VME IRQ: Level: 0x%x, Vector: 0x%x \n", level, vec);

 if(priv->vme_itc)
   outl (1<<level, priv->vme_itc + PEV_ITC_IMASK_CLEAR); // mask source of interrupt


if(level != priv->level)
  vetar_dbg(KERN_INFO "VME IRQ: Warning: irq Level: 0x%x does not match vetar irq level 0x%x", level, priv->level);


 wishbone_slave_ready(&priv->wb);
 vetar_dbg(KERN_INFO "VME IRQ: wishbone_slave_ready returned.\n");
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

#ifdef VETAR_MAP_CONTROLSPACE
  if(privdata->ctrl_registers)
    {
      iounmap(privdata->ctrl_registers);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped registers 0x%x !\n",
                    (unsigned int) privdata->ctrl_registers);
    }

#ifndef VETAR_MAP_ELB_CTRL
  if(privdata->ctrl_regs_phys)
      {

     if(pev_map_free(pev, &(privdata->pev_vetar_ctrl_regs))==0)

        vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys control registers 0x%x with length 0x%lx !\n",
            (unsigned int) privdata->ctrl_regs_phys, (unsigned long) privdata->ctrl_reglen);
      }
#endif // map elb
#endif // map controlspace

#ifdef VETAR_MAP_REGISTERS
  if(privdata->registers)
    {
      iounmap(privdata->registers);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped registers 0x%x !\n",
                    (unsigned int) privdata->registers);
    }
#ifndef VETAR_MAP_ELB_REG
  if(privdata->regs_phys)
      {

     if(pev_map_free(pev, &(privdata->pev_vetar_regs))==0)

        vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys  registers 0x%x with length 0x%lx !\n",
            (unsigned int) privdata->regs_phys, (unsigned long) privdata->reglen);
      }
#endif // map elb
#endif // map registers


if(privdata->cr_csr)
  {
      iounmap(privdata->cr_csr);
      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev iounmapped configspace  0x%x !\n",
            (unsigned int) privdata->cr_csr);
  }
if(privdata->cr_csr_phys)
    {

   if(pev_map_free(pev, &(privdata->pev_vetar_cscr))==0)

      vetar_dbg(KERN_NOTICE "** vetar_cleanup_dev unmapped phys config registers 0x%x with length 0x%lx !\n",
          (unsigned int) privdata->cr_csr_phys, (unsigned long) privdata->configlen);
    }



////////////////////////////////////////////////////
// JAM TODO? How do we unmap the vme bus via pev?
// how to free irq here? not necessary?
////////////////////////////////////////////////////

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (privdata->class_dev)
    {
#ifdef VETAR_SYSFS_ENABLE
    if(privdata->sysfs_has_file){
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

#ifdef VETAR_MAP_ELB_REG
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tWB data window mapped with ELB\n");
#else
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tWB data window mapped with PEV1100/PCI\n");
#endif
#ifdef VETAR_MAP_ELB_CTRL
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tWB ctrl window mapped with ELB\n");
#else
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tWB ctrl window mapped with PEV1100/PCI\n");
#endif
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
    vetar_dbg(KERN_NOTICE "Reading Vendor id from address 0x%x ...\n", (unsigned) addr);
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
  vetar_dbg(KERN_NOTICE "vetar_csr_write value 0x%x to base 0x%x + offset 0x%x \n", value, (unsigned) base, offset);
}

u32 vetar_csr_read(void *base, u32 offset)
{
    u32 value=0;
    offset -= offset % 4;
    value=ioread32be(base + offset);
  vetar_crcsr_delay();
  vetar_dbg(KERN_NOTICE "vetar_csr_read value 0x%x from base 0x%x + offset 0x%x \n", value, (unsigned) base, offset);
    return value;
}



/** Set elb address window to current address modifier and base address window*/
void vetar_elb_set_window( uint am, uint32_t base)
{
  uint vme_am;
  int data;
     if(am == 0x0d)      vme_am = 0x38 | MAP_VME_ELB_A32 | MAP_VME_ELB_SP;
     else if( am == 0x39) vme_am = 0x38 | MAP_VME_ELB_A24;
     else if( am == 0x3d) vme_am = 0x38 | MAP_VME_ELB_A24 | MAP_VME_ELB_SP;
     else if( am == 0x29) vme_am = 0x38 | MAP_VME_ELB_A16;
     else if( am == 0x2d) vme_am = 0x38 | MAP_VME_ELB_A16 | MAP_VME_ELB_SP;
     else if( am == 0x00) vme_am = 0x38 | MAP_VME_ELB_IACK;
     else                    vme_am = 0x38 | MAP_VME_ELB_A32;
     if( pev->csr_ptr)
     {
       data = (vme_am << 24) | ((base&0xf0000000) >> 24);
       *(uint *)(pev->csr_ptr + PEV_CSR_VME_ELB) = data;
     }
     else
     {
       data = vme_am | (base&0xf0000000);
       if( pev->io_remap[0].short_io)
       {
         outl( data, pev->io_base +  PEV_SCSR_VME_ELB);
       }
       else
       {
         outl( data, pev->io_base +  PEV_CSR_VME_ELB);
       }
     }
     //vetar_dbg(KERN_NOTICE "vetar_elb_set window for am: 0x%x , base high=0x%x\n", am, base&0xf0000000);
}


/** map elb window, but do not assing any interrupts here:*/
char __iomem* vetar_elb_map( struct vme_board *v)
{
  char __iomem* mapped;
  uint elb_off;
  uint vme_size;
   vme_size =  (v->size + 0xfff) & 0x0ffff000; /* align to multiple of 4k */
   elb_off = v->base & 0x0ffff000;             /* align to multiple of 4k */
   mapped = ioremap( pev->elb_base + elb_off, vme_size); /* map vme base address through ELB bus */
   mapped +=  (v->base & 0x0fffffff) - elb_off;
   vetar_elb_set_window( v->am, v->base);
   vetar_msg(KERN_NOTICE "vetar_elb_map returns iomem base: 0x%lx\n",
        (unsigned long) mapped);

    return mapped;
}




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
//  vme_cpu_addr +=  (v->base & 0x0fffffff) - elb_off;
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
    am= VME_A32_USER_MBLT; // *0x08*/
    //am=0x50; // JAM compare with rio4 driver !?
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
    am = VME_A24_USER_MBLT; //0x38;
    //am= VME_A24_USER_DATA_SCT; /*0x39*/
    vetar_dbg(KERN_NOTICE "vetar_setup_csr_fa sets address modifier 0x%x\n",am);

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
#ifdef    VETAR_CONFIGURE_VME
  struct pev_ioctl_vme_conf pev_vme_conf;
#endif
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
  // first try to map and look up configuration space if any....
  privdata->configbase = privdata->slot * VETAR_CONFIGSIZE;
  privdata->configlen=VETAR_CONFIGSIZE;

////////////////////////////////////////////////////////////////////////////
// JAM  here is mapping for ipv:

    vetar_dbg(KERN_NOTICE "vme: attaching to PEV driver...");
    pev_drv_p = pev_register();
    vetar_dbg(KERN_NOTICE  "%p - %d\n", pev_drv_p, pev_drv_p->pev_local_crate);
    pev =  pev_drv_p->pev[pev_drv_p->pev_local_crate];

#ifdef    VETAR_CONFIGURE_PEV

    result= pev_inl( pev, pev->io_remap[pev->csr_remap].vme_base + 0x4);
    vetar_msg(KERN_NOTICE "vme: dump mascsr= 0x%x",result);
    mb();

    // change READ_SMP - vme read sampling point relative to DTACK#
    //result=0x80003081; // 0 ns
    //result=0x80000081; // 12 ns (default)
    //result=0x80001081;   // 18 ns
    result=0x80002081; // 24 ns
    pev_outl( pev, result, pev->io_remap[pev->csr_remap].vme_base + 0x4);
    mb();
    result= pev_inl( pev, pev->io_remap[pev->csr_remap].vme_base + 0x4);
    vetar_msg(KERN_NOTICE "vme: changed mascsr to 0x%x",result);
    mb();

    vetar_dump_error_regs();


#endif

// JAM here we investigate what to do with the vme configuration of PEV1100:
#ifdef    VETAR_CONFIGURE_VME

    vetar_msg(KERN_NOTICE "vme: reading PEV vme config...");
    pev_vme_conf_read(pev, &pev_vme_conf);
    vetar_bus_delay();
#ifdef    VETAR_DUMP_REGISTERS
    vetar_msg(KERN_NOTICE "a24_base=0x%x, a24 size=0x%x \n", pev_vme_conf.a24_base, pev_vme_conf.a24_size);
    vetar_msg(KERN_NOTICE "a32_base=0x%x, a32 size=0x%x \n", pev_vme_conf.a32_base, pev_vme_conf.a32_size);
    vetar_msg(KERN_NOTICE "x64=0x%x, slot1=0x%x, sysrst=0x%x, rto=0x%x \n",
        pev_vme_conf.x64, pev_vme_conf.slot1,  pev_vme_conf.sysrst, pev_vme_conf.rto);
    vetar_msg(KERN_NOTICE "arb=0x%x, bto=0x%x, req=0x%x, level=0x%x \n",
           pev_vme_conf.arb, pev_vme_conf.bto,  pev_vme_conf.req, pev_vme_conf.level);
    vetar_msg(KERN_NOTICE "mas_ena=0x%x, slv_ena=0x%x, slv_retry=0x%x, burst=0x%x \n",
               pev_vme_conf.mas_ena, pev_vme_conf.slv_ena,  pev_vme_conf.slv_retry, pev_vme_conf.burst);

#endif

    // TODO: play with vme parameters an set it back:
    //pev_vme_conf.slv_retry=0x0;

    pev_vme_conf.arb=0; // 0=PRI not pipelined; 1=RRS; 2= PRI pipelined; 3=RRS pipelined
    pev_vme_conf.req=1; // 0=release when done; 1= release on request;
    pev_vme_conf.level=0; // defaults to 0

    //pev_vme_conf.bto=20; // bus time out in us (defaults 16us) NO EFFECT!

    //pev_vme_conf.slv_ena |=VME_SLV_1MB; //0x8; try change mapping granularity?
    //pev_vme_conf.slv_ena=VME_SLV_ENA; // 0x1;
    pev_vme_conf.slv_ena=0; // do not export pev mem to vme

    //pev_vme_conf.mas_ena |=0x01; // need to switch on master mode explicitely?

    vetar_msg(KERN_NOTICE "vme: writing PEV vme config...");
    pev_vme_conf_write(pev, &pev_vme_conf);

    vetar_bus_delay();

#ifdef    VETAR_DUMP_REGISTERS
    pev_vme_conf_read(pev, &pev_vme_conf);
    vetar_msg(KERN_NOTICE "vme: CHECK PEV vme config...");
    vetar_msg(KERN_NOTICE "a24_base=0x%x, a24 size=0x%x \n", pev_vme_conf.a24_base, pev_vme_conf.a24_size);
    vetar_msg(KERN_NOTICE "a32_base=0x%x, a32 size=0x%x \n", pev_vme_conf.a32_base, pev_vme_conf.a32_size);
    vetar_msg(KERN_NOTICE "x64=0x%x, slot1=0x%x, sysrst=0x%x, rto=0x%x \n",
        pev_vme_conf.x64, pev_vme_conf.slot1,  pev_vme_conf.sysrst, pev_vme_conf.rto);
    vetar_msg(KERN_NOTICE "arb=0x%x, bto=0x%x, req=0x%x, level=0x%x \n",
           pev_vme_conf.arb, pev_vme_conf.bto,  pev_vme_conf.req, pev_vme_conf.level);
    vetar_msg(KERN_NOTICE "mas_ena=0x%x, slv_ena=0x%x, slv_retry=0x%x, burst=0x%x \n",
               pev_vme_conf.mas_ena, pev_vme_conf.slv_ena,  pev_vme_conf.slv_retry, pev_vme_conf.burst);

#endif

#endif // configure vme


    // JAM cscr space can not be mapped with elb, use always pev_map_alloc:
    privdata->pev_vetar_cscr.rem_addr=privdata->configbase;
    privdata->pev_vetar_cscr.size=privdata->configlen;
    privdata->pev_vetar_cscr.sg_id=MAP_MASTER_32; //MAP_VME_SLAVE;
    privdata->pev_vetar_cscr.mode= MAP_SPACE_VME | MAP_ENABLE | MAP_ENABLE_WR | MAP_SWAP_NO | MAP_VME_CR;


    vetar_dbg(KERN_NOTICE "** vetar_probe_pev_map_alloc for  rem_addr=0x%lx, size=0x%x, sg_id=0x%x, mode=0x%x\n",
                     privdata->pev_vetar_cscr.rem_addr,   privdata->pev_vetar_cscr.size,
                     privdata->pev_vetar_cscr.sg_id, privdata->pev_vetar_cscr.mode);


    pev_map_alloc(pev, &(privdata->pev_vetar_cscr));

    vetar_dbg(KERN_NOTICE "** vetar_probe_pev_map_alloc returns  pci_base=0x%lx, loc_addr=0x%lx, offset=0x%x, win_size=0x%x, rem_base=0x%lx, loc_base=0x%lx , user addr=0x%x\n "
        "pev->mem_base=0x%x",
                     privdata->pev_vetar_cscr.pci_base, privdata->pev_vetar_cscr.loc_addr,   privdata->pev_vetar_cscr.offset,
                     privdata->pev_vetar_cscr.win_size, privdata->pev_vetar_cscr.rem_base, privdata->pev_vetar_cscr.loc_base,
                     (unsigned) privdata->pev_vetar_cscr.usr_addr, pev->mem_base);


    privdata->cr_csr_phys= privdata->pev_vetar_cscr.pci_base + privdata->pev_vetar_cscr.loc_addr; // local address is relative to mapping window
    if (privdata->cr_csr_phys == 0xffffffffffffffffULL) {
          vetar_msg(KERN_ERR "** vetar_probe_vme could not pev_map_alloc configbase 0x%x with length 0x%lx !\n",
                   privdata->configbase, privdata->configlen);
          privdata->cr_csr_phys=0; // reset to avoid map_free in cleanup
            vetar_cleanup_dev(privdata, index);
            return -ENOMEM;
        }

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

  //  may check for vendor id etc...
if(!vetar_is_present(privdata))
  {
    vetar_cleanup_dev(privdata, index);
    return -EFAULT;
  }



  // setup interrupts:

//#ifdef VETAR_ENABLE_IRQ
//
//// JAM this is also done in the vme_board_register function. might do it for one virtual board region only!
////snprintf(privdata->irqname, 64, VETARNAMEFMT,privdata->lun);
////  result = xpc_vme_request_irq(INTVECTOR, (1 << INT_LEVEL) , vetar_irqhandler, privdata, privdata->irqname);
////    if (result)
////    {
////    }
////  vetar_msg(KERN_ERR "** vetar_probe_vme with irq handler, result=%d \n",result);
//
//#endif




vetar_setup_csr_fa(privdata);

#ifdef VETAR_MAP_REGISTERS


#ifdef VETAR_MAP_ELB_REG

privdata->vme_board_registers.irq = -1;
privdata->vme_board_registers.base = privdata->vmebase;
privdata->vme_board_registers.size = privdata->reglen;
privdata->vme_board_registers.am   = VETAR_ELB_DATA; //VME_A32_USER_DATA_SCT;
//privdata->vme_board_registers.irq  = privdata->level; // multiple assigned irq?
//privdata->vme_board_registers.vec  = privdata->vector;
// TODO: own method to get some VME irq

//vme_board_register( &(privdata->vme_board_registers), vetar_irqhandler, NULL);
//privdata->registers=vme_cpu_addr;

privdata->registers=vetar_elb_map(&(privdata->vme_board_registers));
if (!privdata->registers) {
     vetar_msg(KERN_ERR "** vetar_probe_vme could not ELB map vme base 0x%x with length 0x%lx !\n",
         (unsigned int) privdata->vmebase, privdata->reglen);
       vetar_cleanup_dev(privdata, index);
       return -ENOMEM;
    }
privdata->elb_am_mode=VETAR_ELB_DATA; // initialize address window mode

#else

// map via pci bus,  like the configuration space
  privdata->pev_vetar_regs.rem_addr=privdata->vmebase;
  privdata->pev_vetar_regs.size=privdata->reglen;
  privdata->pev_vetar_regs.sg_id=MAP_MASTER_32;
  privdata->pev_vetar_regs.mode= MAP_SPACE_VME | MAP_ENABLE | MAP_ENABLE_WR | MAP_SWAP_NO | MAP_VME_A32;


  vetar_dbg(KERN_NOTICE "** vetar_probe_pev_map_alloc for  rem_addr=0x%lx, size=0x%x, sg_id=0x%x, mode=0x%x\n",
                   privdata->pev_vetar_regs.rem_addr,   privdata->pev_vetar_regs.size,
                   privdata->pev_vetar_regs.sg_id, privdata->pev_vetar_regs.mode);


  pev_map_alloc(pev, &(privdata->pev_vetar_regs));

  vetar_dbg(KERN_NOTICE "** vetar_probe_pev_map_alloc returns  pci_base=0x%lx, loc_addr=0x%lx, offset=0x%x, win_size=0x%x, rem_base=0x%lx, loc_base=0x%lx , user addr=0x%x\n "
      "pev->mem_base=0x%x",
                   privdata->pev_vetar_regs.pci_base, privdata->pev_vetar_regs.loc_addr,   privdata->pev_vetar_regs.offset,
                   privdata->pev_vetar_regs.win_size, privdata->pev_vetar_regs.rem_base, privdata->pev_vetar_regs.loc_base,
                   (unsigned) privdata->pev_vetar_regs.usr_addr, pev->mem_base);


  privdata->regs_phys= privdata->pev_vetar_regs.pci_base + privdata->pev_vetar_regs.loc_addr; // local address is relative to mapping window
  if (privdata->regs_phys == 0xffffffffffffffffULL) {
        vetar_msg(KERN_ERR "** vetar_probe_vme could not pev_map_alloc vmebase 0x%x with length 0x%lx !\n",
                 privdata->vmebase, privdata->reglen);
        privdata->regs_phys=0; // reset to avoid map_free in cleanup
          vetar_cleanup_dev(privdata, index);
          return -ENOMEM;
      }

  mb();


  // we have to ioremap this address to use it in kernel module?
  privdata->registers = ioremap_nocache(privdata->regs_phys, privdata->reglen);
  if (!privdata->registers) {
        vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at registers physical address 0x%x with length 0x%lx !\n",
            (unsigned int) privdata->regs_phys, privdata->reglen);
          vetar_cleanup_dev(privdata, index);
          return -ENOMEM;
       }


#endif



    mb();
    vetar_dbg(KERN_NOTICE "** vetar_probe_vme remapped vme base 0x%x to kernel address 0x%lx\n",
          (unsigned int) privdata->vmebase,  (unsigned long) privdata->registers);

#endif

#ifdef VETAR_MAP_CONTROLSPACE

// JAM: third time for control space:
    privdata->ctrl_reglen=VETAR_CTRLREGS_SIZE;

#ifdef VETAR_MAP_ELB_CTRL
    privdata->vme_board_ctrl.irq = -1;
    privdata->vme_board_ctrl.base = privdata->ctrl_vmebase;
    privdata->vme_board_ctrl.size = privdata->ctrl_reglen;
    privdata->vme_board_ctrl.am   = VETAR_ELB_CONTROL; //VME_A24_USER_DATA_SCT;
    //privdata->vme_board_ctrl.irq  = privdata->level; // multiple assigned irq?
    //privdata->vme_board_ctrl.vec  = privdata->vector;

    //vme_board_register( &(privdata->vme_board_ctrl), vetar_irqhandler, NULL);
    //privdata->ctrl_registers=vme_cpu_addr;

    privdata->ctrl_registers=vetar_elb_map( &(privdata->vme_board_ctrl));
    if (!privdata->ctrl_registers) {
         vetar_msg(KERN_ERR "** vetar_probe_vme could not ELB map control vme base 0x%x with length 0x%lx !\n",
             (unsigned int) privdata->ctrl_vmebase, privdata->ctrl_reglen);
           vetar_cleanup_dev(privdata, index);
           return -ENOMEM;
        }
    privdata->elb_am_mode=VETAR_ELB_CONTROL;
#else

    privdata->pev_vetar_ctrl_regs.rem_addr=privdata->ctrl_vmebase;
    privdata->pev_vetar_ctrl_regs.size=privdata->ctrl_reglen;
    privdata->pev_vetar_ctrl_regs.sg_id=MAP_MASTER_32;
    privdata->pev_vetar_ctrl_regs.mode= MAP_SPACE_VME | MAP_ENABLE | MAP_ENABLE_WR | MAP_SWAP_NO | MAP_VME_A24;


    vetar_dbg(KERN_NOTICE "** vetar_probe_pev_map_alloc for  rem_addr=0x%lx, size=0x%x, sg_id=0x%x, mode=0x%x\n",
                     privdata->pev_vetar_ctrl_regs.rem_addr,   privdata->pev_vetar_ctrl_regs.size,
                     privdata->pev_vetar_ctrl_regs.sg_id, privdata->pev_vetar_ctrl_regs.mode);


    pev_map_alloc(pev, &(privdata->pev_vetar_ctrl_regs));

    vetar_dbg(KERN_NOTICE "** vetar_probe_pev_map_alloc returns  pci_base=0x%lx, loc_addr=0x%lx, offset=0x%x, win_size=0x%x, rem_base=0x%lx, loc_base=0x%lx , user addr=0x%x\n "
        "pev->mem_base=0x%x",
                     privdata->pev_vetar_ctrl_regs.pci_base, privdata->pev_vetar_ctrl_regs.loc_addr,   privdata->pev_vetar_ctrl_regs.offset,
                     privdata->pev_vetar_ctrl_regs.win_size, privdata->pev_vetar_ctrl_regs.rem_base, privdata->pev_vetar_ctrl_regs.loc_base,
                     (unsigned) privdata->pev_vetar_ctrl_regs.usr_addr, pev->mem_base);


    privdata->ctrl_regs_phys= privdata->pev_vetar_ctrl_regs.pci_base + privdata->pev_vetar_ctrl_regs.loc_addr; // local address is relative to mapping window
    if (privdata->ctrl_regs_phys == 0xffffffffffffffffULL) {
          vetar_msg(KERN_ERR "** vetar_probe_vme could not pev_map_alloc control vmebase 0x%x with length 0x%lx !\n",
                   privdata->ctrl_vmebase, privdata->ctrl_reglen);
          privdata->ctrl_regs_phys=0; // reset to avoid map_free in cleanup
            vetar_cleanup_dev(privdata, index);
            return -ENOMEM;
        }

    mb();


    // we have to ioremap this address to use it in kernel module
    privdata->ctrl_registers = ioremap_nocache(privdata->ctrl_regs_phys, privdata->ctrl_reglen);
    if (!privdata->ctrl_registers) {
          vetar_msg(KERN_ERR "** vetar_probe_vme could not ioremap_nocache at control registers physical address 0x%x with length 0x%lx !\n",
              (unsigned int) privdata->ctrl_regs_phys, privdata->ctrl_reglen);
            vetar_cleanup_dev(privdata, index);
            return -ENOMEM;
         }





#endif


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
	privdata->vme_itc=vme_itc_reg;
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
