#ifndef BOARDSETUP_H
#define BOARDSETUP_H

#include "ApfelDefines.h"

#include "ApfelSetup.h"
#include "GainSetup.h"

#include "AdcSample.h"
#include "SequencerCommand.h"
#include "ApfelTestResults.h"

#include <queue>
#include <map>

#include <QString>

/** the setup of the apfel/febex slave board*/
class BoardSetup
{

private:
#include <stdint.h>
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

  /** calibration parameters (adc/dac) mapped to gain1,16, 32*/
  std::map<int, GainSetup> fGainSetups[APFEL_ADC_CHANNELS];

  AdcSample fLastSample[APFEL_ADC_CHANNELS];

  /** This structure contains the test results for comparison. mapped to gain 1,16,32*/
  std::map<int, ApfelTestResults> fTestResults[APFEL_NUMCHIPS];

  /** board id tag*/
  QString fBoardID[2];

  /** current in A for DUT*/
  double fCurrent;

  /** voltage in V for DUT*/
  double fVoltage;

public:

  BoardSetup ();

  void SetBoardID (int ix, const QString& val)
  {
    fBoardID[ix] = val;
  }

  const QString& GetBoardID (int ix)
  {
    return fBoardID[ix];
  }

  void SetCurrent (double val)
  {
    fCurrent = val;
  }

  double GetCurrent ()
  {
    return fCurrent;
  }
  void SetVoltage (double val)
   {
     fVoltage = val;
   }

   double GetVoltage ()
   {
     return fVoltage;
   }
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
  ApfelTestResults& AccessTestResults (int gain, int apfel);

  /** fetch full gain structure to copy it into the test results*/
  GainSetup& AccessGainSetup (int gain, int febexchannel);

  /** following functions belong to gain related DAC/ADC calilbration:*/

  void ResetGain1Calibration ();

  int SetDACmin (int gain, int febexchannel, double val);

  int SetDACmax (int gain, int febexchannel, double val);

  int SetADCmin (int gain, int febexchannel, double val);

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
  int ResetADCSample (int febexchannel);

  /** set sampled ADC point for single febexchannel. */
  int SetADCSample (int febexchannel, int index, uint16_t value);

  /** get most recent sampled ADC point for single febexchannel. */
  uint16_t GetADCSample (int febexchannel, int index);

  /** Find peaks in sample of channel*/
  int EvaluatePeaks(int febexchannel);

  /** Height of peak number num in sample*/
  uint16_t GetSamplePeakHeight(int febexchannel, int num);

  /** Position of peak number num in sample*/
  int  GetSamplePeakPosition(int febexchannel, int num);

  /** number of peaks in sample of channel*/
  int NumSamplePeaks(int febexchanne);


  /** evaluate mean value of most recent ADC sample for febexchannel*/
  double GetADCMean (int febexchannel);

  /** evaluate sigma value of most recent ADC sample for febexchannel*/
  double GetADCSigma (int febexchannel);

  /** get minimum value of most recent ADC sample for febexchannel*/
  double GetADCMiminum (int febexchannel);

  /** get maximum value of most recent ADC sample for febexchannel*/
  double GetADCMaximum (int febexchannel);

  /** display mean and sigma values on text box*/
  int DumpADCSamplePars (int febexchannel);

  /** display most recent sample values or curve on text box*/
  int ShowADCSample (int febexchannel);

};

#endif
