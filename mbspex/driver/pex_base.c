// N.Kurz, EE, GSI, 8-Apr-2010
// J.Adamczewski-Musch, EE, GSI, added mmap and some small fixes 24-Jan-2013
// JAM added generic probe/cleanup, privdata structure, sysfs, etc.  28-Jan-2013
// JAM merge pexormbs driver with large pexor to mbspex driver 8-Apr-2014
//-----------------------------------------------------------------------------

#include "pex_base.h"




#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* pex_class;
static DEVICE_ATTR(codeversion, S_IRUGO, pex_sysfs_codeversion_show, NULL);
static DEVICE_ATTR(trixorregs, S_IRUGO, pex_sysfs_trixorregs_show, NULL);
static DEVICE_ATTR(bar0base, S_IRUGO, pex_sysfs_bar0base_show, NULL);
static DEVICE_ATTR(trixorbase, S_IRUGO, pex_sysfs_trixorbase_show, NULL);
static DEVICE_ATTR(dmaregs, S_IRUGO, pex_sysfs_dmaregs_show, NULL);
static DEVICE_ATTR(sfpregs, S_IRUGO, pex_sysfs_sfpregs_show, NULL);
#endif

/* hold full device number */
static dev_t pex_devt;
static atomic_t pex_numdevs=ATOMIC_INIT(0);
static int my_major_nr = 0;

MODULE_AUTHOR("Nikolaus Kurz, Joern Adamczewski-Musch, EE, GSI, 8-Apr-2014");
MODULE_LICENSE("Dual BSD/GPL");

//void pex_show_version(struct pex_privdata *privdata, char* buf)
//{
//    /* stolen from pex_gosip.h*/
//    u32 tmp, year, month, day, version[2];
//    char txt[512];
//    u32* ptversion = (u32*) (privdata->iomem[0] + PEX_SFP_BASE
//            + PEX_SFP_VERSION);
//    tmp = ioread32(ptversion);
//    mb();
//    ndelay(20);
//    year = ((tmp & 0xff000000) >> 24) + 0x2000;
//    month = (tmp & 0xff0000) >> 16;
//    day = (tmp & 0xff00) >> 8;
//    version[0] = (tmp & 0xf0) >> 4;
//    version[1] = (tmp & 0xf);
//    snprintf(txt, 512,
//            "PEX FPGA code compiled at Year=%x Month=%x Date=%x Version=%x.%x \n",
//            year, month, day, version[0], version[1]);
//    pex_dbg(KERN_NOTICE "%s", txt);
//    if (buf) snprintf(buf, 512, "%s", txt);
//}
//
//ssize_t pex_sysfs_codeversion_show(struct device *dev,
//        struct device_attribute *attr, char *buf)
//{
//    char vstring[512];
//    ssize_t curs = 0;
//    struct pex_privdata *privdata;
//    privdata = (struct pex_privdata*) dev_get_drvdata(dev);
//    curs =
//            snprintf(vstring, 512,
//                    "*** This is PEX driver for MBS, Version %s build on %s at %s \n\t",
//                    PEXVERSION, __DATE__, __TIME__);
//    pex_show_version(privdata, vstring + curs);
//    return snprintf(buf, PAGE_SIZE, "%s\n", vstring);
//}

ssize_t pex_sysfs_trixorregs_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ssize_t curs = 0;
#ifdef PEXOR_WITH_TRIXOR
    struct pex_privdata *privdata;
    privdata = (struct pex_privdata*) dev_get_drvdata(dev);
    curs += snprintf(buf + curs, PAGE_SIZE - curs,
            "*** PEX trixor register dump:\n");

    curs += snprintf(buf + curs, PAGE_SIZE - curs, "\t trixor stat: 0x%x\n",
            readl(privdata->regs.irq_status));
    curs += snprintf(buf + curs, PAGE_SIZE - curs, "\t trixor ctrl: 0x%x\n",
            readl(privdata->regs.irq_control));
    curs += snprintf(buf + curs, PAGE_SIZE - curs, "\t trixor fcti: 0x%x\n",
            readl(privdata->regs.trix_fcti));
    curs += snprintf(buf + curs, PAGE_SIZE - curs, "\t trixor cvti: 0x%x\n",
            readl(privdata->regs.trix_cvti));
#endif

    return curs;
}

ssize_t pex_sysfs_bar0base_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ssize_t curs = 0;
    struct pex_privdata *privdata;
    privdata = (struct pex_privdata*) dev_get_drvdata(dev);
    curs += snprintf(buf + curs, PAGE_SIZE - curs, "%lx\n",
            privdata->bases[0]);
    return curs;
}

ssize_t pex_sysfs_trixorbase_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    ssize_t curs = 0;
#ifdef PEXOR_WITH_TRIXOR
    struct pex_privdata *privdata;
    privdata = (struct pex_privdata*) dev_get_drvdata(dev);
    curs += snprintf(buf + curs, PAGE_SIZE - curs, "%x\n",
            privdata->l_bar0_trix_base);
#endif
    return curs;
}



ssize_t pex_sysfs_codeversion_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  char vstring[512];
  ssize_t curs=0;
  struct  regs_pex* pg;
  struct pex_privdata *privdata;
  privdata= (struct pex_privdata*) dev_get_drvdata(dev);
  curs=snprintf(vstring, 512, "*** This is MBSPEX driver version %s build on %s at %s \n \t", PEXVERSION, __DATE__, __TIME__);
  pg=&(privdata->regs);
  pex_show_version(&(pg->sfp),vstring+curs);
  return snprintf(buf, PAGE_SIZE, "%s\n", vstring);
}






