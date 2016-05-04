#include "pexornet.h"

#include <net/udp.h>

static atomic_t pexornet_numdevs = ATOMIC_INIT(0);

static struct pci_device_id ids[] = { { PCI_DEVICE (PEXOR_VENDOR_ID, PEXOR_DEVICE_ID), },    // classic pexornet
    { PCI_DEVICE (PEXARIA_VENDOR_ID, PEXARIA_DEVICE_ID), },    //pexaria
    { PCI_DEVICE (KINPEX_VENDOR_ID, KINPEX_DEVICE_ID), },    // kinpex
    { 0, } };
MODULE_DEVICE_TABLE(pci, ids);

#ifdef PEXORNET_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static DEVICE_ATTR(freebufs, S_IRUGO, pexornet_sysfs_freebuffers_show, NULL);
static DEVICE_ATTR(usedbufs, S_IRUGO, pexornet_sysfs_usedbuffers_show, NULL);
static DEVICE_ATTR(rcvbufs, S_IRUGO, pexornet_sysfs_rcvbuffers_show, NULL);
static DEVICE_ATTR(codeversion, S_IRUGO, pexornet_sysfs_codeversion_show, NULL);
static DEVICE_ATTR(dmaregs, S_IRUGO, pexornet_sysfs_dmaregs_show, NULL);
static DEVICE_ATTR(trixorregs, S_IRUGO, pexornet_sysfs_trixorregs_show, NULL);
static DEVICE_ATTR(trixorfcti, S_IWUGO | S_IRUGO , pexornet_sysfs_trixor_fctime_show, pexornet_sysfs_trixor_fctime_store);
static DEVICE_ATTR(trixorcvti, S_IWUGO | S_IRUGO , pexornet_sysfs_trixor_cvtime_show, pexornet_sysfs_trixor_cvtime_store);

#ifdef PEXORNET_WITH_SFP
static DEVICE_ATTR(sfpregs, S_IRUGO, pexornet_sysfs_sfpregs_show, NULL);
static DEVICE_ATTR(gosipretries, S_IWUGO | S_IRUGO , pexornet_sysfs_sfp_retries_show, pexornet_sysfs_sfp_retries_store);
static DEVICE_ATTR(gosipbuswait, S_IWUGO | S_IRUGO , pexornet_sysfs_buswait_show, pexornet_sysfs_buswait_store);


#endif

#endif
#endif /* SYSFS_ENABLE*/



/* timeout value for network (preliminary) */
static int timeout = PEXORNET_WAIT_TIMEOUT;




int pexornet_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{

  int retval = 0;
  struct pexornet_privdata *privdata;
  unsigned long arg;
  /* here validity check for magic number etc.*/
  privdata= pexornet_get_privdata(dev);
  if(!privdata) return -EFAULT;

  /* use semaphore to allow multi user mode:*/
  if (down_interruptible(&(privdata->ioctl_sem)))
  {
    pexornet_msg((KERN_INFO "down interruptible of ioctl sem is not zero, restartsys!\n"));
    return -ERESTARTSYS;
  }


   arg= (unsigned long) rq->ifr_data; // pass pointer to ifreq to special ioctls
  /* Select the appropiate command */
  switch (cmd)
  {

    /* first all common ioctls:*/

    // TODO: change arg to struct ifreq *

    case PEXORNET_IOC_RESET:
      pexornet_msg(KERN_NOTICE "** pexornet_ioctl reset\n");
      retval = pexornet_ioctl_reset(privdata,arg);
      break;



    case PEXORNET_IOC_WRITE_BUS:
      pexornet_dbg(KERN_NOTICE "** pexornet_ioctl write bus\n");
      retval = pexornet_ioctl_write_bus(privdata, arg);
      break;

    case PEXORNET_IOC_READ_BUS:
      pexornet_dbg(KERN_NOTICE "** pexornet_ioctl read bus\n");
      retval = pexornet_ioctl_read_bus(privdata, arg);
      break;

    case PEXORNET_IOC_INIT_BUS:
      pexornet_msg(KERN_NOTICE "** pexornet_ioctl init bus\n");
      retval = pexornet_ioctl_init_bus(privdata, arg);
      break;

    case PEXORNET_IOC_CONFIG_BUS:
      pexornet_dbg(KERN_NOTICE "** pexornet_ioctl config bus\n");
      retval = pexornet_ioctl_configure_bus(privdata, arg);
      break;



#ifdef PEXORNET_WITH_TRIXOR
    case PEXORNET_IOC_SET_TRIXOR:
      pexornet_dbg(KERN_NOTICE "** pexornet_ioctl set trixor\n");
      retval = pexornet_ioctl_set_trixor(privdata, arg);
      break;

#endif

    case PEXORNET_IOC_GET_SFP_LINKS:
      pexornet_dbg(KERN_NOTICE "** pexornet_ioctl get sfp links\n");
      retval = pexornet_ioctl_get_sfp_links(privdata, arg);
      break;

//    case PEXORNET_IOC_SET_WAIT_TIMEOUT:
//      pexornet_dbg(KERN_NOTICE "** pexornet_ioctl set wait timeout\n");
//      retval = pexornet_ioctl_set_wait_timeout(privdata, arg);
//      break;

    default:
      retval = -ENOTTY;
      break;
  }

  up(&privdata->ioctl_sem);
  return retval;
}




int pexornet_ioctl_reset (struct pexornet_privdata* priv, unsigned long arg)
{
  pexornet_dbg(KERN_NOTICE "** pexornet_ioctl_reset...\n");

  pexornet_dbg(KERN_NOTICE "Clearing DMA status... \n");
  iowrite32 (0, priv->registers.dma_control_stat);
  mb();
  ndelay(20);
  udelay(10000);

#ifdef PEXORNET_WITH_SFP
  pexornet_sfp_reset (priv);
  pexornet_sfp_clear_all (priv);
#endif
  pexornet_cleanup_buffers (priv);
  pexornet_build_buffers(priv, priv->net_dev->mtu, PEXORNET_DEFAULTBUFFERNUM);
  atomic_set(&(priv->irq_count), 0);


  atomic_set(&(priv->state), PEXORNET_STATE_STOPPED);
#ifdef PEXORNET_WITH_TRIXOR
  pexornet_dbg(KERN_NOTICE "Initalizing TRIXOR... \n");

  iowrite32 (TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR, priv->registers.irq_status); /*reset interrupt source*/
  mb();
  ndelay(20);

  // atomic_set(&(priv->trig_outstanding), 0);
  // clear interrupt type queue:

  iowrite32 (TRIX_BUS_DISABLE, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_HALT, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_MASTER, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_CLEAR, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (0x10000 - 0x20, priv->registers.trix_fcti);
  mb();
  ndelay(20);
  iowrite32 (0x10000 - 0x40, priv->registers.trix_cvti);
  mb();
  ndelay(20);

  iowrite32 (TRIX_DT_CLEAR, priv->registers.irq_status);
  mb();
  ndelay(20);

  iowrite32 (TRIX_BUS_ENABLE, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_HALT, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_MASTER, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_CLEAR, priv->registers.irq_control);
  mb();
  ndelay(20);

  pexornet_dbg(KERN_NOTICE " ... TRIXOR done.\n");
#else

  iowrite32(0, priv->registers.irq_control);
  mb();
  iowrite32(0, priv->registers.irq_status);
  mb();
#endif
  print_pexornet (&(priv->registers));
  return 0;
}




//int pexornet_ioctl_clearreceivebuffers (struct pexornet_privdata* priv, unsigned long arg)
//{
//  int i = 0, outstandingbuffers = 0;
//  unsigned long wjifs = 0;
//  struct pexornet_dmabuf* cursor;
//  struct pexornet_dmabuf* next;
//#ifdef PEXORNET_WITH_TRIXOR
//  struct pexornet_trigger_buf* trigstat;
//#endif
//  pexornet_dbg(KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers...\n");
//  spin_lock_bh( &(priv->buffers_lock));
//  list_for_each_entry_safe(cursor, next, &(priv->received_buffers), queue_list)
//  {
//    pexornet_dbg(KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers moved %lx to free list..\n", (long) cursor);
//    list_move_tail(&(cursor->queue_list) , &(priv->free_buffers));
//  }
//  spin_unlock( &(priv->buffers_lock));
//  /* empty possible wait queue events and dec the outstanding counter*/
//  outstandingbuffers = atomic_read( &(priv->dma_outstanding));
//  for (i = 0; i < outstandingbuffers; ++i)
//  {
//    wjifs = wait_event_interruptible_timeout (priv->irq_dma_queue, atomic_read( &(priv->dma_outstanding) ) > 0,
//        priv->wait_timeout * HZ);
//    pexornet_dbg(
//        KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers after wait_event_interruptible_timeout with TIMEOUT %ds (=%d jiffies), waitjiffies=%ld, outstanding=%d \n", priv->wait_timeout, priv->wait_timeout*HZ, wjifs, atomic_read( &(priv->dma_outstanding)));
//
//    if (wjifs == 0)
//    {
//      pexornet_msg(KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n",priv->wait_timeout*HZ);
//      return -EFAULT;
//    }
//    else if (wjifs == -ERESTARTSYS)
//    {
//      pexornet_msg(KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers after wait_event_interruptible_timeout woken by signal. abort wait\n");
//      return -EFAULT;
//    }
//    atomic_dec (&(priv->dma_outstanding));
//  }    // for outstandingbuffers
//
//#ifdef PEXORNET_WITH_TRIXOR
//  /* empty possible wait queue events for interrupts and dec the outstanding counter*/
//  outstandingbuffers = atomic_read( &(priv->trig_outstanding));
//  for (i = 0; i < outstandingbuffers; ++i)
//  {
//    wjifs = wait_event_interruptible_timeout (priv->irq_trig_queue, atomic_read( &(priv->trig_outstanding) ) > 0,
//        priv->wait_timeout * HZ);
//    pexornet_dbg(
//        KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers after wait_event_interruptible_timeout for trigger queue, with TIMEOUT %ds (=%d jiffies), waitjiffies=%ld, outstanding=%d \n", priv->wait_timeout, priv->wait_timeout*HZ, wjifs, atomic_read( &(priv->trig_outstanding)));
//
//    if (wjifs == 0)
//    {
//      pexornet_dbg(
//          KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers TIMEOUT %d jiffies expired on wait_event_interruptible_timeout for trigger queue... \n", priv->wait_timeout*HZ);
//      return -EFAULT;
//    }
//    else if (wjifs == -ERESTARTSYS)
//    {
//      pexornet_msg(KERN_NOTICE "** pexornet_ioctl_clearreceivebuffers after wait_event_interruptible_timeout for trigger queue woken by signal. abort wait\n");
//      return -EFAULT;
//    }
//    atomic_dec (&(priv->trig_outstanding));
//    spin_lock_bh( &(priv->trigstat_lock));
//    if (list_empty (&(priv->trig_status)))
//    {
//      spin_unlock( &(priv->trigstat_lock));
//      pexornet_msg(KERN_ERR "pexornet_ioctl_clearreceivebuffers never come here - list of trigger type buffers is empty! \n");
//      return -EFAULT;
//    }
//    trigstat=list_first_entry(&(priv->trig_status), struct pexornet_trigger_buf, queue_list);
//    trigstat->trixorstat = 0;    // mark status object as free
//    list_move_tail (&(trigstat->queue_list), &(priv->trig_status));    // move to end of list
//    spin_unlock( &(priv->trigstat_lock));
//
//  }    // for outstandingbuffers
//
//#endif
//  return 0;
//}



#ifdef PEXORNET_WITH_TRIXOR
int pexornet_ioctl_set_trixor (struct pexornet_privdata* priv, unsigned long arg)
{
  int command=0, retval=0;
  struct pexornet_trixor_set descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexornet_trixor_set));
  if (retval)
    return retval;
  command = descriptor.command;
  switch (command)
  {
    case PEXORNET_TRIX_RES:
      retval=pexornet_trigger_reset(priv);
      pexornet_dbg(KERN_ERR "pexornet_ioctl_set_trixor did reset trigger!");

      break;

    case PEXORNET_TRIX_GO:

      retval=pexornet_trigger_start_acq(priv);
      pexornet_msg(KERN_ERR "pexornet_ioctl_set_trixor did really start acquisition!");
      break;

    case PEXORNET_TRIX_HALT:

      retval=pexornet_trigger_stop_acq(priv);
      pexornet_msg(KERN_ERR "pexornet_ioctl_set_trixor did really stop acquisition!");
       break;

    case PEXORNET_TRIX_TIMESET:

//      if ((ps_args->l_slave == 0) && (ps_args->l_multi == 0))
//       {
// //        printf ("\033[7m  master trigger module, single mode! \033[0m \n");
//         *ps_trig->pl_ctrl_wr = 0x1000;    // disable trigger bus
//         *ps_trig->pl_ctrl_wr = HALT;
//         *ps_trig->pl_ctrl_wr = MASTER;
//         *ps_trig->pl_ctrl_wr = CLEAR;
//
//         if ((ps_setup->bh_meb_trig_mode != 0) && (ps_setup->bh_crate_nr == 0))
//         {
//           *ps_trig->pl_fcatr_wr = 0x10000 - ps_setup->bl_trig_fct[0];
//           *ps_trig->pl_ctimr_wr = 0x10000 - ps_setup->bl_trig_cvt[0];
//           sprintf (c_line, "set up SPECIAL trigger module as MASTER");
//           F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
//         }
//         else
//         {
//           *ps_trig->pl_fcatr_wr = 0x10000 - ps_setup->bl_trig_fct[ps_setup->bh_crate_nr];
//           *ps_trig->pl_ctimr_wr = 0x10000 - ps_setup->bl_trig_cvt[ps_setup->bh_crate_nr];
//
//           sprintf (c_line, "trigger module set up as MASTER, crate nr: %d", ps_setup->bh_crate_nr);
//           F_ERROR(ERR__MSG_INFO, 0, c_line, MASK__PRTT);
//         }
//         ps_daqst->bh_trig_master = 1; /* master trigger module */
//      //////////////////////////////////////////////////////////////////////

      // for the moment, initialize trigger module as single branch master here, too
      // later we may add special set trixor commands for this-
//      iowrite32(TRIX_BUS_DISABLE , priv->registers.irq_control);// disable trigger bus?
//      mb();
//      ndelay(20);
//      iowrite32(TRIX_HALT , priv->registers.irq_control);
//      mb();
//      ndelay(20);
//      iowrite32(TRIX_MASTER , priv->registers.irq_control);
//      mb();
//      ndelay(20);
//      iowrite32(TRIX_CLEAR , priv->registers.irq_control);
//      mb();
//      ndelay(20);

      iowrite32 (0x10000 - descriptor.fct, priv->registers.trix_fcti);
      mb();
      ndelay(20);
      iowrite32 (0x10000 - descriptor.cvt, priv->registers.trix_cvti);
      mb();
      ndelay(20);

//      iowrite32(TRIX_BUS_ENABLE , priv->registers.irq_control);// enable trigger bus only for multi mode?
//      mb();
//      ndelay(20);

      pexornet_dbg(KERN_ERR "pexornet_ioctl_set_trixor configured trixor as master with fct=0x%x cvt=0x%x!",descriptor.fct, descriptor.cvt);
      break;

    default:
      pexornet_dbg(KERN_ERR "pexornet_ioctl_set_trixor unknown command %x\n", command);
      return -EFAULT;

  };

  return retval;
}



