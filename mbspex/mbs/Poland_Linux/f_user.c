// N.Kurz, EE, GSI, 20-Mar-2013
// N.Kurz, EE, GSI, 11-Feb-2014: adopted for Linux (compiles and runs also on LynxOS)
// J.Adamczewski-Musch, EE, GSI, 02-Jun-2014: added usage of mbspex lib

/* This define will switch on usage of mbspex lib with locked ioctls instead of direct register mapping usage*/
#define USE_MBSPEX_LIB 1

/* this define switches user readout for virtual pipe
 * TODO: evaluate this directly from mbs setup structure?*/
//#define USE_VIRTUAL_PIPE 1

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

#ifdef USE_MBSPEX_LIB
#include "mbspex/libmbspex.h"

// address map for slave (exploder1): this is user specific data concering frontends , so it is not available from libmbspex
#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC
#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
#define REG_MEM_FLAG_1      0xFFFFBc  // read only:


#define REG_BUF0_DATA_LEN     0xFFFD00  // buffer 0 submemory data length
#define REG_BUF1_DATA_LEN     0xFFFE00  // buffer 1 submemory data length


#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
#define REG_MEM_FLAG_1      0xFFFFBc  // read only:

#define REG_RST 0xFFFFF4
#define REG_LED 0xFFFFF8
#define REG_VERSION 0xFFFFFC




#else
#include  "./pexor_gosip.h"
#endif
//----------------------------------------------------------------------------

// User change area:

// nr of slaves on SFP 0  1  2  3
//                     |  |  |  |
#define NR_SLAVES     {1, 1, 0, 0}

#define STATISTIC   2000000

#define DEBUG 1

#define OFFSET_TRIGGER_TYPE 14         // trigger type that is used to read out offset registers


#define WAIT_FOR_DATA_READY_TOKEN 1    // - waits until data is ready before
                                       //   sending data to PEXOR
                                       // - otherwisse send data immediately
                                       //   after token arrived at qfw/exploder  

#define SEQUENTIAL_TOKEN_SEND 1        // - token sending and receiving is
                                       //   sequential for all used SFPs
                                       // - otherwise token sending and receiving
                                       //   is done parallel for all used SFPs

//----------------------------------------------------------------------------

#ifndef USE_VIRTUAL_PIPE
// direct dma still not possible with virtual pipe in current driver

#ifdef SEQUENTIAL_TOKEN_SEND
 #define DIRECT_DMA    1 
 #ifdef DIRECT_DMA 
  #define BURST_SIZE 128
 #endif
#endif

#endif



#define PEXOR_PC_DRAM_DMA 1

#define USER_TRIG_CLEAR 1

#define CHECK_META_DATA 1

//#define printm printf

#define MAX_TRIG_TYPE     16
#define MAX_SFP           4
#define MAX_SLAVE         16
#define GET_BAR0_BASE     0x1234
#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0_MBS"
#define PCI_BAR0_SIZE     0x800000  // in bytes
#define PEX_MEM_OFF       0x100000
#define PEX_REG_OFF       0x20000
#define PEX_SFP_OFF       0x40000   
#define DATA_SEED         0x12CCE6F7
#define MAX_PAGE          10

#define REG_CTRL       0x200000

#define REG_QFW_OFFSET_BASE 0x200100

#define RON  "\x1B[7m"
#define RES  "\x1B[0m"

/*****************************************************************************/

int  f_pex_slave_rd (long, long, long, long*);
int  f_pex_slave_wr (long, long, long,  long);
int  f_pex_slave_init (long, long);
#ifndef USE_MBSPEX_LIB
int  f_pex_send_and_receive_tok (long, long, long*, long*, long*);
int  f_pex_send_tok (long, long);
int  f_pex_receive_tok (long, long*, long*, long*);
static   s_pexor  sPEXOR;
#endif
void f_qfw_init ();

static long          l_first = 0, l_first2 = 0, l_first3 = 0;       
static unsigned long l_tr_ct[MAX_TRIG_TYPE];
static   INTU4    l_sfp_pat = 0;
static   INTS4    fd_pex;             // file descriptor for PEXOR device
static   INTS4    l_sfp_slaves[MAX_SFP] = NR_SLAVES;

static   INTS4    l_bar0_base;
static   INTS4  volatile *pl_virt_bar0;

static   int   l_i, l_j, l_k;
static  long  l_stat;
static  long  l_dat1, l_dat2, l_dat3;

static  long  l_tog=1;   // start always with buffer 0 !!
static  long  l_tok_mode;
static  long  l_dummy;
static  long  l_tok_check;
static  long  l_n_slaves;
static  long  l_cha_head;
static  long  l_cha_size;
static  long  l_lec_check=-1;
static  long  l_check_err=0;
static  long long l_err_prot_ct=0;
static  long  l_qfw_init_ct=0; 
static  long  l_qfw_buf_off   [MAX_SFP][MAX_SLAVE][2];
static  long  l_qfw_n_chan    [MAX_SFP][MAX_SLAVE];
static  long  l_qfw_chan_off  [MAX_SFP][MAX_SLAVE];
static  long  l_qfw_ctrl;

static  long  l_trig_type;
static  long  l_sfp_id;
static  long  l_qfw_id;
static  long  l_cha_id;

static  INTS4 *pl_dat_save, *pl_tmp;
static  long  l_dat_len_sum[MAX_SFP];
static  long  l_dat_len_sum_long[MAX_SFP];
static  long  volatile *pl_pex_sfp_mem_base[MAX_SFP];

