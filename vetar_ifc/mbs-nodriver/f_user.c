// N.Kurz, EE, GSI, 4-Jul-2014
// adjusted for ifc 11-Apr-2022 JAM
// version for ifc without kernel module 14-Jun-2022 JAM
/////////////////////////////////////////////////////////
// white rabbit vetar tlu time stamp triggered readout
/////////////////////////////////////////////////////////
// description of functions:
// on startup, first f_user_get_virt_ptr() is called:
//      -> f_vetar_init();
//          ->f_ifc_a32_vme_mas_map_CRCSR() = map crcsr space of VETAR to user space
//          * set ADER registers to define address windows for a24 (wishbone control) and a32 (tlu access)
//          -> f_ifc_a24_vme_mas_map_CONTROL() = map a24 control window to user space
//          * write a24 control registers to setup direct access of TLU via a32 register
//      ->f_ifc_a32_vme_mas_map_TLU(= map a32 registers of TLU to user space
////
// at begin of readout (start acquisition):   f_user_init():
//      ->f_wr_init () = select and initialize TLU fifo
///
// for each triggered event: f_user_readout()
//      * *pl_dat is pointer to current data word in output subevent buffer
//      * TLU registers are read, formatted and copied to subevent buffer with leading keywords
//      * TLU fifo is popped, fill count is checked before and after
//      * in case of errors try to repair TLU and restart: ->f_wr_reset_tlu_fifo()
////// JAM - 14-jun-2022 /////////////////////////////////
//------------- ---------------------------------------------------------------

//#define USER_TRIG_CLEAR    1
//#define USER_TRIG_CLEAR_2  1

#define WR_RELEASE_ENIGMA 1  // JAM 8-2019: changed TLU addresses for ENIGMA

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define STATISTIC     1000000
#define MAX_TRIG_TYPE      16

#define VETAR_REGS_ADDR   0x50000000ULL
// for slot number 5, high ADER mapping (in driver)
#define VETAR_REGS_SIZE   0x1000000

#define VETAR_CRCSR_ADDR 0x280000
// configbase CRCSR for slot 5
#define VETAR_CRCSR_SIZE 0x80000
/* size of cr/csr space*/

#define VETAR_CTRL_ADDR 0x1400   
// 5 * 0x400
#define VETAR_CTRL_SIZE 0xA0

#define PAGE_SHIFT 0x1000

#define VETAR_VENDOR_ID		0x80031

/* VETAR CR/CSR offsets: */
#define VME_VENDOR_ID_OFFSET    0x24
#define BOARD_ID        0x33
#define REVISION_ID     0x43
#define PROG_ID         0x7F

#define FUN0ADER    0x7FF63
#define FUN1ADER    0x7FF73
#define INT_LEVEL   0x7ff5b
#define INTVECTOR   0x7ff5f
#define WB_32_64    0x7ff33
#define BIT_SET_REG 0x7FFFB
#define BIT_CLR_REG 0x7FFF7
#define TIME        0x7FF3F
#define BYTES       0x7FF37

/* VETAR CR/CSR control values: */
#define WB32        1
#define WB64        0
#define RESET_CORE  0x80
#define ENABLE_CORE 0x10

/* VME WB Interface*/
#define CTRL 16
#define MASTER_CTRL 24
#define MASTER_ADD 32
#define MASTER_DATA 40
#define EMUL_DAT_WD 48
#define WINDOW_OFFSET_LOW  56
#define WINDOW_OFFSET_HIGH 64

/* new control register by Michael Reese 2017:*/
#define DIRECT_ACCESS_CONTROL 4

//----------------------------------------------------------------------------

#include "stdio.h"
#include "s_veshe.h"
#include "stdarg.h"
#include <sys/file.h>
#include <stdint.h>   /* uint32_t ... */

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

#include "sbs_def.h"
#include "error_mac.h"
#include "errnum_def.h"
#include "err_mask_def.h"
#include "f_ut_printm.h"
#include "f_user_trig_clear.h"