int pexornet_trigger_reset(struct pexornet_privdata* priv)
{
  // taken from corresonding section of mbs m_read_meb:
  //      iowrite32(TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR , priv->registers.irq_status);
  //      mb();
  //      ndelay(20);
  // ? ? do not clear interrupts here, already done in irq handler

  //      iowrite32(TRIX_BUS_DISABLE, priv->registers.irq_control);
  //      mb();
  //      ndelay(20);

  iowrite32 (TRIX_FC_PULSE, priv->registers.irq_status);
  mb();
  ndelay(20);
  iowrite32 (0x100, priv->registers.irq_control);
  mb();
  ndelay(20);
  iowrite32 (TRIX_DT_CLEAR, priv->registers.irq_status);
  mb();
  ndelay(20);
  pexornet_dbg(KERN_NOTICE "pexornet_trigger_reset done.\n");
  return 0;
}

int pexornet_trigger_start_acq(struct pexornet_privdata* priv)
{
// raise interrupt with trigger type 14 for start acquisition
  iowrite32 (TRIX_HALT, priv->registers.irq_control);
  mb();
  udelay(1000);
  // do we need this not to hang up trixor?
  iowrite32 (TRIX_CLEAR, priv->registers.irq_control);    // this will also clear local event counter
  mb();
  ndelay(20);
  atomic_set(&(priv->state), PEXORNET_STATE_TRIGGERED_READ); // enable readout mode for interrupt service routine
  mb();
  ndelay(20);
  iowrite32 (PEXORNET_TRIGTYPE_START, priv->registers.irq_status);    // put start acquisition trigger 14
  mb();
  ndelay(20);
  iowrite32 ((TRIX_EN_IRQ | TRIX_GO), priv->registers.irq_control);
  mb();
  ndelay(20);
  //pexornet_dbg(KERN_ERR "pexornet_trigger_start_acq done .!\n");
return 0;
}



int pexornet_trigger_stop_acq (struct pexornet_privdata* priv)
{
  int retval = 0;
  iowrite32 (TRIX_HALT, priv->registers.irq_control);
  mb();
  ndelay(20);
  // TODO: check for last trigger event finished like in mbs? only for multi branches...
  udelay(10000);
  // do we need this not to hang up trixor?

  iowrite32 (TRIX_CLEAR, priv->registers.irq_control);
  mb();
  ndelay(20);

  iowrite32 (PEXORNET_TRIGTYPE_STOP, priv->registers.irq_status);    // put special stop trigger
  mb();
  ndelay(20);
  iowrite32 ((TRIX_EN_IRQ | TRIX_GO), priv->registers.irq_control);
  mb();
  ndelay(20);
  pexornet_dbg(KERN_NOTICE "pexornet_trigger_stop_acq done.\n");

  // note: the actual halt on trixor register is performed in pexornet_trigger_do_stop
  // on handling the special trigger 15

  return retval;
}


int pexornet_trigger_do_stop (struct pexornet_privdata* priv)
{
  int retval = 0;
  iowrite32 (TRIX_HALT, priv->registers.irq_control);
  mb();
  udelay(10000);
  // do we need this not to hang up trixor?
  iowrite32 (TRIX_CLEAR, priv->registers.irq_control);
  mb();
  ndelay(20);
  atomic_set(&(priv->state), PEXORNET_STATE_STOPPED);
  // reset auto trigger readout mode

  return retval;
}

void pexornet_decode_triggerstatus(u32 trixorstat, struct pexornet_trigger_status* result)
{
  /* decode into descriptor for consumer convenience:*/
    result->typ = (trixorstat & SEC__MASK_TRG_TYP) >> 16;
    result->si = (trixorstat & SEC__MASK_SI) >> 23;
    result->mis = (trixorstat & SEC__MASK_MIS) >> 22;
    result->lec = (trixorstat & SEC__MASK_LEC) >> 24;
    result->di = (trixorstat & SEC__MASK_DI) >> 29;
    result->tdt = (trixorstat & SEC__MASK_TDT) >> 21;
    result->eon = (trixorstat & SEC__MASK_EON) >> 31;
}


#endif






int pexornet_freebuffer (struct pexornet_privdata* priv, struct pexornet_dmabuf* tofree)
{
  struct pexornet_dmabuf* cursor;
  pexornet_dbg(KERN_NOTICE "** pexornet_freebuffer before buffer lock \n");
  spin_lock_bh( &(priv->buffers_lock));
  if (list_empty (&(priv->used_buffers)))
  {
    /* this may happen if user calls free buffer without taking or receiving one before*/
    pexornet_dbg(KERN_NOTICE "** pexornet_free_buffer: No more used buffers to free!\n");
    goto frbufail;
  }
  list_for_each_entry(cursor, &(priv->used_buffers), queue_list)
  {
    if(cursor->kernel_addr == tofree->kernel_addr) // TODO better test for dma address or other id, this fails for sg buffers
    {
      pexornet_dbg(KERN_NOTICE "** pexornet_freebuffer freed buffer %p\n",(void*) cursor->kernel_addr);
      list_move_tail(&(cursor->queue_list) , &(priv->free_buffers));
      spin_unlock_bh( &(priv->buffers_lock) );
      cursor->used_size=0;
      /* ? need to sync buffer for next dma */
      if(cursor->dma_addr!=0) /* kernel buffer*/
        pci_dma_sync_single_for_device( priv->pdev, cursor->dma_addr, cursor->size, PCI_DMA_FROMDEVICE );
      else /* sg buffer*/
        pci_dma_sync_sg_for_device( priv->pdev, cursor->sg, cursor->sg_ents,PCI_DMA_FROMDEVICE );
      return 0;
    }

  }
  pexornet_msg(KERN_NOTICE "** pexornet_free_buffer: buffer to free %p was not found in list!\n", (void*) tofree->kernel_addr);

frbufail:
  spin_unlock_bh( &(priv->buffers_lock));
  return -EFAULT;
}



struct pexornet_dmabuf* new_dmabuffer (struct pci_dev * pdev, size_t size, unsigned long pgoff)
{
  struct pexornet_dmabuf* descriptor;
  descriptor = kmalloc (sizeof(struct pexornet_dmabuf), GFP_ATOMIC);
  if (!descriptor)
  {
    pexornet_dbg(KERN_ERR "new_dmabuffer: could not alloc dma buffer descriptor! \n");
    return NULL ;
  }
  memset (descriptor, 0, sizeof(struct pexornet_dmabuf));
  descriptor->size = size;

  if (pgoff == 0)
  {
    /* no external target address specified, we create internal buffers  */

#ifdef	DMA_MAPPING_STREAMING
    /* here we use plain kernel memory which we explicitly map for dma*/
    descriptor->kernel_addr=(unsigned long) kmalloc(size, GFP_ATOMIC);
    if(!descriptor->kernel_addr)
    {
      pexornet_msg(KERN_ERR "new_dmabuffer: could not alloc streaming dma buffer for size %d \n",size);
      kfree(descriptor);
      return NULL;
    }
    descriptor->dma_addr= dma_map_single(&(pdev->dev), (void*) descriptor->kernel_addr, size, PCI_DMA_FROMDEVICE);
    if(!descriptor->dma_addr)
    {
      pexornet_msg(KERN_ERR "new_dmabuffer: could not map streaming dma buffer for size %d \n",size);
      kfree((void*) descriptor->kernel_addr);
      kfree(descriptor);
      return NULL;
    }

    pexornet_dbg(KERN_ERR "new_dmabuffer created streaming kernel buffer with dma address %lx\n", descriptor->dma_addr);

#else
    /* here we get readily mapped dma memory which was preallocated for the device */
    descriptor->kernel_addr = (unsigned long) pci_alloc_consistent (pdev, size, &(descriptor->dma_addr));
    if (!descriptor->kernel_addr)
    {
      pexornet_msg(KERN_ERR "new_dmabuffer: could not alloc pci dma buffer for size %d \n",(int)size);
      kfree (descriptor);
      return NULL ;
    }
    /* maybe obsolete here, but we could gain performance by defining the data direction...*/
    pci_dma_sync_single_for_device (pdev, descriptor->dma_addr, descriptor->size, PCI_DMA_FROMDEVICE);
    pexornet_dbg(KERN_ERR "new_dmabuffer created coherent kernel buffer with dma address %p\n", (void*) descriptor->dma_addr);

#endif

  }
  else
  {
    /* set dma buffer for external physical dma address*/
    descriptor->kernel_addr = 0; /* can not map external RAM into linux kernel space*/
    descriptor->dma_addr = pgoff << PAGE_SHIFT;
    pci_dma_sync_single_for_device (pdev, descriptor->dma_addr, descriptor->size, PCI_DMA_FROMDEVICE);
    pexornet_dbg(KERN_ERR "new_dmabuffer created dma buffer for external dma address %p\n", (void*) descriptor->dma_addr);
  }

  INIT_LIST_HEAD (&(descriptor->queue_list));
  pexornet_dbg(KERN_NOTICE "**pexornet_created new_dmabuffer, size=%d, addr=%lx \n", (int) size, descriptor->kernel_addr);

  return descriptor;
}

int delete_dmabuffer (struct pci_dev * pdev, struct pexornet_dmabuf* buf)
{
  /*int i=0; */
  if (buf->kernel_addr == 0)
  {
    if (buf->sg != 0)
    {
      /* buffer with no kernel memory but sg list -> must be sglist userbuffer*/
      return (unmap_sg_dmabuffer (pdev, buf));
    }
    else
    {
      /* neither kernel address nor sg list -> external phys memory*/
      pexornet_dbg(
          KERN_NOTICE "**pexornet_delete_dmabuffer of size=%ld, unregistering external physaddr=%lx \n", buf->size, (unsigned long) buf->dma_addr);
      pci_dma_sync_single_for_cpu (pdev, buf->dma_addr, buf->size, PCI_DMA_FROMDEVICE);
      /* Release descriptor memory */
      kfree (buf);
      return 0;
    }

  }

  pexornet_dbg(KERN_NOTICE "**pexornet_deleting dmabuffer, size=%ld, addr=%lx \n", buf->size, buf->kernel_addr);
  /* for (i=0;i<50;++i)
 {
 pexornet_dbg(KERN_NOTICE "dmabuffer[%x]=%x \t", i, ioread32(buf->kernel_addr + i*4));
 }*/
  /* note: unmapping the virtual adresses is done in user application by munmap*/
#ifdef	DMA_MAPPING_STREAMING
  /* release dma mapping and free kernel memory for dma buffer*/
  dma_unmap_single(&(pdev->dev), buf->dma_addr, buf->size, PCI_DMA_FROMDEVICE);

  kfree((void*) buf->kernel_addr);
#else
  /* Release DMA memory */
  pci_free_consistent (pdev, buf->size, (void *) (buf->kernel_addr), buf->dma_addr);
#endif
  /* Release descriptor memory */
  kfree (buf);
  return 0;
}

int unmap_sg_dmabuffer (struct pci_dev *pdev, struct pexornet_dmabuf *buf)
{
  int i = 0;
  pexornet_dbg(KERN_NOTICE "**pexornet unmapping sg dmabuffer, size=%ld, user address=%lx \n", buf->size, 0);//buf->virt_addr);
  pci_unmap_sg (pdev, buf->sg, buf->num_pages, PCI_DMA_FROMDEVICE);
  for (i = 0; i < (buf->num_pages); i++)
  {
    if (!PageReserved (buf->pages[i]))
    {
      SetPageDirty (buf->pages[i]);
      __clear_page_locked (buf->pages[i]);
      //ClearPageLocked(buf->pages[i]);
    }
    page_cache_release( buf->pages[i]);
  }
  vfree (buf->pages);
  vfree (buf->sg);
  kfree (buf);

  return 0;
}

void pexornet_cleanup_buffers (struct pexornet_privdata* priv)
{
  struct pexornet_dmabuf* cursor;
  struct pexornet_dmabuf* next;
  pexornet_msg(KERN_NOTICE "**pexornet_cleanup_buffers...\n");

  if (pexornet_poll_dma_complete (priv))
  {
    pexornet_msg(KERN_NOTICE "**pexornet_cleanup_buffers: dma is not finished, do not touch buffers!\n");
    return;
  }

  spin_lock_bh( &(priv->buffers_lock));
  /* remove reference in receive queue (discard contents):*/
  list_for_each_entry_safe(cursor, next, &(priv->received_buffers), queue_list)
  {
    list_del(&(cursor->queue_list)); /* put out of list*/
    delete_dmabuffer(priv->pdev, cursor);
  }
  /* remove reference in free list:*/
  list_for_each_entry_safe(cursor, next, &(priv->free_buffers), queue_list)
  {
    list_del(&(cursor->queue_list)); /* put out of list*/
    delete_dmabuffer(priv->pdev, cursor);
  }


  /* remove reference in used list:*/
  list_for_each_entry_safe(cursor, next, &(priv->used_buffers), queue_list)
  {
    list_del(&(cursor->queue_list)); /* put out of list*/
    delete_dmabuffer(priv->pdev, cursor);
  }
  spin_unlock_bh( &(priv->buffers_lock));

  pexornet_dbg(KERN_NOTICE "**pexornet_cleanup_buffers...done\n");
}


