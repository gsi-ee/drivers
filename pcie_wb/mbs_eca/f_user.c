// N.Kurz, EE, GSI, 3-Feb-2010

// pexor febex triggered readout

//----------------------------------------------------------------------------
// User change area: comment with // if #defines below shall be switched off


#define USE_MBSPEX_LIB       1 // this define will switch on usage of mbspex lib with locked ioctls
                               // instead of direct register mapping usage
#define WR_TIME_STAMP        1 // white rabbit latched time stamp
//#define WRITE_ANALYSIS_PARAM 1
//#define DEBUG                1

#ifdef WR_TIME_STAMP
//#define USE_TLU_FINE_TIME   1
#define WR_USE_TLU_DIRECT   1
#define WR_USE_LOCALFIFO 1
 
 
#define WR_USE_ECA 1 // JAM 2022: use ECA fifo instead old TLU

//#define WR_ENIGMA 1 // JAM 22: enable this for old enigma firmware, we take fallout if disabled

#ifndef WR_USE_ECA
#define USE_TLU_FINE_TIME   1
#endif
 
 
#endif

#define SERIALIZE_IO __asm__ volatile ("mfence" ::: "memory")

//----------------------------------------------------------------------------

#include "stdio.h"
#include "s_veshe.h"
#include "stdarg.h"
#include <sys/file.h>
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

#include  "./pexor_gosip.h"

#ifdef USE_MBSPEX_LIB
 #include "mbspex/libmbspex.h"
#endif

#define MAX_TRIG_TYPE      16
#define STATISTIC     1000000

#ifdef WR_TIME_STAMP
#include <etherbone.h>

#ifdef WR_USE_ECA 

// JAM22 following from saftlib/drivers/eca_queue_regs.h:

#define ECA_QUEUE_POP_OWR          0x04  //wo,  1 b, Pop action from the channel's queue
#define ECA_QUEUE_FLAGS_GET        0x08  //ro,  5 b, Error flags for this action(0=late, 1=early, 2=conflict, 3=delayed, 4=valid)
#define ECA_QUEUE_DEADLINE_HI_GET  0x28  //ro, 32 b, Deadline (high word)
#define ECA_QUEUE_DEADLINE_LO_GET  0x2c  //ro, 32 b, Deadline (low word)
 // JAM22: according to  saftlib/drivers/eca_flags.h:
#define ECA_FLAGS_LATE  0x1
#define ECA_FLAGS_EARLY 0x2
#define ECA_FLAGS_CONFLICT 0x4
#define ECA_FLAGS_DELAYED  0x8
#define ECA_FLAGS_VALID   0x10
 //(0=late, 1=early, 2=conflict, 3=delayed, 4=valid)
 
#else 
 
#include <gsi_tm_latch.h> // wishbone devices
 
#endif 


 
 #include <gsi_tm_latch.h> // wishbone devices
#endif // WR_TIME_STAMP
                         //   is done parallel for all used SFPs
#ifdef WR_TIME_STAMP
 #define SUB_SYSTEM_ID      0x400
 #define TS__ID_L16         0x3e1
 #define TS__ID_M16         0x4e1
 #define TS__ID_H16         0x5e1
 #define TS__ID_X16         0x6e1

 //#define WR_DEVICE_NAME "dev/ttyUSB0"  // vetar2 usb
 //#define WR_TLU_FIFO_NR        0       // vetar2 usb

 //#define WR_DEVICE_NAME "dev/ttyUSB0"  // exploder usb
 //#define WR_TLU_FIFO_NR       16       // exploder usb

 #define WR_DEVICE_NAME "dev/wbm0"     // pexaria5 pcie
 #define WR_TLU_FIFO_NR       0        // pexaria5 pcie

 #define PEXARIA_DEVICE_NAME "/dev/pcie_wb0"     // pexaria5 direct access handle
#endif // WR_TIME_STAMP

#define USER_TRIG_CLEAR 1

#ifdef WR_TIME_STAMP
 void f_wr_init ();
 void f_wr_reset_tlu_fifo ();
#endif

#ifdef WR_TIME_STAMP
 
 static FILE* dactl_handle =0;
#ifdef WR_USE_ECA

#ifdef WR_ENIGMA
static int tlu_address = 0x40000c0; //
#else
static int tlu_address = 0;//0x4000040;
#endif

#else

