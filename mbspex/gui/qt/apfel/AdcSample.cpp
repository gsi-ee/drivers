#include "AdcSample.h"

//////////////////////////////////////////////////////////////////////////////////////////////////
////// container for single channel sample:
//////////////////////////////////////////

AdcSample::AdcSample ()
{
  Reset ();
}

void AdcSample::Reset ()
{
  fMinValue = 0;
  fMaxValue = 0;
  for (int i = 0; i < APFEL_ADC_SAMPLEVALUES; ++i)
    fSample[i] = 0;

}

double AdcSample::GetMean ()
{
  double val = 0;
  for (int i = 0; i < APFEL_ADC_SAMPLEVALUES; ++i)
  {
    val += fSample[i];
  }
  val /= APFEL_ADC_SAMPLEVALUES;
  return val;
}

double AdcSample::GetSigma ()
{
  double val = 0, sum = 0;
  double mean = GetMean ();
  for (int i = 0; i < APFEL_ADC_SAMPLEVALUES; ++i)
  {
    sum += pow ((fSample[i] - mean), 2);
  }
  val = sqrt (sum / APFEL_ADC_SAMPLEVALUES);
  return val;
}

void AdcSample::DumpParameters (int label)
{
  printm ("AdcSample %d: Mean=%f, Sigma=%f, Minimum=%d, Maximum=%d", label, GetMean (), GetSigma (), GetMinimum (),
      GetMaximum ());
}

void AdcSample::ShowSample (int label)
{
  printm ("Show Sample %d:");
  for (int i = 0; i < APFEL_ADC_SAMPLEVALUES; ++i)
  {
    /* todo: ascii graphics of histogram display?*/
    printm ("i:%d val:%d", i, fSample[i]);
  }

}

