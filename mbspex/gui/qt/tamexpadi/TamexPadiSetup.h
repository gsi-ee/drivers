#ifndef FEBEXSETUP_H
#define FEBEXSETUP_H

#include "GosipGui.h"
#include <stdint.h>


/** generic gosip slave registers:*/
#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000

#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC


/** TAMEX registers:*/
#define REG_TAM_CTRL      0x200000
#define REG_TAM_TRG_WIN   0x200004
#define REG_TAM_EN_1      0x200008
#define REG_TAM_EN_2      0x20000c
#define REG_TAM_CLK_SEL   0x311000
#define REG_TAM_BUS_EN    0x311008


/////////// PADI access below: ////////////////////////

/** SPI data register address for writing*/
#define REG_TAM_PADI_DAT_WR  0x311018

/** SPI control register address*/
#define REG_TAM_PADI_CTL  0x311014

/** SPI data register address for reading*/
#define REG_TAM_PADI_DAT_RD 0x31101C



/* SPI control register base command value for reading back PADI thresholds*/
#define COM_TAM_PADI_READ  0x40004000

/* SPI control register base command value for writing PADI thresholds*/
#define COM_TAM_PADI_WRITE  0x80008000


/* tamex control register value to reset with reference channel 0 enabled*/
#define COM_CTRL_REFCHAN_RESET 0x7c20d0

/* tamex control register value to apply with reference channel 0 enabled*/
#define COM_CTRL_REFCHAN_APPLY 0x7c20c0


/* tamex control register value to reset with no reference channel enabled*/
#define COM_CTRL_NOREF_RESET 0x7c2050

/* tamex control register value to apply with no reference channel enabled*/
#define COM_CTRL_NOREF_APPLY 0x7c20c0


/** register bit for "enable or" lemo output */
#define COM_CTRL_ENABLE_OR_BIT (1 << 29)

/** register bit for "combine or" lemo output */
#define COM_CTRL_COMBINE_OR_BIT (1 << 28)




///** number of PADI8 chips on febex */
#define TAMEX_PADI_NUMCHIPS 2
//
///** number of channels per PADI chip*/
#define TAMEX_PADI_NUMCHAN 8


/** nubmer of tamex2 TDC channels*/
#define TAMEX_TDC_NUMCHAN 16


/// This is default clock setup on reset button. Later we may put this also to gui?
#define CLK_SRC_TDC_TAM2 0x24  // TAMEX2: 0x20 -> External CLK via 2 pin lemo (200 MHz)
                               //         0x21 -> CLK from TRBus (25 MHz) via on-board PLL            ! to be tested
                               //         0x22 -> CLK from TRBus + Module 0 feeds 25 MHz CLK to TRBus ! to be tested
                               //         0x24 -> On-board oscillator (200 MHz)


//// default trigger window on reset button. Later we may put this also to gui?
#define TRIG_WIN_EN 1          // 0 trigger window control is off,
                               // everything will be written out


#define PRE_TRIG_TIME  500     // in nr of time slices a 5 ns: max 0x7ff := 2047 * 5 ns
#define POST_TRIG_TIME 500     // in nr of time slices a 5 ns: max 0x7ff := 2047 * 5 ns

#define PADI_DEF_TH 0xa000a000 // PADI thresholds set at startup 2x16 bits for PADI1/2



/** this is a class (structure) to remember the previous setup read, and the
 * next setup to apply on the currently selected febex device:*/
class TamexPadiSetup : public GosipSetup
{
public:

  /** the (relative) threshold values set on the PADI dacs*/
  uint16_t fDACValueSet[TAMEX_PADI_NUMCHIPS][TAMEX_PADI_NUMCHAN];

  /** the chipversion info of the PADIs*/
  uint8_t fChipVersion[TAMEX_PADI_NUMCHIPS];

  /** enabled register for the TDCS. 2 adjacent bits enables leading and trailing hits of one TDC channel?*/
  uint32_t fTDCEnabled;


  /** TDC clock source mode TAMEX2:
   * 0x20 -> External CLK via 2 pin lemo (200 MHz)
     0x21 -> CLK from TRBus (25 MHz) via on-board PLL            ! to be tested
     0x22 -> CLK from TRBus + Module 0 feeds 25 MHz CLK to TRBus ! to be tested
     0x24 -> On-board oscillator (200 MHz)
     * */
  uint8_t fClockSource;


  /** Enables usage of trigger time windows. if false, everything will be send out*/
  bool fUseTriggerwindow;

  /** pre-trigger window in 5ns time slices (0...0x7ff)*/
  uint16_t fTriggerPre;

  /** post-trigger window in 5ns time slices (0...0x7ff)*/
  uint16_t fTriggerPost;

  /** enable trigger of channel OR at the lemo output*/
  bool fEnableOR;

  /** enable trigger of PADI combined channels OR at the lemo output*/
  bool fCombineOR;

  /** enable channel 0 as reference channel for trigger time**/
  bool fUseReferenceChannel;




  /* all initialization here:*/
  TamexPadiSetup (): GosipSetup(), fTDCEnabled(0xFFFFFFFF), fClockSource(0x24),
      fUseTriggerwindow(true), fTriggerPre(500), fTriggerPost(500), fEnableOR(true), fCombineOR(true), fUseReferenceChannel(true)
  {
    for (int p = 0; p < TAMEX_PADI_NUMCHIPS; ++p)
    {
      fChipVersion[p]=0;
      for (int c = 0; c < TAMEX_PADI_NUMCHAN; ++c)
       {
         fDACValueSet[p][c]=0;
       }
    }




  }

