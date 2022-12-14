#include "BoardSetup.h"

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
//////// the whole slave board setup:

BoardSetup::BoardSetup () : GosipSetup(),
 fPandaTestBoard(false),fUseApfel (true), fHighGainOutput (true), fStretcher (false), fRegularMapping (true), fBaselineInverted(true)
{

  for (int i = 0; i < APFEL_NUMCHIPS; ++i)
  {
    fTestResults[i][1] = ApfelTestResults ();
    fTestResults[i][16] = ApfelTestResults ();
    fTestResults[i][32] = ApfelTestResults ();
  }

  for (int c = 0; c < APFEL_ADC_CHANNELS; ++c)
  {
    fGainSetups[c][1] = GainSetup ();
    fGainSetups[c][1].ResetCalibration(!fBaselineInverted); // consistent with default state of board setup
    fGainSetups[c][16] = GainSetup ();
    fGainSetups[c][16].ResetCalibration(!fBaselineInverted);
    fGainSetups[c][32] = GainSetup ();
    fGainSetups[c][32].ResetCalibration(!fBaselineInverted);
  }

  SetApfelMapping (true);
#ifdef APFEL_GAIN1_INVERTED
  ResetGain1Calibration ();
#endif
}

void BoardSetup::SetApfelMapping (bool regular, bool pandatest)
{
  //std::cout << "SetApfelMapping("<<regular<<","<< pandatest<<"):"<< std::endl;
  fRegularMapping = regular;
  for (int i = 0; i < APFEL_NUMCHIPS; ++i)
  {
    uint8_t add = 0;

    if (pandatest)
    {
      // PANDA test board: ignore regular and inverted mapping. Default setup is just ids 1-8 in ascending order
      add=i;
    }
    else
    {
      // apfelsem with optionally inverted addresses on mezzanine boards:
      if (i < 4)
      {
        // regular mapping: indices 0..3 before 8...11
        add = (regular ? i : i + 8);
      }
      else
      {
        add = (regular ? i + 4 : i - 4);
      }
    }

    fApfel[i].SetAddressID (add + 1);    // shift to id number 1...12 already here!
    //std::cout << "  APFEL["<<i<<"] <- "<<add+1<< std::endl;
  }

}

ApfelTestResults& BoardSetup::AccessTestResults (int gain, int apfel)
{
  return fTestResults[apfel].at (gain);
}

GainSetup& BoardSetup::AccessGainSetup (int gain, int febexchannel)
{
  return fGainSetups[febexchannel].at (gain);
}

void BoardSetup::ResetGain1Calibration ()
{
  // workaround to account inverse polarity of gain 1 dac-adc by default
  for (int ch = 0; ch < APFEL_ADC_CHANNELS; ++ch)
  {
    //fGainSetups[ch].at (1).ResetCalibration (false);
    fGainSetups[ch].at (1).ResetCalibration (fBaselineInverted); // 2017: take into account inverted state?
  }
}

int BoardSetup::SetDACmin (int gain, int febexchannel, double val)
{
  ASSERT_FEBCHAN_VALID(febexchannel);

  fGainSetups[febexchannel].at (gain).SetDACmin (val);
  return 0;

}

int BoardSetup::SetDACmax (int gain, int febexchannel, double val)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  fGainSetups[febexchannel].at (gain).SetDACmax (val);
  return 0;
}

int BoardSetup::SetADCmin (int gain, int febexchannel, double val)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "EvaluateCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
  fGainSetups[febexchannel].at (gain).SetADCmin (val);
  return 0;
}

int BoardSetup::EvaluateCalibration (int gain, int febexchannel, double deltaDAC, double deltaADC, double valDAC,
    double valADC)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "EvaluateCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
  fGainSetups[febexchannel].at (gain).EvaluateCalibration (deltaDAC, deltaADC, valDAC, valADC);
  return 0;
}

int BoardSetup::ResetCalibration (int gain, int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "ResetCalibration for channel "<<febexchannel<<", gain:"<< gain <<", baselineinvertd:"<<fBaselineInverted<< std::endl;

#ifdef APFEL_GAIN1_INVERTED
  if (gain == 1)
    fGainSetups[febexchannel].at (gain).ResetCalibration (fBaselineInverted);
  else
    fGainSetups[febexchannel].at (gain).ResetCalibration (!fBaselineInverted);
#else
  fGainSetups[febexchannel].at(gain).ResetCalibration(!fBaselineInverted);
#endif
  return 0;
}

