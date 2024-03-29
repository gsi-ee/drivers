#include "pexor_base.h"

/** hold full device number */
static dev_t pexor_devt;

/** counts number of probed pexor devices */
/*static atomic_t pexor_numdevs;*/

static atomic_t pexor_numdevs = ATOMIC_INIT(0);

static struct pci_device_id ids[] = { { PCI_DEVICE (PEXOR_VENDOR_ID, PEXOR_DEVICE_ID), },    // classic pexor
    { PCI_DEVICE (PEXARIA_VENDOR_ID, PEXARIA_DEVICE_ID), },    //pexaria
    { PCI_DEVICE (KINPEX_VENDOR_ID, KINPEX_DEVICE_ID), },    // kinpex
    { 0, } };
MODULE_DEVICE_TABLE(pci, ids);

#ifdef PEXOR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static DEVICE_ATTR(freebufs, S_IRUGO, pexor_sysfs_freebuffers_show, NULL);
static DEVICE_ATTR(usedbufs, S_IRUGO, pexor_sysfs_usedbuffers_show, NULL);
static DEVICE_ATTR(rcvbufs, S_IRUGO, pexor_sysfs_rcvbuffers_show, NULL);
static DEVICE_ATTR(codeversion, S_IRUGO, pexor_sysfs_codeversion_show, NULL);
static DEVICE_ATTR(dmaregs, S_IRUGO, pexor_sysfs_dmaregs_show, NULL);
static DEVICE_ATTR(trixorregs, S_IRUGO, pexor_sysfs_trixorregs_show, NULL);

//static DEVICE_ATTR(trixorfcti, S_IWUGO | S_IRUGO , pexor_sysfs_trixor_fctime_show, pexor_sysfs_trixor_fctime_store);
//static DEVICE_ATTR(trixorcvti, S_IWUGO | S_IRUGO , pexor_sysfs_trixor_cvtime_show, pexor_sysfs_trixor_cvtime_store);

static DEVICE_ATTR(trixorfcti,(S_IWUSR| S_IWGRP | S_IRUGO)  , pexor_sysfs_trixor_fctime_show, pexor_sysfs_trixor_fctime_store);
static DEVICE_ATTR(trixorcvti, (S_IWUSR| S_IWGRP | S_IRUGO) , pexor_sysfs_trixor_cvtime_show, pexor_sysfs_trixor_cvtime_store);



#ifdef PEXOR_WITH_SFP
static DEVICE_ATTR(sfpregs, S_IRUGO, pexor_sysfs_sfpregs_show, NULL);
//static DEVICE_ATTR(gosipretries, S_IWUGO | S_IRUGO , pexor_sysfs_sfp_retries_show, pexor_sysfs_sfp_retries_store);
//static DEVICE_ATTR(gosipbuswait, S_IWUGO | S_IRUGO , pexor_sysfs_buswait_show, pexor_sysfs_buswait_store);


static DEVICE_ATTR(gosipretries, (S_IWUSR| S_IWGRP | S_IRUGO) , pexor_sysfs_sfp_retries_show, pexor_sysfs_sfp_retries_store);
static DEVICE_ATTR(gosipbuswait, (S_IWUSR| S_IWGRP | S_IRUGO) , pexor_sysfs_buswait_show, pexor_sysfs_buswait_store);




#endif

#endif
#endif /* SYSFS_ENABLE*/

static struct file_operations pexor_fops = { .owner = THIS_MODULE, .llseek = pexor_llseek, .read = pexor_read, .write =
    pexor_write,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
    .ioctl = pexor_ioctl,
#else
    .unlocked_ioctl = pexor_ioctl,
#endif
    .mmap = pexor_mmap, .open = pexor_open, .release = pexor_release, };

static int my_major_nr = 0;

/* we support sysfs class only for new kernels to avoid backward incompatibilities here */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* pexor_class;
#endif


/* JAM2016 the following nice thing was googled and stolen from compat.h of M.Stapelberg, 2009:
 * https://github.com/lerwys/FPGA_PCIe_drivers/blob/master/opencores_driver/src/driver/compat.h*/
/* JAM 11-2023: rearranged this for most recent kernel developments: */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,0,0)
/* JAM 2023 -in v6.., transformation ongoing for memory folios*/
	#define compat_lock_page(X)   folio_lock_killable(page_folio(X))
	#define compat_unlock_page  unlock_page
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4,5,0)
/* JAM 2018 -in v4.5., lock_page_killable and unlock_page was introduced */
	#define compat_lock_page    lock_page_killable
	#define compat_unlock_page  unlock_page
/* in v2.6.28, __set_page_locked and __clear_page_locked was introduced */
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,28)
	#define compat_lock_page __set_page_locked
	#define compat_unlock_page __clear_page_locked
#elif LINUX_VERSION_CODE == KERNEL_VERSION(2,6,27)
        /* However, in v2.6.27 itself, neither of them is there, so
         * we need to use our own function fiddling with bits inside
         * the page struct :-\ */
        static inline void compat_lock_page(struct page *page) {
            __set_bit(PG_locked, &page->flags);
        }

        static inline void compat_unlock_page(struct page *page) {
            __clear_bit(PG_locked, &page->flags);
        }
#else
        /* SetPageLocked disappeared in v2.6.27 */
        //#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
	#define compat_lock_page SetPageLocked
	#define compat_unlock_page ClearPageLocked

#endif
/** End M.Stapelbergs helper*/




int pexor_open (struct inode *inode, struct file *filp)
{
  struct pexor_privdata *privdata;
  pexor_dbg(KERN_NOTICE "** starting pexor_open...\n");
  /* Set the private data area for the file */

  privdata = container_of( inode->i_cdev, struct pexor_privdata, cdev);
  filp->private_data = privdata;
  return 0;

}

int pexor_release (struct inode *inode, struct file *filp)
{
  pexor_dbg(KERN_NOTICE "** starting pexor_release...\n");
  return 0;
}

loff_t pexor_llseek (struct file *filp, loff_t off, int whence)
{
  loff_t newpos;
  /* set cursor in mapped board RAM for read/write*/
  pexor_dbg(KERN_NOTICE "** starting pexor_llseek ...\n");
  /* may use struct scull_dev *dev = filp->private_data; */
  switch (whence)
  {
    case 0: /* SEEK_SET */
      newpos = off;
      break;

    case 1: /* SEEK_CUR */
      newpos = filp->f_pos + off;
      break;

    case 2: /* SEEK_END */
      newpos = PEXOR_RAMSIZE + off;
      break;

    default: /* can't happen */
      return -EINVAL;
  }
  if (newpos < 0)
    return -EINVAL;
  filp->f_pos = newpos;
  return newpos;

  return 0;
}

ssize_t pexor_read (struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
  /* here we read from mapped pexor memory into user buffer*/
  int i;
  ssize_t retval = 0;
  struct pexor_privdata *privdata;
  void* memstart;
  int lcount = count >> 2;
  u32* kbuf = 0;
  /*  u32 kbuf[lcount];*/
  pexor_dbg(KERN_NOTICE "** starting pexor_read for f_pos=%d count=%d\n", (int) *f_pos, (int) count);
  privdata = get_privdata (filp);
  if (!privdata)
    return -EFAULT;

  if (down_interruptible (&privdata->ramsem))
    return -ERESTARTSYS;
  if (*f_pos >= PEXOR_RAMSIZE)
    goto out;
  kbuf = (u32*) kmalloc (count, GFP_KERNEL);
  if (!kbuf)
  {
    pexor_msg(KERN_ERR "pexor_read: could not alloc %d buffer space! \n",lcount);
    retval = -ENOMEM;
    goto out;
  }
  if (*f_pos + count > PEXOR_RAMSIZE)
  {
    /* TODO: better return error to inform user we exceed ram size?*/
    count = PEXOR_RAMSIZE - *f_pos;
    lcount = count >> 2;
    pexor_dbg(KERN_NOTICE "** pexor_read truncates count to =%d\n", (int) count);
  }
  memstart = (void*) (privdata->pexor.ram_start) + *f_pos;
  /* try to use intermediate kernel buffer here:*/
  pexor_dbg(KERN_NOTICE "** pexor_read begins io loop at memstart=%lx\n", (long) memstart);
  /*wmb();
   memcpy_fromio(&kbuf, memstart, count);*/mb();
   for (i = 0; i < lcount; ++i)
   {
     /*pexor_dbg(KERN_NOTICE "%x from %lx..", i,memstart+(i<<2));*/
     /*pexor_dbg(KERN_NOTICE "%d ..", i);
     if((i%10)==0) pexor_msg(KERN_NOTICE "\n");
     mb();*/
     kbuf[i] = ioread32 (memstart + (i << 2));
     pexor_bus_delay();
     /*udelay(1);*/
   }

   pexor_dbg(KERN_NOTICE "** pexor_read begins copy to user from stack buffer=%lx\n", (long) kbuf);
   if (copy_to_user (buf, kbuf, count))
   {
     pexor_dbg(KERN_ERR "** pexor_read copytouser error!\n");
     retval = -EFAULT;
     goto out;
   }
   *f_pos += count;
   retval = count;
   out: kfree (kbuf);
   up (&privdata->ramsem);
   return retval;

}

ssize_t pexor_write (struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
  int i;
  ssize_t retval = -ENOMEM; /* value used in "goto out" statements */
  struct pexor_privdata *privdata;
  void* memstart;
  int lcount = count >> 2;
  u32* kbuf = 0;
  /*u32 kbuf[lcount];*/
  pexor_dbg(KERN_NOTICE "** starting pexor_write for f_pos=%d count=%d\n", (int) *f_pos, (int) count);
  privdata = get_privdata (filp);
  if (!privdata)
    return -EFAULT;
  if (down_interruptible (&privdata->ramsem))
    return -ERESTARTSYS;
  if (*f_pos >= PEXOR_RAMSIZE)
    goto out;
  kbuf = (u32*) kmalloc (count, GFP_KERNEL);
  if (!kbuf)
  {
    pexor_msg(KERN_ERR "pexor_write: could not alloc %d buffer space! \n",lcount);
    retval = -ENOMEM;
    goto out;
  }

  if (*f_pos + count >= PEXOR_RAMSIZE)
  {
    /* TODO: better return error to inform user we exceed ram size?*/
    count = PEXOR_RAMSIZE - *f_pos;
    lcount = count >> 2;
    pexor_dbg(KERN_NOTICE "** pexor_write truncates count to =%d\n", (int) count);
  }
  memstart = (void*) (privdata->pexor.ram_start) + *f_pos;
  pexor_dbg(KERN_NOTICE "** pexor_write begins copy to user at stack buffer=%lx\n", (long) kbuf);
  mb();
  if (copy_from_user (kbuf, buf, count))
  {
    retval = -EFAULT;
    goto out;
  }pexor_dbg(KERN_NOTICE "** pexor_write begins copy loop at memstart=%lx\n", (long) memstart);
  /*memcpy_toio(memstart, kbuf , count);*/mb();
  for (i = 0; i < lcount; ++i)
  {

    /*pexor_dbg(KERN_NOTICE "kbuf[%d]=%x to %lx..", i,kbuf[i],memstart+(i<<2));*/
    /*pexor_dbg(KERN_NOTICE "%d..", i);
     if((i%10)==0) pexor_msg(KERN_NOTICE "\n");*/
    iowrite32 (kbuf[i], memstart + (i << 2));
    mb();
    ndelay(20);
  }
  *f_pos += count;
  retval = count;
  out: kfree (kbuf);
  up (&privdata->ramsem);
  return retval;

}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int pexor_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
long pexor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif
{
  int retval = 0;
  struct pexor_privdata *privdata;
  /* here validity check for magic number etc.*/

  privdata= get_privdata(filp);
  if(!privdata) return -EFAULT;

  /* use semaphore to allow multi user mode:*/
  if (down_interruptible(&(privdata->ioctl_sem)))
  {
    pexor_msg((KERN_INFO "down interruptible of ioctl sem is not zero, restartsys!\n"));
    return -ERESTARTSYS;
  }

  /* Select the appropiate command */
  switch (cmd)
  {

    /* first all common ioctls:*/

    case PEXOR_IOC_RESET:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl reset\n");
      retval = pexor_ioctl_reset(privdata,arg);
      break;

    case PEXOR_IOC_FREEBUFFER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl free buffer\n");
      retval = pexor_ioctl_freebuffer(privdata, arg);
      break;

    case PEXOR_IOC_DELBUFFER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl delete buffer\n");
      retval = pexor_ioctl_deletebuffer(privdata, arg);
      break;

    case PEXOR_IOC_WAITBUFFER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl waitbuffer\n");
      up(&privdata->ioctl_sem); /* do not lock ioctl during wait*/
      return pexor_ioctl_waitreceive(privdata, arg);
      break;

    case PEXOR_IOC_USEBUFFER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl usebuffer\n");
      retval = pexor_ioctl_usebuffer(privdata, arg);
      break;

    case PEXOR_IOC_MAPBUFFER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl mapbuffer\n");
      retval = pexor_ioctl_mapbuffer(privdata, arg);
      break;

    case PEXOR_IOC_UNMAPBUFFER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl unmapbuffer\n");
      retval = pexor_ioctl_unmapbuffer(privdata, arg);
      break;

    case PEXOR_IOC_CLEAR_RCV_BUFFERS:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl clear receive buffers\n");
      retval = pexor_ioctl_clearreceivebuffers(privdata, arg);
      break;

    case PEXOR_IOC_SETSTATE:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl set\n");
      retval = pexor_ioctl_setrunstate(privdata, arg);
      break;

    case PEXOR_IOC_TEST:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl test\n");
      retval = pexor_ioctl_test(privdata, arg);
      break;

    case PEXOR_IOC_WRITE_REGISTER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl write register\n");
      retval = pexor_ioctl_write_register(privdata, arg);
      break;

    case PEXOR_IOC_READ_REGISTER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl read register\n");
      retval = pexor_ioctl_read_register(privdata, arg);
      break;

    case PEXOR_IOC_WRITE_BUS:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl write bus\n");
#ifdef PEXOR_TRIGGERLESS_SEMAPHORE
      up(&privdata->ioctl_sem); /* do not lock ioctl here, otherwise deadlock with sfp semaphores possible!*/
#endif
      retval = pexor_ioctl_write_bus(privdata, arg);
      break;

    case PEXOR_IOC_READ_BUS:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl read bus\n");
#ifdef PEXOR_TRIGGERLESS_SEMAPHORE
      up(&privdata->ioctl_sem); /* do not lock ioctl here, otherwise deadlock with sfp semaphores possible!*/
#endif
      retval = pexor_ioctl_read_bus(privdata, arg);
      break;

    case PEXOR_IOC_INIT_BUS:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl init bus\n");
      retval = pexor_ioctl_init_bus(privdata, arg);
      break;

    case PEXOR_IOC_CONFIG_BUS:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl config bus\n");
#ifdef PEXOR_TRIGGERLESS_SEMAPHORE
      up(&privdata->ioctl_sem); /* do not lock ioctl here, otherwise deadlock with sfp semaphores possible!*/
#endif
      retval = pexor_ioctl_configure_bus(privdata, arg);
      break;

    case PEXOR_IOC_REQUEST_TOKEN:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl request token\n");
      retval = pexor_ioctl_request_token(privdata, arg);
      break;

    case PEXOR_IOC_WAIT_TOKEN:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl wait token\n");
      retval = pexor_ioctl_wait_token(privdata, arg);
      break;

    case PEXOR_IOC_REQUEST_RECEIVE_TOKENS:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl request receive token parallel\n");
      retval = pexor_ioctl_request_receive_token_parallel(privdata, arg);
      break;

    case PEXOR_IOC_REQUEST_RECEIVE_ASYNC:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl request receive token asynchronous\n");
#ifdef PEXOR_TRIGGERLESS_SEMAPHORE
      up(&privdata->ioctl_sem); /* do not lock ioctl here, otherwise deadlock with sfp semaphores possible!*/
#endif
      retval = pexor_ioctl_request_receive_token_async(privdata, arg);
      break;

    case PEXOR_IOC_REQUEST_ASYNC_POLLING:
       pexor_dbg(KERN_NOTICE "** pexor_ioctl request asynchronous tokens with internal polling\n");
#ifdef PEXOR_TRIGGERLESS_SEMAPHORE
      up(&privdata->ioctl_sem); /* do not lock ioctl here, otherwise deadlock with sfp semaphores possible!*/
#endif
       retval = pexor_ioctl_request_token_async_polling(privdata, arg);
       break;

    case PEXOR_IOC_GET_ASYNC_BUFFER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl get async buffer a\n");
      retval = pexor_ioctl_first_usedbuffer(privdata, arg);
      break;

#ifdef PEXOR_TRIGGERLESS_WORKER
    case PEXOR_IOC_START_ASYNC_ACQ:
      pexor_msg(KERN_NOTICE "** pexor_ioctl start triggerless acquisition a\n");
      retval = pexor_ioctl_start_async_acquisition(privdata, arg);
      break;
    case PEXOR_IOC_STOP_ASYNC_ACQ:
      pexor_msg(KERN_NOTICE "** pexor_ioctl stop triggerless acquisition a\n");
      retval = pexor_ioctl_stop_async_acquisition(privdata, arg);
      break;
#endif


    case PEXOR_IOC_WAIT_TRIGGER:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl wait trigger\n");
      up(&privdata->ioctl_sem); /* do not lock ioctl during wait*/
      return pexor_ioctl_wait_trigger(privdata, arg);
      break;

#ifdef PEXOR_WITH_TRIXOR
    case PEXOR_IOC_SET_TRIXOR:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl set trixor\n");
      retval = pexor_ioctl_set_trixor(privdata, arg);
      break;

#endif

    case PEXOR_IOC_GET_SFP_LINKS:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl get sfp links\n");
      retval = pexor_ioctl_get_sfp_links(privdata, arg);
      break;

    case PEXOR_IOC_SET_WAIT_TIMEOUT:
      pexor_dbg(KERN_NOTICE "** pexor_ioctl set wait timeout\n");
      retval = pexor_ioctl_set_wait_timeout(privdata, arg);
      break;

    default:
      retval = -ENOTTY;
      break;
  }
  up(&privdata->ioctl_sem);
  return retval;
}

