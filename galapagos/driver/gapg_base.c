// N.Kurz, EE, GSI, 8-Apr-2010
// J.Adamczewski-Musch, EE, GSI, added mmap and some small fixes 24-Jan-2013
// JAM added generic probe/cleanup, privdata structure, sysfs, etc.  28-Jan-2013
// JAM merge gapgormbs driver with large gapgor to mbsgapg driver 8-Apr-2014
//-----------------------------------------------------------------------------

#include "gapg_base.h"

/** hold full device number */
static dev_t gapg_devt;
static atomic_t gapg_numdevs = ATOMIC_INIT(0);
static int my_major_nr = 0;

MODULE_AUTHOR("Joern Adamczewski-Musch, EE, GSI, 05-Aug-2019");
MODULE_LICENSE("Dual BSD/GPL");


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* gapg_class;
static DEVICE_ATTR(codeversion, S_IRUGO, gapg_sysfs_codeversion_show, NULL);
static DEVICE_ATTR(gapgregs, S_IRUGO, gapg_sysfs_gapgregs_show, NULL);
static DEVICE_ATTR(bar0base, S_IRUGO, gapg_sysfs_bar0base_show, NULL);
//static DEVICE_ATTR(gapgretries, (S_IWUSR| S_IWGRP | S_IRUGO) , gapg_sysfs_sfp_retries_show, gapg_sysfs_sfp_retries_store);
static DEVICE_ATTR(gapgbuswait, (S_IWUSR| S_IWGRP | S_IRUGO) , gapg_sysfs_buswait_show, gapg_sysfs_buswait_store);


ssize_t gapg_sysfs_gapgregs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct gapg_privdata *privdata;
   privdata = (struct gapg_privdata*) dev_get_drvdata (dev);
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** GAPG register dump:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM start: 0x%x\n", readl(privdata->regs.ram_start));
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM endt:  0x%x\n", readl(privdata->regs.ram_end));

//#ifdef GAPG_WITH_TRIXOR
//  struct gapg_privdata *privdata;
//  privdata = (struct gapg_privdata*) dev_get_drvdata (dev);
//  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** GAPG trixor register dump:\n");
//
//  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor stat: 0x%x\n", readl(privdata->regs.irq_status));
//  gapg_bus_delay();
//  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor ctrl: 0x%x\n", readl(privdata->regs.irq_control));
//  gapg_bus_delay();
//  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n", readl(privdata->regs.trix_fcti));
//  gapg_bus_delay();
//  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n", readl(privdata->regs.trix_cvti));
//#endif

  return curs;
}

ssize_t gapg_sysfs_bar0base_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct gapg_privdata *privdata;
  privdata = (struct gapg_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "%lx\n", privdata->bases[0]);
  return curs;
}


ssize_t gapg_sysfs_codeversion_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  curs += snprintf (buf + curs, PAGE_SIZE, "*** This is %s, version %s build on %s at %s \n",
      GAPG_DESC, GAPG_VERSION, __DATE__, __TIME__);
   curs += snprintf (buf + curs, PAGE_SIZE, "\tmodule authors: %s \n", GAPG_AUTHORS);
   return curs;
}



//ssize_t gapg_sysfs_sfp_retries_show (struct device *dev, struct device_attribute *attr, char *buf)
//{
//  ssize_t curs = 0;
//   struct gapg_privdata *privdata;
//   privdata = (struct gapg_privdata*) dev_get_drvdata (dev);
//   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** GAPG gapg request retries:\n");
//   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_maxpolls);
//   return curs;
//}
//
//ssize_t gapg_sysfs_sfp_retries_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
//{
//  unsigned int val=0;
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
//  int rev=0;
//#else
//  char* endp=0;
//#endif
//  struct gapg_privdata *privdata;
//  privdata = (struct gapg_privdata*) dev_get_drvdata (dev);
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
//  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
//  if(rev!=0) return rev;
//#else
//  val=simple_strtoul(buf,&endp, 0);
//  count= endp - buf; // do we need this?
//#endif
//   privdata->sfp_maxpolls=val;
//   gapg_msg( KERN_NOTICE "GAPG: sfp maximum retries was set to %d => timeout = %d ns \n", privdata->sfp_maxpolls, (privdata->sfp_maxpolls * GAPG_SFP_DELAY));
//  return count;
//}
//

/* show sfp bus read/write waitstate in nanoseconds.
 * this will impose such wait time after each frontend address read/write ioctl */
ssize_t gapg_sysfs_buswait_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
   struct gapg_privdata *privdata;
   privdata = (struct gapg_privdata*) dev_get_drvdata (dev);
   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** GAPG gapg request retries:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_buswait);
   return curs;
}

/* set sfp bus read/write waitstate in nanoseconds. */
ssize_t gapg_sysfs_buswait_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)

