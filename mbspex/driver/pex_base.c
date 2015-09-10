// N.Kurz, EE, GSI, 8-Apr-2010
// J.Adamczewski-Musch, EE, GSI, added mmap and some small fixes 24-Jan-2013
// JAM added generic probe/cleanup, privdata structure, sysfs, etc.  28-Jan-2013
// JAM merge pexormbs driver with large pexor to mbspex driver 8-Apr-2014
//-----------------------------------------------------------------------------

#include "pex_base.h"

/** hold full device number */
static dev_t pex_devt;
static atomic_t pex_numdevs = ATOMIC_INIT(0);
static int my_major_nr = 0;

MODULE_AUTHOR("Nikolaus Kurz, Joern Adamczewski-Musch, EE, GSI, 10-Sep-2015");
MODULE_LICENSE("Dual BSD/GPL");


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* pex_class;
static DEVICE_ATTR(codeversion, S_IRUGO, pex_sysfs_codeversion_show, NULL);
static DEVICE_ATTR(trixorregs, S_IRUGO, pex_sysfs_trixorregs_show, NULL);
static DEVICE_ATTR(bar0base, S_IRUGO, pex_sysfs_bar0base_show, NULL);
static DEVICE_ATTR(trixorbase, S_IRUGO, pex_sysfs_trixorbase_show, NULL);
static DEVICE_ATTR(dmaregs, S_IRUGO, pex_sysfs_dmaregs_show, NULL);
static DEVICE_ATTR(sfpregs, S_IRUGO, pex_sysfs_sfpregs_show, NULL);
static DEVICE_ATTR(mbspipe, S_IRUGO, pex_sysfs_pipe_show, NULL);
static DEVICE_ATTR(gosipretries, S_IWUGO | S_IRUGO , pex_sysfs_sfp_retries_show, pex_sysfs_sfp_retries_store);
static DEVICE_ATTR(gosipbuswait, S_IWUGO | S_IRUGO , pex_sysfs_buswait_show, pex_sysfs_buswait_store);


ssize_t pex_sysfs_trixorregs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
#ifdef PEX_WITH_TRIXOR
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX trixor register dump:\n");

  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor stat: 0x%x\n", readl(privdata->regs.irq_status));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor ctrl: 0x%x\n", readl(privdata->regs.irq_control));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n", readl(privdata->regs.trix_fcti));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n", readl(privdata->regs.trix_cvti));
#endif

  return curs;
}

ssize_t pex_sysfs_bar0base_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "%lx\n", privdata->bases[0]);
  return curs;
}

ssize_t pex_sysfs_trixorbase_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
#ifdef PEX_WITH_TRIXOR
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "%x\n", privdata->l_bar0_trix_base);
#endif
  return curs;
}

ssize_t pex_sysfs_codeversion_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  char vstring[512];
  ssize_t curs = 0;
  struct regs_pex* pg;
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
  curs = snprintf (vstring, 512, "*** This is MBSPEX driver version %s build on %s at %s \n \t", PEXVERSION, __DATE__,
      __TIME__);
  pg = &(privdata->regs);
  pex_show_version (&(pg->sfp), vstring + curs);
  return snprintf (buf, PAGE_SIZE, "%s\n", vstring);
}

ssize_t pex_sysfs_dmaregs_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  struct regs_pex* pg;
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
  pg = &(privdata->regs);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX dma/irq register dump:\n");
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma control/status: 0x%x\n", readl(pg->dma_control_stat));
  pex_bus_delay();
#ifdef PEX_WITH_TRIXOR
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t irq/trixor stat: 0x%x\n", readl(pg->irq_status));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t irq/trixor ctrl: 0x%x\n", readl(pg->irq_control));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n", readl(pg->trix_fcti));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n", readl(pg->trix_cvti));
#endif
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma source      address: 0x%x\n", readl(pg->dma_source));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma destination address: 0x%x\n", readl(pg->dma_dest));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma length:              0x%x\n", readl(pg->dma_len));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t dma burst size:          0x%x\n", readl(pg->dma_burstsize));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM start:               0x%x\n", readl(pg->ram_start));
  pex_bus_delay();
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t RAM end:                 0x%x\n", readl(pg->ram_end));
  return curs;
}

ssize_t pex_sysfs_pipe_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
  int i = 0;
  struct mbs_pipe* pipe;
  struct scatterlist *sg = NULL;
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
  pipe = &(privdata->pipe);
  curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** MBS pipe dump:\n");
  if (pipe->size == 0)
  {
    curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t No virtual pipe is mapped for scatter-gather DMA. Pipe type 4 is required for this!\n");
  }
  else
  {
    curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t virtual start address: 0x%lx\n", pipe->virt_start);
    curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t total length: 0x%lx\n", pipe->size);
    curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t number of pages:     %d\n", pipe->num_pages);
    curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t number of sg chunks: %d\n", pipe->sg_ents);
    for_each_sg(pipe->sg, sg, pipe->sg_ents,i)
    {
      if((i>10) &&  !(pipe->sg_ents - i < 10)) continue; // skip everything except first and last entries
      curs += snprintf (buf + curs, PAGE_SIZE - curs, "\t-- sg chunk %d: start 0x%x length 0x%x \n", i,
          (unsigned) sg_dma_address(sg), sg_dma_len(sg));

      if(PAGE_SIZE - curs < 0) return PAGE_SIZE; // avoid overflowing buffer
    }
  }
  return curs;
}


ssize_t pex_sysfs_sfp_retries_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
   struct pex_privdata *privdata;
   privdata = (struct pex_privdata*) dev_get_drvdata (dev);
   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX gosip request retries:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_maxpolls);
   return curs;
}

ssize_t pex_sysfs_sfp_retries_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   privdata->sfp_maxpolls=val;
   pex_msg( KERN_NOTICE "PEX: sfp maximum retries was set to %d => timeout = %d ns \n", privdata->sfp_maxpolls, (privdata->sfp_maxpolls * PEX_SFP_DELAY));
  return count;
}


/* show sfp bus read/write waitstate in nanoseconds.
 * this will impose such wait time after each frontend address read/write ioctl */
ssize_t pex_sysfs_buswait_show (struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs = 0;
   struct pex_privdata *privdata;
   privdata = (struct pex_privdata*) dev_get_drvdata (dev);
   //curs += snprintf (buf + curs, PAGE_SIZE - curs, "*** PEX gosip request retries:\n");
   curs += snprintf (buf + curs, PAGE_SIZE - curs, "%d\n", privdata->sfp_buswait);
   return curs;
}

/* set sfp bus read/write waitstate in nanoseconds. */
ssize_t pex_sysfs_buswait_store (struct device *dev, struct device_attribute *attr, const char *buf, size_t count)