typedef uint64_t eb_data_t;
//////////////////////////////////////////////
// from gsi_tm_latch.h
//register offsets
#define GSI_TM_LATCH_FIFO_READY      0x000   //n..0 channel(n) timestamp(s) ready       (ro)
#define GSI_TM_LATCH_FIFO_CLEAR      0x004   //n..0 channel(n) FIFO clear               (wo)
#define GSI_TM_LATCH_TEST_CHANNELS       0x008   //Generate a test Event                                        (wo)
#define GSI_TM_LATCH_TRIG_ARMSTAT    0x00C   //n..0 channel(n) trigger armed status     (ro)
#define GSI_TM_LATCH_TRIG_ARMSET     0x010   //n..0 channel(n) trigger set armed        (wo)
#define GSI_TM_LATCH_TRIG_ARMCLR     0x014   //n..0 channel(n) trigger clr armed        (wo)
#define GSI_TM_LATCH_TRIG_EDGESTAT   0x018       //n..0 channel(n) trigger edge status          (ro)
#define GSI_TM_LATCH_TRIG_EDGEPOS    0x01C       //n..0 channel(n) trigger edge set pos         (wo)
#define GSI_TM_LATCH_TRIG_EDGENEG    0x020       //n..0 channel(n) trigger edge set neg         (wo)

// Channels Related Parameters
#define GSI_TM_LATCH_CHNS_TOTAL          0x034   // Total Number of Channels in Device          (ro)
#define GSI_TM_LATCH_CHNS_FIFOSIZE       0x038   // Total size of FIFOs                                         (ro)

// Channel to be selected n....0 and the other operations depend on selected channel
#define GSI_TM_LATCH_CH_SELECT           0x058   //Channel Select                                                       (rw)

// *IMP* All operations below depend on the Channel Selection
#define GSI_TM_LATCH_FIFO_POP        0x05C       //pop the topmost FIFO Q Element           (wo)
#define GSI_TM_LATCH_FIFO_TEST           0x060   // Generate a test Event Pulse                         (wo)
//pop just adjusts the pointer to the FIFO, it does not re-write a default value
#define GSI_TM_LATCH_FIFO_CNT        0x064       //FIFO Queue   fill count                  (ro)
#define GSI_TM_LATCH_FIFO_FTSHI      0x068       //timestamp HIGH words in cycles               (ro)
#define GSI_TM_LATCH_FIFO_FTSLO      0x06c       //timestamp LOW words in cycles            (ro)
#define GSI_TM_LATCH_FIFO_FTSSUB     0x070       //timestamp sub-cycle                          (ro)
//////////////////////////////////////////////

#define SUB_SYSTEM_ID      0x600
#define TS__ID_L16         0x3e1
#define TS__ID_M16         0x4e1
#define TS__ID_H16         0x5e1
#define TS__ID_X16         0x6e1

#define WR_TLU_FIFO_NR       3        // vetar vme
void f_vetar_init ();
void f_wr_init ();
void f_wr_reset_tlu_fifo ();

static INTU4 *pl_dat_save;
static unsigned long l_tr_ct[MAX_TRIG_TYPE];
static int l_i;
static long l_first = 0;

#ifdef WR_RELEASE_ENIGMA
// following is for enigma release:
static int tlu_address = 0x2000100;

#else
// this is for doomsday and before:
static int tlu_address = 0x4000100;
#endif

/* follwoing for direct mapping of ifc mem JAM 25-03-2022:*/
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <errno.h>
#include <altmasioctl.h>
#include <altulib.h>
//int triva_fd;
const char* triva_driverhandle = "/dev/trigvme";
void f_ifc_a32_vme_mas_map_TLU ();
void f_ifc_a32_vme_mas_free_TLU ();
int alt_fd;
static struct alt_ioctl_map_win alt_win;

static struct alt_ioctl_map_win alt_win_crcsr;
static struct alt_ioctl_map_win alt_win_control;

// registers in the crcsr space:

static INTS4* cr_vendor_id;
static INTS4* cr_bit_set;
static INTS4* cr_bit_clear;
static INTS4* cr_wb_32_64;
static INTS4* cr_fun0_ader;
static INTS4* cr_fun1_ader;

// JAM22: registers in the a32 region:
static INTS4* ch_select;
static INTS4* ch_fifosize;
static INTS4* fifoclear;
static INTS4* armset;

static INTS4* fifo_ready;
static INTS4* fifo_cnt;
static INTS4* ft_shi;
static INTS4* ft_slo;
static INTS4* ft_ssub;
static INTS4* fifo_pop;

// JAM22: registers for a24 control space:

static INTS4* ctrl_master_control;
static INTS4* ctrl_master_data;
static INTS4* ctrl_emul_dat_wd;
static INTS4* ctrl_window_offset_low;
static INTS4* ctrl_direct_access_control;

static char* p_TLU;
static INTU4* pl_virt_vme_base_tlu;
static INTU4* pl_virt_vme_base_crcsr;
static INTU4* pl_virt_vme_base_control;

static long l_eb_first1 = 0;
static long l_used_tlu_fifo = 1 << WR_TLU_FIFO_NR;
static eb_data_t eb_fifo_size;
static eb_data_t eb_tlu_high_ts;    // high 32 bit
static eb_data_t eb_tlu_low_ts;    // low  32 bit
static eb_data_t eb_tlu_fine_ts;    // fine 3 bit  ! subject of change !

