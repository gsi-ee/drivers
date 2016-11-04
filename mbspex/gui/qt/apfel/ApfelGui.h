#ifndef APFELGUI_H
#define APFELGUI_H

#include "ui_ApfelGui.h"
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <QProcess>
#include <QString>

/** this define will switch between direct call of mbspex lib or external shell call of gosipcmd*
 * note: we need to call "make nombspex" if we disable this define here!
 * note2: this define is enabled from top Makefile when building regular "make all"*/
//#define USE_MBSPEX_LIB 1

#ifdef USE_MBSPEX_LIB
extern "C"
{
#include "mbspex/libmbspex.h"
}
#else
// provide dummy structure although never filled by driver:
#define PEX_SFP_NUMBER 4
struct pex_sfp_links{
    int numslaves[PEX_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
};

#endif



#include <iostream>

/** uncomment this if we need to explicitely enable i2c before settings apfel registers*/
#define APFEL_NEED_ENABLEI2C 1

/** uncomment this to initialize apfel frontends with some example config:*/
//#define DO_APFEL_INIT 1

#define GOS_I2C_DWR  0x208010  // i2c data write reg.   addr
#define GOS_I2C_DRR1 0x208020  // i2c data read  reg. 1 addr

/** number of apfel chips on each slave board*/
#define APFEL_NUMCHIPS 8


/** number of DAC channels for each apfel chip*/
#define APFEL_NUMDACS 4


/** number of output channels for each apfel chip*/
#define APFEL_NUMCHANS 2



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
#define APFEL_IO_CONTROL 0x1b000000


#define APFEL_SW_NOINPUT  (1<<0)
#define APFEL_SW_HIGAIN   (1<<1)
#define APFEL_SW_STRETCH  (1<<2)


/** this value will enable settings of control register*/
#define APFEL_IO_SET 0x91000000

/** 10 bit registers for apfeldac settings:*/
#define APFEL_DAC_MAXVALUE 0x3ff


/** Calibration variation of DAC:*/
#define APFEL_DAC_DELTACALIB 2


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



#define ASSERT_APFEL_VALID(X)   if(X<0 || X>=APFEL_NUMCHIPS) return -1;
#define ASSERT_DAC_VALID(X)   if(X<0 || X>=APFEL_NUMDACS) return -1;
#define ASSERT_CHAN_VALID(X)   if(X<0 || X>=APFEL_NUMCHANS) return -1;
#define ASSERT_FEBCHAN_VALID(X)   if(X<0 || X>=APFEL_ADC_CHANNELS) return -1;


/** this is a class (structure) to remember the setup of individual APFEL chip*/
class ApfelSetup
{

private:


  /** the address id of this apfel chip on the board*/
  uint8_t fAddressID;

  /** the absolute values of the APFEL dacs*/
  uint16_t fDACValueSet[APFEL_NUMDACS];

  /** low gain setting for high amplification mode (16 or 32). Default is 32*/
  bool fLowGainSet[APFEL_NUMCHANS];

  /** Enabled test pulser for channel*/
  bool fTestPulsEnable[APFEL_NUMCHANS];

  /** True if test pulser with positive polarity. False for negative*/
  bool fTestPulsPositive;




public:

  /* all initialization here:*/
  ApfelSetup ():fAddressID(0)
  {
    for (int c = 0; c < APFEL_NUMDACS; ++c)
    {
      fDACValueSet[c] = 0;
    }

    for (int c = 0; c < APFEL_NUMCHANS; ++c)
    {
      fLowGainSet[c] = true;
      fTestPulsEnable[c] = false;

    }
    fTestPulsPositive = true;

  }

  /** getter and setter methods to avoid possible segfaults at wrong indices: */
  int GetDACValue(int dac)
  {
    ASSERT_DAC_VALID(dac)
    //std::cout << "GetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<< std::endl;
    return (fDACValueSet[dac]& 0x3FF);
  }

  int SetDACValue(int dac, uint16_t value)
    {
      ASSERT_DAC_VALID(dac)
      fDACValueSet[dac]=(value & 0x3FF);
      //std::cout << "SetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<<", val="<<(int) value<< std::endl;
      return 0;
    }


  int SetLowGain(int chan, bool low=true)
    {
      ASSERT_CHAN_VALID(chan);
      fLowGainSet[chan]= low;
      return 0;
    }