#ifdef WR_ENIGMA
static int tlu_address = 0x4000100; // TLU base register to map into vme address space
#else
static int tlu_address = 0x4000200; 
#endif

#endif
 
 
 
 
 
#ifdef WR_USE_TLU_DIRECT
  static   INTS4    fd_tlu;
  static INTU4  volatile * p_TLU;

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
 #endif //WR_USE_TLU_DIRECT
 static  long              l_eb_first1=0, l_eb_first2=0;
 static  long              l_used_tlu_fifo = 1 << WR_TLU_FIFO_NR;
 static  eb_status_t       eb_stat;
 static  eb_device_t       eb_device;
 static  eb_socket_t       eb_socket;
 static  eb_cycle_t        eb_cycle;
 static  struct sdb_device sdbDevice;
 static  eb_address_t      wrTLU;
 static  eb_address_t      wrTLUFIFO;
 static  int               nDevices;

 static  eb_data_t         eb_fifo_cha;
 static  eb_data_t         eb_fifo_size;
 static  eb_data_t         eb_tlu_high_ts; // high 32 bit
 static  eb_data_t         eb_tlu_low_ts;  // low  32 bit
 static  eb_data_t         eb_tlu_fine_ts; // fine 3 bit  ! subject of change !
 static  eb_data_t         eb_stat_before; // status before etherbone cycle
 static  eb_data_t         eb_stat_after;  // status after  etherbone cycle
 static  eb_data_t         eb_fifo_ct_brd; // TLU FIFO fill counter before TLU read
 static  eb_data_t         eb_fifo_ct_ard; //TLU FIFO fill counter after TLU read


 static  unsigned long long ll_ts_hi;
 static  unsigned long long ll_ts_lo;
 static  unsigned long long ll_ts_fi;
 static  unsigned long long ll_ts;
 static  unsigned long long ll_x16;
 static  unsigned long long ll_h16;
 static  unsigned long long ll_m16;
 static  unsigned long long ll_l16;

 static  unsigned long long ll_timestamp;
 static  unsigned long long ll_actu_timestamp;
 static  unsigned long long ll_prev_timestamp;
 static  unsigned long long ll_diff_timestamp;


#ifdef WR_USE_ECA

 /* JAM 17-05-2022: software fifo to buffer the ECA values:*/
#define  TS_FIFO_SIZE 10
 static  unsigned long long ll_timestamp_fifo [TS_FIFO_SIZE];
 static INTU4 ts_fifo_front;
 static INTU4 ts_fifo_back;
 static INTU4 ts_fifo_entries;
 
 unsigned long long f_ts_fifo_pop()
 {
   unsigned long long ret=0;
  if(ts_fifo_entries==0) return 0;
  ret=ll_timestamp_fifo[ts_fifo_front++];
  if(ts_fifo_front>=TS_FIFO_SIZE) ts_fifo_front=0;
  ts_fifo_entries--;
  return ret;
}
 
 void f_ts_fifo_add(unsigned long long value)
 {
 ll_timestamp_fifo[ts_fifo_back++]= value;
 if(ts_fifo_back>=TS_FIFO_SIZE) ts_fifo_back=0;
 ts_fifo_entries++;
 if(ts_fifo_entries>  TS_FIFO_SIZE)
  {
  printm ("Warning: timestamp fifo exceeds %d entries, rolling over",ts_fifo_entries);
  ts_fifo_entries=0;   
  }
 }

 void f_ts_fifo_add_eb_data( eb_data_t* high_ts, eb_data_t* low_ts)
 {
     unsigned long long ll_timestamp = (unsigned long long) *high_ts;
     ll_timestamp = (ll_timestamp << 32);
     ll_timestamp = ll_timestamp + (unsigned long long) *low_ts;
     f_ts_fifo_add(ll_timestamp);;
 }


void f_ts_fifo_clear()
{
ts_fifo_front=0;
ts_fifo_back=0;
ts_fifo_entries=0;
}

INTU4 f_ts_fifo_entries()
{
return ts_fifo_entries;
  
}
#endif
 
 static INTU4  l_time_l16;
 static INTU4  l_time_m16;
 static INTU4  l_time_h16;
 static INTU4  l_time_x16;

 static long l_check_wr_err = 0;
 static long l_wr_init_ct   = 0;
 static long l_err_wr_ct    = 0;