   int GetPadiVersion(int chip)
   {
     if(chip<0 || chip>=TAMEX_PADI_NUMCHIPS ) return -1; // error handling
     return fChipVersion[chip];
   }

   int SetPadiVersion(int chip, uint8_t value)
   {
     if(chip<0 || chip>=TAMEX_PADI_NUMCHIPS ) return -1; // error handling
     fChipVersion[chip]=value;
     return 0;
   }




  int GetDACValue(int chip, int chan)
  {
    if(chip<0 || chip>=TAMEX_PADI_NUMCHIPS || chan <0 || chan >=TAMEX_PADI_NUMCHAN) return -1; // error handling
    //std::cout << "GetDACValue ("<<chip<<","<<chan<<")="<< (int)(fDACValueSet[chip][chan])<< std::endl;
    return fDACValueSet[chip][chan];
  }

  int SetDACValue(int chip, int chan, uint16_t value)
    {
      if(chip<0 || chip>=TAMEX_PADI_NUMCHIPS || chan <0 || chan >=TAMEX_PADI_NUMCHAN) return -1; // error handling
      fDACValueSet[chip][chan]=value;
      //std::cout << "SetDACValue ("<<chip<<","<<chan<<")="<< (int)(fDACValueSet[chip][chan])<<", val="<<(int) value<< std::endl;
      return 0;
    }

  /** convert global channel to DAC indices*/
   void EvaluateDACIndices(int globalchannel, int& chip, int& chan)
     {
           chip= globalchannel/TAMEX_PADI_NUMCHAN ;
           chan= globalchannel-chip*TAMEX_PADI_NUMCHAN;
     }

  /** helper function to access DAC value via global channel number*/
  int GetDACValue(int globalchannel)
    {
      int chip=0, chan=0;
      EvaluateDACIndices(globalchannel, chip, chan);
      return GetDACValue(chip, chan);
    }

  /** helper function to set DAC value via global channel number*/
  int SetDACValue(int globalchannel,  uint16_t value)
     {
          int chip=0, chan=0;
          EvaluateDACIndices(globalchannel, chip, chan);
          return SetDACValue(chip, chan, value);
     }


  void SetEnabledRegister(uint32_t val)
  {
    fTDCEnabled=val;
  }

  uint32_t GetEnabledRegister()
  {
    return fTDCEnabled;
  }


 void SetChannelLeadingEnabled(uint8_t ch, bool on)
 {
   if(ch >=TAMEX_TDC_NUMCHAN) return; // error handling
     uint32_t reg=GetEnabledRegister();
     uint32_t flags= (0x1 << (2*ch)); // set leading edge bit
     if(on)
       reg |= flags;
     else
       reg &= ~flags;
     SetEnabledRegister(reg);
 }

 bool IsChannelLeadingEnabled(uint8_t ch)
  {
     if(ch >=TAMEX_TDC_NUMCHAN) return false; // error handling?
     uint32_t reg=GetEnabledRegister();
     uint32_t flags= (0x1 << (2*ch)); // check leading edge bit
     bool rev = ((reg & flags) == flags);
     return rev;
  }


 void SetChannelTrailingEnabled(uint8_t ch, bool on)
 {
   if(ch >=TAMEX_TDC_NUMCHAN) return; // error handling
     uint32_t reg=GetEnabledRegister();
     uint32_t flags= (0x2 << (2*ch)); // set trailing edge bit
     if(on)
       reg |= flags;
     else
       reg &= ~flags;
     SetEnabledRegister(reg);
 }

 bool IsChannelTrailingEnabled(uint8_t ch)
  {
     if(ch >=TAMEX_TDC_NUMCHAN) return false; // error handling?
     uint32_t reg=GetEnabledRegister();
     uint32_t flags= (0x2 << (2*ch)); // check trailing edge bit
     bool rev = ((reg & flags) == flags);
     return rev;
  }


 uint8_t GetClockSource ()
  {
    return fClockSource;
  }

  void SetClockSource (uint8_t mode)
  {
    fClockSource = mode;
  }

  bool IsEnabledTriggerWindow ()
  {
    return fUseTriggerwindow;
  }

  void SetEnabledTriggerWindow (bool on)
  {
    fUseTriggerwindow = on;
  }

  uint16_t GetPreTriggerWindow ()
  {
    return fTriggerPre;
  }

  void SetPreTriggerWindow (uint16_t val)
  {
    fTriggerPre = val;
  }

  uint16_t GetPostTriggerWindow ()
  {
    return fTriggerPost;
  }

  void SetPostTriggerWindow (uint16_t val)
  {
    fTriggerPost = val;
  }

  bool IsEnableOR ()
  {
    return fEnableOR;
  }

  void SetEnableOR (bool on)
  {
    fEnableOR = on;
  }

  bool IsCombineOR ()
  {
    return fCombineOR;
  }

  void SetCombineOR (bool on)
  {
    fCombineOR = on;
  }

  bool IsTriggerReferenceChannel ()
  {
    return fUseReferenceChannel;
  }

  void SetEnableTriggerReferenceChannel (bool on)
  {
    fUseReferenceChannel = on;
  }


};


#endif