  int GetLowGain(int chan)
      {
        ASSERT_CHAN_VALID(chan);
        return ( fLowGainSet[chan] ? 1: 0);
      }
  int SetTestPulseEnable(int chan, bool on=true)
     {
       ASSERT_CHAN_VALID(chan);
       fTestPulsEnable[chan]= on;
       return 0;
     }

  int GetTestPulseEnable(int chan)
      {
        ASSERT_CHAN_VALID(chan);
        return ( fTestPulsEnable[chan] ? 1: 0);
      }

  int SetTestPulsePostive(bool pos=true)
       {
         fTestPulsPositive= pos;
         return 0;
       }

    int GetTestPulsePositive()
        {
          return ( fTestPulsPositive ? 1: 0);
        }

    void SetAddressID(uint8_t address)
          {
            fAddressID=address;
          }

    uint8_t GetAddressID()
      {
        return fAddressID;
      }




};

/** this structure contains DAC calibration curve parameters for each febex ADC channel*/
class GainSetup
{
private:

  double fDAC_ADC_Slope;
  double fDAC_0;

public:

  GainSetup(): fDAC_ADC_Slope(1.0), fDAC_0(0)
  {
    ResetCalibration();
  }

  void ResetCalibration(bool positive=true)
  {
    if(positive)
    {
      SetSlope(-1.0 * (double) APFEL_DAC_MAXVALUE/ (double) APFEL_ADC_MAXVALUE);
      SetD0(APFEL_DAC_MAXVALUE );
    }
    else
    {
      SetSlope((double) APFEL_DAC_MAXVALUE/ (double) APFEL_ADC_MAXVALUE);
      SetD0(0);
    }
    //DumpCalibration();
  }
  void DumpCalibration()
  {
    printm("dDAC/dADC=%f (DACunit/ADCvalue), DAC0=%f DACunits",fDAC_ADC_Slope,fDAC_0);
  }


  void SetSlope(double val)
  {
    fDAC_ADC_Slope=val;
  }

  void SetD0(double val)
   {
     fDAC_0=val;
   }

  /** function returns dac value to set for relative height of adc baseline in permille*/
  int GetDACValue(double ADC_permille)
  {
    double adctarget=(ADC_permille* ((double) APFEL_ADC_MAXVALUE) / 1000.0);
    int dacsetting= adctarget * fDAC_ADC_Slope + fDAC_0;
    //std::cout << "GetDACValue: dacsetting="<<dacsetting<<", adctarget="<<adctarget<<", permille="<<ADC_permille<< std::endl;
    if(dacsetting<0) dacsetting=0;
    if(dacsetting>APFEL_DAC_MAXVALUE) dacsetting=APFEL_DAC_MAXVALUE;
    return dacsetting;
  }

  int GetADCPermille(double DAC_value)
   {
      double adctarget=(DAC_value - fDAC_0)/fDAC_ADC_Slope;
      if(adctarget<0) adctarget=0;
      if(adctarget>APFEL_ADC_MAXVALUE) adctarget=APFEL_ADC_MAXVALUE;
      double adcpermille= 1000.0 * adctarget/APFEL_ADC_MAXVALUE;
      //std::cout << "GetADCPermille: adctarget="<<adctarget<<", value="<<DAC_value<<", permille="<<adcpermille<< std::endl;
      return adcpermille;
   }


  /** calculate and set calibration curve for measured variations deltaADC and deltaDAC around
   * autocalibrated DAC setting valDAC*/
  void EvaluateCalibration(double deltaDAC, double deltaADC, double valDAC, double valADC)
  {
    if(deltaADC==0)deltaADC=1;
    fDAC_ADC_Slope= deltaDAC/deltaADC;
    fDAC_0 = valADC * fDAC_ADC_Slope + valDAC;
    //std::cout << "EvaluateCalibration("<<deltaDAC<<", "<<deltaADC<<", "<<valDAC<<", "<<valADC<<") - "<< std::endl;
    printm("EvaluateCalibration(dDAC=%f, dADC=%f, DAC1=%f, ADC1=%f",deltaDAC,deltaADC,valDAC,valADC);
    DumpCalibration();
    //std::cout << "   fDAC_ADC_Slope="<<fDAC_ADC_Slope<<", fDAC_0="<<fDAC_0<< std::endl;
  }

};



/** the setup of the apfel/febex slave board*/
class BoardSetup
{

private:

  /** enable apfel input */
  bool fUseApfel;

  /* switch output between gain 16/32 (true) and gain 1 (false)*/
  bool fHighGainOutput;

  /** use stretcher in output (true) or not (false)*/
  bool fStretcher;

 /** property for regular or inverse mounts of apfel addon boards*/
 bool fRegularMapping;

  /** setups of each apfel chip on board*/
  ApfelSetup fApfel[APFEL_NUMCHIPS];

  /** calibration (adc/dac) for gain32*/
  GainSetup fGain_32[APFEL_ADC_CHANNELS];

  /** calibration (adc/dac) for gain16*/
   GainSetup fGain_16[APFEL_ADC_CHANNELS];


   /** calibration (adc/dac) for gain1*/
   GainSetup fGain_1[APFEL_ADC_CHANNELS];

public:

  BoardSetup (): fUseApfel(true),fHighGainOutput(true),fStretcher(false),fRegularMapping(true)
   {
      SetApfelMapping(true);
#ifdef APFEL_GAIN1_INVERTED
      ResetGain1Calibration();
#endif
   }
  bool IsApfelInUse() {return fUseApfel;}
  void SetApfelInUse(bool on){fUseApfel=on;}
  bool IsHighGain() {return fHighGainOutput;}
  void SetHighGain(bool on) {fHighGainOutput=on;}
  bool IsStretcherInUse() {return fStretcher;}
  void SetStretcherInUse(bool on) {fStretcher=on;}
  bool IsRegularMapping() {return fRegularMapping;}

  void SetApfelMapping(bool regular=true)
    {
      //std::cout << "SetApfelMapping("<<regular<<"):"<< std::endl;
      fRegularMapping=regular;
      for(int i=0; i<APFEL_NUMCHIPS; ++i)
        {
          uint8_t add=0;
          if(i<4)
          {
              // regular mapping: indices 0..3 before 8...11
              add= (regular ? i : i+8);
          }
          else
          {
            add= (regular ? i+4 : i-4);
          }

          fApfel[i].SetAddressID(add+1); // shift to id number 1...12 already here!
          //std::cout << "  APFEL["<<i<<"] <- "<<add+1<< std::endl;
        }

    }


  void ResetGain1Calibration()
  {
    // workaround to account inverse polarity of gain 1 dac-adc by default
    for(int ch=0; ch<APFEL_ADC_CHANNELS; ++ch)
    {
      fGain_1[ch].ResetCalibration(false);
    }
  }

  int EvaluateCalibration(int gain, int febexchannel, double deltaDAC, double deltaADC, double valDAC, double valADC)
  {
    ASSERT_FEBCHAN_VALID(febexchannel);
    std::cout << "EvaluateCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
    switch(gain)
    {
      case 1:
        fGain_1[febexchannel].EvaluateCalibration(deltaDAC, deltaADC, valDAC, valADC);
      break;
      case 16:
        fGain_16[febexchannel].EvaluateCalibration(deltaDAC, deltaADC, valDAC, valADC);
      break;
      case 32:
      default:
        fGain_32[febexchannel].EvaluateCalibration(deltaDAC, deltaADC, valDAC, valADC);
        break;
    };
    return 0;
  }

  int ResetCalibration(int gain, int febexchannel)
  {
    ASSERT_FEBCHAN_VALID(febexchannel);
       std::cout << "ResetCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
       switch(gain)
       {
         case 1:
#ifdef APFEL_GAIN1_INVERTED
           fGain_1[febexchannel].ResetCalibration(false);
#else
           fGain_1[febexchannel].ResetCalibration();
#endif
         break;
         case 16:
           fGain_16[febexchannel].ResetCalibration();
         break;
         case 32:
         default:
           fGain_32[febexchannel].ResetCalibration();
           break;
       };
       return 0;
  }

  int DumpCalibration(int gain, int febexchannel)
    {
      ASSERT_FEBCHAN_VALID(febexchannel);
         printm("DumpCalibration for channel %d gain %d:\t",febexchannel,gain);
         switch(gain)
         {
           case 1:
             fGain_1[febexchannel].DumpCalibration();
           break;
           case 16:
             fGain_16[febexchannel].DumpCalibration();
           break;
           case 32:
           default:
             fGain_32[febexchannel].DumpCalibration();
             break;
         };
         return 0;
    }


