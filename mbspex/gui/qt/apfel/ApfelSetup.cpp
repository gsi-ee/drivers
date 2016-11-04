#include "ApfelSetup.h"

#include <iostream>

////////////////////////////////////////////////
//////// the setup data class of a single apfel chip:

ApfelSetup::ApfelSetup () :
    fAddressID (0)
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
int ApfelSetup::GetDACValue (int dac)
{
  ASSERT_DAC_VALID(dac)
  //std::cout << "GetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<< std::endl;
  return (fDACValueSet[dac] & 0x3FF);
}

int ApfelSetup::SetDACValue (int dac, uint16_t value)
{
  ASSERT_DAC_VALID(dac)
  fDACValueSet[dac] = (value & 0x3FF);
  //std::cout << "SetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<<", val="<<(int) value<< std::endl;
  return 0;
}

int ApfelSetup::SetLowGain (int chan, bool low)
{
  ASSERT_CHAN_VALID(chan);
  fLowGainSet[chan] = low;
  return 0;
}

int ApfelSetup::GetLowGain (int chan)
{
  ASSERT_CHAN_VALID(chan);
  return (fLowGainSet[chan] ? 1 : 0);
}
int ApfelSetup::SetTestPulseEnable (int chan, bool on)
{
  ASSERT_CHAN_VALID(chan);
  fTestPulsEnable[chan] = on;
  return 0;
}

int ApfelSetup::GetTestPulseEnable (int chan)
{
  ASSERT_CHAN_VALID(chan);
  return (fTestPulsEnable[chan] ? 1 : 0);
}

int ApfelSetup::SetTestPulsePostive (bool pos)
{
  fTestPulsPositive = pos;
  return 0;
}

int ApfelSetup::GetTestPulsePositive ()
{
  return (fTestPulsPositive ? 1 : 0);
}

void ApfelSetup::SetAddressID (uint8_t address)
{
  fAddressID = address;
}

uint8_t ApfelSetup::GetAddressID ()
{
  return fAddressID;
}



//////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////
//////// the gain setup and calibration properties:


GainSetup::GainSetup(): fDAC_ADC_Slope(1.0), fDAC_0(0)
 {
   ResetCalibration();
 }

 void GainSetup::ResetCalibration(bool positive)
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
 void GainSetup::DumpCalibration()
 {
   printm("dDAC/dADC=%f (DACunit/ADCvalue), DAC0=%f DACunits",fDAC_ADC_Slope,fDAC_0);
 }


 void GainSetup::SetSlope(double val)
 {
   fDAC_ADC_Slope=val;
 }

 void GainSetup::SetD0(double val)
  {
    fDAC_0=val;
  }

 /** function returns dac value to set for relative height of adc baseline in permille*/
 int GainSetup::GetDACValue(double ADC_permille)
 {
   double adctarget=(ADC_permille* ((double) APFEL_ADC_MAXVALUE) / 1000.0);
   int dacsetting= adctarget * fDAC_ADC_Slope + fDAC_0;
   //std::cout << "GetDACValue: dacsetting="<<dacsetting<<", adctarget="<<adctarget<<", permille="<<ADC_permille<< std::endl;
   if(dacsetting<0) dacsetting=0;
   if(dacsetting>APFEL_DAC_MAXVALUE) dacsetting=APFEL_DAC_MAXVALUE;
   return dacsetting;
 }

 int GainSetup::GetADCPermille(double DAC_value)
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
 void GainSetup::EvaluateCalibration(double deltaDAC, double deltaADC, double valDAC, double valADC)
 {
   if(deltaADC==0)deltaADC=1;
   fDAC_ADC_Slope= deltaDAC/deltaADC;
   fDAC_0 = valADC * fDAC_ADC_Slope + valDAC;
   //std::cout << "EvaluateCalibration("<<deltaDAC<<", "<<deltaADC<<", "<<valDAC<<", "<<valADC<<") - "<< std::endl;
   printm("EvaluateCalibration(dDAC=%f, dADC=%f, DAC1=%f, ADC1=%f",deltaDAC,deltaADC,valDAC,valADC);
   DumpCalibration();
   //std::cout << "   fDAC_ADC_Slope="<<fDAC_ADC_Slope<<", fDAC_0="<<fDAC_0<< std::endl;
 }


