#include "BoardSetup.h"

/////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
//////// the whole slave board setup:

BoardSetup::BoardSetup () : GosipSetup(),
 fProtoBoard(false), fUseAwags (true)
{
  //std::cout<<"CTOR  BoardSetup..." << std:endl;


  for (int i = 0; i < AWAGS_NUMCHIPS; ++i)
  {
    fTestResults[i][1] = AwagsTestResults ();
    fTestResults[i][2] = AwagsTestResults ();
    fTestResults[i][4] = AwagsTestResults ();
    fTestResults[i][8] = AwagsTestResults ();
  }

  for (int c = 0; c < AWAGS_ADC_CHANNELS; ++c)
  {

    fGainSetups[c][1] = GainSetup ();
    fGainSetups[c][1].ResetCalibration(true); // consistent with default state of board setup
    fGainSetups[c][2] = GainSetup ();
    fGainSetups[c][2].ResetCalibration(true);
    fGainSetups[c][4] = GainSetup ();
    fGainSetups[c][4].ResetCalibration(true);
    fGainSetups[c][8] = GainSetup ();
        fGainSetups[c][8].ResetCalibration(true);
  }

//  SetAwagsMapping (true);
//#ifdef AWAGS_GAIN1_INVERTED
//  ResetGain1Calibration ();
//#endif

}



AwagsTestResults& BoardSetup::AccessTestResults (int gain, int awags)
{
  return fTestResults[awags].at (gain);
}

GainSetup& BoardSetup::AccessGainSetup (int gain, int febexchannel)
{
  return fGainSetups[febexchannel].at (gain);
}

//void BoardSetup::ResetGain1Calibration ()
//{
//  // workaround to account inverse polarity of gain 1 dac-adc by default
//  for (int ch = 0; ch < AWAGS_ADC_CHANNELS; ++ch)
//  {
//    //fGainSetups[ch].at (1).ResetCalibration (false);
//    //fGainSetups[ch].at (1).ResetCalibration (fBaselineInverted); // 2017: take into account inverted state?
//    AccessGainSetup (1, ch).ResetCalibration (false);
//  }
//}

int BoardSetup::SetDACmin (int gain, int febexchannel, double val)
{
  ASSERT_FEBCHAN_VALID(febexchannel);

  AccessGainSetup(gain, febexchannel).SetDACmin (val);
  return 0;

}

int BoardSetup::SetDACmax (int gain, int febexchannel, double val)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  AccessGainSetup(gain, febexchannel).SetDACmax (val);
  return 0;
}

int BoardSetup::SetADCmin (int gain, int febexchannel, double val)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "EvaluateCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
  AccessGainSetup(gain, febexchannel).SetADCmin (val);
  return 0;
}

int BoardSetup::EvaluateCalibration (int gain, int febexchannel, double deltaDAC, double deltaADC, double valDAC,
    double valADC)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "EvaluateCalibration for channel "<<febexchannel<<", gain:"<< gain << std::endl;
  AccessGainSetup(gain, febexchannel).EvaluateCalibration (deltaDAC, deltaADC, valDAC, valADC);
  return 0;
}

int BoardSetup::ResetCalibration (int gain, int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "ResetCalibration for channel "<<febexchannel<<", gain:"<< gain <<", baselineinvertd:"<<fBaselineInverted<< std::endl;
  AccessGainSetup(gain, febexchannel).ResetCalibration (false);
//#ifdef AWAGS_GAIN1_INVERTED
//  if (gain == 1)
//    AccessGainSetup(gain, febexchannel).ResetCalibration (false);
//  else
//    AccessGainSetup(gain, febexchannel).ResetCalibration (false);
//#else
//  AccessGainSetup(gain, febexchannel).ResetCalibration (false);
//#endif
  return 0;
}

int BoardSetup::DumpCalibration (int gain, int febexchannel)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  printm ("DumpCalibration for channel %d gain %d:\t", febexchannel, gain);
  AccessGainSetup(gain, febexchannel).DumpCalibration ();
  return 0;
}

int BoardSetup::CalculateDACValue (int gain, int febexchannel, double ADC_permille)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  //std::cout << "BoardSetup::GetDACValue for gain:"<<gain<<", channel:"<<febexchannel<<std::endl;
  int rev = 0;
  rev = AccessGainSetup(gain, febexchannel).CalculateDACValue (ADC_permille);
  return rev;
}

int BoardSetup::CalculateADCPermille (int gain, int febexchannel, double DAC_value)
{
  ASSERT_FEBCHAN_VALID(febexchannel);
  int rev = 0;
  rev = AccessGainSetup(gain, febexchannel).CalculateADCPermille (DAC_value);
  return rev;

}

/** convert febex channel to DAC indices*/
void BoardSetup::EvaluateDACIndices (int febexchannel, int& awags, int& dac)
{
  // this function is used for automatic baseline adjustments

  // JAM22: for awags, there is only one dac for each awags
  // but we treat it internally with 4 different indices
  // according to the 4 connected output channels
  awags = febexchannel / AWAGS_NUMCHANS;
  dac=   febexchannel % AWAGS_NUMDACS;

}

