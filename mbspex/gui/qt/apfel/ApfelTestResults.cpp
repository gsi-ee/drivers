#include "ApfelTestResults.h"


 ApfelTestResults::ApfelTestResults()
  {
    Reset();
  }

  void ApfelTestResults::Reset()
  {
    //fAddressID=0;
    for(int dac=0; dac<APFEL_NUMDACS; ++dac)
        {
          fDACValueCalibrate[dac]=0;
          fMean[dac]=0;
          fSigma[dac]=0;
          fMinValue[dac]=0;
          fMaxValue[dac]=0;
          fDAC_ADC_Gain[dac].ResetCalibration();
          fDAC_Curve[dac].Reset();
        }
  }
//
//  void SetAddressId(uint8_t ad)
//  {
//    fAddressID=ad;
//  }
//
//  uint8_t GetAddressId()
//  {
//     return fAddressID;
//   }

  int ApfelTestResults::SetDACValueCalibrate(int dac, uint16_t val)
  {
    ASSERT_DAC_VALID(dac);
    fDACValueCalibrate[dac]=val;
    return 0;
  }

  uint16_t ApfelTestResults::GetDACValueCalibrate(int dac)
  {
    ASSERT_DAC_VALID(dac)
    return fDACValueCalibrate[dac];
  }

  int ApfelTestResults::SetDACSampleMean(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fMean[dac]=val;
    return 0;
  }

  double ApfelTestResults::GetDACSampleMean(int dac)
   {
     ASSERT_DAC_VALID(dac);
     return fMean[dac];
   }
  int ApfelTestResults::SetDACSampleSigma(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fSigma[dac]=val;
    return 0;
  }

  double ApfelTestResults::GetDACSampleSigma(int dac)
   {
     ASSERT_DAC_VALID(dac);
     return fSigma[dac];
   }


  int ApfelTestResults::SetDACSampleMinimum(int dac, uint16_t val)
    {
      ASSERT_DAC_VALID(dac);
      fMinValue[dac]=val;
      return 0;
    }

  uint16_t ApfelTestResults::GetDACSampleMinimum(int dac)
     {
    ASSERT_DAC_VALID(dac);
    return fMinValue[dac];
     }


  int ApfelTestResults::SetDACSampleMaxium(int dac, uint16_t val)
     {
       ASSERT_DAC_VALID(dac);
       fMaxValue[dac]=val;
       return 0;
     }

   uint16_t ApfelTestResults::GetDACSampleMaximum(int dac)
      {
     ASSERT_DAC_VALID(dac);
     return fMaxValue[dac];
      }

int ApfelTestResults::SetGainParameter(int dac, GainSetup structure)
{
  ASSERT_DAC_VALID(dac);
  fDAC_ADC_Gain[dac]=structure; // hope for default assignment operator!
  return 0;
}

double ApfelTestResults::GetSlope (int dac)
{
  ASSERT_DAC_VALID(dac);
  return fDAC_ADC_Gain[dac].GetSlope();
}

 double ApfelTestResults::GetD0 (int dac)
 {
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetD0();
 }

 double ApfelTestResults::GetDACmin (int dac)
 {
   ASSERT_DAC_VALID(dac);
      return fDAC_ADC_Gain[dac].GetDACmin();

 }
 double ApfelTestResults::GetDACmax (int dac)
 {
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetDACmax();

 }

 double ApfelTestResults::GetADCmin (int dac)
 {
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetADCmin();

 }


 int ApfelTestResults::AddDacSample(int dac, uint16_t valdac, uint16_t valadc)
  {
     ASSERT_DAC_VALID(dac);
     fDAC_Curve[dac].AddSample(valdac, valadc);
     return 0;
  }

 int ApfelTestResults::AccessDacSample(DacSample& samp, int dac, int ix)
 {
   ASSERT_DAC_VALID(dac);
   if(ix>=fDAC_Curve[dac].NumSamples()) return -2;
   samp=fDAC_Curve[dac].GetSample(ix);
   return 0;
 }

 int ApfelTestResults::ResetDacSample(int dac)
 {
   ASSERT_DAC_VALID(dac);
   fDAC_Curve[dac].Reset();
   return 0;

 }