void pexornet_build_buffers (struct pexornet_privdata *priv, size_t buflen, unsigned int numbufs)
{
  unsigned int i;
  struct pexornet_dmabuf* buf;
  spin_lock_bh( &(priv->buffers_lock));
  for (i = 0; i < numbufs; ++i)
  {
    buf = new_dmabuffer (priv->pdev, buflen, 0);
    list_add_tail (&(buf->queue_list), &(priv->free_buffers));

  }
  spin_unlock_bh( &(priv->buffers_lock));
  pexornet_msg(KERN_NOTICE "pexornet_build_buffers has allocated %d buffers of length %d bytes.\n",numbufs, (int) buflen);

}




struct pexornet_privdata* pexornet_get_privdata (struct net_device *netdev)
{
  struct pexornet_privdata *privdata;
  struct pexornet_netdev_privdata* netpriv;
  netpriv=netdev_priv(netdev);
  privdata = (struct pexornet_privdata*) netpriv->pci_privdata;
  return privdata;
}

void print_register (const char* description, u32* address)
{
  pexornet_dbg(KERN_NOTICE "%s:\taddr=%lx cont=%x\n", description, (long unsigned int) address, readl(address));
  pexornet_bus_delay();
}

void print_pexornet (struct dev_pexornet* pg)
{
  if (pg == 0)
    return;pexornet_dbg(KERN_NOTICE "\n##print_pexornet: ###################\n");
  pexornet_dbg(KERN_NOTICE "init: \t=%x\n", pg->init_done);
  if (!pg->init_done)
    return;
  print_register ("dma control/status", pg->dma_control_stat);
#ifdef PEXORNET_WITH_TRIXOR
  /*pexornet_dbg(KERN_NOTICE "trixor control add=%x \n",pg->irq_control) ;
 pexornet_dbg(KERN_NOTICE "trixor status  add =%x \n",pg->irq_status);
 pexornet_dbg(KERN_NOTICE "trixor fast clear add=%x \n",pg->trix_fcti) ;
 pexornet_dbg(KERN_NOTICE "trixor conversion time add =%x \n",pg->trix_cvti);*/

  print_register ("trixor fast clear time", pg->trix_fcti);
  print_register ("trixor conversion time", pg->trix_cvti);
#endif

  print_register ("dma source address", pg->dma_source);
  print_register ("dma dest   address", pg->dma_dest);
  print_register ("dma len   address", pg->dma_len);
  print_register ("dma burstsize", pg->dma_burstsize);

  print_register ("RAM start", pg->ram_start);
  print_register ("RAM end", pg->ram_end);
  pexornet_dbg(KERN_NOTICE "RAM DMA base add=%x \n", (unsigned) pg->ram_dma_base);
  pexornet_dbg(KERN_NOTICE "RAM DMA cursor add=%x \n", (unsigned) pg->ram_dma_cursor);

#ifdef PEXORNET_WITH_SFP
  print_sfp (&(pg->sfp));
#endif

}

void clear_pexornet (struct dev_pexornet* pg)
{
  if (pg == 0)
    return;
  pg->init_done = 0x0;
  pexornet_dbg(KERN_NOTICE "** Cleared pexornet structure %lx.\n", (long unsigned int) pg);
}

void set_pexornet (struct dev_pexornet* pg, void* membase, unsigned long bar)
{

  void* dmabase = 0;
  if (pg == 0)
    return;
  dmabase = membase + PEXORNET_DMA_BASE;
#ifdef PEXORNET_WITH_TRIXOR
  pg->irq_control = (u32*) (membase + PEXORNET_TRIXOR_BASE + PEXORNET_TRIX_CTRL);
  pg->irq_status = (u32*) (membase + PEXORNET_TRIXOR_BASE + PEXORNET_TRIX_STAT);
  pg->trix_fcti = (u32*) (membase + PEXORNET_TRIXOR_BASE + PEXORNET_TRIX_FCTI);
  pg->trix_cvti = (u32*) (membase + PEXORNET_TRIXOR_BASE + PEXORNET_TRIX_CVTI);
#else
  pg->irq_control=(u32*)(membase+PEXORNET_IRQ_CTRL);
  pg->irq_status=(u32*)(membase+PEXORNET_IRQ_STAT);
#endif

  pg->dma_control_stat = (u32*) (dmabase + PEXORNET_DMA_CTRLSTAT);
  pg->dma_source = (u32*) (dmabase + PEXORNET_DMA_SRC);
  pg->dma_dest = (u32*) (dmabase + PEXORNET_DMA_DEST);
  pg->dma_len = (u32*) (dmabase + PEXORNET_DMA_LEN);
  pg->dma_burstsize = (u32*) (dmabase + PEXORNET_DMA_BURSTSIZE);

  pg->ram_start = (u32*) (membase + PEXORNET_DRAM);
  pg->ram_end = (u32*) (membase + PEXORNET_DRAM + PEXORNET_RAMSIZE);
  pg->ram_dma_base = (dma_addr_t) (bar + PEXORNET_DRAM);
  pg->ram_dma_cursor = (dma_addr_t) (bar + PEXORNET_DRAM);
#ifdef PEXORNET_WITH_SFP
  set_sfp (&(pg->sfp), membase, bar);
#endif

  pg->init_done = 0x1;
  pexornet_dbg(KERN_NOTICE "** Set pexornet structure %lx.\n", (long unsigned int) pg);

}

irqreturn_t pexornet_isr (int irq, void *dev_id)
{
  u32 irtype, irstat, irmask;
  int state;
#ifdef PEXORNET_WITH_TRIXOR
 // struct pexornet_trigger_buf* trigstat;
#endif
  struct pexornet_privdata *privdata;

  privdata = (struct pexornet_privdata *) dev_id;

  irmask=(TRIX_EV_IRQ_CLEAR | TRIX_DT_CLEAR);

#ifdef PEXORNET_DISABLE_IRQ_ISR
  //disable_irq(irq);  // disable and spinlock until any isr of that irq has been finished -> deadlock!
  disable_irq_nosync (irq);    // disable irq line
#endif
  //ndelay(1000);
  // need this here?
#ifdef PEXORNET_SHARED_IRQ

#ifdef PEXORNET_WITH_TRIXOR
  /* check if this interrupt was raised by our device*/
  irtype = ioread32 (privdata->registers.irq_status);
  mb();
  ndelay(20);
  //ndelay(200);
  pexornet_dbg(KERN_NOTICE "pexornet driver interrupt handler with interrupt status 0x%x!\n", irtype);
  if ((irtype & irmask) == irmask) /* test bits */
  {
    /* prepare for trixor interrupts here:*/
    irstat = (irtype << 16) & 0xffff0000;
    /*< shift trigger status bits to upper words to be compatible for historic mbs mapping definitions later*/
    irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
    iowrite32 (irtype, privdata->registers.irq_status); /*reset interrupt source*/
    mb();
    //ndelay(1000);
    ndelay(20);
    /* pexornet_dbg(KERN_NOTICE "pexornet driver interrupt handler cleared irq status!\n");*/
    /* now find out if we did interrupt test*/
    state = atomic_read(&(privdata->state));
   if (state == PEXORNET_STATE_TRIGGERED_READ)
    {
      /* in this mode we issue a tasklet that will handle automatic token request readout*/
      atomic_set(&(privdata->trigstat), irstat);
      // trigger status will be evaluated by tasklet, no parallel trigger queue!
      atomic_inc (&(privdata->irq_count));

      // test: we reset trigger module before we launch tasklet:
#ifdef PEXORNET_TOPHALF_TRIGGERCLEAR
      pexornet_trigger_reset(privdata);
      // if we want such thing, we need to check if readout of previous ir has been already done
      // before resetting trigger and scheduling the bottom half again!
      // this would be something like napi, and very like the mbs trigger semaphore check
      // we should try such thing asap, since we might save some interrupt latency here?
#endif

      // schedule tasklet
      pexornet_dbg(KERN_NOTICE "pexornet driver interrupt handler schedules tasklet... \n");
      tasklet_hi_schedule (&privdata->irq_bottomhalf);


    }
    else
    {
      /** network driver has only automatic triggered read. do nothing*/
    }

#ifdef PEXORNET_DISABLE_IRQ_ISR
    enable_irq (irq);
#endif

    return IRQ_HANDLED;
  }

#else

  /* check if this interrupt was raised by our device*/
  irtype=ioread32(privdata->registers.irq_status);
  mb();
  ndelay(20);
  if(irtype & PEXORNET_IRQ_USER_BIT)
  {

    /* OLD for pexornet 1*/
    mb();
    irstat=irtype;
    irtype &= ~(PEXORNET_IRQ_USER_BIT);
    iowrite32(irtype, privdata->registers.irq_control); /*reset interrupt source*/
    iowrite32(irtype, privdata->registers.irq_status); /*reset interrupt source*/
    mb();
    ndelay(20);
    pexornet_msg(KERN_NOTICE "pexornet driver interrupt handler cleared irq status!\n");
    /* now find out if we did interrupt test, trigger, or  real dma raised interrupt:*/
    state=atomic_read(&(privdata->state));
    if(state==PEXORNET_STATE_IR_TEST)
    {
      pexornet_msg(KERN_NOTICE "pexornet driver interrupt handler sees ir test!\n");
      state=PEXORNET_STATE_STOPPED;
      atomic_set(&(privdata->state),state);
      //	  ndelay(1000);
      //	  enable_irq(irq);
      return IRQ_HANDLED;
    }

  }
#endif

  else
  {
    pexornet_dbg(KERN_NOTICE "pexornet test driver interrupt handler sees unknown ir type %x !\n", irtype);
#ifdef PEXORNET_DISABLE_IRQ_ISR
    enable_irq (irq);
#endif
    return IRQ_NONE;
  }

#ifdef PEXORNET_DISABLE_IRQ_ISR
  enable_irq (irq);
#endif
  return IRQ_HANDLED;

#else
  pexornet_msg(KERN_NOTICE "pexornet test driver interrupt handler is executed non shared.\n");

  iowrite32(0, privdata->registers.irq_control);
  return IRQ_HANDLED; /* for debug*/

#endif

}