static  long  volatile *pl_dma_source_base;
static  long  volatile *pl_dma_target_base;
static  long  volatile *pl_dma_trans_size;
static  long  volatile *pl_dma_burst_size;
static  long  volatile *pl_dma_stat;
static  long             l_dma_target_base;
static  long            l_dma_trans_size;
static  long            l_burst_size;
static  long            l_dat;
static  long            l_pex_sfp_phys_mem_base[MAX_SFP];
static  long            l_ct;
static  long            l_padd[MAX_SFP]; 
static struct dmachain *pl_page;
static  long l_diff_pipe_phys_virt;

static  long l_err_flg;
static  long l_i_err_flg   [MAX_SFP][MAX_SLAVE];

static  long l_qfw_head_lec_err=0;
static  long l_qfw_trail_lec_err=0;
static  long l_qfw_triva_trig_type_mism=0;
static  long l_qfw_chan_data_size_1_err=0;
static  long l_qfw_chan_data_size_3_err=0;

//-new-//
//static short clk_source[2]=CLK_SOURCE_ID; 
//----//

/*****************************************************************************/

int f_user_get_virt_ptr (long  *pl_loc_hwacc, long  pl_rem_cam[])
{
  int            prot;
  int            flags;
  INTS4     l_stat;
  if (l_first2 == 0)
  {
    l_first2 = 1;

#ifdef USE_MBSPEX_LIB
    if ((fd_pex = mbspex_open (0)) == -1)
       {
         printm (RON"ERROR>>"RES" could not open mbspex device \n");
         exit (0);
       }
    for (l_i=0; l_i<MAX_SFP; l_i++)
        {
          if (l_sfp_slaves[l_i] != 0)
          {
            l_sfp_pat |= (1<<l_i);
            l_pex_sfp_phys_mem_base[l_i] = (long)PEX_MEM_OFF + (long)(PEX_SFP_OFF * l_i);
          }
        }
        printm ("sfp pattern: 0x%x \n", l_sfp_pat);
        printm ("SFP id: %d, Pexor SFP physical memory base: 0x%8x \n",
                                                       l_i, l_pex_sfp_phys_mem_base[l_i]);

  } // if (l_first2 == 0)

#else
    pl_page = (struct dmachain*) malloc (sizeof(struct dmachain*) * MAX_PAGE);

    if ((fd_pex = open (PEXDEV, O_RDWR)) == -1)
    {
      printm (RON"ERROR>>"RES" could not open %s device \n", PEXDEV);
      exit (0);
    }
    else
    {
      printm ("opened device: %s, fd = %d \n", PEXDEV, fd_pex);
    }

    #ifdef Linux
    // map bar0 directly via pexor driver and access trixor base
    prot  = PROT_WRITE | PROT_READ;
    flags = MAP_SHARED | MAP_LOCKED;
    if ((pl_virt_bar0 = (long *) mmap (NULL, PCI_BAR0_SIZE, prot, flags, fd_pex, 0)) == MAP_FAILED)
    {
      printm (RON"failed to mmap bar0 from pexor"RES", return: 0x%x, %d \n", pl_virt_bar0, pl_virt_bar0);
      perror ("mmap"); 
      exit (-1);
    } 
    #ifdef DEBUG
    printm ("first mapped virtual address of bar0: 0x%p \n", pl_virt_bar0);
    #endif // DEBUG

    #else // Linux

    // get bar0 base:
    l_stat = ioctl (fd_pex, GET_BAR0_BASE, &l_bar0_base);
    if (l_stat == -1 )
    {
      printm (RON"ERROR>>"RES" ioctl GET_BAR0_BASE failed \n");
    }
    else
    {
      printm ("PEXOR bar0 base: 0x%x \n", l_bar0_base);
    } 
    // open shared segment
    smem_remove(PCI_BAR0_NAME);
    if((pl_virt_bar0 = (long *) smem_create (PCI_BAR0_NAME,
            (char*) l_bar0_base, PCI_BAR0_SIZE, SM_READ | SM_WRITE))==NULL)
    {
      printm ("smem_create for PEXOR BAR0 failed");
      exit (-1);
    }
    #endif // Linux

    // close pexor device
    l_stat = close (fd_pex);
    if (l_stat == -1 )
    {
      printm (RON"ERROR>>"RES" could not close PEXOR device \n");
    }

    for (l_i=0; l_i<MAX_SFP; l_i++)
    {
      if (l_sfp_slaves[l_i] != 0)
      {
        pl_pex_sfp_mem_base[l_i] = (long*)
         ((long)pl_virt_bar0 + (long)PEX_MEM_OFF + (long)(PEX_SFP_OFF * l_i));   
        l_pex_sfp_phys_mem_base[l_i] = (long)PEX_MEM_OFF + (long)(PEX_SFP_OFF * l_i);

        pl_dma_source_base = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x0 );
        pl_dma_target_base = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x4 );
        pl_dma_trans_size  = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x8 );
        pl_dma_burst_size  = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0xc );
        pl_dma_stat        = (long*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x10);

        l_sfp_pat |= (1<<l_i);
      }
    }
    printm ("sfp pattern: 0x%x \n", l_sfp_pat);     
  } // if (l_first2 == 0)

  printm ("pl_virt_bar0: 0x%x \n", pl_virt_bar0); 
  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      printm ("SFP id: %d, Pexor SFP virtual memory base: 0x%8x \n", 
                                                l_i, pl_pex_sfp_mem_base[l_i]);
      printm ("                     physical:            0x%8x \n",
                                                     l_pex_sfp_phys_mem_base[l_i]);
    }
  }     

#endif


  return 0;
}

/*****************************************************************************/
 
int f_user_init (unsigned char   bh_crate_nr,
                 long           *pl_loc_hwacc,
                 long           *pl_rem_cam,
                 long           *pl_stat)