ssize_t pex_sysfs_dmaregs_show(struct device *dev, struct device_attribute *attr, char *buf)
{
  ssize_t curs=0;
  struct  regs_pex* pg;
  struct pex_privdata *privdata;
  privdata= (struct pex_privdata*) dev_get_drvdata(dev);
  pg=&(privdata->regs);
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "*** PEX dma/irq register dump:\n");
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma control/status: 0x%x\n", readl(pg->dma_control_stat));
#ifdef PEXOR_WITH_TRIXOR
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t irq/trixor stat: 0x%x\n", readl(pg->irq_status));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t irq/trixor ctrl: 0x%x\n", readl(pg->irq_control));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t trixor fcti: 0x%x\n", readl(pg->trix_fcti));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t trixor cvti: 0x%x\n", readl(pg->trix_cvti));
#endif
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma source      address: 0x%x\n", readl(pg->dma_source));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma destination address: 0x%x\n", readl(pg->dma_dest));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma length:              0x%x\n", readl(pg->dma_len));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t dma burst size:          0x%x\n", readl(pg->dma_burstsize));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t RAM start:               0x%x\n", readl(pg->ram_start));
  curs+=snprintf(buf+curs, PAGE_SIZE-curs, "\t RAM end:                 0x%x\n", readl(pg->ram_end));
  return curs;
}















void test_pci(struct pci_dev *dev)
{
    int bar = 0;
    u32 originalvalue = 0;
    u32 base = 0;
    u16 comstat = 0;
    u8 typ = 0;
    u8 revision = 0;
    u16 vid = 0;
    u16 did = 0;

    pci_read_config_byte(dev, PCI_REVISION_ID, &revision);
    pex_dbg(
            KERN_NOTICE "\n PEX: test_pci found PCI revision number 0x%x \n", revision);
    pci_read_config_word(dev, PCI_VENDOR_ID, &vid);
    pex_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
    pci_read_config_word(dev, PCI_DEVICE_ID, &did);
    pex_dbg(KERN_NOTICE "  device id:........0x%x \n", did);

    /*********** test the address regions*/
    for (bar = 0; bar < 6; ++bar)
        {
            pex_dbg(
                    KERN_NOTICE "Resource %d start=%x\n", bar, (unsigned) pci_resource_start( dev,bar ));
            pex_dbg(
                    KERN_NOTICE "Resource %d end=%x\n", bar, (unsigned) pci_resource_end( dev,bar ));
            pex_dbg(
                    KERN_NOTICE "Resource %d len=%x\n", bar, (unsigned) pci_resource_len( dev,bar ));
            pex_dbg(
                    KERN_NOTICE "Resource %d flags=%x\n", bar, (unsigned) pci_resource_flags( dev,bar ));
            if ((pci_resource_flags(dev,bar)& IORESOURCE_IO) )
                {
                    // Ressource im IO-Adressraum
                    pex_dbg(KERN_NOTICE " - resource is IO\n");
                }
            if ((pci_resource_flags(dev,bar)& IORESOURCE_MEM) )
                {
                    pex_dbg(KERN_NOTICE " - resource is MEM\n");
                }
            if ((pci_resource_flags(dev,bar)& PCI_BASE_ADDRESS_SPACE_IO) )
                {
                    pex_dbg(KERN_NOTICE " - resource is PCI IO\n");
                }
            if ((pci_resource_flags(dev,bar)& PCI_BASE_ADDRESS_SPACE_MEMORY) )
                {
                    pex_dbg(KERN_NOTICE " - resource is PCI MEM\n");
                }
            if ((pci_resource_flags(dev,bar)& PCI_BASE_ADDRESS_MEM_PREFETCH) )
                {
                    pex_dbg(KERN_NOTICE " - resource prefetch bit is set \n");
                }
            if ((pci_resource_flags(dev,bar)& PCI_BASE_ADDRESS_MEM_TYPE_64) )
                {
                    pex_dbg(KERN_NOTICE " - resource is 64bit address \n");
                }
            if ((pci_resource_flags(dev,bar)& PCI_BASE_ADDRESS_MEM_TYPE_32) )
                {
                    pex_dbg(KERN_NOTICE " - resource is 32bit address \n");
                }
            if ((pci_resource_flags(dev,bar)& IORESOURCE_PREFETCH) )
                {
                    pex_dbg(KERN_NOTICE " - resource is prefetchable \n");
                }
            if ((pci_resource_flags(dev,bar)& PCI_BASE_ADDRESS_MEM_PREFETCH) )
                {
                    pex_dbg(KERN_NOTICE " - resource is PCI mem prefetchable \n");
                }
            if ((pci_resource_flags(dev,bar)& PCI_BASE_ADDRESS_MEM_TYPE_1M) )
                {
                    pex_dbg(KERN_NOTICE " - resource is PCI memtype below 1M \n");
                }

        }
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &originalvalue);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, 0xffffffff);
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_0, &base);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_0, originalvalue);
    pex_dbg("size of base address 0: %i\n", ~base+1);
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_1, &originalvalue);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_1, 0xffffffff);
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_1, &base);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_1, originalvalue);
    pex_dbg("size of base address 1: %i\n", ~base+1);
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_2, &originalvalue);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_2, 0xffffffff);
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_2, &base);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_2, originalvalue);
    pex_dbg("size of base address 2: %i\n", ~base+1);
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_3, &originalvalue);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_3, 0xffffffff);
    pci_read_config_dword(dev, PCI_BASE_ADDRESS_3, &base);
    pci_write_config_dword(dev, PCI_BASE_ADDRESS_3, originalvalue);
    pex_dbg("size of base address 3: %i\n", ~base+1);

    /***** here tests of configuration/status register:******/
    pci_read_config_word(dev, PCI_COMMAND, &comstat);
    pex_dbg("\n****  Command register is: %d\n", comstat);
    pci_read_config_word(dev, PCI_STATUS, &comstat);
    pex_dbg("\n****  Status register is: %d\n", comstat);
    pci_read_config_byte(dev, PCI_HEADER_TYPE, &typ);
    pex_dbg("\n****  Header type is: %d\n", typ);

}