static eb_data_t eb_tlu_high_ts_prev;    // high 32 bit
static eb_data_t eb_tlu_low_ts_prev;    // low  32 bit
static eb_data_t eb_tlu_fine_ts_prev;    // fine 3 bit  ! subject of change !

static eb_data_t eb_stat_before;    // status before etherbone cycle
static eb_data_t eb_stat_after;    // status after  etherbone cycle
static eb_data_t eb_fifo_ct_brd;    // TLU FIFO fill counter before TLU read
static eb_data_t eb_fifo_ct_ard;    //TLU FIFO fill counter after TLU read

static unsigned long long ll_ts_hi;
static unsigned long long ll_ts_lo;
static unsigned long long ll_ts_fi;
static unsigned long long ll_ts;
static unsigned long long ll_x16;
static unsigned long long ll_h16;
static unsigned long long ll_m16;
static unsigned long long ll_l16;

static unsigned long long ll_timestamp;
static unsigned long long ll_actu_timestamp;
static unsigned long long ll_prev_timestamp;
static unsigned long long ll_diff_timestamp;

static INTU4 l_time_l16;
static INTU4 l_time_m16;
static INTU4 l_time_h16;
static INTU4 l_time_x16;

static long l_check_wr_err = 0;
static long l_wr_init_ct = 0;
static long l_err_wr_ct = 0;

static struct timespec nanotime = { 0, 0 };

/*****************************************************************************/

int f_user_get_virt_ptr (long *pl_loc_hwacc, long pl_rem_cam[])
{
  if (l_eb_first1 == 0)
  {
    l_eb_first1 = 1;
    
/////// JAM 4-2022 put here configuration of cscsr space
    f_vetar_init ();
    // this will switch on vetar addresses    

    f_ifc_a32_vme_mas_map_TLU ();
    p_TLU = (char*) pl_virt_vme_base_tlu;
    printm ("mapped TLU base address 0x%x to virtual address 0x%08lx\n", VETAR_REGS_ADDR, (long) p_TLU);
    
    // used in init function
    ch_select = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_CH_SELECT);
    ch_fifosize = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_CHNS_FIFOSIZE);
    fifoclear = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_FIFO_CLEAR);
    armset = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_TRIG_ARMSET);
    
    // set here pointers to mapped registers used in readout function:  
    fifo_ready = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_FIFO_READY);
    fifo_cnt = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_FIFO_CNT);
    ft_shi = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_FIFO_FTSHI);
    ft_slo = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_FIFO_FTSLO);
    ft_ssub = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_FIFO_FTSSUB);
    fifo_pop = (INTS4*) ((char*) (p_TLU) + GSI_TM_LATCH_FIFO_POP);
    
  }

}

/*****************************************************************************/

int f_user_init (unsigned char bh_crate_nr, long *pl_loc_hwacc, long *pl_rem_cam, long *pl_stat)

{
  if (l_first == 0)
  {
    l_first = 1;
    for (l_i = 0; l_i < MAX_TRIG_TYPE; l_i++)
    {
      l_tr_ct[l_i] = 0;
    }
  }
  f_wr_init ();
  return (1);
}

/*****************************************************************************/
int f_user_readout (CHARU bh_trig_typ, CHARU bh_crate_nr, INTS4 *pl_loc_hwacc, INTS4 *pl_rem_cam, INTS4 *pl_dat,
    s_veshe *ps_veshe, INTS4 *l_se_read_len, INTS4 *l_read_stat)

