#ifndef APFELSETUP_H
#define APFELSETUP_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <queue>

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
struct pex_sfp_links
{
  int numslaves[PEX_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
};

#endif

#include "ApfelDefines.h"


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

  /** amplitude value for test pulse (0-F)*/
  uint8_t fTestPulseAmplitude[APFEL_NUMCHANS];

  /** True if test pulser with positive polarity. False for negative*/
  bool fTestPulsPositive;

public:

  /* all initialization here:*/
  ApfelSetup ();

  /** getter and setter methods to avoid possible segfaults at wrong indices: */
  int GetDACValue (int dac);

  int SetDACValue (int dac, uint16_t value);

  int SetLowGain (int chan, bool low = true);

  int GetLowGain (int chan);

  int SetTestPulseEnable (int chan, bool on = true);

  int GetTestPulseEnable (int chan);

  int SetTestPulseAmplitude (int chan, uint8_t amp);

  uint8_t GetTestPulseAmplitude (int chan);


  int SetTestPulsePostive (bool pos = true);

  int GetTestPulsePositive ();

  void SetAddressID (uint8_t address);

  uint8_t GetAddressID ();

};

/** this structure contains DAC calibration curve parameters for each febex ADC channel*/
class GainSetup
{
private:

  /** slope of linear part of calibration*/
  double fDAC_ADC_Slope;

  /** pseudo axis section of linear part of calibration*/
  double fDAC_0;

  /** minimum reasonable DAC value (corresponds to ADC saturation value ~15000)*/
  double fDAC_min;

  /** maximum reasonable linear DAC value (corresponds to ADC lower offset value fADC_min)*/
  double fDAC_max;

  /** minimum achievable ADC value (corresponds to DAC maximum value 1024)*/
  double fADC_min;



public:

  GainSetup ();

  void ResetCalibration (bool positive = true);
  void DumpCalibration();

  void SetSlope (double val);
  void SetD0 (double val);

  void SetDACmin (double val);
  void SetDACmax (double val);
  void SetADCmin (double val);

  double GetSlope ();
  double GetD0 ();
  double GetDACmin ();
  double GetDACmax ();
  double GetADCmin ();


  /** function returns dac value to set for relative height of adc baseline in permille*/
  int CalculateDACValue (double ADC_permille);

  /** function returns relative adc baseline in permille for given dac value */
  int CalculateADCPermille (double DAC_value);

  /** calculate and set calibration curve for measured variations deltaADC and deltaDAC around
   * autocalibrated DAC setting valDAC*/
  void EvaluateCalibration (double deltaDAC, double deltaADC, double valDAC, double valADC);

};


/** this structure keeps the most recent baseline sample for a single ADC channel*/
class AdcSample
{
private:

  uint16_t fSample[APFEL_ADC_SAMPLEVALUES];

  /** keep minimum value of current sample set*/
  uint16_t fMinValue;

  /** keep maximum value of current sample set*/
  uint16_t fMaxValue;

public:

  AdcSample();


  void Reset();

  void SetSample(int index, uint16_t value)
  {
    if(index<0 || index>=APFEL_ADC_SAMPLEVALUES) return;
    fSample[index]=value;
    if(fMinValue==0 || value<fMinValue) fMinValue=value;
    if(value>fMaxValue) fMaxValue=value;
  }

  uint16_t GetSample(int index)
   {
     if(index<0 || index>=APFEL_ADC_SAMPLEVALUES) return 0;
     return (fSample[index]);
   }

  /** evaluate mean value of sample*/
  double GetMean();

  /** evaluate sigma value of sample*/
  double GetSigma();

  uint16_t GetMinimum()
  {
    return fMinValue;
  }

  uint16_t GetMaximum()
   {
     return fMaxValue;
   }

  /** show mean and sigma values. label can be used to specify channel number*/
  void DumpParameters(int label);

  /** display or dump current sample.  label can be used to specify channel number*/
  void ShowSample(int label);


};


class ApfelTestResults
{
private:

  /** the address id of this apfel chip on the board*/
   uint8_t fAddressID;

   /** the absolute values of the APFEL dacs after autocalibration*/
   uint16_t fDACValueCalibrate[APFEL_NUMDACS];


   /** keep results of DAC-ADC calibration mapped to apfel channels*/
   GainSetup fDAC_ADC_Gain[APFEL_NUMDACS];

   /** mean value of measured baseline after autocalibration*/
   double fMean[APFEL_NUMDACS];

   /** sigma value of measured baseline after autocalibration*/
   double fSigma[APFEL_NUMDACS];

   /** keep minimum value of baseline sample set*/
    uint16_t fMinValue[APFEL_NUMDACS];;

    /** keep maximum value of baseline sample set*/
    uint16_t fMaxValue[APFEL_NUMDACS];;



public:


  ApfelTestResults(uint8_t address=0)
  {
    Reset();
  }

  void Reset()
  {
    fAddressID=0;
    for(int dac=0; dac<APFEL_NUMDACS; ++dac)
        {
          fDACValueCalibrate[dac]=0;
          fMean[dac]=0;
          fSigma[dac]=0;
          fMinValue[dac]=0;
          fMaxValue[dac]=0;
          fDAC_ADC_Gain[dac].ResetCalibration();
        }
  }

  void SetAddressId(uint8_t ad)
  {
    fAddressID=ad;
  }

  uint8_t GetAddressId()
  {
     return fAddressID;
   }

  int SetDACValueCalibrate(int dac, uint16_t val)
  {
    ASSERT_DAC_VALID(dac);
    fDACValueCalibrate[dac]=val;
    return 0;
  }

  uint16_t GetDACValueCalibrate(int dac)
  {
    ASSERT_DAC_VALID(dac)
    return fDACValueCalibrate[dac];
  }

  int SetDACSampleMean(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fMean[dac]=val;
    return 0;
  }

  double GetDACSampleMean(int dac)
   {
     ASSERT_DAC_VALID(dac);
     return fMean[dac];
   }
  int SetDACSampleSigma(int dac, double val)
  {
    ASSERT_DAC_VALID(dac);
    fSigma[dac]=val;
    return 0;
  }

  double GetDACSampleSigma(int dac)
   {
     ASSERT_DAC_VALID(dac);
     return fSigma[dac];
   }


  int SetDACSampleMinimum(int dac, uint16_t val)
    {
      ASSERT_DAC_VALID(dac);
      fMinValue[dac]=val;
      return 0;
    }

  uint16_t GetDACSampleMinimum(int dac)
     {
    ASSERT_DAC_VALID(dac);
    return fMinValue[dac];
     }


  int SetDACSampleMaxium(int dac, uint16_t val)
     {
       ASSERT_DAC_VALID(dac);
       fMaxValue[dac]=val;
       return 0;
     }

   uint16_t GetDACSampleMaximum(int dac)
      {
     ASSERT_DAC_VALID(dac);
     return fMaxValue[dac];
      }

int SetGainParameter(int dac, GainSetup structure)
{
  ASSERT_DAC_VALID(dac);
  fDAC_ADC_Gain[dac]=structure; // hope for default assignment operator!
  return 0;
}

double GetSlope (int dac)
{
  ASSERT_DAC_VALID(dac);
  return fDAC_ADC_Gain[dac].GetSlope();
}

 double GetD0 (int dac)
 {
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetD0();
 }

 double GetDACmin (int dac)
 {
   ASSERT_DAC_VALID(dac);
      return fDAC_ADC_Gain[dac].GetDACmin();

 }
 double GetDACmax (int dac)
 {
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetDACmax();

 }

 double GetADCmin (int dac)
 {
   ASSERT_DAC_VALID(dac);
   return fDAC_ADC_Gain[dac].GetADCmin();

 }



};




/** command action id to be put into the test sequencer list*/

enum SequencerAction
  {
    SEQ_NONE,
    SEQ_GAIN_1,
    SEQ_GAIN_16,
    SEQ_GAIN_32,
    SEQ_AUTOCALIB,
    SEQ_NOISESAMPLE,
    SEQ_BASELINE,
    SEQ_CURVE
  };


/** command token to be put into the test sequencer list*/
class SequencerCommand
{
private:

  /** action command id to be executed*/
  SequencerAction fAction;

  /** optional channel number. used to explictely execute loops with low command granularity in timer*/
  int fChannel;

public:

  SequencerCommand(SequencerAction actionid, int channel=0): fAction(actionid), fChannel(channel){;}

  SequencerAction GetAction(){return fAction;}
  int GetChannel(){return fChannel;}
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

  AdcSample fLastSample[APFEL_ADC_CHANNELS];

  std::queue <SequencerCommand> fTestSequence;

  ApfelTestResults fTest_32[APFEL_NUMCHIPS];
  ApfelTestResults fTest_16[APFEL_NUMCHIPS];
  ApfelTestResults fTest_1[APFEL_NUMCHIPS];


  /** Initial number of sequencer commands*/
  int fTestLength;

  QString fApfelID;

  QString fApfelTag;


public:

  BoardSetup ();


  void setApfelID(const QString& val){fApfelID=val;}
  void setApfelTag(const QString& val){fApfelTag=val;}

  const QString& getApfelID(){return fApfelID;}
  const QString& getApfelTag(){return fApfelTag;}