void pexornet_irq_tasklet (unsigned long arg)
{
  int retval, errcount =0, intcount=0;
  //static int bufid = 0;
  int readflag=0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 dmasize = 0, woffset = 0, comm = 0, trigstat = 0;
  int sfp = 0, channelmask = 0;
  unsigned long flags;
  struct pexornet_trigger_status descriptor;
  struct pexornet_dmabuf dmabuf;
  struct pexornet_sfp* sfpregisters;
  struct pexornet_privdata *privdata;
  privdata = (struct pexornet_privdata*) arg;


  pexornet_gosip_lock(&(privdata->gosip_lock), flags); // to begin with, we lock complete tasklet against ioctl and ifconfig callbacks!


  trigstat = atomic_read(&(privdata->trigstat));
  pexornet_dbg(
      KERN_NOTICE "pexornet_irq_tasklet is executed, irq_count=%d, trigstat=0x%x\n", atomic_read(&(privdata->irq_count)), trigstat);

#ifndef PEXORNET_TOPHALF_TRIGGERCLEAR
  /* check interrupt count before tasklet. should be one! */
  if (!atomic_dec_and_test (&(privdata->irq_count)))
  {
    pexornet_msg(KERN_ALERT "pexornet_irq_tasklet found more than one ir: N.C.H.\n");
  }
#else
  intcount=atomic_dec_return (&(privdata->irq_count));
  pexornet_msg(KERN_ALERT "pexornet_irq_tasklet has interrupt count: %d\n",intcount+1); // just to debug what is going on. later quit
#endif




  pexornet_decode_triggerstatus (trigstat, &descriptor);
  pexornet_dbg(
      KERN_NOTICE "pexornet_irq_tasklet receives trigtyp:0x%x si:0x%x mis:0x%x lec:0x%x di:0x%x tdt:0x%x eon:0x%x \n", descriptor.typ, descriptor.si, descriptor.mis, descriptor.lec, descriptor.di, descriptor.tdt, descriptor.eon);
  // here we have to perform the real halt if trigger was stop acquisition!
  if (descriptor.typ == PEXORNET_TRIGTYPE_STOP)
  {
    pexornet_trigger_do_stop (privdata);
    pexornet_dbg(KERN_NOTICE "pexornet_irq_tasklet has trigger 0x%x, did trixor halt and clear!\n", PEXORNET_TRIGTYPE_STOP);
#ifndef PEXORNET_TOPHALF_TRIGGERCLEAR
#ifdef PEXORNET_EARLY_TRIGGERCLEAR
      pexornet_trigger_reset (privdata);
#endif
#endif

  }
  else if (descriptor.typ == PEXORNET_TRIGTYPE_START)
  {
    /* do nothing special here. Trigger type is passed upwards with dummy buffer,
     * userland application may react by explicit readout request.*/
    woffset = 0;
    atomic_set(&(privdata->bufid), 0); // after start acquisition, always begin with bufid 0
#ifndef PEXORNET_TOPHALF_TRIGGERCLEAR
#ifdef PEXORNET_EARLY_TRIGGERCLEAR
      pexornet_trigger_reset (privdata);
#endif
#endif



  }
  else
  {
    /** here automatic token request mode*/


    /* loop over all configured sfps*/
    sfpregisters = &(privdata->registers.sfp);

    // JAM 2016: we try some kind of user trigger clear before we read out the stuff...
    //pexornet_trigger_reset (privdata);


    // first save the readflag from buffer id and switch to second frontend:
    readflag=atomic_read(&(privdata->bufid));
    #ifdef PEXORNET_WAIT_FOR_DATA_READY
      readflag |=2; // TODO: put this into sysfs configuration
    #endif
      /* switch frontend double buffer id for next request!
         * otherwise frontends may stall after 3rd event*/
        //bufid = (bufid ? 0 : 1);
        // JAM: try the same with mostly atomic operations:
         if(atomic_cmpxchg(&(privdata->bufid), 0, 1)==1)
           atomic_cmpxchg(&(privdata->bufid), 1, 0);
         ///////////////////////////
         // int atomic_cmpxchg (  volatile __global int *p, int cmp, int val)
         //Read the 32-bit value (referred to as old) stored at location p. Compute (old == cmp) ? val : old and store result at location pointed by p.
         //The function returns old.
         ///////////////////////////

#ifndef PEXORNET_TOPHALF_TRIGGERCLEAR
#ifdef PEXORNET_EARLY_TRIGGERCLEAR
         pexornet_trigger_reset (privdata);
#endif
#endif




    for (sfp = 0; sfp < PEXORNET_SFP_NUMBER; ++sfp)
    {
      if (sfpregisters->num_slaves[sfp] == 0)
        continue;
      /* for each do token request with direct dma:*/
      channelmask = 1 << (sfp + 1);    // select SFP for PCI Express DMA

      // allow here some retry if temporarily no buffers are available if user changes mtu:
      do{
        retval = pexornet_next_dma (privdata, 0, 0, woffset, 0, 0, channelmask);
        if((retval !=0) && ((retval != -ENOMEM) || errcount >= PEXORNET_MAXBUFRETRIES)) // other errors will stop acquisition immediately
        {
          pexornet_msg(KERN_ERR "pexornet_irq_tasklet error %d from nextdma, after %d retries\n", retval, errcount);
          atomic_set(&(privdata->state), PEXORNET_STATE_STOPPED);
          goto unlockgosip;
        }
        errcount++;
      } while(retval!=0);


      /** the actual token request: */
      comm = PEXORNET_SFP_PT_TK_R_REQ | (0x1 << (16 + sfp)); /* single sfp token mode*/
      pexornet_sfp_clear_channel (privdata, sfp);
      pexornet_sfp_request (privdata, comm, readflag, 0); /* note: slave is not specified; the chain of all slaves will send everything to receive buffer*/
      ndelay(1000);
      /* give pexornet time to evaluate requests?*/
      //if ((retval = pexornet_sfp_get_reply (privdata, sfp, &rstat, &radd, &rdat, 0)) != 0)    // debug: do not check reply status
      if((retval=pexornet_sfp_get_reply(privdata, sfp, &rstat, &radd, &rdat, PEXORNET_SFP_PT_TK_R_REP))!=0)
      {
        pexornet_msg(KERN_ERR "** pexornet_irq_tasklet: error %d at sfp_%d reply \n",retval,sfp);
        pexornet_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
        goto unlockgosip;
      }

      if ((retval = pexornet_poll_dma_complete (privdata)) != 0)
        goto unlockgosip;
      /* probably poll_dma_complete is not necessary here, since direct dma will reply
       * token request no sooner than dma has finished ?*/


      /* find out real package length after dma has completed:*/
      dmasize = ioread32 (privdata->registers.dma_len);
      mb();
      ndelay(20);
      pexornet_dbg(KERN_NOTICE "pexornet_irq_tasklet for sfp:%dfinds dma len:=0x%x...\n", sfp, dmasize);
      woffset += dmasize;
      /* check if offset will exceed dma receivce buffer is done in pexornet_next_dma(
       * note that we cannot check if dma will reach buffer boundaries, since direct dma*/

      /** after this has finished, evaluate next one with write offset to same buffer*/
    }    // for sfp
    /* this is the final dma receiving that also moves current buffer to receive queue.
     * Used size is derived from final write offset */

  } /* if else trigtype */


  // do not reset trigger before all frontends have been read out!
  // one could do this with special treatment in top half, emulating such napi  behaviour
  // i.e. if new trigger has arrived, readout is postponed until previous readout has been done.
  // would this be faster than directly resetting the irq source?

  pexornet_receive_dma_buffer (privdata, woffset);
  /* poll for final dma completion and wake up "DMA wait queue""
   * note that this function is executed even if no token request DMA was performed (special trigger 14/15)
   * In this case, just the empty next buffer of free queue is moved to receive queue and send
   * to userland as dummy buffer, marked with trigstat.*/

  //udelay(10);
  // waitstate between readout complete and trigger reset
  /** RESET trigger here, probably we can do this already before  pexornet_receive_dma_buffer?*/

  // JAM 2016 test if we can do this before!

  // HERE IT WAS, now we try doing it in top half! not good idea!!!

#ifndef PEXORNET_TOPHALF_TRIGGERCLEAR
#ifndef PEXORNET_EARLY_TRIGGERCLEAR
         pexornet_trigger_reset (privdata);
#endif
#endif



//  /* switch frontend double buffer id for next request!
//   * otherwise frontends may stall after 3rd event*/
//  //bufid = (bufid ? 0 : 1);
//  // JAM: try the same with mostly atomic operations:
//   if(atomic_cmpxchg(&(privdata->bufid), 0, 1)==1)
//     atomic_cmpxchg(&(privdata->bufid), 1, 0);
//   ///////////////////////////
//   // int atomic_cmpxchg (  volatile __global int *p, int cmp, int val)
//   //Read the 32-bit value (referred to as old) stored at location p. Compute (old == cmp) ? val : old and store result at location pointed by p.
//   //The function returns old.
//   ///////////////////////////



  /** for network driver: we do not have waiting ioctl, but tasklet has to fetch received
   * buffer and put it into socket buffer for the moment */
  retval = pexornet_wait_dma_buffer (privdata, &dmabuf);
  if (retval)
    {
          /* error handling, e.g. no more dma buffer available*/
          pexornet_msg(KERN_ERR "pexornet_irq_tasklet error %d from pexornet_wait_dma_buffer\n", retval);
          if(retval!=-ENOMEM)
          {
            // only stop acquisition if we do not have missing buffers due to mtu changes
            atomic_set(&(privdata->state), PEXORNET_STATE_STOPPED);
            goto freebuf;
          }
          dmabuf.used_size=0; // just forward empty package in case of out of buffer error
    }

  /** Set decoded triggerstatus:*/
  dmabuf.trigger_status = descriptor;

  pexornet_rx(privdata->net_dev,  &dmabuf); // copy to socket buffer layer here

freebuf:
  pexornet_freebuffer (privdata, &dmabuf); // back to free buffer list
unlockgosip:
  pexornet_gosip_unlock(&(privdata->gosip_lock), flags);


}

int pexornet_next_dma (struct pexornet_privdata* priv, dma_addr_t source, u32 roffset, u32 woffset, u32 dmasize,
    unsigned long bufid, u32 channelmask)
{
  struct pexornet_dmabuf* nextbuf;
  int i, rev, rest;
  struct scatterlist *sgentry;
  dma_addr_t sgcursor;
  unsigned int sglen, sglensum;
  if (source == 0)
  {
    priv->registers.ram_dma_cursor = (priv->registers.ram_dma_base + roffset);
  }
  else
  {
    priv->registers.ram_dma_cursor = (source + roffset);
  }
  /* setup next free buffer as dma target*/
  pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma...\n");

  spin_lock_bh( &(priv->buffers_lock));
  if (list_empty (&(priv->free_buffers)))
  {
    spin_unlock_bh( &(priv->buffers_lock));
    pexornet_msg(KERN_ERR "pexornet_next_dma: list of free buffers is empty. try again later! \n");
    return -ENOMEM;
    /* TODO: handle dynamically what to do when running out of dma buffers*/
  }
  /* put here search for dedicated buffer in free list:*/
//  if (bufid != 0)
//  {
//    /* we want to fill a special buffer, find it in free list:*/
//    list_for_each_entry(nextbuf, &(priv->free_buffers), queue_list)
//    {
//      if(nextbuf->virt_addr==bufid)
//      {
//        pexornet_dbg(KERN_NOTICE "** pexornet_next_dma is using buffer of id 0x%lx\n",bufid);
//        /* put desired buffer to the begin of the free list, this will be treated subsequently*/
//        list_move(&(nextbuf->queue_list) , &(priv->free_buffers));
//        break;
//      }
//    }
//    if (nextbuf->virt_addr != bufid)
//    {
//      /* check again if we found the correct buffer in list...*/
//      spin_unlock_bh( &(priv->buffers_lock));
//      pexornet_dbg(KERN_ERR "pexornet_next_dma: buffer of desired id 0x%lx is not in free list! \n", bufid);
//      return -EINVAL;
//    }
//  }
//  else
  {
    /* just take next available buffer to fill by DMA:*/
    nextbuf=list_first_entry(&(priv->free_buffers), struct pexornet_dmabuf, queue_list);
  }
  spin_unlock_bh( &(priv->buffers_lock));

  if (woffset > nextbuf->size - 8)
  {
    pexornet_msg(
        KERN_NOTICE "#### pexornet_next_dma illlegal write offset 0x%x for target buffer size 0x%x\n", woffset, (unsigned) nextbuf->size);
    return -EINVAL;
  }

  /* here decision if sg dma or plain*/

  //if((nextbuf->kernel_addr !=0)
  if (nextbuf->sg == 0) /* this check is better, since dma to external phys also has no kernel adress!*/
  {
    /* here we have coherent kernel buffer case*/
    pexornet_dbg(KERN_ERR "#### pexornet_next_dma in kernel buffer mode\n");
    if ((channelmask < 1))
    {
      // regular dma from pexornet memory: check boundaries

      if ((dmasize == 0) || (dmasize > nextbuf->size - woffset))
      {
        pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma resetting old dma size %x to %lx\n", dmasize, nextbuf->size);
        dmasize = nextbuf->size - woffset;
      }

      // JAM NOTE: this check is only meaningfull for dma tests
      // RAMSIZE here covers only ram for sfp0, it will fail for higher sfps!
      //
      //if (priv->registers.ram_dma_cursor + dmasize > priv->registers.ram_dma_base + PEXORNET_RAMSIZE)
      //  {
      //  pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma resetting old dma size %x...\n",dmasize);
      //  dmasize = priv->registers.ram_dma_base + PEXORNET_RAMSIZE - priv->registers.ram_dma_cursor;
      //  }

    }
    else
    {
      // direct dma with gosip request, unknown size!
      // can not check anything here!

    }

    /* check if size is multiple of burstsize and correct:*/
    rest = dmasize % PEXORNET_BURST;
    if (rest)
    {
      dmasize = dmasize + PEXORNET_BURST - rest;
      if (dmasize > nextbuf->size)
        dmasize -= PEXORNET_BURST; /*avoid exceeding buf limits*/

      pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma correcting dmasize %x for rest:%x, burst:%x\n", dmasize, rest, PEXORNET_BURST);
      /*if(dmasize==nextbuf->size)
 {
 pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma substracting dmasize rest:%x\n",rest);
 dmasize-=rest;
 }
 else
 {
 dmasize= dmasize + PEXORNET_BURST - rest;  if not at buffer end, try to increase to next burst edge
 if(dmasize > nextbuf->size) dmasize -= PEXORNET_BURST;  avoid exceeding buf limits anyway
 pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma correcting dmasize %x for rest:%x, burst:%x\n", dmasize, rest, PEXORNET_BURST);
 }*/
    }

#ifdef PEXORNET_WITH_SFP
    if (channelmask > 1)
    {
      if (pexornet_start_dma (priv, 0, nextbuf->dma_addr + woffset, 0, 0, channelmask) < 0)
        return -EINVAL;
    }
    else
#endif

      if (pexornet_start_dma (priv, priv->registers.ram_dma_cursor, nextbuf->dma_addr + woffset, dmasize, 0, channelmask) < 0)
        return -EINVAL;

  }

  else
  {
    /* put emulated sg dma here
     * since pexornet gosip fpga code does not support sglist dma, we do it manually within the driver*/
    pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma in scatter-gather mode\n");

    if (channelmask > 1)
    {
      pexornet_msg(KERN_ERR "#### pexornet_next_dma: ERROR no direct gosip DMA in scatter-gather mode\n");
      return -EINVAL;
    }

    /* test: align complete buffer to maximum burst?*/
    rest = dmasize % PEXORNET_BURST;
    if (rest)
    {
      dmasize = dmasize + PEXORNET_BURST - rest;
      if (dmasize > nextbuf->size)
        dmasize -= PEXORNET_BURST; /*avoid exceeding buf limits*/

      pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma correcting dmasize %x for rest:%x, burst:%x\n", dmasize, rest, PEXORNET_BURST);
    }

    sgcursor = priv->registers.ram_dma_cursor;
    sglensum = 0;
    i = 0;
    for_each_sg(nextbuf->sg,sgentry, nextbuf->sg_ents,i)
    {
      sglen = sg_dma_len(sgentry);
      if (woffset >= sglen)
      {
        /* find out start segment for offset and adjust local offset*/
        woffset -= sglen;
        continue;
      }
      sglen -= woffset; /* reduce transfer length from offset to end of first used segment*/
      if (dmasize < sglen)
        sglen = dmasize; /* source buffer fits into first sg page*/
      if (dmasize - sglensum < sglen)
        sglen = dmasize - sglensum; /* cut dma length for last sg page*/

      /* DEBUG: pretend to do dma, but do not issue it*/
      pexornet_dbg(
          KERN_ERR "#### pexornet_next_dma would start dma from 0x%x to 0x%x of length 0x%x, offset 0x%x, complete chunk length: 0x%x\n", (unsigned) sgcursor, (unsigned) sg_dma_address(sgentry), sglen, woffset, sg_dma_len(sgentry));

      /**** END DEBUG*/
      /* initiate dma to next sg part:*/
      if (pexornet_start_dma (priv, sgcursor, sg_dma_address(sgentry) + woffset, sglen, (woffset > 0), 0) < 0)
        return -EINVAL;
      if (woffset > 0)
        woffset = 0; /* reset write offset, once it was applied to first sg segment*/

      if ((rev = pexornet_poll_dma_complete (priv)) != 0)
      {
        pexornet_dbg(KERN_ERR "#### pexornet_next_dma error on polling for sg entry %d completion, \n", i);
        return rev;
      }
      sglensum += sglen;
      if (sglensum >= dmasize)
      {
        pexornet_dbg(KERN_NOTICE "#### pexornet_next_dma has finished sg buffer dma after %d segments\n", i);
        break;
      }
      sgcursor += sglen;

    }    // for each sg

    if (sglensum < dmasize)
    {
      pexornet_dbg(KERN_ERR "#### pexornet_next_dma could not write full size 0x%x to sg buffer of len 0x%x\n", dmasize, sglensum);
      return -EINVAL;
    }

  }    // end plain dma or emulated sg

  return 0;

}