{
  #ifdef WAIT_FOR_DATA_READY_TOKEN
   l_tok_mode = 2;    // qfw / exploder wait for data ready 
  #else
   l_tok_mode = 0;    // qfw / exploder send data after token reception
  #endif
#ifndef USE_MBSPEX_LIB
  PEXOR_GetPointer(0, pl_virt_bar0, &sPEXOR); 
#endif  
  f_qfw_init();
  l_tog = 1;
  l_lec_check = -1;

  return (1);
}

/*****************************************************************************/

int f_user_readout (unsigned char   bh_trig_typ,
                    unsigned char   bh_crate_nr,
                    register long  *pl_loc_hwacc,
                    register long  *pl_rem_cam,
                    long           *pl_dat_long,
                    s_veshe        *ps_veshe,
                    long           *l_se_read_len,
                    long           *l_read_stat)
{
  INTS4* pl_dat = (INTS4*) pl_dat_long; /* JAM64: need to change pointer type since we still work with 4 byte data words in mbs event!*/

  *l_se_read_len = 0;
  pl_dat_save = pl_dat;

//printm ("\n");
//printm ("pl_dat before readout: 0x%x \n", pl_dat); 

  #ifdef CHECK_META_DATA
  if (l_check_err == 1)
  {
    printm ("");
    printm ("re-initialize all qfw modules \n");
    //l_check_err--;
    f_qfw_init ();
    l_qfw_init_ct++; 
    l_tog = 1;
    l_lec_check = -1;
    *l_read_stat = 0;               
    sleep (1);
    goto bad_event;
  }
  #endif // CHECK_META_DATA

  // think about if and where you shall do this ....
  *l_read_stat = 0;               
  #ifdef USER_TRIG_CLEAR
  if (bh_trig_typ < 14)
  {
    *l_read_stat = TRIG__CLEARED;
    f_user_trig_clear (bh_trig_typ);
  } 
  #endif // USER_TRIG_CLEAR

  switch (bh_trig_typ)
  {
    case 1:
    case 2:
    case 3:
 
    if (l_tog == 1) { l_tog = 0; } else { l_tog = 1; }

    //#ifdef  WAIT_FOR_DATA_READY_TOKEN
#if defined (WAIT_FOR_DATA_READY_TOKEN) && ! (SEQUENTIAL_TOKEN_SEND)
    //printm ("send token in WAIT_FOR_DATA_READY_TOKEN mode \n");
    //printm ("l_tog | l_tok_mode: 0x%x \n", l_tog | l_tok_mode);
    //sleep (1);

#ifdef USE_MBSPEX_LIB
      l_stat =  mbspex_send_tok (fd_pex, l_sfp_pat,  l_tog | l_tok_mode);
#else
    l_stat = f_pex_send_tok (l_sfp_pat, l_tog | l_tok_mode);
#endif
#endif

    //printm ("l_tog: %d \n", l_tog);
    l_lec_check++;
    //sleep (1);

    if (l_first3 == 0)
    {
      l_first3 = 1;
#ifndef Linux
      sleep (1);
      if ((vmtopm (getpid(), pl_page, (char*) pl_dat,
                                        (long)100 *sizeof(long))) == -1)
      {
        printm  (RON"ERROR>>"RES" calling vmtopm, exiting..\n");
        exit (0);
      }

      // get physical - and virtual pipe base
      // pipe is consecutive memory => const difference physical - virtual
      printm ("pl_dat: 0x%x, pl_dat_phys: 0x%x \n", pl_dat_save, pl_page->address);
      l_diff_pipe_phys_virt = (long)pl_page->address - (long)pl_dat;
#else
      l_diff_pipe_phys_virt = (long)pl_rem_cam;
#endif // Linux
      printm ("diff pipe base phys-virt: 0x%x \n", l_diff_pipe_phys_virt);
    }

    // prepare token data sending
    if ((bh_trig_typ != 14) && (bh_trig_typ != 15))
    {
      #ifdef SEQUENTIAL_TOKEN_SEND
      for (l_i=0; l_i<MAX_SFP; l_i++)
      {
        if (l_sfp_slaves[l_i] != 0)
        {






#ifdef DIRECT_DMA
          l_burst_size = BURST_SIZE;
          // target address is (must be) adjusted to burst size ! 
          l_padd[l_i] = 0;
          if ( ((long)pl_dat % l_burst_size) != 0)
          {
            l_padd[l_i] = l_burst_size - ((long)pl_dat % l_burst_size);  
            l_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt + l_padd[l_i];
          }
          else
          {
            l_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt;
          }

#endif //DIRECT_DMA


#ifdef USE_MBSPEX_LIB
          l_stat=mbspex_send_and_receive_tok (fd_pex, l_i, l_tog | l_tok_mode,
              (long) l_dma_target_base, (long unsigned*) &l_dma_trans_size,
              &l_dummy, &l_tok_check, &l_n_slaves);

#else
          *pl_dma_target_base =l_dma_target_base;
          // select SFP for PCI Express DMA
          *pl_dma_stat = 1 << (l_i+1);
          //printm ("depp: %d \n", 1 << (l_i+1));
          // send token to slave(s) / to SFPs
          l_stat = f_pex_send_and_receive_tok (l_i, l_tog | l_tok_mode, &l_dummy, &l_tok_check, &l_n_slaves);

#endif //USE_MBSPEX_LIB
          if (l_stat == -1)
          {
            printm (RON"ERROR>>"RES" PEXOR send token to slave(s) / SFPs failed\n");
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
          }
          if ((l_tok_check & 0x1) != l_tog)
          {
            printm (RON"ERROR>>"RES" double buffer toggle bit differs from token return toggle bit \n");
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
            //printm ("exiting..\n"); exit (0);
          }
          if ((l_tok_check & 0x2) != l_tok_mode)
          {
            printm (RON"ERROR>>"RES" token mode differs from token return token mode bit \n");
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
            //printm ("exiting..\n"); exit (0);
          }
          if (l_n_slaves != l_sfp_slaves[l_i])
          {
            printm (RON"ERROR>>"RES" nr. of slaves specified: %d differ from token return: %d \n",
                                                        l_sfp_slaves[l_i], l_n_slaves);
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
            //printm ("exiting..\n"); exit (0);   
          }

          #ifdef DIRECT_DMA


#ifndef USE_MBSPEX_LIB
          // mbspex lib does this internally, dma is finished when call mbspex_send_and_receive_tok returns
          l_ct = 0; 
          while (1)    // check if dma transfer finished 
          {
            l_dat = *pl_dma_stat;
            //            printm ("status: %d \n", l_dat); sleep (1);
            //printm ("bursts: %d \n", l_burst_size);
            if (l_dat == 0xffffffff)
            {
              printm (RON"ERROR>>"RES" PCIe bus errror, exiting.. \n");
              exit (0);
            }
            else if ((l_dat & 0x1)  == 0)
            {
              break; // dma shall be finished 
            }
            l_ct++;
            if ( (l_ct % 1000000) == 0)
            {
              printm ("DMA not ready after %d queries on SFP: %d: l_dat: %d \n", l_ct, l_i, l_dat);  
              sleep (1);
            }
            #ifndef Linux 
            yield ();
            #else
            sched_yield ();
            #endif
          }
          l_dma_trans_size = *pl_dma_trans_size; // in this case true, not BURST_SIZE aligned size
#endif // not USE_MBSPEX_LIB

          if ((l_dma_trans_size % 8) != 0)
          {
            printm ("data size: %d \n",l_dma_trans_size); 
          }

          // adjust pl_dat, pl_dat comes always 4 byte aligned
          // fill padding space with pattern
          l_padd[l_i] = l_padd[l_i] >> 2;                  // now in 4 bytes (longs) 
          for (l_k=0; l_k<l_padd[l_i]; l_k++)
          {
            //*pl_dat++ = 0xadd00000 + (l_i*0x1000) + l_k;
            *pl_dat++ = 0xadd00000 + (l_padd[l_i]<<8) + l_k;
          }
          // increment pl_dat with true transfer size (not dma transfer size)
          // true transfer size expected and must be 4 bytes aligned
          pl_dat += l_dma_trans_size>>2;
          #ifndef Linux 
          yield ();
          #else
          sched_yield ();
          #endif // Linux
#endif // DIRECT_DMA
        }
      }
      // end SEQUENTIAL_TOKEN_SEND
      #else
      // begin parallel token sending

      // send token to all SFPs used
      #ifndef WAIT_FOR_DATA_READY_TOKEN
      //printm ("send token in NOT WAIT_FOR_DATA_READY_TOKEN mode \n");
      //printm ("l_tog | l_tok_mode: 0x%x \n", l_tog | l_tok_mode);
      //sleep (1);
#ifdef USE_MBSPEX_LIB
      l_stat =  mbspex_send_tok (fd_pex, l_sfp_pat,  l_tog | l_tok_mode);
#else
      l_stat = f_pex_send_tok (l_sfp_pat, l_tog | l_tok_mode);
#endif
      #endif

      for (l_i=0; l_i<MAX_SFP; l_i++)
      {
        if (l_sfp_slaves[l_i] != 0)
        {
          // wait until token of all used SFPs returned successfully

#ifdef USE_MBSPEX_LIB
          l_dma_target_base=0; // disable automatic internal dma, we do it manually with burst adjustment later!
          l_stat =   mbspex_receive_tok (fd_pex, l_i, l_dma_target_base, (long unsigned*) &l_dma_trans_size,
              &l_dummy, &l_tok_check, &l_n_slaves);

#else

          l_stat = f_pex_receive_tok (l_i, &l_dummy, &l_tok_check, &l_n_slaves);
#endif
          if (l_stat == -1)
          {
            printm (RON"ERROR>>"RES" PEXOR receive token from SFP %d failed\n", l_i);
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
          }
          if ((l_tok_check & 0x1) != l_tog)
          {
            printm (RON"ERROR>>"RES" double buffer toggle bit differs from token return toggle bit \n");
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
            //printm ("exiting..\n"); exit (0);
          }
          if ((l_tok_check & 0x2) != l_tok_mode)
          {
            printm (RON"ERROR>>"RES" token mode 0x%x bit differs from token return token mode 0x%x bit  \n",l_tok_mode,l_tok_check);
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
            //printm ("exiting..\n"); exit (0);
          }
          if (l_n_slaves != l_sfp_slaves[l_i])
          {
            printm (RON"ERROR>>"RES" nr. of slaves specified: %d differ from token return: %d \n",
                                                        l_sfp_slaves[l_i], l_n_slaves);
            l_err_prot_ct++;
            l_check_err = 2; goto bad_event; 
            //printm ("exiting..\n"); exit (0);   
          }



        } //if (l_sfp_slaves[l_i] != 0)
      }// for
      #endif // else SEQUENTIAL_TOKEN_SEND := parallel token send






      #ifndef DIRECT_DMA
      // read exploder/qfw data (sent by token mode to the pexor)
      // from pexor the pexor memory 
      for (l_i=0; l_i<MAX_SFP; l_i++)
      {
        if (l_sfp_slaves[l_i] != 0)
        {

#ifdef USE_MBSPEX_LIB
          l_dat_len_sum[l_i] =  mbspex_get_tok_memsize(fd_pex, l_i); // in bytes
#else
          l_dat_len_sum[l_i] = PEXOR_TK_Mem_Size (&sPEXOR, l_i); // in bytes

#endif
          l_dat_len_sum[l_i] += 4; // wg. shizu !!??

          //printm (">>> token data len sum(sfp_%d)=%d bytes\n",l_i, l_dat_len_sum[l_i]);


          #ifdef PEXOR_PC_DRAM_DMA

          // choose burst size to accept max. 20% padding size
                   if      (l_dat_len_sum[l_i] < 0xa0 ) { l_burst_size = 0x10; }
                   else if (l_dat_len_sum[l_i] < 0x140) { l_burst_size = 0x20; }
                   else if (l_dat_len_sum[l_i] < 0x280) { l_burst_size = 0x40; }
                   else                                 { l_burst_size = 0x80; }




#ifdef USE_MBSPEX_LIB



           // transfer size must be adjusted to burst size
                   if ( (l_dat_len_sum[l_i] % l_burst_size) != 0)
                   {
                     l_dma_trans_size    =  l_dat_len_sum[l_i] + l_burst_size     // in bytes
                                         - (l_dat_len_sum[l_i] % l_burst_size);
                   }
                   else
                   {
                     l_dma_trans_size = l_dat_len_sum[l_i];
                   }

                   l_padd[l_i] = 0;
                             if ( ((long)pl_dat % l_burst_size) != 0)
                             {
                               l_padd[l_i] = l_burst_size - ((long)pl_dat % l_burst_size);
#ifdef USE_VIRTUAL_PIPE
                                 l_dma_target_base = (long) pl_dat + l_padd[l_i];
#else
                                 l_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt + l_padd[l_i];
#endif



                             }
                             else
                             {
#ifdef USE_VIRTUAL_PIPE
                               l_dma_target_base = (long) pl_dat;
#else
                               l_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt;
#endif


                             }

#ifdef USE_VIRTUAL_PIPE
                             l_stat =mbspex_dma_rd_virt (fd_pex, l_pex_sfp_phys_mem_base[l_i], l_dma_target_base, l_dma_trans_size,l_burst_size);
                             if(l_stat<0)
                               printm("!!!!!!! error dma to virtual pipe!\n");
                             else if (l_stat !=l_dma_trans_size)
                               {
                                 printm("!!!!!! dma to virtual pipe length mismatch: transferred:0x%x, requested:0x%x\n",l_stat,l_dma_trans_size);
                               }
                             else
                               {
                                 //printm("!!!!!!! dma to virtual pipe did transfer 0x%x bytes. \n",l_dma_trans_size);
                               }

#else
          mbspex_dma_rd (fd_pex, l_pex_sfp_phys_mem_base[l_i], l_dma_target_base, l_dma_trans_size,l_burst_size);
          /* note: return value is true dma transfer size, we do not use this here*/
#endif



#else


          // setup DMA
          *pl_dma_burst_size  = l_burst_size;                          // in bytes
          //printm (">>> burst size=%d bytes\n",l_burst_size);
          // transfer size must be adjusted to burst size
          if ( (l_dat_len_sum[l_i] % l_burst_size) != 0)
          {  
            l_dma_trans_size    =  l_dat_len_sum[l_i] + l_burst_size     // in bytes
                                - (l_dat_len_sum[l_i] % l_burst_size);
          }
          else
          {
            l_dma_trans_size = l_dat_len_sum[l_i];
          }      
          //printm (">>> DMA transfer size=%d bytes\n",l_dma_trans_size);

          *pl_dma_trans_size  =  l_dma_trans_size;   

          // source address is (must be) adjusted to burst size ! 
          *pl_dma_source_base = l_pex_sfp_phys_mem_base[l_i];

          l_padd[l_i] = 0;
          if ( ((long)pl_dat % l_burst_size) != 0)
          {
            l_padd[l_i] = l_burst_size - ((long)pl_dat % l_burst_size);  
            *pl_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt + l_padd[l_i];
          }
          else
          {
            *pl_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt;
          }
//          printm (">>> Padding length (sfp_%d)=%d bytes\n",l_i,l_padd[l_i]);
//          printm (">>> DMA target address =  0x%x bus (0x%x virt.)\n",*pl_dma_target_base, *pl_dma_target_base - l_diff_pipe_phys_virt);

          // do dma transfer pexor memory -> pc dram (sub-event pipe)
          *pl_dma_stat = 1;    // start dma
          l_ct = 0; 
          while (1)    // check if dma transfer finished 
          {
            l_dat = *pl_dma_stat;
            if (l_dat == 0xffffffff)
            {
              printm (RON"ERROR>>"RES" PCIe bus errror, exiting.. \n");
              exit (0);
            }
            else if (l_dat == 0)
            {
              break; // dma shall be finished 
            }
            l_ct++;
            if ( (l_ct % 1000000) == 0)
            {
              printm ("DMA not ready after %d queries on SFP: %d: l_dat: %d \n", l_ct, l_i, l_dat);  
              sleep (1);
            }
            #ifndef Linux 
            yield ();
            #else
            sched_yield ();
            #endif
          }
#endif // not mbspex lib

          // adjust pl_dat, pl_dat comes always 4 byte aligned
          // fill padding space with pattern
          l_padd[l_i] = l_padd[l_i] >> 2;                  // now in 4 bytes (longs) 
          for (l_k=0; l_k<l_padd[l_i]; l_k++)
          {
             //printm (">>> Fill padding pattern at 0x%x with 0x%x ,l_k=%d times\n",pl_dat, l_padd[l_i],l_k);
            //*pl_dat++ = 0xadd00000 + (l_i*0x1000) + l_k;
            *pl_dat++ = 0xadd00000 + (l_padd[l_i]<<8) + l_k;

          }
          // increment pl_dat with true transfer size (not dma transfer size)
          // true transfer size expected and must be 4 bytes aligned
          l_dat_len_sum_long[l_i] = (l_dat_len_sum[l_i] >> 2);
          pl_dat += l_dat_len_sum_long[l_i];
          //printm (">>> Incremented pl_dat with 0x%x words to 0x%x \n",l_dat_len_sum_long[l_i], pl_dat);

          #else // PEXOR_PC_DRAM_DMA 

          //l_dat_len_sum_long[l_i] = (l_dat_len_sum[l_i] >> 2) + 1;  // in 4 bytes
          l_dat_len_sum_long[l_i] = (l_dat_len_sum[l_i] >> 2);  // in 4 bytes

#ifdef USE_MBSPEX_LIB

          for (l_k=0; l_k<l_dat_len_sum_long[l_i]; l_k++)
                   {
                     l_rd_ct++;
                     mbspex_register_rd (fd_pex, 0, PEX_MEM_OFF + (long)(PEX_SFP_OFF * l_i), pl_dat++);
                   }
#else
          pl_tmp = pl_pex_sfp_mem_base[l_i];
          for (l_k=0; l_k<l_dat_len_sum_long[l_i]; l_k++)
          {
            l_rd_ct++;
            *pl_dat++ = *pl_tmp++;
          }
#endif
          #endif // PEXOR_PC_DRAM_DMA 
        }
      }
      #endif // not DIRECT_DMA
    }

    l_tr_ct[0]++;            // event/trigger counter
    l_tr_ct[bh_trig_typ]++;  // individual trigger counter
    //printm ("trigger no: %d \n", l_tr_ct[0]);


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
      
      printm ("QFW - TRIVA trigger type mismatches: %d \n", l_qfw_triva_trig_type_mism);
      //printm ("data size errors (trig. type 1):     %d \n", l_qfw_chan_data_size_1_err);
      //printm ("data size errors (trig. type 3):     %d \n", l_qfw_chan_data_size_3_err);
      //printm ("qfw header  lec mismatches           %d \n", l_qfw_head_lec_err);
      //printm ("qfw trailer lec mismatches           %d \n", l_qfw_trail_lec_err);
      printm ("");
      printm ("re-initialized QFW modules %d times \n", l_qfw_init_ct);
      printm ("gosip error count: %lld", l_err_prot_ct); 
      printm ("----------------------------------------------------\n");  
    } 

    #ifdef CHECK_META_DATA
    //printm ("----------- check next event------------\n");
    //usleep (1);

    pl_tmp = pl_dat_save;
    while (pl_tmp < pl_dat)
    {
      //sleep (1);
    //printm ("             while start \n");
      l_dat = *pl_tmp++;   // must be padding word or channel header
      //printm ("l_dat 0x%x \n", l_dat);
      if ( (l_dat & 0xfff00000) == 0xadd00000 ) // begin of padding 4 byte words
      {
       // printm ("padding found \n");
        l_dat = (l_dat & 0xff00) >> 8;
        pl_tmp += l_dat - 1;  // increment by pointer by nr. of padding  4byte words 
      }
      else if ( (l_dat & 0xff) == 0x34) //channel header
      {
        l_cha_head = l_dat;
        //printm ("l_cha_head: 0x%x \n", l_cha_head);

        l_trig_type = (l_cha_head & 0xf00)      >>  8;
        l_sfp_id    = (l_cha_head & 0xf000)     >> 12;
        l_qfw_id    = (l_cha_head & 0xff0000)   >> 16;
        l_cha_id    = (l_cha_head & 0xff000000) >> 24;

        l_cha_size = *pl_tmp++;

        if ( ((l_cha_head & 0xff) >> 0) != 0x34 )
        {
          printm (RON"ERROR>>"RES" channel header type is not 0x34 \n");
          l_err_prot_ct++;
        } 
        
        if ( l_trig_type != bh_trig_typ )
  {
          printm (RON"ERROR>>"RES" trigger type is not the same as from TRIVA \n");
          printm ("        trigger types: TRIVA: %d, OFW_LU1: %d \n",
                           bh_trig_typ, l_trig_type);
    l_err_prot_ct++;
          l_qfw_triva_trig_type_mism++;
          l_check_err = 2; goto bad_event; 
  }

        //printm ("chan size: 0x%d \n", l_cha_size);

        if(l_cha_size==0)
        {
          printm (RON"ERROR>>"RES" found optic length 0 for sfp:%d poland:%d - ",l_sfp_id, l_qfw_id);
          // todo: write to debug register here!
#ifdef USE_MBSPEX_LIB
      mbspex_slave_wr(fd_pex,l_sfp_id,l_qfw_id,0x2000d8,1);
      printm (" writing debug bit to address 0x2000d8 \n");
#else
      printm (" please enable mbspex lib to send debug bit !!!\n");
#endif
          // probably mark as bad event already here goto bad_event;
        }

        pl_tmp += l_cha_size>>2; // jump to next exploder 
        // instead of jump checks or payload data sahll be done here !! nik 16-Jan-2014 
      }
      else
      {
        printm (RON"ERROR>>"RES" data word neither header nor padding word: \n");
        //printm ("bad event: 0x%x,  0x%x,  0x%x,  0x%x,  0x%x \n",
  //*pl_dat_save, *(pl_dat_save+1), *(pl_dat_save+2), *(pl_dat_save+3), *(pl_dat_save+4));
      }       
    }

    #endif // CHECK_META_DATA 

bad_event:

    if (l_check_err == 0)
    { 
      *l_se_read_len = (long)pl_dat - (long)pl_dat_save;
    }
    else
    {
      printm ("invalidate current trigger/event  (0xbad00bad)\n");
      pl_dat = pl_dat_save;
      *pl_dat++ = 0xbad00bad;
      *l_se_read_len = 4; 
      l_check_err--;
    }  
    break;

    case OFFSET_TRIGGER_TYPE:
      // for the moment we use start acq trigger 14 here

      for (l_i = 0; l_i < MAX_SFP; l_i++)
      {
        // read registers to subevent:
        for (l_j = 0; l_j < l_sfp_slaves[l_i]; l_j++)
        {
          // put header here to indicate sfp and slave:
             printm ("oooooooo Offest readout for spf:%d slave:%d\n", l_i, l_j);
             l_cha_head=0x42 + (l_i << 12) + (l_j << 16) + (bh_trig_typ << 8);
              printm ("oooooooo Use data header :0x%x \n", l_cha_head);
            *pl_dat++= l_cha_head;
          for (l_k = 0; l_k < 32; l_k++)
          {
            l_stat = f_pex_slave_rd (l_i, l_j, REG_QFW_OFFSET_BASE + 4*l_k, (long*) pl_dat++);
            printm ("oooooooo Read offset %d: 0x%x \n", l_k, *(pl_dat - 1));
          }
        }
      }
      *l_se_read_len = (long)pl_dat - (long)pl_dat_save;

      break;






    case 15:
    l_tog = 1;
    l_lec_check = -1;
    break;
    default:
        printm ("TTTTTTTTTTT Found unhandled Trigger Type :%d \n", bh_trig_typ);
      
    break;
  }
  return (1);
}

