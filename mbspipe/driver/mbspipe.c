/*
 *  linux/drivers/char/mem.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  Added devfs support.
 *    Jan-11-1998, C. Scott Ananian <cananian@alumni.princeton.edu>
 *  Shared /dev/zero mmapping support, Feb 2000, Kanoj Sarcar <kanoj@sgi.com>
 *
 *   ***********************
 *   This module for /dev/mbspipe is just a truncated version of mem.c that supports only mmap of physical memory
 *   for pipe buffer of MBS data acquisition systems on IFC power PC VMEvbus linux
 *   This avoids to set capability CAP_SYS_RAWIO for mbs user process (setcap does not work on MBS diskless fs)
 *   v0.01 by JAM (j.adamczewski@gsi.de)
 */

#include <linux/mm.h>
#include <linux/miscdevice.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/mman.h>
#include <linux/random.h>
#include <linux/init.h>
#include <linux/raw.h>
#include <linux/tty.h>
#include <linux/capability.h>
#include <linux/ptrace.h>
#include <linux/device.h>
#include <linux/highmem.h>
#include <linux/backing-dev.h>
#include <linux/splice.h>
#include <linux/pfn.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/uio.h>

#include <linux/uaccess.h>

#include <linux/module.h>
#include <linux/version.h>

// JAM 9-2020: following configs are set in ifc-1 kernel:

#define CONFIG_DEVMEM 1
// this one would require function devmem_is_allowed that is part of module arch/powerpc/mem.c
// unclear how we get to this kernel symbol here?
//#define CONFIG_STRICT_DEVMEM 1
#define CONFIG_MMU 1
#define CONFIG_HAVE_IOREMAP_PROT 1


#ifdef CONFIG_IA64
# include <linux/efi.h>
#endif


/////////////////////////////// JAM take here some useful things from another known driver...

#define MBSPIPE_DEBUGPRINT 1

// this define will enable read/write fops
// if disabled, only mmmap is supported
//#define MBSPIPE_WITH_READ_WRITE 1


#define MBSPIPEVERSION     "0.1.2"
#define MBSPIPEAUTHORS     "Joern Adamczewski-Musch (JAM), GSI Darmstadt (www.gsi.de)"
#define MBSPIPEDESC        "MBS pipe host memory mapping module for IFC Linux"



#ifdef MBSPIPE_DEBUGPRINT
#define mbspipe_dbg( args... )                    \
  printk( args );
#else
#define mbspipe_dbg( args... ) ;
#endif

/** maximum number of devices controlled by this driver*/
#define MBSPIPE_MAXDEVS 4

#define mbspipe_msg( args... )                    \
  printk( args );


/** from mbspex driver to simpplify init exit for generic dev id*/
static dev_t mbspipe_devt;
static atomic_t mbspipe_numdevs = ATOMIC_INIT(0);
static int my_major_nr = 0;


/* these were in privdata, for the moment put it singleton*/
static struct device *class_dev; /**< Class device */
static struct cdev cdev; /**< char device struct */
static dev_t devno; /**< device number (major and minor) */
static int devid; /**< local id (counter number) */


#define MBSPIPENAME       "mbspipe"
#define MBSPIPENAMEFMT    "mbspipe%d"



#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
static struct class* mbspipe_class;



ssize_t mbspipe_sysfs_codeversion_show (struct device *dev, struct device_attribute *attr, char *buf)
{

    ssize_t curs=0;
    curs += snprintf (buf + curs, PAGE_SIZE, "*** This is %s, version %s build on %s at %s \n",
        MBSPIPEDESC, MBSPIPEVERSION, __DATE__, __TIME__);
    curs += snprintf (buf + curs, PAGE_SIZE, "\tmodule authors: %s \n", MBSPIPEAUTHORS);

#ifdef MBSPIPE_WITH_READ_WRITE
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tfile operations read, write, seek are enabled.\n");
#else
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tonly mmap is allowed! other fops are disabled.\n");
#endif
  curs += snprintf (buf + curs, PAGE_SIZE, "\t\tmmap without phys_mem_access_prot -> _PAGE_COHERENT mode.\n");
    return curs;
}

static DEVICE_ATTR(codeversion, S_IRUGO, mbspipe_sysfs_codeversion_show, NULL);