#endif // WR_TIME_STAMP

static  INTU4         *pl_dat_save, *pl_tmp;
static  unsigned long  l_tr_ct[MAX_TRIG_TYPE];
static  INTU4          l_i;
static  long           l_dat;


/*****************************************************************************/

int f_user_get_virt_ptr (long  *pl_loc_hwacc, long  pl_rem_cam[])
{
  int            prot;
  int            flags;
  INTS4          l_stat;

  #ifdef WR_TIME_STAMP
  if (l_eb_first1 == 0)
  {
    l_eb_first1 = 1;

    #ifdef WR_USE_TLU_DIRECT
    printm ("f_user_get_virt_ptr switches PEXARIA to DIRECT ACCESS TLU mode... \n");

    dactl_handle= fopen ("/sys/class/pcie_wb/pcie_wb0/dactl", "a");
    if(dactl_handle==NULL)
    {
      printm ("!!! Could not open dactl control sysfs handle!!! \n");
      exit(-1); // probably wrong driver?
    }
    else
    {
      fprintf(dactl_handle,"%d",tlu_address);
      fclose(dactl_handle);
      //sleep(1);
    }
    printm ("f_user_get_virt_ptr for DIRECT TLU \n");

    if ((fd_tlu = open (PEXARIA_DEVICE_NAME, O_RDWR)) == -1)
    {
      printm (RON"ERROR>>"RES" could not open %s device \n", PEXARIA_DEVICE_NAME);
      exit (0);
    }
    else
    {
      printm ("opened device: %s, fd = %d \n", PEXARIA_DEVICE_NAME, fd_tlu);
    }

    // map bar1 directly via pcie_wb driver and access TLU registers:
    prot  = PROT_WRITE | PROT_READ;
    flags = MAP_SHARED | MAP_LOCKED;
    if ((p_TLU = (INTU4*) mmap (NULL, 0x100, prot, flags, fd_tlu, 0)) == MAP_FAILED)
    {
      printm (RON"failed to mmap bar1 from pexaria"RES", return: 0x%x, %d \n", p_TLU, p_TLU);
      perror ("mmap");
      exit (-1);
    }

    
    
    
    
    #ifdef WR_USE_ECA
   fifo_pop		= (INTS4*) ((char*)(p_TLU) 	+ ECA_QUEUE_POP_OWR);
   fifo_ready	= (INTS4*) ((char*)(p_TLU) 	+ ECA_QUEUE_FLAGS_GET);
   ft_shi 		= (INTS4*) ((char*)(p_TLU) 	+ ECA_QUEUE_DEADLINE_HI_GET);
   ft_slo 		= (INTS4*) ((char*)(p_TLU) 	+ ECA_QUEUE_DEADLINE_LO_GET);

#else
    // used in init function
    ch_select   	= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_CH_SELECT);
    ch_fifosize 	= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_CHNS_FIFOSIZE);
    fifoclear 		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_FIFO_CLEAR);
    armset 		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_TRIG_ARMSET);
    
    // set here pointers to mapped registers used in readout function:  
    fifo_ready		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_FIFO_READY);
    fifo_cnt 		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_FIFO_CNT);
    ft_shi 		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_FIFO_FTSHI);
    ft_slo 		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_FIFO_FTSLO);
    ft_ssub		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_FIFO_FTSSUB);
    fifo_pop		= (INTS4*) ((char*)(p_TLU) 	+ GSI_TM_LATCH_FIFO_POP);
    