int BoardSetup::DumpCalibration (int gain, int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  printm ("DumpCalibration for channel %d gain %d:\t", febexchannel, gain);
  fGainSetups[febexchannel].at (gain).DumpCalibration ();
  return 0;
}

int BoardSetup::CalculateDACValue (int gain, int febexchannel, double ADC_permille)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "BoardSetup::GetDACValue for gain:"<<gain<<", channel:"<<febexchannel<<std::endl;
  int rev = 0;
  rev = fGainSetups[febexchannel].at (gain).CalculateDACValue (ADC_permille);
  return rev;
}

int BoardSetup::CalculateADCPermille (int gain, int febexchannel, double DAC_value)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  int rev = 0;
  rev = fGainSetups[febexchannel].at (gain).CalculateADCPermille (DAC_value);
  return rev;

}

/** convert febex channel to DAC indices*/
void BoardSetup::EvaluateDACIndices (int febexchannel, int& apfel, int& dac)
{
  // this function is used for automatic baseline adjustments
  // not so straighforward to use:
  // DAC1 (dac==0): acts on ch0 when 16/32 gain set
  // DAC2 (dac==1): acts on ch1 when 16/32 gain set
  // DAC3 (dac==2): acts both on ch0 and ch1 when 1 gain set
  // DAC4 (dac==3): acts with low gain both on ch0 and ch1 for both gain settings

  apfel = febexchannel / APFEL_NUMCHANS;
  if (fHighGainOutput)
  {
    // use first 2 dacs for baseline adjustment if set to high gain:
    dac = febexchannel - apfel * APFEL_NUMCHANS;
  }
  else
  {
    // for the moment we always use DAC3 only for gain 1
    dac = 2;    // DAC3 with index 2
  }

  // TODO: take into account DAC4 when regulating the low gain case

}

int BoardSetup::EvaluateADCChannel (int apfel, int dac)
{
  int chan = apfel * APFEL_NUMCHANS;
  if (fHighGainOutput)
  {
    if (dac < APFEL_NUMCHANS)
      chan += dac;
    else
      chan = -1;    // mark dac as invalid for adc
  }
  else
  {
    if (dac != 2)
      chan = -1;       // not sufficient! dac2 works on both adc channels...
  }
  return chan;
}

/** get absolute DAC setting from relative baseline slider*/
int BoardSetup::EvaluateDACvalueAbsolute (int permillevalue, int febexchannel, int gain)
{
  //std::cout<<"EvaluateDACvalueAbsolute for gain:"<<gain<<", channel:"<<febexchannel << std::endl;
  int value = 0;

  if(fBaselineInverted)
  {
    value = APFEL_DAC_MAXVALUE - round ((permillevalue * ((double) APFEL_DAC_MAXVALUE) / 1000.0));
  }
  else
  {
    value = round ((permillevalue * ((double) APFEL_DAC_MAXVALUE) / 1000.0));
  }

  // default: linear interpolation of DAC for complete slider range, note inverted DAC polarity effect on baseline
  if (febexchannel >= 0)
  {
    // if channel specified, use calibration from measurements, inverted baseline was already applied
    value = CalculateDACValue (gain, febexchannel, permillevalue);
  }
  return value;
}

/** get relative ADC slider value from given dac setting*/
int BoardSetup::EvaluateADCvaluePermille (int value, int febexchannel, int gain)
{
  int permille=0;

  if (fBaselineInverted)
  {
    int permille = 1000 - round (1000.0 * ((double) value / (double) APFEL_DAC_MAXVALUE));
  }
  else
  {
    int permille = round (1000.0 * ((double) value / (double) APFEL_DAC_MAXVALUE));
  }

  // default: linear interpolation of DAC for complete slider range, note inverted DAC polarity effect on baseline
  if (febexchannel >= 0)
  {
    // if channel specified, use calibration from measurements, inverted baseline was already applied
    permille = CalculateADCPermille (gain, febexchannel, value);
  }
  return permille;
}

bool  BoardSetup::IsApfelPresent (int apfel)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].IsPresent();
}

int BoardSetup::SetApfelPresent (int apfel, bool on)
{
  ASSERT_APFEL_VALID(apfel);
  return (fApfel[apfel].SetPresent(on));
}


