// N.Kurz, EE, GSI, 10-Jun-2013
// JAM, EE, GSI, 07-May-2020: adjusted to mbspex/pexor driver library to avoid mapping bar 0

#define USE_MBSPEX_LIB 1

#include "stdio.h"
//#include "s_veshe.h"
#include "stdarg.h"
#include <sys/file.h>

#ifndef USE_MBSPEX_LIB
#ifndef Linux
 #include <mem.h>
 #include <smem.h>
#else
 #include "smem_mbs.h"
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <sys/mman.h>
#endif
#endif

#ifdef USE_MBSPEX_LIB
#include "mbspex/libmbspex.h"

#include <stdio.h>
#include <errno.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include <stdarg.h>

void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
  printf ("%s", c_str);
  va_end(args);
}


#else
#include  "./pexor_gosip.h"

static s_pexor         sPEXOR;
#define printm printf

#endif //USE_MBSPEX_LIB

// JAM 20202: put here MBS types to be independent of it
typedef signed   char   CHARS;
typedef unsigned char   CHARU;
typedef char            CHARX;
typedef signed    char  INTS1;
typedef unsigned char   INTU1;
typedef char            INTX1;

typedef   signed short  INTS2;
typedef unsigned short  INTU2;
typedef   signed int    INTS4;
typedef unsigned int    INTU4;
typedef          float  REAL4;
typedef          double REAL8;

typedef          long INTS8;
typedef unsigned long INTU8;
typedef unsigned long ADDRS;
typedef unsigned long long ADDR64; // JAM for 64 bit status structure receiving

//#define DEBUG
//#define DEBUG2
//#define DEBUG3

#define printm printf

#define MAX_INP_LEN      256
#define FLASH_MAN_ID     0x1720c2
#define IM_N_PAGES       0x4000   
#define FLASH_PAGE_SIZE  0x100
#define IM_BUF_SIZE      FLASH_PAGE_SIZE * IM_N_PAGES
#define FLASH_BLOCK_SIZE 0x10000

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define GET_BAR0_BASE     0x1234
#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0_MBS"
#define PCI_BAR0_SIZE     0x800000  // in bytes

static int  l_i, l_j;
//static s_pexor        sPEXOR;
static long           l_stat;
static int            fd_pex; 
#ifndef Linux
 static int            l_bar0_base;
#endif // Linux
static INTU4 volatile *pl_virt_bar0;
static long           l_dat1, l_dat2, l_dat3;
static long           l_data; 

static  int            l_sfp_id;
static  int            l_fir_feb_id;
static  int            l_las_feb_id;
static  int            l_n_feb;

int  f_pex_slave_rd (long, long, long, long*);
int  f_pex_slave_wr (long, long, long,  long);
int  f_pex_slave_init (long, long);
void f_i2c_sleep ();
void f_usage (const char *progname);