#endif
    
  
#else  // WR_USE_TLU_DIRECT

    ////////// JAM comment the following block out to check for old pcie_wb.ko driver
    printm ("f_user_get_virt_ptr  switches PEXARIA to etherbone access mode... \n");
    dactl_handle= fopen ("/sys/class/pcie_wb/pcie_wb0/dactl", "a");
    if (dactl_handle==NULL)
    {
      printm ("!!! Could not open dactl control sysfs handle!!! \n");
      exit(-1); // probably wrong driver?
    }
    else
    {
      //fprintf(dactl_handle,"%d",tlu_direct_off);
      fprintf(dactl_handle,"4294967295"); // -1 or hex 0xffffffff
      fclose(dactl_handle);
      //sleep(1);
    }
    ///////////////////////

    if ((eb_stat = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATA32, &eb_socket)) != EB_OK)
    {
      printm (RON"ERROR>>"RES" etherbone eb_open_socket, status: %s \n", eb_status(eb_stat));
    }

    if ((eb_stat = eb_device_open(eb_socket, WR_DEVICE_NAME, EB_ADDR32|EB_DATA32, 3, &eb_device)) != EB_OK)
    {
      printm (RON"ERROR>>"RES" etherbone eb_device_open, status: %s \n", eb_status(eb_stat));
    }

    nDevices = 1;
    if ((eb_stat = eb_sdb_find_by_identity(eb_device, GSI_TM_LATCH_VENDOR,
                                                  GSI_TM_LATCH_PRODUCT, &sdbDevice, &nDevices)) != EB_OK)
    {
      printm (RON"ERROR>>"RES" etherbone TLU eb_sdb_find_by_identity, status: %s \n", eb_status(eb_stat));
    }

    if (nDevices == 0)
    {
      printm (RON"ERROR>>"RES" etherbone no TLU found, status: %s \n", eb_status(eb_stat));
    }

    if (nDevices > 1)
    {
      printm (RON"ERROR>>"RES" etherbone too many TLUsfound, status: %s \n", eb_status(eb_stat));
    }

    /* Record the address of the device */
    wrTLU = sdbDevice.sdb_component.addr_first;
    //wrTLUFIFO = wrTLU + GSI_TM_LATCH_FIFO_OFFSET + WR_TLU_FIFO_NR * GSI_TM_LATCH_FIFO_INCR;
    #endif // TLU direct

  } // ebfirst
  #endif // WR_TIME_STAMP

  return (0);
}

/*****************************************************************************/

int f_user_init (unsigned char   bh_crate_nr,
                 long           *pl_loc_hwacc,
                 long           *pl_rem_cam,
                 long           *pl_stat)

{
  #ifdef WR_TIME_STAMP
  f_wr_init ();
  #endif
  return (1);
}

/*****************************************************************************/