{
  int rc = 0;
  *l_se_read_len = 0;
  pl_dat_save = (INTU4*) pl_dat;
  l_tr_ct[0]++;            // event/trigger counter
  l_tr_ct[bh_trig_typ]++;    // individual trigger counter

  if (l_check_wr_err == 1)
  {
    printm ("reset TLU fifo of white rabbit time stamp module vetar \n");
    f_wr_reset_tlu_fifo ();
    l_wr_init_ct++;
    *l_read_stat = 0;
    sleep (1);
    goto bad_event;
  }

  // think about if and where you shall do this ....
  *l_read_stat = 0;
#ifdef USER_TRIG_CLEAR
  if (bh_trig_typ < 14)
  {
    *l_read_stat = TRIG__CLEARED;
    f_user_trig_clear (bh_trig_typ);
  }
#endif // USER_TRIG_CLEAR

  if (bh_trig_typ < 14)
  {
    *pl_dat++ = SUB_SYSTEM_ID;
    *l_se_read_len = +4;
    
    eb_stat_before = *fifo_ready;
    eb_fifo_ct_brd = *fifo_cnt;
    *fifo_pop = 0xF;
    nanosleep (0);    // 31 kHz -> implizit sched_yield() ?
    eb_tlu_high_ts = *ft_shi;
    eb_tlu_low_ts = *ft_slo;
    eb_tlu_fine_ts = *ft_ssub;
    //eb_tlu_fine_ts =0; //  not implemented on vme?
    //printm ("timestamps -  hi:0x%x lo:0x%x fine:0x%x\n", eb_tlu_high_ts,  eb_tlu_low_ts, eb_tlu_fine_ts);
    eb_stat_after = *fifo_ready;

    // think about if and where you shall do this ....
    *l_read_stat = 0;
#ifdef USER_TRIG_CLEAR_2
    if (bh_trig_typ < 14)
    {
      *l_read_stat = TRIG__CLEARED;
      f_user_trig_clear (bh_trig_typ);
    }
#endif // USER_TRIG_CLEAR

    if ((eb_stat_before & l_used_tlu_fifo) != l_used_tlu_fifo)
    {
      usleep (100000);
      printm (RON"ERROR>>"RES" TLU fifo %d is empty before time stamp read, stat: 0x%x\n", WR_TLU_FIFO_NR,
          eb_stat_before);
      l_err_wr_ct++;
      l_check_wr_err = 2;
      goto bad_event;
    }
    if ((eb_stat_after & l_used_tlu_fifo) != 0)
    {
      //printm (RON"ERROR>>"RES" TLU fifo %d is not empty after time stamp read, stat: 0x%x\n", WR_TLU_FIFO_NR, eb_stat_after);
    }

#ifdef USER_TRIG_CLEAR
    if (eb_fifo_ct_brd > 2)
    {
      printm (RON"ERROR>>"RES" TLU fill count: %d is bigger than 2\n", eb_fifo_ct_brd);
      l_err_wr_ct++;
      l_check_wr_err = 2; goto bad_event;
    }
#else
    if (eb_fifo_ct_brd > 1)
    {
      printm (RON"ERROR>>"RES" TLU fill count: %d is bigger than 1\n", eb_fifo_ct_brd);
      l_err_wr_ct++;
      l_check_wr_err = 2;
      goto bad_event;
    }
#endif // USER_TRIG_CLEAR

    // eb_tlu_low_ts   represents 8 ns in the least significant bit (125 mhz)
    // eb_tlu_fine_ts  represents 1 ns in the least significant bit (subject of change)
    // if 1 ns granualrity is required for time sorting USE_TLU_FINE_TIME must be defined 

    ll_ts_hi = (unsigned long long) (eb_tlu_high_ts & 0xFFFFFFFF);
    ll_ts_lo = (unsigned long long) (eb_tlu_low_ts & 0xFFFFFFFF);    //  JAM4-2022 -avoid leading 1 bits when reading from mapped tlu
    ll_ts_fi = (unsigned long long) (eb_tlu_fine_ts & 0xFFFFFFFF);
    //ll_ts_fi = 0;

    ll_ts_hi = ll_ts_hi << 35;
    ll_ts_lo = ll_ts_lo << 3;
    ll_ts_fi = ll_ts_fi & 0x7;
    ll_ts = ll_ts_hi + ll_ts_lo + ll_ts_fi;
    ll_timestamp = ll_ts;

    ll_l16 = (ll_ts >> 0) & 0xffff;
    ll_m16 = (ll_ts >> 16) & 0xffff;
    ll_h16 = (ll_ts >> 32) & 0xffff;
    ll_x16 = (ll_ts >> 48) & 0xffff;

    l_time_l16 = (unsigned long) (ll_l16);
    l_time_m16 = (unsigned long) (ll_m16);
    l_time_h16 = (unsigned long) (ll_h16);
    l_time_x16 = (unsigned long) (ll_x16);

    l_time_l16 = (l_time_l16 & 0xffff) + (TS__ID_L16 << 16);
    l_time_m16 = (l_time_m16 & 0xffff) + (TS__ID_M16 << 16);
    l_time_h16 = (l_time_h16 & 0xffff) + (TS__ID_H16 << 16);
    l_time_x16 = (l_time_x16 & 0xffff) + (TS__ID_X16 << 16);

    *pl_dat++ = l_time_l16;
    *pl_dat++ = l_time_m16;
    *pl_dat++ = l_time_h16;
    *pl_dat++ = l_time_x16;
    *l_se_read_len += 16;

    ll_actu_timestamp = ll_timestamp;
    ll_diff_timestamp = ll_actu_timestamp - ll_prev_timestamp;
    if (ll_prev_timestamp > ll_actu_timestamp)
    {
      printf ("ERROR>> actual time stamp earlier than previous one \n");
      printf ("        actual   ts: 0x%llx \n", ll_actu_timestamp);
      printf ("        previous ts: 0x%llx \n", ll_prev_timestamp);
      printf ("        - sizeof(eb_data_t)=%d \n", sizeof(eb_data_t));
      printf ("        - sizeof(unsigned long long)=%d \n", sizeof(unsigned long long));
      printf ("        actual   timestamps -  hi:0x%llx lo:0x%llx fine:0x%llx\n", eb_tlu_high_ts, eb_tlu_low_ts,
          eb_tlu_fine_ts);
      printf ("        previous timestamps -  hi:0x%llx lo:0x%llx fine:0x%llx\n", eb_tlu_high_ts_prev,
          eb_tlu_low_ts_prev, eb_tlu_fine_ts_prev);
    }

    ll_prev_timestamp = ll_actu_timestamp;
    eb_tlu_high_ts_prev = eb_tlu_high_ts;
    eb_tlu_low_ts_prev = eb_tlu_low_ts;
    eb_tlu_fine_ts_prev = eb_tlu_fine_ts;

  }

  if ((l_tr_ct[0] % STATISTIC) == 0)
  {
    printm ("----------------------------------------------------\n");
    printm ("nr of triggers processed: %u \n", l_tr_ct[0]);
    printm ("\n");
    for (l_i = 1; l_i < MAX_TRIG_TYPE; l_i++)
    {
      if (l_tr_ct[l_i] != 0)
      {
        printm ("trigger type %2u found %10u times \n", l_i, l_tr_ct[l_i]);
      }
    }
    printm ("");
    printm ("reset White Rabbit VETAR TLU fifo      %d times \n", l_wr_init_ct);
    printm ("----------------------------------------------------\n");
  }

  bad_event:

  if (l_check_wr_err == 0)
  {
    *l_se_read_len = (INTS4) ((long) pl_dat - (long) pl_dat_save);
  }
  else
  {
    if (l_check_wr_err == 2)
    {
      printm ("white rabbit failure: invalidate current trigger/event (1) (0xbad00bad)\n");
    }
    if (l_check_wr_err == 1)
    {
      printm ("white rabbit failure: invalidate current trigger/event (2) (0xbad00bad)\n");
      printm ("");
    }
    pl_dat = pl_dat_save;
    *pl_dat++ = 0xbad00bad;
    *l_se_read_len = 4;
    l_check_wr_err--;
  }
  return (1);
}