int main(argc, argv)
int argc;
char *argv[];
{
  #ifdef Linux
  int            prot;
  int            flags;
  #endif // Linux

  if ( argc != 4 )
  {
    f_usage (basename (argv[0]));
    exit (1);
  }
  else
  {
    sscanf (argv[1], "%d", &l_sfp_id);
    sscanf (argv[2], "%d", &l_fir_feb_id);
    sscanf (argv[3], "%d", &l_n_feb);

    l_las_feb_id = l_fir_feb_id + l_n_feb - 1;
    if (l_n_feb > 1)
    {
      printf ("reset fpgas on sfp %d on all febex3/tamex from id %d to %d \n",
                               l_sfp_id, l_fir_feb_id, l_las_feb_id);
    }
    else
    {
      printf ("reset fpga on sfp %d on febex3/tamex id %d \n", l_sfp_id, l_fir_feb_id);
    }
  }

  // open PEXOR device 
#ifdef USE_MBSPEX_LIB
    if ((fd_pex = mbspex_open (0)) == -1)
       {
         printm (RON"ERROR>>"RES" could not open mbspex device \n");
         exit (0);
       }

#else
  
  
  
  
  
  if ((fd_pex = open (PEXDEV, O_RDWR)) == -1)
  {
    printf ("ERROR>> could not open %s device \n", PEXDEV);
    exit (0);
  }
  else
  {
    printf ("opened device: %s, fd = %d \n", PEXDEV, fd_pex);
  }

  #ifdef Linux
  // map bar0 directly via pexor driver and access trixor base
  prot  = PROT_WRITE | PROT_READ;
  flags = MAP_SHARED | MAP_LOCKED;
  if ((pl_virt_bar0 = (INTU4 volatile *) mmap (NULL, PCI_BAR0_SIZE, prot, flags, fd_pex, 0)) == MAP_FAILED)
  {
    printf (RON"failed to mmap bar0 from pexor"RES", return: 0x%x, %d \n", pl_virt_bar0, pl_virt_bar0);
    perror ("mmap"); 
    exit (-1);
  } 
  #ifdef DEBUG
  printf ("first mapped virtual address of bar0: 0x%p \n", pl_virt_bar0);
  #endif // DEBUG

  #else // Linux

  // get bar0 base:
  l_stat = ioctl (fd_pex, GET_BAR0_BASE, &l_bar0_base);
  if (l_stat == -1 )
  {
    printf (RON"ERROR>>"RES" ioctl GET_BAR0_BASE failed \n");
  }
  else
  {
    printf ("PEXOR bar0 base: 0x%x \n", l_bar0_base);
  }

  // open shared segment
  smem_remove(PCI_BAR0_NAME);
  if((pl_virt_bar0 = (INTU4 volatile *) smem_create (PCI_BAR0_NAME,
            (char*) l_bar0_base, PCI_BAR0_SIZE, SM_READ | SM_WRITE))==NULL)
  {
    printf ("smem_create for PEXOR BAR0 failed");
    exit (-1);
  }
  #endif // Linux

  // close pexor device
  l_stat = close (fd_pex);
  if (l_stat == -1 )
  {
    printf ("ERROR>> could not close PEXOR device \n");
  }

  // initialize gosip on all connected FEBEXs
  PEXOR_GetPointer(0, pl_virt_bar0, &sPEXOR); 

#endif // mbspex lib  
  


  
  l_stat = f_pex_slave_init (l_sfp_id, l_las_feb_id+1);  
  if (l_stat == -1)
  {
    printf ("ERROR>> febex3/tamex initialization failed on sfp %d \n", l_sfp_id);
    printf ("exiting...\n"); 
    exit (0); 
  }  

  printf ("\n\ncheck if all flash memories specified show the correct manufacturer id \n");   
  for (l_i=l_fir_feb_id; l_i<=l_las_feb_id; l_i++)
  {
    //read flash status register
    f_pex_slave_wr (l_sfp_id, l_i, 0x300010, 0x0);        // max data word to spi data in  
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x1001);     // spi write en high for addr 1 - status register  
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x0);        // spi write en low 
    f_pex_slave_wr (l_sfp_id, l_i, 0x300010, 0x5000000);  // spi commando: read flash status register 
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x1000);     // spi write en high for addr 0 - control register
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x0);
    f_pex_slave_wr (l_sfp_id, l_i, 0x300008, 0x100);      // memory read command + address
    l_data = 0;
    f_pex_slave_rd (l_sfp_id, l_i, 0x300004, &l_data);    // read memory data out 
    //printf ("flash status of                           sfp %d, febex %d: 0x%x \n", l_sfp_id, l_i, l_data & 0xff);

    // read manufacturer id, memory tpe and memory density
    f_pex_slave_wr (l_sfp_id, l_i, 0x300010, 0x2000000);  // max data word to spi data in  
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x1001);     // spi write en high for addr 1 - status register  
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x0);        // spi write en low 
    f_pex_slave_wr (l_sfp_id, l_i, 0x300010, 0x9f000000); // spi commando RDID to data in 
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x1000);     // spi write en high for addr 0 - control register
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x0);
    f_pex_slave_wr (l_sfp_id, l_i, 0x300008, 0x100);      // memory read command + address
    l_data = 0;
    f_pex_slave_rd (l_sfp_id, l_i, 0x300004, &l_data);    // read memory data out 
    if ((l_data & 0xffffff) == FLASH_MAN_ID)
    {
      printf ("FEBEX3 SFP %d, ID %2d: flash memory manufacturer id found: 0x%x is ok! \n", l_sfp_id, l_i, l_data & 0xffffff);
    }
    else if (l_data == 0xadadadad)
    {
      printf ("ERROR>> wrong flash memory manufacturer id found on   sfp %d, febex %d \n");
      printf ("        must be: 0x%x, found: 0x%x, exiting.. \n", FLASH_MAN_ID, l_data);
      printf ("0xadadadad indicates that no SPI master interface is available, exiting..\n");
      exit (0);  
    }
    else
    {
      printf ("ERROR>> wrong flash memory manufacturer id found on   sfp %d, febex %d \n");
      printf ("        must be: 0x%x, found: 0x%x, exiting.. \n", FLASH_MAN_ID, l_data);
      exit (0);  
    }
  }
  for (l_i=l_las_feb_id; l_i>=l_fir_feb_id; l_i--)
  {
    printf ("FEBEX3/TAMEX SFP %d, ID %2d: reloading of fpaga from flash and fpga restart started.. \n", l_sfp_id, l_i);
    usleep (1); 
    f_pex_slave_wr (l_sfp_id, l_i, 0x300018, 0x80000000);
  }

  printf (".. will take ~3 seconds.. \n"); 
  sleep (3);
  printf ("please check with ini_chane or gosipcmd -i, if reset worked..\n");