int pexor_ioctl_mapbuffer (struct pexor_privdata *priv, unsigned long arg)
{
  int i, res = 0;
  int nr_pages = 0;
  struct page **pages;
  struct scatterlist *sg = NULL;
  unsigned int nents;
  unsigned long count, offset, length;
  struct pexor_dmabuf* dmabuf = 0;
  struct pexor_userbuf bufdescriptor;
  res = copy_from_user (&bufdescriptor, (void __user *) arg, sizeof(struct pexor_userbuf));
  if (res)
    return res;
  if (bufdescriptor.size == 0)
    return -EINVAL;

  dmabuf = kmalloc (sizeof(struct pexor_dmabuf), GFP_KERNEL);
  if (!dmabuf)
  {
    pexor_dbg(KERN_ERR "pexor_ioctl_mapbuffer: could not alloc dma buffer descriptor! \n");
    return -ENOMEM;
  }
  memset (dmabuf, 0, sizeof(struct pexor_dmabuf));
  dmabuf->virt_addr = bufdescriptor.addr;
  dmabuf->size = bufdescriptor.size;

  /* calculate the number of pages */
  nr_pages = ((dmabuf->virt_addr & ~PAGE_MASK)+ dmabuf->size + ~PAGE_MASK)>>PAGE_SHIFT;
  pexor_dbg(KERN_NOTICE "nr_pages computed: 0x%x\n", nr_pages);

  /* Allocate space for the page information */
  if ((pages = vmalloc (nr_pages * sizeof(*pages))) == NULL )
    goto mapbuffer_descriptor;
  /* Allocate space for the scatterlist */
  if ((sg = vmalloc (nr_pages * sizeof(*sg))) == NULL )
    goto mapbuffer_pages;

  sg_init_table (sg, nr_pages);

  /* Get the page information */

  // JAM 28-06-22: adjustment for debian bullseye
  #if LINUX_VERSION_CODE < KERNEL_VERSION(5,8,0)
       down_read (&current->mm->mmap_sem);
  #else
       down_read (&current->mm->mmap_lock);
  #endif

  // port to kernel 4.9/Debian 9.0 JAM 9-2017 - 8-2018 for pexor
  // kernel 4.9.0:
  //    long get_user_pages(unsigned long start, unsigned long nr_pages,
  //             unsigned int gup_flags, struct page **pages,
  //             struct vm_area_struct **vmas)
  //
  // kernel 4.8.x :
  //     long get_user_pages(unsigned long start, unsigned long nr_pages,
  //                     int write, int force, struct page **pages,
  //                     struct vm_area_struct **vmas);


  #if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
       res = get_user_pages (current, current->mm, dmabuf->virt_addr, nr_pages, 1, 0, pages, NULL );
  #elif  LINUX_VERSION_CODE < KERNEL_VERSION(4,9,0)
       res = get_user_pages (dmabuf->virt_addr, nr_pages, 1, 0, pages, NULL );
  #elif LINUX_VERSION_CODE < KERNEL_VERSION(6,5,0)
       res = get_user_pages (dmabuf->virt_addr, nr_pages, FOLL_WRITE, pages, NULL );
  #else
       res = get_user_pages (dmabuf->virt_addr, nr_pages, FOLL_WRITE, pages);
  #endif





  // JAM 28-06-22: adjustment for debian bullseye
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,8,0)
  up_read (&current->mm->mmap_sem);
#else
  up_read (&current->mm->mmap_lock);
#endif



  /* Error, not all pages mapped */
  if (res < (int) nr_pages)
  {
    pexor_dbg(KERN_ERR "Could not map all user pages (0x%x of 0x%x)\n", res, nr_pages);
    /* If only some pages could be mapped, we release those. If a real
     * error occured, we set nr_pages to 0 */
    nr_pages = (res > 0 ? res : 0);
    goto mapbuffer_unmap;
  }

  pexor_dbg(KERN_NOTICE "Got the pages (0x%x).\n", res);

  /* populate sg list:*/
  /* page0 is different */
  if (!PageReserved (pages[0]))
    compat_lock_page(pages[0]);
   //__set_page_locked (pages[0]);
  //SetPageLocked(pages[0]);

  /* for first chunk, we take into account that memory is possibly not starting at
   * page boundary:*/
  offset = (dmabuf->virt_addr & ~PAGE_MASK);
  length = (dmabuf->size > (PAGE_SIZE - offset) ? (PAGE_SIZE - offset) : dmabuf->size);
  sg_set_page (&sg[0], pages[0], length, offset);

  count = dmabuf->size - length;
  for (i = 1; i < nr_pages; i++)
  {
    if (!PageReserved (pages[i]))
      compat_lock_page(pages[i]);
//      __set_page_locked (pages[i]);
    //SetPageLocked(pages[i]);

    sg_set_page (&sg[i], pages[i], ((count > PAGE_SIZE)? PAGE_SIZE : count), 0);
    count -= sg[i].length;
  }

  /* Use the page list to populate the SG list */
  /* SG entries may be merged, res is the number of used entries */
  /* We have originally nr_pages entries in the sg list */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
 if ((nents = dma_map_sg (&(priv->pdev->dev), sg, nr_pages, DMA_FROM_DEVICE)) == 0)
#else
 if ((nents = pci_map_sg (priv->pdev, sg, nr_pages, PCI_DMA_FROMDEVICE)) == 0)
#endif


    goto mapbuffer_unmap;

  pexor_dbg(KERN_NOTICE "Mapped SG list (0x%x entries).\n", nents);

  dmabuf->num_pages = nr_pages; /* Will be needed when unmapping */
  dmabuf->pages = pages;
  dmabuf->sg_ents = nents; /* number of coherent dma buffers to transfer*/
  dmabuf->sg = sg;

  pexor_dbg(
      KERN_ERR "pexor_ioctl_mapbuffer mapped user buffer 0x%lx, size 0x%lx, pages 0x%x to 0x%x sg entries \n", dmabuf->virt_addr, dmabuf->size, nr_pages, nents);
  spin_lock( &(priv->buffers_lock));
  /* this list contains only the unused (free) buffers: */
  list_add_tail (&(dmabuf->queue_list), &(priv->free_buffers));
  spin_unlock( &(priv->buffers_lock));

  /* DEBUG ****************************************/for_each_sg(dmabuf->sg,sg, dmabuf->sg_ents,i)
  {
    pexor_dbg(
        KERN_ERR "-- dump sg chunk %d: start 0x%x length 0x%x \n", i, (unsigned) sg_dma_address(sg), sg_dma_len(sg));
  }
  /***************************************************/

  return 0;

  mapbuffer_unmap:
  /* release pages */
  for (i = 0; i < nr_pages; i++)
  {
    if (PageLocked (pages[i]))
      compat_unlock_page(pages[i]);
//      __clear_page_locked (pages[i]);
    //ClearPageLocked(pages[i]);
    if (!PageReserved (pages[i]))
      SetPageDirty (pages[i]);

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        page_cache_release(pages[i]);
#else
        put_page(pages[i]);
#endif

  }
  vfree (sg);
  mapbuffer_pages: vfree (pages);
  mapbuffer_descriptor: kfree (dmabuf);

  return -ENOMEM;
}

int pexor_ioctl_unmapbuffer (struct pexor_privdata *priv, unsigned long arg)
{
  /* deletebuffer will check if we deal with kernel or sg-userbuffer*/
  return (pexor_ioctl_deletebuffer (priv, arg));
}

int pexor_ioctl_freebuffer (struct pexor_privdata* priv, unsigned long arg)
{
  struct pexor_dmabuf* cursor;
  int state, retval = 0;
  struct pexor_userbuf bufdescriptor;
  retval = copy_from_user (&bufdescriptor, (void __user *) arg, sizeof(struct pexor_userbuf));

  if (retval)
    return retval;
  spin_lock( &(priv->buffers_lock));
  if (list_empty (&(priv->used_buffers)))
  {
    /* this may happen if user calls free buffer without taking or receiving one before*/
    spin_unlock( &(priv->buffers_lock));
    pexor_dbg(KERN_NOTICE "** pexor_free_buffer: No more used buffers to free!\n");
    return -EFAULT;
  }
  list_for_each_entry(cursor, &(priv->used_buffers), queue_list)
  {
    if(cursor->virt_addr==bufdescriptor.addr)
    {
      pexor_dbg(KERN_NOTICE "** pexor_ioctl_freebuffer freed buffer %p\n",(void*) cursor->virt_addr);
      list_move_tail(&(cursor->queue_list) , &(priv->free_buffers));
      spin_unlock( &(priv->buffers_lock) );

      cursor->used_size=0;
      /* ? need to sync buffer for next dma */
      if(cursor->dma_addr!=0) /* kernel buffer*/
      {
//        pci_dma_sync_single_for_device( priv->pdev, cursor->dma_addr, cursor->size, PCI_DMA_FROMDEVICE );
//      else /* sg buffer*/
//        pci_dma_sync_sg_for_device( priv->pdev, cursor->sg, cursor->sg_ents,PCI_DMA_FROMDEVICE );

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_single_for_device( &(priv->pdev->dev), cursor->dma_addr, cursor->size,DMA_FROM_DEVICE );
#else
  pci_dma_sync_single_for_device (priv->pdev, cursor->dma_addr, cursor->size,PCI_DMA_FROMDEVICE );
#endif
  }
    else
    /* sg buffer*/
  {
    //pci_dma_sync_sg_for_cpu (priv->pdev, dmabuf->sg, dmabuf->sg_ents, PCI_DMA_FROMDEVICE);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_sg_for_cpu( &(priv->pdev->dev), cursor->sg, cursor->sg_ents,DMA_FROM_DEVICE );
#else
  pci_dma_sync_sg_for_cpu (priv->pdev, cursor->sg, cursor->sg_ents,PCI_DMA_FROMDEVICE );
#endif

  }
      /* trigger here again dma flow*/
      state=atomic_read(&(priv->state));
      if(state==PEXOR_STATE_DMA_SUSPENDED)
      {
        /* this state indicates that dma flow was running out of buffer. We enable it again and restart dma*/
        atomic_set(&(priv->state),PEXOR_STATE_DMA_FLOW);
        pexor_dbg(KERN_NOTICE "** pexor_ioctl_freebuffer restarts dma flow \n");
        retval=pexor_next_dma(priv, priv->pexor.ram_dma_cursor, 0, 0 ,0 ,0, 0, 0); /* set previous dma source that was tried before suspend*/
        if(retval)
        {
          atomic_set(&(priv->state),PEXOR_STATE_STOPPED);
          pexor_dbg(KERN_ALERT "** pexor_ioctl_freebuffer     NEVER COME  HERE - next dma fails although free buffers available!\n");
          return retval;
        }
      }

      return 0;
    }

  }
  spin_unlock( &(priv->buffers_lock));
  return -EFAULT;
}

int pexor_ioctl_usebuffer (struct pexor_privdata* priv, unsigned long arg)
{
  struct pexor_dmabuf* dmabuf;
  int rev = 0;
  struct pexor_userbuf userbuf;
  spin_lock( &(priv->buffers_lock));
  if (list_empty (&(priv->free_buffers)))
  {
    /* this may happen if user calls take buffer without previous mmap, or if running out of buffers*/
    spin_unlock( &(priv->buffers_lock));
    pexor_dbg(KERN_NOTICE "** pexor_use_buffer: No more free buffers to take!\n");
    return -EFAULT;
  }
  dmabuf=list_first_entry(&(priv->free_buffers), struct pexor_dmabuf, queue_list);
  list_move_tail (&(dmabuf->queue_list), &(priv->used_buffers));
  spin_unlock( &(priv->buffers_lock));
  if (dmabuf->dma_addr != 0)
  { /* kernel buffer*/
 //   pci_dma_sync_single_for_cpu (priv->pdev, dmabuf->dma_addr, dmabuf->size, PCI_DMA_FROMDEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_single_for_cpu( &(priv->pdev->dev), dmabuf->dma_addr, dmabuf->size,DMA_FROM_DEVICE );
#else
  pci_dma_sync_single_for_cpu (priv->pdev, dmabuf->dma_addr, dmabuf->size,PCI_DMA_FROMDEVICE );
#endif
  }
    else
    /* sg buffer*/
  {
    //pci_dma_sync_sg_for_cpu (priv->pdev, dmabuf->sg, dmabuf->sg_ents, PCI_DMA_FROMDEVICE);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_sg_for_cpu( &(priv->pdev->dev), dmabuf->sg, dmabuf->sg_ents,DMA_FROM_DEVICE );
#else
  pci_dma_sync_sg_for_cpu (priv->pdev, dmabuf->sg, dmabuf->sg_ents,PCI_DMA_FROMDEVICE );
#endif
  }

  userbuf.addr = dmabuf->virt_addr;
  userbuf.size = dmabuf->size;
  rev = copy_to_user ((void __user *) arg, &userbuf, sizeof(struct pexor_userbuf));
  return rev; /* if address pointers not matching */
}


int pexor_ioctl_first_usedbuffer(struct pexor_privdata *priv, unsigned long arg)
{
  int rev = 0;
  struct pexor_dmabuf* dmabuf;
  struct pexor_userbuf userbuf;
  dmabuf=pexor_get_first_usedbuffer(priv);
  if(dmabuf==0)  return -EFAULT;
  userbuf.addr = dmabuf->virt_addr;
  userbuf.size = dmabuf->used_size;
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_first_usedbuffer: dmabuf %p, virtual:0x%x, used_size:0x%x!\n", dmabuf, dmabuf->virt_addr, dmabuf->used_size);
  rev = copy_to_user ((void __user *) arg, &userbuf, sizeof(struct pexor_userbuf));
  return rev;
}