  int GetDACValue (int gain, int febexchannel, double ADC_permille)
  {
    ASSERT_FEBCHAN_VALID(febexchannel);
    int rev = 0;
    switch (gain)
    {
      case 1:
        rev = fGain_1[febexchannel].GetDACValue (ADC_permille);
        break;
      case 16:
        rev = fGain_16[febexchannel].GetDACValue (ADC_permille);
        break;
      case 32:
      default:
        rev = fGain_32[febexchannel].GetDACValue (ADC_permille);
        break;
    };
    return rev;

  }

  int GetADCPermille (int gain, int febexchannel, double DAC_value)
   {
     ASSERT_FEBCHAN_VALID(febexchannel);
     int rev = 0;
     switch (gain)
     {
       case 1:
         rev = fGain_1[febexchannel].GetADCPermille(DAC_value);
         break;
       case 16:
         rev = fGain_16[febexchannel].GetADCPermille(DAC_value);
         break;
       case 32:
       default:
         rev = fGain_32[febexchannel].GetADCPermille(DAC_value);
         break;
     };
     return rev;

   }

  /** convert febex channel to DAC indices*/
          void EvaluateDACIndices(int febexchannel, int& apfel, int& dac)
            {
                // this function is used for automatic baseline adjustments
                // not so straighforward to use:
                // DAC1 (dac==0): acts on ch0 when 16/32 gain set
                // DAC2 (dac==1): acts on ch1 when 16/32 gain set
                // DAC3 (dac==2): acts both on ch0 and ch1 when 1 gain set
                // DAC4 (dac==3): acts with low gain both on ch0 and ch1 for both gain settings

              apfel= febexchannel/APFEL_NUMCHANS ;
              if(fHighGainOutput)
              {
                  // use first 2 dacs for baseline adjustment if set to high gain:
                  dac= febexchannel-apfel*APFEL_NUMCHANS;
              }
              else
              {
                 // for the moment we always use DAC3 only for gain 1
                 dac=2; // DAC3 with index 2
              }

              // TODO: take into account DAC4 when regulating the low gain case


            }

          int EvaluateADCChannel(int apfel, int dac)
          {
             int chan=apfel*APFEL_NUMCHANS;
             if(fHighGainOutput){
               if(dac<APFEL_NUMCHANS)
               chan+= dac;
               else
                 chan=-1; // mark dac as invalid for adc
             }
             else
             {
               if(dac!=2) chan=-1;       // not sufficient! dac2 works on both adc channels...
             }
             return chan;
          }


     /** get absolute DAC setting from relative baseline slider*/
     int EvaluateDACvalueAbsolute(int permillevalue, int febexchannel=-1, int gain=1)
     {
         int value=APFEL_DAC_MAXVALUE-round((permillevalue* ((double) APFEL_DAC_MAXVALUE) / 1000.0));
         // default: linear interpolation of DAC for complete slider range, note inverted DAC polarity effect on baseline
         if(febexchannel>=0)
         {
           // if channel specified, use calibration from measurements:
           value=GetDACValue(gain, febexchannel, permillevalue);
         }
         return value;
     }

     /** get relative ADC slider value from given dac setting*/
     int EvaluateADCvaluePermille(int value, int febexchannel=-1, int gain=1)
     {

       int permille= 1000 - round (1000.0 * ((double)value/ (double) APFEL_DAC_MAXVALUE));
       // default: linear interpolation of DAC for complete slider range, note inverted DAC polarity effect on baseline
       if(febexchannel>=0)
           {
                  // if channel specified, use calibration from measurements:
             permille=GetADCPermille(gain, febexchannel, value);

           }
       return permille;
     }


    int GetDACValue(int apfel, int dac)
       {
           ASSERT_APFEL_VALID(apfel);
           return fApfel[apfel].GetDACValue(dac);
       }

