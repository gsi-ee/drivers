// N.Kurz, EE, GSI, 4-Jul-2014

// white rabbit vetar tlu time stamp triggered readout

//----------------------------------------------------------------------------

//#define USER_TRIG_CLEAR    1
//#define USER_TRIG_CLEAR_2  1

#define WR_TIME_STAMP      1  // white rabbit latched time stamp

#define MORE_VME_READ 1


#ifdef WR_TIME_STAMP
 #define USE_TLU_FINE_TIME 1
 
#define WR_USE_TLU_DIRECT 1  // JAM8-2017: new for direct tlu access
 
#endif

#ifdef MORE_VME_READ
#define VME_MEM_BASE  0x2000000  // TRIVA used as vme source
  #define MAX_LOOP      14  
#endif

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

#define STATISTIC     1000000
#define MAX_TRIG_TYPE      16

#define SERIALIZE_IO __asm__ volatile ("eieio")

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

 // #define WR_DEVICE_NAME "dev/ttyUSB0"  // vetar2 usb 
 // #define WR_TLU_FIFO_NR        0       // vetar2 usb ?????? 

 //#define WR_DEVICE_NAME "dev/ttyUSB0"  // exploder usb 
 //#define WR_TLU_FIFO_NR       16       // exploder usb 

 #define WR_DEVICE_NAME "dev/wbm0"     // vetar vme
 #define WR_TLU_FIFO_NR       3        // vetar vme 
#endif // WR_TIME_STAMP

#ifdef WR_TIME_STAMP
 void f_wr_init ();
 void f_wr_reset_tlu_fifo ();
#endif

static long          *pl_dat_save;
static unsigned long  l_tr_ct[MAX_TRIG_TYPE];
static int            l_i;
static long           l_first = 0;




#ifdef WR_TIME_STAMP

static FILE* dactl_handle =0;
static int tlu_address = 0x4000100;
static int tlu_direct_off = 0xFFFFFFFF;

#ifdef WR_USE_TLU_DIRECT



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



static unsigned long l_TLU_base;
static char*   p_TLU;
static INTU4 volatile *pl_virt_vme_base_tlu;
    
    
    

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
    
    l_TLU_base = 0x5000000ULL; // for slot number 5, low ADER mapping (in driver)
    //l_TLU_base = 0x50000000ULL; // for slot number 5, high ADER mapping (in driver)
    
    
    f_map_ipv_a32_elb_vme (&pl_virt_vme_base_tlu);
    p_TLU = ((char*) pl_virt_vme_base_tlu +  l_TLU_base); // for slot number 5, high mapping
    printm ("TLU - ELB VME mapping:\n");
    printm ("mapped VME space to virtual base 0x%08lx\n",(long)  pl_virt_vme_base_tlu);
    printm ("mapped TLU base address 0x%x to virtual address 0x%08lx\n",l_TLU_base, (long)  p_TLU);
    
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

#ifdef MORE_VME_READ
  if (l_first_more_vme_read == 0)
  {
    l_first_more_vme_read = 1;
#ifdef WR_USE_TLU_DIRECT
    pl_virt_vme_base = pl_virt_vme_base_tlu;
    // we have already mapped vme to virtual adresses in this case, just reuse it.
#else
    f_map_ipv_a32_elb_vme (&pl_virt_vme_base);
#endif
    pl_virt_vme_base = (INTU4 *) ((INTU4)pl_virt_vme_base + (INTU4)VME_MEM_BASE);
  } 
  printm ("pl_virt_vme_base: 0x%x \n", pl_virt_vme_base); 
#endif // MORE_VME_READ
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

int f_user_readout (unsigned char   bh_trig_typ,
                    unsigned char   bh_crate_nr,
                    register long  *pl_loc_hwacc,
                    register long  *pl_rem_cam,
                    long           *pl_dat,
                    s_veshe        *ps_veshe,
                    long           *l_se_read_len,
                    long           *l_read_stat)
{
  *l_se_read_len = 0;
  pl_dat_save = pl_dat;

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
    //sleep(10);
    eb_fifo_ct_brd= *fifo_cnt; 
    *fifo_pop=0xF;
    SERIALIZE_IO;
    eb_tlu_high_ts = *ft_shi;
    //sleep(10);
    eb_tlu_low_ts = *ft_slo;
    //sleep(10);
    eb_tlu_fine_ts = *ft_ssub; 
    //sleep(10);
    //printm ("---- Event %d DUMP:\n", l_tr_ct[0]);
    //printm ("stat before: 0x%x\n", eb_stat_before);
    //printm ("fifo count: 0x%x\n",  eb_fifo_ct_brd);
    //printm ("timestamps -  hi:0x%x lo:0x%x fine:0x%x\t", eb_tlu_high_ts,  eb_tlu_low_ts, eb_tlu_fine_ts);
 

    // TEST
    //exit(0);
   
  
    eb_stat_after=*fifo_ready;
    //sleep(3);
    //printm ("stat after: 0x%x\n", eb_stat_after);

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
    ll_ts_hi = (unsigned long long) eb_tlu_high_ts;
    ll_ts_lo = (unsigned long long) eb_tlu_low_ts;
    ll_ts_fi = (unsigned long long) eb_tlu_fine_ts;
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

    ll_prev_timestamp = ll_actu_timestamp;
  } 
  #endif // WR_TIME_STAMP  

  #ifdef MORE_VME_READ 
  l_count++;
  *pl_dat++ = l_count; *l_se_read_len += 4;

  l_n_loop += 1;  // increment by 4 bytes
  if (l_n_loop == MAX_LOOP)
  {
    l_n_loop = 1;
  }
  *pl_dat++ = l_n_loop;

  for (l_i=0; l_i<l_n_loop; l_i++)
  {
    *pl_dat++ = *pl_virt_vme_base; // triva status register
  }
  #endif // MORE_VME_READ 

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

  #ifdef MORE_VME_READ 
  *l_se_read_len = (long)pl_dat - (long)pl_dat_save;
  #endif

  return (1);
}

/*****************************************************************************/

#ifdef WR_TIME_STAMP
void f_wr_init ()  
{
  
  
  #ifdef WR_USE_TLU_DIRECT
 
  //sleep(1);
  *ch_select = WR_TLU_FIFO_NR;  
  printm ("directly selected White Rabbit TLU FIFO channel number: %3d \n", *ch_select);
  //sleep(1);
  eb_fifo_size = *ch_fifosize;
  printm ("size of  White Rabbit TLU FIFO:                %3d \n", eb_fifo_size);
  printm ("");  
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

/*****************************************************************************/