struct pexor_dmabuf* pexor_get_first_usedbuffer(struct pexor_privdata *priv)
{
  struct pexor_dmabuf* dmabuf;
  spin_lock( &(priv->buffers_lock));
  if (list_empty (&(priv->used_buffers)))
    {
      /* this may happen if user calls free buffer without taking or receiving one before*/
      spin_unlock( &(priv->buffers_lock));
      pexor_dbg(KERN_NOTICE "** pexor_get_first_usedbuffer: List of used buffers is empty!\n");
      return 0;
    }
  dmabuf=list_first_entry(&(priv->used_buffers), struct pexor_dmabuf, queue_list); // do not change list, just access first entry
  spin_unlock( &(priv->buffers_lock));
  return dmabuf;
}


struct pexor_dmabuf* pexor_get_last_usedbuffer(struct pexor_privdata *priv)
{
  struct pexor_dmabuf* dmabuf;
  spin_lock( &(priv->buffers_lock));
  if (list_empty (&(priv->used_buffers)))
    {
      /* this may happen if user calls free buffer without taking or receiving one before*/
      spin_unlock( &(priv->buffers_lock));
      pexor_dbg(KERN_NOTICE "** pexor_get_last_usedbuffer: List of used buffers is empty!\n");
      return 0;
    }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
  dmabuf=list_last_entry(&(priv->used_buffers), struct pexor_dmabuf, queue_list); // do not change list, just access last entry
#else
  dmabuf=list_entry((&(priv->used_buffers))->prev, struct pexor_dmabuf, queue_list);
#endif
  spin_unlock( &(priv->buffers_lock));
  return dmabuf;
}


int pexor_ioctl_deletebuffer (struct pexor_privdata* priv, unsigned long arg)
{
  struct pexor_dmabuf* cursor;
  int retval = 0;
  struct pexor_userbuf bufdescriptor;
  retval = copy_from_user (&bufdescriptor, (void __user *) arg, sizeof(struct pexor_userbuf));
  if (retval)
    return retval;

  retval = pexor_poll_dma_complete (priv,1);
  if (retval)
  {
    pexor_msg(KERN_NOTICE "**pexor_ioctl_deletebuffer: dma is not finished, do not touch buffers!\n");
    return retval;
  }

  pexor_dma_lock((&(priv->dma_lock)));
  spin_lock( &(priv->buffers_lock));
  if (!list_empty (&(priv->used_buffers)))
  {
    list_for_each_entry(cursor, &(priv->used_buffers), queue_list)
      {
      if(cursor->virt_addr==bufdescriptor.addr)
      {
        pexor_dbg(KERN_NOTICE "** pexor_ioctl_delbuffer deleting used buffer %p\n",cursor);
        list_del(&(cursor->queue_list));
        delete_dmabuffer(priv->pdev, cursor);
        spin_unlock( &(priv->buffers_lock) );
        pexor_dma_unlock((&(priv->dma_lock)));
        return 0;
      }
      }
  }

  if (!list_empty (&(priv->free_buffers)))
  {
    list_for_each_entry(cursor, &(priv->free_buffers), queue_list)
    {
      if(cursor->virt_addr==bufdescriptor.addr)
      {
        pexor_dbg(KERN_NOTICE "** pexor_ioctl_delbuffer deleting free buffer %p\n",cursor);
        list_del(&(cursor->queue_list));
        delete_dmabuffer(priv->pdev, cursor);
        spin_unlock( &(priv->buffers_lock) );
        pexor_dma_unlock((&(priv->dma_lock)));
        return 0;
      }
    }
  }
  if (!list_empty (&(priv->received_buffers)))
  {
    list_for_each_entry(cursor, &(priv->received_buffers), queue_list)
    {
      if(cursor->virt_addr==bufdescriptor.addr)
      {
        pexor_dbg(KERN_NOTICE "** pexor_ioctl_delbuffer deleting receive buffer %p\n",cursor);
        list_del(&(cursor->queue_list));
        delete_dmabuffer(priv->pdev, cursor);
        spin_unlock( &(priv->buffers_lock) );
        pexor_dma_unlock((&(priv->dma_lock)));
        return 0;
      }
    }
  }
  spin_unlock( &(priv->buffers_lock));
  pexor_dma_unlock((&(priv->dma_lock)));
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_freebuffer could not find buffer for address %lx\n", bufdescriptor.addr);
  return -EFAULT;
}

int pexor_ioctl_waitreceive (struct pexor_privdata* priv, unsigned long arg)
{
  int rev = 0;
  struct pexor_dmabuf dmabuf;
  struct pexor_trigger_readout readbuf;
  if ((rev = pexor_wait_dma_buffer (priv, &dmabuf)) != 0)
  {
    return rev;
  }
  readbuf.data.addr = dmabuf.virt_addr;
  readbuf.data.size = dmabuf.used_size;    // account for real used size of dma buffer
  pexor_decode_triggerstatus (dmabuf.triggerstatus, &(readbuf.triggerstatus));
  rev = copy_to_user ((void __user *) arg, &readbuf, sizeof(struct pexor_trigger_readout));
  return rev;
}


int pexor_ioctl_set_wait_timeout(struct pexor_privdata* priv, unsigned long arg)
{
  int timeout=0, retval;
  retval = get_user(timeout, (int*) arg);
  if (retval)
      return retval;
  pexor_msg(KERN_NOTICE "pexor_ioctl_set_wait_timeout sets timeout %d seconds\n",timeout);
  priv->wait_timeout=timeout;
  return 0;
}


int pexor_ioctl_setrunstate (struct pexor_privdata* priv, unsigned long arg)
{
  int state, retval;
  retval = get_user(state, (int*) arg);
  if (retval)
    return retval;
  atomic_set(&(priv->state), state);
  switch (state)
  {
    case PEXOR_STATE_STOPPED:
#ifdef PEXOR_WITH_SFP
      pexor_sfp_clear_all (priv);
#endif
     pexor_msg(KERN_ERR "pexor_ioctl_setrunstate has set stopped state.\n");
      /* TODO: actively stop the wait queues/tasklet etc for shutdown?*/
    case PEXOR_STATE_DMA_SUSPENDED:
      break;
    case PEXOR_STATE_DMA_FLOW:
    case PEXOR_STATE_DMA_SINGLE:
      /* these are test modes for DMA engine*/
      retval = pexor_next_dma (priv, 0, 0, 0, 0, 0, 0, 0); /* TODO: set source address cursor?*/
      if (retval)
      {
        /* error handling, e.g. no more dma buffer available*/
        pexor_dbg(KERN_ERR "pexor_ioctl_setrunstate error %d from nextdma\n", retval);
        atomic_set(&(priv->state), PEXOR_STATE_STOPPED);
        return retval;
      }
      break;
    case PEXOR_STATE_IR_TEST:
      pexor_msg(KERN_NOTICE "pexor_ioctl_setting ir teststate \n");

#ifdef PEXOR_WITH_TRIXOR
      iowrite32 (TRIX_CLEAR, priv->pexor.irq_control);
      mb();
      ndelay(2000);

      iowrite32 ((TRIX_EN_IRQ | TRIX_GO), priv->pexor.irq_control);
      mb();
      ndelay(20);
      iowrite32 (TRIX_DT_CLEAR, priv->pexor.irq_status);
      mb();
#else

      iowrite32(PEXOR_IRQ_USER_BIT, priv->pexor.irq_control);
      mb();
      ndelay(20);

      /*iowrite32(1, priv->pexor.irq_status);
 mb();*/
#endif
      print_pexor (&(priv->pexor));
      break;

#ifdef PEXOR_WITH_TRIXOR
    case PEXOR_STATE_TRIGGERED_READ:
      /** automatic readout after trigger interrupt*/
      pexor_msg(KERN_ERR "pexor_ioctl_setrunstate has enabled triggered auto readout.\n");
      break;
#endif

    default:
      pexor_dbg(KERN_ERR "pexor_ioctl_setrunstate unknown target state %d\n", state);
      return -EFAULT;

  }
  return 0;
}

int pexor_ioctl_test (struct pexor_privdata* priv, unsigned long arg)
{
  /* curently we test here the pio of pexor ram without copy from user stuff*/
  void* memstart;
  int i, memsize, retval;
  int localbuf = 0;
  retval = get_user(memsize, (int*) arg);
  if (retval)
    return retval;
  memstart = (void*) (priv->pexor.ram_start);
  pexor_msg(KERN_NOTICE "pexor_ioctl_test starting to write %d integers to %p\n", memsize, memstart);
  for (i = 0; i < memsize; ++i)
  {
    localbuf = i;
    iowrite32 (localbuf, memstart + (i << 2));
    mb();
    pexor_msg(KERN_NOTICE "%d.. ", i);
    if((i%10)==0) pexor_msg(KERN_NOTICE "\n");
  }
  pexor_msg(KERN_NOTICE "pexor_ioctl_test reading back %d integers from %p\n", memsize, memstart);
  for (i = 0; i < memsize; ++i)
  {
    localbuf = ioread32 (memstart + (i << 2));
    pexor_bus_delay();
    if(localbuf!=i)
      pexor_msg(KERN_ERR "Error reading back value %d\n", i);
  }
  pexor_msg(KERN_NOTICE "pexor_ioctl_test finished. \n");
  return 0;
}

int pexor_ioctl_reset (struct pexor_privdata* priv, unsigned long arg)
{
  int ix;
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_reset...\n");

  pexor_dbg(KERN_NOTICE "Clearing DMA status... \n");
  iowrite32 (0, priv->pexor.dma_control_stat);
  mb();
  ndelay(20);
  udelay(10000);

#ifdef PEXOR_WITH_SFP
  pexor_sfp_reset (priv);
  pexor_sfp_clear_all (priv);
#endif

/** JAM2016 reset flags for the asynchronous triggerless readout:*/
for (ix = 0; ix < PEXOR_SFP_NUMBER; ++ix)
  {
    atomic_set(&(priv->bufid[ix]), 0);
    atomic_set(&(priv->sfprequested[ix]), 0);
    atomic_set(&(priv->sfpreceived[ix]), 0);
#ifdef PEXOR_TRIGGERLESS_SEMAPHORE
    if (atomic_read(&(priv->sfp_worker_haslock[ix])) == 1)
    {
      up( &(priv->sfpsem[ix]));
      atomic_set(&(priv->sfp_worker_haslock[ix]),0);
      atomic_set(&(priv->sfp_lock_count[ix]),0);

    }
#endif





  }

#ifdef PEXOR_TRIGGERLESS_WORKER
  /** terminate worker thread for asynchronous read out before we remove buffers:*/
  atomic_set(&(priv->triggerless_acquisition),0);
  ndelay(100);
  cancel_work_sync(&(priv->triggerless_work));
  flush_workqueue(priv->triggerless_workqueue);
#endif

  cleanup_buffers (priv);
  atomic_set(&(priv->irq_count), 0);

  pexor_ioctl_clearreceivebuffers (priv, arg);    // this will cleanup dma and irtype queues

  atomic_set(&(priv->state), PEXOR_STATE_STOPPED);




#ifdef PEXOR_WITH_TRIXOR
  pexor_dbg(KERN_NOTICE "Initalizing TRIXOR... \n");

  iowrite32 (TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR, priv->pexor.irq_status); /*reset interrupt source*/
  mb();
  ndelay(20);

  // atomic_set(&(priv->trig_outstanding), 0);
  // clear interrupt type queue:

  iowrite32 (TRIX_BUS_DISABLE, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_HALT, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_MASTER, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_CLEAR, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (0x10000 - 0x20, priv->pexor.trix_fcti);
  mb();
  ndelay(20);
  iowrite32 (0x10000 - 0x40, priv->pexor.trix_cvti);
  mb();
  ndelay(20);

  iowrite32 (TRIX_DT_CLEAR, priv->pexor.irq_status);
  mb();
  ndelay(20);

  iowrite32 (TRIX_BUS_ENABLE, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_HALT, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_MASTER, priv->pexor.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_CLEAR, priv->pexor.irq_control);
  mb();
  ndelay(20);

  pexor_dbg(KERN_NOTICE " ... TRIXOR done.\n");
#else

  iowrite32(0, priv->pexor.irq_control);
  mb();
  iowrite32(0, priv->pexor.irq_status);
  mb();
#endif
  print_pexor (&(priv->pexor));
  return 0;
}

int pexor_ioctl_clearreceivebuffers (struct pexor_privdata* priv, unsigned long arg)
{
  int i = 0, outstandingbuffers = 0;
  unsigned long wjifs = 0;
  struct pexor_dmabuf* cursor;
  struct pexor_dmabuf* next;
#ifdef PEXOR_TRIGSTAT_QUEUE
  unsigned long flags=0;
#ifdef PEXOR_WITH_TRIXOR
  struct pexor_trigger_buf* trigstat;
#endif
#endif
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers...\n");
  spin_lock( &(priv->buffers_lock));
  list_for_each_entry_safe(cursor, next, &(priv->received_buffers), queue_list)
  {
    pexor_dbg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers moved %lx to free list..\n", (long) cursor);
    list_move_tail(&(cursor->queue_list) , &(priv->free_buffers));
  }
  spin_unlock( &(priv->buffers_lock));
  /* empty possible wait queue events and dec the outstanding counter*/
  outstandingbuffers = atomic_read( &(priv->dma_outstanding));
  for (i = 0; i < outstandingbuffers; ++i)
  {
    wjifs = wait_event_interruptible_timeout (priv->irq_dma_queue, atomic_read( &(priv->dma_outstanding) ) > 0,
        priv->wait_timeout * HZ);
    pexor_dbg(
        KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout with TIMEOUT %ds (=%d jiffies), waitjiffies=%ld, outstanding=%d \n", priv->wait_timeout, priv->wait_timeout*HZ, wjifs, atomic_read( &(priv->dma_outstanding)));

    if (wjifs == 0)
    {
      pexor_msg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n",priv->wait_timeout*HZ);
      return -EFAULT;
    }
    else if (wjifs == -ERESTARTSYS)
    {
      pexor_msg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout woken by signal. abort wait\n");
      return -EFAULT;
    }
    atomic_dec (&(priv->dma_outstanding));
  }    // for outstandingbuffers

#ifdef PEXOR_WITH_TRIXOR
  /* empty possible wait queue events for interrupts and dec the outstanding counter*/
  outstandingbuffers = atomic_read( &(priv->trig_outstanding));
  for (i = 0; i < outstandingbuffers; ++i)
  {
    wjifs = wait_event_interruptible_timeout (priv->irq_trig_queue, atomic_read( &(priv->trig_outstanding) ) > 0,
        priv->wait_timeout * HZ);
    pexor_dbg(
        KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout for trigger queue, with TIMEOUT %ds (=%d jiffies), waitjiffies=%ld, outstanding=%d \n", priv->wait_timeout, priv->wait_timeout*HZ, wjifs, atomic_read( &(priv->trig_outstanding)));

    if (wjifs == 0)
    {
      pexor_dbg(
          KERN_NOTICE "** pexor_ioctl_clearreceivebuffers TIMEOUT %d jiffies expired on wait_event_interruptible_timeout for trigger queue... \n", priv->wait_timeout*HZ);
      return -EFAULT;
    }
    else if (wjifs == -ERESTARTSYS)
    {
      pexor_msg(KERN_NOTICE "** pexor_ioctl_clearreceivebuffers after wait_event_interruptible_timeout for trigger queue woken by signal. abort wait\n");
      return -EFAULT;
    }
    atomic_dec (&(priv->trig_outstanding));
#ifdef PEXOR_TRIGSTAT_QUEUE
    spin_lock_irqsave( &(priv->trigstat_lock),flags);
    if (list_empty (&(priv->trig_status)))
    {
      spin_unlock_irqrestore( &(priv->trigstat_lock),flags);
      pexor_msg(KERN_ERR "pexor_ioctl_clearreceivebuffers never come here - list of trigger type buffers is empty! \n");
      return -EFAULT;
    }
    trigstat=list_first_entry(&(priv->trig_status), struct pexor_trigger_buf, queue_list);
    trigstat->trixorstat = 0;    // mark status object as free
    list_move_tail (&(trigstat->queue_list), &(priv->trig_status));    // move to end of list
    spin_unlock_irqrestore( &(priv->trigstat_lock),flags);
#endif

  }    // for outstandingbuffers

#endif
  return 0;
}

int pexor_ioctl_write_register (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32* ad = 0;
  u32 val = 0;
  int bar = 0;
  struct pexor_reg_io descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_reg_io));
  if (retval)
    return retval;
  /* here we assume something for this very connection, to be adjusted later*/
  ad = (u32*) (ptrdiff_t) descriptor.address;
  val = (u32) descriptor.value;
  bar = descriptor.bar;
  if ((bar > 5) || priv->iomem[bar] == 0)
  {
    pexor_msg(KERN_ERR "** pexor_ioctl_write_register: no mapped bar %d\n",bar);
    return -EIO;
  }pexor_dbg(KERN_NOTICE "** pexor_ioctl_write_register writes value %x to address %p within bar %d \n", val, ad, bar);
  if ((unsigned long) ad > priv->reglen[bar])
  {
    pexor_msg(KERN_ERR "** pexor_ioctl_write_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
    return -EIO;
  }
  ad = (u32*) ((unsigned long) priv->iomem[bar] + (unsigned long) ad);
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_write_register writes value %x to mapped PCI address %p !\n", val, ad);
  iowrite32 (val, ad);
  mb();
  ndelay(20);
  return retval;
}

int pexor_ioctl_read_register (struct pexor_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32* ad = 0;
  u32 val = 0;
  int bar = 0;
  struct pexor_reg_io descriptor;
  retval = copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pexor_reg_io));
  if (retval)
    return retval;
  ad = (u32*) (ptrdiff_t) descriptor.address;
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_reading from register address %p\n", ad);
  bar = descriptor.bar;
  if ((bar > 5) || priv->iomem[bar] == 0)
  {
    pexor_msg(KERN_ERR "** pexor_ioctl_read_register: no mapped bar %d\n",bar);
    return -EIO;
  }pexor_dbg(KERN_NOTICE "** pexor_ioctl_read_register reads from address %p within bar %d \n", ad, bar);
  if ((unsigned long) ad > priv->reglen[bar])
  {
    pexor_msg(KERN_ERR "** pexor_ioctl_read_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
    return -EIO;
  }
  ad = (u32*) ((unsigned long) priv->iomem[bar] + (unsigned long) ad);
  val = ioread32 (ad);
  pexor_bus_delay();
  pexor_dbg(KERN_NOTICE "** pexor_ioctl_read_register read value %x from mapped PCI address %p !\n", val, ad);
  descriptor.value = val;
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pexor_reg_io));
  return retval;
}

