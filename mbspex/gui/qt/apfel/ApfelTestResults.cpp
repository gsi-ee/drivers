#include "ApfelTestResults.h"


 ApfelTestResults::ApfelTestResults()
  {
    Reset();
  }

  void ApfelTestResults::Reset()
  {
    fValid=false;
    fAddressID=0;
    fBoardLabel="NONE";
    for(int dac=0; dac<APFEL_NUMDACS; ++dac)
        {
          fDACValueCalibrate[dac]=0;
          fBaselineLowerBound[dac]=APFEL_NOVALUE;
          fBaselineUpperBound[dac]=APFEL_NOVALUE;
          fMean[dac]=APFEL_NOVALUE;
          fSigma[dac]=APFEL_NOVALUE;
          fMinValue[dac]=0;
          fMaxValue[dac]=0;
          fDAC_ADC_Gain[dac].ResetCalibration();
          fDAC_Curve[dac].Reset();
          fPeaks[dac].clear();
          fNegativePeaks[dac]=false;


        }
  }

   void ApfelTestResults::Begin()
   {
     Reset();
     fStartTime=QDateTime::currentDateTime();

   }

   void ApfelTestResults::Finish()
   {
     SetValid();
     fEndTime=QDateTime::currentDateTime();

   }

   QString ApfelTestResults::GetStartTime()
   {
     return fStartTime.toString(APFEL_RESULT_TIMEFORMAT);
   }

    QString ApfelTestResults::GetEndTime()
    {
      return fEndTime.toString(APFEL_RESULT_TIMEFORMAT);
    }


  void ApfelTestResults::SetBoardDescriptor(const std::string& label)
  {
    fBoardLabel=label;
  }

  const std::string& ApfelTestResults::GetBoardDescriptor()
    {
      return fBoardLabel;
    }


  void ApfelTestResults::SetAddressId(uint8_t ad)
  {
    fAddressID=ad;
  }

  int ApfelTestResults::GetAddressId()
  {
     return fAddressID;
   }

  int ApfelTestResults::SetDacValueCalibrate(int dac, uint16_t val)
  {
    ASSERT_DAC_VALID(dac);
    fDACValueCalibrate[dac]=val;
    return 0;
  }

  int ApfelTestResults::GetDacValueCalibrate(int dac, bool checkvalid)
  {
    ASSERT_TEST_VALID
    ASSERT_DAC_VALID(dac)
    return fDACValueCalibrate[dac];
  }



  int ApfelTestResults::SetAdcBaselineLowerBound(int dac, int val)
  {
    ASSERT_DAC_VALID(dac);
    fBaselineLowerBound[dac]=val;
    return 0;
  }

   int ApfelTestResults::SetAdcBaselineUpperBound(int dac, int val)
   {
     ASSERT_DAC_VALID(dac);
     fBaselineUpperBound[dac]=val;
     return 0;
   }

   int ApfelTestResults::GetAdcBaselineLowerBound(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID
     ASSERT_DAC_VALID(dac)
     return fBaselineLowerBound[dac];

   }

   int ApfelTestResults::GetAdcBaselineUpperBound(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID
     ASSERT_DAC_VALID(dac)
     return fBaselineUpperBound[dac];

   }



  int ApfelTestResults::SetAdcSampleMean(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fMean[dac]=val;
    return 0;
  }

  double ApfelTestResults::GetAdcSampleMean(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID;
     ASSERT_DAC_VALID(dac);
     return fMean[dac];
   }
  int ApfelTestResults::SetAdcSampleSigma(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fSigma[dac]=val;
    return 0;
  }

  double ApfelTestResults::GetAdcSampleSigma(int dac, bool checkvalid)
   {
     ASSERT_TEST_VALID;
     ASSERT_DAC_VALID(dac);
     return fSigma[dac];
   }


  int ApfelTestResults::SetAdcSampleMinimum(int dac, uint16_t val)
    {
      ASSERT_DAC_VALID(dac);
      fMinValue[dac]=val;
      return 0;
    }

  int ApfelTestResults::GetAdcSampleMinimum(int dac, bool checkvalid)
     {
        ASSERT_TEST_VALID
        ASSERT_DAC_VALID(dac);
        return fMinValue[dac];
     }


  int ApfelTestResults::SetAdcSampleMaximum(int dac, uint16_t val)
     {
       ASSERT_DAC_VALID(dac);
       fMaxValue[dac]=val;
       return 0;
     }

   int ApfelTestResults::GetAdcSampleMaximum(int dac, bool checkvalid)
      {
       ASSERT_TEST_VALID;
       ASSERT_DAC_VALID(dac);
       return fMaxValue[dac];
      }

int ApfelTestResults::SetGainParameter(int dac, GainSetup structure)
{
  ASSERT_DAC_VALID(dac);
  fDAC_ADC_Gain[dac]=structure; // hope for default assignment operator!
  return 0;
}

double ApfelTestResults::GetSlope (int dac, bool checkvalid)
{
  ASSERT_TEST_VALID;
  ASSERT_DAC_VALID(dac);
  return fDAC_ADC_Gain[dac].GetSlope();
}

 double ApfelTestResults::GetD0 (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID;
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetD0();
 }

 double ApfelTestResults::GetDACmin (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetDACmin();

 }
 double ApfelTestResults::GetDACmax (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetDACmax();

 }

 double ApfelTestResults::GetADCmin (int dac, bool checkvalid)
 {
   ASSERT_TEST_VALID
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

 int ApfelTestResults::AddAdcPeak(int dac, int position, uint16_t height)
 {
     ASSERT_DAC_VALID(dac);
     fPeaks[dac].push_back(AdcPeak(position, height));
     return 0;
 }

  int ApfelTestResults::ResetAdcPeaks(int dac)
  {
    ASSERT_DAC_VALID(dac);
    fPeaks[dac].clear();
  }

  int ApfelTestResults::NumAdcPeaks(int dac)
  {
    ASSERT_DAC_VALID(dac);
    return fPeaks[dac].size();
  }

  int ApfelTestResults::GetAdcPeakPosition(int dac, int index,bool checkvalid)
  {
    ASSERT_TEST_VALID;
    ASSERT_DAC_VALID(dac);
    if(index>=fPeaks[dac].size()) return APFEL_NOVALUE;
    AdcPeak& peak=fPeaks[dac].at(index);
    return peak.fPosition;
  }


   int ApfelTestResults::GetAdcPeakHeight(int dac, int index, bool checkvalid)
   {
     ASSERT_TEST_VALID
     ASSERT_DAC_VALID(dac);
     if(index>=fPeaks[dac].size()) return APFEL_NOVALUE;
     AdcPeak& peak=fPeaks[dac].at(index);
     return peak.fHeight;
   }

    bool ApfelTestResults::HasNegativeAdcPeaks(int dac)
    {
      ASSERT_DAC_VALID(dac);
      return fNegativePeaks[dac];
    }

    int ApfelTestResults::SetNegativeAdcPeaks(int dac, bool negative)
    {
      ASSERT_DAC_VALID(dac);
      fNegativePeaks[dac]=negative;
      return 0;
    }