/*****************************************************************************/
void f_wr_init ()
{
  
  *ch_select = WR_TLU_FIFO_NR;
  printm ("directly selected White Rabbit TLU FIFO channel number: %3d \n", *ch_select);
  eb_fifo_size = *ch_fifosize;
  printm ("size of  White Rabbit TLU FIFO:                %3d \n", eb_fifo_size);
  *fifoclear = 0xFFFFFFFF;
  *armset = 0xFFFFFFFF;

}

/*****************************************************************************/

void f_wr_reset_tlu_fifo ()
{
  /* clear all FIFOs */
  *fifoclear = 0xFFFFFFFF;
}

/*****************************************************************************/
void f_ifc_a32_vme_mas_map_TLU ()
{
  int triva_fd = 0;
  long offset = 0, len = 0;
  int alt_fd = alt_init ();    // does not hurt if it has been called before :)
  if (alt_fd <= 0)
  {
    printm ("f_ifc_a32_vme_mas_map_TLU - Error: could not do alt_init\n");
    exit (-1);
  }
  printm ("f_ifc_a32_vme_mas_map_TLU...\n");
  alt_win.req.rem_addr = VETAR_REGS_ADDR;
  alt_win.req.loc_addr = MAP_LOC_ADDR_AUTO;    //=-1;
  alt_win.req.size = VETAR_REGS_SIZE;
  alt_win.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
  alt_win.req.mode.space = MAP_SPACE_VME;
  alt_win.req.mode.am = 0x9;
  alt_win.req.mode.swap = 0;    //MAP_SPLIT_D32; // MAP_SWAP_AUTO;
  alt_win.req.mode.flags = MAP_FLAG_FREE;
  if (alt_map_alloc (&alt_win) < 0)
  {
    printm ("f_map_ifc_a32_mas_vme: ERROR>> alt_map_alloc failed, exiting..\n");
    exit (-1);
  }
  offset = VETAR_REGS_ADDR - alt_win.req.rem_addr;
  printm ("Got althea VME window parameters: vme_addr: 0x%lx loc_addr: 0x%lx size: %x, offset:0x%lx \n",
      alt_win.req.rem_addr, alt_win.req.loc_addr, alt_win.req.size, offset);
  // map local address to user space
  offset += alt_win.req.loc_addr;    // take mapped base into acount
  len = alt_win.req.size;

  // here we use our own trigmod driver for mmap, because we have put the noncached option into it :)
  triva_fd = open (triva_driverhandle, O_RDWR);
  if (triva_fd < 0)
  {
    printm ("ERROR>> cannot open TRIGMOD device for mapping %s\n", triva_driverhandle);
    printm ("Error>> %s\n", strerror (errno));
    exit (-1);
  }

  pl_virt_vme_base_tlu = (INTU4*) mmap (NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, triva_fd, offset);
  if (pl_virt_vme_base_tlu == MAP_FAILED)
  {
    printm ("f_map_ifc_a32_mas_vme ERROR>> mmap failed, cannot get user space address. exiting... Error>> %d (%s)\n",
        errno, strerror (errno));
    exit (-1);
  }
  printm ("Mapped to user space address pl_virt_vme_base_tlu= %p \n", pl_virt_vme_base_tlu);
}