/*****************************************************************************/


int f_pex_slave_init (long l_sfp, long l_n_slaves)
{
#ifdef USE_MBSPEX_LIB

  return mbspex_slave_init (fd_pex, l_sfp, l_n_slaves);

#else
  int  l_ret;
  long l_comm;

  printm ("initialize SFP chain %d ", l_sfp);
  l_comm = PEXOR_INI_REQ | (0x1<<16+l_sfp);

  PEXOR_RX_Clear_Ch (&sPEXOR, l_sfp); 
  PEXOR_TX (&sPEXOR, l_comm, 0, l_n_slaves  - 1) ;
  for (l_j=1; l_j<=10; l_j++)
  {
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
    printm (RON"ERROR>>"RES" initialization of SFP chain %d failed. ", l_sfp);
    printm ("no reply: 0x%x 0x%x 0x%x \n", l_dat1, l_dat2, l_dat3);
    //printm ("exiting.. \n"); exit (0);
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
      printm (RON"ERROR>>"RES" initialization of SFP chain %d failed. ", l_sfp);
      printm ("no slaves found \n"); 
      //printm ("exiting.. \n"); exit (0);
    }
  }
 return (l_ret);

#endif // not MBSPEX LIB
}

/*****************************************************************************/

int f_pex_slave_wr (long l_sfp, long l_slave, long l_slave_off, long l_dat)
{
#ifdef USE_MBSPEX_LIB

  return mbspex_slave_wr (fd_pex, l_sfp, l_slave, l_slave_off, l_dat);

#else


  int  l_ret;
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
    l_err_flg++;
    l_i_err_flg[l_sfp][l_slave]++;
    #ifdef DEBUG
    printm (RON"ERROR>>"RES" writing to SFP: %d, slave id: %d, addr 0x%d \n",
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
        l_err_flg++;
        l_i_err_flg[l_sfp][l_slave]++;
        #ifdef DEBUG
        printm (RON"ERROR>>"RES" packet structure: command reply 0x%x \n", l_dat1);
        #endif // DEBUG
      }
    }
    else
    {
      l_ret = -1;
      l_err_flg++;
      l_i_err_flg[l_sfp][l_slave]++;
      #ifdef DEBUG
      printm (RON"ERROR>>"RES" writing to empty slave or wrong address: \n");
      printm ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
           l_sfp, l_slave, (l_addr & 0xf00000) >> 24 , l_addr & 0xfffff, l_dat1);
      #endif // DEBUG
    }
  }
  return (l_ret);