int f_user_readout (unsigned char   bh_trig_typ,
                    unsigned char   bh_crate_nr,
                    register long  *pl_loc_hwacc,
                    register long  *pl_rem_cam,
//                  long           *pl_dat,
                    long           *pl_dat_long,
                    s_veshe        *ps_veshe,
                    long           *l_se_read_len,
                    long           *l_read_stat)
{
  INTU4* pl_dat = (INTU4*) pl_dat_long;

  *l_se_read_len = 0;
  pl_dat_save = pl_dat;

  //printm ("\n");
  //printm ("pl_dat before readout: 0x%x \n", pl_dat);

  l_tr_ct[0]++;            // event/trigger counter
  l_tr_ct[bh_trig_typ]++;  // individual trigger counter
  //printm ("trigger no: %d \n", l_tr_ct[0]);

  #ifdef WR_TIME_STAMP
  if (l_check_wr_err == 1)
  {
    printm ("reset TLU fifo of white rabbit time stamp module pexaria \n");
    f_wr_reset_tlu_fifo ();
    l_wr_init_ct++;
    *l_read_stat = 0;
    sleep (1);
    goto bad_event;
  }
  #endif //WR_TIME_STAMP

  // think about if and where you shall do this ....
  *l_read_stat = 0;
  #ifdef USER_TRIG_CLEAR
  if (bh_trig_typ < 14)
  {
    *l_read_stat = TRIG__CLEARED;
    f_user_trig_clear (bh_trig_typ);
  }
  #endif // USER_TRIG_CLEAR

#ifdef WR_TIME_STAMP
  if (bh_trig_typ < 14)
  {
    *pl_dat++ = SUB_SYSTEM_ID;  //*l_se_read_len =+ 4;

#ifdef WR_USE_TLU_DIRECT
    
#ifdef WR_USE_ECA


#ifdef WR_USE_LOCALFIFO
    eb_fifo_ct_brd=f_ts_fifo_entries(); // take old entries into account.
    // TODO: loop over fifo pop until empty:
    eb_stat_before= (*fifo_ready) & 0x1F; // 5 bits
    eb_stat_after=eb_stat_before;
    while((eb_stat_before & ECA_FLAGS_VALID) == ECA_FLAGS_VALID)
    {
      SERIALIZE_IO;
      eb_tlu_high_ts = (*ft_shi) & 0xFFFFFFFF;
      eb_tlu_low_ts = (*ft_slo) & 0xFFFFFFFF;
      f_ts_fifo_add_eb_data(&eb_tlu_high_ts, &eb_tlu_low_ts);
      *fifo_pop=0xF;
      eb_fifo_ct_brd++;
      SERIALIZE_IO;
      eb_stat_after= (*fifo_ready) & 0x1F;
      if ((eb_stat_after & ECA_FLAGS_VALID) != ECA_FLAGS_VALID) break;
    } // while

#else
      eb_stat_before= (*fifo_ready) & 0x1F; // 5 bits
      //eb_fifo_ct_brd= (*fifo_cnt) & 0xFFFFFFFF ; // JAM22 - not implemented for ECA!
      eb_fifo_ct_brd=0;
      SERIALIZE_IO;
      eb_tlu_high_ts = (*ft_shi) & 0xFFFFFFFF;
      eb_tlu_low_ts = (*ft_slo) & 0xFFFFFFFF;
      *fifo_pop=0xF;
      SERIALIZE_IO;
      eb_stat_after= (*fifo_ready) & 0x1F;
#endif
      
  #else
  /* JAM 2019: note that on 64bit OS the ebdata_t is also 64 bit!
    we need to mask it to 32 bit when reading from mapped PCIe*/
      *pl_tmp+

      eb_stat_before= (*fifo_ready) & 0xFFFFFFFF;
      eb_fifo_ct_brd= (*fifo_cnt) & 0xFFFFFFFF ;
      *fifo_pop=0xF;
      SERIALIZE_IO;
      eb_tlu_high_ts = (*ft_shi) & 0xFFFFFFFF;
      eb_tlu_low_ts = (*ft_slo) & 0xFFFFFFFF ;
      eb_tlu_fine_ts = (*ft_ssub) & 0xFFFFFFFF ;
      eb_stat_after= (*fifo_ready) & 0xFFFFFFFF;
      //printm ("stat after: 0x%x\n", eb_stat_after);
  #endif
    
    
    
    
    
    
    

 #else // WR_USE_TLU_DIRECT

    if ((eb_stat = eb_cycle_open(eb_device, 0, eb_block, &eb_cycle)) != EB_OK)
    {
      printm (RON"ERROR>>"RES" etherbone EP eb_cycle_open, status: %s \n", eb_status(eb_stat));
      //l_err_wr_ct++;
      //l_check_wr_err = 2; goto bad_event;
    }

    /* Queueing operations to a cycle never fails (EB_OOM is reported later) */
    /* read status of FIFOs */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_READY, EB_BIG_ENDIAN|EB_DATA32, &eb_stat_before);
    /* read fifo fill counter */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_CNT, EB_BIG_ENDIAN|EB_DATA32, &eb_fifo_ct_brd);
    /* pop timestamp from FIFO */
    eb_cycle_write (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_POP, EB_BIG_ENDIAN|EB_DATA32, 0xF);
    /* read high word of latched timestamp */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSHI, EB_BIG_ENDIAN|EB_DATA32, &eb_tlu_high_ts);
    /* read low word of latched timestamp */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSLO, EB_BIG_ENDIAN|EB_DATA32, &eb_tlu_low_ts);
    /* read fine time word of latched timestamp */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSSUB, EB_BIG_ENDIAN|EB_DATA32, &eb_tlu_fine_ts);
    /* read status of FIFOs */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_READY, EB_BIG_ENDIAN|EB_DATA32, &eb_stat_after);

    /* Because the cycle was opened with eb_block, this is a blocking call.
     * Upon termination, data and eb_tlu_high_ts will be valid.
     * For higher performance, use multiple asynchronous cycles in a pipeline.
     */

    if ((eb_stat = eb_cycle_close(eb_cycle)) != EB_OK)
    {
      printm (RON"ERROR>>"RES" etherbone EP eb_cycle_close, status: %s \n", eb_status(eb_stat));
      //l_err_wr_ct++;
      //l_check_wr_err = 2; goto bad_event;
    }
    #endif // WR_USE_TLU_DIRECT
    
    
#ifdef WR_USE_ECA
  // JAM22: for ECA fifo, neeed other checks:
  // 



#ifndef WR_USE_LOCALFIFO

