// N.Kurz, EE, GSI, 10-Jun-2013
// N.Kurz, EE, GSI, 11-Dec-2015: adoted for febex4 board

// JAM, EE, GSI, 02-Jun-2020: adjusted to mbspex/pexor driver library to avoid mapping bar 0

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
#include <ctype.h>


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

#define printm printf

#define MAX_INP_LEN      256
#define FLASH_MAN_ID     0x19ba20
#define IM_N_PAGES       0x10000   
#define FLASH_PAGE_SIZE  0x100
#define IM_BUF_SIZE      FLASH_PAGE_SIZE * IM_N_PAGES
#define FLASH_BLOCK_SIZE 0x40000
#define N_SECTORS        IM_BUF_SIZE / FLASH_BLOCK_SIZE 

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define GET_BAR0_BASE     0x1234
#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0_MBS"
#define PCI_BAR0_SIZE     0x800000  // in bytes

static int  l_i, l_j, l_k;
//static s_pexor        sPEXOR;
static long           l_stat;
static int            fd_pex; 
#ifndef Linux
 static int           l_bar0_base;
#endif //Linux
static INTU4 volatile *pl_virt_bar0;
static long           l_dat1, l_dat2, l_dat3;
static long           l_data; 

static  int            l_sfp_id;
static  int            l_fir_nyx_id;
static  int            l_las_nyx_id;
static  int            l_n_nyx;
static  char           c_in_file [MAX_INP_LEN];
static  unsigned char *pc_im_buf;
static  unsigned char *pc_im_tmp;
static  unsigned char *pc_im_tmp2;
static  INTU4         *pl_im_tmp;
static  FILE          *fd_im_file = NULL;
static  int            l_im_size;
static  int            l_wip; 
static  int            l_im_n_pages;

static  unsigned int   l_im_remain_size;
static  unsigned int   l_spi_trans_size;
static  unsigned int   l_spi_trans_size_code;
static  unsigned char *pc_im_verify_buf;
static  unsigned char *pc_im_verify_tmp;
static  unsigned char *pc_im_verify_tmp2;
static  INTU4         *pl_im_verify_tmp;
static  int            l_wait       = 0;  
static  int            l_ec_running = 0;
static  unsigned long  l_shift_im_size;

#ifdef DEBUG2
static  unsigned char *pc_flash;
static  unsigned char *pc_file;
#endif // DEBUG2

static char  progname [128];
static char  devname [128];



int  f_pex_slave_rd (long, long, long, long*);
int  f_pex_slave_wr (long, long, long,  long);
int  f_pex_slave_init (long, long);
void f_i2c_sleep ();
void f_flash_unprotect (int);
void f_flash_check_write_in_progress (int);
void f_flash_block_erase (int);
void f_flash_chip_erase (int);
void f_flash_load (int);
void f_flash_readback_verify (int);
void f_usage ();