#endif
}

/*****************************************************************************/

int f_pex_slave_rd (long l_sfp, long l_slave, long l_slave_off, long *l_dat)
{
#ifdef USE_MBSPEX_LIB

  return mbspex_slave_rd (fd_pex, l_sfp, l_slave, l_slave_off, l_dat);

#else

  int  l_ret;
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
    l_err_flg++;
    l_i_err_flg[l_sfp][l_slave]++;
    #ifdef DEBUG
    printm (RON"ERROR>>"RES" reading from SFP: %d, slave id: %d, addr 0x%d \n",
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
        l_err_flg++;
        l_i_err_flg[l_sfp][l_slave]++;
        #ifdef DEBUG
        printm (RON"ERROR>>"RES" packet structure: command reply 0x%x \n", l_dat1);
        #endif //DEBUG
      }
    }
    else
    {
      l_ret = -1;
      l_err_flg++;
      l_i_err_flg[l_sfp][l_slave]++;
      #ifdef DEBUG 
      printm (RON"ERROR>>"RES" Reading from empty slave or wrong address: \n");
      printm ("  SFP: %d, slave id: %d, 0x%x addr: 0x%x,  command reply:  0x%x \n",
              l_sfp, l_slave, (l_addr & 0xf0000) >> 24 , l_addr & 0xfffff, l_dat1);
      #endif // DEBUG
    }
  }
  return (l_ret);