void f_ifc_a32_vme_mas_free_TLU ()
{
  // JAM 5-2021: note that this function is never called. Should be something like f_user_exit() later?
  if (pl_virt_vme_base_tlu > 0)
    munmap ((void*) pl_virt_vme_base_tlu, alt_win.req.size);
  if (alt_map_free (&alt_win) < 0)
  {
    printm ("WARNING: alt_map_free failed.\n");
  }
  close (alt_fd);
}

/*****************************************************************************/
void f_ifc_a24_vme_mas_map_CONTROL ()
{
  int triva_fd = 0;
  long offset = 0, len = 0, offsetbase = 0, offsetdiff = 0;
  int alt_fd = alt_init ();    // does not hurt if it has been called before :)
  if (alt_fd <= 0)
  {
    printm ("f_ifc_a24_vme_mas_map_CONTROL - Error: could not do alt_init\n");
    exit (-1);
  }
  printm ("f_ifc_a24_vme_mas_map_CONTROL...\n");
  alt_win_control.req.rem_addr = VETAR_CTRL_ADDR;
  alt_win_control.req.loc_addr = MAP_LOC_ADDR_AUTO;    //=-1;
  alt_win_control.req.size = VETAR_CTRL_SIZE;
  alt_win_control.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
  alt_win_control.req.mode.space = MAP_SPACE_VME;
  alt_win_control.req.mode.am = 0x39;    //VME_AM_A24 | VME_AM_DATA;
  alt_win_control.req.mode.swap = 0;    //MAP_SPLIT_D32; // MAP_SWAP_AUTO;
  alt_win_control.req.mode.flags = MAP_FLAG_FREE;
  if (alt_map_alloc (&alt_win_control) < 0)
  {
    printm ("f_ifc_a24_vme_mas_map_CONTROL ERROR>> alt_map_alloc failed, exiting..\n");
    exit (-1);
  }
  offset = VETAR_CTRL_ADDR - alt_win_control.req.rem_addr;
  printm ("Got althea VME window parameters: vme_addr: 0x%lx loc_addr: 0x%lx size: %x, offset:0x%lx \n",
      alt_win_control.req.rem_addr, alt_win_control.req.loc_addr, alt_win_control.req.size, offset);
  // map local address to user space
  offset += alt_win_control.req.loc_addr;    // take mapped base into acount
  len = alt_win_control.req.size;

  // here we use our own trigmod driver for mmap, because we have put the noncached option into it :)
  triva_fd = open (triva_driverhandle, O_RDWR);
  if (triva_fd < 0)
  {
    printm ("ERROR>> cannot open TRIGMOD device for mapping %s\n", triva_driverhandle);
    printm ("Error>> %s\n", strerror (errno));
    exit (-1);
  }

  // JAM we need to align offset to PAGE_SHIFT size 4k
  offsetdiff = (offset % PAGE_SHIFT);
  offsetbase = offset - offsetdiff;

  pl_virt_vme_base_control = (INTU4*) mmap (NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, triva_fd, offsetbase);
  if (pl_virt_vme_base_control == MAP_FAILED)
  {
    printm (
        "f_ifc_a24_vme_mas_map_CONTROL ERROR>> mmap failed, cannot get user space address at len 0x%x, offset 0x%x.. exiting... Error>> %d (%s)\n",
        len, offset, errno, strerror (errno));
    exit (-1);
  }

  printm ("Mapped to user space address pl_virt_vme_base_control= %p \n", pl_virt_vme_base_control);
  pl_virt_vme_base_control += (offsetdiff / sizeof(INTU4));
  printm ("Shift to begin of registers: pl_virt_vme_base_control= %p \n", pl_virt_vme_base_control);
}

