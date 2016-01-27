// N.Kurz, EE, GSI, 3-Feb-2010

// pexor febex triggered readout 

//----------------------------------------------------------------------------
// User change area: comment with // if #defines below shall be switched off


#define USE_MBSPEX_LIB       1 // this define will switch on usage of mbspex lib with locked ioctls
                               // instead of direct register mapping usage
//#define WR_TIME_STAMP        1 // white rabbit latched time stamp
#define WRITE_ANALYSIS_PARAM 1 
#define DEBUG                1

#ifdef WR_TIME_STAMP
 #define USE_TLU_FINE_TIME   1
#endif

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

#ifdef WR_TIME_STAMP
 #include <etherbone.h>
 #include <gsi_tm_latch.h> // wishbone devices
#endif // WR_TIME_STAMP 

//----------------------------------------------------------------------------

// User change area:

#define MAX_SFP       4
#define MAX_SLAVE    16
#define FEBEX_CH     16 

// nr of slaves on SFP 0   1   2   3
//                     |   |   |   |
#define NR_SLAVES    { 2,  0,  0,  0} 

                              // maximum trace length 8000 (133 us)
                              // attention
                              // CVT to set: trace length - irq latency (10us)
  
//#define FEB_TRACE_LEN  1024  // in nr of samples
//#define FEB_TRIG_DELAY  300  // in nr.of samples
#define FEB_TRACE_LEN   300  // in nr of samples
#define FEB_TRIG_DELAY   30  // in nr.of samples

//#define CLK_SOURCE_ID     {0xff,0}  // sfp_port, module_id of the module to distribute clock
#define CLK_SOURCE_ID     {0x0,0}  // sfp_port, module_id of the module to distribute clock

//--------------------------------------------------------------------------------------------------------

#define DATA_FILT_CONTROL_REG 0x2080C0
#define DATA_FILT_CONTROL_DAT 0x80         // (0x80 E,t summary always +  data trace                 always
                                           // (0x82 E,t summery always + (data trace + filter trace) always
                                           // (0x84 E,t summery always +  data trace                 if > 1 hit
                                           // (0x86 E,t summery always + (data trace + filter trace) if > 1 hit
// Trigger/Hit finder filter

#define TRIG_SUM_A_REG    0x2080D0
#define TRIG_GAP_REG      0x2080E0
#define TRIG_SUM_B_REG    0x2080F0

#define TRIG_SUM_A     8  // for 12 bit: 8, 4 ,9 (8+1); for 14 bit: 14, 4, 15 (14 + 1).
#define TRIG_GAP       4
#define TRIG_SUM_B     9 // 8 + 1: one has to be added.

// Energy Filters and Modes

#define ENABLE_ENERGY_FILTER 1

#define TRAPEZ               1  // if TRAPEZ is off, MWD will be activated

#ifdef ENABLE_ENERGY_FILTER
 #ifdef TRAPEZ
  #define ENERGY_SUM_A_REG  0x208090
  #define ENERGY_GAP_REG    0x2080A0
  #define ENERGY_SUM_B_REG  0x2080B0

  #define ENERGY_SUM_A  64
  #define ENERGY_GAP    32
  #define ENERGY_SUM_B  65  // 64 + 1: one has to be added.
 #endif 

#endif

//--------------------------------------------------------------------------------------------------------
//
// bit 31            12 bit adc:  0    
//                   14 bit adc:  1  
//
// bit 28       signal polarity:  0: positive,    <-- very important info for fpga hit finder!
//                                1: negative     <-- "

// bit 24 - 27  trigger methode:  0: 3step
//                                1: 2-window 60  MHz
//                                2: 2-window 30  MHz
//                                4: 2-window 15  MHz
//                                8: 2-window 7.5 MHz

// bit 20       even-odd or       0: disabled 
//                                1: enabled              

// bit  0 - 16  disable channels: bit 0: special channel, bit 1-16: adc channels
//                                0x00000: all enabled
//                                0x1fffe: all adc channels disabled, special channel enabled
//--------------------------------------------------------------------------------------------------------
static long l_sfp0_feb_ctrl0[MAX_SLAVE] = { 0x01000000, 0x01000000, 0x01000000, 0x01000000,
                                            0x01000000, 0x01000000, 0x01000000, 0x01000000,
                                            0x01000000, 0x01000000, 0x01000000, 0x01000000,
                                            0x01000000, 0x01000000, 0x01000000, 0x01000000 };

static long l_sfp1_feb_ctrl0[MAX_SLAVE] = { 0x81000000, 0x92000000, 0x92000000, 0x92000000,
                                            0x92000000, 0x92000000, 0x92000000, 0x92000000,
                                            0x92000000, 0x92000000, 0x92000000, 0x92000000,
                                            0x92000000, 0x92000000, 0x92000000, 0x92000000 };

static long l_sfp2_feb_ctrl0[MAX_SLAVE] = { 0x81000000, 0x81000000, 0x81000000, 0x81000000,
                                            0x81000000, 0x81000000, 0x81000000, 0x81000000,
                                            0x81000000, 0x81000000, 0x01000000, 0x01000000,
                                            0x81000000, 0x81000000, 0x81000000, 0x81000000 };

static long l_sfp3_feb_ctrl0[MAX_SLAVE] = { 0x01000000, 0x01000000, 0x01000000, 0x01000000,
                                            0x01000000, 0x01000000, 0x01000000, 0x01000000,
                                            0x01000000, 0x01000000, 0x01000000, 0x01000000,
                                            0x01000000, 0x01000000, 0x01000000, 0x01000000 };
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
// bit  0 - 16  data sparsifying: bit 0: special channel, bit 1-16: adc channels
//                                0x00000: sparsifying disabled for all channles
//                                0x1fffe: sparsifying for all adc channels enabled
//                                         sparcifying for special channel  disabled
//--------------------------------------------------------------------------------------------------------
static long l_sfp0_feb_ctrl1[MAX_SLAVE] = { 0x0, 0x0, 0x0, 0x0,
                                            0x0, 0x0, 0x0, 0x0,
                                            0x0, 0x0, 0x0, 0x0,
                                            0x0, 0x0, 0x0, 0x0 };

static long l_sfp1_feb_ctrl1[MAX_SLAVE] = { 0x0,     0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff };

static long l_sfp2_feb_ctrl1[MAX_SLAVE] = { 0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff };

static long l_sfp3_feb_ctrl1[MAX_SLAVE] = { 0x0,     0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff,
                                            0x1ffff, 0x1ffff, 0x1ffff, 0x1ffff };
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
// bit  0 - 15    internal trigger enable/disable for aadc channels 0-15
//                0x0000: trigger disabled for all adc channels
//                0xffff: trigger enabled  for all adc channels
//--------------------------------------------------------------------------------------------------------
static long l_sfp0_feb_ctrl2[MAX_SLAVE] = { 0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff };

static long l_sfp1_feb_ctrl2[MAX_SLAVE] = { 0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff };

static long l_sfp2_feb_ctrl2[MAX_SLAVE] = { 0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff };

static long l_sfp3_feb_ctrl2[MAX_SLAVE] = { 0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff,
                                            0xffff, 0xffff, 0xffff, 0xffff };
