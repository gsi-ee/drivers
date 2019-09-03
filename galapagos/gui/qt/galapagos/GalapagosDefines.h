#ifndef GAPG_DEFINES_H
#define GAPG_DEFINES_H


// magic number for pattern file header,  ASCII identifier GAPG:
#define PATTERN_FILE_TAG 0x47415047

//0xFA1AEAF0

// pattern file format version
#define PATTERN_FILE_VERSION 0x1


/** total number of output channels of pattern generator*/
//#define GAPG_CHANNELS 64
#define GAPG_CHANNELS 16
#define GAPG_SEQUENCES 128
#define GAPG_PATTLETS 128

/** following just fake addresses at the moment to test kernel module*/
#define GAPG_DRAM        0x100000
/*for register testing: use pexor sfp memory*/
#define GAPG_MAIN_CONTROL GAPG_DRAM

#define GAPG_CHANNEL_ENABLE_LOW GAPG_DRAM +4
#define GAPG_CHANNEL_ENABLE_HI  GAPG_DRAM + 8

/** register set to assign sequence id to channels*/
#define GAPG_CHANNEL_SEQUENCE_BASE  GAPG_DRAM + 12

/** register set to assign sequence id to channels*/
#define GAPG_CHANNEL_PATTERN_BASE  GAPG_DRAM + 128

/** start updating sequence by writing id into control word*/
#define GAPG_KNOWN_SEQUENCE_CONTROL  GAPG_DRAM + 256

/** port address fifo to send sequence command tokens*/
#define GAPG_KNOWN_SEQUENCE_PORT  GAPG_DRAM + 264


#define GAPG_BIT_MAIN_ENABLE 0x1

////// end FAKE register setup

/////////////////// JAM 2019 -
///////// below are old definitions from febex, not used.
// keep them as some example what we could do


//#define GOS_I2C_DWR  0x208010  // i2c data write reg.   addr
//#define GOS_I2C_DRR1 0x208020  // i2c data read  reg. 1 addr
//
//
///** i2c address of first mcp443x/5x/ chip on febex for writing values. Used as base
// * to evaluate values for all 4 chips on board with 4 channels each:*/
//#define GAPG_MCP433_BASE_WRITE 0x62580000
//
///** i2c address of first mcp443x/5x/ chip on febex for read request. Used as base
// * to evaluate values for all 4 chips on board with 4 channels each:*/
//#define GAPG_MCP433_BASE_READ  0xe2580c00
//
///** this value is i2c adressing offset between mcp chips*/
//#define GAPG_MCP433_OFFSET 0x20000
//
///** i2c command value to request a data read from mcp433 */
//#define GAPG_MCP433_REQUEST_READ 0x86000000
//
///** number of dac chips on febex */
//#define GAPG_MCP433_NUMCHIPS 4
//
///** number of dac  channels per chip*/
//#define GAPG_MCP433_NUMCHAN 4
//
///** maximum value to set for DAC*/
//#define GAPG_MCP433_MAXVAL 0xFF
//
///** adress to read actual adc value. adc id and channel must be
// * written to this address first*/
//#define GAPG_ADC_PORT  0x20001c
//
///** number of adc units per febex*/
//#define GAPG_ADC_NUMADC 2
//
///** number of channels per adc unit*/
//#define GAPG_ADC_NUMCHAN 8
//
//
///** total number of channels on febex*/
//#define GAPG_CH 16
//
//
///* number of samples to evaluate average adc baseline value*/
//#define GAPG_ADC_BASELINESAMPLES 3
//
//
////////////////////////////////////////////////////////////////////////7
//
///* The following is taken from mbs code for initialization of febex after startup:*/
//
//
//#define FEB_TRACE_LEN   300  // in nr of samples
//#define FEB_TRIG_DELAY   30  // in nr.of samples
//
//
//
//#define REG_BUF0_DATA_LEN     0xFFFD00  // buffer 0 submemory data length
//#define REG_BUF1_DATA_LEN     0xFFFE00  // buffer 1 submemory data length
//
//
//#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
//#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
//#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
//#define REG_MEM_FLAG_1      0xFFFFBc  // read only:
//
//
//#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
//#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
//#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
//#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
//
//#define REG_MODID     0xFFFFE0
//#define REG_HEADER    0xFFFFE4
//#define REG_FOOTER    0xFFFFE8
//#define REG_DATA_LEN  0xFFFFEC
//
//#define REG_RST 0xFFFFF4
//#define REG_LED 0xFFFFF8
//#define REG_VERSION 0xFFFFFC
//
//#define REG_FEB_CTRL       0x200000
//#define REG_FEB_TRIG_DELAY 0x200004
//#define REG_FEB_TRACE_LEN  0x200008
//#define REG_FEB_SELF_TRIG  0x20000C
//#define REG_FEB_STEP_SIZE  0x200010
//#define REG_FEB_SPI        0x200014
//#define REG_FEB_TIME       0x200018
//#define REG_FEB_XXX        0x20001C
//
//
//
//#define DATA_FILT_CONTROL_REG 0x2080C0
//#define DATA_FILT_CONTROL_DAT 0x80         // (0x80 E,t summary always +  data trace                 always
//                                           // (0x82 E,t summery always + (data trace + filter trace) always
//                                           // (0x84 E,t summery always +  data trace                 if > 1 hit
//                                           // (0x86 E,t summery always + (data trace + filter trace) if > 1 hit
//// Trigger/Hit finder filter
//
//#define TRIG_SUM_A_REG    0x2080D0
//#define TRIG_GAP_REG      0x2080E0
//#define TRIG_SUM_B_REG    0x2080F0
//
//#define TRIG_SUM_A     8  // for 12 bit: 8, 4 ,9 (8+1); for 14 bit: 14, 4, 15 (14 + 1).
//#define TRIG_GAP       4
//#define TRIG_SUM_B     9 // 8 + 1: one has to be added.
//
//// Energy Filters and Modes
//
//#define ENABLE_ENERGY_FILTER 1
//
//#define TRAPEZ               1  // if TRAPEZ is off, MWD will be activated
//
//#ifdef ENABLE_ENERGY_FILTER
// #ifdef TRAPEZ
//  #define ENERGY_SUM_A_REG  0x208090
//  #define ENERGY_GAP_REG    0x2080A0
//  #define ENERGY_SUM_B_REG  0x2080B0
//
//  #define ENERGY_SUM_A  64
//  #define ENERGY_GAP    32
//  #define ENERGY_SUM_B  65  // 64 + 1: one has to be added.
// #endif

//#endif




#endif