int main(argc, argv)
int argc;
char *argv[];
{
  #ifdef Linux
  int            prot;
  int            flags;
  #endif // Linux

  char c_opt1 [20];
  char c_opt2 [20];
  int  l_ec=0;
  int  l_nv=0;
  int  l_vo=0;

  // JAM 2020 -here evaluate actual name of device for runtime messages:
  char* theSlave=0;
  int l_t=0;
  strncpy (progname, basename(argv[0]),128);
  theSlave=strstr(progname,"_");
  if(theSlave)
	  theSlave++;
  else
	  theSlave="slave";
  strncpy (devname, theSlave, 128);
  for(l_t=0;l_t<strlen(devname); ++l_t)
  {
	  devname[l_t]=toupper(devname[l_t]);
  }


  printf ("Starting program %s for frontend slave %s\n", progname, devname);


  if ( (argc < 5) || (argc > 7) )
  {
    f_usage ();
    exit (1);
  }
  else
  {
    sscanf (argv[1], "%d", &l_sfp_id);
    sscanf (argv[2], "%d", &l_fir_nyx_id);
    sscanf (argv[3], "%d", &l_n_nyx);
    sscanf (argv[4], "%s", c_in_file);
    if (argc > 5)
    {
      sscanf (argv[5], "%s",    c_opt1);
      //printf ("depp5: %s \n\n", c_opt1);
      if (strcmp (c_opt1, "-ec")  == 0)
      {
        l_ec = 1; //printf ("option -ec found \n"); 
      } 
      else if (strcmp (c_opt1, "-nv")  == 0)
      {
        l_nv = 1; //printf ("option -nv found \n"); 
      } 
      else if (strcmp (c_opt1, "-vo")  == 0)
      {
        l_vo = 1; //printf ("option -vo found \n"); 
      }
      else
      {
        printf ("unkown option >%s<  exiting.. \n", c_opt1);
        exit (0);
      } 
    }
    if (argc > 6)
    {
      sscanf (argv[6], "%s",    c_opt2);
      //printf ("depp5: %s \n\n", c_opt2);
      if (strcmp (c_opt2, "-ec")  == 0)
      {
        l_ec = 1; //printf ("option -ec found \n"); 
      } 
      else if (strcmp (c_opt2, "-nv")  == 0)
      {
        l_nv = 1; //printf ("option -nv found \n"); 
      } 
      else if (strcmp (c_opt2, "-vo")  == 0)
      {
        l_vo = 1; //printf ("option -vo found \n"); 
      }
      else
      {
        printf ("\n unkown option >%s< \n\n", c_opt2);
        f_usage ();
      } 
    }

    if ( (l_vo == 1) && ((l_ec == 1) || (l_nv == 1)) )
    { 
      printf ("\n options are in contradiction \n\n");    
      f_usage ();
    }


    l_las_nyx_id = l_fir_nyx_id + l_n_nyx - 1;
    if (l_n_nyx > 1)
    {
      printf ("use flash on sfp %d on all %ss from id %d to %d \n",
                               l_sfp_id, devname, l_fir_nyx_id, l_las_nyx_id);
    }
    else
    {
      printf ("use flash on sfp %d on %s id %d \n", l_sfp_id, devname,  l_fir_nyx_id);
    }
    printf ("fpga bit file specified: %s \n", c_in_file);   
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

  // initialize gosip on all connected FEBEX4s
  PEXOR_GetPointer(0, pl_virt_bar0, &sPEXOR); 

#endif // mbspex lib  




  l_stat = f_pex_slave_init (l_sfp_id, l_las_nyx_id+1);  
  if (l_stat == -1)
  {
    printf ("ERROR>> %s initialization failed on sfp %d \n", devname, l_sfp_id);
    printf ("exiting...\n"); 
    exit (0); 
  }  

  pc_im_buf = (char*) malloc (sizeof(char) * IM_BUF_SIZE);
  if (pc_im_buf == NULL)
  {
    printf ("ERROR>> could not malloc buffer for fpga bit file, exiting... \n");
    exit (0);
  }
  else
  {
    printf ("allocated 0x%x (%d) bytes for fpga bit file buffer \n", IM_BUF_SIZE, IM_BUF_SIZE); 
  }
  pc_im_tmp = pc_im_buf;  
  for (l_i=0; l_i<IM_BUF_SIZE; l_i++) { *pc_im_tmp++ = 0xff; }

  pc_im_verify_buf = (char*) malloc (sizeof(char) * IM_BUF_SIZE);
  if (pc_im_verify_buf == NULL)
  {
    printf ("ERROR>> could not malloc read back buffer for verification, exiting... \n");
    exit (0);
  }
  else
  {
    printf ("allocated 0x%x (%d) bytes for read back verification buffer \n", IM_BUF_SIZE, IM_BUF_SIZE); 
  }
  pc_im_verify_tmp = pc_im_verify_buf;  
  for (l_i=0; l_i<IM_BUF_SIZE; l_i++) { *pc_im_verify_tmp++ = 0xff; }

  fd_im_file = fopen (c_in_file, "r");
  if (fd_im_file == NULL)
  {
    printf ("ERROR>> could not open fpga bit file %s, exiting... \n", c_in_file);
    exit (0);
  }
 
  l_im_size = 0;
  do
  {
    l_im_size += fread ((void *)(pc_im_buf + l_im_size), sizeof(char), FLASH_PAGE_SIZE, fd_im_file);
  } while ((feof (fd_im_file) == 0) && (l_im_size <= IM_BUF_SIZE));

  fclose (fd_im_file);

  printf ("fpga bit file %s of 0x%x (%d) bytes read into buffer \n", c_in_file, l_im_size, l_im_size); 

  // do some necessary byte gymnastics with input file buffer
  pc_im_tmp       = pc_im_buf;
  pc_im_tmp2      = pc_im_buf + 113;
  l_shift_im_size = l_im_size - 113;

  for (l_i=0; l_i<l_shift_im_size; l_i++)
  { 
    *pc_im_tmp++ = *pc_im_tmp2++;
  }
  for (l_i=0; l_i<113; l_i++)
  {
    *pc_im_tmp++ = 0xff;
  }

  l_im_n_pages = (l_im_size/FLASH_PAGE_SIZE) + 1;
  printm ("%d pages of 256 bytes to load \n", l_im_n_pages);


  printf ("\n\ncheck if all flash memories specified show the correct manufacturer id \n");   
  for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
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
    //printf ("flash status of                           sfp %d, febex4 %d: 0x%x \n", l_sfp_id, l_i, l_data & 0xff);

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
      printf ("%s SFP %d, ID %2d: flash memory manufacturer id found: 0x%x is ok! \n", devname, l_sfp_id, l_i, l_data & 0xffffff);
    }
    else if (l_data == 0xadadadad)
    {
      printf ("ERROR>> wrong flash memory manufacturer id found on   sfp %d, %s %d \n", l_sfp_id, devname, l_i);
      printf ("        must be: 0x%x, found: 0x%x, exiting.. \n", FLASH_MAN_ID, l_data);
      printf ("0xadadadad indicates that no SPI master interface is available, exiting..\n");
      exit (0);  
    }
    else
    {
      printf ("ERROR>> wrong flash memory manufacturer id found on   sfp %d, %s %d \n", l_sfp_id, devname, l_i);
      printf ("        must be: 0x%x, found: 0x%x, exiting.. \n", FLASH_MAN_ID, l_data);
      exit (0);  
    }
  }

  // we have 5 modes

  if ( (l_ec == 0) && (l_nv == 0) && (l_vo == 0) ) //standard
  {
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      f_flash_unprotect       (l_i);
      f_flash_block_erase     (l_i);
      f_flash_load            (l_i);
      f_flash_readback_verify (l_i);
    }
    printf ("\n\n Summmary: \n");
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      printf ("%s SFP %d, ID %2d: flash loading and verification successfully finished \n", devname, l_sfp_id, l_i);
    }
  }

  if ( (l_ec == 0) && (l_nv == 1) )
  {
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      f_flash_unprotect   (l_i);
      f_flash_block_erase (l_i);
      f_flash_load        (l_i);
    }
    printf ("\n\n Summmary: \n");
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      printf ("%s SFP %d, ID %2d: flash successfully programmed. no verification \n", devname, l_sfp_id, l_i);
    }
  }

  if ( (l_ec == 1) && (l_nv == 0))
  {
    printf ("\n");
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      f_flash_unprotect  (l_i);
      f_flash_chip_erase (l_i);
    }
    l_wait = 5;  // sec 
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      if (l_i == l_fir_nyx_id)
      {
        l_ec_running = 1;
      }
      else
      {
        l_ec_running = 0;
      }
      f_flash_check_write_in_progress (l_i);
      printf ("%s SFP %d, ID %2d: erase finished\n", devname, l_sfp_id, l_i);
    }
    l_ec_running = 0;
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      f_flash_load            (l_i);
      f_flash_readback_verify (l_i);
    }
    printf ("\n\n Summmary: \n");
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      printf ("%s SFP %d, ID %2d: flash loading and verification successfully finished \n", devname, l_sfp_id, l_i);
    }
  }

  if ( (l_ec == 1) && (l_nv == 1))
  {
    printf ("\n");
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      f_flash_unprotect  (l_i);
      f_flash_chip_erase (l_i);
    }
    l_wait = 5;  // sec
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      if (l_i == l_fir_nyx_id)
      {
        l_ec_running = 1;
      }
      else
      {
        l_ec_running = 0;
      }
      f_flash_check_write_in_progress (l_i);
      printf ("%s SFP %d, ID %2d: erase finished\n", devname, l_sfp_id, l_i);
    }
    l_ec_running = 0;
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      f_flash_load (l_i);
    }
    printf ("\n\n Summmary: \n");
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      printf ("%s SFP %d, ID %2d: flash loading successfully finished. no verifcation \n", devname, l_sfp_id, l_i);
    }
  }

  if (l_vo == 1)
  {
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      f_flash_readback_verify (l_i);
    }
    printf ("\n\n Summmary: \n");
    for (l_i=l_fir_nyx_id; l_i<=l_las_nyx_id; l_i++)
    {
      printf ("%s SFP %d, ID %2d: flash verification successfully finished \n", devname, l_sfp_id, l_i);
    }
  }
