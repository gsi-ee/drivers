#include "AwagsTestResults.h"


 AwagsTestResults::AwagsTestResults()
  {
    //std::cout<< "TTTT Ctor ApfrelTestResults object"<< std::endl;
    Reset();
  }

  void AwagsTestResults::Reset(bool invertedslope)
  {
    fValid=false;
    fAddressID=0;
    fChipLabel="";
    for(int dac=0; dac<AWAGS_NUMDACS; ++dac)
        {
          fDACValueCalibrate[dac]=0;
          fBaselineLowerBound[dac]=AWAGS_NOVALUE;
          fBaselineUpperBound[dac]=AWAGS_NOVALUE;
          fMean[dac]=AWAGS_NOVALUE;
          fSigma[dac]=AWAGS_NOVALUE;
          fMinValue[dac]=0;
          fMaxValue[dac]=0;
          fDAC_ADC_Gain[dac].ResetCalibration(!invertedslope);
          fDAC_Curve[dac].Reset();
//          fPeaks[dac].clear();
//          fNegativePeaks[dac]=false;


        }
  }

   void AwagsTestResults::Begin()
   {
     Reset();
     fStartTime=QDateTime::currentDateTime();

   }

   void AwagsTestResults::Finish()
   {
     SetValid();
     fEndTime=QDateTime::currentDateTime();

   }

   QString AwagsTestResults::GetStartTime()
   {
     return fStartTime.toString(AWAGS_RESULT_TIMEFORMAT);
   }

    QString AwagsTestResults::GetEndTime()
    {
      return fEndTime.toString(AWAGS_RESULT_TIMEFORMAT);
    }






  void AwagsTestResults::SetAddressId(uint8_t ad)
  {
    fAddressID=ad;
  }

  int AwagsTestResults::GetAddressId()
  {
     return fAddressID;
   }

  int AwagsTestResults::SetDacValueCalibrate(int dac, uint16_t val)
  {
    ASSERT_DAC_VALID(dac);
    fDACValueCalibrate[dac]=val;
    return 0;
  }

  int AwagsTestResults::GetDacValueCalibrate(int dac, bool checkvalid)
  {
    ASSERT_TEST_VALID
    ASSERT_DAC_VALID(dac)
    return fDACValueCalibrate[dac];
  }



  int AwagsTestResults::SetAdcBaselineLowerBound(int dac, int val)
  {
    ASSERT_DAC_VALID(dac);
    fBaselineLowerBound[dac]=val;
    return 0;
  }

   int AwagsTestResults::SetAdcBaselineUpperBound(int dac, int val)
   {
     ASSERT_DAC_VALID(dac);
     fBaselineUpperBound[dac]=val;
     return 0;
   }

   int AwagsTestResults::GetAdcBaselineLowerBound(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID
     ASSERT_DAC_VALID(dac)
     return fBaselineLowerBound[dac];

   }

   int AwagsTestResults::GetAdcBaselineUpperBound(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID
     ASSERT_DAC_VALID(dac)
     return fBaselineUpperBound[dac];

   }



  int AwagsTestResults::SetAdcSampleMean(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fMean[dac]=val;
    return 0;
  }

  double AwagsTestResults::GetAdcSampleMean(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID;
     ASSERT_DAC_VALID(dac);
     return fMean[dac];
   }
  int AwagsTestResults::SetAdcSampleSigma(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fSigma[dac]=val;
    return 0;
  }

  double AwagsTestResults::GetAdcSampleSigma(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID;
     ASSERT_DAC_VALID(dac);
     return fSigma[dac];
   }


  int AwagsTestResults::SetAdcSampleMinimum(int dac, uint16_t val)
    {
      ASSERT_DAC_VALID(dac);
      fMinValue[dac]=val;
      return 0;
    }

  int AwagsTestResults::GetAdcSampleMinimum(int dac, bool checkvalid)
     {
        ASSERT_TEST_VALID
        ASSERT_DAC_VALID(dac);
        return fMinValue[dac];
     }


  int AwagsTestResults::SetAdcSampleMaximum(int dac, uint16_t val)
     {
       ASSERT_DAC_VALID(dac);
       fMaxValue[dac]=val;
       return 0;
     }

   int AwagsTestResults::GetAdcSampleMaximum(int dac, bool checkvalid)
      {
       ASSERT_TEST_VALID;
       ASSERT_DAC_VALID(dac);
       return fMaxValue[dac];
      }