{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct gapg_privdata *privdata;
  privdata = (struct gapg_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   privdata->sfp_buswait=val;
   gapg_msg( KERN_NOTICE "GAPG: gapg bus io wait interval was set to %d microseconds\n", privdata->sfp_buswait);
  return count;
}


#endif // INUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)


void test_pci (struct pci_dev *dev)
{
  int bar = 0;
  u32 originalvalue = 0;
  u32 base = 0;
  u16 comstat = 0;
  u8 typ = 0;
  u8 revision = 0;
  u16 vid = 0;
  u16 did = 0;

  pci_read_config_byte (dev, PCI_REVISION_ID, &revision);
  gapg_dbg( KERN_NOTICE "\n GAPG: test_pci found PCI revision number 0x%x \n", revision);
  pci_read_config_word (dev, PCI_VENDOR_ID, &vid);
  gapg_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
  pci_read_config_word (dev, PCI_DEVICE_ID, &did);
  gapg_dbg(KERN_NOTICE "  device id:........0x%x \n", did);

  /*********** test the address regions*/
  for (bar = 0; bar < 6; ++bar)
  {
    gapg_dbg( KERN_NOTICE "Resource %d start=%x\n", bar, (unsigned) pci_resource_start( dev,bar ));
    gapg_dbg( KERN_NOTICE "Resource %d end=%x\n", bar, (unsigned) pci_resource_end( dev,bar ));
    gapg_dbg( KERN_NOTICE "Resource %d len=%x\n", bar, (unsigned) pci_resource_len( dev,bar ));
    gapg_dbg( KERN_NOTICE "Resource %d flags=%x\n", bar, (unsigned) pci_resource_flags( dev,bar ));
    if ((pci_resource_flags (dev, bar) & IORESOURCE_IO))
    {
      // Ressource im IO-Adressraum
      gapg_dbg(KERN_NOTICE " - resource is IO\n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_MEM))
    {
      gapg_dbg(KERN_NOTICE " - resource is MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_IO))
    {
      gapg_dbg(KERN_NOTICE " - resource is PCI IO\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_MEMORY))
    {
      gapg_dbg(KERN_NOTICE " - resource is PCI MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      gapg_dbg(KERN_NOTICE " - resource prefetch bit is set \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_64))
    {
      gapg_dbg(KERN_NOTICE " - resource is 64bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_32))
    {
      gapg_dbg(KERN_NOTICE " - resource is 32bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_PREFETCH))
    {
      gapg_dbg(KERN_NOTICE " - resource is prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      gapg_dbg(KERN_NOTICE " - resource is PCI mem prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_1M))
    {
      gapg_dbg(KERN_NOTICE " - resource is PCI memtype below 1M \n");
    }

  }
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, originalvalue);
  gapg_dbg("size of base address 0: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, originalvalue);
  gapg_dbg("size of base address 1: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, originalvalue);
  gapg_dbg("size of base address 2: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, originalvalue);
  gapg_dbg("size of base address 3: %i\n", ~base+1);

  /***** here tests of configuration/status register:******/
  pci_read_config_word (dev, PCI_COMMAND, &comstat);
  gapg_dbg("\n****  Command register is: %d\n", comstat);
  pci_read_config_word (dev, PCI_STATUS, &comstat);
  gapg_dbg("\n****  Status register is: %d\n", comstat);
  pci_read_config_byte (dev, PCI_HEADER_TYPE, &typ);
  gapg_dbg("\n****  Header type is: %d\n", typ);

}

void clear_pointers (struct regs_gapg* pg)
{
  if (pg == 0)
    return;
  pg->init_done = 0x0;
  gapg_dbg(KERN_NOTICE "** Cleared gapg pointer structure %lx.\n", (long unsigned int) pg);
}

void set_pointers (struct regs_gapg* pg, void* membase, unsigned long bar)
{

//  void* dmabase = 0;
  if (pg == 0)
    return;
// JAM 2019 - for the moment, we just test with a pexor+trixor if this driver works-
//  dmabase = membase + GAPG_DMA_BASE;
//#ifdef GAPG_WITH_TRIXOR
  pg->irq_control = (u32*) (membase + GAPG_TRIXOR_BASE + GAPG_TRIX_CTRL);
  pg->irq_status = (u32*) (membase + GAPG_TRIXOR_BASE + GAPG_TRIX_STAT);
//  pg->trix_fcti = (u32*) (membase + GAPG_TRIXOR_BASE + GAPG_TRIX_FCTI);
//  pg->trix_cvti = (u32*) (membase + GAPG_TRIXOR_BASE + GAPG_TRIX_CVTI);
//#else
//  pg->irq_control=(u32*)(membase+GAPG_IRQ_CTRL);
//  pg->irq_status=(u32*)(membase+GAPG_IRQ_STAT);
//#endif
//
//  pg->dma_control_stat = (u32*) (dmabase + GAPG_DMA_CTRLSTAT);
//  pg->dma_source = (u32*) (dmabase + GAPG_DMA_SRC);
//  pg->dma_dest = (u32*) (dmabase + GAPG_DMA_DEST);
//  pg->dma_len = (u32*) (dmabase + GAPG_DMA_LEN);
//  pg->dma_burstsize = (u32*) (dmabase + GAPG_DMA_BURSTSIZE);

  pg->ram_start = (u32*) (membase + GAPG_DRAM);
  pg->ram_end = (u32*) (membase + GAPG_DRAM + GAPG_RAMSIZE);

//  pg->ram_dma_base = (dma_addr_t) (bar + GAPG_DRAM);
//  pg->ram_dma_cursor = (dma_addr_t) (bar + GAPG_DRAM);
//  set_sfp (&(pg->sfp), membase, bar);

  pg->init_done = 0x1;
  gapg_dbg(KERN_NOTICE "** Set gapg structure %lx.\n", (long unsigned int) pg);

}

void print_register (const char* description, u32* address)
{
  gapg_dbg(KERN_NOTICE "%s:\taddr=%lx cont=%x\n", description, (long unsigned int) address, readl(address));
  gapg_bus_delay();
}

void print_regs (struct regs_gapg* pg)
{
  if (pg == 0)
  return;gapg_dbg(KERN_NOTICE "\n##print_gapg: ###################\n");
  gapg_dbg(KERN_NOTICE "init: \t=%x\n", pg->init_done);
  if (!pg->init_done)
    return;
  print_register ("irq status", pg->irq_status);
  print_register ("irq control", pg->irq_control);


  print_register ("RAM start", pg->ram_start);
  print_register ("RAM end", pg->ram_end);


}

//int gapg_start_dma (struct gapg_privdata *priv, dma_addr_t source, dma_addr_t dest, u32 dmasize, u32 channelmask,
//    u32 burstsize)
//{
//  int rev;
//  u32 enable = GAPG_DMA_ENABLED_BIT; /* this will start dma immediately from given source address*/
//  if (burstsize > GAPG_BURST)
//    burstsize = GAPG_BURST;
//  if (channelmask > 1)
//    enable = channelmask; /* set sfp token transfer to initiate the DMA later*/
//
//  if (enable == GAPG_DMA_ENABLED_BIT) // JAM test for nyxor problem: only check previous dma if not in direct dma preparation mode
//  {
//    rev = gapg_poll_dma_complete (priv);
//    if (rev)
//    {
//      gapg_msg(KERN_NOTICE "**gapg_start_dma: dma was not finished, do not start new one!\n");
//      return rev;
//    }
//  }
//  if (burstsize == 0)
//  {
//    /* automatic burst size mode*/
//    burstsize = GAPG_BURST;
//    /* calculate maximum burstsize here:*/
//    while (dmasize % burstsize)
//    {
//      burstsize = (burstsize >> 1);
//    }
//    if (burstsize < GAPG_BURST_MIN)
//    {
//      gapg_dbg(KERN_NOTICE "**gapg_start_dma: correcting for too small burstsize %x\n", burstsize);
//      burstsize = GAPG_BURST_MIN;
//      while (dmasize % burstsize)
//      {
//        dmasize += 2;
//        /* otherwise this can only happen in the last chunk of sg dma.
//         * here it should be no deal to transfer a little bit more...*/
//      }
//      gapg_dbg(
//          KERN_NOTICE "**changed source address to 0x%x, dest:0x%x, dmasize to 0x%x, burstsize:0x%x\n", (unsigned) source, (unsigned) dest, dmasize, burstsize)
//    }
//
//  }    // if automatic burstsizemode
//  gapg_dbg(
//      KERN_NOTICE "#### gapg_start_dma will initiate dma from 0x%x to 0x%x, len=0x%x, burstsize=0x%x...\n", (unsigned) source, (unsigned) dest, dmasize, burstsize);
//
//  iowrite32 (source, priv->regs.dma_source);
//  mb();
//  iowrite32 ((u32) dest, priv->regs.dma_dest);
//  mb();
//  iowrite32 (burstsize, priv->regs.dma_burstsize);
//  mb();
//  iowrite32 (dmasize, priv->regs.dma_len);
//  mb();
//  iowrite32 (enable, priv->regs.dma_control_stat);
//  mb();
//  if (enable > 1)
//  {
//    gapg_dbg(KERN_NOTICE "#### gapg_start_dma sets sfp mask to 0x%x \n", enable);
//  }
//  else
//  {
//    gapg_dbg(KERN_NOTICE "#### gapg_start_dma started dma\n");
//  }
//  return 0;
//}
//
//int gapg_poll_dma_complete (struct gapg_privdata* priv)
//{
//  int loops = 0;
//  u32 enable = GAPG_DMA_ENABLED_BIT;
//   gapg_tdbg(KERN_ERR "gapg_poll_dma_complete: starts...\n");
//
//  while (1)
//  {
//    /* gapg_dbg(KERN_ERR "gapg_poll_dma_complete reading from 0x%p \n",priv->regs.dma_control_stat);*/
//
//    enable = ioread32 (priv->regs.dma_control_stat);
//    mb();
//    //   gapg_dbg(KERN_ERR "gapg_poll_dma_complete sees dmactrl=: 0x%x , looking for %x\n",enable, GAPG_DMA_ENABLED_BIT);
//    if ((enable & GAPG_DMA_ENABLED_BIT) == 0)
//      break;
//    /* poll until the dma bit is cleared => dma complete*/
//
//    //gapg_dbg(KERN_NOTICE "#### gapg_poll_dma_complete wait for dma completion #%d\n",loops);
//    if (loops++ > GAPG_DMA_MAXPOLLS)
//    {
//      gapg_msg(KERN_ERR "gapg_poll_dma_complete: polling longer than %d cycles (delay %d ns) for dma complete!!!\n",GAPG_DMA_MAXPOLLS, GAPG_DMA_POLLDELAY );
//      return -EFAULT;
//    }
//    if (GAPG_DMA_POLLDELAY)
//      ndelay(GAPG_DMA_POLLDELAY);
//    if (GAPG_DMA_POLL_SCHEDULE)
//      schedule ();
//  };
//  gapg_tdbg(KERN_ERR "gapg_poll_dma_complete: returns...\n");
//  return 0;
//}

//-----------------------------------------------------------------------------
int gapg_open (struct inode *inode, struct file *filp)
{
  struct gapg_privdata *dev;      // device information
  gapg_dbg(KERN_INFO "\nBEGIN gapg_open \n");
  dev = container_of(inode->i_cdev, struct gapg_privdata, cdev);
  filp->private_data = dev;    // for other methods
  gapg_dbg(KERN_INFO "END   gapg_open \n");
  return 0;                   // success
}
//-----------------------------------------------------------------------------
int gapg_release (struct inode *inode, struct file *filp)
{
  gapg_dbg(KERN_INFO "BEGIN gapg_release \n");
  gapg_dbg(KERN_INFO "END   gapg_release \n");
  return 0;
}

//--------------------------

int gapg_mmap (struct file *filp, struct vm_area_struct *vma)
{
  struct gapg_privdata *privdata;
//    u64 phstart, phend;
//    unsigned phtype;
  int ret = 0;
  unsigned long bufsize, barsize;
  privdata = (struct gapg_privdata*) filp->private_data;
  gapg_dbg(KERN_NOTICE "** starting gapg_mmap for vm_start=0x%p\n", vma->vm_start);
  if (!privdata)
    return -EFAULT;
  bufsize = (vma->vm_end - vma->vm_start);
  gapg_dbg(KERN_NOTICE "** starting gapg_mmap for size=%ld \n", bufsize);

  if (vma->vm_pgoff == 0)
  {
    /* user does not specify external physical address, we deliver mapping of bar 0:*/
    gapg_dbg(
        KERN_NOTICE "Galapagos is Mapping bar0 base address %x / PFN %x\n", privdata->l_bar0_base, privdata->l_bar0_base >> PAGE_SHIFT);
    barsize = privdata->l_bar0_end - privdata->l_bar0_base;
    if (bufsize > barsize)
    {
      gapg_dbg(
          KERN_WARNING "Requested length %ld exceeds bar0 size, shrinking to %ld bytes\n", bufsize, barsize);
      bufsize = barsize;
    }

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,7,0)
    vma->vm_flags |= (VM_RESERVED);
#else
    vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);
#endif


    ret = remap_pfn_range (vma, vma->vm_start, privdata->l_bar0_base >> PAGE_SHIFT, bufsize, vma->vm_page_prot);
  }
  else
  {
    /* for external phys memory, use directly pfn*/
    gapg_dbg(
        KERN_NOTICE "Galapagos is Mapping external address %lx / PFN %lx\n", (vma->vm_pgoff << PAGE_SHIFT ), vma->vm_pgoff);

    /* JAM tried to check via bios map if the requested region is usable or reserved
     * This will not work, since the e820map as present in Linux kernel was already cut above mem=1024M
     * So we would need to rescan the original bios entries, probably too much effort if standard MBS hardware is known
     * */
    /* phstart=  (u64) vma->vm_pgoff << PAGE_SHIFT;
     phend = phstart +  (u64) bufsize;
     phtype = E820_RAM;
     if(e820_any_mapped(phstart, phend, phtype)==0)
     {
     printk(KERN_ERR "Galapagos mmap: requested physical memory region  from %lx to %lx is not completely usable!\n", (long) phstart, (long) phend);
     return -EFAULT;
     }
     NOTE that e820_any_mapped only checks if _any_ memory inside region is mapped
     So it is the wrong method anyway?*/

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,7,0)
    vma->vm_flags |= (VM_RESERVED);
#else
    vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP);
#endif

    ret = remap_pfn_range (vma, vma->vm_start, vma->vm_pgoff, bufsize, vma->vm_page_prot);

  }

  if (ret)
  {
    gapg_msg(
        KERN_ERR "Galapagos mmap: remap_pfn_range failed with %d\n", ret);
    return -EFAULT;
  }
  return ret;
}

/*********************** IOCTL STUFF ***************************************************************/

int gapg_ioctl_test (struct gapg_privdata* priv, unsigned long arg)
{
  /* curently we test here the pio of gapg ram without copy from user stuff*/
  void* memstart;
  int i, memsize, retval;
  int localbuf = 0;
  retval = get_user(memsize, (int*) arg);
  if (retval)
    return retval;
  memstart = (void*) (priv->regs.ram_start);
  gapg_msg(KERN_NOTICE "gapg_ioctl_test starting to write %d integers to %p\n", memsize, memstart);
  for (i = 0; i < memsize; ++i)
  {
    localbuf = i;
    iowrite32 (localbuf, memstart + (i << 2));
    mb();
    gapg_msg(KERN_NOTICE "%d.. ", i);
    if((i%10)==0) gapg_msg(KERN_NOTICE "\n");
  }
  gapg_msg(KERN_NOTICE "gapg_ioctl_test reading back %d integers from %p\n", memsize, memstart);
  for (i = 0; i < memsize; ++i)
  {
    localbuf = ioread32 (memstart + (i << 2));
    mb();
    if(localbuf!=i)
    gapg_msg(KERN_ERR "Error reading back value %d\n", i);
  }
  gapg_msg(KERN_NOTICE "gapg_ioctl_test finished. \n");
  return 0;
}

int gapg_ioctl_reset (struct gapg_privdata* priv, unsigned long arg)
{
  gapg_dbg(KERN_NOTICE "** gapg_ioctl_reset...\n");

//  gapg_dbg(KERN_NOTICE "Clearing DMA status... \n");
//  iowrite32 (0, priv->regs.dma_control_stat);
//  mb();
//  ndelay(20);
//
//  gapg_sfp_reset (priv);
//  gapg_sfp_clear_all (priv);
  // note that clear sfp channels is done via gapg_ioctl_init_bus

//#ifdef GAPG_WITH_TRIXOR
//  gapg_dbg(KERN_NOTICE "Initalizing TRIXOR... \n");
//
//#ifdef  GAPG_IRQ_WAITQUEUE
//  atomic_set(&(priv->trig_outstanding), 0);
//#endif
//  iowrite32 (TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR, priv->regs.irq_status); /*reset interrupt source*/
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_BUS_DISABLE, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_HALT, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_MASTER, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_CLEAR, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  iowrite32 (0x10000 - 0x20, priv->regs.trix_fcti);
//  mb();
//  ndelay(20);
//  iowrite32 (0x10000 - 0x40, priv->regs.trix_cvti);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_DT_CLEAR, priv->regs.irq_status);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_BUS_ENABLE, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_HALT, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_MASTER, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  iowrite32 (TRIX_CLEAR, priv->regs.irq_control);
//  mb();
//  ndelay(20);
//
//  gapg_dbg(KERN_NOTICE " ... TRIXOR done.\n");
//#else
//
//  iowrite32(0, priv->regs.irq_control);
//  mb();
//  iowrite32(0, priv->regs.irq_status);
//  mb();
//#endif
  print_regs (&(priv->regs));
  return 0;
}

int gapg_ioctl_write_register (struct gapg_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32* ad = 0;
  u32 val = 0;
  int bar = 0;
  struct gapg_reg_io descriptor;
  retval = gapg_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct gapg_reg_io));
  if (retval)
    return retval;


  /* here we assume something for this very connection, to be adjusted later*/
  ad = (u32*) (ptrdiff_t) descriptor.address;
  val = (u32) descriptor.value;
  bar = descriptor.bar;

//  gapg_dbg(KERN_NOTICE "** gapg_ioctl_write_register pretendsto write value %x to address %p within bar %d \n", val, ad, bar);
//  return 0;
  // JAM  aug-2019 -  we  just fake writing for the moment to test application  infrastructure without actual hardware

  if ((bar > 5) || priv->iomem[bar] == 0)
  {
    gapg_msg(KERN_ERR "** gapg_ioctl_write_register: no mapped bar %d\n",bar);
    return -EIO;
  }gapg_dbg(KERN_NOTICE "** gapg_ioctl_write_register writes value %x to address %p within bar %d \n", val, ad, bar);
  if ((unsigned long) ad > priv->reglen[bar])
  {
    gapg_msg(KERN_ERR "** gapg_ioctl_write_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
    return -EIO;
  }
  ad = (u32*) ((unsigned long) priv->iomem[bar] + (unsigned long) ad);
  gapg_dbg(KERN_NOTICE "** gapg_ioctl_write_register writes value %x to mapped PCI address %p !\n", val, ad);
  iowrite32 (val, ad);
  mb();
  ndelay(20);
  return retval;
}

int gapg_ioctl_read_register (struct gapg_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32* ad = 0;
  u32 val = 0;
  int bar = 0;
  struct gapg_reg_io descriptor;
  retval = gapg_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct gapg_reg_io));
  if (retval)
    return retval;
  ad = (u32*) (ptrdiff_t) descriptor.address;
  gapg_dbg(KERN_NOTICE "** gapg_ioctl_reading from register address %p\n", ad);
  bar = descriptor.bar;
//  gapg_dbg(KERN_NOTICE "** gapg_ioctl_read_register pretends to read from address %p within bar %d \n", ad, bar);


  if ((bar > 5) || priv->iomem[bar] == 0)
  {
    gapg_msg(KERN_ERR "** gapg_ioctl_read_register: no mapped bar %d\n",bar);
    return -EIO;
  }
  gapg_dbg(KERN_NOTICE "** gapg_ioctl_read_register reads from address %p within bar %d \n", ad, bar);
  if ((unsigned long) ad > priv->reglen[bar])
  {
    gapg_msg(KERN_ERR "** gapg_ioctl_read_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
    return -EIO;
  }
  ad = (u32*) ((unsigned long) priv->iomem[bar] + (unsigned long) ad);
  val = ioread32 (ad);
  mb();
  ndelay(20);
  gapg_dbg(KERN_NOTICE "** gapg_ioctl_read_register read value %x from mapped PCI address %p !\n", val, ad);


  descriptor.value = val;
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct gapg_reg_io));
  return retval;
}



#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int gapg_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
long gapg_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif

{
  int retval = 0;

  struct gapg_privdata *privdata;

  privdata = (struct gapg_privdata*) filp->private_data;
  gapg_dbg((KERN_INFO "BEGIN gapg_ioctl... \n"));

#ifndef  GAPG_NO_IOCTL_SEM
  /* use semaphore to allow multi user mode:*/
  if (down_interruptible(&(privdata->ioctl_sem)))
  {
    gapg_dbg((KERN_INFO "down interruptible of ioctl sem is not zero, restartsys!\n"));
    return -ERESTARTSYS;
  }
#endif
  switch (cmd)
  {
    case GAPG_IOC_RESET:
    gapg_dbg(KERN_NOTICE "** gapg_ioctl reset\n");
    retval = gapg_ioctl_reset(privdata,arg);
    break;

    case GAPG_IOC_TEST:
    gapg_dbg(KERN_NOTICE "** gapg_ioctl test\n");
    retval = gapg_ioctl_test(privdata, arg);
    break;



    case GAPG_IOC_WRITE_REGISTER:
    gapg_dbg(KERN_NOTICE "** gapg_ioctl write register\n");
    retval = gapg_ioctl_write_register(privdata, arg);
    break;

    case GAPG_IOC_READ_REGISTER:
    gapg_dbg(KERN_NOTICE "** gapg_ioctl read register\n");
    retval = gapg_ioctl_read_register(privdata, arg);
    break;

  }
    gapg_dbg((KERN_INFO "END   gapg_ioctl \n"));

#ifndef  GAPG_NO_IOCTL_SEM
  up(&privdata->ioctl_sem);
#endif

  return retval;
}

//-----------------------------------------------------------------------------
struct file_operations gapg_fops = { .owner = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
    .ioctl = gapg_ioctl,
#else
    .unlocked_ioctl = gapg_ioctl,
#endif
    .mmap = gapg_mmap, .open = gapg_open, .release = gapg_release, };
//-----------------------------------------------------------------------------
static struct pci_device_id ids[] = { { PCI_DEVICE (GAPGOR_VENDOR_ID, GAPGOR_DEVICE_ID), },    // GAPG
    { 0, } };
//-----------------------------------------------------------------------------
MODULE_DEVICE_TABLE(pci, ids);
//-----------------------------------------------------------------------------



/* simple isr from original mbs driver with semaphore:
 * */
irqreturn_t irq_hand (int irq, void *dev_id)
{

  struct gapg_privdata *privdata;
  /*gapg_dbg(KERN_INFO "BEGIN irq_hand \n");*/
  u32 irtype, irmask;
  irmask=(TRIX_EV_IRQ_CLEAR | TRIX_DT_CLEAR);
  privdata = (struct gapg_privdata *) dev_id;
  irtype = ioread32 (privdata->regs.irq_status);
  mb();
//  // JAM test: do not wait when testing status reg
//  //ndelay(200); // ORIG
//  //gapg_dbg(KERN_NOTICE "mbsgapg driver interrupt handler with interrupt status 0x%x!\n", irtype);
//
  if ((irtype & irmask) == irmask) /* test bits, is this interupt from us?*/
  {
    disable_irq_nosync (irq);
    //gapg_dbg(KERN_NOTICE "mbsgapg driver handling interrupt type :0x%x\n", irtype);
    ndelay(200); // JAM test: only wait before writing

    // clear source of pending interrupts (in trixor)
    iowrite32 ((TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR), privdata->regs.irq_status);
    mb ();
    ndelay(1000);
    enable_irq (irq);

    //ndelay (200);

//    privdata->trix_val = 1;
//    up (&(privdata->trix_sem));

    /*    gapg_dbg(KERN_INFO "END   irq_hand \n");*/


    //printk (KERN_INFO "END   irq_hand \n");
    return IRQ_HANDLED;
  }
  else
  {
    //gapg_dbg(KERN_NOTICE "mbsgapg driver unknown irtype 0x%x\n", irtype);
    //enable_irq (irq);
    return IRQ_NONE;
  }
}


void cleanup_device (struct gapg_privdata* priv)
{
  int j = 0;
 // unsigned long arg=0;
  struct pci_dev* pcidev;
  if (!priv)
    return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (priv->class_dev)
  {
//    device_remove_file (priv->class_dev, &dev_attr_trixorbase);
    device_remove_file (priv->class_dev, &dev_attr_bar0base);
    device_remove_file (priv->class_dev, &dev_attr_gapgregs);
    device_remove_file (priv->class_dev, &dev_attr_codeversion);
  //  device_remove_file (priv->class_dev, &dev_attr_sfpregs);
   // device_remove_file (priv->class_dev, &dev_attr_dmaregs);
   // device_remove_file (priv->class_dev, &dev_attr_mbspipe);
   // device_remove_file (priv->class_dev, &dev_attr_gapgretries);
    device_remove_file (priv->class_dev, &dev_attr_gapgbuswait);
    device_destroy (gapg_class, priv->devno);
    priv->class_dev = 0;
  }

#endif



  /* character device cleanup*/
  if (priv->cdev.owner)
    cdev_del (&priv->cdev);
  if (priv->devid)
    atomic_dec (&gapg_numdevs);
  pcidev = priv->pdev;
  if (!pcidev)
    return;
  if (priv->devno) /* misuse devno as flag if we already had successfully requested irq JAM*/
    free_irq (pcidev->irq, priv);
  for (j = 0; j < 6; ++j)
  {
    if (priv->bases[j] == 0)
      continue;
    if ((pci_resource_flags (pcidev, j) & IORESOURCE_IO))
    {
      gapg_dbg(KERN_NOTICE " releasing IO region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
      release_region (priv->bases[j], priv->reglen[j]);
    }
    else
    {
      if (priv->iomem[j] != 0)
      {
        gapg_dbg(
            KERN_NOTICE " unmapping virtual MEM region at:%lx -len:%lx \n", (unsigned long) priv->iomem[j], priv->reglen[j]);
        iounmap (priv->iomem[j]);
      }gapg_dbg(KERN_NOTICE " releasing MEM region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
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
  u16 vid = 0;
  u16 did = 0;
  char devnameformat[64];
  char devname[64];
  struct gapg_privdata *privdata;
  gapg_msg(KERN_NOTICE "GAPG pci driver starts probe...\n");
  if ((err = pci_enable_device (dev)) != 0)
  {
    gapg_msg(
        KERN_ERR "GAPG pci driver probe: Error %d enabling PCI device! \n", err);
    return -ENODEV;
  }gapg_dbg(KERN_NOTICE "GAPG Device is enabled.\n");

  /* Set Memory-Write-Invalidate support */
  if (!pci_set_mwi (dev))
  {
    gapg_dbg(KERN_NOTICE "MWI enabled.\n");
  }
  else
  {
    gapg_dbg(KERN_NOTICE "MWI not supported.\n");
  }
  pci_set_master (dev); /* NNOTE: DMA worked without, but maybe depends on bios...*/
  test_pci (dev);

  /* Allocate and initialize the private data for this device */
  privdata = kmalloc (sizeof(struct gapg_privdata), GFP_KERNEL);
  if (privdata == NULL )
  {
    cleanup_device (privdata);
    return -ENOMEM;
  }
  memset (privdata, 0, sizeof(struct gapg_privdata));
  pci_set_drvdata (dev, privdata);
  privdata->pdev = dev;

  // here check which board we have: gapg, gapgaria, kingapg
  pci_read_config_word (dev, PCI_VENDOR_ID, &vid);
  gapg_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
  pci_read_config_word (dev, PCI_DEVICE_ID, &did);
  gapg_dbg(KERN_NOTICE "  device id:........0x%x \n", did);
  if (vid == GAPGOR_VENDOR_ID && did == GAPGOR_DEVICE_ID)
  {
    //privdata->board_type = BOARDTYPE_GAPGOR;
    strncpy (devnameformat, GAPGNAMEFMT, 32);
    gapg_msg(KERN_NOTICE "  Found board type GALAPAGOS, vendor id: 0x%x, device id:0x%x\n",vid,did);
  }
//  else if (vid == GAPGARIA_VENDOR_ID && did == GAPGARIA_DEVICE_ID)
//  {
//    privdata->board_type = BOARDTYPE_GAPGARIA;
//    strncpy (devnameformat, GAPGARIANAMEFMT, 32);
//    gapg_msg(KERN_NOTICE "  Found board type GAPGARIA, vendor id: 0x%x, device id:0x%x\n",vid,did);
//
//  }
//  else if (vid == KINGAPG_VENDOR_ID && did == KINGAPG_DEVICE_ID)
//  {
//    privdata->board_type = BOARDTYPE_KINGAPG;
//    strncpy (devnameformat, KINGAPGNAMEFMT, 32);
//    gapg_msg(KERN_NOTICE "  Found board type KINGAPG, vendor id: 0x%x, device id:0x%x\n",vid,did);
//  }
  else
  {
    //privdata->board_type = BOARDTYPE_GAPGOR;
    strncpy (devnameformat, GAPGNAMEFMT, 32);
    gapg_msg(KERN_NOTICE "  Unknown board type, vendor id: 0x%x, device id:0x%x. Assuming gapg mode...\n",vid,did);
  }

  for (ix = 0; ix < 6; ++ix)
  {
    privdata->bases[ix] = pci_resource_start (dev, ix);
    privdata->reglen[ix] = pci_resource_len (dev, ix);
    if (privdata->bases[ix] == 0)
      continue;
    /* JAM here workaround for wrong reglen from kingapg baro (old fpga code only!)*/
//    if (privdata->board_type == BOARDTYPE_KINGAPG)
//    {
//      if (privdata->reglen[ix] > GAPG_KINGAPG_BARSIZE)
//      {
//        gapg_dbg(
//            KERN_NOTICE " KINGAPG- Reducing exported barsize 0x%lx to 0x%x\n", privdata->reglen[ix], GAPG_KINGAPG_BARSIZE);
//        privdata->reglen[ix] = GAPG_KINGAPG_BARSIZE;
//      }
//    }
    if (pci_resource_flags (dev, ix) & IORESOURCE_IO)
    {
      gapg_dbg(KERN_NOTICE " - Requesting io ports for bar %d\n", ix);
      if (request_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        gapg_dbg(KERN_ERR "I/O address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }gapg_dbg("requested ioport at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
    }
    else if (pci_resource_flags (dev, ix) & IORESOURCE_MEM)
    {
      gapg_dbg(KERN_NOTICE " - Requesting memory region for bar %d\n", ix);
      if (request_mem_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        gapg_dbg(KERN_ERR "Memory address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }gapg_dbg("requested memory at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
      privdata->iomem[ix] = ioremap_nocache (privdata->bases[ix], privdata->reglen[ix]);
      if (privdata->iomem[ix] == NULL )
      {
        gapg_dbg(KERN_ERR "Could not remap memory  at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }gapg_dbg("remapped memory to %lx with length %lx\n", (unsigned long) privdata->iomem[ix], privdata->reglen[ix]);
    }
  }    //for


  // initialize maximum polls value:
//  privdata->sfp_maxpolls=GAPG_SFP_MAXPOLLS;

// set pointer structures:
  set_pointers (&(privdata->regs), privdata->iomem[0], privdata->bases[0]);

  print_regs (&(privdata->regs));

// semaphore for ioctl access:
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
  init_MUTEX (&(privdata->ioctl_sem));
#else
  sema_init (&(privdata->ioctl_sem), 1);
#endif

// here set custom registers for mbs implementation:
  privdata->l_bar0_base = privdata->bases[0];
  privdata->l_bar0_end = privdata->bases[0] + privdata->reglen[0];
//
//    printk(KERN_INFO " Assigning TRIXOR registers \n");
//  privdata->l_bar0_trix_base = privdata->bases[0] + GAPG_TRIXOR_BASE;

  //    privdata->l_map_bar0_trix_base = (u32) privdata->iomem[0] + BAR0_TRIX_OFF;
//    // map TRIXOR register
//    privdata->pl_stat = (unsigned int*) ((long) privdata->l_map_bar0_trix_base
//            + 0x0);
//    privdata->pl_ctrl = (unsigned int*) ((long) privdata->l_map_bar0_trix_base
//            + 0x4);
//    privdata->pl_fcti = (unsigned int*) ((long) privdata->l_map_bar0_trix_base
//            + 0x8);
//    privdata->pl_cvti = (unsigned int*) ((long) privdata->l_map_bar0_trix_base
//            + 0xc);
//
//    printk(KERN_INFO " Ptr. TRIXOR stat: 0x%x \n",
//            (unsigned int) privdata->pl_stat);
//    printk(KERN_INFO " Ptr. TRIXOR ctrl: 0x%x \n",
//            (unsigned int) privdata->pl_ctrl);
//    printk(KERN_INFO " Ptr. TRIXOR fcti: 0x%x \n",
//            (unsigned int) privdata->pl_fcti);
//    printk(KERN_INFO " Ptr. TRIXOR cvti: 0x%x \n",
//            (unsigned int) privdata->pl_cvti);
//
//    printk(KERN_INFO " TRIXOR registers content \n");
//    printk(KERN_INFO " TRIXOR stat: .... 0x%x \n", ioread32(privdata->pl_stat));
//    printk(KERN_INFO " TRIXOR ctrl: .... 0x%x \n", ioread32(privdata->pl_ctrl));
//    printk(KERN_INFO " TRIXOR fcti: .... 0x%x \n",
//            0x10000 - ioread32(privdata->pl_fcti));
//    printk(KERN_INFO " TRIXOR cvti: .... 0x%x \n",
//            0x10000 - ioread32(privdata->pl_cvti));

//#ifdef GAPG_IRQ_WAITQUEUE
//  init_waitqueue_head(&(privdata->irq_trig_queue));
//  atomic_set(&(privdata->trig_outstanding), 0);
//#else
//  printk(KERN_INFO " Initialize mutex in locked state \n");
//#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
//  init_MUTEX_LOCKED (&(privdata->trix_sem));
//#else
//  sema_init (&(privdata->trix_sem), 0);
//#endif
//
//  privdata->trix_val = 0;
//#endif

  /* debug: do we have valid ir pins/lines here?*/
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_PIN, &(privdata->irqpin))) != 0)
  {
    gapg_msg(
        KERN_ERR "GAPG pci driver probe: Error %d getting the PCI interrupt pin \n", err);
  }
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_LINE, &(privdata->irqline))) != 0)
  {
    gapg_msg(
        KERN_ERR "GAPG pci driver probe: Error %d getting the PCI interrupt line.\n", err);
  }
  snprintf (privdata->irqname, 64, devnameformat, atomic_read(&gapg_numdevs));
  if (request_irq (dev->irq, irq_hand, IRQF_SHARED, privdata->irqname, privdata))
  {
    gapg_msg( KERN_ERR "GAPG pci_drv: IRQ %d not free.\n", dev->irq);
    cleanup_device (privdata);
    return -EIO;
  }
  gapg_msg(
      KERN_NOTICE " assigned IRQ %d for name %s, pin:%d, line:%d \n", dev->irq, privdata->irqname, privdata->irqpin, privdata->irqline);

  ////////////////// here chardev registering
  privdata->devid = atomic_inc_return(&gapg_numdevs) - 1;
  if (privdata->devid >= GAPG_MAXDEVS)
  {
    gapg_msg(
        KERN_ERR "Maximum number of devices reached! Increase MAXDEVICES.\n");
    cleanup_device (privdata);
    return -ENOMSG;
  }

  privdata->devno = MKDEV(MAJOR(gapg_devt), MINOR(gapg_devt) + privdata->devid);

  /* Register character device */
  cdev_init (&(privdata->cdev), &gapg_fops);
  privdata->cdev.owner = THIS_MODULE;
  privdata->cdev.ops = &gapg_fops;
  err = cdev_add (&privdata->cdev, privdata->devno, 1);
  if (err)
  {
    gapg_msg( "Couldn't add character device.\n");
    cleanup_device (privdata);
    return err;
  }

  /* export special things to class in sysfs: */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (!IS_ERR (gapg_class))
  {
    /* driver init had successfully created class, now we create device:*/
    snprintf (devname, 64, devnameformat, MINOR(gapg_devt) + privdata->devid);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
    privdata->class_dev = device_create (gapg_class, NULL, privdata->devno, privdata, devname);
#else
    privdata->class_dev = device_create(gapg_class, NULL,
        privdata->devno, devname);
#endif
    dev_set_drvdata (privdata->class_dev, privdata);
    gapg_msg (KERN_NOTICE "Added GAPG device: %s", devname);

    if (device_create_file (privdata->class_dev, &dev_attr_codeversion) != 0)
    {
      gapg_msg (KERN_ERR "Could not add device file node for code version.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_gapgregs) != 0)
    {
      gapg_msg (KERN_ERR "Could not add device file node for trixor registers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_bar0base) != 0)
    {
      gapg_msg (KERN_ERR "Could not add device file node for bar0 base address.\n");
    }
//    if (device_create_file (privdata->class_dev, &dev_attr_trixorbase) != 0)
//    {
//      gapg_msg (KERN_ERR "Could not add device file node for trixor base address.\n");
//    }
//    if (device_create_file (privdata->class_dev, &dev_attr_dmaregs) != 0)
//    {
//      gapg_msg(KERN_ERR "Could not add device file node for dma registers.\n");
//    }
//
//    if (device_create_file (privdata->class_dev, &dev_attr_sfpregs) != 0)
//    {
//      gapg_msg(KERN_ERR "Could not add device file node for sfp registers.\n");
//    }
//    if (device_create_file (privdata->class_dev, &dev_attr_mbspipe) != 0)
//        {
//          gapg_msg(KERN_ERR "Could not add device file node for mbs pipe dump.\n");
//        }
//    if (device_create_file (privdata->class_dev, &dev_attr_gapgretries) != 0)
//        {
//             gapg_msg(KERN_ERR "Could not add device file node for gapg retries.\n");
//        }
//    if (device_create_file (privdata->class_dev, &dev_attr_gapgbuswait) != 0)
//      {
//        gapg_msg(KERN_ERR "Could not add device file node for gapg bus wait.\n");
//      }



  }
  else
  {
    /* something was wrong at class creation, we skip sysfs device support here:*/
    gapg_msg(KERN_ERR "Could not add GAPG device node to /dev !");
  }

#endif

  gapg_msg(KERN_NOTICE "probe has finished.\n");
  return 0;
}

static void remove (struct pci_dev *dev)
{
  struct gapg_privdata* priv = (struct gapg_privdata*) pci_get_drvdata (dev);
  cleanup_device (priv);
  gapg_msg(KERN_NOTICE "GAPG pci driver end remove.\n");
}

//-----------------------------------------------------------------------------
static struct pci_driver pci_driver = { .name = GAPGNAME, .id_table = ids, .probe = probe, .remove = remove, };
//-----------------------------------------------------------------------------

static int __init gapg_init (void)
{

  int result;
  gapg_msg(KERN_NOTICE "gapg driver init...\n");
  gapg_devt = MKDEV(my_major_nr, 0);

  /*
   * Register your major, and accept a dynamic number.
   */
  if (my_major_nr)
  {
    result = register_chrdev_region (gapg_devt, GAPG_MAXDEVS, GAPGNAME);
  }
  else
  {
    result = alloc_chrdev_region (&gapg_devt, 0, GAPG_MAXDEVS, GAPGNAME);
    my_major_nr = MAJOR(gapg_devt);
  }
  if (result < 0)
  {
    gapg_msg(
        KERN_ALERT "Could not alloc chrdev region for major: %d !\n", my_major_nr);
    return result;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  gapg_class = class_create (THIS_MODULE, GAPGNAME);
  if (IS_ERR (gapg_class))
  {
    gapg_msg(KERN_ALERT "Could not create class for sysfs support!\n");
  }

#endif
  if (pci_register_driver (&pci_driver) < 0)
  {
    gapg_msg(KERN_ALERT "pci driver could not register!\n");
    unregister_chrdev_region (gapg_devt, GAPG_MAXDEVS);
    return -EIO;
  }
  gapg_msg(
      KERN_NOTICE "\t\tdriver init with registration for major no %d done.\n", my_major_nr);
  return 0;

  /* note: actual assignment will be done on probe time*/

}

static void __exit gapg_exit (void)
{
  gapg_msg(KERN_NOTICE "gapg driver exit...\n");
  unregister_chrdev_region (gapg_devt, GAPG_MAXDEVS);
  pci_unregister_driver (&pci_driver);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (gapg_class != NULL )
    class_destroy (gapg_class);
#endif

  gapg_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}

//-----------------------------------------------------------------------------
module_init(gapg_init);
module_exit(gapg_exit);
//-----------------------------------------------------------------------------