return 0;
}
/*****************************************************************************/

void f_flash_check_write_in_progress (int l_nyx_ident)
{
  int l_ct1 = 0;
  int l_ct2 = 0;
  //check write in progress bit (#0) of flash is 0. 
  l_wip = 1;  // write in progress bit of flash status register
  l_ct1 = 0;
  l_ct2 = 0;
  while (l_wip == 1)
  {
    if (l_ec_running == 1)
    {
      sleep (l_wait);
      printf ("still erasing.. \n");
      l_ct1++;
      if (l_ct2 > 100)
      {
        printf ("erase should be finished, but isn't! if you want to abort type ctrl-c \n"); 
      } 
    }
    else
    {
      l_ct2++;
      if (l_ct2 > 100000)
      {
        sleep (1);
        printf ("write in progress should be finished, but isn't! if you want to abort type ctrl-c \n"); 
      } 

    }
    //usleep (250000); 
    //read flash status register
    f_pex_slave_wr (l_sfp_id, l_nyx_ident, 0x300010, 0x0);    
    f_pex_slave_wr (l_sfp_id, l_nyx_ident, 0x300018, 0x1001); 
    f_pex_slave_wr (l_sfp_id, l_nyx_ident, 0x300018, 0x0);    
    f_pex_slave_wr (l_sfp_id, l_nyx_ident, 0x300010, 0x5000000); 
    f_pex_slave_wr (l_sfp_id, l_nyx_ident, 0x300018, 0x1000);    
    f_pex_slave_wr (l_sfp_id, l_nyx_ident, 0x300018, 0x0);
    f_pex_slave_wr (l_sfp_id, l_nyx_ident, 0x300008, 0x100);     
    l_data = 0;
    f_pex_slave_rd (l_sfp_id, l_nyx_ident, 0x300004, &l_data);
    if ((l_data & 0x1) == 0) {l_wip = 0;}   
    //printf ("flash status after block %2d erased:       sfp %d, febex4 %d: 0x%x \n", l_j, l_sfp_id, l_nyx_ident, l_data & 0xff);

    // inifinite loop protection to be done
  }
}