int pexor_mmap (struct file *filp, struct vm_area_struct *vma)
{
  struct pexor_privdata *privdata;
  struct pexor_dmabuf* buf;
  int ret = 0;
  unsigned long bufsize;
  privdata = get_privdata (filp);
  pexor_dbg(KERN_NOTICE "** starting pexor_mmap...\n");

  if (!privdata)
    return -EFAULT;

  bufsize = (vma->vm_end - vma->vm_start);
  pexor_dbg(KERN_NOTICE "** starting pexor_mmap for size=%ld \n", bufsize);
  /* create new dma buffer for pci and put it into free list*/
  buf = new_dmabuffer (privdata->pdev, bufsize, vma->vm_pgoff); /* todo: add physmem adress parameter if we have  vma->vm_pgoff
   */
  if (!buf)
    return -EFAULT;

  if (vma->vm_pgoff == 0)
  {
    /* user does not specify external physical address, memory is available in kernelspace:*/

    /* map kernel addresses to vma*/
    pexor_dbg(
        KERN_NOTICE "Mapping address %p / PFN %lx\n", (void*) virt_to_phys((void*)buf->kernel_addr), page_to_pfn(virt_to_page((void*)buf->kernel_addr)));

    /*vma->vm_flags |= (VM_RESERVED);*/
    /* TODO: do we need this?*/
    ret = remap_pfn_range (vma, vma->vm_start, page_to_pfn (virt_to_page((void*)buf->kernel_addr)), buf->size,
        vma->vm_page_prot);
  }
  else
  {
    /* for external phys memory, use directly pfn*/
    pexor_dbg(KERN_NOTICE "Using external address %p / PFN %lx\n", (void*) (vma->vm_pgoff << PAGE_SHIFT ), vma->vm_pgoff);


#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,7,0)
        vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
#endif



    ret = remap_pfn_range (vma, vma->vm_start, vma->vm_pgoff, buf->size, vma->vm_page_prot);

  }

  if (ret)
  {
    pexor_dbg(KERN_ERR "remap_pfn_range failed: %d (%lx)\n", ret, buf->kernel_addr);
    delete_dmabuffer (privdata->pdev, buf);
    return -EFAULT;
  }
  buf->virt_addr = vma->vm_start; /* remember as identifier here*/
  pexor_dbg(
      KERN_ERR "pexor_mmap mapped kernel buffer %lx, size %lx, to virtual address %lx\n", buf->kernel_addr, buf->size, buf->virt_addr);
  spin_lock( &(privdata->buffers_lock));
  /* this list contains only the unused (free) buffers: */
  list_add_tail (&(buf->queue_list), &(privdata->free_buffers));
  spin_unlock( &(privdata->buffers_lock));

  return ret;
}

struct pexor_dmabuf* new_dmabuffer (struct pci_dev * pdev, size_t size, unsigned long pgoff)
{
  struct pexor_dmabuf* descriptor;
  descriptor = kmalloc (sizeof(struct pexor_dmabuf), GFP_KERNEL);
  if (!descriptor)
  {
    pexor_dbg(KERN_ERR "new_dmabuffer: could not alloc dma buffer descriptor! \n");
    return NULL ;
  }
  memset (descriptor, 0, sizeof(struct pexor_dmabuf));
  descriptor->size = size;

  if (pgoff == 0)
  {
    /* no external target address specified, we create internal buffers  */

#ifdef	DMA_MAPPING_STREAMING
    /* here we use plain kernel memory which we explicitly map for dma*/
    descriptor->kernel_addr=(unsigned long) kmalloc(size, GFP_KERNEL);
    if(!descriptor->kernel_addr)
    {
      pexor_msg(KERN_ERR "new_dmabuffer: could not alloc streaming dma buffer for size %d \n",size);
      kfree(descriptor);
      return NULL;
    }
    descriptor->dma_addr= dma_map_single(&(pdev->dev), (void*) descriptor->kernel_addr, size, PCI_DMA_FROMDEVICE);
    if(!descriptor->dma_addr)
    {
      pexor_msg(KERN_ERR "new_dmabuffer: could not map streaming dma buffer for size %d \n",size);
      kfree((void*) descriptor->kernel_addr);
      kfree(descriptor);
      return NULL;
    }

    pexor_dbg(KERN_ERR "new_dmabuffer created streaming kernel buffer with dma address %lx\n", descriptor->dma_addr);

#else
    /* here we get readily mapped dma memory which was preallocated for the device */
   // descriptor->kernel_addr = (unsigned long) pci_alloc_consistent (pdev, size, &(descriptor->dma_addr));

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
    descriptor->kernel_addr = (unsigned long) dma_alloc_coherent(&(pdev->dev), size, &(descriptor->dma_addr), GFP_ATOMIC);
#else
    descriptor->kernel_addr = (unsigned long) pci_alloc_consistent (pdev, size, &(descriptor->dma_addr));
#endif

	//return

    if (!descriptor->kernel_addr)
    {
      pexor_msg(KERN_ERR "new_dmabuffer: could not alloc pci dma buffer for size %d \n",(int)size);
      kfree (descriptor);
      return NULL ;
    }
    /* maybe obsolete here, but we could gain performance by defining the data direction...*/
  //  pci_dma_sync_single_for_device (pdev, descriptor->dma_addr, descriptor->size, PCI_DMA_FROMDEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_single_for_device( &(pdev->dev), descriptor->dma_addr, descriptor->size,DMA_FROM_DEVICE );
#else
  pci_dma_sync_single_for_device( pdev, descriptor->dma_addr, descriptor->size,PCI_DMA_FROMDEVICE );
#endif

    pexor_dbg(KERN_ERR "new_dmabuffer created coherent kernel buffer with dma address %p\n", (void*) descriptor->dma_addr);

#endif

  }
  else
  {
    /* set dma buffer for external physical dma address*/
    descriptor->kernel_addr = 0; /* can not map external RAM into linux kernel space*/
    descriptor->dma_addr = pgoff << PAGE_SHIFT;
  //  pci_dma_sync_single_for_device (pdev, descriptor->dma_addr, descriptor->size, PCI_DMA_FROMDEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_single_for_device( &(pdev->dev), descriptor->dma_addr, descriptor->size,DMA_FROM_DEVICE );
#else
    pci_dma_sync_single_for_device (pdev, descriptor->dma_addr, descriptor->size,PCI_DMA_FROMDEVICE);
#endif


    pexor_dbg(KERN_ERR "new_dmabuffer created dma buffer for external dma address %p\n", (void*) descriptor->dma_addr);
  }

  INIT_LIST_HEAD (&(descriptor->queue_list));
  pexor_dbg(KERN_NOTICE "**pexor_created new_dmabuffer, size=%d, addr=%lx \n", (int) size, descriptor->kernel_addr);

  return descriptor;
}

int delete_dmabuffer (struct pci_dev * pdev, struct pexor_dmabuf* buf)
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
      pexor_dbg(
          KERN_NOTICE "**pexor_delete_dmabuffer of size=%ld, unregistering external physaddr=%lx \n", buf->size, (unsigned long) buf->dma_addr);
     // pci_dma_sync_single_for_cpu (pdev, buf->dma_addr, buf->size, PCI_DMA_FROMDEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_single_for_cpu( &(pdev->dev), buf->dma_addr, buf->size,DMA_FROM_DEVICE );
#else
    pci_dma_sync_single_for_cpu (pdev, buf->dma_addr, buf->size,PCI_DMA_FROMDEVICE);
#endif

      /* Release descriptor memory */
      kfree (buf);
      return 0;
    }

  }

  pexor_dbg(KERN_NOTICE "**pexor_deleting dmabuffer, size=%ld, addr=%lx \n", buf->size, buf->kernel_addr);
  /* for (i=0;i<50;++i)
 {
 pexor_dbg(KERN_NOTICE "dmabuffer[%x]=%x \t", i, ioread32(buf->kernel_addr + i*4));
 }*/
  /* note: unmapping the virtual adresses is done in user application by munmap*/
#ifdef	DMA_MAPPING_STREAMING
  /* release dma mapping and free kernel memory for dma buffer*/
  //dma_unmap_single(&(pdev->dev), buf->dma_addr, buf->size, PCI_DMA_FROMDEVICE);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
   dma_unmap_sg (&(priv->pdev->dev), (priv->pipe).sg, (priv->pipe).num_pages, DMA_FROM_DEVICE);
#else
   pci_unmap_sg (priv->pdev, (priv->pipe).sg, (priv->pipe).num_pages, PCI_DMA_FROMDEVICE);
#endif



  kfree((void*) buf->kernel_addr);
#else
  /* Release DMA memory */


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_free_coherent(&pdev->dev, buf->size, (void *) (buf->kernel_addr), buf->dma_addr);
#else
  pci_free_consistent (pdev, buf->size, (void *) (buf->kernel_addr), buf->dma_addr);
#endif

 // dma_free_coherent(&hwdev->dev, size, vaddr, dma_handle);

#endif
  /* Release descriptor memory */
  kfree (buf);
  return 0;
}

int unmap_sg_dmabuffer (struct pci_dev *pdev, struct pexor_dmabuf *buf)
{
  int i = 0;
  pexor_dbg(KERN_NOTICE "**pexor unmapping sg dmabuffer, size=%ld, user address=%lx \n", buf->size, buf->virt_addr);
 // pci_unmap_sg (pdev, buf->sg, buf->num_pages, PCI_DMA_FROMDEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
   dma_unmap_sg (&(pdev->dev), buf->sg, buf->num_pages, DMA_FROM_DEVICE);
#else
   pci_unmap_sg (pdev, buf->sg, buf->num_pages, PCI_DMA_FROMDEVICE);
#endif


  for (i = 0; i < (buf->num_pages); i++)
  {
    if (!PageReserved (buf->pages[i]))
    {
      SetPageDirty (buf->pages[i]);
      compat_unlock_page(buf->pages[i]);
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
        page_cache_release(buf->pages[i]);
#else
        put_page(buf->pages[i]);
#endif


  }
  vfree (buf->pages);
  vfree (buf->sg);
  kfree (buf);

  return 0;
}

void cleanup_buffers (struct pexor_privdata* priv)
{
  struct pexor_dmabuf* cursor;
  struct pexor_dmabuf* next;
  pexor_dbg(KERN_NOTICE "**pexor_cleanup_buffers...\n");

  if (pexor_poll_dma_complete (priv,1))
  {
    pexor_msg(KERN_NOTICE "**pexor_cleanup_buffers: dma is not finished, do not touch buffers!\n");
    return;
  }

  pexor_dma_lock((&(priv->dma_lock)));
  spin_lock( &(priv->buffers_lock));
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
  spin_unlock( &(priv->buffers_lock));
  pexor_dma_unlock((&(priv->dma_lock)));

  pexor_dbg(KERN_NOTICE "**pexor_cleanup_buffers...done\n");
}

struct pexor_privdata* get_privdata (struct file *filp)
{
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) filp->private_data;
  if (privdata->pexor.init_done == 0)
  {
    pexor_dbg(KERN_ERR "*** PEXOR structure was not initialized!\n");
    return NULL ;
  }
  return privdata;
}

void print_register (const char* description, u32* address)
{
  pexor_dbg(KERN_NOTICE "%s:\taddr=%lx cont=%x\n", description, (long unsigned int) address, readl(address));
  pexor_bus_delay();
}

void print_pexor (struct dev_pexor* pg)
{
  if (pg == 0)
    return;pexor_dbg(KERN_NOTICE "\n##print_pexor: ###################\n");
  pexor_dbg(KERN_NOTICE "init: \t=%x\n", pg->init_done);
  if (!pg->init_done)
    return;
  print_register ("dma control/status", pg->dma_control_stat);
#ifdef PEXOR_WITH_TRIXOR
  /*pexor_dbg(KERN_NOTICE "trixor control add=%x \n",pg->irq_control) ;
 pexor_dbg(KERN_NOTICE "trixor status  add =%x \n",pg->irq_status);
 pexor_dbg(KERN_NOTICE "trixor fast clear add=%x \n",pg->trix_fcti) ;
 pexor_dbg(KERN_NOTICE "trixor conversion time add =%x \n",pg->trix_cvti);*/

  print_register ("trixor fast clear time", pg->trix_fcti);
  print_register ("trixor conversion time", pg->trix_cvti);
#endif

  print_register ("dma source address", pg->dma_source);
  print_register ("dma dest   address", pg->dma_dest);
  print_register ("dma len   address", pg->dma_len);
  print_register ("dma burstsize", pg->dma_burstsize);

  print_register ("RAM start", pg->ram_start);
  print_register ("RAM end", pg->ram_end);
  pexor_dbg(KERN_NOTICE "RAM DMA base add=%x \n", (unsigned) pg->ram_dma_base);
  pexor_dbg(KERN_NOTICE "RAM DMA cursor add=%x \n", (unsigned) pg->ram_dma_cursor);

#ifdef PEXOR_WITH_SFP
  print_sfp (&(pg->sfp));
#endif

}

