#ifndef APFELDEFINES_H
#define APFELDEFINES_H


#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <iostream>


/** enable this for alternative programming of switches in panda test mode - later gui parameter*/
#define APFEL_USE_SIMPLE_SWITCHES 1


/** enable this for old toellner power mode*/
//#define APFEL_USE_TOELLNER_POWER 1



/** uncomment this if we need to explicitely enable i2c before settings apfel registers*/
#define APFEL_NEED_ENABLEI2C 1

/** uncomment this to initialize apfel frontends with some example config:*/
#define DO_APFEL_INIT 1

/** enable initialization sequence for febex4 ADCs. Works only if DO_APFEL_INIT is set*/
#define USE_FEBEX4 1


#define GOS_I2C_DWR  0x208010  // i2c data write reg.   addr
#define GOS_I2C_DRR1 0x208020  // i2c data read  reg. 1 addr
/** number of apfel chips on each slave board*/
#define APFEL_NUMCHIPS 8

/** number of DAC channels for each apfel chip*/
#define APFEL_NUMDACS 4

/** number of output channels for each apfel chip*/
#define APFEL_NUMCHANS 2


/** number of multipulser peak samples*/
#define APFELTEST_MULTIPULSER_PEAKS 3

/** base address for writing apfel core register (corresponds to DAC 1/index0)*/
#define APFEL_CORE_REQUEST_BASE_WR 0x12000000

/** base address for reading apfel core register (corresponds to DAC 1)*/
#define APFEL_CORE_REQUEST_BASE_RD 0x92000000

/** offset for each daq index when accessing core registers*/
#define APFEL_CORE_REQUEST_DAC_OFFSET 0x1000000

/** base address for reading apfel DAC register (corresponds to DAC 1)*/
#define APFEL_DAC_REQUEST_BASE_RD 0x96000000

/** base address for writing transfer of data to apfel DACs*/
#define APFEL_TRANSFER_BASE_WR 0x11810000

/** base address for reading transfer of data from apfel DACs*/
#define APFEL_TRANSFER_BASE_RD 0x11820000

/** offset for each daq index when requesting data transfer*/
#define APFEL_TRANSFER_DAC_OFFSET 0x100

/** base address for setting channel amplifications */
#define APFEL_GAIN_BASE_WR      0x11840000

/** base address for setting channel test pulses */
#define APFEL_TESTPULSE_CHAN_WR 0x11880000

/** base address for setting test pulse polarities */
#define APFEL_TESTPULSE_FLAG_WR 0x11900000

#define APFEL_AUTOCALIBRATE_BASE_WR 0x11A00000

/** general call with reset */
#define APFEL_RESET 0x60010600

/** general call with wake up */
#define APFEL_RESET_WAKE 0x60010900

/* control register base for io setup, data is in ls bits:
 * [0] - 0: use apfel, 1: use something else (POLAND)
 * [1] - 0: gain 1, 1: gain16/32/
 * [2] - 0: no stretcher, 1: stretcher
 * */
#define APFEL_IO_CONTROL_WR 0x1b000000

/** low 16bit of ioc control data word*/
#define APFEL_IO_DATA_LO_WR 0x1c000000

/** hi 16bit of ioc control data word*/
#define APFEL_IO_DATA_HI_WR 0x1d000000




/** read back control register bits here:*/
#define APFEL_IO_CONTROL_RD 0x95000000

#define APFEL_SW_NOINPUT  (1<<0)
#define APFEL_SW_HIGAIN   (1<<1)
#define APFEL_SW_STRETCH  (1<<2)

/** this value will enable settings of control register*/
#define APFEL_IO_SET 0x91000000

/** 10 bit registers for apfeldac settings:*/
#define APFEL_DAC_MAXVALUE 0x3ff

/** Calibration variation of DAC:*/
//#define APFEL_DAC_DELTACALIB 2