#endif


///////////////////////////////////// BELOW original mem.c code JAM JAM2020

static inline unsigned long size_inside_page(unsigned long start,
					     unsigned long size)
{
	unsigned long sz;

	sz = PAGE_SIZE - (start & (PAGE_SIZE - 1));

	return min(sz, size);
}

#ifndef ARCH_HAS_VALID_PHYS_ADDR_RANGE
static inline int valid_phys_addr_range(phys_addr_t addr, size_t count)
{
	return addr + count <= __pa(high_memory);
}

static inline int valid_mmap_phys_addr_range(unsigned long pfn, size_t size)
{
	return 1;
}
#endif

//#ifdef CONFIG_STRICT_DEVMEM
//static inline int range_is_allowed(unsigned long pfn, unsigned long size)
//{
//	u64 from = ((u64)pfn) << PAGE_SHIFT;
//	u64 to = from + size;
//	u64 cursor = from;
//
//	while (cursor < to) {
//		if (!devmem_is_allowed(pfn)) {
//			printk(KERN_INFO
//		"Program %s tried to access /dev/mem between %Lx->%Lx.\n",
//				current->comm, from, to);
//			return 0;
//		}
//		cursor += PAGE_SIZE;
//		pfn++;
//	}
//	return 1;
//}
//#else
static inline int range_is_allowed(unsigned long pfn, unsigned long size)
{
	return 1;
}

//#endif

#ifndef unxlate_dev_mem_ptr
#define unxlate_dev_mem_ptr unxlate_dev_mem_ptr
void __weak unxlate_dev_mem_ptr(phys_addr_t phys, void *addr)
{
}
#endif



///////////////////////////////////////////////////////////////////////////////7
/// JAM2020 additional fops are disabled if not set the define
#ifdef MBSPIPE_WITH_READ_WRITE

/*
 * This funcion reads the *physical* memory. The f_pos points directly to the
 * memory location.
 */
static ssize_t read_mem(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	phys_addr_t p = *ppos;
	ssize_t read, sz;
	void *ptr;

	if (p != *ppos)
		return 0;

	if (!valid_phys_addr_range(p, count))
		return -EFAULT;
	read = 0;
#ifdef __ARCH_HAS_NO_PAGE_ZERO_MAPPED
	/* we don't have page 0 mapped on sparc and m68k.. */
	if (p < PAGE_SIZE) {
		sz = size_inside_page(p, count);
		if (sz > 0) {
			if (clear_user(buf, sz))
				return -EFAULT;
			buf += sz;
			p += sz;
			count -= sz;
			read += sz;
		}
	}
#endif

	while (count > 0) {
		unsigned long remaining;

		sz = size_inside_page(p, count);

		if (!range_is_allowed(p >> PAGE_SHIFT, count))
			return -EPERM;

		/*
		 * On ia64 if a page has been mapped somewhere as uncached, then
		 * it must also be accessed uncached by the kernel or data
		 * corruption may occur.
		 */
		ptr = xlate_dev_mem_ptr(p);
		if (!ptr)
			return -EFAULT;

		remaining = copy_to_user(buf, ptr, sz);
		unxlate_dev_mem_ptr(p, ptr);
		if (remaining)
			return -EFAULT;

		buf += sz;
		p += sz;
		count -= sz;
		read += sz;
	}

	*ppos += read;
	return read;
}

static ssize_t write_mem(struct file *file, const char __user *buf,
			 size_t count, loff_t *ppos)
{
	phys_addr_t p = *ppos;
	ssize_t written, sz;
	unsigned long copied;
	void *ptr;

	if (p != *ppos)
		return -EFBIG;

	if (!valid_phys_addr_range(p, count))
		return -EFAULT;

	written = 0;

#ifdef __ARCH_HAS_NO_PAGE_ZERO_MAPPED
	/* we don't have page 0 mapped on sparc and m68k.. */
	if (p < PAGE_SIZE) {
		sz = size_inside_page(p, count);
		/* Hmm. Do something? */
		buf += sz;
		p += sz;
		count -= sz;
		written += sz;
	}
#endif

	while (count > 0) {
		sz = size_inside_page(p, count);

		if (!range_is_allowed(p >> PAGE_SHIFT, sz))
			return -EPERM;

		/*
		 * On ia64 if a page has been mapped somewhere as uncached, then
		 * it must also be accessed uncached by the kernel or data
		 * corruption may occur.
		 */
		ptr = xlate_dev_mem_ptr(p);
		if (!ptr) {
			if (written)
				break;
			return -EFAULT;
		}

		copied = copy_from_user(ptr, buf, sz);
		unxlate_dev_mem_ptr(p, ptr);
		if (copied) {
			written += sz - copied;
			if (written)
				break;
			return -EFAULT;
		}

		buf += sz;
		p += sz;
		count -= sz;
		written += sz;
	}

	*ppos += written;
	return written;
}