#endif
}

/*****************************************************************************/

void f_qfw_init ()

{
#ifndef USE_MBSPEX_LIB
  PEXOR_Port_Monitor (&sPEXOR);
#endif  
  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      l_stat = f_pex_slave_init (l_i, l_sfp_slaves[l_i]);  
      if (l_stat == -1)
      {
        printm (RON"ERROR>>"RES" slave address initialization failed \n");
        printm ("exiting...\n"); 
        exit (-1); 
      }
    }
    printm ("");
  }

  //sleep (1); 

  if (l_first == 0)
  {
    l_first = 1;
    for (l_i=0; l_i<MAX_TRIG_TYPE; l_i++)
    {
      l_tr_ct[l_i] = 0;
    }
  }

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        // needed for check of meta data, read it in any case
        printm ("SFP: %d, OFW/EXPLODER: %d \n", l_i, l_j); 
        // get address offset of qfw buffer 0,1 for each qfw/exploder
        l_stat = f_pex_slave_rd (l_i, l_j, REG_BUF0, &(l_qfw_buf_off[l_i][l_j][0]));
        l_stat = f_pex_slave_rd (l_i, l_j, REG_BUF1, &(l_qfw_buf_off[l_i][l_j][1]));
        // get nr. of channels per qfw
        l_stat = f_pex_slave_rd (l_i, l_j, REG_SUBMEM_NUM, &(l_qfw_n_chan[l_i][l_j]));
        // get buffer per channel offset
        l_stat = f_pex_slave_rd (l_i, l_j, REG_SUBMEM_OFF, &(l_qfw_chan_off[l_i][l_j]));

        printm ("addr offset: buf0: 0x%x, buf1: 0x%x \n",
                l_qfw_buf_off[l_i][l_j][0], l_qfw_buf_off[l_i][l_j][1]);
        printm ("No. channels: %d \n", l_qfw_n_chan[l_i][l_j]);
        printm ("channel addr offset: 0x%x \n", l_qfw_chan_off[l_i][l_j]);

        // disable test data length
        l_stat = f_pex_slave_wr (l_i, l_j, REG_DATA_LEN, 0x10000000);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" disabling test data length failed\n");
          l_err_prot_ct++;
        }

        // disable trigger acceptance in exploder2a
        l_stat = f_pex_slave_wr (l_i, l_j, REG_CTRL, 0);
        if (l_stat == -1)
  {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
          l_err_prot_ct++;
        }
        l_stat = f_pex_slave_rd (l_i, l_j, REG_CTRL, &l_qfw_ctrl);
        if ( (l_qfw_ctrl & 0x1) != 0)
        {
          printm (RON"ERROR>>"RES" disabling trigger acceptance in qfw failed, exiting \n");
          l_err_prot_ct++;
          exit (0);
        }
  
        // enable trigger acceptance in exploder2a

        l_stat = f_pex_slave_wr (l_i, l_j, REG_CTRL, 1);
        if (l_stat == -1)
  {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
          l_err_prot_ct++;
        }
        l_stat = f_pex_slave_rd (l_i, l_j, REG_CTRL, &l_qfw_ctrl);
        if ( (l_qfw_ctrl & 0x1) != 1)
  {
          printm (RON"ERROR>>"RES" enabling trigger acceptance in qfw failed, exiting \n");
          l_err_prot_ct++;
          exit (0);
   }

        // write SFP id for channel header
        l_stat = f_pex_slave_wr (l_i, l_j, REG_HEADER, l_i);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_HEADER  failed\n");
          l_err_prot_ct++;
        }
        
        l_stat = f_pex_slave_wr (l_i, l_j, REG_RST, 1);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_HEADER  failed\n");
          l_err_prot_ct++;
        }
      }
    }
  }
}
/*****************************************************************************/