int pexornet_start_dma (struct pexornet_privdata *priv, dma_addr_t source, dma_addr_t dest, u32 dmasize, int firstchunk,
    u32 channelmask)
{
  int rev;
  u32 burstsize = PEXORNET_BURST;
  u32 enable = PEXORNET_DMA_ENABLED_BIT; /* this will start dma immediately from given source address*/
  if (channelmask > 1)
    enable = channelmask; /* set sfp token transfer to initiate the DMA later*/

  if (enable == PEXORNET_DMA_ENABLED_BIT) // JAM test for nyxor problem: only check previous dma if not in direct dma preparation mode
    {
      rev = pexornet_poll_dma_complete (priv);
      if (rev)
      {
        pexornet_msg(KERN_NOTICE "**pexornet_start_dma: dma was not finished, do not start new one!\n");
        return rev;
      }
    }


  /* calculate maximum burstsize here:*/
  while (dmasize % burstsize)
  {
    burstsize = (burstsize >> 1);
  }
  if (burstsize < PEXORNET_BURST_MIN)
  {
    pexornet_dbg(KERN_NOTICE "**pexornet_start_dma: correcting for too small burstsize %x\n", burstsize);
    burstsize = PEXORNET_BURST_MIN;
    while (dmasize % burstsize)
    {
      if (firstchunk)
      {
        /* We assume this only happens in sg mode for the first chunk when applying header offset which is word aligned
         * In this case we just start a little bit before the payload and overwrite some bytes of the header part
         * We also assume that complete PCIe bar of pexornet is dma mapped, so it doesnt hurt for the source!*/
        source -= 2;
        dest -= 2;
      }
      dmasize += 2;
      /* otherwise this can only happen in the last chunk of sg dma.
       * here it should be no deal to transfer a little bit more...*/
    }
    pexornet_dbg(
        KERN_NOTICE "**changed source address to 0x%x, dest:0x%x, dmasize to 0x%x, burstsize:0x%x\n", (unsigned) source, (unsigned) dest, dmasize, burstsize)
  }

  if(dmasize==0)
  {
    /* JAM 2016 - for direct DMA mode : always use minimum burst because actual payload size is unknown!*/
    burstsize=PEXORNET_BURST_MIN;
  }


  pexornet_dbg(
      KERN_NOTICE "#### pexornet_start_dma will initiate dma from %p to %p, len=%x, burstsize=%x...\n", (void*) source, (void*) dest, dmasize, burstsize);

  iowrite32 (source, priv->registers.dma_source);
  mb();
  iowrite32 ((u32) dest, priv->registers.dma_dest);
  mb();
  iowrite32 (burstsize, priv->registers.dma_burstsize);
  mb();
  iowrite32 (dmasize, priv->registers.dma_len);
  mb();
  iowrite32 (enable, priv->registers.dma_control_stat);
  mb();
  if (enable > 1)
  {
    pexornet_dbg(KERN_NOTICE "#### pexornet_start_dma sets sfp mask to 0x%x \n", enable);
  }
  else
  {
    pexornet_dbg(KERN_NOTICE "#### pexornet_start_dma started dma\n");
  }
  return 0;
}

int pexornet_poll_dma_complete (struct pexornet_privdata* priv)
{
  int loops = 0;
  u32 enable = PEXORNET_DMA_ENABLED_BIT;

  while (1)
  {
    /* pexornet_dbg(KERN_ERR "pexornet_poll_dma_complete reading from 0x%p \n",priv->registers.dma_control_stat);*/

    enable = ioread32 (priv->registers.dma_control_stat);
    mb();
    //   pexornet_dbg(KERN_ERR "pexornet_poll_dma_complete sees dmactrl=: 0x%x , looking for %x\n",enable, PEXORNET_DMA_ENABLED_BIT);
    if ((enable & PEXORNET_DMA_ENABLED_BIT) == 0)
      break;
    /* poll until the dma bit is cleared => dma complete*/

    //pexornet_dbg(KERN_NOTICE "#### pexornet_poll_dma_complete wait for dma completion #%d\n",loops);
    if (loops++ > PEXORNET_DMA_MAXPOLLS)
    {
      pexornet_msg(KERN_ERR "pexornet_poll_dma_complete: polling longer than %d cycles (delay %d ns) for dma complete!!!\n",PEXORNET_DMA_MAXPOLLS, PEXORNET_DMA_POLLDELAY );
      return -EFAULT;
    }
    if (PEXORNET_DMA_POLLDELAY)
      ndelay(PEXORNET_DMA_POLLDELAY);
    //    if (PEXORNET_DMA_POLL_SCHEDULE)
    //      schedule (); // never do this in irq tasklet!
  };    // while

  return 0;
}

int pexornet_receive_dma_buffer (struct pexornet_privdata *privdata, unsigned long used_size)
{
  int state, rev = 0;
  struct pexornet_dmabuf* nextbuf;
  if ((rev = pexornet_poll_dma_complete (privdata)) != 0)
    return rev;

  /* transfer buffer from free queue to receive queue*/
  spin_lock_bh( &(privdata->buffers_lock));

  /* check if free list is empty <- can happen if dma flow gets suspended
   * and waitreceive is called in polling mode*/
  if (list_empty (&(privdata->free_buffers)))
  {
    spin_unlock_bh( &(privdata->buffers_lock));
    pexornet_dbg(KERN_ERR "pexornet_receive_dma_buffer: list of free buffers is empty. no DMA could have been received!\n");
    /* return;  this would put the waitreceive into timeout, so does not try to read empty receive queue*/
    goto wakeup;
    /* to have immediate response and error from receive queue as well.*/
  }

  nextbuf=list_first_entry(&(privdata->free_buffers), struct pexornet_dmabuf, queue_list);
  list_move_tail (&(nextbuf->queue_list), &(privdata->received_buffers));
  spin_unlock_bh( &(privdata->buffers_lock));

  state = atomic_read(&(privdata->state));
  switch (state)
  {
    case PEXORNET_STATE_STOPPED:
      /* this can happen after processing trigger 15 in irq tasklet.
       * we just mark dummy buffer with trigger type and then wake up consumer:*/
      nextbuf->used_size = 0; /* need to tell consumer the real token data size*/
      //nextbuf->triggerstatus = triggerstatus; /* pass up triggerstatus for this data! done after this function before rx JAM*/
      break;


    case PEXORNET_STATE_TRIGGERED_READ:
      /* in case of auto trigger readout, do not change state to stopped.
       * we continue this mode until user stops acquisition*/
      if (used_size > nextbuf->size)
      {
        pexornet_msg(KERN_ALERT "pexornet_receive_dma_buffer - used size:%ld exceeds kernel buffer size:%ld, truncating!\n",
            used_size, nextbuf->size);
        used_size = nextbuf->size;
      }
      nextbuf->used_size = used_size; /* need to tell consumer the real token data size*/
      //nextbuf->triggerstatus = triggerstatus; /* remember triggerstatus for this data!*/

      break;

    default:
      atomic_set(&(privdata->state), PEXORNET_STATE_STOPPED);

  };

  wakeup:
  /* wake up the waiting ioctl*/
  //atomic_inc (&(privdata->dma_outstanding));
  // JAM2016 do not use this wake queue anymore!
  //wake_up_interruptible (&(privdata->irq_dma_queue));

  return rev;
}

int pexornet_wait_dma_buffer (struct pexornet_privdata* priv, struct pexornet_dmabuf* result)
{
  //unsigned long wjifs = 0;
  struct pexornet_dmabuf* dmabuf;

/** JAM: obviously not a good idea to sleep within interrupt tasklet!
 * deactivate the wait and rename function later*/
#if 0
  /**
   * wait_event_interruptible_timeout - sleep until a condition gets true or a timeout elapses
   * The process is put to sleep (TASK_INTERRUPTIBLE) until the
   * condition evaluates to true or a signal is received.
   * The condition is checked each time the waitqueue wq is woken up.
   *
   * wake_up() has to be called after changing any variable that could
   * change the result of the wait condition.
   *
   * The function returns 0 if the timeout elapsed, -ERESTARTSYS if it
   * was interrupted by a signal, and the remaining jiffies otherwise
   * if the condition evaluated to true before the timeout elapsed.
   */
  wjifs = wait_event_interruptible_timeout (priv->irq_dma_queue, atomic_read( &(priv->dma_outstanding) ) > 0,
      priv->wait_timeout * HZ);
  pexornet_dbg(
      KERN_NOTICE "** pexornet_wait_dma_buffer after wait_event_interruptible_timeout with TIMEOUT %d s (=%d jiffies), waitjiffies=%ld, outstanding=%d \n",   priv->wait_timeout,priv->wait_timeout * HZ, wjifs, atomic_read( &(priv->dma_outstanding)));

  if (wjifs == 0)
  {
    pexornet_dbg(
        KERN_NOTICE "** pexornet_wait_dma_buffer TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n", priv->wait_timeout * HZ);
    return PEXORNET_TRIGGER_TIMEOUT;
  }
  else if (wjifs == -ERESTARTSYS)
  {
    pexornet_msg(KERN_NOTICE "** pexornet_wait_dma _buffer after wait_event_interruptible_timeout woken by signal. abort wait\n");
    return -EFAULT;
  }
  else
  {
  }
#endif


//  atomic_dec (&(priv->dma_outstanding));

  /* Take next buffer out of receive queue */
  spin_lock_bh( &(priv->buffers_lock));
  /* need to check here for empty list, since list_first_entry will crash otherwise!*/
  if (list_empty (&(priv->received_buffers)))
  {
    /* this may happen if user calls waitreceive without a DMA been activated, or at flow DMA suspended*/
    spin_unlock_bh( &(priv->buffers_lock));
    pexornet_msg(KERN_NOTICE "** pexornet_wait_dma_buffer: receive queue is empty after wait! maybe someone changed MTU...\n");
    return -ENOMEM;
  }

  dmabuf=list_first_entry(&(priv->received_buffers), struct pexornet_dmabuf, queue_list);
  list_move_tail (&(dmabuf->queue_list), &(priv->used_buffers));
  spin_unlock_bh( &(priv->buffers_lock));
  if (dmabuf->dma_addr != 0) /* kernel buffer*/
    pci_dma_sync_single_for_cpu (priv->pdev, dmabuf->dma_addr, dmabuf->size, PCI_DMA_FROMDEVICE);
  else
    /* sg buffer*/
    pci_dma_sync_sg_for_cpu (priv->pdev, dmabuf->sg, dmabuf->sg_ents, PCI_DMA_FROMDEVICE);

  *result = *dmabuf;
  return 0;
}

#ifdef PEXORNET_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
ssize_t pexornet_sysfs_freebuffers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount=0;
  struct pexornet_privdata *privdata;
  struct list_head* cursor;
  privdata= (struct pexornet_privdata*) dev_get_drvdata(dev);
  spin_lock_bh( &(privdata->buffers_lock) );
  list_for_each(cursor, &(privdata->free_buffers))
  {
    bufcount++;
  }
  spin_unlock_bh( &(privdata->buffers_lock) );
  return snprintf(buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexornet_sysfs_usedbuffers_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount = 0;
  struct pexornet_privdata *privdata;
  struct list_head* cursor;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
  spin_lock_bh( &(privdata->buffers_lock));
  list_for_each(cursor, &(privdata->used_buffers))
  {
    bufcount++;
  }
  spin_unlock_bh( &(privdata->buffers_lock));
  return snprintf (buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexornet_sysfs_rcvbuffers_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount = 0;
  struct pexornet_privdata *privdata;
  struct list_head* cursor;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
  spin_lock_bh( &(privdata->buffers_lock));
  list_for_each(cursor, &(privdata->received_buffers))
  {
    bufcount++;
  }
  spin_unlock_bh( &(privdata->buffers_lock));
  return snprintf (buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexornet_sysfs_codeversion_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  char vstring[512];
  ssize_t curs = 0;
#ifdef PEXORNET_WITH_SFP
  struct dev_pexornet* pg;
#endif
  struct pexornet_privdata *privdata;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
  curs = snprintf (vstring, 512, "*** This is PEXORNET driver version %s build on %s at %s \n\t", PEXORNETVERSION, __DATE__,
      __TIME__);
#ifdef PEXORNET_WITH_SFP
  pg = &(privdata->registers);
  pexornet_show_version (&(pg->sfp), vstring + curs);
#endif
  return snprintf (buf, PAGE_SIZE, "%s\n", vstring);
}

ssize_t pexornet_sysfs_dmaregs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct dev_pexornet* pg;
  struct pexornet_privdata *privdata;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
  pg = &(privdata->registers);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEXORNET dma/irq register dump:\n");
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma control/status: 0x%x\n", readl(pg->dma_control_stat));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t irq/trixor stat  0x%x\n", readl(pg->irq_status));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t irq/trixor ctrl: 0x%x\n", readl(pg->irq_control));
  pexornet_bus_delay();
#ifdef PEXORNET_WITH_TRIXOR
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n", readl(pg->trix_fcti));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n", readl(pg->trix_cvti));
  pexornet_bus_delay();
#endif
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma source      address: 0x%x\n", readl(pg->dma_source));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma destination address: 0x%x\n", readl(pg->dma_dest));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma length:              0x%x\n", readl(pg->dma_len));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma burst size:          0x%x\n", readl(pg->dma_burstsize));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM start:               0x%x\n", readl(pg->ram_start));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM end:                 0x%x\n", readl(pg->ram_end));
  pexornet_bus_delay();
  return curs;
}

#ifdef PEXORNET_WITH_SFP

ssize_t pexornet_sysfs_sfp_retries_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
   struct pexornet_privdata *privdata;
   privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX gosip request retries:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_maxpolls);
   return curs;
}