    int SetDACValue(int apfel, int dac, uint16_t value)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].SetDACValue(dac, value);
    }

     /** helper function to access DAC value via global febex channel number*/
    int GetDACValue(int febexchannel)
      {
        int chip=0, chan=0;
        EvaluateDACIndices(febexchannel, chip, chan);
        return GetDACValue(chip,chan);
      }

    /** helper function to set DAC value via global febex channel number*/
    int SetDACValue(int febexchannel,  uint16_t value)
       {
            int chip=0, chan=0;
            EvaluateDACIndices(febexchannel, chip, chan);
            return SetDACValue(chip,chan, value);
       }

    int SetLowGain(int apfel, int chan, bool low=true)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].SetLowGain(chan, low);

    }

    int GetLowGain(int apfel, int chan)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].GetLowGain(chan);
    }


    int SetTestPulseEnable(int apfel, int chan, bool on=true)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].SetTestPulseEnable(chan,on);
    }

    int GetTestPulseEnable(int apfel,int chan)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].GetTestPulseEnable(chan);
    }


    int SetTestPulsePostive(int apfel, bool pos=true)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].SetTestPulsePostive(pos);
    }

    int GetTestPulsePositive(int apfel)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].GetTestPulsePositive();
    }


    int GetApfelID(int apfel)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].GetAddressID();

    }

    /** evaluate gain factor from setup. returns 1, 16 or 32*/
    int GetGain(int apfel, int dac)
    {
    int gain=0;
     if(!IsHighGain())
     {
       gain=1;
     }
     else
     {
       if(GetLowGain(apfel, dac)) // for high gain, apfel channel index is same as dac index
         gain=16;
       else
         gain=32;
     }
    return gain;
    }

};




class ApfelGui: public QWidget, public Ui::ApfelGui
{
  Q_OBJECT

public:
  ApfelGui (QWidget* parent = 0);
  virtual ~ApfelGui ();


  void AppendTextWindow (const QString& text);

  void AppendTextWindow (const char* txt)
         {
           QString buf (txt);
           AppendTextWindow (buf);
         }

  void FlushTextWindow();

   /** singleton pointer to forward mbspex lib output, also useful without mbspex lib:*/
  static ApfelGui* fInstance;

protected:

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  QProcessEnvironment fEnv;
#endif


  /** for saving of configuration, we now have setup structures for all slaves.
   * array index is sfp, vector index is febex in chain*/
  std::vector<BoardSetup> fSetup[4];


  /** contains currently configured slaves at the chains.*/
  struct pex_sfp_links fSFPChains;


  /** auxiliary references to checkboxes for baseline adjustments*/
  QCheckBox* fBaselineBoxes[16];

  /** auxiliary references to spinbox for baseline adjustment view*/
  QSpinBox* fDACSpinBoxes[16];

  /** auxiliary references to adc baseline display for refresh view*/
  QLineEdit* fADCLineEdit[16];


  /** auxiliary references to dac value display for refresh view*/
  QSlider* fDACSlider[APFEL_NUMCHIPS][APFEL_NUMDACS];


  /** auxiliary references to dac value display for refresh view*/
  QLineEdit* fDACLineEdit[APFEL_NUMCHIPS][APFEL_NUMDACS];

  /** auxiliary references to pulser display for refresh view*/
  QComboBox* fApfelPulsePolarityCombo[APFEL_NUMCHIPS];

  /** auxiliary references to pulser display for refresh view*/
  QCheckBox* fApfelPulseEnabledCheckbox[APFEL_NUMCHIPS][APFEL_NUMCHANS];


  QComboBox* fApfelGainCombo[APFEL_NUMCHIPS][APFEL_NUMCHANS];



  /** text debug mode*/
  bool fDebug;

  /** save configuration file instead of setting device values*/
  bool fSaveConfig;

  /** this flag protects some slots during broadcast write mode*/
  bool fBroadcasting;

  /** base for number display (10 or 16)*/
  int fNumberBase;

  /** index of sfp channel,   -1 for broadcast */
  int fSFP;
  /** index of slave device , -1 for broadcast*/
  int fSlave;

  /** remember sfp channel to recover after broadcast*/
  int fSFPSave;

  /** remember slave channel to recover after broadcast*/
  int fSlaveSave;

  /** configuration output file handle*/
  FILE* fConfigFile;

#ifdef USE_MBSPEX_LIB



  /** file descriptor on mbspex device*/
  int fPexFD;

  /** speed down mbspex io with this function from Nik*/
  void I2c_sleep ();

#endif

  /** update register display*/
  void RefreshView ();

  /** udpate display of dac settings for apfel chip with given index */
  void RefreshDAC(int apfel);

  /** udpate display of adc value of channel. specify gain to set relative dac slider from calibration */
   void RefreshADC_channel(int channel, int gain);

   /** udpate display of adc  that currently belongs to apfel and dac indices*/
   void RefreshADC_Apfel(int apfel, int dac);



//  /** update febex device index display*/
  void RefreshStatus ();