bool BoardSetup::HasApfelPower (int apfel)
  {
    ASSERT_APFEL_VALID(apfel);
    return fApfel[apfel].HasPower();
  }

int BoardSetup::SetApfelPowered (int apfel, bool on)
  {
    ASSERT_APFEL_VALID(apfel);
    return fApfel[apfel].SetPower(on);
  }


int BoardSetup::SetChipID (int apfel, const QString& val)
{
  ASSERT_APFEL_VALID(apfel);
  return (fApfel[apfel].SetChipID(val));
}

int BoardSetup::GetChipID (int apfel, QString& val)
{
  ASSERT_APFEL_VALID(apfel);
  val=fApfel[apfel].GetChipID();
  return 0;
}



int  BoardSetup::SetCurrentASIC(int apfel, double val)
{
  ASSERT_APFEL_VALID(apfel);
  fApfel[apfel].SetCurrentASIC(val);
  return 0;
}

double BoardSetup::GetCurrentASIC(int apfel)
{
  ASSERT_APFEL_VALID(apfel)
  return (fApfel[apfel].GetCurrentASIC());
}

int  BoardSetup::SetCurrentHV(int apfel, double val)
{
  ASSERT_APFEL_VALID(apfel);
  fApfel[apfel].SetCurrentHV(val);
   return 0;
}

double  BoardSetup::GetCurrentHV(int apfel)
{
  ASSERT_APFEL_VALID(apfel);
  return (fApfel[apfel].GetCurrentHV());
}

int  BoardSetup::SetCurrentDiode(int apfel, double val)
{
  ASSERT_APFEL_VALID(apfel);
  fApfel[apfel].SetCurrentDiode(val);
  return 0;
}
double  BoardSetup::GetCurrentDiode(int apfel)
{
    ASSERT_APFEL_VALID(apfel);
    return (fApfel[apfel].GetCurrentDiode());
 }


int  BoardSetup::IsIDScanOK(int apfel)
{
  ASSERT_APFEL_VALID(apfel);
  return (fApfel[apfel].IsIDScanOK() ? 1 :0);

}
 int  BoardSetup::IsGeneralScanOK(int apfel)
 {
   ASSERT_APFEL_VALID(apfel);
     return (fApfel[apfel].IsGeneralCallScanOK() ? 1 :0);
 }
 int  BoardSetup::IsReverseIDScanOK(int apfel)
 {
   ASSERT_APFEL_VALID(apfel);
   return (fApfel[apfel].IsReverseIDScanOK() ? 1 :0);
 }


 int  BoardSetup::IsRegisterScanOK(int apfel)
 {
   ASSERT_APFEL_VALID(apfel);
   return (fApfel[apfel].IsRegisterScanOK() ? 1 :0);
 }

 int  BoardSetup::SetIDScan(int apfel, bool ok)
 {
   ASSERT_APFEL_VALID(apfel);
   fApfel[apfel].SetIDScan(ok);
   return 0;
 }

 int  BoardSetup::SetGeneralScan(int apfel, bool ok)
 {
   ASSERT_APFEL_VALID(apfel);
     fApfel[apfel].SetGeneralCallScan(ok);
     return 0;
 }

 int  BoardSetup::SetReverseIDScan(int apfel, bool ok)
 {
   ASSERT_APFEL_VALID(apfel);
   fApfel[apfel].SetReverseIDScan(ok);
   return 0;
 }

 int  BoardSetup::SetRegisterScan(int apfel, bool ok)
 {
   ASSERT_APFEL_VALID(apfel);
   fApfel[apfel].SetRegisterScan(ok);
   return 0;
 }



int BoardSetup::GetDACValue (int apfel, int dac)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].GetDACValue (dac);
}

int BoardSetup::SetDACValue (int apfel, int dac, uint16_t value)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].SetDACValue (dac, value);
}

/** helper function to access DAC value via global febex channel number*/
int BoardSetup::GetDACValue (int febexchannel)
{
  int chip = 0, chan = 0;
  EvaluateDACIndices (febexchannel, chip, chan);
  return GetDACValue (chip, chan);
}

/** helper function to set DAC value via global febex channel number*/
int BoardSetup::SetDACValue (int febexchannel, uint16_t value)
{
  int chip = 0, chan = 0;
  EvaluateDACIndices (febexchannel, chip, chan);
  return SetDACValue (chip, chan, value);
}

int BoardSetup::SetLowGain (int apfel, int chan, bool low)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].SetLowGain (chan, low);

}

