#ifndef APFELSETUP_H
#define APFELSETUP_H

#include <stdio.h>
#include <stdint.h>
#include <math.h>


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

  int SetTestPulsePostive (bool pos = true);

  int GetTestPulsePositive ();

  void SetAddressID (uint8_t address);

  uint8_t GetAddressID ();

};

/** this structure contains DAC calibration curve parameters for each febex ADC channel*/
class GainSetup
{
private:

  double fDAC_ADC_Slope;
  double fDAC_0;

public:

  GainSetup ();

  void ResetCalibration (bool positive = true);
  void DumpCalibration();

  void SetSlope (double val);
  void SetD0 (double val);

  /** function returns dac value to set for relative height of adc baseline in permille*/
  int GetDACValue (double ADC_permille);

  /** function returns relative adc baseline in permille for given dac value */
  int GetADCPermille (double DAC_value);

  /** calculate and set calibration curve for measured variations deltaADC and deltaDAC around
   * autocalibrated DAC setting valDAC*/
  void EvaluateCalibration (double deltaDAC, double deltaADC, double valDAC, double valADC);

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

public:

  BoardSetup ();

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

  void ResetGain1Calibration ();

  int EvaluateCalibration (int gain, int febexchannel, double deltaDAC, double deltaADC, double valDAC, double valADC);

  int ResetCalibration (int gain, int febexchannel);

  int DumpCalibration (int gain, int febexchannel);

  int GetDACValue (int gain, int febexchannel, double ADC_permille);

  int GetADCPermille (int gain, int febexchannel, double DAC_value);

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

  int SetTestPulsePostive (int apfel, bool pos = true);

  int GetTestPulsePositive (int apfel);

  int GetApfelID (int apfel);

  /** evaluate gain factor from setup. returns 1, 16 or 32*/
  int GetGain (int apfel, int dac);

};

#endif