void clear_pexor (struct dev_pexor* pg)
{
  if (pg == 0)
    return;
  pg->init_done = 0x0;
  pexor_dbg(KERN_NOTICE "** Cleared pexor structure %lx.\n", (long unsigned int) pg);
}

void set_pexor (struct dev_pexor* pg, void* membase, unsigned long bar)
{

  void* dmabase = 0;
  if (pg == 0)
    return;
  dmabase = membase + PEXOR_DMA_BASE;
#ifdef PEXOR_WITH_TRIXOR
  pg->irq_control = (u32*) (membase + PEXOR_TRIXOR_BASE + PEXOR_TRIX_CTRL);
  pg->irq_status = (u32*) (membase + PEXOR_TRIXOR_BASE + PEXOR_TRIX_STAT);
  pg->trix_fcti = (u32*) (membase + PEXOR_TRIXOR_BASE + PEXOR_TRIX_FCTI);
  pg->trix_cvti = (u32*) (membase + PEXOR_TRIXOR_BASE + PEXOR_TRIX_CVTI);
#else
  pg->irq_control=(u32*)(membase+PEXOR_IRQ_CTRL);
  pg->irq_status=(u32*)(membase+PEXOR_IRQ_STAT);
#endif

  pg->dma_control_stat = (u32*) (dmabase + PEXOR_DMA_CTRLSTAT);
  pg->dma_source = (u32*) (dmabase + PEXOR_DMA_SRC);
  pg->dma_dest = (u32*) (dmabase + PEXOR_DMA_DEST);
  pg->dma_len = (u32*) (dmabase + PEXOR_DMA_LEN);
  pg->dma_burstsize = (u32*) (dmabase + PEXOR_DMA_BURSTSIZE);

  pg->ram_start = (u32*) (membase + PEXOR_DRAM);
  pg->ram_end = (u32*) (membase + PEXOR_DRAM + PEXOR_RAMSIZE);
  pg->ram_dma_base = (dma_addr_t) (bar + PEXOR_DRAM);
  pg->ram_dma_cursor = (dma_addr_t) (bar + PEXOR_DRAM);
#ifdef PEXOR_WITH_SFP
  set_sfp (&(pg->sfp), membase, bar);
#endif

  pg->init_done = 0x1;
  pexor_dbg(KERN_NOTICE "** Set pexor structure %lx.\n", (long unsigned int) pg);

}

irqreturn_t pexor_isr (int irq, void *dev_id)
{
  u32 irtype, irstat, irmask;
  int state;
#ifdef  PEXOR_TRIGSTAT_QUEUE
  unsigned long flags;
#ifdef PEXOR_WITH_TRIXOR
  struct pexor_trigger_buf* trigstat;
#endif
#endif
  struct pexor_privdata *privdata;

  privdata = (struct pexor_privdata *) dev_id;

  irmask=(TRIX_EV_IRQ_CLEAR | TRIX_DT_CLEAR);

#ifdef PEXOR_DISABLE_IRQ_ISR
  //disable_irq(irq);  // disable and spinlock until any isr of that irq has been finished -> deadlock!
  disable_irq_nosync (irq);    // disable irq line
#endif
  ndelay(1000);
  // need this here?
#ifdef PEXOR_SHARED_IRQ

#ifdef PEXOR_WITH_TRIXOR
  /* check if this interrupt was raised by our device*/
  irtype = ioread32 (privdata->pexor.irq_status);
  mb();
  ndelay(200);
  pexor_dbg(KERN_NOTICE "pexor driver interrupt handler with interrupt status 0x%x!\n", irtype);
  if ((irtype & irmask) == irmask) /* test bits */
  {
    /* prepare for trixor interrupts here:*/
    irstat = (irtype << 16) & 0xffff0000;
    /*< shift trigger status bits to upper words to be compatible for historic mbs mapping definitions later*/
    irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
    iowrite32 (irtype, privdata->pexor.irq_status); /*reset interrupt source*/
    mb();
    ndelay(1000);
    /* pexor_dbg(KERN_NOTICE "pexor driver interrupt handler cleared irq status!\n");*/
    /* now find out if we did interrupt test*/
    state = atomic_read(&(privdata->state));
    if (state == PEXOR_STATE_IR_TEST)
    {
      pexor_msg(KERN_NOTICE "pexor driver interrupt handler sees ir test!\n");
      state = PEXOR_STATE_STOPPED;
      atomic_set(&(privdata->state), state);
    }
    else if (state == PEXOR_STATE_TRIGGERED_READ)
    {
      /* in this mode we issue a tasklet that will handle automatic token request readout*/
      atomic_set(&(privdata->trigstat), irstat);
      // trigger status will be evaluated by tasklet, no parallel trigger queue!
      atomic_inc (&(privdata->irq_count));
      // schedule tasklet
      pexor_dbg(KERN_NOTICE "pexor driver interrupt handler schedules tasklet... \n");
      tasklet_schedule (&privdata->irq_bottomhalf);
    }
    else
    {
      /* regular readout mode with explicit user request for data.
       * We just put trigger type into queue (or atomic variable), and wake up waiting application:*/


#ifdef PEXOR_TRIGSTAT_QUEUE
      /* put current irtype to queue for consumer evaluation: */
      // use first unassigned entry in ring buffer:
      spin_lock_irqsave( &(privdata->trigstat_lock),flags);
      list_for_each_entry(trigstat, &(privdata->trig_status), queue_list)
      {
        if(trigstat->trixorstat == 0)
          break;
      }
      // todo: handle case where ringbuffer is full, i.e. there is no entry marked with zero status
      // in the above case we would overwrite the last (oldest) entry
      trigstat->trixorstat = irstat;    // change of entry state is also inside the list lock!
      spin_unlock_irqrestore( &(privdata->trigstat_lock),flags);
      pexor_dbg(KERN_NOTICE "pexor driver interrupt handler has queued triggerstatus 0x%x!\n", trigstat->trixorstat);
#else
      atomic_set(&(privdata->trigstat), irstat);
      pexor_dbg(KERN_NOTICE "pexor driver interrupt handler has atomic triggerstatus 0x%x!\n", irstat);
#endif


      /* trigger interrupt from trixor. wake up waiting application if any:*/
      /* pexor_dbg(KERN_NOTICE "pexor driver interrupt handler sees trigger ir!\n"); */
      atomic_inc (&(privdata->trig_outstanding));
      wake_up_interruptible (&(privdata->irq_trig_queue));
    }

#ifdef PEXOR_DISABLE_IRQ_ISR
    enable_irq (irq);
#endif

    return IRQ_HANDLED;
  }

#else

  /* check if this interrupt was raised by our device*/
  irtype=ioread32(privdata->pexor.irq_status);
  mb();
  ndelay(20);
  if(irtype & PEXOR_IRQ_USER_BIT)
  {

    /* OLD for pexor 1*/
    mb();
    irstat=irtype;
    irtype &= ~(PEXOR_IRQ_USER_BIT);
    iowrite32(irtype, privdata->pexor.irq_control); /*reset interrupt source*/
    iowrite32(irtype, privdata->pexor.irq_status); /*reset interrupt source*/
    mb();
    ndelay(20);
    pexor_msg(KERN_NOTICE "pexor driver interrupt handler cleared irq status!\n");
    /* now find out if we did interrupt test, trigger, or  real dma raised interrupt:*/
    state=atomic_read(&(privdata->state));
    if(state==PEXOR_STATE_IR_TEST)
    {
      pexor_msg(KERN_NOTICE "pexor driver interrupt handler sees ir test!\n");
      state=PEXOR_STATE_STOPPED;
      atomic_set(&(privdata->state),state);
      //	  ndelay(1000);
      //	  enable_irq(irq);
      return IRQ_HANDLED;
    }

  }
#endif

  else
  {
    pexor_dbg(KERN_NOTICE "pexor test driver interrupt handler sees unknown ir type %x !\n", irtype);
#ifdef PEXOR_DISABLE_IRQ_ISR
    enable_irq (irq);
#endif
    return IRQ_NONE;
  }

#ifdef PEXOR_DISABLE_IRQ_ISR
  enable_irq (irq);
#endif
  return IRQ_HANDLED;

#else
  pexor_msg(KERN_NOTICE "pexor test driver interrupt handler is executed non shared.\n");

  iowrite32(0, privdata->pexor.irq_control);
  return IRQ_HANDLED; /* for debug*/

#endif

}

void pexor_irq_tasklet (unsigned long arg)
{
  int retval;
  static int bufid = 0;
  u32 rstat = 0, radd = 0, rdat = 0;
  u32 dmasize = 0, woffset = 0, comm = 0, trigstat = 0;
  int sfp = 0, channelmask = 0;
  struct pexor_trigger_status descriptor;
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) arg;
  trigstat = atomic_read(&(privdata->trigstat));
  pexor_dbg(
      KERN_NOTICE "pexor_irq_tasklet is executed, irq_count=%d, trigstat=0x%x\n", atomic_read(&(privdata->irq_count)), trigstat);

  /* check interrupt count before tasklet. should be one! */
  if (!atomic_dec_and_test (&(privdata->irq_count)))
  {
    pexor_msg(KERN_ALERT "pexor_irq_tasklet found more than one ir: N.C.H.\n");
  }
  udelay(20);
  // waitstate between trigger interrupt and accessing pexor

  pexor_decode_triggerstatus (trigstat, &descriptor);
  pexor_dbg(
      KERN_NOTICE "pexor_irq_tasklet receives trigtyp:0x%x si:0x%x mis:0x%x lec:0x%x di:0x%x tdt:0x%x eon:0x%x \n", descriptor.typ, descriptor.si, descriptor.mis, descriptor.lec, descriptor.di, descriptor.tdt, descriptor.eon);
  // here we have to perform the real halt if trigger was stop acquisition!
  if (descriptor.typ == PEXOR_TRIGTYPE_STOP)
  {
    pexor_trigger_do_stop (privdata);
    pexor_dbg(KERN_NOTICE "pexor_irq_tasklet has trigger 0x%x, did trixor halt and clear!\n", PEXOR_TRIGTYPE_STOP);
  }
  else if (descriptor.typ == PEXOR_TRIGTYPE_START)
  {
    /* do nothing special here. Trigger type is passed upwards with dummy buffer,
     * userland application may react by explicit readout request.*/
    woffset = 0;
    bufid = 1;    // after start acquisition, always begin with bufid 0
  }
  else
  {
    /** here automatic token request mode*/

    /* loop over all configured sfps*/
    struct pexor_sfp* sfpregisters = &(privdata->pexor.sfp);
    for (sfp = 0; sfp < PEXOR_SFP_NUMBER; ++sfp)
    {
      if (sfpregisters->num_slaves[sfp] == 0)
        continue;
      /* for each do token request with direct dma:*/
      channelmask = 1 << (sfp + 1);    // select SFP for PCI Express DMA
      retval = pexor_next_dma (privdata, 0, 0, woffset, 0, 0, channelmask,0);
      if (retval)
      {
        /* error handling, e.g. no more dma buffer available*/
        pexor_msg(KERN_ERR "pexor_irq_tasklet error %d from nextdma\n", retval);
        atomic_set(&(privdata->state), PEXOR_STATE_STOPPED);
        return;
      }
      /** the actual token request: */
      comm = PEXOR_SFP_PT_TK_R_REQ | (0x1 << (16 + sfp)); /* single sfp token mode*/
      pexor_sfp_clear_channel (privdata, sfp);
      pexor_sfp_request (privdata, comm, bufid, 0); /* note: slave is not specified; the chain of all slaves will send everything to receive buffer*/
      ndelay(1000);
      /* give pexor time to evaluate requests?*/
      if ((retval = pexor_sfp_get_reply (privdata, sfp, &rstat, &radd, &rdat, 0, 0)) != 0)    // debug: do not check reply status
        //if((retval=pexor_sfp_get_reply(priv, chan, &rstat, &radd, &rdat, PEXOR_SFP_PT_TK_R_REP,0))!=0)
      {
        pexor_msg(KERN_ERR "** pexor_irq_tasklet: error %d at sfp_%d reply \n",retval,sfp);
        pexor_msg(KERN_ERR "    incorrect reply: 0x%x 0x%x 0x%x \n", rstat, radd, rdat)
        return;
      }
      if ((retval = pexor_poll_dma_complete (privdata,0)) != 0)
        return;
      /* probably poll_dma_complete is not necessary here, since direct dma will reply
       * token request no sooner than dma has finished ?*/

      /* find out real package length after dma has completed:*/
      dmasize = ioread32 (privdata->pexor.dma_len);
      mb();
      ndelay(20);
      pexor_dbg(KERN_NOTICE "pexor_irq_tasklet for sfp:%dfinds dma len:=0x%x...\n", sfp, dmasize);
      woffset += dmasize;
      /* check if offset will exceed dma receivce buffer is done in pexor_next_dma(
       * note that we cannot check if dma will reach buffer boundaries, since direct dma*/

      /** after this has finished, evaluate next one with write offset to same buffer*/
    }    // for sfp
    /* this is the final dma receiving that also moves current buffer to receive queue.
     * Used size is derived from final write offset */

  } /* if else trigtype */

  pexor_receive_dma_buffer (privdata, woffset, trigstat);
  /* poll for final dma completion and wake up "DMA wait queue""
   * note that this function is executed even if no token request DMA was performed (special trigger 14/15)
   * In this case, just the empty next buffer of free queue is moved to receive queue and send
   * to userland as dummy buffer, marked with trigstat.*/

  udelay(10);
  // waitstate between readout complete and trigger reset
  /** RESET trigger here, probably we can do this already before  pexor_receive_dma_buffer?*/
  pexor_trigger_reset (privdata);

  bufid = (bufid ? 0 : 1);
  /* switch frontend double buffer id for next request!
   * otherwise frontends may stall after 3rd event*/

  /* error handling? tasklet has no return values...*/
}