//--------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------
// max: 255 (adc counts)
static long l_sfp0_thresh[MAX_SLAVE][FEBEX_CH] = {
// channel               0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15
      /* FEBEX  0  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  1  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  2  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  3  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  4  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  5  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  6  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  7  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  8  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  9  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 10  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 11  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 12  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 13  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 14  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 15  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff } };

// max: 255 (adc counts)
static long l_sfp1_thresh[MAX_SLAVE][FEBEX_CH] = {
// channel               0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15
      /* FEBEX  0  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  1  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  2  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  3  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  4  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  5  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  6  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  7  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  8  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  9  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 10  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 11  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 12  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 13  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 14  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 15  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff } }; 

// max: 255 (adc counts)
static long l_sfp2_thresh[MAX_SLAVE][FEBEX_CH] = {
// channel               0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15
      /* FEBEX  0  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  1  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  2  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  3  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  4  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  5  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  6  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  7  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  8  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  9  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 10  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 11  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 12  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 13  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 14  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 15  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff } }; 

// max: 255 (adc counts)
static long l_sfp3_thresh[MAX_SLAVE][FEBEX_CH] = {
// channel               0      1      2      3      4      5      6      7      8      9      10     11     12     13     14     15
      /* FEBEX  0  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  1  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  2  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  3  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  4  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  5  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  6  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  7  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  8  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX  9  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 10  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 11  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 12  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 13  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 14  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff },
      /* FEBEX 15  */ { 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff, 0x1ff } };
//--------------------------------------------------------------------------------------------------------

#define STATISTIC   1000000

#define DEBUG 1

#define WAIT_FOR_DATA_READY_TOKEN 1    // - waits until data is ready before
                                       //   sending data to PEXOR
                                       // - otherwisse send data immediately
                                       //   after token arrived at febex/exploder  

#define SEQUENTIAL_TOKEN_SEND 1        // - token sending and receiving is
                                       //   sequential for all used SFPs
                                       // - otherwise token sending and receiving
                                       //   is done parallel for all used SFPs
#ifdef WR_TIME_STAMP
 #define SUB_SYSTEM_ID      0x100
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
#endif // WR_TIME_STAMP

//----------------------------------------------------------------------------

#ifdef SEQUENTIAL_TOKEN_SEND
 #define DIRECT_DMA    1 
 #ifdef DIRECT_DMA 
  #define BURST_SIZE 128
 #endif
#endif

#if defined (USE_MBSPEX_LIB) && ! defined (SEQUENTIAL_TOKEN_SEND)
 #define USE_DRIVER_PARALLEL_TOKENREAD 1
#endif 

#define PEXOR_PC_DRAM_DMA 1

#define USER_TRIG_CLEAR 1

#define CHECK_META_DATA 1

//#define printm printf

#define MAX_TRIG_TYPE     16
#define GET_BAR0_BASE     0x1234
#define PEXDEV            "/dev/pexor"
#define PCI_BAR0_NAME     "PCI_BAR0_MBS"
#define PCI_BAR0_SIZE     0x800000  // in bytes
#define PEX_MEM_OFF       0x100000
#define PEX_REG_OFF       0x20000
#define PEX_SFP_OFF       0x40000   
#define DATA_SEED         0x12CCE6F7
#define MAX_PAGE          10

#define REG_FEB_CTRL       0x200000
#define REG_FEB_TRIG_DELAY 0x200004
#define REG_FEB_TRACE_LEN  0x200008
#define REG_FEB_SELF_TRIG  0x20000C
#define REG_FEB_STEP_SIZE  0x200010
#define REG_FEB_SPI        0x200014
#define REG_FEB_TIME       0x200018
#define REG_FEB_XXX        0x20001C

#define ADC_FIX_SET        0xd41
#define ADC_FIX_VAL        0x800

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
#endif // USE_MBSPEX_LIB 
void f_feb_init ();

#ifdef WR_TIME_STAMP
 void f_wr_init ();
 void f_wr_reset_tlu_fifo ();
#endif
static l_check;

static  long          l_first = 0, l_first2 = 0, l_first3 = 0, l_first4 = 0;       
static  unsigned long l_tr_ct[MAX_TRIG_TYPE];
static  INTU4         l_sfp_pat = 0;
static  INTS4         fd_pex;             // file descriptor for PEXOR device
static  INTS4         l_sfp_slaves[MAX_SFP] = NR_SLAVES;

static  INTS4         l_bar0_base;
static  INTU4  volatile *pl_virt_bar0;
static  s_pexor       sPEXOR;

static  int   l_i, l_j, l_k;
static  long  l_stat;
static  long  l_dat1, l_dat2, l_dat3;
//static  long  l_data;

static  long  l_tog=1;   // start always with buffer 0 !!
static  long  l_tok_mode;
static  long  l_dummy;
static  long  l_tok_check;
static  long  l_n_slaves;
static  long  l_cha_head;
static  long  l_cha_size;
static  long  l_trace_head;
static  long  l_trace_trail;
static  long  l_filt_on_off;
static  long  l_lec_check=-1;
static  long  l_check_err=0;
static  long long l_err_prot_ct=0;
static  long  l_feb_init_ct=0; 
static  long  l_feb_buf_off   [MAX_SFP][MAX_SLAVE][2];
static  long  l_feb_n_chan    [MAX_SFP][MAX_SLAVE];
static  long  l_feb_chan_off  [MAX_SFP][MAX_SLAVE];
//static  long  l_feb_trace_len [MAX_SFP][MAX_SLAVE];
//static  long  l_feb_trig_delay[MAX_SFP][MAX_SLAVE];
static  long  l_feb_ctrl;
static  long  l_feb_time;

static  long  l_trig_type;
static  long  l_sfp_id;
static  long  l_feb_id;
static  long  l_cha_id;

static  long  l_spec_head;
static  long  l_spec_trail;

static  INTU4 *pl_dat_save, *pl_tmp;

#ifndef SEQUENTIAL_TOKEN_SEND
 static  long  l_dat_len_sum[MAX_SFP];
 static  long  l_dat_len_sum_long[MAX_SFP];
#endif

static  INTU4 volatile *pl_pex_sfp_mem_base[MAX_SFP];
static  INTU4 volatile *pl_dma_source_base;
static  INTU4 volatile *pl_dma_target_base;
static  INTU4 volatile *pl_dma_trans_size;
static  INTU4 volatile *pl_dma_burst_size;
static  INTU4 volatile *pl_dma_stat;
static  long            l_dma_target_base;
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

static  long l_trace_head_lec_err=0;
static  long l_trace_trail_lec_err=0;
static  long l_feb_triva_trig_type_mism=0;
static  long l_feb_chan_data_size_1_err=0;
static  long l_feb_chan_data_size_3_err=0;

//-new-//
static short clk_source[2]=CLK_SOURCE_ID; 
//static short step_size[FEBEX_CH]=TRIG_STEP_SIZE; 
//----//