int BoardSetup::EvaluateADCChannel (int awags, int dac)
{
  int chan = awags * AWAGS_NUMCHANS + dac;
  // all 4 channels of each awags share same dac, but we use internally different dac indices for the benchmark results
  return chan;
}

/** get absolute DAC setting from relative baseline slider*/
int BoardSetup::EvaluateDACvalueAbsolute (int permillevalue, int febexchannel, int gain)
{
  //std::cout<<"EvaluateDACvalueAbsolute for gain:"<<gain<<", channel:"<<febexchannel << std::endl;
  int value = 0;
  value = round ((permillevalue * ((double) AWAGS_DAC_MAXVALUE) / 1000.0));
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
  int permille = round (1000.0 * ((double) value / (double) AWAGS_DAC_MAXVALUE));
  // default: linear interpolation of DAC for complete slider range, note inverted DAC polarity effect on baseline
  if (febexchannel >= 0)
  {
    // if channel specified, use calibration from measurements, inverted baseline was already applied
    permille = CalculateADCPermille (gain, febexchannel, value);
  }
  return permille;
}

bool  BoardSetup::IsAwagsPresent (int awags)
{
  ASSERT_AWAGS_VALID(awags);
  return fAwags[awags].IsPresent();
}

int BoardSetup::SetAwagsPresent (int awags, bool on)
{
  ASSERT_AWAGS_VALID(awags);
  return (fAwags[awags].SetPresent(on));
}


bool BoardSetup::HasAwagsPower (int awags)
  {
    ASSERT_AWAGS_VALID(awags);
    return fAwags[awags].HasPower();
  }

int BoardSetup::SetAwagsPowered (int awags, bool on)
  {
    ASSERT_AWAGS_VALID(awags);
    return fAwags[awags].SetPower(on);
  }


int BoardSetup::SetChipID (int awags, const QString& val)
{
  ASSERT_AWAGS_VALID(awags);
  return (fAwags[awags].SetChipID(val));
}

int BoardSetup::GetChipID (int awags, QString& val)
{
  ASSERT_AWAGS_VALID(awags);
  val=fAwags[awags].GetChipID();
  return 0;
}



int  BoardSetup::SetCurrentASIC(int awags, double val)
{
  ASSERT_AWAGS_VALID(awags);
  fAwags[awags].SetCurrentASIC(val);
  return 0;
}

double BoardSetup::GetCurrentASIC(int awags)
{
  ASSERT_AWAGS_VALID(awags)
  return (fAwags[awags].GetCurrentASIC());
}

int  BoardSetup::SetCurrentHV(int awags, double val)
{
  ASSERT_AWAGS_VALID(awags);
  fAwags[awags].SetCurrentHV(val);
   return 0;
}

double  BoardSetup::GetCurrentHV(int awags)
{
  ASSERT_AWAGS_VALID(awags);
  return (fAwags[awags].GetCurrentHV());
}

int  BoardSetup::SetCurrentDiode(int awags, double val)
{
  ASSERT_AWAGS_VALID(awags);
  fAwags[awags].SetCurrentDiode(val);
  return 0;
}
double  BoardSetup::GetCurrentDiode(int awags)
{
    ASSERT_AWAGS_VALID(awags);
    return (fAwags[awags].GetCurrentDiode());
 }





int BoardSetup::GetDACValue (int awags, int dac)
{
  ASSERT_AWAGS_VALID(awags);
  return fAwags[awags].GetDACValue (dac);
}

int BoardSetup::SetDACValue (int awags, int dac, uint16_t value)
{
  ASSERT_AWAGS_VALID(awags);
  return fAwags[awags].SetDACValue (dac, value);
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

//int BoardSetup::SetLowGain (int awags, int chan, bool low)
//{
//  ASSERT_AWAGS_VALID(awags);
//  return fAwags[awags].SetLowGain (chan, low);
//
//}
//
//int BoardSetup::GetLowGain (int awags, int chan)
//{
//  ASSERT_AWAGS_VALID(awags);
//  return fAwags[awags].GetLowGain (chan);
//}



int BoardSetup::GetAwagsID (int awags)
{
  ASSERT_AWAGS_VALID(awags);
  return fAwags[awags].GetAddressID ();

}

/** evaluate gain factor from setup. returns 1, 2,  4, or 8*/
int BoardSetup::GetGain (int awags, int dac)
{
  int gain = 1;
  ASSERT_AWAGS_VALID(awags);
  gain=fAwags[awags].GetGain (dac);
  return gain;
}


 /** record current gain factor to setup. */
 int BoardSetup::SetGain (int awags, int dac, uint8_t value)
 {
   ASSERT_AWAGS_VALID(awags);
   fAwags[awags].SetGain (dac,value);
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