  /** update initilized chain display and slave limit*/
  void RefreshChains();

 /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
  void ApplyGUISettings();

  /** copy values from gui to internal status object*/
  void EvaluateView ();

  /** copy sfp and slave from gui to variables*/
  void EvaluateSlave ();


  /** put io switch settings for apfel chip from gui into setup structure*/
  void EvaluateIOSwitch();

  /** put test pulser settings for apfel chip from gui into setup structure*/
  void EvaluatePulser(int apfel);


  /** put gain settings for apfel chip and channel from gui into setup structure*/
  void EvaluateGain(int apfel, int channel);

  /** set register from status structure*/
  void SetRegisters ();


  /** apply test pulser settings for apfel chip from setup structure to device*/
  void SetPulser(uint8_t apfel);


  /** set io switch from setup structures to device */
  void SetIOSwitch();


  /** set apfel addon boards to inverted mount mode
   * (apfel9-12 first, apfel1-4 second) */
  void SetInverseMapping(int on);


  /** get register contents to status structure*/
  void GetRegisters ();

  /** get DAC settings of apfel into status structure*/
  void GetDACs (int apfel);



  /** get registers and write them to config file*/
  void SaveRegisters();


  /** retrieve slave configuration from driver*/
  void GetSFPChainSetup();


  /** Read from address from sfp and slave, returns value*/
  int ReadGosip (int sfp, int slave, int address);

  /** Write value to address from sfp and slave*/
  int WriteGosip (int sfp, int slave, int address, int value);

  /** Save value to currently open *.gos configuration file*/
  int SaveGosip(int sfp, int slave, int address, int value);

  /** execute (gosip) command in shell. Return value is output of command*/
  QString ExecuteGosipCmd (QString& command,  int timeout=5000);


  /** Map index of apfel chip on board to addressing id number*/
  uint8_t GetApfelId(int sfp, int slave, uint8_t apfelchip);