ssize_t pexornet_sysfs_sfp_retries_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pexornet_privdata *privdata;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   privdata->sfp_maxpolls=val;
   pexornet_msg( KERN_NOTICE "PEXORNET: sfp maximum retries was set to %d => timeout = %d ns \n", privdata->sfp_maxpolls, (privdata->sfp_maxpolls * PEXORNET_SFP_DELAY));
  return count;
}


/* show sfp bus read/write waitstate in nanoseconds.
 * this will impose such wait time after each frontend address read/write ioctl */
ssize_t pexornet_sysfs_buswait_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
   struct pexornet_privdata *privdata;
   privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX gosip request retries:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_buswait);
   return curs;
}

/* set sfp bus read/write waitstate in nanoseconds. */
ssize_t pexornet_sysfs_buswait_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)

{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pexornet_privdata *privdata;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   privdata->sfp_buswait=val;
   pexornet_msg( KERN_NOTICE "PEXORNET: gosip bus io wait interval was set to %d microseconds\n", privdata->sfp_buswait);
  return count;
}


ssize_t pexornet_sysfs_trixorregs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
#ifdef PEXORNET_WITH_TRIXOR
  struct pexornet_privdata *privdata;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEXORNET trixor register dump:\n");

  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor stat: 0x%x\n", readl(privdata->registers.irq_status));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor ctrl: 0x%x\n", readl(privdata->registers.irq_control));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n", readl(privdata->registers.trix_fcti));
  pexornet_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n", readl(privdata->registers.trix_cvti));
#endif

  return curs;
}

ssize_t pexornet_sysfs_trixor_fctime_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
     struct pexornet_privdata *privdata;
     privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
     curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", (0x10000 - readl(privdata->registers.trix_fcti)));
     pexornet_bus_delay();
     return curs;
}

ssize_t pexornet_sysfs_trixor_fctime_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pexornet_privdata *privdata;
  privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif


   pexornet_bus_delay();
   iowrite32 (0x10000 - val, privdata->registers.trix_fcti);
   pexornet_msg( KERN_NOTICE "PEXORNET: trixor fast clear time was set to %d \n", val);
  return count;
}


ssize_t pexornet_sysfs_trixor_cvtime_show (struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t curs = 0;
    struct pexornet_privdata *privdata;
    privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
    curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", (0x10000 - readl(privdata->registers.trix_cvti)));
    pexornet_bus_delay();
    return curs;
}

ssize_t pexornet_sysfs_trixor_cvtime_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  unsigned int val=0;
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
    int rev=0;
  #else
    char* endp=0;
  #endif
    struct pexornet_privdata *privdata;
    privdata = (struct pexornet_privdata*) dev_get_drvdata (dev);
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
    rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
    if(rev!=0) return rev;
  #else
    val=simple_strtoul(buf,&endp, 0);
    count= endp - buf; // do we need this?
  #endif
     pexornet_bus_delay();
     iowrite32 (0x10000 - val, privdata->registers.trix_cvti);
     pexornet_msg( KERN_NOTICE "PEXORNET: trixor conversion time was set to %d \n", val);
    return count;

}





#endif // WITH SFP

#endif // KERNELVERSION CHECK
#endif // PEXORNET_SYSFS_ENABLE

#ifdef PEXORNET_DEBUGPRINT
static unsigned char get_pci_revision (struct pci_dev *dev)
{
  u8 revision;
  pci_read_config_byte (dev, PCI_REVISION_ID, &revision);
  return revision;
}

#endif