int pexor_next_dma (struct pexor_privdata* priv, dma_addr_t source, u32 roffset, u32 woffset, u32 dmasize,
    unsigned long* bufid, u32 channelmask, u32 burstsize)
{
  struct pexor_dmabuf* nextbuf;
  int i, rev, rest;
  struct scatterlist *sgentry;
  dma_addr_t sgcursor;
  unsigned int sglen, sglensum;
  if (source == 0)
  {
    priv->pexor.ram_dma_cursor = (priv->pexor.ram_dma_base + roffset);
  }
  else
  {
    priv->pexor.ram_dma_cursor = (source + roffset);
  }
  /* setup next free buffer as dma target*/
  pexor_dbg(KERN_NOTICE "#### pexor_next_dma...\n");

  spin_lock( &(priv->buffers_lock));
  if (list_empty (&(priv->free_buffers)))
  {
    spin_unlock( &(priv->buffers_lock));
    pexor_msg(KERN_ERR "pexor_next_dma: list of free buffers is empty. try again later! \n");
    return -EINVAL;
    /* TODO: handle dynamically what to do when running out of dma buffers*/
  }
  /* put here search for dedicated buffer in free list:*/
  if (bufid && *bufid != 0)
  {
    /* we want to fill a special buffer, find it in free list:*/
    list_for_each_entry(nextbuf, &(priv->free_buffers), queue_list)
    {
      if(nextbuf->virt_addr==*bufid)
      {
        pexor_dbg(KERN_NOTICE "** pexor_next_dma is using buffer of id 0x%lx\n",*bufid);
        /* put desired buffer to the begin of the free list, this will be treated subsequently*/
        list_move(&(nextbuf->queue_list) , &(priv->free_buffers));
        break;
      }
    }
    if (nextbuf->virt_addr != *bufid)
    {
      /* check again if we found the correct buffer in list...*/
      spin_unlock( &(priv->buffers_lock));
      pexor_dbg(KERN_ERR "pexor_next_dma: buffer of desired id 0x%lx is not in free list! \n", *bufid);
      return -EINVAL;
    }
  }
  else
  {
    /* just take next available buffer to fill by DMA:*/
    nextbuf=list_first_entry(&(priv->free_buffers), struct pexor_dmabuf, queue_list);
    *bufid=nextbuf->virt_addr; // pass to caller to optionally reuse the same buffer
    pexor_dbg(KERN_ERR "pexor_next_dma: using buffer id 0x%lx (virt addr:0x%lx)! \n", *bufid, nextbuf->virt_addr);
  }
  spin_unlock( &(priv->buffers_lock));

  if (woffset > nextbuf->size - 8)
  {
    pexor_msg(
        KERN_NOTICE "#### pexor_next_dma illlegal write offset 0x%x for target buffer size 0x%x\n", woffset, (unsigned) nextbuf->size);
    return -EINVAL;
  }

  /* here decision if sg dma or plain*/

  //if((nextbuf->kernel_addr !=0)
  if (nextbuf->sg == 0) /* this check is better, since dma to external phys also has no kernel adress!*/
  {
    /* here we have coherent kernel buffer case*/
    pexor_dbg(KERN_ERR "#### pexor_next_dma in kernel buffer mode\n");
    if ((channelmask < 1))
    {
      // regular dma from pexor memory: check boundaries

      if ((dmasize == 0) || (dmasize > nextbuf->size - woffset))
      {
        pexor_msg(KERN_NOTICE "#### pexor_next_dma resetting old dma size %x to %lx\n", dmasize, nextbuf->size);
        dmasize = nextbuf->size - woffset;
      }

      // JAM NOTE: this check is only meaningfull for dma tests
      // RAMSIZE here covers only ram for sfp0, it will fail for higher sfps!
      //
      //if (priv->pexor.ram_dma_cursor + dmasize > priv->pexor.ram_dma_base + PEXOR_RAMSIZE)
      //  {
      //  pexor_dbg(KERN_NOTICE "#### pexor_next_dma resetting old dma size %x...\n",dmasize);
      //  dmasize = priv->pexor.ram_dma_base + PEXOR_RAMSIZE - priv->pexor.ram_dma_cursor;
      //  }

    }
    else
    {
      // direct dma with gosip request, unknown size!
      // can not check anything here!

    }

#ifdef PEXOR_WITH_SFP
    if (channelmask > 1)
    {
      if (pexor_start_dma (priv, 0, nextbuf->dma_addr + woffset, 0, 0, channelmask, burstsize) < 0)
        return -EINVAL;
    }
    else
#endif

      if (pexor_start_dma (priv, priv->pexor.ram_dma_cursor, nextbuf->dma_addr + woffset, dmasize, 0, channelmask, burstsize) < 0)
        return -EINVAL;

  }

  else
  {
    /* put emulated sg dma here
     * since pexor gosip fpga code does not support sglist dma, we do it manually within the driver*/
    pexor_dbg(KERN_NOTICE "#### pexor_next_dma in scatter-gather mode\n");

    if (channelmask > 1)
    {
      pexor_msg(KERN_ERR "#### pexor_next_dma: ERROR no direct gosip DMA in scatter-gather mode\n");
      return -EINVAL;
    }

    /* test: align complete buffer to maximum burst?*/
    rest = dmasize % PEXOR_BURST;
    if (rest)
    {
      dmasize = dmasize + PEXOR_BURST - rest;
      if (dmasize > nextbuf->size)
        dmasize -= PEXOR_BURST; /*avoid exceeding buf limits*/

      pexor_dbg(KERN_NOTICE "#### pexor_next_dma correcting dmasize %x for rest:%x, burst:%x\n", dmasize, rest, PEXOR_BURST);
    }

    sgcursor = priv->pexor.ram_dma_cursor;
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
      pexor_dbg(
          KERN_ERR "#### pexor_next_dma would start dma from 0x%x to 0x%x of length 0x%x, offset 0x%x, complete chunk length: 0x%x\n", (unsigned) sgcursor, (unsigned) sg_dma_address(sgentry), sglen, woffset, sg_dma_len(sgentry));

      /**** END DEBUG*/
      /* initiate dma to next sg part:*/
      if (pexor_start_dma (priv, sgcursor, sg_dma_address(sgentry) + woffset, sglen, (woffset > 0), 0, 0 ) < 0)
        return -EINVAL;
      if (woffset > 0)
        woffset = 0; /* reset write offset, once it was applied to first sg segment*/

      if ((rev = pexor_poll_dma_complete (priv,1)) != 0)
      {
        pexor_dbg(KERN_ERR "#### pexor_next_dma error on polling for sg entry %d completion, \n", i);
        return rev;
      }
      sglensum += sglen;
      if (sglensum >= dmasize)
      {
        pexor_dbg(KERN_NOTICE "#### pexor_next_dma has finished sg buffer dma after %d segments\n", i);
        break;
      }
      sgcursor += sglen;

    }    // for each sg

    if (sglensum < dmasize)
    {
      pexor_dbg(KERN_ERR "#### pexor_next_dma could not write full size 0x%x to sg buffer of len 0x%x\n", dmasize, sglensum);
      return -EINVAL;
    }

  }    // end plain dma or emulated sg

  return 0;

}

int pexor_start_dma (struct pexor_privdata *priv, dma_addr_t source, dma_addr_t dest, u32 dmasize, int firstchunk,
    u32 channelmask, u32 burstsize)
{
  int rev;
  u32 enable = PEXOR_DMA_ENABLED_BIT; /* this will start dma immediately from given source address*/
  if (channelmask > 1)
    enable = channelmask; /* set sfp token transfer to initiate the DMA later*/

  if (enable == PEXOR_DMA_ENABLED_BIT) // JAM test for nyxor problem: only check previous dma if not in direct dma preparation mode
   {
     rev = pexor_poll_dma_complete (priv,1);
     if (rev)
     {
       pexor_msg(KERN_NOTICE "**pexor_start_dma: dma was not finished, do not start new one!\n");
       return rev;
     }
   }



  if (burstsize == 0)
    {
    /* automatic burst size mode. currently only used for sg emulation*/
    burstsize = PEXOR_BURST;

    /* calculate maximum burstsize here:*/
    while (dmasize % burstsize)
    {
    burstsize = (burstsize >> 1);
    }
    if (burstsize < PEXOR_BURST_MIN)
    {
    pexor_msg(KERN_NOTICE "**pexor_start_dma: correcting for too small burstsize %x\n", burstsize);
    burstsize = PEXOR_BURST_MIN;
    while (dmasize % burstsize)
    {
      if (firstchunk)
      {
        /* We assume this only happens in sg mode for the first chunk when applying header offset which is word aligned
         * In this case we just start a little bit before the payload and overwrite some bytes of the header part
         * We also assume that complete PCIe bar of pexor is dma mapped, so it doesnt hurt for the source!*/
        source -= 2;
        dest -= 2;
      }
      dmasize += 2;
      /* otherwise this can only happen in the last chunk of sg dma.
       * here it should be no deal to transfer a little bit more...*/
    }
    pexor_msg(
      KERN_NOTICE "**changed source address to 0x%x, dest:0x%x, dmasize to 0x%x, burstsize:0x%x\n", (unsigned) source, (unsigned) dest, dmasize, burstsize)
    }

    }    // if automatic burstsizemode


  pexor_dbg(
      KERN_NOTICE "#### pexor_start_dma will initiate dma from %p to %p, len=%x, burstsize=%x...\n", (void*) source, (void*) dest, dmasize, burstsize);

  pexor_dma_lock((&(priv->dma_lock)));
  iowrite32 (source, priv->pexor.dma_source);
  mb();
  iowrite32 ((u32) dest, priv->pexor.dma_dest);
  mb();
  iowrite32 (burstsize, priv->pexor.dma_burstsize);
  mb();
  iowrite32 (dmasize, priv->pexor.dma_len);
  mb();
  iowrite32 (enable, priv->pexor.dma_control_stat);
  mb();
  if (enable > 1)
  {
    pexor_dbg(KERN_NOTICE "#### pexor_start_dma sets sfp mask to 0x%x \n", enable);
  }
  else
  {
    pexor_dbg(KERN_NOTICE "#### pexor_start_dma started dma\n");
  }
  return 0;
}


void pexor_eval_dma_size(u32* dmasize, u32* dmaburst)
{
  // choose burst size to accept max. 20% padding size
     if (*dmasize < 0xa0)
     {
       *dmaburst = 0x10;
     }
     else if (*dmasize < 0x140)
     {
       *dmaburst = 0x20;
     }
     else if (*dmasize < 0x280)
     {
       *dmaburst = 0x40;
     }
     else
     {
       *dmaburst = 0x80;
     }


     // JAM2016: try to avoid case with wrongly read tkmemsize which might confuse dma engine: ???
     if(*dmasize<*dmaburst)
     {

       pexor_msg(KERN_NOTICE "** pexor_eval_dma_size sees dmasize 0x%x, correcting to burstsize:0x%x\n", *dmasize,
           *dmaburst);
       *dmasize=*dmaburst;
     }



     // - calculate DMA transfer size up to full burstlength multiples
     if ((*dmasize % *dmaburst) != 0)
     {
       *dmasize = *dmasize + *dmaburst     // in bytes
       - (*dmasize % *dmaburst);

     }
     else
     {
       //dmalen = *dmasize;
     }

}




int pexor_poll_dma_complete (struct pexor_privdata* priv, int doschedule)
{
  int loops = 0;
  u32 enable = PEXOR_DMA_ENABLED_BIT;

  while (1)
  {
    /* pexor_dbg(KERN_ERR "pexor_poll_dma_complete reading from 0x%p \n",priv->pexor.dma_control_stat);*/

    enable = ioread32 (priv->pexor.dma_control_stat);
    mb();
    //   pexor_dbg(KERN_ERR "pexor_poll_dma_complete sees dmactrl=: 0x%x , looking for %x\n",enable, PEXOR_DMA_ENABLED_BIT);
    if ((enable & PEXOR_DMA_ENABLED_BIT) == 0)
      break;
    /* poll until the dma bit is cleared => dma complete*/

    //pexor_dbg(KERN_NOTICE "#### pexor_poll_dma_complete wait for dma completion #%d\n",loops);
    if (loops++ > PEXOR_DMA_MAXPOLLS)
    {
      pexor_dma_unlock((&(priv->dma_lock)));
      pexor_msg(KERN_ERR "pexor_poll_dma_complete: polling longer than %d cycles (delay %d ns) for dma complete!!!\n",PEXOR_DMA_MAXPOLLS, PEXOR_DMA_POLLDELAY );
      return -EFAULT;
    }
    if (PEXOR_DMA_POLLDELAY)
      ndelay(PEXOR_DMA_POLLDELAY);
#ifdef  PEXOR_DMA_POLL_SCHEDULE
    if (doschedule)
          schedule (); // never do this in irq tasklet!
#endif
  };    // while

  pexor_dma_unlock((&(priv->dma_lock)));
  //spin_unlock(&(priv->dma_lock));
  return 0;
}

int pexor_receive_dma_buffer (struct pexor_privdata *privdata, unsigned long used_size, u32 triggerstatus)
{
  int state, rev = 0;
  struct pexor_dmabuf* nextbuf;
  if ((rev = pexor_poll_dma_complete (privdata,1)) != 0)
    return rev;

  /* transfer buffer from free queue to receive queue*/
  spin_lock( &(privdata->buffers_lock));

  /* check if free list is empty <- can happen if dma flow gets suspended
   * and waitreceive is called in polling mode*/
  if (list_empty (&(privdata->free_buffers)))
  {
    spin_unlock( &(privdata->buffers_lock));
    pexor_dbg(KERN_ERR "pexor_receive_dma_buffer: list of free buffers is empty. no DMA could have been received!\n");
    /* return;  this would put the waitreceive into timeout, so does not try to read empty receive queue*/
    goto wakeup;
    /* to have immediate response and error from receive queue as well.*/
  }

  nextbuf=list_first_entry(&(privdata->free_buffers), struct pexor_dmabuf, queue_list);
  list_move_tail (&(nextbuf->queue_list), &(privdata->received_buffers));
  spin_unlock( &(privdata->buffers_lock));

  /** JAM TODO: do we still need dma flow states and states anyway?*/
  state = atomic_read(&(privdata->state));
  switch (state)
  {
    case PEXOR_STATE_STOPPED:
      /* this can happen after processing trigger 15 in irq tasklet.
       * we just mark dummy buffer with trigger type and then wake up consumer:*/
      nextbuf->used_size = 0; /* need to tell consumer the real token data size*/
      nextbuf->triggerstatus = triggerstatus; /* pass up triggerstatus for this data!*/
      break;

    case PEXOR_STATE_DMA_FLOW:
      /*if(atomic_read(&(privdata->dma_outstanding))>PEXOR_MAXOUTSTANDING)
 {
 pexor_msg(KERN_ALERT "pexor_irq_tasklet finds more than %d pending receive buffers! Emergency suspend dma flow!\n",PEXOR_MAXOUTSTANDING);
 atomic_set(&(privdata->state),PEXOR_STATE_DMA_SUSPENDED);
 break;
 }*/
      rev = pexor_next_dma (privdata, 0, 0, 0, 0, 0, 0, 0); /* TODO: inc source address cursor? Handle sfp double buffering?*/
      if (rev)
      {
        /* no more dma buffers at the moment: suspend flow?*/
        atomic_set(&(privdata->state), PEXOR_STATE_DMA_SUSPENDED);
        pexor_dbg(KERN_ALERT "pexor_receive_dma_buffer suspends DMA flow because no more free buffers!\n");
      }
      break;

    case PEXOR_STATE_TRIGGERED_READ:
      /* in case of auto trigger readout, do not change state to stopped.
       * we continue this mode until user stops acquisition*/
      if (used_size > nextbuf->size)
      {
        pexor_msg(KERN_ALERT "pexor_receive_dma_buffer - used size:%ld exceeds kernel buffer size:%ld, truncating!\n",
            used_size, nextbuf->size);
        used_size = nextbuf->size;
      }
      nextbuf->used_size = used_size; /* need to tell consumer the real token data size*/
      nextbuf->triggerstatus = triggerstatus; /* remember triggerstatus for this data!*/

      break;

    case PEXOR_STATE_DMA_SINGLE:
    default:
      atomic_set(&(privdata->state), PEXOR_STATE_STOPPED);

  };

  wakeup:
  /* wake up the waiting ioctl*/
  atomic_inc (&(privdata->dma_outstanding));
  wake_up_interruptible (&(privdata->irq_dma_queue));

  return rev;
}