  /** Write value to i2c bus address of currently selected slave. apfel chip id and local dac id are specified*/
    int WriteDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac, uint16_t value);

    /** Read value to i2c bus address of currently selected slave. apfel id and local dac id are specified*/
    int ReadDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac);


    /** evaluate i2c channel adress offset on apfel for given channel number*/
    int GetChannelOffsetDAC(uint8_t chan);


    /** Read value from adc channel of currently selected slave. adc unit id and local channel id are specified*/
    int ReadADC_Apfel (uint8_t adc, uint8_t chan);

    /** sample adc baseline of global channel febexchan
     *  by avering over several readouts of ADC. Baseline value is returned.*/
    int AcquireBaselineSample(uint8_t febexchan);


    /** set gain factor for each apfel channel on board. High gain switch must be enabled for board.
     *  gain is 16 if useGain16=true, or 32 if useGain16=false (default)
     * */
    void SetGain(uint8_t apfelchip, uint8_t chan, bool useGain16);

    /** set test pulser properties for each apfel channel on board.
         *  flag on=true switches pulser on
         *  chan1 and chan2 specify which channel to activate, both must be set with a single call
         *  flag positive=true: positive polarity, =false: negative pulse
         * */
    void SetTestPulse(uint8_t apfelchip, bool on, bool chan1, bool chan2, bool positive);


    /* Perform automatic calibration of specified apfel chip*/
    void DoAutoCalibrate(uint8_t apfelchip);


    /** set switch register of currently selected slave (apfel input on/off), gain 1 or 16/32, stretcher on/off)*/
    void SetSwitches(bool useApfel, bool useHighGain, bool useStretcher);




   /* Initialize febex after power up*/
   void InitApfel();


  /** helper function that either does enable i2c on board, or writes such commands to .gos file*/
  void EnableI2C();

  /** helper function that either does disable i2c on board, or writes such commands to .gos file*/
  void DisableI2C ();

  /** dump current ADC values of currently set APFEL*/
  void DumpADCs();


  /** *dump dac2 channel calibrations */
  void DumpCalibrations();

  /** open configuration file for writing*/
  int OpenConfigFile(const QString& fname);

  /** guess what...*/
  int CloseConfigFile();

  /** append text to currently open config file*/
  int WriteConfigFile(const QString& text);

  /** Set relativ DAC value permille to APFELchannel, returns ADC value*/
  int autoApply(int channel, int permille);


  /** apply relative DAC value permille and refresh gui from ADC sample.
   * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyRefresh(int channel, int permille);

  /** evaluate change of spinbox for febex channel channel*/
  void DAC_spinBox_changed(int channel, int val);


  /** apply io switch settings directly.
   * * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplySwitch();



  /** apply pulser settings directly
    * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyPulser(int apfel);

 /** slot forward when change of pulser settings on gui*/
  void PulserChanged(int apfel);

  /** apply gain settings directly
      * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyGain(int apfel, int channel);


  /** slot forward when change of pulser settings on gui*/
   void GainChanged(int apfel, int channel);


   /** apply absolute DAC value val directly
  * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyDAC(int apfel, int dac, int val);


  /** slot forward when change of dacslider for apfelchip and dac
   * refresh display of textline
   * may do automatic apply*/
  void DAC_changed(int apfel, int dac, int val);


  /** slot forward when  input of dac textline for apfelchip and dac
   * also refresh display of slider here
   *  may do automatic apply**/
  void DAC_enterText(int apfel, int dac);


  /** start interactive autocalibration of apfel chip dacs.*/
  void AutoCalibrate(int apfel);



  /** Automatic adjustment of adc baseline to adctarget value for global febex channel.
   * will return final dac setup value or -1 in case of error*/
  int AdjustBaseline(int channel, int adctarget);

  /** Adjust baselines of the currently selected febex device.*/
  void AutoAdjust();


  /** Automatic calibration of DAC->ADC relation for febex channel.
   * Will AutoCalibrate corresponding apfel first*/
    int CalibrateADC(int channel);

    /** Calibrate DAC->ADC for ADC channels with set checkbox checked.*/
    void CalibrateSelectedADCs();

    /** Reset calibration of DAC->ADC relation for febex channel.
      * Default is linear falling curve betwen adc and dac*/
    int CalibrateResetADC(int channel);

    /** Reset Calibration  DAC->ADC for ADC channels with set checkbox checked.*/
    void CalibrateResetSelectedADCs();




  void DebugTextWindow (const char*txt)
  {
      AppendTextWindow (txt);
  }
  void DebugTextWindow (const QString& text)
  {
    if (fDebug)
      AppendTextWindow (text);
  }
  /** Check if broadast mode is not set. If set, returns false and prints error message if verbose is true*/
  bool AssertNoBroadcast (bool verbose=true);


  /** Check if chain for given sfp and slave index is configured correctly*/
  bool AssertChainConfigured (bool verbose=true);