/*
 * The memory devices use the full 32/64 bits of the offset, and so we cannot
 * check against negative addresses: they are ok. The return value is weird,
 * though, in that case (0).
 *
 * also note that seeking relative to the "end of file" isn't supported:
 * it has no meaning, so it returns -EINVAL.
 */
static loff_t memory_lseek(struct file *file, loff_t offset, int orig)
{
  loff_t ret;

  mutex_lock(&file_inode(file)->i_mutex);
  switch (orig) {
  case SEEK_CUR:
      offset += file->f_pos;
  case SEEK_SET:
      /* to avoid userland mistaking f_pos=-9 as -EBADF=-9 */
      if (IS_ERR_VALUE((unsigned long long)offset)) {
          ret = -EOVERFLOW;
          break;
      }
      file->f_pos = offset;
      ret = file->f_pos;
      force_successful_syscall_return();
      break;
  default:
      ret = -EINVAL;
  }
  mutex_unlock(&file_inode(file)->i_mutex);
  return ret;
}





#endif
//MBSPIPE_WITH_READ_WRITE

int __weak phys_mem_access_prot_allowed(struct file *file,
	unsigned long pfn, unsigned long size, pgprot_t *vma_prot)
{
	return 1;
}

#ifndef __HAVE_PHYS_MEM_ACCESS_PROT

/*
 * Architectures vary in how they handle caching for addresses
 * outside of main memory.
 *
 */
#ifdef pgprot_noncached
static int uncached_access(struct file *file, phys_addr_t addr)
{
#if defined(CONFIG_IA64)
	/*
	 * On ia64, we ignore O_DSYNC because we cannot tolerate memory
	 * attribute aliases.
	 */
	return !(efi_mem_attributes(addr) & EFI_MEMORY_WB);
#elif defined(CONFIG_MIPS)
	{
		extern int __uncached_access(struct file *file,
					     unsigned long addr);

		return __uncached_access(file, addr);
	}
#else
	/*
	 * Accessing memory above the top the kernel knows about or through a
	 * file pointer
	 * that was marked O_DSYNC will be done non-cached.
	 */
	if (file->f_flags & O_DSYNC)
		return 1;
	return addr >= __pa(high_memory);
#endif
}
#endif

static pgprot_t phys_mem_access_prot(struct file *file, unsigned long pfn,
				     unsigned long size, pgprot_t vma_prot)
{
#ifdef pgprot_noncached
	phys_addr_t offset = pfn << PAGE_SHIFT;

	if (uncached_access(file, offset))
		return pgprot_noncached(vma_prot);
#endif
	return vma_prot;
}
#endif

#ifndef CONFIG_MMU
static unsigned long get_unmapped_area_mem(struct file *file,
					   unsigned long addr,
					   unsigned long len,
					   unsigned long pgoff,
					   unsigned long flags)
{
	if (!valid_mmap_phys_addr_range(pgoff, len))
		return (unsigned long) -EINVAL;
	return pgoff << PAGE_SHIFT;
}

/* permit direct mmap, for read, write or exec */
static unsigned memory_mmap_capabilities(struct file *file)
{
	return NOMMU_MAP_DIRECT |
		NOMMU_MAP_READ | NOMMU_MAP_WRITE | NOMMU_MAP_EXEC;
}

static unsigned zero_mmap_capabilities(struct file *file)
{
	return NOMMU_MAP_COPY;
}

/* can't do an in-place private mapping if there's no MMU */
static inline int private_mapping_ok(struct vm_area_struct *vma)
{
	return vma->vm_flags & VM_MAYSHARE;
}
#else

