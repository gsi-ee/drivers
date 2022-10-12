#include "DacWorkCurve.h"

DacWorkCurve::DacWorkCurve ()
{
  Reset ();
}

void DacWorkCurve::Reset ()
{
  fPoints.clear ();
}

void DacWorkCurve::AddSample (uint16_t dac, uint16_t adc)
{
  fPoints.push_back (DacSample (dac, adc));
}

DacSample& DacWorkCurve::GetSample (int ix)
{
  return fPoints[ix];
}

int DacWorkCurve::NumSamples ()
{
  return fPoints.size ();
}

