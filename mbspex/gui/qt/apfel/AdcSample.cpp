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
  fPeaks.clear();

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



void  AdcSample::AddPeak(int pos, uint16_t height)
{

  fPeaks.push_back(AdcPeak(pos,height));
}

uint16_t  AdcSample::GetPeakHeight(int num)
 {

  if(num >= GetNumPeaks()) return 0; // todo error handling properly
  AdcPeak pk=fPeaks[num];
  return pk.fHeight;
 }


 int  AdcSample::GetPeakPosition(int num)
 {
   if(num >= GetNumPeaks()) return 0; // todo error handling properly
   AdcPeak pk=fPeaks[num];
   return pk.fPosition;
 }


 int AdcSample::GetNumPeaks()
 {
   return fPeaks.size();

 }


void AdcSample::FindPeaks()
 {
    uint16_t peak[APFEL_ADC_NUMMAXIMA];
    int pos[APFEL_ADC_NUMMAXIMA];
    int deltanextpeak=5; // minimum distance to next peak
    int falldelta=500; // stop peak finding if we decrease down such number of counts
    int startpos=0;
    fPeaks.clear();

  for(int p=0; p<APFEL_ADC_NUMMAXIMA;++p)
  {
    pos[p]=0;
    peak[p]=0;
  for (int i = startpos; i < APFEL_ADC_SAMPLEVALUES; ++i)
  {
      if(fSample[i]>peak[p]) {
         peak[p]=fSample[i];
         pos[p]=i;
       }
      if(fSample[i]< peak[p] - falldelta) break; // stop peak search if we are on falling edge.
  } // for i
  //if(pos[p]==0) break;
  AddPeak(pos[p], peak[p]); // add in order of appereance. sort later
  //std::cout<<"FindPeaks added peak"<<pos[p]<<", "<<peak[p] << std::endl;
  startpos=pos[p] + deltanextpeak; // next peak search a bit right from last peak
  }// for p

  // TODO: vector sort here, later

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

