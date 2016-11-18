#ifndef GAINSETUP_H
#define GAINSETUP_H


#include "ApfelDefines.h"


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




#endif