void test_pci (struct pci_dev *dev)
{
  int bar = 0;
  u32 originalvalue = 0;
  u32 base = 0;
  u16 comstat = 0;
  u8 typ = 0;
  pexornet_dbg(KERN_NOTICE "\n test_pci found PCI revision number %x \n", get_pci_revision(dev));

  /*********** test the address regions*/
  for (bar = 0; bar < 6; ++bar)
  {
    pexornet_dbg(KERN_NOTICE "Resource %d start=%x\n", bar, (unsigned) pci_resource_start( dev,bar ));
    pexornet_dbg(KERN_NOTICE "Resource %d end=%x\n", bar, (unsigned) pci_resource_end( dev,bar ));
    pexornet_dbg(KERN_NOTICE "Resource %d len=%x\n", bar, (unsigned) pci_resource_len( dev,bar ));
    pexornet_dbg(KERN_NOTICE "Resource %d flags=%x\n", bar, (unsigned) pci_resource_flags( dev,bar ));
    if ((pci_resource_flags (dev, bar) & IORESOURCE_IO))
    {
      // Ressource im IO-Adressraum
      pexornet_dbg(KERN_NOTICE " - resource is IO\n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_MEM))
    {
      pexornet_dbg(KERN_NOTICE " - resource is MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_IO))
    {
      pexornet_dbg(KERN_NOTICE " - resource is PCI IO\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_MEMORY))
    {
      pexornet_dbg(KERN_NOTICE " - resource is PCI MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      pexornet_dbg(KERN_NOTICE " - resource prefetch bit is set \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_64))
    {
      pexornet_dbg(KERN_NOTICE " - resource is 64bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_32))
    {
      pexornet_dbg(KERN_NOTICE " - resource is 32bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_PREFETCH))
    {
      pexornet_dbg(KERN_NOTICE " - resource is prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      pexornet_dbg(KERN_NOTICE " - resource is PCI mem prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_1M))
    {
      pexornet_dbg(KERN_NOTICE " - resource is PCI memtype below 1M \n");
    }

  }
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, originalvalue);
  pexornet_dbg("size of base address 0: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, originalvalue);
  pexornet_dbg("size of base address 1: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, originalvalue);
  pexornet_dbg("size of base address 2: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, originalvalue);
  pexornet_dbg("size of base address 3: %i\n", ~base+1);

  /***** here tests of configuration/status register:******/
  pci_read_config_word (dev, PCI_COMMAND, &comstat);
  pexornet_dbg("\n****  Command register is: %d\n", comstat);
  pci_read_config_word (dev, PCI_STATUS, &comstat);
  pexornet_dbg("\n****  Status register is: %d\n", comstat);
  pci_read_config_byte (dev, PCI_HEADER_TYPE, &typ);
  pexornet_dbg("\n****  Header type is: %d\n", typ);
}

void pexornet_cleanup_device (struct pexornet_privdata* priv)
{
  int j = 0;
  struct pci_dev* pcidev;
//  struct pexornet_trigger_buf* trigstat;
//  struct pexornet_trigger_buf* nexttrigstat;
  if (!priv)
    return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (priv->class_dev)
  {
#ifdef PEXORNET_SYSFS_ENABLE
#ifdef PEXORNET_WITH_SFP
    device_remove_file (priv->class_dev, &dev_attr_sfpregs);
  	device_remove_file (priv->class_dev, &dev_attr_gosipretries);
    device_remove_file (priv->class_dev, &dev_attr_gosipbuswait);
#endif
    device_remove_file (priv->class_dev, &dev_attr_trixorfcti);
    device_remove_file (priv->class_dev, &dev_attr_trixorcvti);
    device_remove_file (priv->class_dev, &dev_attr_trixorregs);
    device_remove_file (priv->class_dev, &dev_attr_dmaregs);
    device_remove_file (priv->class_dev, &dev_attr_codeversion);
    device_remove_file (priv->class_dev, &dev_attr_rcvbufs);
    device_remove_file (priv->class_dev, &dev_attr_usedbufs);
    device_remove_file (priv->class_dev, &dev_attr_freebufs);
#endif

    //device_destroy (pexornet_class, priv->devno); // we work on external net device, do not destroy it
    priv->class_dev = 0;
  }

#endif


/* network device cleanup:*/
  if (priv->net_dev)
    {
      unregister_netdev (priv->net_dev);
      free_netdev (priv->net_dev);
      atomic_dec (&pexornet_numdevs);
    }


  pcidev = priv->pdev;
  if (!pcidev)
    return;

  /* need to explicitely disable interrupt tasklet?*/
  tasklet_kill (&priv->irq_bottomhalf);

  /* may put disabling device irqs here?*/
#ifdef PEXORNET_ENABLE_IRQ
  free_irq (pcidev->irq, priv);
#endif

#ifdef IRQ_ENABLE_MSI
  pci_disable_msi(pcidev);
#endif

  pexornet_cleanup_buffers (priv);

  // here remove trigger status objects:
//  spin_lock_bh( &(priv->trigstat_lock));
//  list_for_each_entry_safe(trigstat, nexttrigstat, &(priv->trig_status), queue_list)
//  {
//    list_del(&(trigstat->queue_list)); /* put out of list*/
//    kfree(trigstat);
//  }
//  spin_unlock_bh( &(priv->trigstat_lock));

  for (j = 0; j < 6; ++j)
  {
    if (priv->bases[j] == 0)
      continue;
    if ((pci_resource_flags (pcidev, j) & IORESOURCE_IO))
    {
      pexornet_dbg(KERN_NOTICE " releasing IO region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
      release_region (priv->bases[j], priv->reglen[j]);
    }
    else
    {
      if (priv->iomem[j] != 0)
      {
        pexornet_dbg(
            KERN_NOTICE " unmapping virtual MEM region at:%lx -len:%lx \n", (unsigned long) priv->iomem[j], priv->reglen[j]);
        iounmap (priv->iomem[j]);
      }pexornet_dbg(KERN_NOTICE " releasing MEM region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
      release_mem_region (priv->bases[j], priv->reglen[j]);
    }
    priv->bases[j] = 0;
    priv->reglen[j] = 0;
  }
  kfree (priv);
  pci_disable_device (pcidev);
}


/***********************************************************************************************/
/*** NETWOK PROBES HERE **/


/*
 * Return statistics to the caller
 */
struct net_device_stats *pexornet_stats(struct net_device *dev)
{
  struct pexornet_privdata* priv=pexornet_get_privdata(dev);
  return &priv->stats;
}


int pexornet_open(struct net_device *dev)
{
    struct pexornet_privdata* priv;
    pexornet_msg(KERN_NOTICE "pexornet_open...\n");
    // JAM: probably some parts of the probe may better put here?
    /* request_region(), request_irq(), ....  (like fops->open) */
    priv=pexornet_get_privdata(dev);


    /*
     * Assign the hardware address of the board: use "\0PEXOR". TODO: real hardware address of board?
     * The first byte is '\0' to avoid being a multicast
     * address (the first byte of multicast addrs is odd).
     */
    memcpy(dev->dev_addr, "\0PEXOR", ETH_ALEN);


    /* JAM test: set LG bit*/
    dev->dev_addr[0] |= 0x2;

    /** allocate some dma buffers here:*/
    pexornet_build_buffers(priv, dev->mtu, PEXORNET_DEFAULTBUFFERNUM);

    netif_start_queue(dev); // JAM do we need transmit queue here?
    pexornet_msg(KERN_NOTICE "pexornet_open done.\n");
    return 0;
}

int pexornet_release(struct net_device *dev)
{
    /* release ports, irq and such -- like fops->close */

  /* JAM probably some parts of remove here?*/
    struct pexornet_privdata* priv;
    netif_stop_queue(dev); /* can't transmit any more */
    priv=pexornet_get_privdata(dev);
    pexornet_cleanup_buffers (priv);

    pexornet_msg(KERN_NOTICE "pexornet_release.\n");
    return 0;
}

/*
 * Configuration changes (passed on by ifconfig)
 */
int pexornet_config(struct net_device *dev, struct ifmap *map)
{
  printk(KERN_NOTICE "pexornet_config..\n");


    if (dev->flags & IFF_UP) /* can't act on a running interface */
        return -EBUSY;

    /* Don't allow changing the I/O address */
    if (map->base_addr != dev->base_addr) {
        printk(KERN_WARNING "pexornet: Can't change I/O address\n");
        return -EOPNOTSUPP;
    }

    /* Allow changing the IRQ */
//    if (map->irq != dev->irq) {
//        dev->irq = map->irq;
//            /* request_irq() is delayed to open-time */
//    }

    /* ignore other fields */
    return 0;
}


int pexornet_change_mtu (struct net_device *dev, int new_mtu)
{
  struct pexornet_privdata *priv = pexornet_get_privdata (dev);
  /* check ranges */
  if ((new_mtu < 68) || (new_mtu > PEXORNET_MAXMTU))
    return -EINVAL;
  dev->mtu = new_mtu;
  pexornet_cleanup_buffers (priv);
  pexornet_build_buffers (priv, dev->mtu, PEXORNET_DEFAULTBUFFERNUM);
  pexornet_msg(KERN_NOTICE "pexornet_change_mtu to %d\n",new_mtu);
  return 0; /* success */
}


int pexornet_rebuild_header(struct sk_buff *skb)
{
    int i;
    struct ethhdr *eth = (struct ethhdr *) skb->data;
    struct net_device *dev = skb->dev;

    memcpy(eth->h_source, dev->dev_addr, dev->addr_len);
    memcpy(eth->h_dest, dev->dev_addr, dev->addr_len);
    //eth->h_dest[ETH_ALEN-1]   ^= 0x01;   /* dest is us xor 1 */

    pexornet_msg(KERN_NOTICE "pexornet_rebuild_header: setting addresses:");
       for(i=0;i<ETH_ALEN;++i) pexornet_msg(KERN_NOTICE "%d.",(unsigned int) ((char*)eth->h_source)[i]);
       pexornet_msg(KERN_NOTICE "\n");

    return 0;
}


int pexornet_header(struct sk_buff *skb, struct net_device *dev,
                unsigned short type, const void *daddr, const void *saddr,
                unsigned int len)
{
    int i;
    struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);
    eth->h_proto = htons(type);
    if(saddr)
    {
      pexornet_dbg(KERN_NOTICE "pexornet_header: source:");
      for(i=0;i<ETH_ALEN;++i) pexornet_dbg(KERN_NOTICE "%d.",(unsigned int) ((char*)saddr)[i]);
      pexornet_dbg(KERN_NOTICE "\n");
    }
    if(daddr)
    {
      pexornet_dbg(KERN_NOTICE "pexornet_header: destination:");
      for(i=0;i<ETH_ALEN;++i) pexornet_dbg(KERN_NOTICE "%d.",(unsigned int) ((char*)daddr)[i]);
      pexornet_dbg(KERN_NOTICE "\n");
    }


    memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
    memcpy(eth->h_dest,   daddr ? daddr : dev->dev_addr, dev->addr_len);
//    pexornet_msg(KERN_NOTICE "pexornet_header: setting addresses for destination:");
//    for(i=0;i<ETH_ALEN;++i) pexornet_msg(KERN_NOTICE "%d.",(unsigned int) ((char*)eth->h_dest)[i]);
//      pexornet_msg(KERN_NOTICE "\n");
//      pexornet_dbg(KERN_NOTICE "pexornet_header: source:");
//          for(i=0;i<ETH_ALEN;++i) pexornet_msg(KERN_NOTICE "%d.",(unsigned int) ((char*)eth->h_source)[i]);
//          pexornet_msg(KERN_NOTICE "\n");





    return (dev->hard_header_len);
}


/*
 * Receive a packet: retrieve, encapsulate and pass over to upper levels
 * JAM this one is called from receive interrupt
 * prelliminary, later dmabuf will directly be pre-allocated skb
 */
void pexornet_rx (struct net_device *dev, struct pexornet_dmabuf *pkt)
{

#ifdef PEXORNET_DEBUGPRINT
  unsigned int *dmabufbase, *dmabufend , *cursor;
#endif

  int rev = 0, i;
  unsigned long datacnt = 0;
  struct sk_buff *skb;
  struct ethhdr *eth;
  struct iphdr *iph;
  struct udphdr *udph;
  struct pexornet_data_header dathead;
  struct pexornet_privdata *priv = pexornet_get_privdata (dev);
  //int maxheadroom = sizeof(struct ethhdr) + sizeof(struct iphdr)   + sizeof(struct udphdr) + 2; /* add 2 bytes to align IP on 16B boundary? */
  int maxheadroom = sizeof(struct ethhdr) + sizeof(struct iphdr)   + sizeof(struct udphdr); // test april16
  static __be16 ipid=1;
#ifdef PEXORNET_UDP_CSUM
  __wsum csum;
  unsigned short len;
#endif
  /*
   * The dma buffer pkt has been retrieved from the transmission
   * medium. Build an skb around it, so upper layers can handle it
   */
  skb = dev_alloc_skb (pkt->used_size + sizeof(struct pexornet_data_header)+ maxheadroom);
  if (!skb)
  {
    if (printk_ratelimit())
    printk(KERN_NOTICE "pexornet rx: low on mem - packet dropped\n");
    priv->stats.rx_dropped++;
    goto out;
  }

  skb_reserve (skb, maxheadroom);    // reserve front of buffer for headers


  /** first fill user data after headers:*/


  /** here we have to place user payload header with
    * trigger number etc.  For the moment we use own structure.
    * Later probably directly write mbs header? or hadtu header for existing trbnet clients?*/

// need to add data with internal checksumming:
  dathead.trigger  = pkt->trigger_status;
  dathead.datalen = pkt->used_size;
#ifdef PEXORNET_UDP_CSUM
  skb_add_data(skb,(char*) &dathead, sizeof(struct pexornet_data_header)); // internally does memcopy, skb_put and checksumming
#else
  memcpy (skb_put (skb, sizeof(struct pexornet_data_header)), (unsigned char*) &dathead, sizeof(struct pexornet_data_header));
#endif
  datacnt += sizeof(struct pexornet_data_header);

   /** rest of skb is filled with actual dma payload:*/

#ifdef PEXORNET_UDP_CSUM
  // JAM again we try function that internally provides checksum:
  skb_add_data(skb,(char*) pkt->kernel_addr, pkt->used_size); // internally does memcopy, skb_put and checksumming
  //! beware: this does a copy from user (may sleep) and we are inside ir tasklet?
  // we see kernel dump: BUG: scheduling while atomic: swapper/2/0/0x10000100
#else
  memcpy (skb_put (skb, pkt->used_size), (unsigned char*) pkt->kernel_addr, pkt->used_size);
#endif
  datacnt += pkt->used_size;
   /** Now prepare the layered headers in the headroom section:*/

  // JAM 2016: we first have to construct udp header (goes from inside to outside of buffer)
  // must be pushed here:
  udph = (struct udphdr *) skb_push (skb, sizeof(struct udphdr));
  skb_reset_transport_header(skb); // assign current data position for transport (udp) header
  udph->source=htons(0);// use same port for source and dest? htons(0); // udp port number
  udph->dest=htons(PEXORNET_RECVPORT);
  udph->len= htons(sizeof(struct udphdr) + sizeof(struct pexornet_data_header) + pkt->used_size);
  udph->check=0; // by default no checksum used






  // add ip header:
  iph = (struct iphdr *) skb_push (skb, sizeof(struct iphdr)); //
  skb_reset_network_header (skb);    // assign current cursor position as network (ip) header
  iph->saddr = htonl (priv->send_host); /* set pseudo remote data sender*/
  iph->daddr = htonl (priv->recv_host); /* set our local receiver host address*/
  iph->ihl = 5;    // unit 4 bytes
  iph->ttl = 64; // hop count (time to live) probably should not be zero
  iph->frag_off = htons((1 << 14)); // set DF bit in fragment offset field
//  A three-bit field follows and is used to control or identify fragments. They are (in order, from high order to low order):
//
//          bit 0: Reserved; must be zero.[note 1]
//          bit 1: Don't Fragment (DF)
//          bit 2: More Fragments (MF)



  iph->version = 4;
  iph->tot_len = htons(sizeof(struct udphdr) + sizeof(struct iphdr) + sizeof(struct pexornet_data_header) + pkt->used_size);

  iph->protocol = IPPROTO_UDP;// IPPROTO_RAW; //IPPROTO_UDP

  iph->id=htons(ipid++); /* always increment non zero id, does this prevent our packets being dropped?*/
  iph->check = 0; /* rebuild the checksum (ip needs it) */
  iph->check = ip_fast_csum ((unsigned char *) iph, iph->ihl);

#ifdef PEXORNET_UDP_CSUM
  //try to deliver udp checksum, to be sure:
  // JAM stolen from  net/ipv4/udp.c :
   csum = udp_csum(skb); // this is checksum of udp header and data payload of socket buffer (from skb_add_data above)
   len = skb->len - skb_transport_offset(skb); // JAM that is why we first have to fill the payload
   //len=ntohs(udph->len); // is the same value as previous line (wireshark)
  /* add protocol-dependent pseudo-header */
   udph->check = csum_tcpudp_magic( iph->saddr, iph->daddr, len,
       iph->protocol, csum);
   pexornet_dbg(
            KERN_NOTICE "pexornet_rx udp check: 0x%x for length 0x%x, csum:0x%x",udph->check, len, csum);
#endif
  /* just call socket buffer header function explicitely thus emulating what would have been done on a virtual sender: */
  pexornet_header (skb, dev, ETH_P_IP, dev->dev_addr, dev->dev_addr, 0);
  skb_reset_mac_header (skb);    // remember eth header location in buffer
  eth = (struct ethhdr*) skb_mac_header (skb);

  eth->h_source[ETH_ALEN - 1] ^= 0x01; /* emulate virtual remote source by setting to us xor 1. adapted from snull example */

  skb->dev = dev;
  skb->protocol = eth_type_trans (skb, dev);    // this will shift data cursor to location after ethheader
  skb->ip_summed =CHECKSUM_UNNECESSARY; /* don't check it */







  priv->stats.rx_packets++;
  priv->stats.rx_bytes += datacnt;

  //    if (printk_ratelimit())
  pexornet_dbg(
      KERN_NOTICE "pexornet_rx received packet of size %ld, trigtyp:0x%x si:0x%x mis:0x%x lec:0x%x di:0x%x tdt:0x%x eon:0x%x \n",pkt->used_size,
      dathead.trigger.typ, dathead.trigger.si, dathead.trigger.mis, dathead.trigger.lec, dathead.trigger.di, dathead.trigger.tdt, dathead.trigger.eon);

  pexornet_dbg(
       KERN_NOTICE "pexornet_rx skb dump: head:0x%x data:0x%x tail:0x%x end:0x%x ethhdr:0x%x iphdr:0x%x udphdr:0x%x maxheadroom:0x%x packet type:0x%x\n",
       skb->head, skb->data, skb->tail, skb->end, skb_mac_header (skb), skb_network_header(skb), skb_transport_header(skb), maxheadroom, skb->pkt_type);
  pexornet_dbg(
          KERN_NOTICE "pexornet_rx udp dump: srcport:%d destport:%d len:%d\n",
          ntohs(udph->source), ntohs(udph->dest), ntohs(udph->len));

  pexornet_dbg(
        KERN_NOTICE "pexornet_rx ip dump: srcadd:0x%x destadd:0x%x proto:0x%x, totlen:%d\n",
        ntohl(iph->saddr), ntohl(iph->daddr), iph->protocol, ntohs(iph->tot_len));
  pexornet_dbg(
          KERN_NOTICE "pexornet_rx eth dump: protocol:0x%x ",
          ntohs(eth->h_proto));

  pexornet_dbg(KERN_NOTICE "destination:");
  for(i=0;i<ETH_ALEN;++i) pexornet_dbg(KERN_NOTICE "%d.",(unsigned int) ((char*)eth->h_dest)[i]);
  pexornet_dbg(KERN_NOTICE "\n");
  pexornet_dbg(KERN_NOTICE "source:");
  for(i=0;i<ETH_ALEN;++i) pexornet_dbg(KERN_NOTICE "%d.",(unsigned int) ((char*)eth->h_source)[i]);
  pexornet_dbg(KERN_NOTICE "\n");


  /**finally some dump of the last words:*/
#ifdef PEXORNET_DEBUGPRINT
  pexornet_dbg(KERN_NOTICE "DMA buffer end contents:\n");
  dmabufbase=(unsigned int*)pkt->kernel_addr;
  dmabufend=(unsigned int*)(pkt->kernel_addr+pkt->used_size);
  for(i=0, cursor=dmabufend;i<8;++i, cursor--) {
    pexornet_dbg(KERN_NOTICE "0x%lx:0x%x \t",(unsigned long) cursor, *cursor);
  }
#endif

  rev = netif_rx (skb);
//    if (printk_ratelimit())
 pexornet_dbg(KERN_NOTICE "pexornet_rx: netif_rx has return value %d !!!\n",rev);

  out: return;
}


/*
 * Transmit a packet (called by the kernel)
 * JAM this is not necessary for pexornet, we only receive on trigger interrupt
 */
int pexornet_tx(struct sk_buff *skb, struct net_device *dev)
{
  struct pexornet_privdata *priv = pexornet_get_privdata(dev);

  pexornet_dbg(KERN_WARNING "pexornet_tx is called.");

  priv->stats.tx_packets++;
  priv->stats.tx_bytes += skb->len;
  dev_kfree_skb(skb);

  // pexornet may never send anything. This is just to test statistics when pinging

//    int len;
//    char *data, shortpkt[ETH_ZLEN];
//    struct snull_priv *priv = netdev_priv(dev);
//
//    data = skb->data;
//    len = skb->len;
//    if (len < ETH_ZLEN) {
//        memset(shortpkt, 0, ETH_ZLEN);
//        memcpy(shortpkt, skb->data, skb->len);
//        len = ETH_ZLEN;
//        data = shortpkt;
//    }
//    dev->trans_start = jiffies; /* save the timestamp */
//
//    /* Remember the skb, so we can free it at interrupt time */
//    priv->skb = skb;
//
//    /* actual deliver of data is device-specific, and not shown here */
//    snull_hw_tx(data, len, dev);

    return 0; /* Our simple device can not fail */
}




static void pexornet_ethtool_get_drvinfo(struct net_device *dev,
                                        struct ethtool_drvinfo *drvinfo)
{
        printk (KERN_NOTICE "pexornet_ethtool_get_drvinfo...\n");
        strlcpy(drvinfo->driver, "pexornet", sizeof(drvinfo->driver));
        strlcpy(drvinfo->version, "0.042", sizeof(drvinfo->version));
}

static int pexornet_ethtool_get_settings(struct net_device *dev,
                                        struct ethtool_cmd *cmd)
{
        //const struct snull_priv *priv = netdev_priv(dev);
        //printk (KERN_NOTICE "snull_ethtool_get_settings...\n");

        // JAM examples ripped from mellanox:
        cmd->autoneg = AUTONEG_DISABLE;
        cmd->supported = SUPPORTED_10000baseT_Full;
        cmd->advertising = ADVERTISED_10000baseT_Full;
        cmd->duplex = DUPLEX_FULL;
        cmd->port = PORT_FIBRE;
        cmd->speed = SPEED_1000;
        cmd->transceiver = XCVR_INTERNAL;
        cmd->supported |= SUPPORTED_FIBRE;
        cmd->advertising |= ADVERTISED_FIBRE;

        return 0;
}


static int pexornet_ethtool_set_settings(struct net_device *dev,
                                        struct ethtool_cmd *cmd)
{
        //const struct snull_priv *priv = netdev_priv(dev);
        printk (KERN_NOTICE "snull_ethtool_set_settings...\n");

        // JAM examples ripped from mellanox:
//         cmd->autoneg = AUTONEG_DISABLE;
//         cmd->supported = SUPPORTED_10000baseT_Full;
//         cmd->advertising = ADVERTISED_10000baseT_Full;
//         cmd->duplex = DUPLEX_FULL;
//         cmd->port = PORT_FIBRE;
//         cmd->transceiver = XCVR_EXTERNAL;
//         cmd->supported |= SUPPORTED_FIBRE;
//         cmd->advertising |= ADVERTISED_FIBRE;

        return 0;
}



/** JAM: these are necessary for kernel 3:*/

static const struct ethtool_ops pexornet_ethtool_ops = {
        .get_link       = ethtool_op_get_link,
        .get_settings   = pexornet_ethtool_get_settings,
        .get_drvinfo    = pexornet_ethtool_get_drvinfo,
        .set_settings   = pexornet_ethtool_set_settings,
};


static const struct header_ops pexornet_header_ops = {
        .create       = pexornet_header, //eth_header
        .rebuild      = pexornet_rebuild_header, //eth_rebuild_header,
        .parse        = eth_header_parse,
        .cache        = NULL, //eth_header_cache,
        .cache_update  = NULL, //eth_header_cache_update,
};


static const struct net_device_ops pexornet_netdev_ops = {

        .ndo_open               = pexornet_open,
        .ndo_stop              = pexornet_release,
        .ndo_set_config        = pexornet_config,
        .ndo_start_xmit        = pexornet_tx,
        .ndo_do_ioctl          = pexornet_ioctl,
        .ndo_get_stats         = pexornet_stats,
        .ndo_change_mtu        = pexornet_change_mtu,
       // .ndo_tx_timeout        = pexornet_tx_timeout,

//          if (use_napi) {
//          .ndo_poll        =  snull_poll;
//          }



};







/*
 * The init function (sometimes called probe).
 * It is invoked by register_netdev()
 */
void pexornet_init_netdev(struct net_device *dev)
{

      pexornet_msg(KERN_NOTICE "PEXORNET init_netdev..\n");
        /*
     * Then, assign other fields in dev, using ether_setup() and some
     * hand assignments
     */
    ether_setup(dev); /* assign some of the fields */
    dev->netdev_ops = &pexornet_netdev_ops;
    dev->header_ops = &pexornet_header_ops;
    dev->ethtool_ops = &pexornet_ethtool_ops;



    dev->watchdog_timeo = timeout;
    /* keep the default flags, just add NOARP */
    dev->flags           |= IFF_NOARP;

    //dev->features        |= NETIF_F_NO_CSUM;

    dev->features      = dev->features & ~(NETIF_F_ALL_CSUM); // for kernel 3 JAM

     //   dev->hard_header_cache = NULL;      /* Disable caching */

}














static int pexornet_probe (struct pci_dev *dev, const struct pci_device_id *id)
{
  int err = 0, ix = 0;
  u16 vid = 0, did = 0;
  char devnameformat[64];
//  char devname[64];
 // struct pexornet_trigger_buf* trigstat;
#ifdef PEXORNET_ENABLE_IRQ
  unsigned char irpin = 0, irline = 0, irnumbercount = 0;
#ifdef PEXORNET_WITH_TRIXOR
  int irtype = 0;
#endif
#endif
  struct pexornet_privdata *privdata;
  struct pexornet_netdev_privdata *netpriv;
  pexornet_msg(KERN_NOTICE "PEXORNET pci driver starts probe...\n");
  if ((err = pci_enable_device (dev)) != 0)
  {
    pexornet_msg(KERN_ERR "PEXORNET pci driver probe: Error %d enabling PCI device! \n",err);
    return -ENODEV;
  }pexornet_dbg(KERN_NOTICE "PEXORNET Device is enabled.\n");

  /* Set Memory-Write-Invalidate support */
  if (!pci_set_mwi (dev))
  {
    pexornet_dbg(KERN_NOTICE "MWI enabled.\n");
  }
  else
  {
    pexornet_dbg(KERN_NOTICE "MWI not supported.\n");
  }
  pci_set_master (dev);
  test_pci (dev);

  /* Allocate and initialize the private data for this device */
  privdata = kmalloc (sizeof(struct pexornet_privdata), GFP_KERNEL);
  if (privdata == NULL )
  {
    pexornet_cleanup_device (privdata);
    return -ENOMEM;
  }
  memset (privdata, 0, sizeof(struct pexornet_privdata));
  pci_set_drvdata (dev, privdata);
  privdata->pdev = dev;
  
  privdata->wait_timeout = PEXORNET_WAIT_TIMEOUT;
  // default values
  // here check which board we have: pexornet, pexaria, kinpex
  pci_read_config_word (dev, PCI_VENDOR_ID, &vid);
  pexornet_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
  pci_read_config_word (dev, PCI_DEVICE_ID, &did);
  pexornet_dbg(KERN_NOTICE "  device id:........0x%x \n", did);
  if (vid == PEXOR_VENDOR_ID && did == PEXOR_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_PEXOR;
    strncpy (devnameformat, PEXORNAMEFMT, 32);
    pexornet_msg(KERN_NOTICE "  Found board type PEXOR, vendor id: 0x%x, device id:0x%x\n",vid,did);
  }
  else if (vid == PEXARIA_VENDOR_ID && did == PEXARIA_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_PEXARIA;
    strncpy (devnameformat, PEXARIANAMEFMT, 32);
    pexornet_msg(KERN_NOTICE "  Found board type PEXARIA, vendor id: 0x%x, device id:0x%x\n",vid,did);

  }
  else if (vid == KINPEX_VENDOR_ID && did == KINPEX_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_KINPEX;
    strncpy (devnameformat, KINPEXNAMEFMT, 32);
    pexornet_msg(KERN_NOTICE "  Found board type KINPEX, vendor id: 0x%x, device id:0x%x\n",vid,did);
  }
  else
  {
    privdata->board_type = BOARDTYPE_PEXOR;
    strncpy (devnameformat, PEXORNAMEFMT, 32);
    pexornet_msg(KERN_NOTICE "  Unknown board type, vendor id: 0x%x, device id:0x%x. Assuming pexornet mode...\n",vid,did);
  }


  atomic_set(&(privdata->state), PEXORNET_STATE_STOPPED);

  for (ix = 0; ix < 6; ++ix)
  {
    privdata->bases[ix] = pci_resource_start (dev, ix);
    privdata->reglen[ix] = pci_resource_len (dev, ix);
    if (privdata->bases[ix] == 0)
      continue;
    if ((pci_resource_flags (dev, ix) & IORESOURCE_IO))
    {

      pexornet_dbg(KERN_NOTICE " - Requesting io ports for bar %d\n", ix);
      if (request_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        pexornet_dbg(KERN_ERR "I/O address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        pexornet_cleanup_device (privdata);
        return -EIO;
      }pexornet_dbg( "requested ioport at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
    }
    else if ((pci_resource_flags (dev, ix) & IORESOURCE_MEM))
    {
      pexornet_dbg(KERN_NOTICE " - Requesting memory region for bar %d\n", ix);
      if (request_mem_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        pexornet_dbg(KERN_ERR "Memory address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        pexornet_cleanup_device (privdata);
        return -EIO;
      }pexornet_dbg( "requested memory at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
      privdata->iomem[ix] = ioremap_nocache (privdata->bases[ix], privdata->reglen[ix]);
      if (privdata->iomem[ix] == NULL )
      {
        pexornet_dbg(KERN_ERR "Could not remap memory  at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        pexornet_cleanup_device (privdata);
        return -EIO;
      }pexornet_dbg(
          "remapped memory to %lx with length %lx\n", (unsigned long) privdata->iomem[ix], privdata->reglen[ix]);
    }
  }    //for
  
   // initialize maximum polls value:
  privdata->sfp_maxpolls=PEXORNET_SFP_MAXPOLLS;
  
  // IP address default values for virtual data sender and receiving host:
  privdata->send_host= PEXORNET_SENDHOST;
  privdata->recv_host= PEXORNET_RECVHOST;

  set_pexornet (&(privdata->registers), privdata->iomem[0], privdata->bases[0]);
  print_pexornet (&(privdata->registers));
  
#ifdef PEXORNET_ENABLE_IRQ
  /* reset pexornet ir registers if still active from previous crash...*/

#ifdef PEXORNET_WITH_TRIXOR
  irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
  iowrite32 (irtype, privdata->registers.irq_status);
#else
  iowrite32(0, privdata->registers.irq_control);
#endif
  mb();
  ndelay(20);

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
  init_MUTEX (&(privdata->ioctl_sem));
#else
  sema_init (&(privdata->ioctl_sem), 1);
#endif

  /* TODO may use rw semaphore instead? init_rwsem(struct rw_semaphore *sem); */

  spin_lock_init(&(privdata->gosip_lock));

  spin_lock_init(&(privdata->buffers_lock));
  INIT_LIST_HEAD (&(privdata->free_buffers));
  INIT_LIST_HEAD (&(privdata->received_buffers));
  INIT_LIST_HEAD (&(privdata->used_buffers));

  /* the interrupt related stuff:*/
  atomic_set(&(privdata->irq_count), 0);
  //init_waitqueue_head (&(privdata->irq_dma_queue));
  //atomic_set(&(privdata->dma_outstanding), 0);

  atomic_set(&(privdata->trigstat), 0);

  atomic_set(&(privdata->bufid), 0);

  tasklet_init (&(privdata->irq_bottomhalf), pexornet_irq_tasklet, (unsigned long) privdata);




  /*******************************************************/
  /** interrupt related inits:****************************/


#ifdef IRQ_ENABLE_MSI
  /* here try to activate MSI ?*/
  if ((err=pci_enable_msi(dev)) == 0 )
  {
    pexornet_dbg(KERN_NOTICE "MSI enabled.\n");
  }
  else
  {
    pexornet_dbg(KERN_NOTICE "Failed activating MSI with error %d\n",err);
  }

#endif

#ifdef PEXORNET_ENABLE_IRQ
  /* debug: do we have valid ir pins/lines here?*/
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_PIN, &irpin)) != 0)
  {
    pexornet_msg(KERN_ERR "PEXORNET pci driver probe: Error %d getting the PCI interrupt pin \n",err);
  }
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_LINE, &irline)) != 0)
  {
    pexornet_msg(KERN_ERR "PEXORNET pci driver probe: Error %d getting the PCI interrupt line.\n",err);
  }
  snprintf (privdata->irqname, 64, devnameformat, atomic_read(&pexornet_numdevs));


#ifdef PEXORNET_SHARED_IRQ
  if (request_irq (dev->irq, pexornet_isr, IRQF_SHARED, privdata->irqname, privdata))
  {
    pexornet_msg( KERN_ERR "PEXORNET pci_drv: IRQ %d not free.\n", dev->irq );
    irnumbercount = 1; /* suppress warnings from unused variable here*/
    pexornet_cleanup_device (privdata);
    return -EIO;
  }

#else

  dev->irq=irline;
  while(request_irq(dev->irq, pexornet_isr , 0 , privdata->irqname, privdata))
  {
    pexornet_msg( KERN_ERR "PEXORNET pci_drv: IRQ %d not free. try next...\n", dev->irq++ );
    if(irnumbercount++ > 100)
    {
      pexornet_msg( KERN_ERR "PEXORNET pci_drv: tried to get ir more than %d times, aborting\n", irnumbercount )
          pexornet_cleanup_device(privdata);
      return -EIO;
    }
  }

#endif

  pexornet_msg(KERN_NOTICE " assigned IRQ %d for name %s, pin:%d, line:%d \n",dev->irq, privdata->irqname,irpin,irline);

#endif

  /*******************************************************/
  /** register network device:****************************/
  privdata->net_dev = alloc_netdev(sizeof(struct pexornet_privdata), "pex%d",
                pexornet_init_netdev);
   if (privdata->net_dev == NULL )
     {
         pexornet_msg(KERN_ALERT "Network device pexor could not register!\n");
         pexornet_cleanup_device(privdata);
         return -EIO;
     }
   netpriv = netdev_priv(privdata->net_dev);
   netpriv->pci_privdata=privdata; // backlink to our privdata field
   err = register_netdev(privdata->net_dev);
  if (err)
      {
            pexornet_msg(KERN_ALERT" error %i registering device \"%s\"\n",
                        err, privdata->net_dev->name);
            pexornet_cleanup_device(privdata);
            return -EIO;
       }
  pexornet_msg(KERN_NOTICE "pexornet_probe has registered network device \"%s\"\n", privdata->net_dev->name);
  /*******************************************************/
  /** sysfs entries: *************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#ifdef PEXORNET_SYSFS_ENABLE
  // we create sysfs attributes directly under the link of the network device in class /sys/class/net
  privdata->class_dev=&(privdata->net_dev->dev);
  dev_set_drvdata (privdata->class_dev, privdata);

    if (device_create_file (privdata->class_dev, &dev_attr_freebufs) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for free buffers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_usedbufs) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for used buffers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_rcvbufs) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for receive buffers.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_codeversion) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for code version.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_dmaregs) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for dma registers.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_trixorregs) != 0)
    {
          pexornet_msg(KERN_ERR "Could not add device file node for trixor registers.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_trixorfcti) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for trixor fast clear time.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_trixorcvti) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for trixor conversion time\n");
    }




#ifdef PEXORNET_WITH_SFP
    if (device_create_file (privdata->class_dev, &dev_attr_sfpregs) != 0)
    {
      pexornet_msg(KERN_ERR "Could not add device file node for sfp registers.\n");
    }
    
    if (device_create_file (privdata->class_dev, &dev_attr_gosipretries) != 0)
        {
             pexornet_msg(KERN_ERR "Could not add device file node for gosip retries.\n");
        }
    if (device_create_file (privdata->class_dev, &dev_attr_gosipbuswait) != 0)
      {
        pexornet_msg(KERN_ERR "Could not add device file node for gosip bus wait.\n");
      }
#endif // with sfp
#endif // PEXORNET_SYSFS_ENABLE
#endif // LINUX_VERSION_CODE

  atomic_inc(&pexornet_numdevs); // count our devices for interrupt naming TODO: replace
  pexornet_msg(KERN_NOTICE "probe has finished.\n");
  return 0;
}


static void pexornet_remove (struct pci_dev *dev)
{
  struct pexornet_privdata* priv = (struct pexornet_privdata*) pci_get_drvdata (dev);
  pexornet_cleanup_device (priv);
  pexornet_msg(KERN_NOTICE "PEXORNET pci driver end remove.\n");
}

static struct pci_driver pci_driver = { .name = PEXORNETNAME, .id_table = ids, .probe = pexornet_probe, .remove = pexornet_remove, };

static int __init pexornet_init (void)
{
  pexornet_msg(KERN_NOTICE "pexornet driver init...\n");
  if (pci_register_driver (&pci_driver) < 0)
  {
    pexornet_msg(KERN_ALERT "pci driver could not register!\n");
    return -EIO;
  }
  pexornet_msg(KERN_NOTICE "\t\tpexornet driver init done.\n");
  return 0;
  /* note: actual assignment will be done on pci probe time*/
}

static void __exit pexornet_exit (void)
{
  pexornet_msg(KERN_NOTICE "pexornet driver exit...\n");

  pci_unregister_driver (&pci_driver);
  pexornet_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joern Adamczewski-Musch");

module_init(pexornet_init);
module_exit(pexornet_exit);