public slots:
  virtual void ShowBtn_clicked();
  virtual void ApplyBtn_clicked ();
  virtual void InitChainBtn_clicked ();
  virtual void ResetBoardBtn_clicked ();
  virtual void ResetSlaveBtn_clicked ();
  virtual void BroadcastBtn_clicked (bool checked);
  virtual void DumpBtn_clicked ();
  virtual void ClearOutputBtn_clicked ();
  virtual void ConfigBtn_clicked ();
  virtual void SaveConfigBtn_clicked ();
  virtual void AutoAdjustBtn_clicked ();
  virtual void CalibrateADCBtn_clicked();
  virtual void CalibrateResetBtn_clicked();
  virtual void DebugBox_changed (int on);
  virtual void HexBox_changed(int on);
  virtual void Slave_changed(int val);
  virtual void DAC_spinBox_all_changed(int val);
  virtual void Any_spinBox00_changed(int val);
  virtual void Any_spinBox01_changed(int val);
  virtual void Any_spinBox02_changed(int val);
  virtual void Any_spinBox03_changed(int val);
  virtual void Any_spinBox04_changed(int val);
  virtual void Any_spinBox05_changed(int val);
  virtual void Any_spinBox06_changed(int val);
  virtual void Any_spinBox07_changed(int val);
  virtual void Any_spinBox08_changed(int val);
  virtual void Any_spinBox09_changed(int val);
  virtual void Any_spinBox10_changed(int val);
  virtual void Any_spinBox11_changed(int val);
  virtual void Any_spinBox12_changed(int val);
  virtual void Any_spinBox13_changed(int val);
  virtual void Any_spinBox14_changed(int val);
  virtual void Any_spinBox15_changed(int val);



  virtual void DAC_changed_0_0(int val);
  virtual void DAC_changed_0_1(int val);
  virtual void DAC_changed_0_2(int val);
  virtual void DAC_changed_0_3(int val);
  virtual void DAC_changed_1_0(int val);
  virtual void DAC_changed_1_1(int val);
  virtual void DAC_changed_1_2(int val);
  virtual void DAC_changed_1_3(int val);
  virtual void DAC_changed_2_0(int val);
  virtual void DAC_changed_2_1(int val);
  virtual void DAC_changed_2_2(int val);
  virtual void DAC_changed_2_3(int val);
  virtual void DAC_changed_3_0(int val);
  virtual void DAC_changed_3_1(int val);
  virtual void DAC_changed_3_2(int val);
  virtual void DAC_changed_3_3(int val);
  virtual void DAC_changed_4_0(int val);
  virtual void DAC_changed_4_1(int val);
  virtual void DAC_changed_4_2(int val);
  virtual void DAC_changed_4_3(int val);
  virtual void DAC_changed_5_0(int val);
  virtual void DAC_changed_5_1(int val);
  virtual void DAC_changed_5_2(int val);
  virtual void DAC_changed_5_3(int val);
  virtual void DAC_changed_6_0(int val);
  virtual void DAC_changed_6_1(int val);
  virtual void DAC_changed_6_2(int val);
  virtual void DAC_changed_6_3(int val);
  virtual void DAC_changed_7_0(int val);
  virtual void DAC_changed_7_1(int val);
  virtual void DAC_changed_7_2(int val);
  virtual void DAC_changed_7_3(int val);


  virtual void DAC_enterText_0_0 ();
  virtual void DAC_enterText_0_1 ();
  virtual void DAC_enterText_0_2 ();
  virtual void DAC_enterText_0_3 ();
  virtual void DAC_enterText_1_0 ();
  virtual void DAC_enterText_1_1 ();
  virtual void DAC_enterText_1_2 ();
  virtual void DAC_enterText_1_3 ();
  virtual void DAC_enterText_2_0 ();
  virtual void DAC_enterText_2_1 ();
  virtual void DAC_enterText_2_2 ();
  virtual void DAC_enterText_2_3 ();
  virtual void DAC_enterText_3_0 ();
  virtual void DAC_enterText_3_1 ();
  virtual void DAC_enterText_3_2 ();
  virtual void DAC_enterText_3_3 ();
  virtual void DAC_enterText_4_0 ();
  virtual void DAC_enterText_4_1 ();
  virtual void DAC_enterText_4_2 ();
  virtual void DAC_enterText_4_3 ();
  virtual void DAC_enterText_5_0 ();
  virtual void DAC_enterText_5_1 ();
  virtual void DAC_enterText_5_2 ();
  virtual void DAC_enterText_5_3 ();
  virtual void DAC_enterText_6_0 ();
  virtual void DAC_enterText_6_1 ();
  virtual void DAC_enterText_6_2 ();
  virtual void DAC_enterText_6_3 ();
  virtual void DAC_enterText_7_0 ();
  virtual void DAC_enterText_7_1 ();
  virtual void DAC_enterText_7_2 ();
  virtual void DAC_enterText_7_3 ();


  virtual void AutoCalibrate_0();
  virtual void AutoCalibrate_1();
  virtual void AutoCalibrate_2();
  virtual void AutoCalibrate_3();
  virtual void AutoCalibrate_4();
  virtual void AutoCalibrate_5();
  virtual void AutoCalibrate_6();
  virtual void AutoCalibrate_7();
  virtual void AutoCalibrate_all();

  virtual void PulserChanged_0();
  virtual void PulserChanged_1();
  virtual void PulserChanged_2();
  virtual void PulserChanged_3();
  virtual void PulserChanged_4();
  virtual void PulserChanged_5();
  virtual void PulserChanged_6();
  virtual void PulserChanged_7();

  virtual void GainChanged_0();
  virtual void GainChanged_1();
  virtual void GainChanged_2();
  virtual void GainChanged_3();
  virtual void GainChanged_4();
  virtual void GainChanged_5();
  virtual void GainChanged_6();
  virtual void GainChanged_7();
  virtual void GainChanged_8();
  virtual void GainChanged_9();
  virtual void GainChanged_10();
  virtual void GainChanged_11();
  virtual void GainChanged_12();
  virtual void GainChanged_13();
  virtual void GainChanged_14();
  virtual void GainChanged_15();

  virtual void SwitchChanged();

  virtual void InverseMapping_changed (int on);
};

#endif