#ifndef USE_MBSPEX_LIB

int f_pex_send_and_receive_tok (long l_sfp, long l_toggle,
                    long *pl_check1, long *pl_check2, long *pl_check3)
{
  int  l_ret;
  long l_comm;

  l_comm = PEXOR_PT_TK_R_REQ | (0x1<<16+l_sfp);
  PEXOR_RX_Clear_Ch(&sPEXOR, l_sfp);
  PEXOR_TX (&sPEXOR, l_comm, l_toggle, 0);
  l_stat = PEXOR_RX (&sPEXOR, l_sfp, pl_check1, pl_check2, pl_check3); 
  // return values:
  // l_check1: l_comm
  // l_check2: toggle bit
  // l_check3: nr. of slaves connected to token chain  

  l_ret = 0;   
  if (l_stat == -1)
  {
    l_ret = -1;
    #ifdef DEBUG
    printm (RON"ERROR>>"RES" sending token to SFP: %d \n", l_sfp);
    printm ("  no reply: 0x%x 0x%x 0x%x \n", *pl_check1, *pl_check2, *pl_check3);
    #endif // DEBUG
  }

  return (l_ret);
}

/*****************************************************************************/

int f_pex_send_tok (long l_sfp_p, long l_toggle)
{
  // sends token to all SFPs marked in l_sfp_p pattern: 1: sfp 0, 2: sfp 1, 
  //                                                    4: sfp 2, 8: sfp 3,
  //                                                  0xf: all four SFPs

  long l_comm;

  l_comm = PEXOR_PT_TK_R_REQ | (l_sfp_p << 16);
  PEXOR_RX_Clear_Pattern(&sPEXOR, l_sfp_p);
  PEXOR_TX (&sPEXOR, l_comm, l_toggle, 0);

  return (0);
}

/*****************************************************************************/

int f_pex_receive_tok (long l_sfp, long *pl_check1, long *pl_check2, long *pl_check3)
{
  // checks token return for a single, individual SFPS
  int  l_ret;

  l_stat = PEXOR_RX (&sPEXOR, l_sfp, pl_check1, pl_check2, pl_check3); 
  // return values:
  // l_check1: l_comm
  // l_check2: toggle bit
  // l_check3: nr. of slaves connected to token chain  

  l_ret = 0;   
  if (l_stat == -1)
  {
    l_ret = -1;
    #ifdef DEBUG
    printm (RON"ERROR>>"RES" receiving token from SFP: %d event:%d \n", l_sfp, l_lec_check);
    printm ("  no reply: 0x%x 0x%x 0x%x \n", *pl_check1, *pl_check2, *pl_check3);
    #endif // DEBUG
  }

  return (l_ret);
}
#endif
/*****************************************************************************/