static long l_12_14   [MAX_SFP][MAX_SLAVE];
static long l_pol     [MAX_SFP][MAX_SLAVE];
static long l_trig_mod[MAX_SFP][MAX_SLAVE];
static long l_ev_od_or[MAX_SFP][MAX_SLAVE];
static long l_dis_cha [MAX_SFP][MAX_SLAVE];
static long l_dat_redu[MAX_SFP][MAX_SLAVE];
static long l_ena_trig[MAX_SFP][MAX_SLAVE];
static long l_thresh  [MAX_SFP][MAX_SLAVE][FEBEX_CH]; 

#ifdef WR_TIME_STAMP
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

#ifdef WRITE_ANALYSIS_PARAM
 static long l_pola    [MAX_SFP];
#endif

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
  }
  #endif // WR_TIME_STAMP

  #ifdef USE_MBSPEX_LIB
  if (l_first2 == 0)
  {
    l_first2 = 1;

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

  } // if (l_first2 == 0) // if defined USE_MBSPEX_LIB

  #else // USE_MBSPEX_LIB 

  if (l_first2 == 0)
  {
    l_first2 = 1;

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
    if ((pl_virt_bar0 = (INTU4 *) mmap (NULL, PCI_BAR0_SIZE, prot, flags, fd_pex, 0)) == MAP_FAILED)
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
    if((pl_virt_bar0 = (INTU4 *) smem_create (PCI_BAR0_NAME,
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
        pl_pex_sfp_mem_base[l_i] = (INTU4 volatile*)
         ((long)pl_virt_bar0 + (long)PEX_MEM_OFF + (long)(PEX_SFP_OFF * l_i));   
        l_pex_sfp_phys_mem_base[l_i] = (long)PEX_MEM_OFF + (long)(PEX_SFP_OFF * l_i);

        pl_dma_source_base = (INTU4 volatile*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x0 );
        pl_dma_target_base = (INTU4 volatile*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x4 );
        pl_dma_trans_size  = (INTU4 volatile*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x8 );
        pl_dma_burst_size  = (INTU4 volatile*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0xc );
        pl_dma_stat        = (INTU4 volatile*)((long)pl_virt_bar0 + (long)PEX_REG_OFF + (long) 0x10);

        l_sfp_pat |= (1<<l_i);
      }
    }
    printm ("sfp pattern: 0x%x \n", l_sfp_pat);     
  } // if (l_first2 == 0) // if NOT defined USE_MBSPEX_LIB
  printm ("pl_virt_bar0: 0x%p \n", pl_virt_bar0); 
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
  #endif // (else) USE_MBSPEX_LIB 
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

  if (l_first4 == 0)
  {
    l_first4 = 1;

    #ifdef WAIT_FOR_DATA_READY_TOKEN
    l_tok_mode = 2;    // febex / exploder wait for data ready 
    #else
    l_tok_mode = 0;    // febex / exploder send data after token reception
    #endif

    // map febex setup parameters 
    for (l_j=0; l_j<MAX_SLAVE; l_j++)
    {
      l_12_14   [0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 31) & 0x1;
      l_pol     [0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 28) & 0x1;
      l_trig_mod[0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 24) & 0xf;
      l_ev_od_or[0][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 20) & 0x1;
      l_dis_cha [0][l_j] =  l_sfp0_feb_ctrl0[l_j]        & 0x1ffff;
      l_dat_redu[0][l_j] =  l_sfp0_feb_ctrl1[l_j]        & 0x1ffff;
      l_ena_trig[0][l_j] =  l_sfp0_feb_ctrl2[l_j]        & 0xffff; 

      l_12_14   [1][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 31) & 0x1;
      l_pol     [1][l_j] = (l_sfp1_feb_ctrl0[l_j] >> 28) & 0x1;
      l_trig_mod[1][l_j] = (l_sfp1_feb_ctrl0[l_j] >> 24) & 0xf;
      l_ev_od_or[1][l_j] = (l_sfp1_feb_ctrl0[l_j] >> 20) & 0x1;
      l_dis_cha [1][l_j] =  l_sfp1_feb_ctrl0[l_j]        & 0x1ffff;
      l_dat_redu[1][l_j] =  l_sfp1_feb_ctrl1[l_j]        & 0x1ffff;
      l_ena_trig[1][l_j] =  l_sfp1_feb_ctrl2[l_j]        & 0xffff; 

      l_12_14   [2][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 31) & 0x1;
      l_pol     [2][l_j] = (l_sfp2_feb_ctrl0[l_j] >> 28) & 0x1;
      l_trig_mod[2][l_j] = (l_sfp2_feb_ctrl0[l_j] >> 24) & 0xf;
      l_ev_od_or[2][l_j] = (l_sfp2_feb_ctrl0[l_j] >> 20) & 0x1;
      l_dis_cha [2][l_j] =  l_sfp2_feb_ctrl0[l_j]        & 0x1ffff;
      l_dat_redu[2][l_j] =  l_sfp2_feb_ctrl1[l_j]        & 0x1ffff;
      l_ena_trig[2][l_j] =  l_sfp2_feb_ctrl2[l_j]        & 0xffff; 

      l_12_14   [3][l_j] = (l_sfp0_feb_ctrl0[l_j] >> 31) & 0x1;
      l_pol     [3][l_j] = (l_sfp3_feb_ctrl0[l_j] >> 28) & 0x1;
      l_trig_mod[3][l_j] = (l_sfp3_feb_ctrl0[l_j] >> 24) & 0xf;
      l_ev_od_or[3][l_j] = (l_sfp3_feb_ctrl0[l_j] >> 20) & 0x1;
      l_dis_cha [3][l_j] =  l_sfp3_feb_ctrl0[l_j]        & 0x1ffff;
      l_dat_redu[3][l_j] =  l_sfp3_feb_ctrl1[l_j]        & 0x1ffff;
      l_ena_trig[3][l_j] =  l_sfp3_feb_ctrl2[l_j]        & 0xffff; 

      #ifdef WRITE_ANALYSIS_PARAM
      l_pola[0] |= (l_pol[0][l_j] << l_j);
      l_pola[1] |= (l_pol[1][l_j] << l_j);
      l_pola[2] |= (l_pol[2][l_j] << l_j); 
      l_pola[3] |= (l_pol[3][l_j] << l_j);
      #endif

      for (l_k=0; l_k<FEBEX_CH; l_k++)
      {
        l_thresh[0][l_j][l_k] = l_sfp0_thresh[l_j][l_k] & 0xfff;
        l_thresh[1][l_j][l_k] = l_sfp1_thresh[l_j][l_k] & 0xfff;
        l_thresh[2][l_j][l_k] = l_sfp2_thresh[l_j][l_k] & 0xfff;
        l_thresh[3][l_j][l_k] = l_sfp3_thresh[l_j][l_k] & 0xfff;
      }
    }  

    #ifndef USE_MBSPEX_LIB
    PEXOR_GetPointer(0, pl_virt_bar0, &sPEXOR); 
    #endif  

    f_feb_init();
    l_tog = 1;
    l_lec_check = -1;
  }
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

  #ifdef CHECK_META_DATA
  if (l_check_err == 1)
  {
    printm ("");
    printm ("re-initialize all febex modules \n");
    f_feb_init ();
    l_feb_init_ct++; 
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

  #ifdef WR_TIME_STAMP  
  if (bh_trig_typ < 14)
  {
    *pl_dat++ = SUB_SYSTEM_ID;  *l_se_read_len =+ 4;

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
    /* read high word of latched timestamp */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSHI, EB_BIG_ENDIAN|EB_DATA32, &eb_tlu_high_ts);
    /* read low word of latched timestamp */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSLO, EB_BIG_ENDIAN|EB_DATA32, &eb_tlu_low_ts);
    /* read fine time word of latched timestamp */
    eb_cycle_read (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_FTSSUB, EB_BIG_ENDIAN|EB_DATA32, &eb_tlu_fine_ts);
    /* pop timestamp from FIFO */
    eb_cycle_write (eb_cycle, wrTLU + GSI_TM_LATCH_FIFO_POP, EB_BIG_ENDIAN|EB_DATA32, 0xF);
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

  #ifdef WRITE_ANALYSIS_PARAM
  *pl_dat++ = (l_sfp_slaves[3] << 24) + (l_sfp_slaves[2] << 16)
            + (l_sfp_slaves[1] << 8)  +  l_sfp_slaves[0];
  *pl_dat++ = (FEB_TRIG_DELAY << 16) + FEB_TRACE_LEN;
  #ifdef ENABLE_ENERGY_FILTER
  *pl_dat++ = (ENERGY_SUM_A << 21) + (ENERGY_GAP << 11) + ENERGY_SUM_B;
  #else  
  *pl_dat++ = 0;
  #endif // ENABLE_ENERGY_FILTER  
  *pl_dat++ = l_pola[0];
  *pl_dat++ = l_pola[1];
  *pl_dat++ = l_pola[2];
  *pl_dat++ = l_pola[3];
  #endif // WRITE_ANALYSIS_PARAM

  switch (bh_trig_typ)
  {
    case 1:
    case 2:
    case 3:

    if (l_tog == 1) { l_tog = 0; } else { l_tog = 1; }

    //#ifdef  WAIT_FOR_DATA_READY_TOKEN
    #if defined (WAIT_FOR_DATA_READY_TOKEN) && ! (SEQUENTIAL_TOKEN_SEND) && ! defined (USE_DRIVER_PARALLEL_TOKENREAD)
    //printm ("send token in WAIT_FOR_DATA_READY_TOKEN mode \n");
    //printm ("l_tog | l_tok_mode: 0x%x \n", l_tog | l_tok_mode);
    //sleep (1);
    #ifdef USE_MBSPEX_LIB
    l_stat =  mbspex_send_tok (fd_pex, l_sfp_pat,  l_tog | l_tok_mode);
    #else
    l_stat = f_pex_send_tok (l_sfp_pat, l_tog | l_tok_mode);
    #endif // USE_MBSPEX_LIB
    #endif // (WAIT_FOR_DATA_READY_TOKEN) && ! (SEQUENTIAL_TOKEN_SEND) && ! defined(USE_DRIVER_PARALLEL_TOKENREAD) 

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
      #if  defined (USE_DRIVER_PARALLEL_TOKENREAD) &&  ! (SEQUENTIAL_TOKEN_SEND)
      l_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt;

      l_stat=mbspex_send_and_receive_parallel_tok (fd_pex, l_sfp_pat, l_tog | l_tok_mode,
      (long) l_dma_target_base, (long unsigned*) &l_dma_trans_size, &l_dummy, &l_tok_check, &l_n_slaves);
      if (l_stat !=0)
      {
        printm (RON"ERROR>>"RES" mbspex_send_and_receive_parallel_tok to slave(s) / SFPs failed\n");
        l_err_prot_ct++;
        l_check_err = 2; goto bad_event;
      }
      pl_dat += (l_dma_trans_size>>2); // l_dma_trans_size bytes to pointer units - int
      #else // (USE_DRIVER_PARALLEL_TOKENREAD) &&  ! (SEQUENTIAL_TOKEN_SEND)

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

          #ifndef USE_MBSPEX_LIB
          // select SFP for PCI Express DMA
          *pl_dma_stat = 1 << (l_i+1);
          #endif // USE_MBSPEX_LIB

          #endif //DIRECT_DMA

          #ifdef USE_MBSPEX_LIB
          l_stat = mbspex_send_and_receive_tok (fd_pex, l_i, l_tog | l_tok_mode,
                  (long) l_dma_target_base, (long unsigned*) &l_dma_trans_size, 
                  &l_dummy, &l_tok_check, &l_n_slaves);
          #else
          *pl_dma_target_base = l_dma_target_base;
          l_stat = f_pex_send_and_receive_tok (l_i, l_tog | l_tok_mode, &l_dummy, &l_tok_check, &l_n_slaves);
          #endif // USE_MBSPEX_LIB

          if (l_stat == -1)
          {
            printm (RON"ERROR>>"RES" PEXOR send token to slave(s) / SFPs failed\n");
            //exit(1);
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
            // l_dat1 = l_dat;
            l_dat = *pl_dma_stat;
            //            printm ("status: %d \n", l_dat); sleep (1);
            //printm ("bursts: %d \n", l_burst_size);
            if (l_dat == 0xffffffff)
            {
              printm (RON"ERROR>>"RES" PCIe bus errror, check again\n" );
              l_dat = *pl_dma_stat;
              if (l_dat == 0xffffffff)
              {
                printm (RON"ERROR>>"RES" PCIe bus errror, exiting.. \n" );
                exit (0);
              }
            }
            else if ((l_dat & 0x1)  == 0)
            {
              break; // dma shall be finished 
            }
            l_ct++;
            if ( (l_ct % 1000000) == 0)
            {
              printm ("DMA not ready after %d queries on SFP %d: l_dat: %d \n", l_ct, l_i, l_dat);  
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

          //if(l_dma_trans_size % 8 != 0) {printm ("dma data size  0x%x\n", l_dma_trans_size);}

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
      #endif // USE_MBSPEX_LIB 
      #endif // (ifndef) WAIT_FOR_DATA_READY_TOKEN 

      for (l_i=0; l_i<MAX_SFP; l_i++)
      {
        if (l_sfp_slaves[l_i] != 0)
        {
          // wait until token of all used SFPs returned successfully
          #ifdef USE_MBSPEX_LIB
          l_dma_target_base = 0; // disable automatic internal dma, 
                                 // we do it manually with burst adjustment later!
          l_stat = mbspex_receive_tok (fd_pex, l_i, l_dma_target_base, (long unsigned*) &l_dma_trans_size,
                   &l_dummy, &l_tok_check, &l_n_slaves);
          #else
          l_stat = f_pex_receive_tok (l_i, &l_dummy, &l_tok_check, &l_n_slaves);
          #endif // USE_MBSPEX_LIB

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
            printm (RON"ERROR>>"RES" token mode bit differs from token return token mode bit \n");
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
        }
      }
      #endif // else SEQUENTIAL_TOKEN_SEND := parallel token send

      #ifndef DIRECT_DMA
      // read exploder/febex data (sent by token mode to the pexor)
      // from pexor the pexor memory 
      for (l_i=0; l_i<MAX_SFP; l_i++)
      {
        if (l_sfp_slaves[l_i] != 0)
        {
          #ifdef USE_MBSPEX_LIB
          l_dat_len_sum[l_i] = mbspex_get_tok_memsize(fd_pex, l_i); // in bytes
          #else
          l_dat_len_sum[l_i] = PEXOR_TK_Mem_Size (&sPEXOR, l_i);    // in bytes
          #endif // USE_MBSPEX_LIB
          l_dat_len_sum[l_i] += 4; // wg. shizu !!??
      
          #ifdef PEXOR_PC_DRAM_DMA

          // choose burst size to accept max. 20% padding size
          if      (l_dat_len_sum[l_i] < 0xa0 ) { l_burst_size = 0x10; }
          else if (l_dat_len_sum[l_i] < 0x140) { l_burst_size = 0x20; }
          else if (l_dat_len_sum[l_i] < 0x280) { l_burst_size = 0x40; }
          else                                 { l_burst_size = 0x80; }
 
          // setup DMA

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
            l_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt + l_padd[l_i];
          }
          else
          {
            l_dma_target_base = (long) pl_dat + l_diff_pipe_phys_virt;
          }

          #ifdef USE_MBSPEX_LIB
          mbspex_dma_rd (fd_pex, l_pex_sfp_phys_mem_base[l_i], l_dma_target_base,
                                                                         l_dma_trans_size,l_burst_size);
          // note: return value is true dma transfer size, we do not use this here

          #else // USE_MBSPEX_LIB 

          // source address is (must be) adjusted to burst size ! 
          *pl_dma_source_base = l_pex_sfp_phys_mem_base[l_i];
          *pl_dma_target_base = l_dma_target_base;
          *pl_dma_burst_size  = l_burst_size;                          // in bytes
          *pl_dma_trans_size  = l_dma_trans_size;   

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
              printm ("DMA not ready after %d queries: l_dat: %d \n", l_ct, l_dat);  
              sleep (1);
            }
            #ifndef Linux 
            yield ();
            #else
            sched_yield ();
            #endif
          }
          #endif // (else) USE_MBSPEX_LIB

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
          l_dat_len_sum_long[l_i] = (l_dat_len_sum[l_i] >> 2);
          pl_dat += l_dat_len_sum_long[l_i];      

          #else // PEXOR_PC_DRAM_DMA 

          //l_dat_len_sum_long[l_i] = (l_dat_len_sum[l_i] >> 2) + 1;  // in 4 bytes
          l_dat_len_sum_long[l_i] = (l_dat_len_sum[l_i] >> 2);  // in 4 bytes

          #ifdef USE_MBSPEX_LIB
          for (l_k=0; l_k<l_dat_len_sum_long[l_i]; l_k++)
          {
            l_rd_ct++;
            mbspex_register_rd (fd_pex, 0, PEX_MEM_OFF + (long)(PEX_SFP_OFF * l_i), pl_dat++);
          }
          #else // USE_MBSPEX_LIB 
          pl_tmp = pl_pex_sfp_mem_base[l_i];
          for (l_k=0; l_k<l_dat_len_sum_long[l_i]; l_k++)
          {
            *pl_dat++ = *pl_tmp++;
          }
          #endif // (else) USE_MBSPEX_LIB
          #endif // PEXOR_PC_DRAM_DMA 
        }
      }
      #endif // not DIRECT_DMA
      #endif //  (USE_DRIVER_PARALLEL_TOKENREAD) &&  ! (SEQUENTIAL_TOKEN_SEND)
    }

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
      
      #ifdef CHECK_META_DATA
      printm ("FEBEX - TRIVA trigger type mismatches:   %d \n", l_feb_triva_trig_type_mism);
      printm ("channel data size errors (trig. type 1): %d \n", l_feb_chan_data_size_1_err);
      printm ("channel data size errors (trig. type 3): %d \n", l_feb_chan_data_size_3_err);
      printm ("trace header  lec mismatches             %d \n", l_trace_head_lec_err);
      printm ("trace trailer lec mismatches             %d \n", l_trace_trail_lec_err);
      printm ("");
      printm ("re-initialized FEBEX modules             %d times \n", l_feb_init_ct);
      printm ("gosip error count:                       %lld", l_err_prot_ct);
      #endif // CHECK_META_DATA
      #ifdef WR_TIME_STAMP
      printm ("");
      printm ("reset White Rabbit PEXARIA TLU fifo      %d times \n", l_wr_init_ct);
      #endif // WR_TIME_STAMP 
      printm ("----------------------------------------------------\n");  
    } 

    #ifdef CHECK_META_DATA
    //printm ("----------- check next event------------\n");
    //usleep (100000);
    pl_tmp = pl_dat_save;

    #ifdef WR_TIME_STAMP
    // 5 first 32 bits must be WR time stamp
    l_dat = *pl_tmp++;
    if (l_dat != SUB_SYSTEM_ID)
    {
      printm (RON"ERROR>>"RES" 1. data word is not sub-system id: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", SUB_SYSTEM_ID, l_dat);
    } 
    l_dat = (*pl_tmp++) >> 16;
    if (l_dat != TS__ID_L16)
    {
      printm (RON"ERROR>>"RES" 2. data word does not contain low WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_L16, l_dat);
    }
    l_dat = (*pl_tmp++) >> 16;
    if (l_dat != TS__ID_M16)
    {
      printm (RON"ERROR>>"RES" 3. data word does not contain middle WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_M16, l_dat);
    }
    l_dat = (*pl_tmp++) >> 16; 
    if (l_dat != TS__ID_H16)
    {
      printm (RON"ERROR>>"RES" 4. data word does not contain high WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_H16, l_dat);
    } 
    l_dat = (*pl_tmp++) >> 16; 
    if (l_dat != TS__ID_X16)
    {
      printm (RON"ERROR>>"RES" 5. data word does not contain 48-63 bit WR 16bit identifier: %d \n");
      printm ("should be: 0x%x, but is: 0x%x\n", TS__ID_X16, l_dat);
    } 
    #endif // WR_TIME_STAMP
    
    #ifdef WRITE_ANALYSIS_PARAM
    pl_tmp += 7;
    #endif

    //printm ("total data size (in 4 bytes): %d \n", pl_dat - pl_dat_save);
    while (pl_tmp < pl_dat)
    {
      //printm ("             while start \n");
      l_dat = *pl_tmp++;   // must be padding word or channel header
      //printm ("l_dat 0x%x \n", l_dat);
      if ( (l_dat & 0xfff00000) == 0xadd00000 ) // begin of padding 4 byte words
      {
        //printm ("padding found \n");
        l_dat = (l_dat & 0xff00) >> 8;
        //printm ("padding: %d \n", l_dat); 
        pl_tmp += l_dat - 1;  // increment pointer with nr. of padding  4byte words 
      }
      else if ( (l_dat & 0xff) == 0x34) //channel header
      {
        l_cha_head = l_dat;
        //printm ("gosip header: 0x%x \n", l_cha_head);


        l_trig_type = (l_cha_head & 0xf00)      >>  8;
        l_sfp_id    = (l_cha_head & 0xf000)     >> 12;
        l_feb_id    = (l_cha_head & 0xff0000)   >> 16;
        l_cha_id    = (l_cha_head & 0xff000000) >> 24;
 
        if ((l_cha_id > 16) && (l_cha_id < 0xff)) 
        {  
          printm (RON"ERROR>>"RES" wrong channel id: %d \n", l_cha_id);
          printm ("        for SFP: %d, FEBEX id: %d, channel %d \n", l_sfp_id, l_feb_id, l_cha_id);
          l_err_prot_ct++;
          l_check_err = 2; goto bad_event;
        }

        if (l_sfp_id > 3) 
        {  
          printm (RON"ERROR>>"RES" wrong SFP id: %d \n", l_sfp_id);
          printm ("        for SFP: %d, FEBEX id: %d, channel %d \n", l_sfp_id, l_feb_id, l_cha_id);
          l_err_prot_ct++;
          l_check_err = 2; goto bad_event;
        }

        if (l_feb_id >= l_sfp_slaves[l_sfp_id]) 
        {  
          printm (RON"ERROR>>"RES" wrong febex id: %d \n", l_cha_id);
          printm ("        for SFP: %d, FEBEX id: %d, channel %d \n", l_sfp_id, l_feb_id, l_cha_id);
          l_err_prot_ct++;
          l_check_err = 2; goto bad_event;
        }

        if ( ((l_cha_head & 0xff) >> 0) != 0x34 )
        {
          printm (RON"ERROR>>"RES" channel header type is not 0x34 \n");
          l_err_prot_ct++;
        }

        if ( l_trig_type != bh_trig_typ )
        {
          printm (RON"ERROR>>"RES" trigger type is not the same as from TRIVA \n");
          printm ("        trigger types: TRIVA: %d, FEBEX: %d \n",
                            bh_trig_typ, (l_cha_head & 0xff00) >> 8);
          printm ("        for SFP: %d, FEBEX id: %d, channel %d \n", l_sfp_id, l_feb_id, l_cha_id);
          l_err_prot_ct++;
          l_feb_triva_trig_type_mism++;
          l_check_err = 2; goto bad_event; 
        }

        if ( (l_cha_head & 0xff000000) == 0xff000000) // special channel 0xff 
        {
          //printm ("special channel \n");
          l_cha_size = *pl_tmp++;
          //printm ("E,t size: %d \n", l_cha_size);

          l_spec_head = *pl_tmp++;
          if ( (l_spec_head & 0xff000000) != 0xaf000000)
          {  
            printm (RON"ERROR>>"RES" E,t summary: wrong header is 0x%x, must be: 0x%x\n",
                    (l_spec_head & 0xff000000)>>24, 0xaf);              
            l_err_prot_ct++;
            //l_feb_chan_data_size_1_err++;
            l_check_err = 2; goto bad_event;
            //sleep (1); 
          }

          pl_tmp+=((l_cha_size>>2)-2);

          l_spec_trail = *pl_tmp++;
          if ( (l_spec_trail & 0xff000000) != 0xbf000000)
          {  
            printm (RON"ERROR>>"RES" E,t summary: wrong trailer is 0x%x, must be: 0x%x\n",
                    (l_spec_trail & 0xff000000)>>24, 0xbf);              
            l_err_prot_ct++;
            //l_feb_chan_data_size_1_err++;
            l_check_err = 2; goto bad_event;
          }
          else
          {
            //printm ("E,t check \n");
            //usleep (1);
          } 

        }
        else // real channel 
        {
          //printm ("real channel \n");
          // channel data size
          l_cha_size = *pl_tmp++; 
          //printm ("size: %d \n", l_cha_size);

          l_trace_head = *pl_tmp++;
          l_filt_on_off = (l_trace_head & 0x80000) >> 19;
          if (l_filt_on_off == 1)
          {
            //printm ("Energy Filter is enabled \n");
          }
          else
          {
            //printm ("Energy Filter is disabled \n");
          }

          if (bh_trig_typ == 1)
          {
            if (l_filt_on_off == 0)  // energy filter off
            {
              if (l_cha_size != ((FEB_TRACE_LEN * 2) + 8))
              {
                printm (RON"ERROR>>"RES" channel data size: %d is  wrong \n", l_cha_size);
                printm ("        for trigger type %d \n", bh_trig_typ);
                l_err_prot_ct++;
                l_feb_chan_data_size_1_err++;
                //l_check_err = 2; goto bad_event; 
              }
            }
            else                      // energy filter on 
            {
              if (FEB_TRACE_LEN <= 2000)
              {
                if (l_cha_size != ((FEB_TRACE_LEN * 8) + 8))
                {
                  printm (RON"ERROR>>"RES" channel data size: %d is  wrong \n", l_cha_size);
                  printm ("        for trigger type %d \n", bh_trig_typ);
                  l_err_prot_ct++;
                  l_feb_chan_data_size_1_err++;
                  //l_check_err = 2; goto bad_event; 
                }
              } 
              else                   // trace length > 2000,  cutoff for trace and filter trace
              {
                if (l_cha_size != 16008) // 2000*8+8 = 16008 bytes per channel 
                {
                  printm (RON"ERROR>>"RES" channel data size: %d is  wrong \n", l_cha_size);
                  printm ("        for trigger type %d \n", bh_trig_typ);
                  l_err_prot_ct++;
                  l_feb_chan_data_size_1_err++;
                  //l_check_err = 2; goto bad_event; 
                }
              }
            }
          }
          else if (bh_trig_typ == 3)
          {
            //printm ("synch. trigger, bh_trig_typ: 3 \n"); 
            if (l_cha_size != 8)
            {
              printm (RON"ERROR>>"RES" channel data size: %d is  wrong \n", l_cha_size);
              printm ("        for trigger type %d \n", bh_trig_typ);
              l_err_prot_ct++;
              l_feb_chan_data_size_3_err++;
              //l_check_err = 2; goto bad_event; 
            }
          }
          else
          {
            printm ("TRIVA trigger type neither 1 nor 3 ?? \n");
          }  

          // trace header
          //printm ("trace header \n");

          if ( (l_trace_head & 0xffff) != (l_lec_check & 0xffff) )
          {
            printm (RON"ERROR>>"RES" local event counter mismatch in trace header \n");
            printm ("        SFP: %d, slave id: %d, channel: %d \n", l_sfp_id, l_feb_id, l_cha_id);
            printm ("        lec is: %d, but must be %d \n",
                    l_trace_head & 0xffff, l_lec_check & 0xffff);
            l_err_prot_ct++;
            l_trace_head_lec_err++;
            l_check_err = 2; goto bad_event; 
          }
          if ( ((l_trace_head & 0x10000) >> 16) != l_tog )
          {
            printm (RON"ERROR>>"RES" buffer (0,1) mismatch with toggle bit in trace header\n");
            l_err_prot_ct++;
          }
          if ( ((l_trace_head & 0x300000) >> 20) != bh_trig_typ )
          {
            printm (RON"ERROR>>"RES" wrong trigger type in trace header \n");
            l_err_prot_ct++;
          }
          if ( ((l_trace_head & 0xff000000) >> 24) != 0xaa)
          {
            printm (RON"ERROR>>"RES" trace header id is not 0xaa \n");
            l_err_prot_ct++;              
          }

          // jump over trace
          pl_tmp += (l_cha_size >> 2) - 2;          
            
          // trace trailer
          //printm ("trace trailer \n");
          l_trace_trail = *pl_tmp++;
          if ( (l_trace_trail & 0xffff) != (l_lec_check & 0xffff) )
          {
            printm (RON"ERROR>>"RES" local event counter mismatch in trace trailer\n");
            printm ("        SFP: %d, slave id: %d, channel: %d \n", l_sfp_id, l_feb_id, l_cha_id);
            printm ("        lec is: %d, but must be %d \n\n",
                    l_trace_trail & 0xffff, l_lec_check & 0xffff);
            l_err_prot_ct++;
            l_trace_trail_lec_err++;
            l_check_err = 2; goto bad_event; 
          }

          if ( ((l_trace_trail & 0x10000) >> 16) != l_tog )
          {
            printm (RON"ERROR>>"RES" buffer (0,1) mismatch with toggle bit in trace trailer\n");
            l_err_prot_ct++; 
          }
          if ( ((l_trace_trail & 0x300000) >> 20) != bh_trig_typ )
          {
            printm (RON"ERROR>>"RES" wrong trigger type in trace trailer \n");
            printm ("        TRIVA: %d, trailer: %d \n",
                            bh_trig_typ, ((l_trace_trail & 0xf00000) >> 20));
            l_err_prot_ct++;
          }
          if ( ((l_trace_trail & 0xff000000) >> 24) != 0xbb)
          {
            printm (RON"ERROR>>"RES" trace trailer id is not 0xbb \n");
            l_err_prot_ct++;
          }
        }
      }
      else
      {
        printm (RON"ERROR>>"RES" evt: %d data word: 0x%x neither channel header nor padding word \n", l_tr_ct[0], l_dat);
        sleep (1);
        goto bad_event;
      }       
    }
    #endif // CHECK_META_DATA 

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

    if (l_check_err == 0)
    { 
      *l_se_read_len = (long)pl_dat - (long)pl_dat_save;
    }
    else
    {
      printm ("febex failure: invalidate current trigger/event  (0xbad00bad)\n");
      pl_dat = pl_dat_save;
      *pl_dat++ = 0xbad00bad;
      *l_se_read_len = 4; 
      l_check_err--;
    }  
    break;

    case 15:
      //l_tog = 1;
      //l_lec_check = -1;
    break;
    default:
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
  #endif // (else) USE_MBSPEX_LIB
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
  #endif // (else) USE_MBSPEX_LIB
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
  #endif // (else) USE_MBSPEX_LIB
}

/*****************************************************************************/

void f_feb_init ()

{
  //PEXOR_Port_Monitor (&sPEXOR);
  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      l_stat = f_pex_slave_init (l_i, l_sfp_slaves[l_i]);
      usleep(4000);  
      if (l_stat == -1)
      {
        printm (RON"ERROR>>"RES" slave address initialization failed \n");
        printm ("exiting...\n"); 
        exit (-1); 
      }
    }
    printm ("");
  }

  if (l_first == 0)
  {
    l_first = 1;
    for (l_i=0; l_i<MAX_TRIG_TYPE; l_i++)
    {
      l_tr_ct[l_i] = 0;
    }
  }
  // disable all receivers in all Febex board (first reset; Ivan -16.01.2013):
  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        // reset FEBEX, Ivan's febex implementation
        l_stat = f_pex_slave_wr (l_i, l_j, DATA_FILT_CONTROL_REG, 0x00);
        usleep (4000);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" FEBEX reset failed\n");
          l_err_prot_ct++;
        }
      }
    }
  }
  sleep (1);

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        // reset FEBEX, Ivan's febex implementation
        // needed for check of meta data, read it in any case
        printm ("SFP: %d, FEBEX/EXPLODER: %d \n", l_i, l_j); 
        // get address offset of febex buffer 0,1 for each febex/exploder
        l_stat = f_pex_slave_rd (l_i, l_j, REG_BUF0, &(l_feb_buf_off[l_i][l_j][0]));

        l_stat = f_pex_slave_rd (l_i, l_j, REG_BUF1, &(l_feb_buf_off[l_i][l_j][1]));

        // get nr. of channels per febex
        l_stat = f_pex_slave_rd (l_i, l_j, REG_SUBMEM_NUM, &(l_feb_n_chan[l_i][l_j]));

        // get buffer per channel offset
        l_stat = f_pex_slave_rd (l_i, l_j, REG_SUBMEM_OFF, &(l_feb_chan_off[l_i][l_j]));

        printm ("addr offset: buf0: 0x%x, buf1: 0x%x \n",
                l_feb_buf_off[l_i][l_j][0], l_feb_buf_off[l_i][l_j][1]);
        printm ("No. channels: %d \n", l_feb_n_chan[l_i][l_j]);
        printm ("channel addr offset: 0x%x \n", l_feb_chan_off[l_i][l_j]);

        // disable test data length
        l_stat = f_pex_slave_wr (l_i, l_j, REG_DATA_LEN, 0x10000000);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" disabling test data length failed\n");
          l_err_prot_ct++;
        }

        // specify trace length in slices
        l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TRACE_LEN, FEB_TRACE_LEN);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_TRACE_LEN failed\n");
          l_err_prot_ct++;
        }

        /*
        l_stat = f_pex_slave_rd (l_i, l_j, REG_FEB_TRACE_LEN, &(l_feb_trace_len[l_i][l_j]));
        if (l_feb_trace_len[l_i][l_j] != FEB_TRACE_LEN)
        {
          printm (RON"ERROR>>"RES" writing trace length failed, exiting \n");
          exit (0);
        }
        */

        // specify trigger delay in slices
        l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TRIG_DELAY, FEB_TRIG_DELAY);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_TRIG_DELAY failed\n");
          l_err_prot_ct++;
        }

        /*
        l_stat = f_pex_slave_rd (l_i, l_j, REG_FEB_TRIG_DELAY, &(l_feb_trig_delay[l_i][l_j]));
        if (l_feb_trig_delay[l_i][l_j] != FEB_TRIG_DELAY)
        {
          printm (RON"ERROR>>"RES" writing trigger delay failed, exiting \n");
          exit (0);
        }
        */

        // disable trigger acceptance in febex
        l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_CTRL, 0);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
          l_err_prot_ct++;
        }

        l_stat = f_pex_slave_rd (l_i, l_j, REG_FEB_CTRL, &l_feb_ctrl);
        if ( (l_feb_ctrl & 0x1) != 0)
        {
          printm (RON"ERROR>>"RES" disabling trigger acceptance in febex failed, exiting \n");
          l_err_prot_ct++;
          exit (0);
        }

        // enable trigger acceptance in febex
        l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_CTRL, 1);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
          l_err_prot_ct++;
        }

        l_stat = f_pex_slave_rd (l_i, l_j, REG_FEB_CTRL, &l_feb_ctrl);
        if ( (l_feb_ctrl & 0x1) != 1)
        {
          printm (RON"ERROR>>"RES" enabling trigger acceptance in febex failed, exiting \n");
          l_err_prot_ct++;
          exit (0);
        }

        // set channels used for self trigger signal
        // l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_SELF_TRIG, TRIG_ENA_CHANNEL);
        l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_SELF_TRIG,
        ((l_ev_od_or[l_i][l_j]<<21)|(l_pol[l_i][l_j]<<20)|(l_trig_mod[l_i][l_j]<<16)|l_ena_trig[l_i][l_j]) );
        //                         ((TRIG_2OR<<21)|(TRIG_POSNEG<<20)|(TRIG_METHOD<<16)|TRIG_ENA_CHANNEL) );
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_SELF_TRIG failed\n");
          l_err_prot_ct++;
        }

        // set the step size for self trigger and data reduction
        // l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_STEP_SIZE, ((TRIG_POSNEG<<8) | (TRIG_STEP_SIZE)));

        for (l_k=0; l_k < FEBEX_CH ; l_k++)
        {
          l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_STEP_SIZE, (( l_k<<24 ) | l_thresh[l_i][l_j][l_k]) );
          //l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_STEP_SIZE, (( l_k<<24 ) |step_size[l_k]) );
          if (l_stat == -1)
          {
            printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_STEP_SIZE failed\n");
            l_err_prot_ct++;
          }
        }

        // reset the time stamp and set the clock source for time stamp counter
        if( l_i==clk_source[0] &&   l_j==clk_source[1] )
        {
          l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME,0x0 );
          l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME,0x7 );
          //l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME,0x3 );
        }
        else
        {
          l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME, 0x0 );
          l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME, 0x5 );
          //l_stat = f_pex_slave_wr (l_i, l_j, REG_FEB_TIME, 0x1 );
        }
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_TIME failed\n");
          l_err_prot_ct++;
        }

        // enable/disable no hit in trace data suppression of channel
        l_stat = f_pex_slave_wr (l_i, l_j, REG_DATA_REDUCTION, l_dat_redu[l_i][l_j]);
        //l_stat = f_pex_slave_wr (l_i, l_j, REG_DATA_REDUCTION, DATA_REDUCTION);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_DATA_REDUCTION failed\n");
          l_err_prot_ct++;
        }

        // set channels used for self trigger signal
        l_stat = f_pex_slave_wr (l_i, l_j, REG_MEM_DISABLE, l_dis_cha[l_i][l_j] );
        //l_stat = f_pex_slave_wr (l_i, l_j, REG_MEM_DISABLE, DISABLE_CHANNEL );
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_MEM_DISABLE  failed\n");
          l_err_prot_ct++;
        }

        // write SFP id for channel header
        l_stat = f_pex_slave_wr (l_i, l_j, REG_HEADER, l_i);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" PEXOR slave write REG_HEADER  failed\n");
          l_err_prot_ct++;
        }

        // set/checks fpga clocks with fixed code for adc channel 0 (adc 0)
        //                                       and  adc channel 8 (adc 1)

        // set trapez parameters for trigger/hit finding
        l_stat = f_pex_slave_wr (l_i, l_j, TRIG_SUM_A_REG, TRIG_SUM_A);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" setting TRIG_SUM_A failed\n");
          l_err_prot_ct++;
        }

        l_stat = f_pex_slave_wr (l_i, l_j, TRIG_GAP_REG, TRIG_SUM_A + TRIG_GAP);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" setting GAP failed\n");
          l_err_prot_ct++;
        }

        l_stat = f_pex_slave_wr (l_i, l_j, TRIG_SUM_B_REG, TRIG_SUM_A  + TRIG_GAP + TRIG_SUM_B );
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" setting TRIG_SUM_B failed\n");
          l_err_prot_ct++;
        }

        #ifdef ENABLE_ENERGY_FILTER
        #ifdef TRAPEZ   

        // set trapez parameters for energy estimation
        l_stat = f_pex_slave_wr (l_i, l_j, ENERGY_SUM_A_REG, ENERGY_SUM_A);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" setting ENERGY_SUM_A failed\n");
          l_err_prot_ct++;
        }

        l_stat = f_pex_slave_wr (l_i, l_j, ENERGY_GAP_REG, ENERGY_SUM_A + ENERGY_GAP);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" setting GAP failed\n");
          l_err_prot_ct++;
        }

        l_stat = f_pex_slave_wr (l_i, l_j, ENERGY_SUM_B_REG, ENERGY_SUM_A  + ENERGY_GAP + ENERGY_SUM_B );
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" setting ENERGY_SUM_B failed\n");
          l_err_prot_ct++;
        }

        // select clock source and enable trapezoidal energy filter
        #endif // TRAPEZ
        #endif // ENABLE_ENERGY_FILTER 
      }
    }

  }

  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        // check presence of external clock  
        l_stat = f_pex_slave_rd (l_i, l_j, REG_FEB_TIME, &l_feb_time);
        if ( (l_feb_time & 0x10) != 0x10)
        {
          printm (RON"ERROR>>"RES" external clock not present for SFP: %d FEB: %d \n", l_i, l_j);
          printm ("exiting.. \n"); 
          l_err_prot_ct++;
          exit (0);
        }
      }
    }
  }
  usleep(50);
  // enabling after "ini" of all registers (Ivan - 16.01.2013):
  for (l_i=0; l_i<MAX_SFP; l_i++)
  {
    if (l_sfp_slaves[l_i] != 0)
    {
      for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
      {
        // reset FEBEX, Ivan's febex implementation
        l_stat = f_pex_slave_wr (l_i, l_j, DATA_FILT_CONTROL_REG, DATA_FILT_CONTROL_DAT);
        if (l_stat == -1)
        {
          printm (RON"ERROR>>"RES" FEBEX reset failed\n");
          l_err_prot_ct++;
        }
      }
    }
  }
  sleep (1);
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