if ((eb_stat_before & ECA_FLAGS_VALID) != ECA_FLAGS_VALID)
    {
      usleep (100000);
      printm (RON"ERROR>>"RES" ECA fifo is empty (not valid) before time stamp read, stat: 0x%x\n", eb_stat_before);
      l_err_wr_ct++;
      l_check_wr_err = 2; goto bad_event; 
    }
    
#ifndef  USER_TRIG_CLEAR   
 if ((eb_stat_after & ECA_FLAGS_VALID) != 0)
    {
      printm (RON"ERROR>>"RES" ECA fifo is not empty after time stamp read, stat: 0x%x\n", eb_stat_after);
    }
#endif

#endif


#else

    
    
    
    
    

    if ((eb_stat_before & l_used_tlu_fifo) != l_used_tlu_fifo)
    {
      usleep (100000);
      printm (RON"ERROR>>"RES" TLU fifo %d is empty before time stamp read, stat: 0x%x\n", WR_TLU_FIFO_NR, eb_stat_before);
      l_err_wr_ct++;
      l_check_wr_err = 2; goto bad_event;
    }
    if ((eb_stat_after & l_used_tlu_fifo) != 0)
    {
      //printm (RON"ERROR>>"RES" TLU fifo %d is not empty after time stamp read, stat: 0x%x\n", WR_TLU_FIFO_NR, eb_stat_after);
    }
#endif


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
      l_check_wr_err = 2; goto bad_event;
    }
    #endif // USER_TRIG_CLEAR

    // eb_tlu_low_ts   represents 8 ns in the least significant bit (125 mhz)
    // eb_tlu_fine_ts  represents 1 ns in the least significant bit (subject of change)
    // if 1 ns granualrity is required for time sorting USE_TLU_FINE_TIME must be defined 

    #ifdef USE_TLU_FINE_TIME
    ll_ts_hi = (unsigned long long) eb_tlu_high_ts;
    ll_ts_lo = (unsigned long long) eb_tlu_low_ts;
    ll_ts_fi = (unsigned long long) eb_tlu_fine_ts;

    ll_ts_hi = ll_ts_hi << 35;
    ll_ts_lo = ll_ts_lo <<  3;
    ll_ts_fi = ll_ts_fi & 0x7;
    //ll_ts    = ll_ts_hi + ll_ts_lo + ll_ts_fi;
    ll_ts    = ll_ts_hi | ll_ts_lo | ll_ts_fi;
    ll_timestamp = ll_ts;

    ll_l16   = (ll_ts >>  0) & 0xffff;
    ll_m16   = (ll_ts >> 16) & 0xffff;
    ll_h16   = (ll_ts >> 32) & 0xffff;
    ll_x16   = (ll_ts >> 48) & 0xffff;

    l_time_l16 = (unsigned long) (ll_l16);
    l_time_m16 = (unsigned long) (ll_m16);
    l_time_h16 = (unsigned long) (ll_h16);
    l_time_x16 = (unsigned long) (ll_x16);

    l_time_l16 = (l_time_l16 & 0xffff) + (TS__ID_L16 << 16);
    l_time_m16 = (l_time_m16 & 0xffff) + (TS__ID_M16 << 16);
    l_time_h16 = (l_time_h16 & 0xffff) + (TS__ID_H16 << 16);
    l_time_x16 = (l_time_x16 & 0xffff) + (TS__ID_X16 << 16);

    *pl_dat++ = l_time_l16;*pl_tmp+
    *pl_dat++ = l_time_m16;
    *pl_dat++ = l_time_h16;
    *pl_dat++ = l_time_x16;

#else // USE_TLU_FINE_TIME

#ifdef WR_USE_LOCALFIFO
    ll_timestamp    = f_ts_fifo_pop();
    eb_tlu_low_ts   = ll_timestamp          & 0xffffffff;
    eb_tlu_high_ts  = (ll_timestamp >> 32)  & 0xffffffff;
#endif

    l_time_l16 = (eb_tlu_low_ts  >>  0) & 0xffff;
    l_time_m16 = (eb_tlu_low_ts  >> 16) & 0xffff;
    l_time_h16 = (eb_tlu_high_ts >>  0) & 0xffff;
    l_time_x16 = (eb_tlu_high_ts >> 16) & 0xffff;

    l_time_l16 = (l_time_l16 & 0xffff) + (TS__ID_L16 << 16);
    l_time_m16 = (l_time_m16 & 0xffff) + (TS__ID_M16 << 16);
    l_time_h16 = (l_time_h16 & 0xffff) + (TS__ID_H16 << 16);
    l_time_x16 = (l_time_x16 & 0xffff) + (TS__ID_X16 << 16);

    *pl_dat++ = l_time_l16;
    *pl_dat++ = l_time_m16;
    *pl_dat++ = l_time_h16;
    *pl_dat++ = l_time_x16;
    //*l_se_read_len += 16;

