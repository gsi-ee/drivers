// N.Kurz, EE, GSI, 4-Jul-2014
// adjusted for ifc 11-Apr-2022 JAM
// white rabbit vetar tlu time stamp triggered readout

//------------- ---------------------------------------------------------------

//#define USER_TRIG_CLEAR    1
//#define USER_TRIG_CLEAR_2  1

/* set this to use direct mapping of VETAR registers with mbs local hw access*/
#define VETAR_USE_DIRECTMAP 1

#define WR_TIME_STAMP  1  // white rabbit latched time stamp


#ifdef WR_TIME_STAMP
 #define USE_TLU_FINE_TIME 1
#define WR_USE_TLU_DIRECT 1  // JAM8-2017: new for direct tlu access
#define WR_RELEASE_ENIGMA 1  // JAM 8-2019: changed TLU addresses for ENIGMA
#endif



#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define STATISTIC     1000000
#define MAX_TRIG_TYPE      16

//#define SERIALIZE_IO __asm__ volatile ("eieio");
//#define IFC_LWSYNC   asm("lwsync");

#define VETAR_REGS_ADDR   0x50000000ULL
// for slot number 5, high ADER mapping (in driver)
#define VETAR_REGS_SIZE   0x1000000


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






#ifdef WR_TIME_STAMP
 #include <etherbone.h>
 #include <gsi_tm_latch.h> // wishbone devices
#endif // WR_TIME_STAMP 
#ifdef WR_TIME_STAMP
 #define SUB_SYSTEM_ID      0x600
 #define TS__ID_L16         0x3e1
 #define TS__ID_M16         0x4e1
 #define TS__ID_H16         0x5e1
 #define TS__ID_X16         0x6e1

 #define WR_DEVICE_NAME "dev/wbm0"     // vetar vme
 #define WR_TLU_FIFO_NR       3        // vetar vme 
#endif // WR_TIME_STAMP

#ifdef WR_TIME_STAMP
 void f_wr_init ();
 void f_wr_reset_tlu_fifo ();
#endif

//static long          *pl_dat_save;
static INTU4  *pl_dat_save;
static unsigned long  l_tr_ct[MAX_TRIG_TYPE];
static int            l_i;
static long           l_first = 0;




#ifdef WR_TIME_STAMP

static FILE* dactl_handle =0;


#ifdef WR_RELEASE_ENIGMA
// following is for enigma release:
static int   tlu_address =   0x2000100;

//2000100 also new enigma and fallout?
#else
// this is for doomsday and before:
static int tlu_address = 0x4000100;
#endif

//static int tlu_direct_off = 0xFFFFFFFF;

#ifdef WR_USE_TLU_DIRECT

/* follwoing for direct mapping of ifc mem JAM 25-03-2022:*/
 #include <sys/types.h>
 #include <sys/mman.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <stdint.h>
 #include <errno.h>
 #include <altmasioctl.h>
 #include <altulib.h>
int triva_fd;
 const char* triva_driverhandle ="/dev/trigvme";
 void f_ifc_a32_vme_mas_map_TLU ();
 void f_ifc_a32_vme_mas_free_TLU ();
 int alt_fd;
 static struct alt_ioctl_map_win alt_win;
 static INTU4 volatile   *p32_virt_vme_base;

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



static char*   p_TLU;
static INTU4*  pl_virt_vme_base_tlu;
    
    
    

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

 static  eb_data_t         eb_tlu_high_ts_prev; // high 32 bit
  static  eb_data_t        eb_tlu_low_ts_prev;  // low  32 bit
  static  eb_data_t        eb_tlu_fine_ts_prev; // fine 3 bit  ! subject of change !

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

 static INTU4  l_time_l16;
 static INTU4  l_time_m16;
 static INTU4  l_time_h16;
 static INTU4  l_time_x16;

 static long l_check_wr_err = 0;
 static long l_wr_init_ct   = 0;
 static long l_err_wr_ct    = 0;

 //static INTU4  l_check1_time_l16;
 //static INTU4  l_check2_time_l16;

 //static unsigned long long  ll_48_act_time=0;
 //static unsigned long long  ll_48_pre_time=0;
#endif // WR_TIME_STAMP

#ifdef MORE_VME_READ 
static INTU4            l_first_more_vme_read = 0;
static INTU4 volatile   *pl_virt_vme_base;
static INTU4             l_n_loop = 0;
static INTU4             l_count  = 0;
#endif