/** enable this define to make local variation around calib point*/
//#define APFEL_DAC_LOCALCALIB 1


#define APFEL_DAC_DELTACALIB 2

#define APFEL_DAC_DELTASTEP 1

/** ADC saturation value (end of linear calibration range)*/
#define APFEL_ADC_MAXSATURATION 15000

/** ADC gain 1 saturation value (end of linear calibration range)*/
#define APFEL_ADC_MINSATURATION 3000

#define APFEL_ADC_SATURATIONDELTA 100


/** adress to read actual adc value. adc id and channel must be
 * written to this address first*/
#define APFEL_ADC_PORT  0x20001c

/** 14 bit registers for apfeldac settings:*/
#define APFEL_ADC_MAXVALUE 0x3fff

/** number of adc units per febex*/
#define APFEL_ADC_NUMADC 2

/** number of channels per adc unit*/
#define APFEL_ADC_NUMCHAN 8

/** total number of channels on febex*/
#define APFEL_ADC_CHANNELS 16

/* number of samples to evaluate average adc baseline value*/
#define APFEL_ADC_BASELINESAMPLES 3

/** comment the following if apfel gain 1 dac is not inverted anymore TODO!*/
#define APFEL_GAIN1_INVERTED 1
// note that we can always hanlde inverted slope with button on gui now :)


/** number of samples read out for each channel*/
#define APFEL_ADC_SAMPLEVALUES 200


#define APFEL_ADC_NUMMAXIMA 7


/** core read request write address to fetch ADC DAQBUFFER*/
#define APFEL_ADC_DAQBUFFER_REQ_PORT 0x2080c0


/** base address for DAQ buffer in MBS readout mode.
 * actual address is this times ADC index (1...16)*/
#define APFEL_ADC_DAQBUFFER_BASE 0x8000

/** maximum length of 16bit word buffer for mbs trace sample*/
#define APFEL_MBS_TRACELEN 8000

/** number of points on DAC curve*/
#define APFEL_DAC_CURVEPOINTS 24


/** value that is set to table when parameter was not measured or evaluated*/
#define APFEL_NOVALUE -1



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


// new for febex4:

#define DATA_FILT_CONTROL_DAT       0x000080

#define ADC0_CLK_PHASE          0x23001607
#define ADC1_CLK_PHASE          0x23001607

#define DATA_FILT_CONTROL_RST       0x000000

#define ADC_FIX_CODE       0x23000d00

///////////////

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

#define ASSERT_APFEL_VALID(X)   if(X<0 || X>=APFEL_NUMCHIPS) return APFEL_NOVALUE;
#define ASSERT_DAC_VALID(X)   if(X<0 || X>=APFEL_NUMDACS) return APFEL_NOVALUE;
#define ASSERT_CHAN_VALID(X)   if(X<0 || X>=APFEL_NUMCHANS) return APFEL_NOVALUE;
#define ASSERT_FEBCHAN_VALID(X)   if(X<0 || X>=APFEL_ADC_CHANNELS) return APFEL_NOVALUE;



#define APFEL_RESULT_TIMEFORMAT "dd.MM.yyyy hh:mm:ss.zzz"


#define APFEL_ADDRESSTEST_SLEEP usleep(1);
//usleep(50000);

// need this for definition of sfp_links etc:
#include "GosipGui.h"
/** this define will switch between direct call of mbspex lib or external shell call of gosipcmd*
 * note: we need to call "make nombspex" if we disable this define here!
 * note2: this define is enabled from top Makefile when building regular "make all"*/
//#define USE_MBSPEX_LIB 1
//#ifdef USE_MBSPEX_LIB
//extern "C"
//{
//#include "mbspex/libmbspex.h"
//}
//#else
//// provide dummy structure although never filled by driver:
//#define PEX_SFP_NUMBER 4
//struct pex_sfp_links
//{
//  int numslaves[PEX_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
//};
//
//#endif





#endif