/*
  for (l_i=l_fir_feb_id; l_i<=l_las_feb_id; l_i++)
  {
    usleep (1); 
    f_pex_slave_rd (l_sfp_id, l_i, 0x300018, &l_data);
    if (l_data == 0)
    {
      printf ("FEBEX3 SFP %d, ID %2d: fpga restarted and running \n", l_sfp_id, l_i);
    }
    else
    {
      printf ("ERROR>> fpga restart failed on sfp %d, febex %d, exiting.. \n", l_sfp_id, l_i); 
      exit (0);  
    }
  }
*/
return 0;
}
/*****************************************************************************/

int f_pex_slave_init (long l_sfp, long l_n_slaves)
{
 #ifdef USE_MBSPEX_LIB
  return mbspex_slave_init (fd_pex, l_sfp, l_n_slaves);
#else
  int  l_ret;
  long l_comm;

  printm ("initialize SFP chain %d \n", l_sfp);
  l_comm = PEXOR_INI_REQ | (0x1<<16+l_sfp);

  for (l_j=1; l_j<=10; l_j++)
  {
    PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp); 
    PEXOR_TX (&sPEXOR, l_comm, 0, l_n_slaves  - 1) ;

    //printm ("SFP %d: try nr. %d \n", l_sfp, l_j);
    l_stat = 0; l_dat1 = 0; l_dat2 = 0; l_dat3 = 0;
    l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, &l_dat3);

    printf ("l_stat: %d, dat: 0x%x 0x%x 0x%x \n", l_stat, l_dat1, l_dat2, l_dat3);

    if ( (l_stat != -1) && (l_dat2 > 0) && (l_dat2<=32))
    {
      break;
    }
    #ifndef Linux 
    yield ();
    #else
    sched_yield ();
    #endif
  }
  l_ret = 0;
  if (l_stat == -1)
  {
    l_ret = -1;
    printm ("ERROR>> initialization of SFP chain %d failed. ", l_sfp);
    printm ("no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, l_dat3);
    printm ("exiting.. \n"); exit (0);
  }
  else
  {
    if (l_dat2 != 0)
    { 
      printm ("initialization for SFP chain %d done. \n", l_sfp),
      printm ("No of slaves : %d \n", l_dat2);
    }
    else
    {
      l_ret = -1;
      printm ("ERROR>> initialization of SFP chain %d failed. ", l_sfp);
      printm ("no slaves found. \n"); 
      printm ("exiting.. \n"); exit (0);
    }
  }
  return (l_ret);
  
#endif // not MBSPEX LIB
}


/*****************************************************************************/

