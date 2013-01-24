// N.Kurz, EE, GSI, 30-Mar-2010
// JAM modified for testing mmap with /dev/pexor, 24 Jan 2013

// only for the moment:
#define WAIT_SEM              12
#define POLL_SEM              16
#define GET_BAR0_BASE       0x1234
#define GET_BAR0_TRIX_BASE  0x1235
#define RESET_SEM           0x1236


#define PEX_MEM_OFF       0x100000
#define PEX_REG_OFF       0x20000
#define PEXOR_TRIXOR_BASE      0x40000
#define PEXOR_TRIX_CTRL 0x04
#define PEXOR_TRIX_STAT 0x00
#define PEXOR_TRIX_FCTI 0x08
#define PEXOR_TRIX_CVTI 0x0C

#define PCI_BAR0_SIZE     0x800000  // in bytes

#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>


main (int argc, char *argv[])
{
  int          l_status;
  int          fd_pex; 
  mode_t       mode;
  long         l_bar0; 
  long         l_trix_base;
  long         l_trix_val=0; 
  uid_t        uid;

  int            l_ret;



  // this one for pipe memory:
  size_t         len   = 0x50000000;//0x80000000; crashes
  off_t          off   = (off_t)0x40000000;
  int            prot  = PROT_WRITE | PROT_READ;
  int            flags = MAP_SHARED | MAP_LOCKED;
  char           *pa, *pe;

  int* pdat;
  int i;

  // try to read some pexor registers here:
  long  volatile *pl_virt_bar0;

  long  volatile *pl_dma_source_base;
  long  volatile *pl_dma_target_base;
  long  volatile *pl_dma_trans_size;
  long  volatile *pl_dma_burst_size;
  long  volatile *pl_dma_stat;
  long  volatile *pl_irq_control;
  long  volatile *pl_irq_status;
  long  volatile *pl_trix_fcti;
  long  volatile *pl_trix_cvti;

  if ((fd_pex = open ("/dev/pexor", O_RDWR)) == -1)
  {
    printf ("ERROR>> open /dev/pexor \n");
    exit(1);
  }

  // get BAR0:
  l_status = ioctl (fd_pex, GET_BAR0_BASE, &l_bar0);
  if (l_status == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_BASE failed \n");
	}
  else
  {
    printf ("BAR0:        0x%x \n", l_bar0); 
  }

  // get TRIXOR base:
  l_status = ioctl (fd_pex, GET_BAR0_TRIX_BASE, &l_trix_base);
  if (l_status == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_TRIX_BASE failed \n");
	}
  else
  {
    printf ("TRIXOR base: 0x%x \n", l_trix_base); 
  }

  // POLL_SEM:
  l_status = ioctl (fd_pex, POLL_SEM, &l_trix_val);
  if (l_status == -1 )
  {
    printf ("ERROR>> ioctl GET_BAR0_TRIX_BASE failed \n");
	}
  else
  {
    printf ("trix_val: %d \n", l_trix_val); 
  }



  // here test mapping of pipe:
  printf ("Test to map pipe at 0x%x, size: 0x%x, ...\n", off, len);
  if ((pa = (char *) mmap (NULL, len, prot, flags, fd_pex, off)) == MAP_FAILED)
   {
     printf ("failed to mmap pipe at pexor, return: 0x%x, %d \n", pa, pa);
     perror ("mmap");
   }
   else
   {
     pe = pa + len - 1;
     printf ("physical address:             0x%x \n", off);
     printf ("size:                         0x%x \n", len);
     printf ("first mapped virtual address: 0x%x \n", pa);
     printf ("last  mapped virtual address: 0x%x \n", pe);

     // io test:
     printf ("Writing values to pipe...\n", pe);
     i=0;
     for(pdat= (int*)pa; pdat< (int*) pe; pdat++)
         {
             *pdat=i++;
         }
     printf ("Wrote %d integers.\n", i);
     printf ("Reading values from pipe...\n", pe);
     i=0;
     for(pdat= (int*)pa; pdat< (int*) pe; pdat++)
             {
                 if (*pdat!=i)
                     printf ("IO Error at index %d, %d != %d...\n", i,*pdat, i);
                 ++i;
             }
     printf ("Read back %d integers.\n", i);


   }


  // here test mapping of bar0 registers:
      len=PCI_BAR0_SIZE;
    printf ("Test to map bar0 with size: 0x%x, ...\n", len);
    if ((pa = (char *) mmap (NULL, len, prot, flags, fd_pex, 0)) == MAP_FAILED)
     {
       printf ("failed to mmap bar0 from pexor, return: 0x%x, %d \n", pa, pa);
       perror ("mmap");
     }
     else
     {
       pe = pa + len - 1;
//       printf ("physical address:             0x%x \n", off);
//       printf ("size:                         0x%x \n", len);
       printf ("first mapped virtual address: 0x%x \n", pa);
       printf ("last  mapped virtual address: 0x%x \n", pe);

       pl_virt_bar0 = (long*)pa;

       pl_dma_source_base = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x0 );
       pl_dma_target_base = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x4 );
       pl_dma_trans_size  = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x8 );
       pl_dma_burst_size  = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0xc );
       pl_dma_stat        = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x10);

       pl_irq_control=(long*)((long)pl_virt_bar0 + PEXOR_TRIXOR_BASE + PEXOR_TRIX_CTRL);
       pl_irq_status=(long*)((long)pl_virt_bar0 +  PEXOR_TRIXOR_BASE + PEXOR_TRIX_STAT);
       pl_trix_fcti=(long*)((long)pl_virt_bar0 + PEXOR_TRIXOR_BASE + PEXOR_TRIX_FCTI);
       pl_trix_cvti=(long*)((long)pl_virt_bar0 + PEXOR_TRIXOR_BASE + PEXOR_TRIX_CVTI);


       printf ("Dump Pexor Registers:\n");
       printf ("DMA source:             0x%x \n", *pl_dma_source_base);
       printf ("DMA target:             0x%x \n", *pl_dma_target_base);
       printf ("DMA transfer size:             0x%x \n", *pl_dma_trans_size);
       printf ("DMA burst size:             0x%x \n", *pl_dma_burst_size);
       printf ("DMA status:             0x%x \n", *pl_dma_stat);
       printf ("Trixor control:             0x%x \n", *pl_irq_control);
       printf ("Trixor status:             0x%x \n", *pl_irq_status);
       printf ("Trixor fcti:             0x%x \n", *pl_trix_fcti);
       printf ("Trixor cvti:             0x%x \n", *pl_trix_cvti);
     }



}