static struct timespec nanotime = {0,0}; // damit auch 11 kHz


/*****************************************************************************/

int f_user_get_virt_ptr (long  *pl_loc_hwacc, long  pl_rem_cam[])
{
#ifdef WR_TIME_STAMP
  if (l_eb_first1 == 0)
  {
    l_eb_first1 = 1;
  
  
#ifdef WR_USE_TLU_DIRECT 
  
     printm ("f_user_get_virt_ptr switches VETAR to DIRECT ACCESS TLU mode... \n");
    
    dactl_handle= fopen ("/sys/class/vetar/vetar0/dactl", "a");
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
    

    // now ifc:
#ifdef VETAR_USE_DIRECTMAP
   p_TLU=(char*) pl_loc_hwacc; // mapped by MBS framework from setup_usf with direct vme
#else
    f_ifc_a32_vme_mas_map_TLU ();
    p_TLU=(char*) pl_virt_vme_base_tlu;
#endif
    printm ("mapped TLU base address 0x%x to virtual address 0x%08lx\n",VETAR_REGS_ADDR, (long)  p_TLU);
    
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
    

  
  
  
 
  
#else  // WR_USE_TLU_DIRECT
    
    printm ("f_user_get_virt_ptr  switches VETAR to etherbone access mode... \n");
    dactl_handle= fopen ("/sys/class/vetar/vetar0/dactl", "a");
    if(dactl_handle==NULL)
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
    wrTLU     = sdbDevice.sdb_component.addr_first;
    //wrTLUFIFO = wrTLU + GSI_TM_LATCH_FIFO_OFFSET + WR_TLU_FIFO_NR * GSI_TM_LATCH_FIFO_INCR;
    #endif //WR_USE_TLU_DIRECT
  }


#endif // WR_TIME_STAMP

}

/*****************************************************************************/
 
int f_user_init (unsigned char   bh_crate_nr,
                 long           *pl_loc_hwacc,
                 long           *pl_rem_cam,
                 long           *pl_stat)

{
  if (l_first == 0)
  {
    l_first = 1;
    for (l_i=0; l_i<MAX_TRIG_TYPE; l_i++)
    {
      l_tr_ct[l_i] = 0;
    }
  }

  #ifdef WR_TIME_STAMP
  f_wr_init ();
  #endif

  return (1);
}

/*****************************************************************************/
int f_user_readout (CHARU    bh_trig_typ,
                    CHARU    bh_crate_nr,
                    INTS4   *pl_loc_hwacc,
                    INTS4   *pl_rem_cam,
                    INTS4   *pl_dat,
                    s_veshe *ps_veshe,
                    INTS4   *l_se_read_len,
                    INTS4   *l_read_stat)