int f_pex_slave_wr (long l_sfp, long l_slave, long l_slave_off, long l_dat)
{
  int  l_ret;
#ifdef USE_MBSPEX_LIB

  l_ret = mbspex_slave_wr (fd_pex, l_sfp, l_slave, l_slave_off, l_dat);

#else
 
    

  long l_comm;
  long l_addr;

  l_comm = PEXOR_PT_AD_W_REQ | (0x1<<16+l_sfp);
  l_addr = l_slave_off + (l_slave << 24);
  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp); 
  PEXOR_TX (&sPEXOR, l_comm, l_addr, l_dat);
  l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, &l_dat3); 

  l_ret = 0;   
  if (l_stat == -1)
  {
    l_ret = -1;
    #ifdef DEBUG
    printm ("ERROR>> writing to SFP: %d, slave id: %d, addr 0x%d \n",
                                                l_sfp, l_slave, l_slave_off);
    printm ("  no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, l_dat3);
    #endif // DEBUG
  }
  else
  {
    // printm ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
    if( (l_dat1 & 0xfff) == PEXOR_PT_AD_W_REP)
    {
      //printm ("SFP: %d, slave id: %d addr: 0x%x  \n",
      //                l_sfp, (l_dat2 & 0xf0000) >> 24, l_dat2 & 0xfffff);
      if ( (l_dat1 & 0x4000) != 0)
      {
        l_ret = -1;
        #ifdef DEBUG
        printm ("ERROR>> packet structure: command reply 0x%x \n", l_dat1);
        #endif // DEBUG
      }
    }
    else
    {
      l_ret = -1;
      #ifdef DEBUG
      printm ("ERROR>> writing to empty slave or wrong address: \n");
      printm ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
           l_sfp, l_slave, (l_addr & 0xf00000) >> 24 , l_addr & 0xfffff, l_dat1);
      #endif // DEBUG
    }
  }

#endif //MBSPEX LIB
  f_i2c_sleep ();
  return (l_ret);
}

/*****************************************************************************/

int f_pex_slave_rd (long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{

 int  l_ret;
#ifdef USE_MBSPEX_LIB

 l_ret=mbspex_slave_rd (fd_pex, l_sfp, l_slave, l_slave_off, l_dat);

#else

  long l_comm;
  long l_addr;

  l_comm = PEXOR_PT_AD_R_REQ | (0x1<<16+l_sfp);
  l_addr = l_slave_off + (l_slave << 24);
  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp); 
  PEXOR_TX (&sPEXOR, l_comm, l_addr, 0);
  l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, l_dat); 
  //printm ("f_pex_slave_rd, l_dat: 0x%x, *l_dat: 0x%x \n", l_dat, *l_dat);

  l_ret = 0;
  if (l_stat == -1)
  {
    l_ret = -1;
    #ifdef DEBUG
    printm ("ERROR>> reading from SFP: %d, slave id: %d, addr 0x%d \n",
                                  l_sfp, l_slave, l_slave_off);
    printm ("  no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, *l_dat);
    #endif // DEBUG
  }
  else
  {
    // printm ("Reply to PEXOR from SFP: 0x%x ", l_sfp);
    if( (l_dat1 & 0xfff) == PEXOR_PT_AD_R_REP)
    {
      //printm ("SFP: %d, slave id: %d addr: 0x%x  \n",
      //     l_sfp, (l_dat2 & 0xf00000) >> 24, l_dat2 & 0xfffff);
      if ( (l_dat1 & 0x4000) != 0)
      {
        l_ret = -1;
        #ifdef DEBUG
        printm ("ERROR>> packet structure: command reply 0x%x \n", l_dat1);
        #endif //DEBUG
      }
    }
    else
    {
      l_ret = -1;
      #ifdef DEBUG 
      printm ("ERROR>> Reading from empty slave or wrong address: \n");
      printm ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
              l_sfp, l_slave, (l_addr & 0xf0000) >> 24 , l_addr & 0xfffff, l_dat1);
      #endif // DEBUG
    }
  }

 #endif //MBSPEX LIB
  f_i2c_sleep ();
  return (l_ret);
}

/*****************************************************************************/

void f_i2c_sleep ()
{
  //#define N_LOOP 300000
  #define N_LOOP 1000000

  int l_ii;
  int volatile l_depp=0; 
 
  for (l_ii=0; l_ii<N_LOOP; l_ii++)
  {
    l_depp++;
  }
}

/*****************************************************************************/

void f_usage (const char *progname)
{
  printf ("\n*** %s v 1.0 02-Jun 2020 by NK, JAM (GSI EEL department) *** \n", progname);
  printf ("\nwrong parameter list specified \n\n");
  printf ("%s <sfp id> <first febex3/tamex id> <# of febex3/tamex>  \n\n",progname); 

  exit (0);
}

/*****************************************************************************/