/*****************************************************************************/

void f_flash_unprotect (int l_nyx_id)
{
  // flash write enable
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x6000000); // spi commando RDID to data in 
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);    // spi write en high for addr 0 - control register
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);       // spi write enable low

  //read flash status register
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x0);        // max data word to spi data in  
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1001);     // spi write en high for addr 1 - status register  
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);        // spi write en low 
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x5000000);  // spi commando: read flash status register 
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);     // spi write en high for addr 0 - control register
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300008, 0x100);      // memory read command + address
  l_data = 0;
  f_pex_slave_rd (l_sfp_id, l_nyx_id, 0x300004, &l_data);    // read memory data out 
  //printf ("flash status after flash write enable:    sfp %d, febex4 %d: 0x%x \n", l_sfp_id, l_nyx_id, l_data & 0xff);

  // write 0 to flash status register (unprotect all sectors in trb code)

  // flash write enable
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x6000000);
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);
  // write 0 to spi mem
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300000, 0x0);        
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300008, 0x1000);    
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300008, 0x0);       
  // write flash status
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x1000000); 
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000); 
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);    

  f_flash_check_write_in_progress (l_nyx_id);
}

/*****************************************************************************/

void f_flash_block_erase (int l_nyx_id)
{
  // erase first 8 MB of flash (32 blocks x 0x40000 bytes), flash total: 8 MB febex4
  printf ("\n%sSFP %d, ID %2d: start erasing first 8 MB (out of 16 MB) in flash memory \n", devname, l_sfp_id, l_nyx_id);
  for (l_j=0; l_j<N_SECTORS; l_j++)
  {
    // flash write enable
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x6000000);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);
    // erase block
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0xd8000000 + (l_j * FLASH_BLOCK_SIZE));
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);

    f_flash_check_write_in_progress (l_nyx_id);
    if (((l_j%10) == 0) && (l_j !=0) )
    { 
      printf ("%s SFP %d, ID %2d: %3d blocks of 256 Kbytes erased \n", devname, l_sfp_id, l_nyx_id, l_j);
    }
  }
  printf ("%s SFP %d, ID %2d: %3d blocks of 256 Kbytes erased \n", devname, l_sfp_id, l_nyx_id, l_j);
  printf ("%s SFP %d, ID %2d: erase finished\n", devname, l_sfp_id, l_nyx_id);
}