static inline int private_mapping_ok(struct vm_area_struct *vma)
{
	return 1;
}
#endif

static const struct vm_operations_struct mmap_mem_ops = {
#ifdef CONFIG_HAVE_IOREMAP_PROT
	.access = generic_access_phys
#endif
};

static int mmap_mem(struct file *file, struct vm_area_struct *vma)
{
	size_t size = vma->vm_end - vma->vm_start;

	if (!valid_mmap_phys_addr_range(vma->vm_pgoff, size))
		return -EINVAL;

	if (!private_mapping_ok(vma))
		return -ENOSYS;

	if (!range_is_allowed(vma->vm_pgoff, size))
		return -EPERM;

	if (!phys_mem_access_prot_allowed(file, vma->vm_pgoff, size,
						&vma->vm_page_prot))
		return -EINVAL;

////////////////////////////////////////////////////////////
// JAM 9-9-2020: this one will slow down speed by factor 10:
//
//	vma->vm_page_prot = phys_mem_access_prot(file, vma->vm_pgoff,
//						 size,
//						 vma->vm_page_prot);
////
// delivers 0x54023d (flags for bok3e-MMU) : _PAGE_NO_CACHE, _PAGE_GUARDED, _PAGE_ACCESSED, _PAGE_PSIZE_4K,  _PAGE_RW,  _PAGE_USER
// without calling this function, vm_page_prot has:
//          0x24023d: _PAGE_COHERENT, PAGE_ACCESSED, _PAGE_PSIZE_4K,  _PAGE_RW,  _PAGE_USER


	// JAM 8-Oct-2020: fiddle for performance with dedicated flags:
	//vma->vm_page_prot |= _PAGE_NO_CACHE; //0x400000 from  /usr/include/arch/powerpc/include/asm/pte-book3e.h
	// => only slightly (9%) faster than phys_mem_access_prot, but safe -> 64023d

	//vma->vm_page_prot |= _PAGE_GUARDED; //0x100000 -> 0x34023d
	// almost as fast as without guarded (82%), but still faster as with phys_mem_access_prot (11 x)
	// however, many errors when reading out mem module

	// same as phys_mem_access_prot without the _PAGE_GUARDED - 0x44023d
//	vma->vm_page_prot &= ~_PAGE_COHERENT;
//	vma->vm_page_prot |= _PAGE_NO_CACHE;
	// NO ERRORS, same as without clearing the _PAGE_COHERENT


	// another try... 0xa4023d
	//vma->vm_page_prot |= _PAGE_WRITETHRU;
	// as fast as with page_coherent only, but also with errors

	// still standing: 0x8423
//	 vma->vm_page_prot &= ~_PAGE_COHERENT;
//	 vma->vm_page_prot |= _PAGE_WRITETHRU;
	 // ERRORS, same as a4023d - 40 errs/day

	// maybe guarded access helps?
//	vma->vm_page_prot &= ~_PAGE_COHERENT;
//	vma->vm_page_prot |= _PAGE_WRITETHRU;
	//vma->vm_page_prot |= _PAGE_GUARDED;
	// MANY ERRORS even without mem readout

	// 0x44023d -fastest setup without erors (?): - 60 errors/day with read mem module
//	vma->vm_page_prot &= ~_PAGE_COHERENT;
//	vma->vm_page_prot |= _PAGE_NO_CACHE;

	// another one 0xe423d : also 60 err/day
//	vma->vm_page_prot |= _PAGE_WRITETHRU;
//	vma->vm_page_prot |= _PAGE_NO_CACHE;

	////////////////////////////////////////////////////////////////////////////////////////////
// vm_flags was 0xFB -> may read, may write , may execute, may share, read, write, shared

	vma->vm_ops = &mmap_mem_ops;


	mbspipe_dbg(KERN_NOTICE "mbspipe - mmap_mem has set vm_page_prot=0x%lx, vm_flags=0x%lx",
	    (long) vma->vm_page_prot, (long) vma->vm_flags);
//#ifdef CONFIG_HAVE_IOREMAP_PROT
//	mbspipe_dbg(KERN_NOTICE "mbspipe with CONFIG_HAVE_IOREMAP_PROT uses generic_access_phys");
//#endif

	/* Remap-pfn-range will mark the range VM_IO */
	if (remap_pfn_range(vma,
			    vma->vm_start,
			    vma->vm_pgoff,
			    size,
			    vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}






static int open_port(struct inode *inode, struct file *filp)
{
	//return capable(CAP_SYS_RAWIO) ? 0 : -EPERM;
    return 0; // JAM 2020 - this is main difference to /dev/mem
}

#define write_iter_zero	write_iter_null
#define open_mem	open_port

static const struct file_operations __maybe_unused mbspipe_fops = {
    .owner = THIS_MODULE,
#ifdef MBSPIPE_WITH_READ_WRITE
    .llseek		= memory_lseek,
	.read		= read_mem,
	.write		= write_mem,
#endif
	.mmap		= mmap_mem,
	.open		= open_mem,
#ifndef CONFIG_MMU
	.get_unmapped_area = get_unmapped_area_mem,
	.mmap_capabilities = memory_mmap_capabilities,
#endif
};


///////////////////////////////////////////////////////////////////////////////
//// BEGIN original dev init of mem.c




//static const struct memdev {
//	const char *name;
//	umode_t mode;
//	const struct file_operations *fops;
//	fmode_t fmode;
//} devlist[] = {
//#ifdef CONFIG_DEVMEM
//	 [1] = { "mem", 0, &mem_fops, FMODE_UNSIGNED_OFFSET },
//#endif
//#ifdef CONFIG_DEVKMEM
//	 [2] = { "kmem", 0, &kmem_fops, FMODE_UNSIGNED_OFFSET },
//#endif
//	 [3] = { "null", 0666, &null_fops, 0 },
//#ifdef CONFIG_DEVPORT
//	 [4] = { "port", 0, &port_fops, 0 },
//#endif
//	 [5] = { "zero", 0666, &zero_fops, 0 },
//	 [7] = { "full", 0666, &full_fops, 0 },
//	 [8] = { "random", 0666, &random_fops, 0 },
//	 [9] = { "urandom", 0666, &urandom_fops, 0 },
//#ifdef CONFIG_PRINTK
//	[11] = { "kmsg", 0644, &kmsg_fops, 0 },
//#endif
//};

//static int memory_open(struct inode *inode, struct file *filp)
//{
//	int minor;
//	const struct memdev *dev;
//
//	minor = iminor(inode);
//	if (minor >= ARRAY_SIZE(devlist))
//		return -ENXIO;
//
//	dev = &devlist[minor];
//	if (!dev->fops)
//		return -ENXIO;
//
//	filp->f_op = dev->fops;
//	filp->f_mode |= dev->fmode;
//
//	if (dev->fops->open)
//		return dev->fops->open(inode, filp);
//
//	return 0;
//}

//static const struct file_operations memory_fops = {
//	.open = memory_open,
//	.llseek = noop_llseek,
//};
//
//static char *mem_devnode(struct device *dev, umode_t *mode)
//{
//	if (mode && devlist[MINOR(dev->devt)].mode)
//		*mode = devlist[MINOR(dev->devt)].mode;
//	return NULL;
//}
//
//static struct class *mem_class;
//
//static int __init chr_dev_init(void)
//{
//	int minor;
//
//	if (register_chrdev(MEM_MAJOR, "mbspipe", &memory_fops))
//		printk("unable to get major %d for MBS pipe devs\n", MEM_MAJOR);
//
//	mem_class = class_create(THIS_MODULE, "mbspipe");
//	if (IS_ERR(mem_class))
//		return PTR_ERR(mem_class);
//
//	mem_class->devnode = mem_devnode;
//	for (minor = 1; minor < ARRAY_SIZE(devlist); minor++) {
//		if (!devlist[minor].name)
//			continue;
//
//		/*
//		 * Create /dev/port?
//		 */
////		if ((minor == DEVPORT_MINOR) && !arch_has_dev_port())
////			continue;
//
//		device_create(mem_class, NULL, MKDEV(MEM_MAJOR, minor),
//			      NULL, devlist[minor].name);
//	}
//
//	return tty_init(); // JAM2020 - to avoid dependency loops on module load if it fails?
//}
//
//fs_initcall(chr_dev_init);

/////////////////////////////////////7


// JAM2020 - we do it in the known way, but skip the privdata stuff...

static int __init mbspipe_init (void)
{

  int result;
  int err = 0;
  char devname[64];

  mbspipe_msg(KERN_NOTICE "mbspipe driver init...\n");
  mbspipe_devt = MKDEV(my_major_nr, 0);

  /*
   * Register your major, and accept a dynamic number.
   */
  if (my_major_nr)
  {
    result = register_chrdev_region (mbspipe_devt, MBSPIPE_MAXDEVS, MBSPIPENAME);
  }
  else
  {
    result = alloc_chrdev_region (&mbspipe_devt, 0, MBSPIPE_MAXDEVS, MBSPIPENAME);
    my_major_nr = MAJOR(mbspipe_devt);
  }
  if (result < 0)
  {
    mbspipe_msg(
        KERN_ALERT "Could not alloc chrdev region for major: %d !\n", my_major_nr);
    return result;
  }
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)

  mbspipe_class = class_create (THIS_MODULE, MBSPIPENAME);
  if (IS_ERR (mbspipe_class))
  {
    mbspipe_msg(KERN_ALERT "Could not create class for sysfs support!\n");
  }

#endif


  // following things were in probe of mbspex, but we know that memory is plugged in...
  devid = atomic_inc_return(&mbspipe_numdevs) - 1;
  devno = MKDEV(MAJOR(mbspipe_devt), MINOR(mbspipe_devt) + devid);
  /* Register character device */
   cdev_init (&(cdev), &mbspipe_fops);
   cdev.owner = THIS_MODULE;
   cdev.ops = &mbspipe_fops;
   err = cdev_add (&cdev, devno, 1);
   if (err)
   {
     mbspipe_msg( "Couldn't add character device.\n");
     //cleanup_device (privdata);
     return err;
   }

   /* export special things to class in sysfs: */
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
   if (!IS_ERR (mbspipe_class))
   {
     /* driver init had successfully created class, now we create device:*/
     snprintf (devname, 64, "mbspipe%d", MINOR(mbspipe_devt) + devid);
 #if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
     //privdata->class_dev =

         class_dev=device_create (mbspipe_class, NULL, devno, NULL, devname);
 #else
     //privdata->class_dev =
      class_dev=device_create(mbspipe_class, NULL,
         devno, devname);
 #endif
     //dev_set_drvdata (privdata->class_dev, privdata);
     mbspipe_msg (KERN_NOTICE "Added MBSPIPE device: %s", devname);

     if (device_create_file (class_dev, &dev_attr_codeversion) != 0)
     {
       mbspipe_msg (KERN_ERR "Could not add device file node for code version.\n");
     }
   }
     else
       {
         /* something was wrong at class creation, we skip sysfs device support here:*/
         mbspipe_msg(KERN_ERR "Could not add MBSPIPE device node to sysfs !");
       }

#endif


  mbspipe_msg(
      KERN_NOTICE "\t\tdriver init with registration for major no %d done. Device name is %s\n",
      my_major_nr, devname);
  return 0;


}

static void __exit mbspipe_exit (void)
{
  mbspipe_msg(KERN_NOTICE "mbspipe driver exit...\n");

// taken from remove of mbspex:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  /* sysfs device cleanup */
  if (class_dev)
  {
    device_remove_file (class_dev, &dev_attr_codeversion);
    device_destroy (mbspipe_class, devno);
    class_dev = 0;
  }

  #endif

  /* character device cleanup*/
   if (cdev.owner)
     cdev_del (&cdev);
   if (devid)
     atomic_dec (&mbspipe_numdevs);


// from exit:
  unregister_chrdev_region (mbspipe_devt, MBSPIPE_MAXDEVS);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
  if (mbspipe_class != NULL )
    class_destroy (mbspipe_class);
#endif

  mbspipe_msg(KERN_NOTICE "\t\tdriver exit done.\n");
}

//-----------------------------------------------------------------------------
module_init(mbspipe_init);
module_exit(mbspipe_exit);
//-----------------------------------------------------------------------------


MODULE_AUTHOR(MBSPIPEAUTHORS);
MODULE_DESCRIPTION(MBSPIPEDESC);
MODULE_LICENSE("GPL");
MODULE_VERSION(MBSPIPEVERSION);

