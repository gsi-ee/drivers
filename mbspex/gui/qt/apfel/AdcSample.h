#ifndef ADCSAMPLE_H
#define ADCSAMPLE_H

#include "ApfelDefines.h"

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

  AdcSample ();

  void Reset ();

  void SetSample (int index, uint16_t value)
  {
    if (index < 0 || index >= APFEL_ADC_SAMPLEVALUES)
      return;
    fSample[index] = value;
    if (fMinValue == 0 || value < fMinValue)
      fMinValue = value;
    if (value > fMaxValue)
      fMaxValue = value;
  }

  uint16_t GetSample (int index)
  {
    if (index < 0 || index >= APFEL_ADC_SAMPLEVALUES)
      return 0;
    return (fSample[index]);
  }

  /** evaluate mean value of sample*/
  double GetMean ();

  /** evaluate sigma value of sample*/
  double GetSigma ();

  uint16_t GetMinimum ()
  {
    return fMinValue;
  }

  uint16_t GetMaximum ()
  {
    return fMaxValue;
  }

  /** show mean and sigma values. label can be used to specify channel number*/
  void DumpParameters (int label);

  /** display or dump current sample.  label can be used to specify channel number*/
  void ShowSample (int label);

};

#endif
