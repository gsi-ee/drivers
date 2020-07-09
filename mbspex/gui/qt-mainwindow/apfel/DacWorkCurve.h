#ifndef DACWORKCURVE_H
#define DACWORKCURVE_H

#include "ApfelDefines.h"

#include <vector>

/** a single sampled point of the DAC-ADC curve*/
class DacSample
{
protected:
  /** DAC setting for a scan*/
  uint16_t fDacSetting;

  /** measured ADC value corresponding to DAC setting*/
  uint16_t fAdcValue;

public:

  DacSample (uint16_t dac, uint16_t adc) :
      fDacSetting (dac), fAdcValue (adc)
  {
  }

  virtual ~DacSample(){;}

  void SetSample (uint16_t dac, uint16_t adc)
  {
    fDacSetting = dac;
    fAdcValue = adc;
  }

  uint16_t GetDACValue ()
  {
    return fDacSetting;
  }

  uint16_t GetADCValue ()
  {
    return fAdcValue;
  }

};

/** this object contains a scan of the work curve for a single DAC*/
class DacWorkCurve
{
private:

  /** Contains the scanned points of the work curve*/
  std::vector<DacSample> fPoints;

public:

  DacWorkCurve ();

  virtual ~DacWorkCurve(){;}

  /** clear all points on the curve*/
  void Reset ();

  /** add another sampled point (dac,adc)*/
  void AddSample (uint16_t dac, uint16_t adc);

  /** Get Sampled point DacSample at index ix*/
  DacSample& GetSample (int ix);

  /** number of points stored in the curve*/
  int NumSamples ();

};

#endif
