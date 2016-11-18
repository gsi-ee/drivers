#ifndef APFELTESTRESULTS_H
#define APFELTESTRESULTS_H



#include "ApfelDefines.h"

#include "GainSetup.h"
#include "DacWorkCurve.h"

class ApfelTestResults
{
private:

  /** the address id of this apfel chip on the board*/
   //uint8_t fAddressID;

   /** the absolute values of the APFEL dacs after autocalibration*/
   uint16_t fDACValueCalibrate[APFEL_NUMDACS];


   /** keep results of DAC-ADC calibration mapped to apfel channels*/
   GainSetup fDAC_ADC_Gain[APFEL_NUMDACS];

   /** keep measured work curve*/
   DacWorkCurve fDAC_Curve[APFEL_NUMDACS];

   /** mean value of measured baseline after autocalibration*/
   double fMean[APFEL_NUMDACS];

   /** sigma value of measured baseline after autocalibration*/
   double fSigma[APFEL_NUMDACS];

   /** keep minimum value of baseline sample set*/
    uint16_t fMinValue[APFEL_NUMDACS];;

    /** keep maximum value of baseline sample set*/
    uint16_t fMaxValue[APFEL_NUMDACS];;



public:


  ApfelTestResults();

  /** clear the previous results*/
  void Reset();

  /** set the DAC value val after APFEL chip autocalibration for dac unit 0...3*/
  int SetDACValueCalibrate(int dac, uint16_t val);

  /** get the DAC value after APFEL chip autocalibration for dac 0...3**/
  uint16_t GetDACValueCalibrate(int dac);

  /** set the sampled mean ADC value after APFEL chip autocalibration for dac 0...3*/
  int SetDACSampleMean(int dac, double val);

  /** get the sampled mean ADC value after APFEL chip autocalibration for dac 0...3*/
  double GetDACSampleMean(int dac);

  /** set the sampled ADC sigma value after APFEL chip autocalibration for dac 0...3*/
  int SetDACSampleSigma(int dac, double val);

  /** get the sampled ADC sigma value after APFEL chip autocalibration for dac 0...3*/
  double GetDACSampleSigma(int dac);

  /** set the minimum ADC value of the acuired samples for dac 0...3*/
  int SetDACSampleMinimum(int dac, uint16_t val);

  /** get the minimum ADC value of the acuired samples for dac 0...3*/
  uint16_t GetDACSampleMinimum(int dac);

  /** set the maximum ADC value of the acuired samples for dac 0...3*/
  int SetDACSampleMaxium(int dac, uint16_t val);

  /** get the maximum ADC value of the acuired samples for dac 0...3*/
  uint16_t GetDACSampleMaximum(int dac);

  /** save the DAC-ADC calibration  GainSetup structure into the result object. for dac unit 0...3*/
int SetGainParameter(int dac, GainSetup structure);

/** get the slope parameter of the DAC-ADC calibration functionfor dac unit 0...3*/
double GetSlope (int dac);

/** get the D0 parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetD0 (int dac);

 /** get the DACmin parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetDACmin (int dac);

 /** get the DACmax parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetDACmax (int dac);

 /** get the ADCmin parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetADCmin (int dac);


 /** Add another sampled point (valdac,valadc) of the DAC working curve, for dac unit 0...3*/
 int AddDacSample(int dac, uint16_t valdac, uint16_t valadc);

 /** Get reference to sampled DacSample point (valdac, valadc) of the DAC working curve at index x, for dac unit 0...3*/
 int AccessDacSample(DacSample& samp, int dac, int ix);

 /** clear samples of the DAC working curve */
 int ResetDacSample(int dac);

};






#endif