void clear_pointers(struct  regs_pex* pg)
{
  if(pg==0) return;
  pg->init_done=0x0;
  pex_dbg(KERN_NOTICE "** Cleared pex pointer structure %lx.\n",(long unsigned int) pg);
}


void set_pointers(struct  regs_pex* pg, void* membase, unsigned long bar)
{

  void* dmabase=0;
  if(pg==0) return;
  dmabase=membase+PEX_DMA_BASE;
#ifdef PEX_WITH_TRIXOR
  pg->irq_control=(u32*)(membase+ PEX_TRIXOR_BASE + PEX_TRIX_CTRL);
  pg->irq_status=(u32*)(membase+ PEX_TRIXOR_BASE + PEX_TRIX_STAT);
  pg->trix_fcti=(u32*)(membase+ PEX_TRIXOR_BASE + PEX_TRIX_FCTI);
  pg->trix_cvti=(u32*)(membase+ PEX_TRIXOR_BASE + PEX_TRIX_CVTI);
#else
  pg->irq_control=(u32*)(membase+PEX_IRQ_CTRL);
  pg->irq_status=(u32*)(membase+PEX_IRQ_STAT);
#endif

  pg->dma_control_stat=(u32*)(dmabase+PEX_DMA_CTRLSTAT);
  pg->dma_source=(u32*)(dmabase+PEX_DMA_SRC);
  pg->dma_dest=(u32*)(dmabase+PEX_DMA_DEST);
  pg->dma_len=(u32*)(dmabase+PEX_DMA_LEN);
  pg->dma_burstsize=(u32*)(dmabase+PEX_DMA_BURSTSIZE);



  pg->ram_start=(u32*)(membase+PEX_DRAM);
  pg->ram_end=(u32*)(membase+PEX_DRAM+PEX_RAMSIZE);
  pg->ram_dma_base =   (dma_addr_t) (bar+PEX_DRAM);
  pg->ram_dma_cursor = (dma_addr_t)(bar+PEX_DRAM);
  set_sfp(&(pg->sfp), membase, bar);

  pg->init_done=0x1;
  pex_dbg(KERN_NOTICE "** Set pex structure %lx.\n",(long unsigned int) pg);

}

void print_register(const char* description, u32* address)
{
  pex_dbg(KERN_NOTICE "%s:\taddr=%lx cont=%x\n", description, (long unsigned int) address, readl(address));
}



void print_regs(struct  regs_pex* pg)
{
  if(pg==0) return;
  pex_dbg(KERN_NOTICE "\n##print_pex: ###################\n");
  pex_dbg(KERN_NOTICE "init: \t=%x\n", pg->init_done);
  if(!pg->init_done) return;
  print_register("dma control/status", pg->dma_control_stat);
#ifdef PEXOR_WITH_TRIXOR
  print_register("irq status", pg->irq_status);
  print_register("irq control", pg->irq_control);

  /*pex_dbg(KERN_NOTICE "trixor control add=%x \n",pg->irq_control) ;
    pex_dbg(KERN_NOTICE "trixor status  add =%x \n",pg->irq_status);
    pex_dbg(KERN_NOTICE "trixor fast clear add=%x \n",pg->trix_fcti) ;
    pex_dbg(KERN_NOTICE "trixor conversion time add =%x \n",pg->trix_cvti);*/

  print_register("trixor fast clear time", pg->trix_fcti);
  print_register("trixor conversion time", pg->trix_cvti);
#endif

  print_register("dma source address",pg->dma_source);
  print_register("dma dest   address", pg->dma_dest);
  print_register("dma len   address", pg->dma_len);
  print_register("dma burstsize", pg->dma_burstsize);

  print_register("RAM start", pg->ram_start) ;
  print_register("RAM end",pg->ram_end);
  pex_dbg(KERN_NOTICE "RAM DMA base add=%x \n",(unsigned) pg->ram_dma_base) ;
  pex_dbg(KERN_NOTICE "RAM DMA cursor add=%x \n",(unsigned) pg->ram_dma_cursor);

  print_sfp(&(pg->sfp));

}