{
  unsigned int val=0;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  int rev=0;
#else
  char* endp=0;
#endif
  struct pex_privdata *privdata;
  privdata = (struct pex_privdata*) dev_get_drvdata (dev);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
  rev=kstrtouint(buf,0,&val); // this can handle both decimal, hex and octal formats if specified by prefix JAM
  if(rev!=0) return rev;
#else
  val=simple_strtoul(buf,&endp, 0);
  count= endp - buf; // do we need this?
#endif
   privdata->sfp_buswait=val;
   pex_msg( KERN_NOTICE "PEX: gosip bus io wait interval was set to %d microseconds\n", privdata->sfp_buswait);
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
  pex_dbg( KERN_NOTICE "\n PEX: test_pci found PCI revision number 0x%x \n", revision);
  pci_read_config_word (dev, PCI_VENDOR_ID, &vid);
  pex_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
  pci_read_config_word (dev, PCI_DEVICE_ID, &did);
  pex_dbg(KERN_NOTICE "  device id:........0x%x \n", did);

  /*********** test the address regions*/
  for (bar = 0; bar < 6; ++bar)
  {
    pex_dbg( KERN_NOTICE "Resource %d start=%x\n", bar, (unsigned) pci_resource_start( dev,bar ));
    pex_dbg( KERN_NOTICE "Resource %d end=%x\n", bar, (unsigned) pci_resource_end( dev,bar ));
    pex_dbg( KERN_NOTICE "Resource %d len=%x\n", bar, (unsigned) pci_resource_len( dev,bar ));
    pex_dbg( KERN_NOTICE "Resource %d flags=%x\n", bar, (unsigned) pci_resource_flags( dev,bar ));
    if ((pci_resource_flags (dev, bar) & IORESOURCE_IO))
    {
      // Ressource im IO-Adressraum
      pex_dbg(KERN_NOTICE " - resource is IO\n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_MEM))
    {
      pex_dbg(KERN_NOTICE " - resource is MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_IO))
    {
      pex_dbg(KERN_NOTICE " - resource is PCI IO\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_SPACE_MEMORY))
    {
      pex_dbg(KERN_NOTICE " - resource is PCI MEM\n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      pex_dbg(KERN_NOTICE " - resource prefetch bit is set \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_64))
    {
      pex_dbg(KERN_NOTICE " - resource is 64bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_32))
    {
      pex_dbg(KERN_NOTICE " - resource is 32bit address \n");
    }
    if ((pci_resource_flags (dev, bar) & IORESOURCE_PREFETCH))
    {
      pex_dbg(KERN_NOTICE " - resource is prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_PREFETCH))
    {
      pex_dbg(KERN_NOTICE " - resource is PCI mem prefetchable \n");
    }
    if ((pci_resource_flags (dev, bar) & PCI_BASE_ADDRESS_MEM_TYPE_1M))
    {
      pex_dbg(KERN_NOTICE " - resource is PCI memtype below 1M \n");
    }

  }
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_0, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_0, originalvalue);
  pex_dbg("size of base address 0: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_1, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_1, originalvalue);
  pex_dbg("size of base address 1: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_2, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_2, originalvalue);
  pex_dbg("size of base address 2: %i\n", ~base+1);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &originalvalue);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, 0xffffffff);
  pci_read_config_dword (dev, PCI_BASE_ADDRESS_3, &base);
  pci_write_config_dword (dev, PCI_BASE_ADDRESS_3, originalvalue);
  pex_dbg("size of base address 3: %i\n", ~base+1);

  /***** here tests of configuration/status register:******/
  pci_read_config_word (dev, PCI_COMMAND, &comstat);
  pex_dbg("\n****  Command register is: %d\n", comstat);
  pci_read_config_word (dev, PCI_STATUS, &comstat);
  pex_dbg("\n****  Status register is: %d\n", comstat);
  pci_read_config_byte (dev, PCI_HEADER_TYPE, &typ);
  pex_dbg("\n****  Header type is: %d\n", typ);

}

void clear_pointers (struct regs_pex* pg)
{
  if (pg == 0)
    return;
  pg->init_done = 0x0;
  pex_dbg(KERN_NOTICE "** Cleared pex pointer structure %lx.\n", (long unsigned int) pg);
}

void set_pointers (struct regs_pex* pg, void* membase, unsigned long bar)
{

  void* dmabase = 0;
  if (pg == 0)
    return;
  dmabase = membase + PEX_DMA_BASE;
#ifdef PEX_WITH_TRIXOR
  pg->irq_control = (u32*) (membase + PEX_TRIXOR_BASE + PEX_TRIX_CTRL);
  pg->irq_status = (u32*) (membase + PEX_TRIXOR_BASE + PEX_TRIX_STAT);
  pg->trix_fcti = (u32*) (membase + PEX_TRIXOR_BASE + PEX_TRIX_FCTI);
  pg->trix_cvti = (u32*) (membase + PEX_TRIXOR_BASE + PEX_TRIX_CVTI);
#else
  pg->irq_control=(u32*)(membase+PEX_IRQ_CTRL);
  pg->irq_status=(u32*)(membase+PEX_IRQ_STAT);
#endif

  pg->dma_control_stat = (u32*) (dmabase + PEX_DMA_CTRLSTAT);
  pg->dma_source = (u32*) (dmabase + PEX_DMA_SRC);
  pg->dma_dest = (u32*) (dmabase + PEX_DMA_DEST);
  pg->dma_len = (u32*) (dmabase + PEX_DMA_LEN);
  pg->dma_burstsize = (u32*) (dmabase + PEX_DMA_BURSTSIZE);

  pg->ram_start = (u32*) (membase + PEX_DRAM);
  pg->ram_end = (u32*) (membase + PEX_DRAM + PEX_RAMSIZE);
  pg->ram_dma_base = (dma_addr_t) (bar + PEX_DRAM);
  pg->ram_dma_cursor = (dma_addr_t) (bar + PEX_DRAM);
  set_sfp (&(pg->sfp), membase, bar);

  pg->init_done = 0x1;
  pex_dbg(KERN_NOTICE "** Set pex structure %lx.\n", (long unsigned int) pg);

}

void print_register (const char* description, u32* address)
{
  pex_dbg(KERN_NOTICE "%s:\taddr=%lx cont=%x\n", description, (long unsigned int) address, readl(address));
  pex_bus_delay();
}

void print_regs (struct regs_pex* pg)
{
  if (pg == 0)
    return;pex_dbg(KERN_NOTICE "\n##print_pex: ###################\n");
  pex_dbg(KERN_NOTICE "init: \t=%x\n", pg->init_done);
  if (!pg->init_done)
    return;
  print_register ("dma control/status", pg->dma_control_stat);
#ifdef PEX_WITH_TRIXOR
  print_register ("irq status", pg->irq_status);
  print_register ("irq control", pg->irq_control);

  /*pex_dbg(KERN_NOTICE "trixor control add=%x \n",pg->irq_control) ;
   pex_dbg(KERN_NOTICE "trixor status  add =%x \n",pg->irq_status);
   pex_dbg(KERN_NOTICE "trixor fast clear add=%x \n",pg->trix_fcti) ;
   pex_dbg(KERN_NOTICE "trixor conversion time add =%x \n",pg->trix_cvti);*/

  print_register ("trixor fast clear time", pg->trix_fcti);
  print_register ("trixor conversion time", pg->trix_cvti);
#endif

  print_register ("dma source address", pg->dma_source);
  print_register ("dma dest   address", pg->dma_dest);
  print_register ("dma len   address", pg->dma_len);
  print_register ("dma burstsize", pg->dma_burstsize);

  print_register ("RAM start", pg->ram_start);
  print_register ("RAM end", pg->ram_end);
  pex_dbg(KERN_NOTICE "RAM DMA base add=%x \n", (unsigned) pg->ram_dma_base);
  pex_dbg(KERN_NOTICE "RAM DMA cursor add=%x \n", (unsigned) pg->ram_dma_cursor);

  print_sfp (&(pg->sfp));

}

int pex_start_dma (struct pex_privdata *priv, dma_addr_t source, dma_addr_t dest, u32 dmasize, u32 channelmask,
    u32 burstsize)
{
  int rev;
  u32 enable = PEX_DMA_ENABLED_BIT; /* this will start dma immediately from given source address*/
  if (burstsize > PEX_BURST)
    burstsize = PEX_BURST;
  if (channelmask > 1)
    enable = channelmask; /* set sfp token transfer to initiate the DMA later*/

  if (enable < 1) // JAM test for nyxor problem: only check previous dma if not in direct dma preparation mode
  {
    rev = pex_poll_dma_complete (priv);
    if (rev)
    {
      pex_msg(KERN_NOTICE "**pex_start_dma: dma was not finished, do not start new one!\n");
      return rev;
    }
  }
  if (burstsize == 0)
  {
    /* automatic burst size mode*/
    burstsize = PEX_BURST;
    /* calculate maximum burstsize here:*/
    while (dmasize % burstsize)
    {
      burstsize = (burstsize >> 1);
    }
    if (burstsize < PEX_BURST_MIN)
    {
      pex_dbg(KERN_NOTICE "**pex_start_dma: correcting for too small burstsize %x\n", burstsize);
      burstsize = PEX_BURST_MIN;
      while (dmasize % burstsize)
      {
        dmasize += 2;
        /* otherwise this can only happen in the last chunk of sg dma.
         * here it should be no deal to transfer a little bit more...*/
      }
      pex_dbg(
          KERN_NOTICE "**changed source address to 0x%x, dest:0x%x, dmasize to 0x%x, burstsize:0x%x\n", (unsigned) source, (unsigned) dest, dmasize, burstsize)
    }

  }    // if automatic burstsizemode
  pex_dbg(
      KERN_NOTICE "#### pex_start_dma will initiate dma from 0x%x to 0x%x, len=0x%x, burstsize=0x%x...\n", (unsigned) source, (unsigned) dest, dmasize, burstsize);

  iowrite32 (source, priv->regs.dma_source);
  mb();
  iowrite32 ((u32) dest, priv->regs.dma_dest);
  mb();
  iowrite32 (burstsize, priv->regs.dma_burstsize);
  mb();
  iowrite32 (dmasize, priv->regs.dma_len);
  mb();
  iowrite32 (enable, priv->regs.dma_control_stat);
  mb();
  if (enable > 1)
  {
    pex_dbg(KERN_NOTICE "#### pex_start_dma sets sfp mask to 0x%x \n", enable);
  }
  else
  {
    pex_dbg(KERN_NOTICE "#### pex_start_dma started dma\n");
  }
  return 0;
}

int pex_poll_dma_complete (struct pex_privdata* priv)
{
  int loops = 0;
  u32 enable = PEX_DMA_ENABLED_BIT;
   pex_tdbg(KERN_ERR "pex_poll_dma_complete: starts...\n");
   
  while (1)
  {
    /* pex_dbg(KERN_ERR "pex_poll_dma_complete reading from 0x%p \n",priv->regs.dma_control_stat);*/

    enable = ioread32 (priv->regs.dma_control_stat);
    mb();
    //   pex_dbg(KERN_ERR "pex_poll_dma_complete sees dmactrl=: 0x%x , looking for %x\n",enable, PEX_DMA_ENABLED_BIT);
    if ((enable & PEX_DMA_ENABLED_BIT) == 0)
      break;
    /* poll until the dma bit is cleared => dma complete*/

    //pex_dbg(KERN_NOTICE "#### pex_poll_dma_complete wait for dma completion #%d\n",loops);
    if (loops++ > PEX_DMA_MAXPOLLS)
    {
      pex_msg(KERN_ERR "pex_poll_dma_complete: polling longer than %d cycles (delay %d ns) for dma complete!!!\n",PEX_DMA_MAXPOLLS, PEX_DMA_POLLDELAY );
      return -EFAULT;
    }
    if (PEX_DMA_POLLDELAY)
      ndelay(PEX_DMA_POLLDELAY);
    if (PEX_DMA_POLL_SCHEDULE)
      schedule ();
  };
  pex_tdbg(KERN_ERR "pex_poll_dma_complete: returns...\n");
  return 0;
}

//-----------------------------------------------------------------------------
int pex_open (struct inode *inode, struct file *filp)
{
  struct pex_privdata *dev;      // device information
  pex_dbg(KERN_INFO "\nBEGIN pex_open \n");
  dev = container_of(inode->i_cdev, struct pex_privdata, cdev);
  filp->private_data = dev;    // for other methods
  pex_dbg(KERN_INFO "END   pex_open \n");
  return 0;                   // success
}
//-----------------------------------------------------------------------------
int pex_release (struct inode *inode, struct file *filp)
{
  pex_dbg(KERN_INFO "BEGIN pex_release \n");
  pex_dbg(KERN_INFO "END   pex_release \n");
  return 0;
}

//--------------------------

int pex_mmap (struct file *filp, struct vm_area_struct *vma)
{
  struct pex_privdata *privdata;
//    u64 phstart, phend;
//    unsigned phtype;
  int ret = 0;
  unsigned long bufsize, barsize;
  privdata = (struct pex_privdata*) filp->private_data;
  pex_dbg(KERN_NOTICE "** starting pex_mmap...\n");
  if (!privdata)
    return -EFAULT;
  bufsize = (vma->vm_end - vma->vm_start);
  pex_dbg(KERN_NOTICE "** starting pex_mmap for size=%ld \n", bufsize);

  if (vma->vm_pgoff == 0)
  {
    /* user does not specify external physical address, we deliver mapping of bar 0:*/
    pex_dbg(
        KERN_NOTICE "Pexor is Mapping bar0 base address %x / PFN %x\n", privdata->l_bar0_base, privdata->l_bar0_base >> PAGE_SHIFT);
    barsize = privdata->l_bar0_end - privdata->l_bar0_base;
    if (bufsize > barsize)
    {
      pex_msg(
          KERN_WARNING "Requested length %ld exceeds bar0 size, shrinking to %ld bytes\n", bufsize, barsize);
      bufsize = barsize;
    }

    vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
    ret = remap_pfn_range (vma, vma->vm_start, privdata->l_bar0_base >> PAGE_SHIFT, bufsize, vma->vm_page_prot);
  }
  else
  {
    /* for external phys memory, use directly pfn*/
    pex_dbg(
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

    vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
    ret = remap_pfn_range (vma, vma->vm_start, vma->vm_pgoff, bufsize, vma->vm_page_prot);

  }

  if (ret)
  {
    pex_msg(
        KERN_ERR "Pexor mmap: remap_pfn_range failed with %d\n", ret);
    return -EFAULT;
  }
  return ret;
}

/*********************** IOCTL STUFF ***************************************************************/

int pex_ioctl_test (struct pex_privdata* priv, unsigned long arg)
{
  /* curently we test here the pio of pex ram without copy from user stuff*/
  void* memstart;
  int i, memsize, retval;
  int localbuf = 0;
  retval = get_user(memsize, (int*) arg);
  if (retval)
    return retval;
  memstart = (void*) (priv->regs.ram_start);
  pex_msg(KERN_NOTICE "pex_ioctl_test starting to write %d integers to %p\n", memsize, memstart);
  for (i = 0; i < memsize; ++i)
  {
    localbuf = i;
    iowrite32 (localbuf, memstart + (i << 2));
    mb();
    pex_msg(KERN_NOTICE "%d.. ", i);
    if((i%10)==0) pex_msg(KERN_NOTICE "\n");
  }
  pex_msg(KERN_NOTICE "pex_ioctl_test reading back %d integers from %p\n", memsize, memstart);
  for (i = 0; i < memsize; ++i)
  {
    localbuf = ioread32 (memstart + (i << 2));
    mb();
    if(localbuf!=i)
    pex_msg(KERN_ERR "Error reading back value %d\n", i);
  }
  pex_msg(KERN_NOTICE "pex_ioctl_test finished. \n");
  return 0;
}

int pex_ioctl_reset (struct pex_privdata* priv, unsigned long arg)
{
  pex_dbg(KERN_NOTICE "** pex_ioctl_reset...\n");

  pex_dbg(KERN_NOTICE "Clearing DMA status... \n");
  iowrite32 (0, priv->regs.dma_control_stat);
  mb();
  ndelay(20);

  pex_sfp_reset (priv);
  pex_sfp_clear_all (priv);
  // note that clear sfp channels is done via pex_ioctl_init_bus

#ifdef PEX_WITH_TRIXOR
  pex_dbg(KERN_NOTICE "Initalizing TRIXOR... \n");

#ifdef  PEX_IRQ_WAITQUEUE
  atomic_set(&(priv->trig_outstanding), 0);
#endif
  iowrite32 (TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR, priv->regs.irq_status); /*reset interrupt source*/
  mb();
  ndelay(20);

  iowrite32 (TRIX_BUS_DISABLE, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_HALT, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_MASTER, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_CLEAR, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32 (0x10000 - 0x20, priv->regs.trix_fcti);
  mb();
  ndelay(20);
  iowrite32 (0x10000 - 0x40, priv->regs.trix_cvti);
  mb();
  ndelay(20);

  iowrite32 (TRIX_DT_CLEAR, priv->regs.irq_status);
  mb();
  ndelay(20);

  iowrite32 (TRIX_BUS_ENABLE, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_HALT, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_MASTER, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32 (TRIX_CLEAR, priv->regs.irq_control);
  mb();
  ndelay(20);

  pex_dbg(KERN_NOTICE " ... TRIXOR done.\n");
#else

  iowrite32(0, priv->regs.irq_control);
  mb();
  iowrite32(0, priv->regs.irq_status);
  mb();
#endif
  print_regs (&(priv->regs));
  return 0;
}

int pex_ioctl_write_register (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32* ad = 0;
  u32 val = 0;
  int bar = 0;
  struct pex_reg_io descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_reg_io));
  if (retval)
    return retval;
  /* here we assume something for this very connection, to be adjusted later*/
  ad = (u32*) (ptrdiff_t) descriptor.address;
  val = (u32) descriptor.value;
  bar = descriptor.bar;
  if ((bar > 5) || priv->iomem[bar] == 0)
  {
    pex_msg(KERN_ERR "** pex_ioctl_write_register: no mapped bar %d\n",bar);
    return -EIO;
  }pex_dbg(KERN_NOTICE "** pex_ioctl_write_register writes value %x to address %p within bar %d \n", val, ad, bar);
  if ((unsigned long) ad > priv->reglen[bar])
  {
    pex_msg(KERN_ERR "** pex_ioctl_write_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
    return -EIO;
  }
  ad = (u32*) ((unsigned long) priv->iomem[bar] + (unsigned long) ad);
  pex_dbg(KERN_NOTICE "** pex_ioctl_write_register writes value %x to mapped PCI address %p !\n", val, ad);
  iowrite32 (val, ad);
  mb();
  ndelay(20);
  return retval;
}

int pex_ioctl_read_register (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  u32* ad = 0;
  u32 val = 0;
  int bar = 0;
  struct pex_reg_io descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_reg_io));
  if (retval)
    return retval;
  ad = (u32*) (ptrdiff_t) descriptor.address;
  pex_dbg(KERN_NOTICE "** pex_ioctl_reading from register address %p\n", ad);
  bar = descriptor.bar;
  if ((bar > 5) || priv->iomem[bar] == 0)
  {
    pex_msg(KERN_ERR "** pex_ioctl_read_register: no mapped bar %d\n",bar);
    return -EIO;
  }pex_dbg(KERN_NOTICE "** pex_ioctl_read_register reads from address %p within bar %d \n", ad, bar);
  if ((unsigned long) ad > priv->reglen[bar])
  {
    pex_msg(KERN_ERR "** pex_ioctl_read_register: address %p is exceeding length %lx of bar %d\n",ad, priv->reglen[bar], bar);
    return -EIO;
  }
  ad = (u32*) ((unsigned long) priv->iomem[bar] + (unsigned long) ad);
  val = ioread32 (ad);
  mb();
  ndelay(20);
  pex_dbg(KERN_NOTICE "** pex_ioctl_read_register read value %x from mapped PCI address %p !\n", val, ad);
  descriptor.value = val;
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_reg_io));
  return retval;
}

int pex_ioctl_read_dma (struct pex_privdata* priv, unsigned long arg)
{
  int retval = 0;
  dma_addr_t dmasource, dmadest;
  u32 dmasize, dmaburst;
  struct pex_dma_io descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_dma_io));
  if (retval)
    return retval;
  dmasource = descriptor.source;
  dmadest = descriptor.target;
  dmasize = descriptor.size;
  dmaburst = descriptor.burst;

  if ((retval = pex_start_dma (priv, dmasource, dmadest, dmasize, 1, dmaburst)) != 0)
    return retval;

  if ((retval = pex_poll_dma_complete (priv)) != 0)
    return retval;
  /* find out real package length after dma:*/
  descriptor.size = ioread32 (priv->regs.dma_len);
  pex_bus_delay();
  retval = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_dma_io));

  return retval;
}

int pex_ioctl_read_dma_pipe (struct pex_privdata* priv, unsigned long arg)
{
  int rev = 0, i=0;
  unsigned long virtdest, woffset;
  dma_addr_t dmasource, sgcursor;
  u32 dmasize, dmaburst, sglensum, sglen;
  struct mbs_pipe* pipe;
  struct scatterlist *sgentry = NULL;

#ifndef  PEX_SG_SYNCFULLPIPE
  struct scatterlist *firstentry = NULL;
  int dmaentries=0;
#endif


  struct pex_dma_io descriptor;
  pex_dbg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe starts copy from user\n");
  rev = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_dma_io));

  if (rev)
    return rev;
  pipe = &(priv->pipe);
  dmasource = descriptor.source;
  virtdest = descriptor.virtdest;
  dmasize = descriptor.size;

  pex_dbg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe with source 0x%x, virt dest 0x%lx, size 0x%x, vpipe start:0x%lx\n", (unsigned int) dmasource, virtdest, dmasize, pipe->virt_start);
  // find which sg entry belongs to our start address:

  woffset=virtdest - pipe->virt_start; // write offset relative to pipe begin
                                       // this will only work if calling process is the same that had mapped pipe!
                                       // usually, this is m_read_meb

#ifdef  PEX_SG_SYNCFULLPIPE
  // sync complete pipe buffer to allow device DMA:
  pci_dma_sync_sg_for_device( priv->pdev, pipe->sg, pipe->sg_ents,PCI_DMA_FROMDEVICE );
#endif
  // loop over all sg chunks:
  sgcursor = dmasource;
  sglensum = 0;
  i = 0;
  pex_dbg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe looping sglist with %d entries\n", pipe->sg_ents);

  for_each_sg(pipe->sg,sgentry, pipe->sg_ents,i)
       {
         sglen = sg_dma_len(sgentry);
         if (woffset >= sglen)
         {
           /* find out start segment for pipe write offset and adjust local offset*/
           woffset -= sglen;

           /* limit debug output, otherwise we flood syslogd...*/
           if((i<10) ||  (pipe->sg_ents - i < 10)) {
             pex_dbg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe skipping entry %d, woffset=0x%lx bytes", i,woffset);
           }
           continue;
         }

         sglen -= woffset; /* reduce transfer length from offset to end of first used segment*/
         if (dmasize < sglen)
           sglen = dmasize; /* source buffer fits into first sg page*/
         if (dmasize - sglensum < sglen)
           sglen = dmasize - sglensum; /* cut dma length for last sg page*/

         // adjust burst size to match start address and length within chunk:
         dmaburst = descriptor.burst;
         while (sglen % dmaburst)
         {
             dmaburst = (dmaburst >> 1);
             if(dmaburst == PEX_BURST_MIN)
             {
               pex_msg(
                          KERN_NOTICE "#### pex_ioctl_read_dma_pipe reduced dmaburst to min:0x%x \n", dmaburst);
               break;

             }
         }
         //dmaburst =PEX_BURST_MIN; // JAM test for sg chunks override user given burstsize

         /* DEBUG: pretend to do dma, but do not issue it*/
         pex_dbg(
             KERN_NOTICE "#### pex_ioctl_read_dma_pipe would start dma from 0x%x to 0x%x of length 0x%x, offset 0x%lx, complete chunk length: 0x%x\n", (unsigned) sgcursor, (unsigned) sg_dma_address(sgentry), sglen, woffset, sg_dma_len(sgentry));

         /**** END DEBUG*/

#ifndef  PEX_SG_SYNCFULLPIPE
         // remember which segments we use:
         if(firstentry==NULL)
           firstentry=sgentry;
         dmaentries++;
         // sync each single sg member right before starting dma:
         pci_dma_sync_sg_for_device( priv->pdev, sgentry, 1 ,PCI_DMA_FROMDEVICE );
#endif

         /* initiate dma to next sg part:*/
         if (pex_start_dma (priv, sgcursor, sg_dma_address(sgentry) + woffset, sglen, 0, dmaburst) < 0)
           {
             rev = -EINVAL;
             goto pipe_sync_for_cpu;
           }
         if (woffset > 0)
           woffset = 0; /* reset write offset, once it was applied to first sg segment*/

         if ((rev = pex_poll_dma_complete (priv)) != 0)
         {
           pex_msg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe error on polling for sg entry %d completion, \n", i);
           goto pipe_sync_for_cpu;
         }

         // find out the real transferred length?
         sglen = ioread32 (priv->regs.dma_len);
         pex_bus_delay(); // this could give some time penalty?


         sglensum += sglen;
         if (sglensum >= dmasize)
         {
           pex_dbg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe has finished sg buffer dma after %d segments\n", i);
           break;
         }
         sgcursor += sglen;

       }    // for each sg




  descriptor.size = sglensum;
  rev = copy_to_user ((void __user *) arg, &descriptor, sizeof(struct pex_dma_io));

  if (sglensum < dmasize)
  {
    pex_msg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe could not write full size 0x%x , transferred was: 0x%x\n", dmasize, sglensum);
    pex_msg(KERN_NOTICE "#### -------- at sgentry %d of length 0x%x at dma address 0x%x sgoffs=0x%lx, pipeoffset=0x%lx, dmasource=0x%x, dmacur=0x%x\n", i, (unsigned  int) sg_dma_len(sgentry), (unsigned int)sg_dma_address(sgentry),
        woffset, (virtdest - pipe->virt_start), (unsigned  int) dmasource, (unsigned  int) sgcursor);

    rev= -EINVAL;
  }



#ifndef  PEX_SG_SYNCFULLPIPE
    // only sync parts of pipe that really have been touched:
    pci_dma_sync_sg_for_cpu (priv->pdev, firstentry, dmaentries, PCI_DMA_FROMDEVICE);
    pex_dbg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe has synced for cpu used %d entries\n", dmaentries);
    return rev;
#endif

    pipe_sync_for_cpu:

  // need to sync pipe for cpu after dma is complete:
   pci_dma_sync_sg_for_cpu (priv->pdev, pipe->sg, pipe->sg_ents, PCI_DMA_FROMDEVICE);
   pex_dbg(KERN_NOTICE "#### pex_ioctl_read_dma_pipe has synced for cpu all %d pipe entries\n", pipe->sg_ents);

  return rev;
}


int pex_reduce_sg(struct scatterlist **psg, int entries)
{
  int i,j, first=1;
  unsigned int sglen, sgnewlen=0;
  dma_addr_t sgstart=0, sgnext=0, dmalen=0,newdmalen=0;
  struct scatterlist* sgentry;
  struct scatterlist* sgreduced=NULL;
  struct scatterlist* sgnewentry;

  sgreduced=*psg; // just reuse original list and overwrite
  j=0;
  sgnewentry=*psg; // first init to suppress warnings below
  for_each_sg(*psg , sgentry, entries,i)
        {
          sglen = sgentry->length;       // <- probably redundant, but better to take into account
          dmalen = sg_dma_len(sgentry);
          sgstart=sg_dma_address(sgentry);
          if(first)
          {
            sgnewentry=sgentry;
            first=0;
            pex_dbg(KERN_NOTICE "pex_reduce_sg with first entry at 0x%x..\n", (unsigned) sg_dma_address(sgnewentry));
          }
          else if(sgstart!=sgnext) {
              // this chunk address is separate from the one before, finalize reduced entry:
            sgreduced[j].dma_address=sg_dma_address(sgnewentry); //
            sgreduced[j].dma_length=newdmalen;
            sgreduced[j].length=sgnewlen;
            sgreduced[j].offset=sgnewentry->offset;
            sgreduced[j].page_link=sgnewentry->page_link;
            sgnewlen=0;
            newdmalen=0;
            if(j<20)
            {
              pex_dbg(
                      KERN_NOTICE "-- pex_reduce_sg finalized chunk %d: start 0x%x length 0x%x dmalen 0x%x offset 0x%x , at original entry %d  \n", j, (unsigned) sgreduced[j].dma_address, sgreduced[j].length, sg_dma_len(&sgreduced[j]),sgreduced[j].offset, i);
            }
            j++;
            sgnewentry=sgentry;
          }
          else {}
          sgnewlen+=sglen;
          newdmalen+=dmalen;
          sgnext=sgstart+sglen;
        } // for_each_sg
  // do not forget to put the last coherent segment into new list:
  sgreduced[j].dma_address=sg_dma_address(sgnewentry); //
  sgreduced[j].dma_length=newdmalen;
  sgreduced[j].length=sgnewlen;
  sgreduced[j].offset=sgnewentry->offset;
  sgreduced[j].page_link=sgnewentry->page_link;
  sg_mark_end(&sgreduced[j]);
  pex_dbg(KERN_NOTICE "-- pex_reduce_sg finalized last chunk %d: start 0x%x length 0x%x dmalen 0x%x offset 0x%x , at original entry %d  \n", j, (unsigned) sgreduced[j].dma_address, sgreduced[j].length, sg_dma_len(&sgreduced[j]),sgreduced[j].offset, i);
  j++; // return length, not index!
  pex_dbg(KERN_NOTICE "pex_reduce_sg reduced sglist from %d to %d entries).\n", entries,j);
//  *psg=sgreduced; // exchange pointers not necessary since we reuse original array
  return j;




}




int pex_ioctl_map_pipe (struct pex_privdata *priv, unsigned long arg)
{
  // here evalulate sg list f

  int i, res = 0;
   int nr_pages = 0;
   struct page **pages;
   struct scatterlist *sg = NULL;
   struct scatterlist *sgentry = NULL;
   unsigned int nents;
   unsigned long count, offset, length;
   struct pex_pipebuf pipedesc;
   res = copy_from_user (&pipedesc, (void __user *) arg, sizeof(struct pex_pipebuf));
   if (res)
     return res;
   if (pipedesc.size == 0)
     return -EINVAL;

   /* calculate the number of pages */
     nr_pages = ((pipedesc.addr & ~PAGE_MASK)+ pipedesc.size + ~PAGE_MASK)>>PAGE_SHIFT;
     pex_msg(KERN_NOTICE "nr_pages computed: 0x%x\n", nr_pages);

     /* Allocate space for the page information */
     if ((pages = vmalloc (nr_pages * sizeof(*pages))) == NULL )
         goto mapbuffer_descriptor;
     // JAM todo: reimplement this maybe with newly found
     //  sg_alloc_table_from_pages() (kernel > 3.6 only!)
     // otherwise better use sg_alloc_table and sg_free_table


     /* Allocate space for the scatterlist */
     if ((sg = vmalloc (nr_pages * sizeof(*sg))) == NULL )
       goto mapbuffer_pages;

     sg_init_table (sg, nr_pages);

     /* Get the page information */
     down_read (&current->mm->mmap_sem);
     res = get_user_pages (current, current->mm, pipedesc.addr, nr_pages, 1, 0, pages, NULL );
     up_read (&current->mm->mmap_sem);

     /* Error, not all pages mapped */
     if (res < (int) nr_pages)
     {
       pex_msg(KERN_ERR "Could not map all user pages (0x%x of 0x%x)\n", res, nr_pages);
       /* If only some pages could be mapped, we release those. If a real
        * error occured, we set nr_pages to 0 */
       nr_pages = (res > 0 ? res : 0);
       goto mapbuffer_unmap;
     }

     pex_dbg(KERN_NOTICE "Got the pages (0x%x).\n", res);

     if(pex_ioctl_unmap_pipe(priv,arg)==0)
         pex_msg(KERN_NOTICE "Cleaned up previous pipe \n"); // clean up previous pipe if not done before


      /* populate sg list:*/
      /* page0 is different */

#ifndef     PEX_SG_NO_MEMLOCK
      if (!PageReserved (pages[0]))
        //lock_page_killable(pages[0]);
          __set_page_locked (pages[0]);
      //SetPageLocked(pages[0]);
#endif





      /* for first chunk, we take into account that memory is possibly not starting at
       * page boundary:*/
      offset = (pipedesc.addr & ~PAGE_MASK);
      length = (pipedesc.size > (PAGE_SIZE - offset) ? (PAGE_SIZE - offset) : pipedesc.size);
      sg_set_page (&sg[0], pages[0], length, offset);

      count = pipedesc.size - length;
      for (i = 1; i < nr_pages; i++)
      {

#ifndef     PEX_SG_NO_MEMLOCK
        if (!PageReserved (pages[i]))
          //lock_page_killable(pages[0]); // will block this calling process if used on shared memory! sleep on wait queue
          __set_page_locked (pages[i]); // will block other process that read shared memory,
        //SetPageLocked(pages[i]);
#endif
        sg_set_page (&sg[i], pages[i], ((count > PAGE_SIZE)? PAGE_SIZE : count), 0);
        count -= sg[i].length;
      }

      /* Use the page list to populate the SG list */
      /* SG entries may be merged, res is the number of used entries */
      /* We have originally nr_pages entries in the sg list */
      if ((nents = pci_map_sg (priv->pdev, sg, nr_pages, PCI_DMA_FROMDEVICE)) == 0)
        goto mapbuffer_unmap;

      pex_msg(KERN_NOTICE "Mapped SG list 0x%lx (0x%x entries).\n", (unsigned long) sg, nents);


#ifdef PEX_SG_REDUCE_SGLIST
      // optimize here scatterlist
      res=pex_reduce_sg(&sg,nents);
      if(res<0)
      {
        pex_msg(KERN_NOTICE "Error %d at pex_reduce_sg!!!\n", res);
        goto mapbuffer_unmap;
      }
      nents= res;
      pex_msg(KERN_NOTICE "Reduced to SG list 0x%lx (0x%x entries).\n", (unsigned long) sg, nents);
#endif

      // put sg list to privdata pipe singleton:
      (priv->pipe).num_pages = nr_pages; /* Will be needed when unmapping */
      (priv->pipe).pages = pages;
      (priv->pipe).sg_ents = nents; /* number of coherent dma buffers to transfer*/
      (priv->pipe).sg = sg;

      (priv->pipe).virt_start = pipedesc.addr;
      (priv->pipe).size = pipedesc.size;

      /* DEBUG ****************************************       */
      pex_msg(KERN_NOTICE "\t virtual start address: 0x%lx\n", (priv->pipe).virt_start);
      pex_msg(KERN_NOTICE "\t total length: 0x%lx\n", (priv->pipe).size);
      pex_msg(KERN_NOTICE "\t number of pages:     %d\n", (priv->pipe).num_pages);
      pex_msg(KERN_NOTICE "\t number of sg chunks: %d\n", (priv->pipe).sg_ents);
      for_each_sg((priv->pipe).sg, sgentry, (priv->pipe).sg_ents,i)
      {
        if((i>10) &&  !((priv->pipe).sg_ents - i < 10)) continue; // skip everything except first and last entries
        pex_msg(
            KERN_NOTICE "-- dump sg chunk %d: start 0x%x length 0x%x dmalen 0x%x\n", i, (unsigned) sg_dma_address(sgentry), sgentry->length, sg_dma_len(sgentry));
      }
      /***************************************************/

      return 0;

      mapbuffer_unmap:
      /* release pages */


      for (i = 0; i < nr_pages; i++)
      {
#ifndef     PEX_SG_NO_MEMLOCK
        if (PageLocked (pages[i]))
          //unlock_page(pages[i]);
          __clear_page_locked (pages[i]);
        //ClearPageLocked(pages[i]);
#endif

        if (!PageReserved (pages[i]))
          SetPageDirty (pages[i]);
        page_cache_release(pages[i]);
      }
      vfree (sg);
      mapbuffer_pages: vfree (pages);
      mapbuffer_descriptor: ;

      return -ENOMEM;






  return 0;
}





int pex_ioctl_unmap_pipe (struct pex_privdata *priv, unsigned long arg)
{
  int i = 0;
  if((priv->pipe).size==0) return 1;
   pex_dbg(KERN_NOTICE "**mbspex unmapping sg pipe, size=%ld, user start address=%lx \n", (priv->pipe).size, (priv->pipe).virt_start);
   pci_unmap_sg (priv->pdev, (priv->pipe).sg, (priv->pipe).num_pages, PCI_DMA_FROMDEVICE);
   for (i = 0; i < ((priv->pipe).num_pages); i++)
   {
     if (!PageReserved ((priv->pipe).pages[i]))
     {
       SetPageDirty ((priv->pipe).pages[i]);

#ifndef     PEX_SG_NO_MEMLOCK
       //  unlock_page((priv->pipe).pages[i]);
       __clear_page_locked ((priv->pipe).pages[i]);
       //ClearPageLocked(buf->pages[i]);
#endif
     }
     page_cache_release( (priv->pipe).pages[i]);
   }
   vfree ((priv->pipe).pages);
   vfree ((priv->pipe).sg);
   (priv->pipe).size=0;
  return 0;
}



#ifdef PEX_WITH_TRIXOR
int pex_ioctl_set_trixor (struct pex_privdata* priv, unsigned long arg)
{
  int command, retval;
  struct pex_trixor_set descriptor;
  retval = pex_copy_from_user (&descriptor, (void __user *) arg, sizeof(struct pex_trixor_set));
  if (retval)
    return retval;
  command = descriptor.command;
  switch (command)
  {
    case PEX_TRIX_RES:
      iowrite32 (TRIX_CLEAR, priv->regs.irq_control);
      mb();
      ndelay(20);
      break;

    case PEX_TRIX_GO:
      iowrite32 ((TRIX_EN_IRQ | TRIX_GO), priv->regs.irq_control);
      mb();
      ndelay(20);
      break;

    case PEX_TRIX_HALT:
      iowrite32 (TRIX_HALT, priv->regs.irq_control);
      mb();
      ndelay(20);
      break;

    case PEX_TRIX_TIMESET:
      iowrite32 (0x10000 - descriptor.fct, priv->regs.trix_fcti);
      mb();
      ndelay(20);
      iowrite32 (0x10000 - descriptor.cvt, priv->regs.trix_cvti);
      mb();
      ndelay(20);

      break;

    default:
      pex_dbg(KERN_ERR "pex_ioctl_set_trixor unknown command %x\n", command);
      return -EFAULT;

  };

  return 0;
}

int pex_ioctl_wait_trigger (struct pex_privdata* priv, unsigned long arg)
{
#ifdef  PEX_IRQ_WAITQUEUE
  int wjifs=0;
  wjifs=wait_event_interruptible_timeout( priv->irq_trig_queue, atomic_read( &(priv->trig_outstanding) ) > 0, PEX_WAIT_TIMEOUT );
  pex_dbg(KERN_NOTICE "** pex_wait_trigger after wait_event_interruptible_timeout with TIMEOUT %d, waitjiffies=%d, outstanding=%d \n",PEX_WAIT_TIMEOUT, wjifs, atomic_read( &(priv->trig_outstanding)));
  if(wjifs==0)
  {
    pex_msg(KERN_NOTICE "** pex_wait_trigger TIMEOUT %d jiffies expired on wait_event_interruptible_timeout... \n",PEX_WAIT_TIMEOUT);
    return PEX_TRIGGER_TIMEOUT;
  }
  else if(wjifs==-ERESTARTSYS)
  {
    pex_msg(KERN_NOTICE "** pex_wait_trigger after wait_event_interruptible_timeout woken by signal. abort wait\n");
    return -EFAULT;
  }
  else
  {}
  atomic_dec(&(priv->trig_outstanding));
#endif
  return PEX_TRIGGER_FIRED;
}

#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
int pex_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
#else
long pex_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
#endif

{
  int retval = 0;
#ifdef PEX_WITH_TRIXOR
#ifdef PEX_IRQ_WAITQUEUE
  int trigcount=0;
#endif
#endif

  struct pex_privdata *privdata;

  privdata = (struct pex_privdata*) filp->private_data;
  pex_dbg((KERN_INFO "BEGIN pex_ioctl... \n"));

#ifndef  PEX_NO_IOCTL_SEM
  /* use semaphore to allow multi user mode:*/
  if (down_interruptible(&(privdata->ioctl_sem)))
  {
    pex_dbg((KERN_INFO "down interruptible of ioctl sem is not zero, restartsys!\n"));
    return -ERESTARTSYS;
  }
#endif
  switch (cmd)
  {
    case PEX_IOC_RESET:
    pex_dbg(KERN_NOTICE "** pex_ioctl reset\n");
    retval = pex_ioctl_reset(privdata,arg);
    break;

    case PEX_IOC_TEST:
    pex_dbg(KERN_NOTICE "** pex_ioctl test\n");
    retval = pex_ioctl_test(privdata, arg);
    break;

    case PEX_IOC_WRITE_BUS:
    pex_dbg(KERN_NOTICE "** pex_ioctl write bus\n");
    retval = pex_ioctl_write_bus(privdata, arg);
    break;

    case PEX_IOC_READ_BUS:
    pex_dbg(KERN_NOTICE "** pex_ioctl read bus\n");
    retval = pex_ioctl_read_bus(privdata, arg);
    break;

    case PEX_IOC_INIT_BUS:
    pex_dbg(KERN_NOTICE "** pex_ioctl init bus\n");
    retval = pex_ioctl_init_bus(privdata, arg);
    break;

    case PEX_IOC_CONFIG_BUS:
    pex_dbg(KERN_NOTICE "** pex_ioctl config bus\n");
    retval = pex_ioctl_configure_bus(privdata, arg);
    break;

    case PEX_IOC_REQUEST_TOKEN:
    pex_tdbg(KERN_NOTICE "** pex_ioctl request token\n");
    retval = pex_ioctl_request_token(privdata, arg);
    break;

    case PEX_IOC_WAIT_TOKEN:
    pex_tdbg(KERN_NOTICE "** pex_ioctl wait token\n");
    retval = pex_ioctl_wait_token(privdata, arg);
    break;

    case PEX_IOC_REQUEST_RECEIVE_TOKENS:
    pex_tdbg(KERN_NOTICE "** pex_ioctl request and receive parallel tokens\n");
    retval = pex_ioctl_request_receive_token_parallel(privdata, arg);
    break;


    case PEX_IOC_WRITE_REGISTER:
    pex_dbg(KERN_NOTICE "** pex_ioctl write register\n");
    retval = pex_ioctl_write_register(privdata, arg);
    break;

    case PEX_IOC_READ_REGISTER:
    pex_dbg(KERN_NOTICE "** pex_ioctl read register\n");
    retval = pex_ioctl_read_register(privdata, arg);
    break;

    case PEX_IOC_READ_DMA:
    pex_tdbg(KERN_NOTICE "** pex_ioctl read dma\n");
    retval = pex_ioctl_read_dma(privdata, arg);
    break;

    case PEX_IOC_READ_DMA_PIPE:
      pex_dbg(KERN_NOTICE "** pex_ioctl read dma_pipe\n");
      retval = pex_ioctl_read_dma_pipe(privdata, arg);
      break;

    case PEX_IOC_MAP_PIPE:
      pex_msg(KERN_NOTICE "** pex_ioctl map pipe\n");
      retval = pex_ioctl_map_pipe(privdata, arg);
      break;

    case PEX_IOC_UNMAP_PIPE:
      pex_msg(KERN_NOTICE "** pex_ioctl unmap pipe\n");
      retval = pex_ioctl_unmap_pipe(privdata, arg);
      break;


#ifdef PEX_WITH_TRIXOR
    case PEX_IOC_WAIT_TRIGGER:
    pex_dbg(KERN_NOTICE "** pex_ioctl wait trigger\n");
    retval = pex_ioctl_wait_trigger(privdata, arg);
    break;

    case PEX_IOC_SET_TRIXOR:
    pex_dbg(KERN_NOTICE "** pex_ioctl set trixor\n");
    retval = pex_ioctl_set_trixor(privdata, arg);
    break;

#ifdef  PEX_IRQ_WAITQUEUE
    case WAIT_SEM:
    case PEX_IOC_WAIT_SEM:
    pex_dbg(KERN_INFO "Emulated WAIT_SEM using waitqueu\n");
#ifndef  PEX_NO_IOCTL_SEM
    up(&privdata->ioctl_sem); /* do not lock ioctl during wait on next trigger*/
#endif
    retval = pex_ioctl_wait_trigger(privdata,arg);
    return retval;
    break;
    case POLL_SEM:
    case PEX_IOC_POLL_SEM:
    trigcount=atomic_read( &(privdata->trig_outstanding) );
    pex_dbg(KERN_INFO "Emulated POLL_SEM, trix_val: %d \n", trigcount);
    retval = __put_user(trigcount>0 ? 1 : 0, (int __user *)arg);
    atomic_set(&(privdata->trig_outstanding), 0); /* discard mbs we do not process */
    pex_dbg((KERN_INFO " after POLL_SEM \n"));
    break;
    case RESET_SEM:
    case PEX_IOC_RESET_SEM:
    pex_dbg(KERN_INFO " Emulated RESET_SEM flushes waitqueue:\n");
    while((trigcount=atomic_read( &(privdata->trig_outstanding))>0))
    pex_ioctl_wait_trigger(privdata,arg);
    break;

#else
    case WAIT_SEM:
    case PEX_IOC_WAIT_SEM:
    pex_tdbg(KERN_INFO " before WAIT_SEM \n");
#ifndef  PEX_NO_IOCTL_SEM
    up(&privdata->ioctl_sem); /* do not lock ioctl during wait for next trigger*/
    pex_tdbg(KERN_INFO " after up ioctl_sem WAIT_SEM \n");
#endif
    if (down_interruptible(&(privdata->trix_sem)))
    {
      pex_tdbg((KERN_INFO "down interruptible of trix  sem is not zero, restartsys!\n"));
      return -ERESTARTSYS; /* JAM avoid possible hangup of m_read_meb when killed by resl*/
    }
    privdata->trix_val = 0;
    pex_tdbg((KERN_INFO " after  WAIT_SEM, returning now\n"));
    return retval;
    break;
    case POLL_SEM:
    case PEX_IOC_POLL_SEM:
    pex_tdbg(
        KERN_INFO " before POLL_SEM, trix_val: %ld \n", privdata->trix_val);
    retval = __put_user(privdata->trix_val, (int __user *)arg);
    pex_tdbg((KERN_INFO " after POLL_SEM \n"));
    break;
    case RESET_SEM:
    case PEX_IOC_RESET_SEM:
    pex_tdbg(KERN_INFO " before RESET_SEM \n");
    privdata->trix_val = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
    init_MUTEX_LOCKED(&(privdata->trix_sem));
#else
    sema_init (&(privdata->trix_sem), 0);
#endif
    retval = __put_user(0, (int __user *)arg);
    pex_dbg(KERN_INFO " after  RESET_SEM \n");
    break;

#endif /* irq waitqueue */
#endif /* pex with trixor*/
    case GET_BAR0_BASE:
    case PEX_IOC_GET_BAR0_BASE:
    pex_dbg(KERN_INFO " before GET_BAR0_BASE \n");
    retval = __put_user(privdata->l_bar0_base, (int __user *)arg);
    pex_dbg(KERN_INFO " after  GET_BAR0_BASE \n");
    break;
    case GET_BAR0_TRIX_BASE:
    case PEX_IOC_GET_BAR0_TRIX_BASE:
    pex_dbg(KERN_INFO " before GET_TRIX_BASE \n");
    retval = __put_user(privdata->l_bar0_trix_base, (int __user *)arg);
    pex_dbg(KERN_INFO " after  GET_TRIX_BASE \n");
    break;

    case PEX_IOC_GET_SFP_LINKS:
    pex_dbg(KERN_NOTICE "** pex_ioctl get sfp links\n");
    retval = pex_ioctl_get_sfp_links(privdata, arg);
    break;

    default:
    retval=-ENOTTY;
    break;
  }
  pex_dbg((KERN_INFO "END   pex_ioctl \n"));

#ifndef  PEX_NO_IOCTL_SEM
  up(&privdata->ioctl_sem);
#endif

  return retval;
}

//-----------------------------------------------------------------------------
struct file_operations pex_fops = { .owner = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
    .ioctl = pex_ioctl,
#else
    .unlocked_ioctl = pex_ioctl,
#endif
    .mmap = pex_mmap, .open = pex_open, .release = pex_release, };
//-----------------------------------------------------------------------------
static struct pci_device_id ids[] = { { PCI_DEVICE (PEXOR_VENDOR_ID, PEXOR_DEVICE_ID), },    // PEX
    { PCI_DEVICE (PEXARIA_VENDOR_ID, PEXARIA_DEVICE_ID), },    //pexaria
    { PCI_DEVICE (KINPEX_VENDOR_ID, KINPEX_DEVICE_ID), },    // kinpex
    { 0, } };
//-----------------------------------------------------------------------------
MODULE_DEVICE_TABLE(pci, ids);
//-----------------------------------------------------------------------------

#ifdef PEX_IRQ_WAITQUEUE

/* here alternative isr from full driver with waitqueue:
 * */
irqreturn_t irq_hand( int irq, void *dev_id)
{
#ifdef PEX_WITH_TRIXOR
  u32 irtype;
  struct pex_privdata *privdata;
  privdata=(struct pex_privdata *) dev_id;

  /* check if this interrupt was raised by our device*/
  irtype=ioread32(privdata->regs.irq_status);
  mb();
  ndelay(20);
  if(irtype & (TRIX_EV_IRQ_CLEAR | TRIX_DT_CLEAR)) /* test bits */
  {
//      disable_irq_nosync(irq);
//      ndelay(1000);
    /* prepare for trixor interrupts here:*/
    irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
    iowrite32(irtype, privdata->regs.irq_status); /*reset interrupt source*/
    mb();
    ndelay(20);
//      ndelay(1000);
//      enable_irq(irq);
    /* trigger interrupt from trixor. wake up waiting application if any:*/
    /* pex_dbg(KERN_NOTICE "mbspex driver interrupt handler sees trigger ir!\n"); */
    atomic_inc(&(privdata->trig_outstanding));
    wake_up_interruptible(&(privdata->irq_trig_queue));

#ifdef INTERNAL_TRIG_TEST
    /* from mbs driver irqtest:*/
    /* irtype = TRIX_FC_PULSE;
     iowrite32(irtype, privdata->regs.irq_status);    send fast clear pulse TODO: later in application
     mb();
     ndelay(20);
     irtype = TRIX_DT_CLEAR;
     iowrite32(irtype, privdata->regs.irq_status);   clear deadtime flag TODO: later in application
     mb();
     ndelay(20);*/

#endif
    return IRQ_HANDLED;
  }
  else
  {
    pex_dbg(KERN_NOTICE "mbspex driver interrupt handler sees unknown ir type %x !\n",irtype);
    return IRQ_NONE;
  }

#else
  /* avoid to flood dmesg with this:
   * pex_dbg(KERN_WARNING "NEVER COME HERE: mbspex driver interrupt handler sees interrupt with trixor disabled!!\n");
   */
  return IRQ_HANDLED;
#endif

}

#else

/* simple isr from original mbs driver with semaphore:
 * */
irqreturn_t irq_hand (int irq, void *dev_id)
{

  struct pex_privdata *privdata;
  /*pex_dbg(KERN_INFO "BEGIN irq_hand \n");*/
  u32 irtype, irmask;
  irmask=(TRIX_EV_IRQ_CLEAR | TRIX_DT_CLEAR);
  privdata = (struct pex_privdata *) dev_id;
  irtype = ioread32 (privdata->regs.irq_status);
  mb();
  // JAM test: do not wait when testing status reg
  //ndelay(200); // ORIG
  //pex_dbg(KERN_NOTICE "mbspex driver interrupt handler with interrupt status 0x%x!\n", irtype);

  if ((irtype & irmask) == irmask) /* test bits, is this interupt from us?*/
  {
    disable_irq_nosync (irq);
    //pex_dbg(KERN_NOTICE "mbspex driver handling interrupt type :0x%x\n", irtype);
    ndelay(200); // JAM test: only wait before writing

    // clear source of pending interrupts (in trixor)
    iowrite32 ((TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR), privdata->regs.irq_status);
    mb ();
    ndelay(1000);
    enable_irq (irq);

    //ndelay (200);

    privdata->trix_val = 1;
    up (&(privdata->trix_sem));

#ifdef INTERNAL_TRIG_TEST
    // clear trigger module for test reasons only
    iowrite32 ((EV_IRQ_CLEAR | IRQ_CLEAR), privdata->pl_stat);
    iowrite32 (FC_PULSE, privdata->pl_stat);
    iowrite32 (DT_CLEAR, privdata->pl_stat);
#endif //INTERNAL_TRIG_TEST
    /*    pex_dbg(KERN_INFO "END   irq_hand \n");*/


    //printk (KERN_INFO "END   irq_hand \n");
    return IRQ_HANDLED;
  }
  else
  {
    //pex_dbg(KERN_NOTICE "mbspex driver unknown irtype 0x%x\n", irtype);
    //enable_irq (irq);
    return IRQ_NONE;
  }
}

#endif

void cleanup_device (struct pex_privdata* priv)
{
  int j = 0;
  unsigned long arg=0;
  struct pci_dev* pcidev;
  if (!priv)
    return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (priv->class_dev)
  {
    device_remove_file (priv->class_dev, &dev_attr_trixorbase);
    device_remove_file (priv->class_dev, &dev_attr_bar0base);
    device_remove_file (priv->class_dev, &dev_attr_trixorregs);
    device_remove_file (priv->class_dev, &dev_attr_codeversion);
    device_remove_file (priv->class_dev, &dev_attr_sfpregs);
    device_remove_file (priv->class_dev, &dev_attr_dmaregs);
    device_remove_file (priv->class_dev, &dev_attr_mbspipe);
    device_remove_file (priv->class_dev, &dev_attr_gosipretries);
    device_remove_file (priv->class_dev, &dev_attr_gosipbuswait);
    device_destroy (pex_class, priv->devno);
    priv->class_dev = 0;
  }

#endif

  if(pex_ioctl_unmap_pipe(priv,arg) ==0)
  {
      pex_msg(KERN_NOTICE "Cleaned up previous pipe \n")
      ; // clean up previous pipe if not done before
  }


  /* character device cleanup*/
  if (priv->cdev.owner)
    cdev_del (&priv->cdev);
  if (priv->devid)
    atomic_dec (&pex_numdevs);
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
      pex_dbg(KERN_NOTICE " releasing IO region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
      release_region (priv->bases[j], priv->reglen[j]);
    }
    else
    {
      if (priv->iomem[j] != 0)
      {
        pex_dbg(
            KERN_NOTICE " unmapping virtual MEM region at:%lx -len:%lx \n", (unsigned long) priv->iomem[j], priv->reglen[j]);
        iounmap (priv->iomem[j]);
      }pex_dbg(KERN_NOTICE " releasing MEM region at:%lx -len:%lx \n", priv->bases[j], priv->reglen[j]);
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
  struct pex_privdata *privdata;
  pex_msg(KERN_NOTICE "PEX pci driver starts probe...\n");
  if ((err = pci_enable_device (dev)) != 0)
  {
    pex_msg(
        KERN_ERR "PEX pci driver probe: Error %d enabling PCI device! \n", err);
    return -ENODEV;
  }pex_dbg(KERN_NOTICE "PEX Device is enabled.\n");

  /* Set Memory-Write-Invalidate support */
  if (!pci_set_mwi (dev))
  {
    pex_dbg(KERN_NOTICE "MWI enabled.\n");
  }
  else
  {
    pex_dbg(KERN_NOTICE "MWI not supported.\n");
  }
  pci_set_master (dev); /* NNOTE: DMA worked without, but maybe depends on bios...*/
  test_pci (dev);

  /* Allocate and initialize the private data for this device */
  privdata = kmalloc (sizeof(struct pex_privdata), GFP_KERNEL);
  if (privdata == NULL )
  {
    cleanup_device (privdata);
    return -ENOMEM;
  }
  memset (privdata, 0, sizeof(struct pex_privdata));
  pci_set_drvdata (dev, privdata);
  privdata->pdev = dev;

  // here check which board we have: pex, pexaria, kinpex
  pci_read_config_word (dev, PCI_VENDOR_ID, &vid);
  pex_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
  pci_read_config_word (dev, PCI_DEVICE_ID, &did);
  pex_dbg(KERN_NOTICE "  device id:........0x%x \n", did);
  if (vid == PEXOR_VENDOR_ID && did == PEXOR_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_PEXOR;
    strncpy (devnameformat, PEXORNAMEFMT, 32);
    pex_msg(KERN_NOTICE "  Found board type PEXOR, vendor id: 0x%x, device id:0x%x\n",vid,did);
  }
  else if (vid == PEXARIA_VENDOR_ID && did == PEXARIA_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_PEXARIA;
    strncpy (devnameformat, PEXARIANAMEFMT, 32);
    pex_msg(KERN_NOTICE "  Found board type PEXARIA, vendor id: 0x%x, device id:0x%x\n",vid,did);

  }
  else if (vid == KINPEX_VENDOR_ID && did == KINPEX_DEVICE_ID)
  {
    privdata->board_type = BOARDTYPE_KINPEX;
    strncpy (devnameformat, KINPEXNAMEFMT, 32);
    pex_msg(KERN_NOTICE "  Found board type KINPEX, vendor id: 0x%x, device id:0x%x\n",vid,did);
  }
  else
  {
    privdata->board_type = BOARDTYPE_PEXOR;
    strncpy (devnameformat, PEXORNAMEFMT, 32);
    pex_msg(KERN_NOTICE "  Unknown board type, vendor id: 0x%x, device id:0x%x. Assuming pex mode...\n",vid,did);
  }

  for (ix = 0; ix < 6; ++ix)
  {
    privdata->bases[ix] = pci_resource_start (dev, ix);
    privdata->reglen[ix] = pci_resource_len (dev, ix);
    if (privdata->bases[ix] == 0)
      continue;
    /* JAM here workaround for wrong reglen from kinpex baro (old fpga code only!)*/
    if (privdata->board_type == BOARDTYPE_KINPEX)
    {
      if (privdata->reglen[ix] > PEX_KINPEX_BARSIZE)
      {
        pex_dbg(
            KERN_NOTICE " KINPEX- Reducing exported barsize 0x%lx to 0x%x\n", privdata->reglen[ix], PEX_KINPEX_BARSIZE);
        privdata->reglen[ix] = PEX_KINPEX_BARSIZE;
      }
    }
    if (pci_resource_flags (dev, ix) & IORESOURCE_IO)
    {
      pex_dbg(KERN_NOTICE " - Requesting io ports for bar %d\n", ix);
      if (request_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        pex_dbg(KERN_ERR "I/O address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }pex_dbg("requested ioport at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
    }
    else if (pci_resource_flags (dev, ix) & IORESOURCE_MEM)
    {
      pex_dbg(KERN_NOTICE " - Requesting memory region for bar %d\n", ix);
      if (request_mem_region (privdata->bases[ix], privdata->reglen[ix], kobject_name (&dev->dev.kobj)) == NULL )
      {
        pex_dbg(KERN_ERR "Memory address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }pex_dbg("requested memory at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
      privdata->iomem[ix] = ioremap_nocache (privdata->bases[ix], privdata->reglen[ix]);
      if (privdata->iomem[ix] == NULL )
      {
        pex_dbg(KERN_ERR "Could not remap memory  at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
        cleanup_device (privdata);
        return -EIO;
      }pex_dbg("remapped memory to %lx with length %lx\n", (unsigned long) privdata->iomem[ix], privdata->reglen[ix]);
    }
  }    //for


  // initialize maximum polls value:
  privdata->sfp_maxpolls=PEX_SFP_MAXPOLLS;

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
  privdata->l_bar0_trix_base = privdata->bases[0] + PEX_TRIXOR_BASE;

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

#ifdef PEX_IRQ_WAITQUEUE
  init_waitqueue_head(&(privdata->irq_trig_queue));
  atomic_set(&(privdata->trig_outstanding), 0);
#else
  printk(KERN_INFO " Initialize mutex in locked state \n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37)
  init_MUTEX_LOCKED (&(privdata->trix_sem));
#else
  sema_init (&(privdata->trix_sem), 0);
#endif

  privdata->trix_val = 0;
#endif

  /* debug: do we have valid ir pins/lines here?*/
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_PIN, &(privdata->irqpin))) != 0)
  {
    pex_msg(
        KERN_ERR "PEX pci driver probe: Error %d getting the PCI interrupt pin \n", err);
  }
  if ((err = pci_read_config_byte (dev, PCI_INTERRUPT_LINE, &(privdata->irqline))) != 0)
  {
    pex_msg(
        KERN_ERR "PEX pci driver probe: Error %d getting the PCI interrupt line.\n", err);
  }
  snprintf (privdata->irqname, 64, devnameformat, atomic_read(&pex_numdevs));
  if (request_irq (dev->irq, irq_hand, IRQF_SHARED, privdata->irqname, privdata))
  {
    pex_msg( KERN_ERR "PEX pci_drv: IRQ %d not free.\n", dev->irq);
    cleanup_device (privdata);
    return -EIO;
  }
  pex_msg(
      KERN_NOTICE " assigned IRQ %d for name %s, pin:%d, line:%d \n", dev->irq, privdata->irqname, privdata->irqpin, privdata->irqline);

  ////////////////// here chardev registering
  privdata->devid = atomic_inc_return(&pex_numdevs) - 1;
  if (privdata->devid >= PEX_MAXDEVS)
  {
    pex_msg(
        KERN_ERR "Maximum number of devices reached! Increase MAXDEVICES.\n");
    cleanup_device (privdata);
    return -ENOMSG;
  }

  privdata->devno = MKDEV(MAJOR(pex_devt), MINOR(pex_devt) + privdata->devid);

  /* Register character device */
  cdev_init (&(privdata->cdev), &pex_fops);
  privdata->cdev.owner = THIS_MODULE;
  privdata->cdev.ops = &pex_fops;
  err = cdev_add (&privdata->cdev, privdata->devno, 1);
  if (err)
  {
    pex_msg( "Couldn't add character device.\n");
    cleanup_device (privdata);
    return err;
  }

  /* export special things to class in sysfs: */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (!IS_ERR (pex_class))
  {
    /* driver init had successfully created class, now we create device:*/
    snprintf (devname, 64, devnameformat, MINOR(pex_devt) + privdata->devid);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
    privdata->class_dev = device_create (pex_class, NULL, privdata->devno, privdata, devname);
#else
    privdata->class_dev = device_create(pex_class, NULL,
        privdata->devno, devname);
#endif
    dev_set_drvdata (privdata->class_dev, privdata);
    pex_msg (KERN_NOTICE "Added PEX device: %s", devname);

    if (device_create_file (privdata->class_dev, &dev_attr_codeversion) != 0)
    {
      pex_msg (KERN_ERR "Could not add device file node for code version.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_trixorregs) != 0)
    {
      pex_msg (KERN_ERR "Could not add device file node for trixor registers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_bar0base) != 0)
    {
      pex_msg (KERN_ERR "Could not add device file node for bar0 base address.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_trixorbase) != 0)
    {
      pex_msg (KERN_ERR "Could not add device file node for trixor base address.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_dmaregs) != 0)
    {
      pex_msg(KERN_ERR "Could not add device file node for dma registers.\n");
    }

    if (device_create_file (privdata->class_dev, &dev_attr_sfpregs) != 0)
    {
      pex_msg(KERN_ERR "Could not add device file node for sfp registers.\n");
    }
    if (device_create_file (privdata->class_dev, &dev_attr_mbspipe) != 0)
        {
          pex_msg(KERN_ERR "Could not add device file node for mbs pipe dump.\n");
        }
    if (device_create_file (privdata->class_dev, &dev_attr_gosipretries) != 0)
        {
             pex_msg(KERN_ERR "Could not add device file node for gosip retries.\n");
        }
    if (device_create_file (privdata->class_dev, &dev_attr_gosipbuswait) != 0)
      {
        pex_msg(KERN_ERR "Could not add device file node for gosip bus wait.\n");
      }


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
    pex_msg(KERN_ERR "Could not add PEX device node to /dev !");
  }

#endif

  pex_msg(KERN_NOTICE "probe has finished.\n");
  return 0;
}

static void remove (struct pci_dev *dev)
{
  struct pex_privdata* priv = (struct pex_privdata*) pci_get_drvdata (dev);
  cleanup_device (priv);
  pex_msg(KERN_NOTICE "PEX pci driver end remove.\n");
}

//-----------------------------------------------------------------------------
static struct pci_driver pci_driver = { .name = PEXNAME, .id_table = ids, .probe = probe, .remove = remove, };
//-----------------------------------------------------------------------------

static int __init pex_init (void)
{

  int result;
  pex_msg(KERN_NOTICE "pex driver init...\n");
  pex_devt = MKDEV(my_major_nr, 0);

  /*
   * Register your major, and accept a dynamic number.
   */
  if (my_major_nr)
  {
    result = register_chrdev_region (pex_devt, PEX_MAXDEVS, PEXNAME);
  }
  else
  {
    result = alloc_chrdev_region (&pex_devt, 0, PEX_MAXDEVS, PEXNAME);
    my_major_nr = MAJOR(pex_devt);
  }
  if (result < 0)
  {
    pex_msg(
        KERN_ALERT "Could not alloc chrdev region for major: %d !\n", my_major_nr);
    return result;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  pex_class = class_create (THIS_MODULE, PEXNAME);
  if (IS_ERR (pex_class))
  {
    pex_msg(KERN_ALERT "Could not create class for sysfs support!\n");
  }

#endif
  if (pci_register_driver (&pci_driver) < 0)
  {
    pex_msg(KERN_ALERT "pci driver could not register!\n");
    unregister_chrdev_region (pex_devt, PEX_MAXDEVS);
    return -EIO;
  }
  pex_msg(
      KERN_NOTICE "\t\tdriver init with registration for major no %d done.\n", my_major_nr);
  return 0;

  /* note: actual assignment will be done on probe time*/

}

static void __exit pex_exit (void)
{
  pex_msg(KERN_NOTICE "pex driver exit...\n");
  unregister_chrdev_region (pex_devt, PEX_MAXDEVS);
  pci_unregister_driver (&pci_driver);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (pex_class != NULL )
    class_destroy (pex_class);
#endif

  pex_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}

//-----------------------------------------------------------------------------
module_init(pex_init);
module_exit(pex_exit);
//-----------------------------------------------------------------------------
