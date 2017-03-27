#ifndef FEBEXSETUP_H
#define FEBEXSETUP_H

#include "GosipGui.h"
#include <stdint.h>


#define GOS_I2C_DWR  0x208010  // i2c data write reg.   addr
#define GOS_I2C_DRR1 0x208020  // i2c data read  reg. 1 addr


/** i2c address of first mcp443x/5x/ chip on febex for writing values. Used as base
 * to evaluate values for all 4 chips on board with 4 channels each:*/
#define FEBEX_MCP433_BASE_WRITE 0x62580000

/** i2c address of first mcp443x/5x/ chip on febex for read request. Used as base
 * to evaluate values for all 4 chips on board with 4 channels each:*/
#define FEBEX_MCP433_BASE_READ  0xe2580c00

/** this value is i2c adressing offset between mcp chips*/
#define FEBEX_MCP433_OFFSET 0x20000

/** i2c command value to request a data read from mcp433 */
#define FEBEX_MCP433_REQUEST_READ 0x86000000

/** number of dac chips on febex */
#define FEBEX_MCP433_NUMCHIPS 4

/** number of dac  channels per chip*/
#define FEBEX_MCP433_NUMCHAN 4

/** maximum value to set for DAC*/
#define FEBEX_MCP433_MAXVAL 0xFF

/** adress to read actual adc value. adc id and channel must be
 * written to this address first*/
#define FEBEX_ADC_PORT  0x20001c

/** number of adc units per febex*/
#define FEBEX_ADC_NUMADC 2

/** number of channels per adc unit*/
#define FEBEX_ADC_NUMCHAN 8


/** total number of channels on febex*/
#define FEBEX_CH 16


/* number of samples to evaluate average adc baseline value*/
#define FEBEX_ADC_BASELINESAMPLES 3


//////////////////////////////////////////////////////////////////////7

/* The following is taken from mbs code for initialization of febex after startup:*/


#define FEB_TRACE_LEN   300  // in nr of samples
#define FEB_TRIG_DELAY   30  // in nr.of samples



#define REG_BUF0_DATA_LEN     0xFFFD00  // buffer 0 submemory data length
#define REG_BUF1_DATA_LEN     0xFFFE00  // buffer 1 submemory data length


#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
#define REG_MEM_FLAG_1      0xFFFFBc  // read only:


#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000

#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC

#define REG_RST 0xFFFFF4
#define REG_LED 0xFFFFF8
#define REG_VERSION 0xFFFFFC

#define REG_FEB_CTRL       0x200000
#define REG_FEB_TRIG_DELAY 0x200004
#define REG_FEB_TRACE_LEN  0x200008
#define REG_FEB_SELF_TRIG  0x20000C
#define REG_FEB_STEP_SIZE  0x200010
#define REG_FEB_SPI        0x200014
#define REG_FEB_TIME       0x200018
#define REG_FEB_XXX        0x20001C



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






/** this is a class (structure) to remember the previous setup read, and the
 * next setup to apply on the currently selected febex device:*/
class FebexSetup : public GosipSetup
{
public:

  /** the (relative) baseline values set on the dacs*/
  uint8_t fDACValueSet[FEBEX_MCP433_NUMCHIPS][FEBEX_MCP433_NUMCHAN];

  /** TODO: probably keep the real adc values also here and display them...*/


  /* all initialization here:*/
  FebexSetup (): GosipSetup()
  {
    for (int m = 0; m < FEBEX_MCP433_NUMCHIPS; ++m)
    {
      for (int c = 0; c < FEBEX_MCP433_NUMCHAN; ++c)
       {
         fDACValueSet[m][c]=0;
       }
    }
  }

  /** getter and setter methods to avoid possible segfaults at wrong indices: */
  int GetDACValue(int chip, int chan)
  {
    if(chip<0 || chip>=FEBEX_MCP433_NUMCHIPS || chan <0 || chan >=FEBEX_MCP433_NUMCHAN) return -1; // error handling
    //std::cout << "GetDACValue ("<<chip<<","<<chan<<")="<< (int)(fDACValueSet[chip][chan])<< std::endl;
    return fDACValueSet[chip][chan];
  }

  int SetDACValue(int chip, int chan, uint8_t value)
    {
      if(chip<0 || chip>=FEBEX_MCP433_NUMCHIPS || chan <0 || chan >=FEBEX_MCP433_NUMCHAN) return -1; // error handling
      fDACValueSet[chip][chan]=value;
      //std::cout << "SetDACValue ("<<chip<<","<<chan<<")="<< (int)(fDACValueSet[chip][chan])<<", val="<<(int) value<< std::endl;
      return 0;
    }

  /** convert febex channel to DAC indices*/
   void EvaluateDACIndices(int febexchannel, int& chip, int& chan)
     {
           chip= febexchannel/FEBEX_MCP433_NUMCHAN ;
           chan= febexchannel-chip*FEBEX_MCP433_NUMCHAN;
     }

  /** helper function to access DAC value via global febex channel number*/
  int GetDACValue(int febexchannel)
    {
      int chip=0, chan=0;
      EvaluateDACIndices(febexchannel, chip, chan);
      return GetDACValue(chip, chan);
    }

  /** helper function to set DAC value via global febex channel number*/
  int SetDACValue(int febexchannel,  uint8_t value)
     {
          int chip=0, chan=0;
          EvaluateDACIndices(febexchannel, chip, chan);
          return SetDACValue(chip, chan, value);
     }




};


#endif