int pex_start_dma(struct pex_privdata *priv, dma_addr_t source, dma_addr_t dest, u32 dmasize, u32 channelmask)
{
    int rev;
    u32 burstsize=PEX_BURST;
    u32 enable=PEX_DMA_ENABLED_BIT; /* this will start dma immediately from given source address*/
    if(channelmask>1) enable=channelmask; /* set sfp token transfer to initiate the DMA later*/


      rev=pex_poll_dma_complete(priv);
      if(rev)
          {
            pex_msg(KERN_NOTICE "**pex_start_dma: dma was not finished, do not start new one!\n");
            return rev;
          }
      /* calculate maximum burstsize here:*/
      while (dmasize % burstsize)
         {
              burstsize = (burstsize >> 1);
         }
      if(burstsize<PEX_BURST_MIN)
          {
              pex_dbg(KERN_NOTICE "**pex_start_dma: correcting for too small burstsize %x\n",burstsize);
              burstsize=PEX_BURST_MIN;
              while (dmasize % burstsize)
                     {
                          dmasize+=2;
                          /* otherwise this can only happen in the last chunk of sg dma.
                           * here it should be no deal to transfer a little bit more...*/
                     }
              pex_dbg(KERN_NOTICE "**changed source address to 0x%x, dest:0x%x, dmasize to 0x%x, burstsize:0x%x\n", (unsigned) source, (unsigned) dest,dmasize, burstsize)
          }


      pex_dbg(KERN_NOTICE "#### pex_start_dma will initiate dma from 0x%x to 0x%x, len=0x%x, burstsize=0x%x...\n",
            (unsigned) source, (unsigned) dest,  dmasize, burstsize);

      iowrite32(source, priv->regs.dma_source);
      mb();
      iowrite32((u32) dest, priv->regs.dma_dest);
      mb();
      iowrite32(burstsize, priv->regs.dma_burstsize);
      mb();
      iowrite32(dmasize, priv->regs.dma_len);
      mb();
      iowrite32(enable, priv->regs.dma_control_stat);
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



int pex_poll_dma_complete(struct pex_privdata* priv)
{
  int loops=0;
  u32 enable=PEX_DMA_ENABLED_BIT;

  while(1)
    {
     /* pex_dbg(KERN_ERR "pex_poll_dma_complete reading from 0x%p \n",priv->regs.dma_control_stat);*/

      enable=ioread32(priv->regs.dma_control_stat);
      mb();
   //   pex_dbg(KERN_ERR "pex_poll_dma_complete sees dmactrl=: 0x%x , looking for %x\n",enable, PEX_DMA_ENABLED_BIT);
      if((enable & PEX_DMA_ENABLED_BIT) == 0) break;
      /* poll until the dma bit is cleared => dma complete*/

      //pex_dbg(KERN_NOTICE "#### pex_poll_dma_complete wait for dma completion #%d\n",loops);
      if(loops++ > PEX_DMA_MAXPOLLS)
    {
      pex_msg(KERN_ERR "pex_poll_dma_complete: polling longer than %d cycles (delay %d ns) for dma complete!!!\n",PEX_DMA_MAXPOLLS, PEX_DMA_POLLDELAY );
      return -EFAULT;
    }
      if(PEX_DMA_POLLDELAY) ndelay(PEX_DMA_POLLDELAY);
      if(PEX_DMA_POLL_SCHEDULE) schedule();
    };
  return 0;
}









//-----------------------------------------------------------------------------
int pex_open(struct inode *inode, struct file *filp)
{
    struct pex_privdata *dev;      // device information
    pex_dbg(KERN_INFO "\nBEGIN pex_open \n");
    dev = container_of(inode->i_cdev, struct pex_privdata, cdev);
    filp->private_data = dev;   // for other methods
    pex_dbg(KERN_INFO "END   pex_open \n");
    return 0;                   // success
}
//-----------------------------------------------------------------------------
int pex_release(struct inode *inode, struct file *filp)
{
    pex_dbg(KERN_INFO "BEGIN pex_release \n");
    pex_dbg(KERN_INFO "END   pex_release \n");
    return 0;
}

//--------------------------

int pex_mmap(struct file *filp, struct vm_area_struct *vma)
{
    struct pex_privdata *privdata;
//    u64 phstart, phend;
//    unsigned phtype;
    int ret = 0;
    unsigned long bufsize, barsize;
    privdata = (struct pex_privdata*) filp->private_data;
    pex_dbg(KERN_NOTICE "** starting pex_mmap...\n");
    if (!privdata) return -EFAULT;
    bufsize = (vma->vm_end - vma->vm_start);
    pex_dbg(KERN_NOTICE "** starting pex_mmap for size=%ld \n", bufsize);

    if (vma->vm_pgoff == 0)
        {
            /* user does not specify external physical address, we deliver mapping of bar 0:*/
            pex_dbg(KERN_NOTICE "Pexor is Mapping bar0 base address %x / PFN %x\n", privdata->l_bar0_base, privdata->l_bar0_base >> PAGE_SHIFT);
            barsize = privdata->l_bar0_end - privdata->l_bar0_base;
            if (bufsize > barsize)
                {
                    pex_msg(
                            KERN_WARNING "Requested length %ld exceeds bar0 size, shrinking to %ld bytes\n", bufsize, barsize);
                    bufsize = barsize;
                }

            vma->vm_flags |= (VM_RESERVED); /* TODO: do we need this?*/
            ret = remap_pfn_range(vma, vma->vm_start,
                    privdata->l_bar0_base >> PAGE_SHIFT, bufsize,
                    vma->vm_page_prot);
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
            ret = remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff, bufsize,
                    vma->vm_page_prot);

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

int pex_ioctl_test(struct pex_privdata* priv, unsigned long arg)
{
  /* curently we test here the pio of pex ram without copy from user stuff*/
  void* memstart;
  int i,memsize,retval;
  int localbuf=0;
  retval=get_user(memsize, (int*) arg);
  if(retval) return retval;
  memstart=(void*) (priv->regs.ram_start);
  pex_msg(KERN_NOTICE "pex_ioctl_test starting to write %d integers to %p\n", memsize, memstart);
  for(i=0; i<memsize;++i)
    {
      localbuf=i;
      iowrite32(localbuf, memstart+(i<<2));
      mb();
      pex_msg(KERN_NOTICE "%d.. ", i);
      if((i%10)==0) pex_msg(KERN_NOTICE "\n");
    }
  pex_msg(KERN_NOTICE "pex_ioctl_test reading back %d integers from %p\n", memsize, memstart);
  for(i=0; i<memsize;++i)
    {
      localbuf=ioread32(memstart+(i<<2));
      mb();
      if(localbuf!=i)
    pex_msg(KERN_ERR "Error reading back value %d\n", i);
    }
  pex_msg(KERN_NOTICE "pex_ioctl_test finished. \n");
  return 0;
}

int pex_ioctl_reset(struct pex_privdata* priv, unsigned long arg)
{
  pex_dbg(KERN_NOTICE "** pex_ioctl_reset...\n");
  pex_sfp_clear_all(priv);
  // note that clear sfp channels is done via pex_ioctl_init_bus

#ifdef PEX_WITH_TRIXOR
  pex_dbg(KERN_NOTICE "Initalizing TRIXOR... \n");
  atomic_set(&(priv->trig_outstanding), 0);

  iowrite32(TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR, priv->regs.irq_status);   /*reset interrupt source*/
  mb();
  ndelay(20);


  iowrite32(TRIX_BUS_DISABLE, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32(TRIX_HALT , priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32(TRIX_MASTER , priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32(TRIX_CLEAR, priv->regs.irq_control);
  mb();
  ndelay(20);


  iowrite32(0x10000 - 0x20 , priv->regs.trix_fcti);
  mb();
  ndelay(20);
  iowrite32(0x10000 - 0x40 , priv->regs.trix_cvti);
  mb();
  ndelay(20);

  iowrite32(TRIX_DT_CLEAR, priv->regs.irq_status);
  mb();
  ndelay(20);

  iowrite32(TRIX_BUS_ENABLE, priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32(TRIX_HALT , priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32(TRIX_MASTER , priv->regs.irq_control);
  mb();
  ndelay(20);

  iowrite32(TRIX_CLEAR, priv->regs.irq_control);
  mb();
  ndelay(20);



  pex_dbg(KERN_NOTICE " ... TRIXOR done.\n");
#else

  iowrite32(0, priv->regs.irq_control);
  mb();
  iowrite32(0, priv->regs.irq_status);
  mb();
#endif
  print_regs(&(priv->regs));
  return 0;
}












#ifdef PEX_WITH_TRIXOR
int pex_ioctl_set_trixor(struct pex_privdata* priv, unsigned long arg)
{
 int command,retval;
 struct pex_trixor_set descriptor;
 retval=copy_from_user(&descriptor, (void __user *) arg, sizeof(struct pex_trixor_set));
 if(retval) return retval;
 command=descriptor.command;
 switch(command)
   {
   case PEX_TRIX_RES:
     iowrite32(TRIX_CLEAR, priv->regs.irq_control);
     mb();
     ndelay(20);
     break;

   case PEX_TRIX_GO:
     iowrite32((TRIX_EN_IRQ | TRIX_GO), priv->regs.irq_control);
     mb();
     ndelay(20);
     break;

   case PEX_TRIX_HALT:
     iowrite32(TRIX_HALT , priv->regs.irq_control);
     mb();
     ndelay(20);
     break;

   case PEX_TRIX_TIMESET:
     iowrite32(0x10000 - descriptor.fct , priv->regs.trix_fcti);
     mb();
     ndelay(20);
     iowrite32(0x10000 - descriptor.cvt , priv->regs.trix_cvti);
     mb();
     ndelay(20);

     break;

   default:
     pex_dbg(KERN_ERR "pex_ioctl_set_trixor unknown command %x\n", command);
     return -EFAULT;


   };

 return 0;
}


int pex_ioctl_wait_trigger(struct pex_privdata* priv, unsigned long arg)
{
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
  else{}
  atomic_dec(&(priv->trig_outstanding));
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
    pex_dbg((KERN_INFO "BEGIN pex_ioctl \n"));
    privdata = (struct pex_privdata*) filp->private_data;
    switch (cmd)
        {
      case PEX_IOC_RESET:
          pex_dbg(KERN_NOTICE "** pex_ioctl reset\n");
          return pex_ioctl_reset(privdata,arg);
          break;

        case PEX_IOC_TEST:
          pex_dbg(KERN_NOTICE "** pex_ioctl test\n");
          return pex_ioctl_test(privdata, arg);
          break;

        case PEX_IOC_WRITE_BUS:
           pex_dbg(KERN_NOTICE "** pex_ioctl write bus\n");
           return pex_ioctl_write_bus(privdata, arg);
           break;

         case PEX_IOC_READ_BUS:
           pex_dbg(KERN_NOTICE "** pex_ioctl read bus\n");
           return pex_ioctl_read_bus(privdata, arg);
           break;

         case PEX_IOC_INIT_BUS:
           pex_dbg(KERN_NOTICE "** pex_ioctl init bus\n");
           return pex_ioctl_init_bus(privdata, arg);
           break;

        case PEX_IOC_REQUEST_TOKEN:
           pex_dbg(KERN_NOTICE "** pex_ioctl request token\n");
           return pex_ioctl_request_token(privdata, arg);
           break;

         case PEX_IOC_WAIT_TOKEN:
           pex_dbg(KERN_NOTICE "** pex_ioctl wait token\n");
           return pex_ioctl_wait_token(privdata, arg);
           break;

#ifdef PEX_WITH_TRIXOR
         case PEX_IOC_WAIT_TRIGGER:
           pex_dbg(KERN_NOTICE "** pex_ioctl wait trigger\n");
           return pex_ioctl_wait_trigger(privdata, arg);
           break;

         case PEX_IOC_SET_TRIXOR:
          pex_dbg(KERN_NOTICE "** pex_ioctl set trixor\n");
          return pex_ioctl_set_trixor(privdata, arg);
          break;

#ifdef  PEX_IRQ_WAITQUEUE

        case PEX_IOC_WAIT_SEM:
            pex_dbg(KERN_INFO "Emulated WAIT_SEM using waitqueu\n");
            return pex_ioctl_wait_trigger(privdata,arg);
            break;
        case PEX_IOC_POLL_SEM:
            trigcount=atomic_read( &(privdata->trig_outstanding) );
            pex_dbg(KERN_INFO "Emulated POLL_SEM, trix_val: %d \n", trigcount);
            retval = __put_user(trigcount, (int __user *)arg);
            pex_dbg((KERN_INFO " after POLL_SEM \n"));
            break;

        case PEX_IOC_RESET_SEM:
            pex_dbg(KERN_INFO " Emulated RESET_SEM doing nothing\n");
            break;


#else

        case PEX_IOC_WAIT_SEM:
                   pex_dbg(KERN_INFO " before WAIT_SEM \n");
                   if (down_interruptible(&(privdata->trix_sem)))
                     return -ERESTARTSYS; /* JAM avoid possible hangup of m_read_meb when killed by resl*/
                   privdata->trix_val = 0;
                   pex_dbg((KERN_INFO " after  WAIT_SEM \n"));
                   break;
        case PEX_IOC_POLL_SEM:
                   pex_dbg(
                           KERN_INFO " before POLL_SEM, trix_val: %ld \n", privdata->trix_val);
                   retval = __put_user(privdata->trix_val, (int __user *)arg);
                   pex_dbg((KERN_INFO " after POLL_SEM \n"));
                   break;

         case PEX_IOC_RESET_SEM:
                          pex_dbg(KERN_INFO " before RESET_SEM \n");
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
#endif /* pexor with trixor*/

        case PEX_IOC_GET_BAR0_BASE:
            pex_dbg(KERN_INFO " before GET_BAR0_BASE \n");
            retval = __put_user(privdata->l_bar0_base, (int __user *)arg);
            pex_dbg(KERN_INFO " after  GET_BAR0_BASE \n");
            break;
        case PEX_IOC_GET_BAR0_TRIX_BASE:
            pex_dbg(KERN_INFO " before GET_TRIX_BASE \n");
            retval = __put_user(privdata->l_bar0_trix_base, (int __user *)arg);
            pex_dbg(KERN_INFO " after  GET_TRIX_BASE \n");
            break;


        default:
                 return -ENOTTY;
        }
    pex_dbg((KERN_INFO "END   pex_ioctl \n"));
    return retval;
}

//PEX_IOC_RESET             _IO(  PEX_IOC_MAGIC, 0)
//#define PEX_IOC_TEST            _IOR(  PEX_IOC_MAGIC, 1, int)
//#define PEX_IOC_WAIT_SEM          _IO(  PEX_IOC_MAGIC, 2)
//#define PEX_IOC_POLL_SEM          _IOR(  PEX_IOC_MAGIC, 3, int)
//#define PEX_IOC_RESET_SEM         _IOR(  PEX_IOC_MAGIC, 4, int)
//#define PEX_IOC_GET_BAR0_BASE      _IOR(  PEX_IOC_MAGIC, 5, int)
//#define PEX_IOC_GET_BAR0_TRIX_BASE      _IOR(  PEX_IOC_MAGIC, 6, int)
//#define PEX_IOC_WRITE_BUS   _IOWR(  PEX_IOC_MAGIC, 7, struct pex_bus_io)
//#define PEX_IOC_READ_BUS    _IOWR(  PEX_IOC_MAGIC, 8, struct pex_bus_io)
//#define PEX_IOC_INIT_BUS    _IOW(  PEX_IOC_MAGIC, 9, struct pex_bus_io)
//#define PEX_IOC_SET_TRIXOR    _IOR(  PEX_IOC_MAGIC, 10, struct pex_trixor_set)
//#define PEX_IOC_REQUEST_TOKEN    _IOWR(  PEX_IOC_MAGIC, 11, struct pex_token_io)
//#define PEX_IOC_WAIT_TOKEN    _IOWR(  PEX_IOC_MAGIC, 12, struct pex_token_io)
//#define PEX_IOC_WAIT_TRIGGER










//-----------------------------------------------------------------------------
struct file_operations pex_fops = { .owner = THIS_MODULE,
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 35)
        .ioctl = pex_ioctl,
#else
        .unlocked_ioctl = pex_ioctl,
#endif
        .mmap = pex_mmap, .open = pex_open, .release = pex_release, };
//-----------------------------------------------------------------------------
static struct pci_device_id ids[] = {
      { PCI_DEVICE(PEXOR_VENDOR_ID, PEXOR_DEVICE_ID), },  // PEX
      { PCI_DEVICE(PEXARIA_VENDOR_ID, PEXARIA_DEVICE_ID), }, //pexaria
      { PCI_DEVICE(KINPEX_VENDOR_ID, KINPEX_DEVICE_ID), }, // kinpex
        { 0, } };
//-----------------------------------------------------------------------------
MODULE_DEVICE_TABLE(pci, ids);
//-----------------------------------------------------------------------------







#ifdef PEX_IRQ_WAITQUEUE

/* here alternative isr from full driver with waitqueue:
 * */
irqreturn_t irq_hand( int irq, void *dev_id)
{
  struct pex_privdata *privdata;
  privdata=(struct pex_privdata *) dev_id;
#ifdef PEX_WITH_TRIXOR
  u32 irtype;
  /* check if this interrupt was raised by our device*/
  irtype=ioread32(privdata->regs.irq_status);
  mb();
  ndelay(20);
  if(irtype & (TRIX_EV_IRQ_CLEAR | TRIX_DT_CLEAR)) /* test bits */
    {
      /* prepare for trixor interrupts here:*/
      irtype = TRIX_EV_IRQ_CLEAR | TRIX_IRQ_CLEAR;
      iowrite32(irtype, privdata->regs.irq_status);   /*reset interrupt source*/
      mb();
      ndelay(20);
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
irqreturn_t irq_hand(int irq, void *dev_id)
{

    struct pex_privdata *privdata;
    pex_dbg(KERN_INFO "BEGIN irq_hand \n");

    privdata = (struct pex_privdata *) dev_id;
    disable_irq_nosync(irq);

    ndelay(1000);

    // clear source of pending interrupts (in trixor)
    iowrite32((EV_IRQ_CLEAR | IRQ_CLEAR), privdata->pl_stat);
    //wmb ();
    mb ();

    ndelay(1000);

    enable_irq(irq);

    //ndelay (200);

    privdata->trix_val = 1;
    up(&(privdata->trix_sem));

#ifdef INTERNAL_TRIG_TEST
    // clear trigger module for test reasons only
    iowrite32 ((EV_IRQ_CLEAR | IRQ_CLEAR), privdata->pl_stat);
    iowrite32 (FC_PULSE, privdata->pl_stat);
    iowrite32 (DT_CLEAR, privdata->pl_stat);
#endif //INTERNAL_TRIG_TEST
    pex_dbg(KERN_INFO "END   irq_hand \n");
    //printk (KERN_INFO "END   irq_hand \n");
    return IRQ_HANDLED;
}


#endif











void cleanup_device(struct pex_privdata* priv)
{
    int j = 0;
    struct pci_dev* pcidev;
    if (!priv) return;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
    /* sysfs device cleanup */
    if (priv->class_dev)
        {
            device_remove_file(priv->class_dev, &dev_attr_trixorbase);
            device_remove_file(priv->class_dev, &dev_attr_bar0base);
            device_remove_file(priv->class_dev, &dev_attr_trixorregs);
            device_remove_file(priv->class_dev, &dev_attr_codeversion);
            device_remove_file(priv->class_dev, &dev_attr_sfpregs);
            device_remove_file(priv->class_dev, &dev_attr_dmaregs);
            device_destroy(pex_class, priv->devno);
            priv->class_dev = 0;
        }

#endif
    /* character device cleanup*/
    if (priv->cdev.owner) cdev_del(&priv->cdev);
    if (priv->devid) atomic_dec(&pex_numdevs);
    pcidev = priv->pdev;
    if (!pcidev) return;
    if(priv->devno) /* misuse devno as flag if we already had successfully requested irq JAM*/
      free_irq(pcidev->irq, priv);
    for (j = 0; j < 6; ++j)
        {
            if (priv->bases[j] == 0) continue;
            if ((pci_resource_flags(pcidev, j)& IORESOURCE_IO))
                {
                    pex_dbg(KERN_NOTICE " releasing IO region at:%lx -len:%lx \n",priv->bases[j],priv->reglen[j]);
                    release_region(priv->bases[j], priv->reglen[j]);
                }
            else
                {
                    if (priv->iomem[j] != 0)
                        {
                            pex_dbg(KERN_NOTICE " unmapping virtual MEM region at:%lx -len:%lx \n",(unsigned long) priv->iomem[j],priv->reglen[j]);
                            iounmap(priv->iomem[j]);
                        }
                    pex_dbg(KERN_NOTICE " releasing MEM region at:%lx -len:%lx \n",priv->bases[j],priv->reglen[j]);
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
    u16 vid = 0; u16 did = 0;
    char devnameformat[64];
    char devname[64];
    struct pex_privdata *privdata;
    pex_msg(KERN_NOTICE "PEX pci driver starts probe...\n");
    if ((err = pci_enable_device(dev)) != 0)
        {
            pex_msg(
                    KERN_ERR "PEX pci driver probe: Error %d enabling PCI device! \n", err);
            return -ENODEV;
        }
    pex_dbg(KERN_NOTICE "PEX Device is enabled.\n");

    /* Set Memory-Write-Invalidate support */
    if (!pci_set_mwi(dev))
        {
            pex_dbg(KERN_NOTICE "MWI enabled.\n");
        }
    else
        {
            pex_dbg(KERN_NOTICE "MWI not supported.\n");
        }
    pci_set_master(dev); /* NNOTE: DMA worked without, but maybe depends on bios...*/
    test_pci(dev);

    /* Allocate and initialize the private data for this device */
    privdata = kmalloc(sizeof(struct pex_privdata), GFP_KERNEL);
    if (privdata == NULL )
        {
            cleanup_device(privdata);
            return -ENOMEM;
        }
    memset(privdata, 0, sizeof(struct pex_privdata));
    pci_set_drvdata(dev, privdata);
    privdata->pdev = dev;

    // here check which board we have: pex, pexaria, kinpex
    pci_read_config_word(dev, PCI_VENDOR_ID, &vid);
    pex_dbg(KERN_NOTICE "  vendor id:........0x%x \n", vid);
    pci_read_config_word(dev, PCI_DEVICE_ID, &did);
    pex_dbg(KERN_NOTICE "  device id:........0x%x \n", did);
    if(vid==PEXOR_VENDOR_ID && did==PEXOR_DEVICE_ID){
      privdata->board_type=BOARDTYPE_PEXOR;
      strncpy(devnameformat,PEXORNAMEFMT,32);
      pex_msg(KERN_NOTICE "  Found board type PEXOR, vendor id: 0x%x, device id:0x%x\n",vid,did);
    }
    else if(vid==PEXARIA_VENDOR_ID && did==PEXARIA_DEVICE_ID){
      privdata->board_type=BOARDTYPE_PEXARIA;
      strncpy(devnameformat,PEXARIANAMEFMT,32);
      pex_msg(KERN_NOTICE "  Found board type PEXARIA, vendor id: 0x%x, device id:0x%x\n",vid,did);

    }
    else if(vid==KINPEX_VENDOR_ID && did==KINPEX_DEVICE_ID){
         privdata->board_type=BOARDTYPE_KINPEX;
         strncpy(devnameformat,KINPEXNAMEFMT,32);
         pex_msg(KERN_NOTICE "  Found board type KINPEX, vendor id: 0x%x, device id:0x%x\n",vid,did);
       }
    else
    {
        privdata->board_type=BOARDTYPE_PEXOR;
        strncpy(devnameformat,PEXORNAMEFMT,32);
        pex_msg(KERN_NOTICE "  Unknown board type, vendor id: 0x%x, device id:0x%x. Assuming pex mode...\n",vid,did);
    }


    for (ix = 0; ix < 6; ++ix)
        {
            privdata->bases[ix] = pci_resource_start(dev, ix);
            privdata->reglen[ix] = pci_resource_len(dev, ix);
            if (privdata->bases[ix] == 0)
            continue;
            /* JAM here workaround for wrong reglen from kinpex baro (old fpga code only!)*/
            if(privdata->board_type==BOARDTYPE_KINPEX)
            {
              if( privdata->reglen[ix]>PEX_KINPEX_BARSIZE)
                {
                    pex_dbg(KERN_NOTICE " KINPEX- Reducing exported barsize 0x%lx to 0x%x\n",privdata->reglen[ix],PEX_KINPEX_BARSIZE);
                    privdata->reglen[ix]=PEX_KINPEX_BARSIZE;
                }
            }
            if (pci_resource_flags(dev, ix) & IORESOURCE_IO)
                {
                    pex_dbg(KERN_NOTICE " - Requesting io ports for bar %d\n",ix);
                    if (request_region(privdata->bases[ix], privdata->reglen[ix],kobject_name(&dev->dev.kobj)) == NULL)
                        {
                                pex_dbg(KERN_ERR "I/O address conflict at bar %d for device \"%s\"\n",ix, kobject_name(&dev->dev.kobj));
                                cleanup_device(privdata);
                                return -EIO;
                        }
                    pex_dbg("requested ioport at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
                }
            else if (pci_resource_flags(dev, ix) & IORESOURCE_MEM)
                {
                  pex_dbg(KERN_NOTICE " - Requesting memory region for bar %d\n",ix);
                  if (request_mem_region(privdata->bases[ix], privdata->reglen[ix],
                         kobject_name(&dev->dev.kobj)) == NULL)
                    {
                      pex_dbg(KERN_ERR "Memory address conflict at bar %d for device \"%s\"\n", ix, kobject_name(&dev->dev.kobj));
                      cleanup_device(privdata);
                      return -EIO;
                    }
                  pex_dbg("requested memory at %lx with length %lx\n", privdata->bases[ix], privdata->reglen[ix]);
                  privdata->iomem[ix] = ioremap_nocache(privdata->bases[ix], privdata->reglen[ix]);
                  if (privdata->iomem[ix] == NULL)
                    {
                      pex_dbg(KERN_ERR "Could not remap memory  at bar %d for device \"%s\"\n",ix, kobject_name(&dev->dev.kobj));
                      cleanup_device(privdata);
                      return -EIO;
                    }
                  pex_dbg("remapped memory to %lx with length %lx\n", (unsigned long) privdata->iomem[ix], privdata->reglen[ix]);
                }
        } //for

// set pointer structures:
    set_pointers(&(privdata->regs), privdata->iomem[0], privdata->bases[0]);

    print_regs(&(privdata->regs));



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
    init_MUTEX_LOCKED(&(privdata->trix_sem));
#else
    sema_init (&(privdata->trix_sem), 0);
#endif
    privdata->trix_val = 0;
#endif

    /* debug: do we have valid ir pins/lines here?*/
    if ((err = pci_read_config_byte(dev, PCI_INTERRUPT_PIN, &(privdata->irqpin)))
            != 0)
        {
            pex_msg(
                    KERN_ERR "PEX pci driver probe: Error %d getting the PCI interrupt pin \n", err);
        }
    if ((err = pci_read_config_byte(dev, PCI_INTERRUPT_LINE,
            &(privdata->irqline))) != 0)
        {
            pex_msg(
                    KERN_ERR "PEX pci driver probe: Error %d getting the PCI interrupt line.\n", err);
        }
    snprintf(privdata->irqname, 64, devnameformat, atomic_read(&pex_numdevs));
    if (request_irq(dev->irq, irq_hand, IRQF_SHARED, privdata->irqname,
            privdata))
        {
            pex_msg( KERN_ERR "PEX pci_drv: IRQ %d not free.\n", dev->irq);
            cleanup_device(privdata);
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
            cleanup_device(privdata);
            return -ENOMSG;
        }

    privdata->devno =
            MKDEV(MAJOR(pex_devt), MINOR(pex_devt) + privdata->devid);

    /* Register character device */
    cdev_init(&(privdata->cdev), &pex_fops);
    privdata->cdev.owner = THIS_MODULE;
    privdata->cdev.ops = &pex_fops;
    err = cdev_add(&privdata->cdev, privdata->devno, 1);
    if (err)
        {
            pex_msg( "Couldn't add character device.\n");
            cleanup_device(privdata);
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

static void remove(struct pci_dev *dev)
{
    struct pex_privdata* priv = (struct pex_privdata*) pci_get_drvdata(dev);
    cleanup_device(priv);
    pex_msg(KERN_NOTICE "PEX pci driver end remove.\n");
}

//-----------------------------------------------------------------------------
static struct pci_driver pci_driver = { .name = PEXNAME, .id_table = ids,
        .probe = probe, .remove = remove, };
//-----------------------------------------------------------------------------

static int __init pex_init(void)
{

    int result;
    pex_msg(KERN_NOTICE "pex driver init...\n");
    pex_devt = MKDEV(my_major_nr, 0);

    /*
     * Register your major, and accept a dynamic number.
     */
    if (my_major_nr)
        {
            result = register_chrdev_region(pex_devt, PEX_MAXDEVS,
                    PEXNAME);
        }
    else
        {
            result = alloc_chrdev_region(&pex_devt, 0, PEX_MAXDEVS,
                    PEXNAME);
            my_major_nr = MAJOR(pex_devt);
        }
    if (result < 0)
        {
            pex_msg(
                    KERN_ALERT "Could not alloc chrdev region for major: %d !\n", my_major_nr);
            return result;
        }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
    pex_class = class_create(THIS_MODULE, PEXNAME);
    if (IS_ERR(pex_class))
        {
            pex_msg(KERN_ALERT "Could not create class for sysfs support!\n");
        }

#endif
    if (pci_register_driver(&pci_driver) < 0)
        {
            pex_msg(KERN_ALERT "pci driver could not register!\n");
            unregister_chrdev_region(pex_devt, PEX_MAXDEVS);
            return -EIO;
        }
    pex_msg(
            KERN_NOTICE "\t\tdriver init with registration for major no %d done.\n", my_major_nr);
    return 0;

    /* note: actual assignment will be done on probe time*/

}

static void __exit pex_exit(void)
{
    pex_msg(KERN_NOTICE "pex driver exit...\n");
    unregister_chrdev_region(pex_devt, PEX_MAXDEVS);
    pci_unregister_driver(&pci_driver);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
    if (pex_class != NULL ) class_destroy(pex_class);
#endif

    pex_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}

//-----------------------------------------------------------------------------
module_init(pex_init);
module_exit(pex_exit);
//-----------------------------------------------------------------------------