int pexor_wait_dma_buffer (struct pexor_privdata* priv, struct pexor_dmabuf* result)
{
  unsigned long wjifs = 0;
  struct pexor_dmabuf* dmabuf;

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
  pexor_dbg(
      KERN_NOTICE "** pexor_wait_dma_buffer after wait_event_interruptible_timeout with TIMEOUT %d s (=%d jiffies), waitjiffies=%ld, outstanding=%d \n",   priv->wait_timeout,priv->wait_timeout * HZ, wjifs, atomic_read( &(priv->dma_outstanding)));

  if (wjifs == 0)
  {
    pexor_dbg(
        KERN_NOTICE "** pexor_wait_dma_buffer TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n", priv->wait_timeout * HZ);
    return PEXOR_TRIGGER_TIMEOUT;
  }
  else if (wjifs == -ERESTARTSYS)
  {
    pexor_msg(KERN_NOTICE "** pexor_wait_dma_buffer after wait_event_interruptible_timeout woken by signal. abort wait\n");
    return -EFAULT;
  }
  else
  {
  }

  atomic_dec (&(priv->dma_outstanding));

  /* Take next buffer out of receive queue */
  spin_lock( &(priv->buffers_lock));
  /* need to check here for empty list, since list_first_entry will crash otherwise!*/
  if (list_empty (&(priv->received_buffers)))
  {
    /* this may happen if user calls waitreceive without a DMA been activated, or at flow DMA suspended*/
    spin_unlock( &(priv->buffers_lock));
    pexor_msg(KERN_NOTICE "** pexor_wait_dma_buffer: NEVER COME HERE receive queue is empty after wait\n");
    return -EFAULT;
  }

  dmabuf=list_first_entry(&(priv->received_buffers), struct pexor_dmabuf, queue_list);
  list_move_tail (&(dmabuf->queue_list), &(priv->used_buffers));
  spin_unlock( &(priv->buffers_lock));
  if (dmabuf->dma_addr != 0) /* kernel buffer*/
  {
//    pci_dma_sync_single_for_cpu (priv->pdev, dmabuf->dma_addr, dmabuf->size, PCI_DMA_FROMDEVICE);
//  else
//    /* sg buffer*/
//    pci_dma_sync_sg_for_cpu (priv->pdev, dmabuf->sg, dmabuf->sg_ents, PCI_DMA_FROMDEVICE);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_single_for_cpu( &(priv->pdev->dev), dmabuf->dma_addr, dmabuf->size,DMA_FROM_DEVICE );
#else
  pci_dma_sync_single_for_cpu (priv->pdev, dmabuf->dma_addr, dmabuf->size,PCI_DMA_FROMDEVICE );
#endif
  }
    else
    /* sg buffer*/
  {
    //pci_dma_sync_sg_for_cpu (priv->pdev, dmabuf->sg, dmabuf->sg_ents, PCI_DMA_FROMDEVICE);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
  dma_sync_sg_for_cpu( &(priv->pdev->dev), dmabuf->sg, dmabuf->sg_ents,DMA_FROM_DEVICE );
#else
  pci_dma_sync_sg_for_cpu (priv->pdev, dmabuf->sg, dmabuf->sg_ents,PCI_DMA_FROMDEVICE );
#endif
  }

  *result = *dmabuf;
  //spin_unlock(&(priv->dma_lock));
  return 0;
}

#ifdef PEXOR_SYSFS_ENABLE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
ssize_t pexor_sysfs_freebuffers_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount=0;
  struct pexor_privdata *privdata;
  struct list_head* cursor;
  privdata= (struct pexor_privdata*) dev_get_drvdata(dev);
  spin_lock( &(privdata->buffers_lock) );
  list_for_each(cursor, &(privdata->free_buffers))
  {
    bufcount++;
  }
  spin_unlock( &(privdata->buffers_lock) );
  return snprintf(buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexor_sysfs_usedbuffers_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount = 0;
  struct pexor_privdata *privdata;
  struct list_head* cursor;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
  spin_lock( &(privdata->buffers_lock));
  list_for_each(cursor, &(privdata->used_buffers))
  {
    bufcount++;
  }
  spin_unlock( &(privdata->buffers_lock));
  return snprintf (buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexor_sysfs_rcvbuffers_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  int bufcount = 0;
  struct pexor_privdata *privdata;
  struct list_head* cursor;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
  spin_lock( &(privdata->buffers_lock));
  list_for_each(cursor, &(privdata->received_buffers))
  {
    bufcount++;
  }
  spin_unlock( &(privdata->buffers_lock));
  return snprintf (buf, PAGE_SIZE, "%d\n", bufcount);
}

ssize_t pexor_sysfs_codeversion_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  char vstring[512];
  ssize_t curs = 0;
#ifdef PEXOR_WITH_SFP
  struct dev_pexor* pg;
#endif
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
  curs = snprintf (vstring, 512, "*** This is PEXOR driver by JAM(j.adamczewski@gsi.de) version %s build on %s at %s \n\t", PEXORVERSION, __DATE__,
      __TIME__);
#ifdef PEXOR_WITH_SFP
  pg = &(privdata->pexor);
  pexor_show_version (&(pg->sfp), vstring + curs);
#endif
  return snprintf (buf, PAGE_SIZE, "%s\n", vstring);
}

ssize_t pexor_sysfs_dmaregs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct dev_pexor* pg;
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
  pg = &(privdata->pexor);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEXOR dma/irq register dump:\n");
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma control/status: 0x%x\n", readl(pg->dma_control_stat));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t irq/trixor stat  0x%x\n", readl(pg->irq_status));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t irq/trixor ctrl: 0x%x\n", readl(pg->irq_control));
  pexor_bus_delay();
#ifdef PEXOR_WITH_TRIXOR
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n", readl(pg->trix_fcti));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n", readl(pg->trix_cvti));
  pexor_bus_delay();
#endif
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma source      address: 0x%x\n", readl(pg->dma_source));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma destination address: 0x%x\n", readl(pg->dma_dest));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma length:              0x%x\n", readl(pg->dma_len));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma burst size:          0x%x\n", readl(pg->dma_burstsize));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM start:               0x%x\n", readl(pg->ram_start));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM end:                 0x%x\n", readl(pg->ram_end));
  pexor_bus_delay();
  return curs;
}

#ifdef PEXOR_WITH_SFP

ssize_t pexor_sysfs_sfp_retries_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
   struct pexor_privdata *privdata;
   privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX gosip request retries:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_maxpolls);
   return curs;
}

ssize_t pexor_sysfs_sfp_retries_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   privdata->sfp_maxpolls=val;
   pexor_msg( KERN_NOTICE "PEXOR: sfp maximum retries was set to %d => timeout = %d ns \n", privdata->sfp_maxpolls, (privdata->sfp_maxpolls * PEXOR_SFP_DELAY));
  return count;
}


/* show sfp bus read/write waitstate in nanoseconds.
 * this will impose such wait time after each frontend address read/write ioctl */
ssize_t pexor_sysfs_buswait_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
   struct pexor_privdata *privdata;
   privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX gosip request retries:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_buswait);
   return curs;
}

/* set sfp bus read/write waitstate in nanoseconds. */
ssize_t pexor_sysfs_buswait_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)

{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   privdata->sfp_buswait=val;
   pexor_msg( KERN_NOTICE "PEXOR: gosip bus io wait interval was set to %d microseconds\n", privdata->sfp_buswait);
  return count;
}



ssize_t pexor_sysfs_trixorregs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
#ifdef PEXOR_WITH_TRIXOR
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEXOR trixor register dump:\n");

  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor stat: 0x%x\n", readl(privdata->pexor.irq_status));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor ctrl: 0x%x\n", readl(privdata->pexor.irq_control));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n", readl(privdata->pexor.trix_fcti));
  pexor_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n", readl(privdata->pexor.trix_cvti));
#endif

  return curs;
}

ssize_t pexor_sysfs_trixor_fctime_show (struct device *dev, struct device_attribute *attr, char *buf)
{

  ssize_t curs = 0;
#ifdef PEXOR_WITH_TRIXOR
     struct pexor_privdata *privdata;
     privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
     curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", (0x10000 - readl(privdata->pexor.trix_fcti)));
     pexor_bus_delay();
#endif
     return curs;

}

ssize_t pexor_sysfs_trixor_fctime_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
#ifdef PEXOR_WITH_TRIXOR
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pexor_privdata *privdata;
  privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif


   pexor_bus_delay();
   iowrite32 (0x10000 - val, privdata->pexor.trix_fcti);
   pexor_msg( KERN_NOTICE "PEXOR: trixor fast clear time was set to %d \n", val);
#endif
  return count;
}


ssize_t pexor_sysfs_trixor_cvtime_show (struct device *dev, struct device_attribute *attr, char *buf)
{
    ssize_t curs = 0;
#ifdef PEXOR_WITH_TRIXOR
    struct pexor_privdata *privdata;
    privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
    curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", (0x10000 - readl(privdata->pexor.trix_cvti)));
    pexor_bus_delay();
#endif
    return curs;
}

ssize_t pexor_sysfs_trixor_cvtime_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
#ifdef PEXOR_WITH_TRIXOR
  unsigned int val=0;
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
    int rev=0;
  #else
    char* endp=0;
  #endif
    struct pexor_privdata *privdata;
    privdata = (struct pexor_privdata*) dev_get_drvdata (dev);
  #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
    rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
    if(rev!=0) return rev;
  #else
    val=simple_strtoul(buf,&endp, 0);
    count= endp - buf; // do we need this?
  #endif
     pexor_bus_delay();
     iowrite32 (0x10000 - val, privdata->pexor.trix_cvti);
     pexor_msg( KERN_NOTICE "PEXOR: trixor conversion time was set to %d \n", val);
#endif
    return count;

}






#endif // WITH SFP

#endif // KERNELVERSION CHECK
#endif // PEXOR_SYSFS_ENABLE

