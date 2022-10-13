#ifndef BOARDSETUP_H
#define BOARDSETUP_H

#include "GosipGui.h"

#include "AwagsDefines.h"

#include "AwagsSetup.h"
#include "GainSetup.h"

#include "AdcSample.h"
#include "SequencerCommand.h"
#include "AwagsTestResults.h"

#include <queue>
#include <map>

#include <QString>




/** the setup of the awags/febex slave board*/
class BoardSetup : public GosipSetup
{

private:
#include <stdint.h>

  /** if true, hardware is prototype board. If false, use benchmark test hardware
   * JAM: currently just some extra switch for later...*/
  bool fProtoBoard;

  /** enable awags input */
  bool fUseAwags;

  /* switch output between gain 16/32 (true) and gain 1 (false)*/
  bool fHighGainOutput;




  /** setups of each awags chip on board*/
  AwagsSetup fAwags[AWAGS_NUMCHIPS];



  /** calibration parameters (adc/dac) mapped to gain 1, 2, 4, 8*/
  std::map<int, GainSetup> fGainSetups[AWAGS_ADC_CHANNELS];

  AdcSample fLastSample[AWAGS_ADC_CHANNELS];

  /** This structure contains the test results for comparison. mapped to gain 1,16,32*/
  std::map<int, AwagsTestResults> fTestResults[AWAGS_NUMCHIPS];
  /** carrier board id tag*/
  QString fBoardID;

  /** environemnt temperature logged by operator*/
  QString fTemperature;

public:

  BoardSetup ();

  virtual ~BoardSetup(){std::cout<<"DDDD DTOR  BoardSetup..." << std::endl;;}

  void SetBoardID (const QString& val)
  {
    fBoardID = val;
  }

  const QString& GetBoardID ()
  {
    return fBoardID;
  }



  void SetTemperature (const QString& val)
  {
    fTemperature= val;
  }

  const QString& GetTemperature ()
  {
    return fTemperature;
  }





  bool IsAwagsInUse ()
  {
    return fUseAwags;
  }
  void SetAwagsInUse (bool on)
  {
    fUseAwags = on;
  }


  void SetUsePrototypeBoard(bool on)
  {
    fProtoBoard=on;
  }

  bool IsUsePrototypeBoard()
  {
    return fProtoBoard;
  }



  /** access to test result structure for gain and awags id.
   * Todo: separate setters and getters?*/
  AwagsTestResults& AccessTestResults (int gain, int awags);

  /** fetch full gain structure to copy it into the test results*/
  GainSetup& AccessGainSetup (int gain, int febchannel);

  /** following functions belong to gain related DAC/ADC calilbration:*/



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

  /** convert febex channel to awags and DAC indices*/
  void EvaluateDACIndices (int febexchannel, int& awags, int& dac);

  /** convert awags and dac index to boardwise febex channel */
  int EvaluateADCChannel (int awags, int dac);

  /** get absolute DAC setting from relative baseline slider*/
  int EvaluateDACvalueAbsolute (int permillevalue, int febexchannel = -1, int gain = 1);

  /** get relative ADC slider value from given dac setting*/
  int EvaluateADCvaluePermille (int value, int febexchannel = -1, int gain = 1);

  /** check chip presence flag evaluated from last request*/
  bool IsAwagsPresent (int awags);

  /** mark the awags chip as active */
  int SetAwagsPresent (int awags, bool on);

  /** check chip power on flag set after last switching on/off*/
   bool HasAwagsPower (int awags);

   /** mark that awags chip at this slot has been switched on/off */
   int SetAwagsPowered (int awags, bool on);


  int GetDACValue (int awags, int dac);

  int SetDACValue (int awags, int dac, uint16_t value);

  /** helper function to access DAC value via global febex channel number*/
  int GetDACValue (int febexchannel);

  /** helper function to set DAC value via global febex channel number*/
  int SetDACValue (int febexchannel, uint16_t value);


  // awags bus address id as configured
  int GetAwagsID (int awags);


  int SetChipID (int awags, const QString& val);

  // awags chip id as taken from tag sticker. passes description as string val. Returns -1 in case of error.
  int GetChipID (int awags,  QString& val);

  int SetCurrentASIC(int awags, double val);

  double GetCurrentASIC(int awags);

  int SetCurrentHV(int awags, double val);

  double GetCurrentHV(int awags);

  int SetCurrentDiode(int awags, double val);

  double GetCurrentDiode(int awags);




  /** evaluate gain factor from setup. returns 1, 2,4, or 8*/
  int GetGain (int awags, int dac);

  /** record current gain factor to setup. */
  int SetGain (int awags, int dac, uint8_t value);


  /** clear values and statistics of ADC sample for single febexchannel*/
  int ResetADCSample (int febexchannel);

  /** add next sampled ADC point to the end of the trace for single febexchannel. */
  int AddADCSample (int febexchannel, uint16_t value);

  /** get most recent sampled ADC point for single febexchannel. */
  uint16_t GetADCSample (int febexchannel, int index);

  /** number of sampled points for febexchannel. Depends on mode (mbs or monitoring port)*/
  int GetADCSampleLength (int febexchannel);

  /** Find peaks in sample of channel*/
  int EvaluatePeaks(int febexchannel, double deltaratio=0.025, double falldistance=1000, bool negative=false);

  /** Height of peak number num in sample*/
  uint16_t GetSamplePeakHeight(int febexchannel, int num);

  /** Position of peak number num in sample*/
  int  GetSamplePeakPosition(int febexchannel, int num);

  /** number of peaks in sample of channel*/
  int NumSamplePeaks(int febexchanne);

  /** true if peakfinder was applied for negative peaks*/
  bool IsSamplePeaksNegative(int febexchannel);

  /** delta height used for last found peaks*/
  double GetSamplePeaksHeightDelta(int febexchannel);

  /** delta position used for last found peaks*/
  double GetSamplePeaksPositionDelta(int febexchannel);



  /** Calculate baseline figures of merit mean and sigma for febexchanne.
   * Region of baseline may be defined by first and last index in sample trace.
   * This allows to cut out the pulser peaks*/
  int EvaluateBaseline(int febexchannel, int firstindex=0, int lastindex=0);


  /** get baseline region lower bound (firstindex in sample) of last recent mean and sigma calculation for febexchannel*/
  int GetADCBaslineLowerBound(int febexchannel);

  /** get baseline region upper bound (lastindex in sample) of last recent mean and sigma calculation for febexchannel*/
  int GetADCBaslineUpperBound(int febexchannel);


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
