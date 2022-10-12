#include "GainSetup.h"

////////////////////////////////////////////////
//////// the gain setup and calibration properties:


GainSetup::GainSetup(): fDAC_ADC_Slope(1.0), fDAC_0(0), fDAC_min(0), fADC_min(0)
 {
   ResetCalibration();

 }

 void GainSetup::ResetCalibration(bool positive)
 {
   if(positive)
   {
     SetSlope(-1.0 * (double) AWAGS_DAC_MAXVALUE/ (double) AWAGS_ADC_MAXVALUE);
     SetD0(AWAGS_DAC_MAXVALUE );
     SetDACmin(0);
     SetDACmax(AWAGS_DAC_MAXVALUE);
     SetADCmin(0);
   }
   else
   {
     SetSlope((double) AWAGS_DAC_MAXVALUE/ (double) AWAGS_ADC_MAXVALUE);
     SetD0(0);
     SetDACmin(0);
     SetDACmax(AWAGS_DAC_MAXVALUE);
     SetADCmin(0);
   }
   //DumpCalibration();
 }
 void GainSetup::DumpCalibration()
 {
   printm("dDAC/dADC=%f (DACunit/ADCvalue), DAC0=%f DACunits, DAC_min=%f DACunits, DAC_max=%f DACunits, ADC_min=%f",
       fDAC_ADC_Slope,fDAC_0,fDAC_min, fDAC_max, fADC_min);
 }


 void GainSetup::SetSlope(double val)
 {
   fDAC_ADC_Slope=val;
 }

 void GainSetup::SetD0(double val)
  {
    fDAC_0=val;
  }


 void GainSetup::SetDACmin (double val)
 {
   fDAC_min=val;
 }
 void GainSetup::SetDACmax (double val)
  {
    fDAC_max=val;
  }

 void GainSetup::SetADCmin (double val)
 {
   fADC_min=val;
 }


 double GainSetup::GetSlope()
  {
    return fDAC_ADC_Slope;
  }

  double GainSetup::GetD0()
   {
     return fDAC_0;
   }


  double GainSetup::GetDACmin ()
  {
    return fDAC_min;
  }

  double  GainSetup::GetDACmax ()
   {
     return fDAC_max;
   }

  double GainSetup::GetADCmin ()
  {
    return fADC_min;
  }


 /** function returns dac value to set for relative height of adc baseline in permille*/
int GainSetup::CalculateDACValue (double ADC_permille)
{
  double adctarget = (ADC_permille * ((double) AWAGS_ADC_MAXVALUE) / 1000.0);    // + fADC_min;
  // take into account fADC_min for permille range?
  double dacsetting = 0;

  if (fDAC_ADC_Slope < 0)
  {
    // this is the case for gain 16 and gain32:
    if (adctarget < fADC_min)
      dacsetting = fDAC_max;
    else
      dacsetting = adctarget * fDAC_ADC_Slope + fDAC_0;

    if (dacsetting < fDAC_min)
      dacsetting = fDAC_min;

    if (dacsetting > AWAGS_DAC_MAXVALUE)
      dacsetting = AWAGS_DAC_MAXVALUE;
  }
  else
  {
    // for gain 1, need to revert boundaries:
    if (adctarget < fADC_min)
      dacsetting = fDAC_min;
    else
      dacsetting = adctarget * fDAC_ADC_Slope + fDAC_0;

    if (dacsetting > fDAC_max)
      dacsetting = fDAC_max;

    if (dacsetting < 0)
      dacsetting = 0;
  }

  //std::cout << "CalculateDACValue: dacsetting=" << dacsetting << ", adctarget=" << adctarget << ", permille="
  //    << ADC_permille << std::endl;

  return (int) dacsetting;
}

int GainSetup::CalculateADCPermille (double DAC_value)
{
  double adctarget;
  if (fDAC_ADC_Slope < 0)
  {
    // gain 16 or gain32
    if (DAC_value < fDAC_min)
      adctarget = AWAGS_ADC_MAXSATURATION;
    //else if(DAC_value>=AWAGS_DAC_MAXVALUE)
    else if (DAC_value >= fDAC_max)
      adctarget = fADC_min;
    else
      adctarget = (DAC_value - fDAC_0) / fDAC_ADC_Slope;

    if (adctarget < 0)
      adctarget = 0;
    if (adctarget > AWAGS_ADC_MAXVALUE)
      adctarget = AWAGS_ADC_MAXVALUE;

  }
  else
  {
    // gain 1
    if (DAC_value < fDAC_min)
      adctarget = fADC_min;
    else if (DAC_value >= fDAC_max)
      adctarget = AWAGS_ADC_MAXSATURATION;
    else
      adctarget = (DAC_value - fDAC_0) / fDAC_ADC_Slope;
    if (adctarget < 0)
      adctarget = 0;
    if (adctarget > AWAGS_ADC_MAXVALUE)
      adctarget = AWAGS_ADC_MAXVALUE;
  }
  double adcpermille = 1000.0 * adctarget / AWAGS_ADC_MAXVALUE;

  // shift ADC_min to zero of slider:
  //double adcpermille= 1000.0 * (adctarget-fADC_min)/AWAGS_ADC_MAXVALUE;
  //std::cout << "CalculateADCPermille: adctarget="<<adctarget<<", value="<<DAC_value<<", permille="<<adcpermille<< std::endl;
     return adcpermille;

  }


 /** calculate and set calibration curve for measured variations deltaADC and deltaDAC around
  * autocalibrated DAC setting valDAC*/
 void GainSetup::EvaluateCalibration(double deltaDAC, double deltaADC, double valDAC, double valADC)
 {
   if(deltaADC==0)deltaADC=1;
   fDAC_ADC_Slope= deltaDAC/deltaADC;
   fDAC_0 = valADC * fDAC_ADC_Slope + valDAC;
   printm("EvaluateCalibration(dDAC=%f, dADC=%f, DAC1=%f, ADC1=%f",deltaDAC,deltaADC,valDAC,valADC);
   DumpCalibration();
 }