{
  int rc=0;
  *l_se_read_len = 0;
  pl_dat_save = (INTU4*)pl_dat;
  l_tr_ct[0]++;            // event/trigger counter
  l_tr_ct[bh_trig_typ]++;  // individual trigger counter

  #ifdef WR_TIME_STAMP
  if (l_check_wr_err == 1)
  {
    printm ("reset TLU fifo of white rabbit time stamp module vetar \n");
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
    *pl_dat++ = SUB_SYSTEM_ID;  *l_se_read_len =+ 4;

    
    #ifdef WR_USE_TLU_DIRECT
     eb_stat_before= *fifo_ready;
    eb_fifo_ct_brd= *fifo_cnt; 
    *fifo_pop=0xF;
      //nanosleep(&nanotime,0); //11 kHz mit null Zeitintervall, segv mit -1 (INVALID )
     nanosleep(0); // 31 kHz -> implizit sched_yield() ? //  mit direct mapping 38 kHz
    eb_tlu_high_ts = *ft_shi;
    eb_tlu_low_ts = *ft_slo;
    eb_tlu_fine_ts = *ft_ssub;
    //eb_tlu_fine_ts =0; //  not implemented on vme?
    //printm ("timestamps -  hi:0x%x lo:0x%x fine:0x%x\n", eb_tlu_high_ts,  eb_tlu_low_ts, eb_tlu_fine_ts);
    eb_stat_after=*fifo_ready;
#else

    
    
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

#endif //WR_USE_TLU_DIRECT

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
      printm (RON"ERROR>>"RES" TLU fifo %d is empty before time stamp read, stat: 0x%x\n", WR_TLU_FIFO_NR, eb_stat_before);
      l_err_wr_ct++;
      l_check_wr_err = 2; goto bad_event; 
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
      l_check_wr_err = 2; goto bad_event; 
    }
    #endif // USER_TRIG_CLEAR

    // eb_tlu_low_ts   represents 8 ns in the least significant bit (125 mhz)
    // eb_tlu_fine_ts  represents 1 ns in the least significant bit (subject of change)
    // if 1 ns granualrity is required for time sorting USE_TLU_FINE_TIME must be defined 

    #ifdef USE_TLU_FINE_TIME 
    ll_ts_hi = (unsigned long long) (eb_tlu_high_ts & 0xFFFFFFFF);
    ll_ts_lo = (unsigned long long) (eb_tlu_low_ts  & 0xFFFFFFFF);  //  JAM4-2022 -avoid leading 1 bits when reading from mapped tlu
    ll_ts_fi = (unsigned long long) (eb_tlu_fine_ts & 0xFFFFFFFF);
    //ll_ts_fi = 0;
     
    ll_ts_hi = ll_ts_hi << 35;
    ll_ts_lo = ll_ts_lo <<  3;
    ll_ts_fi = ll_ts_fi & 0x7;
    ll_ts    = ll_ts_hi + ll_ts_lo + ll_ts_fi;
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

    *pl_dat++ = l_time_l16;
    *pl_dat++ = l_time_m16;
    *pl_dat++ = l_time_h16;
    *pl_dat++ = l_time_x16;
    *l_se_read_len += 16;

    #else // USE_TLU_FINE_TIME

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
    *l_se_read_len += 16;

    ll_timestamp = (unsigned long long)eb_tlu_high_ts;
    ll_timestamp = (ll_timestamp << 32);
    ll_timestamp = ll_timestamp + (unsigned long long)eb_tlu_low_ts;
    #endif // USE_TLU_FINE_TIME

    ll_actu_timestamp = ll_timestamp;
    ll_diff_timestamp = ll_actu_timestamp - ll_prev_timestamp;   
    if (ll_prev_timestamp > ll_actu_timestamp)
     {
       printf ("ERROR>> actual time stamp earlier than previous one \n");
       printf ("        actual   ts: 0x%llx \n", ll_actu_timestamp);
       printf ("        previous ts: 0x%llx \n", ll_prev_timestamp);
       printf ("        - sizeof(eb_data_t)=%d \n", sizeof(eb_data_t));
       printf ("        - sizeof(unsigned long long)=%d \n", sizeof(unsigned long long));
       printf ("        actual   timestamps -  hi:0x%llx lo:0x%llx fine:0x%llx\n", eb_tlu_high_ts,  eb_tlu_low_ts, eb_tlu_fine_ts);
       printf ("        previous timestamps -  hi:0x%llx lo:0x%llx fine:0x%llx\n", eb_tlu_high_ts_prev,  eb_tlu_low_ts_prev, eb_tlu_fine_ts_prev);
      }

    ll_prev_timestamp = ll_actu_timestamp;
    eb_tlu_high_ts_prev=eb_tlu_high_ts;
    eb_tlu_low_ts_prev=eb_tlu_low_ts;
    eb_tlu_fine_ts_prev=eb_tlu_fine_ts;

  } 
  #endif // WR_TIME_STAMP  


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
    printm ("reset White Rabbit VETAR TLU fifo      %d times \n", l_wr_init_ct);
    #endif // WR_TIME_STAMP 
    printm ("----------------------------------------------------\n");  
  } 

  bad_event:

  #ifdef WR_TIME_STAMP  
  if (l_check_wr_err == 0)
  { 
    //*l_se_read_len = (long)pl_dat - (long)pl_dat_save;

    *l_se_read_len = (INTS4)((long)pl_dat - (long)pl_dat_save);

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

  #ifdef MORE_VME_READ 
  *l_se_read_len = (long)pl_dat - (long)pl_dat_save;
  #endif

  //printm("end of user reaadout sees *l_se_read_len=%d", *l_se_read_len);
  //sleep(1);
  return (1);
}

/*****************************************************************************/