void f_ifc_a24_vme_mas_free_CONTROL ()
{
  // JAM 5-2021: note that this function is never called. Should be something like f_user_exit() later?
  if (pl_virt_vme_base_control > 0)
    munmap ((void*) pl_virt_vme_base_control, alt_win_control.req.size);
  if (alt_map_free (&alt_win_control) < 0)
  {
    printm ("WARNING: alt_map_free failed.\n");
  }
  close (alt_fd);
}

/*****************************************************************************/
void f_ifc_a32_vme_mas_map_CRCSR ()
{
  int triva_fd = 0;
  long offset = 0, len = 0;
  int alt_fd = alt_init ();    // does not hurt if it has been called before :)
  if (alt_fd <= 0)
  {
    printm ("f_ifc_a32_vme_mas_map_CRCSR - Error: could not do alt_init\n");
    exit (-1);
  }
  printm ("f_ifc_a32_vme_mas_map_CRCSR...\n");
  alt_win_crcsr.req.rem_addr = VETAR_CRCSR_ADDR;
  alt_win_crcsr.req.loc_addr = MAP_LOC_ADDR_AUTO;    //=-1;
  alt_win_crcsr.req.size = VETAR_CRCSR_SIZE;
  alt_win_crcsr.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
  alt_win_crcsr.req.mode.space = MAP_SPACE_VME;
  alt_win_crcsr.req.mode.am = 0x2F;    //VME_AM_CRCSR;
  alt_win_crcsr.req.mode.swap = 0;    //MAP_SPLIT_D32; // MAP_SWAP_AUTO;
  alt_win_crcsr.req.mode.flags = MAP_FLAG_FREE;
  if (alt_map_alloc (&alt_win_crcsr) < 0)
  {
    printm ("f_map_ifc_a32_mas_vme: ERROR>> alt_map_alloc failed, exiting..\n");
    exit (-1);
  }
  offset = VETAR_CRCSR_ADDR - alt_win_crcsr.req.rem_addr;
  printm ("Got althea VME window parameters: vme_addr: 0x%lx loc_addr: 0x%lx size: %x, offset:0x%lx \n",
      alt_win_crcsr.req.rem_addr, alt_win_crcsr.req.loc_addr, alt_win_crcsr.req.size, offset);
  // map local address to user space
  offset += alt_win_crcsr.req.loc_addr;    // take mapped base into acount
  len = alt_win_crcsr.req.size;

  // here we use our own trigmod driver for mmap, because we have put the noncached option into it :)
  triva_fd = open (triva_driverhandle, O_RDWR);
  if (triva_fd < 0)
  {
    printm ("ERROR>> cannot open TRIGMOD device for mapping %s\n", triva_driverhandle);
    printm ("Error>> %s\n", strerror (errno));
    exit (-1);
  }

  pl_virt_vme_base_crcsr = (INTU4*) mmap (NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, triva_fd, offset);
  if (pl_virt_vme_base_crcsr == MAP_FAILED)
  {
    printm (
        "f_ifc_a32_vme_mas_map_CRCSR ERROR>> mmap failed, cannot get user space address. exiting.. Error>> %d (%s)\n",
        errno, strerror (errno));
    exit (-1);
  }
  printm ("Mapped to user space address pl_virt_vme_base_crcsr= %p \n", pl_virt_vme_base_crcsr);
}

void f_ifc_a32_vme_mas_free_CRCSR ()
{
  // JAM 5-2021: note that this function is never called. Should be something like f_user_exit() later?
  if (pl_virt_vme_base_crcsr > 0)
    munmap ((void*) pl_virt_vme_base_crcsr, alt_win_crcsr.req.size);
  if (alt_map_free (&alt_win_crcsr) < 0)
  {
    printm ("WARNING: alt_map_free failed.\n");
  }
  close (alt_fd);
}

INTS4* f_set_csr_address (int off)
{
  INTS4* address = 0;
  off -= off % 4;    // JAM22: account address boundary of crcsr
  address = (INTS4*) ((char*) (pl_virt_vme_base_crcsr) + off);
  return address;
}

