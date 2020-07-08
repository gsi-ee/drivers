#ifndef ADCSAMPLE_H
#define ADCSAMPLE_H

#include "ApfelDefines.h"

#include <vector>

/** one peak found in the sample */
class AdcPeak
{
public:

  /** adc value at peak centroid*/
  uint16_t fHeight;

  /** position index in sample*/
  int fPosition;

  AdcPeak(int pos, uint16_t height):fPosition(pos),fHeight(height){}


  /** define less than operator for vector sort*/
  bool operator < (const AdcPeak& other) const
      {
          return (fHeight < other.fHeight);
      }

  /** define greater than operator for vector sort*/
   bool operator > (const AdcPeak& other) const
       {
           return (fHeight > other.fHeight);
       }




};



/** this structure keeps the most recent baseline sample for a single ADC channel*/
class AdcSample
{
private:


  std::vector<uint16_t> fSample;

  /** true if baseline mean and sigma have been calculated at least once*/
  bool fBaselineDone;

  /** true if peak seardch has benn done at least once*/
  bool fPeakfindDone;

  /** keep minimum value of current sample set*/
  uint16_t fMinValue;

  /** keep maximum value of current sample set*/
  uint16_t fMaxValue;


  /** index in sample where baseline region starts*/
  int fBaselineStart;

  /** index in sample where baseline region stops. should be before the pulser peaks begin*/
  int fBaselineStop;

  /** most recently evaluated mean value of baseline region*/
  double fMean;

  /** most recently evaluated sigma value of baseline region*/
  double fSigma;

  /** heighs of the first n maxima peaks in sample*/
  std::vector<AdcPeak> fPeaks;

  /** property indicates that peaks are negative*/
  bool fNegative;

  /** absolute window size used to find the peaks*/
  double fPeakDelta;

  /** absolute peak fall distance used to find the peaks*/
  double fHeightDelta;


public:

  AdcSample ();

  virtual ~AdcSample(){;}

  void Reset ();

  /** sample is ready to read out the figures of merit*/
  bool IsValid();

  /** set sample point (index,value)*/
//  void SetSample (int index, uint16_t value);

  /** add next sample point value to the trace*/
  void AddSample(uint16_t value);

  uint16_t GetSample (int index);

  int GetNumSamples(){ return fSample.size();}


  void SetBaselineStartIndex(int index)
  {
    fBaselineStart=index;
  }

  int GetBaselineStartIndex()
  {
    return fBaselineStart;
  }

  void SetBaselineStopIndex(int index)
  {
    fBaselineStop=index;
  }

  int GetBaselineStopIndex()
  {
    return fBaselineStop;
  }

  /** evaluate mean value of sample with respect to baseline region as defined*/
  void CalculateMeanAndSigma();



  /** get most recent evaluated mean value of sample*/
  double GetMean ()
  {
    //if(!IsValid()) return APFEL_NOVALUE;
    return fMean;
  }

  /** get most recent evaluated sigma value of sample*/
  double GetSigma ()
  {
    //if(!IsValid()) return APFEL_NOVALUE;
    return fSigma;
  }



  int GetMinimum ()
  {
    //if(!IsValid()) return APFEL_NOVALUE;
    return fMinValue;
  }

  int GetMaximum ()
  {
    //if(!IsValid()) return APFEL_NOVALUE;
    return fMaxValue;
  }


  /** find the first n absolute maxima within the current sample.
   * Arguments may specify parameters for peakfinding*/
  void FindPeaks(double deltaratio, double falldistance, bool negative=false);

  /* add found peak (position, height) to the list*/
  void AddPeak(int pos, uint16_t height);

  int GetPeakHeight(int num);

  int GetPeakPosition(int num);

  int GetNumPeaks();


double GetPosDelta()
{
  return fPeakDelta;
}

double GetHeightDelta()
{
  return fHeightDelta;
}

bool IsNegativePeaks()
{
  return fNegative;
}

  /** show mean and sigma values. label can be used to specify channel number*/
  void DumpParameters (int label);

  /** display or dump current sample.  label can be used to specify channel number*/
  void ShowSample (int label);

};

#endif