/*****************************************************************************/
void f_flash_chip_erase (int l_nyx_id)
{
  printf ("%s SFP %d, ID %2d: start erasing full flash memory (needs about 4 minutes)\n", devname, l_sfp_id, l_nyx_id);
  // flash write enable
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x6000000);
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);
  // erase chip
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0xc7000000);
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
  f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);
}

/*****************************************************************************/
void f_flash_load (int l_nyx_id)
{
  // load flash
  printf ("\n%s SFP %d, ID %2d: start loading fpga bit file into flash memory \n", devname, l_sfp_id, l_nyx_id);
  pl_im_tmp = (INTU4*) pc_im_buf; 
  l_im_remain_size = l_im_size;
  for (l_j=0; l_j<l_im_n_pages; l_j++)
  {
    // write 256 bytes into spi communication memory inside fpga
    for (l_k=0; l_k<64; l_k++)
    {
      f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300000, *pl_im_tmp++);       
      f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300008, 0x1000 + l_k);
      f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300008, 0x0);           
    }

    // check for last page size
    if (l_im_remain_size >= FLASH_PAGE_SIZE)
    {
      l_spi_trans_size_code = (FLASH_PAGE_SIZE - 1) << 24;
    }
    else 
    {
      l_spi_trans_size_code = (l_im_remain_size - 1) << 24;
    }
    l_im_remain_size -= FLASH_PAGE_SIZE; 
    //printf ("page nr: %d, spi transfer size long word: 0x%x \n", l_j, l_spi_trans_size_code);      

    // set memory size usage of spi
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, l_spi_trans_size_code);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1001);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);

    // flash write enable
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x6000000);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);

    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x2000000 + l_j * FLASH_PAGE_SIZE);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);
  
    f_flash_check_write_in_progress (l_nyx_id);

    if (((l_j%1000) == 0) && (l_j !=0) )
    { 
      printf ("%s SFP %d, ID %2d: %5d pages of 256 bytes loaded \n", devname, l_sfp_id, l_nyx_id, l_j);
    }
  } 
  printf ("%s SFP %d, ID %2d: %5d pages of 256 bytes loaded \n", devname, l_sfp_id, l_nyx_id, l_j);
  printf ("%s SFP %d, ID %2d: loading finished \n", devname, l_sfp_id, l_nyx_id);
}