int f_vetar_is_present ()
{
  int idc;
  printm ("Check if VETAR is present at config base address 0x%x\n", VETAR_CRCSR_ADDR);
  idc = (*cr_vendor_id << 16);
  idc += (*(cr_vendor_id + 1) << 8);
  idc += (*(cr_vendor_id + 2));
  if (idc == VETAR_VENDOR_ID)
  {
    printm ("Found Vetar vendor ID: 0x%08x\n", idc);
    return 1;
  }
  printm ("wrong vendor ID. 0x%08x found, 0x%08x expected\n", idc, VETAR_VENDOR_ID);
  return 0;
}

void f_vetar_init ()
{
// JAM 4-2022: put here actions which are usually done in kernel module  
  int i = 0;
  unsigned char fa[4]; /* FUN0 ADER contents */
  unsigned char am = 0;
  
  // map crcsr space here:
  f_ifc_a32_vme_mas_map_CRCSR ();
  // prepare vetar crcsr space registers
  cr_vendor_id = f_set_csr_address (VME_VENDOR_ID_OFFSET);
  cr_bit_set = f_set_csr_address (BIT_SET_REG);
  cr_bit_clear = f_set_csr_address (BIT_CLR_REG);
  cr_wb_32_64 = f_set_csr_address (WB_32_64);
  cr_fun0_ader = f_set_csr_address (FUN0ADER);
  cr_fun1_ader = f_set_csr_address (FUN1ADER);

  if (f_vetar_is_present () == 0)
  {
    printm ("f_vetar_init() - Error: could not find VETAR board!\n");
    exit (-1);
  }
  /* reset the core */
  *cr_bit_set = RESET_CORE;
  usleep (10000);
  /* disable the core */
  *cr_bit_clear = ENABLE_CORE;
  /* default to 32bit WB interface */
  *cr_wb_32_64 = WB32;

  am = 0x09;    //VME_A32_USER_DATA_SCT
  fa[0] = (VETAR_REGS_ADDR >> 24) & 0xFF;
  fa[1] = (VETAR_REGS_ADDR >> 16) & 0xFF;
  fa[2] = (VETAR_REGS_ADDR >> 8) & 0xFF;
  fa[3] = (am & 0x3F) << 2;
  /* DFSR and XAM are zero */

  for (i = 0; i < 4; i++)
  {
    *(cr_fun0_ader + i) = fa[i];    // address increment is 4 here!
  }
  am = 0x39;    //VME_A24_USER_DATA_SCT;
  fa[0] = (VETAR_CTRL_ADDR >> 24) & 0xFF;
  fa[1] = (VETAR_CTRL_ADDR >> 16) & 0xFF;
  fa[2] = (VETAR_CTRL_ADDR >> 8) & 0xFF;
  fa[3] = (am & 0x3F) << 2;
  for (i = 0; i < 4; i++)
  {
    *(cr_fun1_ader + i) = fa[i];    // address increment is 4 here!
  }
  /* enable module, hence make FUN0/FUN1 available */
  *cr_bit_set = ENABLE_CORE;
  sleep (1);

  // JAM22: TODO now map control space to configure TLU and direct access:
  f_ifc_a24_vme_mas_map_CONTROL ();
  ctrl_master_control = (INTS4*) ((char*) (pl_virt_vme_base_control) + MASTER_CTRL);
  ctrl_master_data = (INTS4*) ((char*) (pl_virt_vme_base_control) + MASTER_DATA);
  ctrl_emul_dat_wd = (INTS4*) ((char*) (pl_virt_vme_base_control) + EMUL_DAT_WD);
  ctrl_window_offset_low = (INTS4*) ((char*) (pl_virt_vme_base_control) + WINDOW_OFFSET_LOW);
  ctrl_direct_access_control = (INTS4*) ((char*) (pl_virt_vme_base_control) + DIRECT_ACCESS_CONTROL);
  
// #from vetar_wb_request// reply
// #  <- since we cannot branch on conditions much here, always acknowledge the control register JAM22;
  printm ("first value of master control: 0x%x\n", *ctrl_master_control);
  *ctrl_master_control = 1;
  usleep (50000);
  *ctrl_master_data = 0;
  *ctrl_master_control = 3;
//following is in probe after wishbone register:
  *ctrl_emul_dat_wd = 0;
  *ctrl_window_offset_low = 0;
  *ctrl_master_control = 0;
  printm ("first value of dactl: 0x%x\n", *ctrl_direct_access_control);
  printm ("set to direct TLU access at address 0x%x\n", tlu_address);
  *ctrl_direct_access_control = tlu_address;
  printm ("read back dactl: 0x%x\n", *ctrl_direct_access_control);
  usleep (100000);
}