#ifdef WR_TIME_STAMP
void f_wr_init ()  
{
#ifdef WR_USE_TLU_DIRECT
  
  *ch_select = WR_TLU_FIFO_NR;  
  printm ("directly selected White Rabbit TLU FIFO channel number: %3d \n", *ch_select);
  eb_fifo_size = *ch_fifosize;
  printm ("size of  White Rabbit TLU FIFO:                %3d \n", eb_fifo_size);
  *fifoclear =0xFFFFFFFF;
  *armset =0xFFFFFFFF;
#else  

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
    printm ("selected White Rabbit TLU FIFO channel number: %3d \n", eb_fifo_cha);
  }

  l_eb_first2 = 1;
  // size of TLU FIFO
  if ((eb_stat = eb_device_read (eb_device, wrTLU + GSI_TM_LATCH_CHNS_FIFOSIZE, EB_BIG_ENDIAN|EB_DATA32, &eb_fifo_size, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" when reading TLU FIFO size \n");
  }
  else
  {
    printm ("size of  White Rabbit TLU FIFO:                %3d \n", eb_fifo_size);
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
  
#endif
}
#endif // WR_TIME_STAMP

/*****************************************************************************/

#ifdef WR_TIME_STAMP
void f_wr_reset_tlu_fifo ()  
{
  /* clear all FIFOs */
 
   #ifdef WR_USE_TLU_DIRECT
  *fifoclear =0xFFFFFFFF;
#else  

  
  
  if ((eb_stat = eb_device_write(eb_device, wrTLU + GSI_TM_LATCH_FIFO_CLEAR,
                                          EB_BIG_ENDIAN|EB_DATA32, 0xFFFFFFFF, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" etherbone TLU eb_device_write (CLEAR TLU FIFOS), status: %s \n", eb_status(eb_stat));
  }
  
#endif
}
#endif // WR_TIME_STAMP

#ifdef WR_USE_TLU_DIRECT


/*****************************************************************************/
void f_ifc_a32_vme_mas_map_TLU ()
{
  long offset = 0, len = 0;
  int alt_fd = alt_init ();    // does not hurt if it has been called before :)
  if(alt_fd<=0)
       {
         printm ("f_map_ifc_a32_alt_vme - Error: could not do alt_init\n");
         exit (-1);
       }

  alt_win.req.rem_addr = VETAR_REGS_ADDR;
  alt_win.req.loc_addr = MAP_LOC_ADDR_AUTO; //=-1;
  alt_win.req.size = VETAR_REGS_SIZE;
  alt_win.req.mode.sg_id = MAP_ID_MAS_PCIE_PMEM;
  alt_win.req.mode.space = MAP_SPACE_VME;
  alt_win.req.mode.am = 0x9;
  alt_win.req.mode.swap = 0; //MAP_SPLIT_D32; // MAP_SWAP_AUTO;
  alt_win.req.mode.flags = MAP_FLAG_FREE;
  if (alt_map_alloc (&alt_win) < 0)
  {
    printm ("f_map_ifc_a32_mas_vme: ERROR>> alt_map_alloc failed, exiting..\n");
    exit (-1);
  }
  offset =  VETAR_REGS_ADDR - alt_win.req.rem_addr;
  printm ("Got althea VME window parameters: vme_addr: 0x%lx loc_addr: 0x%lx size: %x, offset:0x%lx \n",
  alt_win.req.rem_addr, alt_win.req.loc_addr, alt_win.req.size , offset);
  // map local address to user space
  offset += alt_win.req.loc_addr; // take mapped base into acount
  len = alt_win.req.size;

  // here we use our own trigmod driver for mmap, because we have put the noncached option into it :)
  triva_fd = open (triva_driverhandle, O_RDWR);
   if (triva_fd < 0)
   {
     printm ("ERROR>> cannot open TRIGMOD device for mapping %s\n", triva_driverhandle);
     printm ("Error>> %s\n", strerror (errno));
     exit (-1);
   }

   pl_virt_vme_base_tlu = (INTU4*)  mmap (NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, triva_fd, offset);
  if (pl_virt_vme_base_tlu == MAP_FAILED)
  {
    printm ("f_map_ifc_a32_mas_vme ERROR>> mmap failed, cannot get user space address. exiting..\n");
    exit (-1);
  }
  printm ("Mapped to user space address pl_virt_vme_base_tlu= %p \n", pl_virt_vme_base_tlu);
}

void f_ifc_a32_vme_mas_free_TLU ()
{
  // JAM 5-2021: note that this function is never called. Should be something like f_user_exit() later?
  if (pl_virt_vme_base_tlu > 0)
  munmap ((void*) pl_virt_vme_base_tlu, alt_win.req.size);
 if(alt_map_free(&alt_win)<0)
     {
         printm ("WARNING: alt_map_free failed.\n");
     }
  close (alt_fd);
}
#endif