/*****************************************************************************/
void f_flash_readback_verify (int l_nyx_id)
{
  int l_ct = 0;
  // read back flash and compare with image buffer from input file
  printf ("\n%s SFP %d, ID %2d: start reading flash memory \n", devname, l_sfp_id, l_nyx_id);
  pl_im_verify_tmp  = (INTU4*) pc_im_verify_buf; 
  pc_im_verify_tmp2 = pc_im_verify_buf;
  pc_im_tmp2        = pc_im_buf;
  l_im_remain_size  = l_im_size;
  for (l_j=0; l_j<l_im_n_pages; l_j++)
  {
    // check for last page size
    if (l_im_remain_size >= FLASH_PAGE_SIZE)
    {
      l_spi_trans_size      =  FLASH_PAGE_SIZE;
      l_spi_trans_size_code = (FLASH_PAGE_SIZE - 1) << 24;
    }
    else 
    {
      l_spi_trans_size      =  l_im_remain_size;
      l_spi_trans_size_code = (l_im_remain_size - 1) << 24;
    }
    l_im_remain_size -= FLASH_PAGE_SIZE; 
    //printf ("page nr: %d, spi transfer size long word: 0x%x \n", l_j, l_spi_trans_size_code);      

    // set memory size usage of spi
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, l_spi_trans_size_code);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1001);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);

    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300010, 0x3000000 + l_j * FLASH_PAGE_SIZE);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x1000);
    f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300018, 0x0);

    // wait until SPI is not busy
    l_data = 1;
    l_ct   = 0;
    while (l_data == 1)
    {
      f_pex_slave_rd (l_sfp_id, l_nyx_id, 0x30001c, &l_data);
      l_data = l_data & 1;
      l_ct++;
      if (l_ct > 3)
      {
        sleep (1);
        printf ("SPI busy should be off, but is still on! if you want to abort type ctrl-c \n"); 
      } 
    }
    //sleep (1);
    for (l_k=0; l_k<64; l_k++)
    {
      f_pex_slave_wr (l_sfp_id, l_nyx_id, 0x300008, 0x100 + l_k);     
      f_pex_slave_rd (l_sfp_id, l_nyx_id, 0x300004, &l_data);
      *pl_im_verify_tmp++ = l_data;
    }

    #ifdef DEBUG2
    pc_flash = pc_im_verify_tmp2;
    pc_file  = pc_im_tmp2;
    
    printf ("file: \n");
    for (l_k=0; l_k<256; l_k++)
    {
      printf ("%2x ", *pc_file++);
      if (((l_k+1) % 16) == 0) {printf ("\n");}
    }
    printf ("\n");
    printf ("readback flash: \n");   
    for (l_k=0; l_k<256; l_k++)
    {
      printf ("%2x ", *pc_flash++);
      if (((l_k+1) % 16) == 0) {printf ("\n");}
    }
    printf ("\n");
    #endif //DEBUG2


    if ((memcmp (pc_im_verify_tmp2, pc_im_tmp2, l_spi_trans_size)) != 0)
    {
      printf ("ERROR>> input bit file and read back buffer are different, exiting.. \n");
      printf ("page: %d \n", l_j);
      exit (0);
    }

    pc_im_verify_tmp2 += l_spi_trans_size;
    pc_im_tmp2        += l_spi_trans_size;

    if (((l_j%1000) == 0) && (l_j !=0) )
    { 
      printf ("%s SFP %d, ID %2d: %5d pages of 256 bytes read back and verified with input file, ok!\n", devname, l_sfp_id, l_nyx_id, l_j);
    }
  } 
  printf ("%s SFP %d, ID %2d: %5d pages of 256 bytes read back and verified with input file, ok!\n", devname,  l_sfp_id, l_nyx_id, l_j);
  printf ("%s SFP %d, ID %2d: verification finished \n", devname, l_sfp_id, l_nyx_id);
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
    l_dat1 = 0; l_dat2 = 0; l_dat3 = 0;
    l_stat = PEXOR_RX (&sPEXOR, l_sfp, &l_dat1 , &l_dat2, &l_dat3);
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
  #define N_LOOP 1500
  //#define N_LOOP 1000000

  int l_ii;
  int volatile l_depp=0; 
 
  for (l_ii=0; l_ii<N_LOOP; l_ii++)
  {
    l_depp++;
  }
}

/*****************************************************************************/

void f_usage ()
{
  printf ("\n*** %s v 1.05 03-June 2020 by NK, JAM (GSI EEL department) *** \n", progname);
  printf ("\nusage of %s,  < > are mandatory parameters   \n\n", progname);
  printf ("%s <sfp id> <first %s id> <# of %ss> <bitfile input fn> \n", progname, devname, devname);
  printf ("%s <sfp id> <first %s id> <# of %ss> <bitfile input fn> -nv \n", progname, devname, devname);
  printf ("%s <sfp id> <first %s id> <# of %ss> <bitfile input fn> -ec\n", progname, devname, devname);
  printf ("%s <sfp id> <first %s id> <# of %ss> <bitfile input fn> -ec -nv \n", progname, devname, devname);
  printf ("%s <sfp id> <first %s id> <# of %ss> <bitfile input fn> -vo \n", progname, devname, devname);
  printf ("\n");
  printf ("no option: block erase, load, verify \n");
  printf (" -nv     : no verify \n");
  printf (" -ec     : chip  erase (parallel erase, faster if more than 2 %ss specified) \n",devname);
  printf (" -vo     : only verify (-nv and -vo are exclusive) \n");
  exit (0);
}

/*****************************************************************************/