#endif // (ifndef) USE_MBSPEX_LIB 

/*****************************************************************************/

#ifndef USE_MBSPEX_LIB

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

#endif // (ifndef) USE_MBSPEX_LIB

/*****************************************************************************/

#ifndef USE_MBSPEX_LIB

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
    printm (RON"ERROR>>"RES" receiving token from SFP: %d \n", l_sfp);
    printm ("  no reply: 0x%x 0x%x 0x%x \n", *pl_check1, *pl_check2, *pl_check3);
    #endif // DEBUG
  }

  return (l_ret);
}

#endif // (ifndef) USE_MBSPEX_LIB

/*****************************************************************************/

#ifdef WR_TIME_STAMP
void f_wr_init ()  
{
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
}
#endif // WR_TIME_STAMP

/*****************************************************************************/

#ifdef WR_TIME_STAMP
void f_wr_reset_tlu_fifo ()  
{
  /* clear all FIFOs */
  if ((eb_stat = eb_device_write(eb_device, wrTLU + GSI_TM_LATCH_FIFO_CLEAR,
                                          EB_BIG_ENDIAN|EB_DATA32, 0xFFFFFFFF, 0, eb_block)) != EB_OK)
  {
    printm (RON"ERROR>>"RES" etherbone TLU eb_device_write (CLEAR TLU FIFOS), status: %s \n", eb_status(eb_stat));
  }
}
#endif // WR_TIME_STAMP

/*****************************************************************************/