int AwagsTestResults::SetGainParameter(int dac, GainSetup structure)
{
  ASSERT_DAC_VALID(dac);
  fDAC_ADC_Gain[dac]=structure; // hope for default assignment operator!
  return 0;
}

double AwagsTestResults::GetSlope (int dac, bool checkvalid)
{
  ASSERT_TEST_VALID;
  ASSERT_DAC_VALID(dac);
  return fDAC_ADC_Gain[dac].GetSlope();
}

 double AwagsTestResults::GetD0 (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID;
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetD0();
 }

 double AwagsTestResults::GetDACmin (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetDACmin();

 }
 double AwagsTestResults::GetDACmax (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetDACmax();

 }

 double AwagsTestResults::GetADCmin (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetADCmin();

 }


 int AwagsTestResults::AddDacSample(int dac, uint16_t valdac, uint16_t valadc)
  {
     ASSERT_DAC_VALID(dac);
     fDAC_Curve[dac].AddSample(valdac, valadc);
     return 0;
  }

 int AwagsTestResults::AccessDacSample(DacSample& samp, int dac, int ix)
 {
   ASSERT_DAC_VALID(dac);
   if(ix>=fDAC_Curve[dac].NumSamples()) return -2;
   samp=fDAC_Curve[dac].GetSample(ix);
   return 0;
 }

 int AwagsTestResults::ResetDacSample(int dac)
 {
   ASSERT_DAC_VALID(dac);
   fDAC_Curve[dac].Reset();
   return 0;

 }

// int AwagsTestResults::AddAdcPeak(int dac, int position, uint16_t height)
// {
//     ASSERT_DAC_VALID(dac);
//     fPeaks[dac].push_back(AdcPeak(position, height));
//     return 0;
// }
//
//  int AwagsTestResults::ResetAdcPeaks(int dac)
//  {
//    ASSERT_DAC_VALID(dac);
//    fPeaks[dac].clear();
//  }
//
//  int AwagsTestResults::NumAdcPeaks(int dac)
//  {
//    ASSERT_DAC_VALID(dac);
//    return fPeaks[dac].size();
//  }
//
//  int AwagsTestResults::GetAdcPeakPosition(int dac, int index,bool checkvalid)
//  {
//    ASSERT_TEST_VALID;
//    ASSERT_DAC_VALID(dac);
//    if(index>=fPeaks[dac].size()) return AWAGS_NOVALUE;
//    AdcPeak& peak=fPeaks[dac].at(index);
//    return peak.fPosition;
//  }
//
//
//   int AwagsTestResults::GetAdcPeakHeight(int dac, int index, bool checkvalid)
//   {
//     ASSERT_TEST_VALID
//     ASSERT_DAC_VALID(dac);
//     if(index>=fPeaks[dac].size()) return AWAGS_NOVALUE;
//     AdcPeak& peak=fPeaks[dac].at(index);
//     return peak.fHeight;
//   }
//
//    bool AwagsTestResults::HasNegativeAdcPeaks(int dac)
//    {
//      ASSERT_DAC_VALID(dac);
//      return fNegativePeaks[dac];
//    }
//
//    int AwagsTestResults::SetNegativeAdcPeaks(int dac, bool negative)
//    {
//      ASSERT_DAC_VALID(dac);
//      fNegativePeaks[dac]=negative;
//      return 0;
//    }