#ifndef WR_USE_LOCALFIFO
    ll_timestamp = (unsigned long long)eb_tlu_high_ts;
    ll_timestamp = (ll_timestamp << 32);
    ll_timestamp = ll_timestamp + (unsigned long long)eb_tlu_low_ts;
#endif

#endif // USE_TLU_FINE_TIME

    ll_actu_timestamp = ll_timestamp;
    ll_diff_timestamp = ll_actu_timestamp - ll_prev_timestamp;

    if (ll_prev_timestamp >= ll_actu_timestamp)
    {
      printf ("ERROR>> actual time stamp earlier or equal than previous one \n");
      printf ("        actual   ts: 0x%llx \n", ll_actu_timestamp);  
      printf ("        previous ts: 0x%llx \n", ll_prev_timestamp);  
     } 

    ll_prev_timestamp = ll_actu_timestamp;
  }
  #endif // WR_TIME_STAMP

  switch (bh_trig_typ)
  {
    case 1:
    case 2:
    case 3:

    if ( (l_tr_ct[0] % STATISTIC) == 0)
    {
      printm ("----------------------------------------------------\n");
      printm ("nr of triggers processed: %u \n", l_tr_ct[0]);
      printm ("\n");
      for (l_i=1; l_i<MAX_TRIG_TYPE; l_i++)
      {
        if (l_tr_ct[l_i] != 0)
        {
          printm ("trigger type %2u found %10u times \n", l_i, l_tr_ct[l_i]);
        }
      }
      #ifdef WR_TIME_STAMP
      printm ("");
      printm ("reset White Rabbit PEXARIA TLU fifo      %d times \n", l_wr_init_ct);
      #endif // WR_TIME_STAMP
      printm ("----------------------------------------------------\n");
    }

    #ifdef WR_TIME_STAMP
    // check integrity of wr time stamp

    //printm ("----------- check next event------------\n");
    //usleep (100000);

    // 5 first 32 bits must be WR time stamp
    pl_tmp = pl_dat_save;

    l_dat = *pl_tmp++;
    if (l_dat != SUB_SYSTEM_ID)
    {
      sleep (1);
      printm (RON"ERROR>>"RES" 1. data word is not sub-system id: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", SUB_SYSTEM_ID, l_dat);
      l_check_wr_err = 2; goto bad_event;
    }
    l_dat = (*pl_tmp++) >> 16;
    if (l_dat != TS__ID_L16)
    {
      sleep (1);
      printm (RON"ERROR>>"RES" 2. data word does not contain low WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_L16, l_dat);
      l_check_wr_err = 2; goto bad_event;
    }
    l_dat = (*pl_tmp++) >> 16;
    if (l_dat != TS__ID_M16)
    {
      sleep (1);
      printm (RON"ERROR>>"RES" 3. data word does not contain middle WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_M16, l_dat);
      l_check_wr_err = 2; goto bad_event;
    }
    l_dat = (*pl_tmp++) >> 16;
    if (l_dat != TS__ID_H16)
    {
      sleep (1);
      printm (RON"ERROR>>"RES" 4. data word does not contain high WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_H16, l_dat);
      l_check_wr_err = 2; goto bad_event;
    }
    l_dat = (*pl_tmp++) >> 16;
    if (l_dat != TS__ID_X16)
    {
      sleep (1);
      printm (RON"ERROR>>"RES" 5. data word does not contain 48-63 bit WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_X16, l_dat);
      l_check_wr_err = 2; goto bad_event;
    }
    #endif // WR_TIME_STAMP

bad_event:

    #ifdef WR_TIME_STAMP
    if (l_check_wr_err == 0)
    {
      *l_se_read_len = (long)pl_dat - (long)pl_dat_save;
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
    #endif // WR_TIME_STAMP
    break;

    case 15:

    break;
    default:
    break;
  }
  return (1);
}

/*****************************************************************************/


#ifdef WR_TIME_STAMP
void f_wr_init ()
{
#ifdef WR_USE_TLU_DIRECT
  
  #ifdef WR_USE_ECA
  printm ("initial eca queue flags: 0x%x \n", *fifo_ready);
  f_wr_reset_tlu_fifo();
  #else  
  //sleep(1);
  *ch_select = WR_TLU_FIFO_NR;
  printm ("directly selected White Rabbit TLU FIFO channel number: %3d \n", *ch_select);
  //sleep(1);
  eb_fifo_size = *ch_fifosize;
  printm ("size of  White Rabbit TLU FIFO:                %3d \n", eb_fifo_size);
  printm ("");
  *fifoclear =0xFFFFFFFF;
  *armset =0xFFFFFFFF;
 #endif // WR_USE_ECA

#else // WR_USE_TLU_DIRECT
  // select FIFO channel
  if ((eb_stat = eb_device_write (eb_device, wrTLU + GSI_TM_LATCH_CH_SELECT, EB_BIG_ENDIAN|EB_DATA32, WR_TLU_FIFO_NR, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" when selecting TLU FIFO channel \n");
  }

  // read back selected FIFO channel
  if ((eb_stat = eb_device_read (eb_device, wrTLU + GSI_TM_LATCH_CH_SELECT, EB_BIG_ENDIAN|EB_DATA32, &eb_fifo_cha, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" when reading selected FIFO channel number \n");
  }
  else
  {
    printm ("");
    printm ("selected White Rabbit TLU FIFO channel number: %d \n", eb_fifo_cha);
  }

  l_eb_first2 = 1;
  // size of TLU FIFO
  if ((eb_stat = eb_device_read (eb_device, wrTLU + GSI_TM_LATCH_CHNS_FIFOSIZE, EB_BIG_ENDIAN|EB_DATA32, &eb_fifo_size, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" when reading TLU FIFO size \n");
  }
  else
  {
    printm ("size of  White Rabbit TLU FIFO:                %d \n", eb_fifo_size);
    printm ("");
  }

  /* prepare the TLU for latching of timestamps */
  /* clear all FIFOs */
  if ((eb_stat = eb_device_write(eb_device, wrTLU + GSI_TM_LATCH_FIFO_CLEAR,
                                          EB_BIG_ENDIAN|EB_DATA32, 0xFFFFFFFF, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" etherbone TLU eb_device_write (CLEAR TLU FIFOS), status: %s \n", eb_status(eb_stat));
  }

  /* arm triggers for latching */
  if ((eb_stat = eb_device_write(eb_device, wrTLU + GSI_TM_LATCH_TRIG_ARMSET,
                                          EB_BIG_ENDIAN|EB_DATA32, 0xFFFFFFFF, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" etherbone TLU eb_device_write (ARM LATCHING), status: %s \n", eb_status(eb_stat));
  }
  #endif // WR_USE_TLU_DIRECT
}
#endif // WR_TIME_STAMP

/*****************************************************************************/

#ifdef WR_TIME_STAMP
void f_wr_reset_tlu_fifo ()
{
   /* clear all FIFOs */
#ifdef WR_USE_TLU_DIRECT

 #ifdef WR_USE_ECA
 // TODO- how to better clear ECA fifo here?
 int fifocounts=0, localcounts=0;
 while ((*fifo_ready & ECA_FLAGS_VALID) == ECA_FLAGS_VALID)
 {
  *fifo_pop=0xf;
  fifocounts++;
  if(fifocounts>100000) break; // get us out here if hardware is crazy...
 }
 localcounts=f_ts_fifo_entries();
 f_ts_fifo_clear();
 printm ("f_wr_reset_tlu_fifo found %d ECA fifo entries and %d local fifo count, all popped now.", fifocounts, localcounts);



 
 #else
 *fifoclear =0xFFFFFFFF;
 #endif // WR_USE_ECA
#else
  if ((eb_stat = eb_device_write(eb_device, wrTLU + GSI_TM_LATCH_FIFO_CLEAR,
                                          EB_BIG_ENDIAN|EB_DATA32, 0xFFFFFFFF, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" etherbone TLU eb_device_write (CLEAR TLU FIFOS), status: %s \n", eb_status(eb_stat));
  }
#endif
}
#endif // WR_TIME_STAMP

/*****************************************************************************/