#ifdef PEXOR_DEBUGPRINT
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
  pexor_dbg(KERN_NOTICE "\n test_pci found PCI revision number %x \n", get_pci_revision(dev));

  /*********** test the address regions*/
  for (bar = 0; bar < 6; ++bar)
  {
    pexor_dbg(KERN_NOTICE "Resource %d start=%x\n", bar, (unsigned) pci_resource_start( dev,bar ));
    pexor_dbg(KERN_NOTICE "Resource %d end=%x\n", bar, (unsigned) pci_resource_end( dev,bar ));
    pexor_dbg(KERN_NOTICE "Resource %d len=%x\n", bar, (unsigned) pci_resource_len( dev,bar ));
    pexor_dbg(KERN_NOTICE "Resource %d flags=%x\n", bar, (unsigned) pci_resource_flags( dev,bar ));
    if ((pci_resource_flags (dev, bar) & IORESOURCE_IO))
    {
      // Ressource im IO-Adressraum
      pexor_dbg(KERN_NOTICE " - resource is IO\n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_MEM))
    {
      pexor_dbg(KERN_NOTICE " - resource is MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_IO))
    {
      pexor_dbg(KERN_NOTICE " - resource is PCI IO\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_MEMORY))
    {
      pexor_dbg(KERN_NOTICE " - resource is PCI MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      pexor_dbg(KERN_NOTICE " - resource prefetch bit is set \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_64))
    {
      pexor_dbg(KERN_NOTICE " - resource is 64bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_32))
    {
      pexor_dbg(KERN_NOTICE " - resource is 32bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_PREFETCH))
    {
      pexor_dbg(KERN_NOTICE " - resource is prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      pexor_dbg(KERN_NOTICE " - resource is PCI mem prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_1M))
    {
      pexor_dbg(KERN_NOTICE " - resource is PCI memtype below 1M \n");
    }

  }
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, originalvalue);
  pexor_dbg("size of base address 0: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, originalvalue);
  pexor_dbg("size of base address 1: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, originalvalue);
  pexor_dbg("size of base address 2: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, originalvalue);
  pexor_dbg("size of base address 3: %i\n", ~base+1);

  /***** here tests of configuration/status register:******/
  pci_read_config_word (dev, PCI_COMMAND, &comstat);
  pexor_dbg("\n****  Command register is: %d\n", comstat);
  pci_read_config_word (dev, PCI_STATUS, &comstat);
  pexor_dbg("\n****  Status register is: %d\n", comstat);
  pci_read_config_byte (dev, PCI_HEADER_TYPE, &typ);
  pexor_dbg("\n****  Header type is: %d\n", typ);
}

void cleanup_device (struct pexor_privdata* priv)
{
  int j = 0;
  struct pci_dev* pcidev;
#ifdef PEXOR_TRIGSTAT_QUEUE
   unsigned long flags=0;
  struct pexor_trigger_buf* trigstat;
  struct pexor_trigger_buf* nexttrigstat;
#endif
  if (!priv)
    return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (priv->class_dev)
  {
#ifdef PEXOR_SYSFS_ENABLE
#ifdef PEXOR_WITH_SFP
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
    device_destroy (pexor_class, priv->devno);
    priv->class_dev = 0;
  }

#endif

  /* character device cleanup*/
  if (priv->cdev.owner)
    cdev_del (&priv->cdev);
  if (priv->devid)
    atomic_dec (&pexor_numdevs);

  pcidev = priv->pdev;
  if (!pcidev)
    return;

  /* need to explicitely disable interrupt tasklet?*/
  tasklet_kill (&priv->irq_bottomhalf);

#ifdef   PEXOR_TRIGGERLESS_WORKER
  cancel_work_sync(&(priv->triggerless_work));
  flush_workqueue(priv->triggerless_workqueue);
  destroy_workqueue(priv->triggerless_workqueue);
#endif


  /* may put disabling device irqs here?*/
#ifdef PEXOR_ENABLE_IRQ
  free_irq (pcidev->irq, priv);
#endif

#ifdef IRQ_ENABLE_MSI
  pci_disable_msi(pcidev);
#endif

  cleanup_buffers (priv);

#ifdef PEXOR_TRIGSTAT_QUEUE
  // here remove trigger status objects:
  spin_lock_irqsave( &(priv->trigstat_lock),flags);

  list_for_each_entry_safe(trigstat, nexttrigstat, &(priv->trig_status), queue_list)
  {
    list_del(&(trigstat->queue_list)); /* put out of list*/
    kfree(trigstat);
  }
  spin_unlock_irqrestore( &(priv->trigstat_lock),flags);
#endif
  for (j = 0; j < 6; ++j)
  {
    if (priv->bases[j] == 0)
      continue;
    if ((pci_resource_flags (pcidev, j) & IORESOURCE_IO))
    {
      pexor_dbg(KERN_NOTICE " releasing IO region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
      release_region (priv->bases[j], priv->reglen[j]);
    }
    else
    {
      if (priv->iomem[j] != 0)
      {
        pexor_dbg(
            KERN_NOTICE " unmapping virtual MEM region at:%lx -len:%lx \n", (unsigned long) priv->iomem[j], priv->reglen[j]);
        iounmap (priv->iomem[j]);
      }pexor_dbg(KERN_NOTICE " releasing MEM region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
      release_mem_region (priv->bases[j], priv->reglen[j]);
    }
    priv->bases[j] = 0;
    priv->reglen[j] = 0;
  }
  kfree (priv);
  pci_disable_device (pcidev);
}

static int probe (struct pci_dev *dev, const struct pci_device_id *id)
{
  int err = 0, ix = 0;
  u16 vid = 0, did = 0;

  char devnameformat[64];
  char devname[64];
#ifdef PEXOR_TRIGSTAT_QUEUE
  unsigned long flags=0;
  struct pexor_trigger_buf* trigstat;
#endif
#ifdef PEXOR_ENABLE_IRQ
  unsigned char irpin = 0, irline = 0, irnumbercount = 0;
#ifdef PEXOR_WITH_TRIXOR
  int irtype = 0;
#endif
#endif
  struct pexor_privdata *privdata;
  pexor_msg(KERN_NOTICE "PEXOR pci driver starts probe...\n");
  if ((err = pci_enable_device (dev)) != 0)
  {
    pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d enabling PCI device! \n",err);
    return -ENODEV;
  }pexor_dbg(KERN_NOTICE "PEXOR Device is enabled.\n");

  /* Set Memory-Write-Invalidate support */
  if (!pci_set_mwi (dev))
  {
    pexor_dbg(KERN_NOTICE "MWI enabled.\n");
  }
  else
  {
    pexor_dbg(KERN_NOTICE "MWI not supported.\n");
  }
  pci_set_master (dev); /* NNOTE: DMA worked without, but maybe depends on bios...*/

  /* Do we need setting            DMA mask? this part stolen from web:*/

  /*if (!pci_set_dma_mask(dev, DMA_64BIT_MASK) && !pci_set_consistent_dma_mask(
   dev, DMA_64BIT_MASK))
   {
   pexor_dbg(KERN_NOTICE "Set 64 bit DMA mask.\n");
   pci_using_dac = 1;
   }
   else
   {
   err = pci_set_dma_mask(dev, DMA_32BIT_MASK);
   if (err)
   {
   err = pci_set_consistent_dma_mask(dev, DMA_32BIT_MASK);
   if (err)
   {
   pexor_msg( KERN_ERR "No usable DMA , aborting\n");
   return -EIO;
   }pexor_dbg(KERN_NOTICE "Set 32 bit DMA mask.\n");
   }
   pci_using_dac = 0;
   }*/

  /* end stolen*/

  test_pci (dev);

  /* Allocate and initialize the private data for this device */
  privdata = kmalloc (sizeof(struct pexor_privdata), GFP_KERNEL);
  if (privdata == NULL )
  {
    cleanup_device (privdata);
    return -ENOMEM;
  }
  memset (privdata, 0, sizeof(struct pexor_privdata));
  pci_set_drvdata (dev, privdata);
  privdata->pdev = dev;
  
  privdata->wait_timeout = PEXOR_WAIT_TIMEOUT;
  // default values
  // here check which board we have: pexor, pexaria, kinpex
  pci_read_config_word (dev, PCI_VENDOR_ID, &vid);
  pexor_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
  pci_read_config_word (dev, PCI_DEVICE_ID, &did);
  pexor_dbg(KERN_NOTICE "  device id:........0x%x \n", did);
  if (vid == PEXOR_VENDOR_ID && did == PEXOR_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_PEXOR;
    strncpy (devnameformat, PEXORNAMEFMT, 32);
    pexor_msg(KERN_NOTICE "  Found board type PEXOR, vendor id: 0x%x, device id:0x%x\n",vid,did);
  }
  else if (vid == PEXARIA_VENDOR_ID && did == PEXARIA_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_PEXARIA;
    strncpy (devnameformat, PEXARIANAMEFMT, 32);
    pexor_msg(KERN_NOTICE "  Found board type PEXARIA, vendor id: 0x%x, device id:0x%x\n",vid,did);

  }
  else if (vid == KINPEX_VENDOR_ID && did == KINPEX_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_KINPEX;
    strncpy (devnameformat, KINPEXNAMEFMT, 32);
    pexor_msg(KERN_NOTICE "  Found board type KINPEX, vendor id: 0x%x, device id:0x%x\n",vid,did);
  }
  else
  {
    privdata->board_type = BOARDTYPE_PEXOR;
    strncpy (devnameformat, PEXORNAMEFMT, 32);
    pexor_msg(KERN_NOTICE "  Unknown board type, vendor id: 0x%x, device id:0x%x. Assuming pexor mode...\n",vid,did);
  }

  //privdata->magic = PEXOR_DEVICE_ID; /* for isr test TODO: what if multiple pexors share same irq?*/

  atomic_set(&(privdata->state), PEXOR_STATE_STOPPED);

  for (ix = 0; ix < 6; ++ix)
  {
    privdata->bases[ix] = pci_resource_start (dev, ix);
    privdata->reglen[ix] = pci_resource_len (dev, ix);
    if (privdata->bases[ix] == 0)
      continue;
    if ((pci_resource_flags (dev, ix) & IORESOURCE_IO))
    {

      pexor_dbg(KERN_NOTICE " - Requesting io ports for bar %d\n", ix);
      if (request_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        pexor_dbg(KERN_ERR "I/O address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }pexor_dbg( "requested ioport at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
    }
    else if ((pci_resource_flags (dev, ix) & IORESOURCE_MEM))
    {
      pexor_dbg(KERN_NOTICE " - Requesting memory region for bar %d\n", ix);
      if (request_mem_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        pexor_dbg(KERN_ERR "Memory address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }pexor_dbg( "requested memory at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
//JAM2020:Linux tumbleweed at homeoffice shows it!
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,5,19)
      privdata->iomem[ix] = ioremap(privdata->bases[ix], privdata->reglen[ix]);
#else
      privdata->iomem[ix] = ioremap_nocache (privdata->bases[ix], privdata->reglen[ix]);
#endif
      if (privdata->iomem[ix] == NULL )
      {
        pexor_dbg(KERN_ERR "Could not remap memory  at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }pexor_dbg(
          "remapped memory to %lx with length %lx\n", (unsigned long) privdata->iomem[ix], privdata->reglen[ix]);
    }
  }    //for
  
   // initialize maximum polls value:
  privdata->sfp_maxpolls=PEXOR_SFP_MAXPOLLS;
  
  
  set_pexor (&(privdata->pexor), privdata->iomem[0], privdata->bases[0]);
  
  print_pexor (&(privdata->pexor));
  
#ifdef PEXOR_ENABLE_IRQ
  /* reset pexor ir registers if still active from previous crash...*/

#ifdef PEXOR_WITH_TRIXOR
  irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
  iowrite32 (irtype, privdata->pexor.irq_status);
#else
  iowrite32(0, privdata->pexor.irq_control);
#endif
  mb();
  ndelay(20);

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
  init_MUTEX (&(privdata->ramsem));
  init_MUTEX (&(privdata->ioctl_sem));
#else
  sema_init(&(privdata->ramsem),1);
  sema_init (&(privdata->ioctl_sem), 1);
#endif

  /* TODO may use rw semaphore instead? init_rwsem(struct rw_semaphore *sem); */

  spin_lock_init(&(privdata->buffers_lock));
  INIT_LIST_HEAD (&(privdata->free_buffers));
  INIT_LIST_HEAD (&(privdata->received_buffers));
  INIT_LIST_HEAD (&(privdata->used_buffers));

  /* the interrupt related stuff:*/
  spin_lock_init(&(privdata->irq_lock));
  atomic_set(&(privdata->irq_count), 0);
  init_waitqueue_head (&(privdata->irq_dma_queue));
  atomic_set(&(privdata->dma_outstanding), 0);
  init_waitqueue_head (&(privdata->irq_trig_queue));
  atomic_set(&(privdata->trig_outstanding), 0);


#ifdef PEXOR_TRIGSTAT_QUEUE
  spin_lock_init(&(privdata->trigstat_lock));
  INIT_LIST_HEAD (&(privdata->trig_status));
#endif
  atomic_set(&(privdata->trigstat), 0);

  /* TODO: fill ringbuffer of irq status in advance:*/
#ifdef PEXOR_TRIGSTAT_QUEUE
  for (ix = 0; ix < PEXOR_IRSTATBUFFER_SIZE; ++ix)
  {
    trigstat = kmalloc (sizeof(struct pexor_trigger_buf), GFP_KERNEL);
    if (!trigstat)
    {
      pexor_dbg(KERN_ERR "pexor probe: could not alloc triggger status buffer #%d! \n", ix);
      continue;
    }
    memset (trigstat, 0, sizeof(struct pexor_trigger_buf));
    INIT_LIST_HEAD (&(trigstat->queue_list));
    spin_lock_irqsave( &(privdata->trigstat_lock),flags);
    list_add (&(trigstat->queue_list), &(privdata->trig_status));
    spin_unlock_irqrestore( &(privdata->trigstat_lock),flags);
    pexor_dbg(KERN_NOTICE "pexor probe added trigger status buffer #%d .\n", ix);
  }
#endif
  tasklet_init (&(privdata->irq_bottomhalf), pexor_irq_tasklet, (unsigned long) privdata);
  spin_lock_init(&(privdata->dma_lock));

  /** JAM2016 flags for the asynchronous triggerless readout:*/
  for(ix=0; ix<PEXOR_SFP_NUMBER;++ix)
   {
    atomic_set(&(privdata->bufid[ix]), 0);
    atomic_set (&(privdata->sfprequested[ix]), 0);
    atomic_set (&(privdata->sfpreceived[ix]), 0);
   }



#ifdef   PEXOR_TRIGGERLESS_WORKER
  /** CONSTRUCTION AREA for triggerless readout JAM2016: */
  atomic_set(&(privdata->triggerless_acquisition), 0);
  atomic_set(&(privdata->triggerless_ringmode), 0);
  privdata->triggerless_workqueue= create_singlethread_workqueue("PexorQueue"); // TODO: change to new API later
  INIT_WORK(&(privdata->triggerless_work), &pexor_triggerless_workfunc);
#endif
#ifdef   PEXOR_TRIGGERLESS_SPINLOCK
  for(ix=0; ix<PEXOR_SFP_NUMBER;++ix)
    {
      spin_lock_init(&(privdata->sfp_lock[ix]));
      atomic_set(&(privdata->sfp_lock_count[ix]),0);
    }

 #endif
#ifdef   PEXOR_TRIGGERLESS_SEMAPHORE
  for(ix=0; ix<PEXOR_SFP_NUMBER;++ix)
     {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
      init_MUTEX (&(privdata->sfpsem[ix]));
#else
      sema_init(&(privdata->sfpsem[ix]),1);
#endif
      atomic_set(&(privdata->sfp_lock_count[ix]),0);
      atomic_set(&(privdata->sfp_worker_haslock[ix]),0);
     }
#endif

  /* pexor_msg(KERN_NOTICE "Initialized ircount to %d.\n",atomic_read( &(privdata->dma_outstanding)));
   */

#ifdef IRQ_ENABLE_MSI
  /* here try to activate MSI ?*/
  if ((err=pci_enable_msi(dev)) == 0 )
  {
    pexor_dbg(KERN_NOTICE "MSI enabled.\n");
  }
  else
  {
    pexor_dbg(KERN_NOTICE "Failed activating MSI with error %d\n",err);
  }

#endif

#ifdef PEXOR_ENABLE_IRQ
  /* debug: do we have valid ir pins/lines here?*/
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_PIN, &irpin)) != 0)
  {
    pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d getting the PCI interrupt pin \n",err);
  }
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_LINE, &irline)) != 0)
  {
    pexor_msg(KERN_ERR "PEXOR pci driver probe: Error %d getting the PCI interrupt line.\n",err);
  }
  snprintf (privdata->irqname, 64, devnameformat, atomic_read(&pexor_numdevs));

  /*  if(request_irq(dev->irq,  pexor_isr , IRQF_DISABLED | IRQF_SHARED, privdata->irqname, privdata))
   */

  /* test: assign irq from our preset ir line*/

  /*  	dev->irq=irline;


   if(request_irq(dev->irq,  pexor_isr , 0 , privdata->irqname, privdata))
   */

#ifdef PEXOR_SHARED_IRQ
  if (request_irq (dev->irq, pexor_isr, IRQF_SHARED, privdata->irqname, privdata))
  {
    pexor_msg( KERN_ERR "PEXOR pci_drv: IRQ %d not free.\n", dev->irq );
    irnumbercount = 1; /* suppress warnings from unused variable here*/
    cleanup_device (privdata);
    return -EIO;
  }

#else

  dev->irq=irline;
  while(request_irq(dev->irq, pexor_isr , 0 , privdata->irqname, privdata))
  {
    pexor_msg( KERN_ERR "PEXOR pci_drv: IRQ %d not free. try next...\n", dev->irq++ );
    if(irnumbercount++ > 100)
    {
      pexor_msg( KERN_ERR "PEXOR pci_drv: tried to get ir more than %d times, aborting\n", irnumbercount )
          cleanup_device(privdata);
      return -EIO;
    }
  }

#endif

  pexor_msg(KERN_NOTICE " assigned IRQ %d for name %s, pin:%d, line:%d \n",dev->irq, privdata->irqname,irpin,irline);

#endif

  ////////////////// here chardev registering
  privdata->devid = atomic_inc_return(&pexor_numdevs) - 1;
  if (privdata->devid >= PEXOR_MAXDEVS)
  {
    pexor_msg(KERN_ERR "Maximum number of devices reached! Increase MAXDEVICES.\n");
    cleanup_device (privdata);
    return -ENOMSG;
  }

  privdata->devno = MKDEV(MAJOR(pexor_devt), MINOR(pexor_devt) + privdata->devid);

  /* Register character device */
  cdev_init (&(privdata->cdev), &pexor_fops);
  privdata->cdev.owner = THIS_MODULE;
  privdata->cdev.ops = &pexor_fops;
  err = cdev_add (&privdata->cdev, privdata->devno, 1);
  if (err)
  {
    pexor_msg( "Couldn't add character device.\n");
    cleanup_device (privdata);
    return err;
  }

  /* TODO: export special things to class in sysfs ?*/

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (!IS_ERR (pexor_class))
  {
    /* driver init had successfully created class, now we create device:*/

    snprintf (devname, 64, devnameformat, MINOR(pexor_devt) + privdata->devid);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
    privdata->class_dev = device_create (pexor_class, NULL, privdata->devno, privdata, devname);
#else
    privdata->class_dev = device_create(pexor_class, NULL,
        privdata->devno, devname);
#endif	

    dev_set_drvdata (privdata->class_dev, privdata);
    pexor_msg(KERN_NOTICE "Added PEXOR device: %s",devname);

#ifdef PEXOR_SYSFS_ENABLE

    if (device_create_file (privdata->class_dev, &dev_attr_freebufs) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for free buffers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_usedbufs) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for used buffers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_rcvbufs) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for receive buffers.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_codeversion) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for code version.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_dmaregs) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for dma registers.\n");
    }


    if (device_create_file (privdata->class_dev, &dev_attr_trixorregs) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for trixor registers.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_trixorfcti) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for trixor fast clear time.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_trixorcvti) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for trixor conversion time\n");
    }




#ifdef PEXOR_WITH_SFP
    if (device_create_file (privdata->class_dev, &dev_attr_sfpregs) != 0)
    {
      pexor_msg(KERN_ERR "Could not add device file node for sfp registers.\n");
    }
    
    if (device_create_file (privdata->class_dev, &dev_attr_gosipretries) != 0)
        {
             pexor_msg(KERN_ERR "Could not add device file node for gosip retries.\n");
        }
    if (device_create_file (privdata->class_dev, &dev_attr_gosipbuswait) != 0)
      {
        pexor_msg(KERN_ERR "Could not add device file node for gosip bus wait.\n");
      }
#endif

#endif
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

static void remove (struct pci_dev *dev)
{
  struct pexor_privdata* priv = (struct pexor_privdata*) pci_get_drvdata (dev);
  cleanup_device (priv);

  pexor_msg(KERN_NOTICE "PEXOR pci driver end remove.\n");
}

static struct pci_driver pci_driver = { .name = PEXORNAME, .id_table = ids, .probe = probe, .remove = remove, };

static int __init pexor_init (void)
{

  int result;
  pexor_msg(KERN_NOTICE "pexor driver init...\n");

  pexor_devt = MKDEV(my_major_nr, 0);

  /*
   * Register your major, and accept a dynamic number.
   */
  if (my_major_nr)
    result = register_chrdev_region (pexor_devt, PEXOR_MAXDEVS, PEXORNAME);
  else
  {
    result = alloc_chrdev_region (&pexor_devt, 0, PEXOR_MAXDEVS, PEXORNAME);
    my_major_nr = MAJOR(pexor_devt);
  }
  if (result < 0)
    return result;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  #if LINUX_VERSION_CODE <= KERNEL_VERSION(6,4,0)
    pexor_class = class_create (THIS_MODULE, PEXORNAME);
  #else
    pexor_class = class_create (PEXORNAME);
  #endif


  if (IS_ERR (pexor_class))
  {
    pexor_msg(KERN_ALERT "Could not create class for sysfs support!\n");
  }

#endif


  if (pci_register_driver (&pci_driver) < 0)
  {
    pexor_msg(KERN_ALERT "pci driver could not register!\n");
    unregister_chrdev_region (pexor_devt, PEXOR_MAXDEVS);
    return -EIO;
  }

  pexor_msg(KERN_NOTICE "\t\tdriver init with registration for major no %d done.\n",my_major_nr);
  return 0;

  /* note: actual assignment will be done on probe time*/

}

static void __exit pexor_exit (void)
{
  pexor_msg(KERN_NOTICE "pexor driver exit...\n");

  unregister_chrdev_region (pexor_devt, PEXOR_MAXDEVS);
  pci_unregister_driver (&pci_driver);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (pexor_class != NULL )
    class_destroy (pexor_class);
#endif

  pexor_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joern Adamczewski-Musch");

module_init(pexor_init);
module_exit(pexor_exit);