  bool IsApfelInUse ()
  {
    return fUseApfel;
  }
  void SetApfelInUse (bool on)
  {
    fUseApfel = on;
  }
  bool IsHighGain ()
  {
    return fHighGainOutput;
  }
  void SetHighGain (bool on)
  {
    fHighGainOutput = on;
  }
  bool IsStretcherInUse ()
  {
    return fStretcher;
  }
  void SetStretcherInUse (bool on)
  {
    fStretcher = on;
  }
  bool IsRegularMapping ()
  {
    return fRegularMapping;
  }

  void SetApfelMapping (bool regular = true);

  /** access to test result structure for gain and apfel id.
   * Todo: separate setters and getters?*/
  ApfelTestResults& AccessTestResults(int gain, int apfel);

  /** fetch full gain structure to copy it into the test results*/
  GainSetup& AccessGainSetup(int gain, int febexchannel);


  /** following functions belong to gain related DAC/ADC calilbration:*/

  void ResetGain1Calibration ();

  int SetDACmin (int gain, int febexchannel, double val);

  int SetDACmax (int gain, int febexchannel, double val);

  int SetADCmin (int gain, int febexchannel,double val);

  int EvaluateCalibration (int gain, int febexchannel, double deltaDAC, double deltaADC, double valDAC, double valADC);

  int ResetCalibration (int gain, int febexchannel);

  int DumpCalibration (int gain, int febexchannel);

  /** return calibrated DAC value to set to achieve ADC_permille baseline*/
  int CalculateDACValue (int gain, int febexchannel, double ADC_permille);

  /** return calibrated ADC_permille baseline from DAC_value set*/
  int CalculateADCPermille (int gain, int febexchannel, double DAC_value);

  /** convert febex channel to apfel and DAC indices*/
  void EvaluateDACIndices (int febexchannel, int& apfel, int& dac);

  /** convert apfel and dac index to boardwise febex channel */
  int EvaluateADCChannel (int apfel, int dac);

  /** get absolute DAC setting from relative baseline slider*/
  int EvaluateDACvalueAbsolute (int permillevalue, int febexchannel = -1, int gain = 1);

  /** get relative ADC slider value from given dac setting*/
  int EvaluateADCvaluePermille (int value, int febexchannel = -1, int gain = 1);

  int GetDACValue (int apfel, int dac);

  int SetDACValue (int apfel, int dac, uint16_t value);

  /** helper function to access DAC value via global febex channel number*/
  int GetDACValue (int febexchannel);

  /** helper function to set DAC value via global febex channel number*/
  int SetDACValue (int febexchannel, uint16_t value);

  /** set gain 16 (low=true) or 32 for local channel (0,1) on apfel of index*/
  int SetLowGain (int apfel, int chan, bool low = true);

  /** returns true for gain 16 or false for gain 32 in local channel (0,1) on apfel of index*/
  int GetLowGain (int apfel, int chan);

  int SetTestPulseEnable (int apfel, int chan, bool on = true);

  int GetTestPulseEnable (int apfel, int chan);

  int SetTestPulseAmplitude (int apfel, int chan, uint8_t val);

  int GetTestPulseAmplitude (int apfel, int chan);

  int SetTestPulsePostive (int apfel, bool pos = true);

  int GetTestPulsePositive (int apfel);

  int GetApfelID (int apfel);

  /** evaluate gain factor from setup. returns 1, 16 or 32*/
  int GetGain (int apfel, int dac);

  /** clear values and statistics of ADC sample for single febexchannel*/
  int ResetADCSample(int febexchannel);

  /** set sampled ADC point for single febexchannel. */
  int SetADCSample(int febexchannel, int index, uint16_t value);

  /** get most recent sampled ADC point for single febexchannel. */
  uint16_t GetADCSample(int febexchannel, int index);

  /** evaluate mean value of most recent ADC sample for febexchannel*/
  double GetADCMean(int febexchannel);

  /** evaluate sigma value of most recent ADC sample for febexchannel*/
  double GetADCSigma(int febexchannel);

  /** get minimum value of most recent ADC sample for febexchannel*/
   double GetADCMiminum(int febexchannel);

   /** get maximum value of most recent ADC sample for febexchannel*/
   double GetADCMaximum(int febexchannel);


  /** display mean and sigma values on text box*/
  int DumpADCSamplePars(int febexchannel);

  /** display most recent sample values or curve on text box*/
  int ShowADCSample(int febexchannel);


  /** put a command for the test sequence to the list*/
  void AddSequencerCommand(SequencerCommand com);

  /** get next command from sequencer list and remove it*/
  SequencerCommand NextSequencerCommand();

  /** Mark Sequencer list as complete*/
  void FinalizeSequencerList();

  /** clear all entries for sequencer*/
  void ResetSequencerList();

  /** returns percentage of sequencer commands already done.*/
  int GetSequencerProgress();


};

#endif