/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
 ////////////////////////////////////////////////
 //////// the whole slave board setup:

 BoardSetup::BoardSetup (): fUseApfel(true),fHighGainOutput(true),fStretcher(false),fRegularMapping(true)
   {
      SetApfelMapping(true);
#ifdef APFEL_GAIN1_INVERTED
      ResetGain1Calibration();
#endif
   }

  void BoardSetup::SetApfelMapping(bool regular)
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


  void BoardSetup::ResetGain1Calibration()
  {
    // workaround to account inverse polarity of gain 1 dac-adc by default
    for(int ch=0; ch<APFEL_ADC_CHANNELS; ++ch)
    {
      fGain_1[ch].ResetCalibration(false);
    }
  }

  int BoardSetup::EvaluateCalibration(int gain, int febexchannel, double deltaDAC, double deltaADC, double valDAC, double valADC)
  {
    ASSERT_FEBCHAN_VALID(febexchannel);
    //std::cout << "EvaluateCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
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

  int BoardSetup::ResetCalibration(int gain, int febexchannel)
  {
    ASSERT_FEBCHAN_VALID(febexchannel);
       //std::cout << "ResetCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
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

  int BoardSetup::DumpCalibration(int gain, int febexchannel)
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


  int BoardSetup::GetDACValue (int gain, int febexchannel, double ADC_permille)
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

  int BoardSetup::GetADCPermille (int gain, int febexchannel, double DAC_value)
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
          void BoardSetup::EvaluateDACIndices(int febexchannel, int& apfel, int& dac)
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

          int BoardSetup::EvaluateADCChannel(int apfel, int dac)
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
     int BoardSetup::EvaluateDACvalueAbsolute(int permillevalue, int febexchannel, int gain)
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
     int BoardSetup::EvaluateADCvaluePermille(int value, int febexchannel, int gain)
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


    int BoardSetup::GetDACValue(int apfel, int dac)
       {
           ASSERT_APFEL_VALID(apfel);
           return fApfel[apfel].GetDACValue(dac);
       }

    int BoardSetup::SetDACValue(int apfel, int dac, uint16_t value)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].SetDACValue(dac, value);
    }

     /** helper function to access DAC value via global febex channel number*/
    int BoardSetup::GetDACValue(int febexchannel)
      {
        int chip=0, chan=0;
        EvaluateDACIndices(febexchannel, chip, chan);
        return GetDACValue(chip,chan);
      }

    /** helper function to set DAC value via global febex channel number*/
    int BoardSetup::SetDACValue(int febexchannel,  uint16_t value)
       {
            int chip=0, chan=0;
            EvaluateDACIndices(febexchannel, chip, chan);
            return SetDACValue(chip,chan, value);
       }

    int BoardSetup::SetLowGain(int apfel, int chan, bool low)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].SetLowGain(chan, low);

    }

    int BoardSetup::GetLowGain(int apfel, int chan)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].GetLowGain(chan);
    }


    int BoardSetup::SetTestPulseEnable(int apfel, int chan, bool on)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].SetTestPulseEnable(chan,on);
    }

    int BoardSetup::GetTestPulseEnable(int apfel,int chan)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].GetTestPulseEnable(chan);
    }


    int BoardSetup::SetTestPulsePostive(int apfel, bool pos)
    {
      ASSERT_APFEL_VALID(apfel);
      return fApfel[apfel].SetTestPulsePostive(pos);
    }

    int BoardSetup::GetTestPulsePositive(int apfel)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].GetTestPulsePositive();
    }


    int BoardSetup::GetApfelID(int apfel)
    {
        ASSERT_APFEL_VALID(apfel);
        return fApfel[apfel].GetAddressID();

    }

    /** evaluate gain factor from setup. returns 1, 16 or 32*/
    int BoardSetup::GetGain(int apfel, int dac)
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