int BoardSetup::GetLowGain (int apfel, int chan)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].GetLowGain (chan);
}

int BoardSetup::SetTestPulseEnable (int apfel, int chan, bool on)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].SetTestPulseEnable (chan, on);
}

int BoardSetup::GetTestPulseEnable (int apfel, int chan)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].GetTestPulseEnable (chan);
}

int BoardSetup::SetTestPulsePostive (int apfel, bool pos)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].SetTestPulsePostive (pos);
}

int BoardSetup::GetTestPulsePositive (int apfel)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].GetTestPulsePositive ();
}

int BoardSetup::SetTestPulseAmplitude (int apfel, int chan, uint8_t val)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].SetTestPulseAmplitude (chan, val);

}

int BoardSetup::GetTestPulseAmplitude (int apfel, int chan)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].GetTestPulseAmplitude (chan);
}

int BoardSetup::GetApfelID (int apfel)
{
  ASSERT_APFEL_VALID(apfel);
  return fApfel[apfel].GetAddressID ();

}

/** evaluate gain factor from setup. returns 1, 16 or 32*/
int BoardSetup::GetGain (int apfel, int dac)
{
  int gain = 0;
  if (!IsHighGain ())
  {
    gain = 1;
  }
  else
  {
    if (GetLowGain (apfel, dac))    // for high gain, apfel channel index is same as dac index
      gain = 16;
    else
      gain = 32;
  }
  return gain;
}

int BoardSetup::ResetADCSample (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  fLastSample[febexchannel].Reset ();
  return 0;
}

int BoardSetup::AddADCSample (int febexchannel, uint16_t value)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  fLastSample[febexchannel].AddSample (value);
  return 0;
}

uint16_t BoardSetup::GetADCSample (int febexchannel, int index)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetSample (index);
}

int BoardSetup::GetADCSampleLength (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetNumSamples();
}

int BoardSetup::EvaluatePeaks(int febexchannel, double deltaratio, double falldistance, bool negative)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  fLastSample[febexchannel].FindPeaks(deltaratio, falldistance, negative);
  return 0;
}


uint16_t BoardSetup::GetSamplePeakHeight (int febexchannel, int num)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetPeakHeight (num);
}

int BoardSetup::GetSamplePeakPosition (int febexchannel, int num)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetPeakPosition (num);
}

int BoardSetup::NumSamplePeaks (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetNumPeaks ();
}

 bool  BoardSetup::IsSamplePeaksNegative(int febexchannel)
 {
   ASSERT_FEBCHAN_VALID(febexchannel);
   return fLastSample[febexchannel].IsNegativePeaks();
 }

 double  BoardSetup::GetSamplePeaksHeightDelta(int febexchannel)
 {
   ASSERT_FEBCHAN_VALID(febexchannel);
   return fLastSample[febexchannel].GetHeightDelta();
 }

 double  BoardSetup::GetSamplePeaksPositionDelta(int febexchannel)
 {
   ASSERT_FEBCHAN_VALID(febexchannel);
   return fLastSample[febexchannel].GetPosDelta();
 }


 int BoardSetup::EvaluateBaseline(int febexchannel, int firstindex, int lastindex)
  {
    ASSERT_FEBCHAN_VALID(febexchannel);
    fLastSample[febexchannel].SetBaselineStartIndex(firstindex);
    fLastSample[febexchannel].SetBaselineStopIndex(lastindex);
    fLastSample[febexchannel].CalculateMeanAndSigma();
    return 0;
  }



int BoardSetup::GetADCBaslineLowerBound(int febexchannel)
{
   ASSERT_FEBCHAN_VALID(febexchannel);
   return fLastSample[febexchannel].GetBaselineStartIndex();
}

int BoardSetup::GetADCBaslineUpperBound(int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetBaselineStopIndex();
}


double BoardSetup::GetADCMean (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetMean ();
}

double BoardSetup::GetADCSigma (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetSigma ();
}

double BoardSetup::GetADCMiminum (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetMinimum ();
}

double BoardSetup::GetADCMaximum (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  return fLastSample[febexchannel].GetMaximum ();
}

int BoardSetup::DumpADCSamplePars (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  fLastSample[febexchannel].DumpParameters (febexchannel);
  return 0;
}

int BoardSetup::ShowADCSample (int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  fLastSample[febexchannel].ShowSample (febexchannel);
  return 0;
}

