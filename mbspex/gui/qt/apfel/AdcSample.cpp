#include "AdcSample.h"

#include <algorithm>

//////////////////////////////////////////////////////////////////////////////////////////////////
////// container for single channel sample:
//////////////////////////////////////////

AdcSample::AdcSample ()
{
  Reset ();
}

void AdcSample::Reset ()
{
  fBaselineDone=false;
  fPeakfindDone=false;
  fMinValue = 0;
  fMaxValue = 0;
  fSample.clear();
  //fSample.resize(APFEL_ADC_SAMPLEVALUES);

  fBaselineStart=0;
  fBaselineStop=APFEL_ADC_SAMPLEVALUES;
  fMean=APFEL_NOVALUE;
  fSigma=APFEL_NOVALUE;
  fPeaks.clear();
  fPeakDelta=0;
  fHeightDelta=0;
  fNegative=false;

}

bool AdcSample::IsValid()
{
  return (fSample.size()>0 && fBaselineDone && fPeakfindDone);
}

void AdcSample::AddSample (uint16_t value)
 {
   fSample.push_back(value);
   fBaselineStop=fSample.size(); // use full sample for baseline mean and sigma evaluation by default

   if (fMinValue == 0 || value < fMinValue)
     fMinValue = value;
   if (value > fMaxValue)
     fMaxValue = value;


 }




 uint16_t AdcSample::GetSample (int index)
 {
   if (index < 0 || index >= fSample.size())
     return 0;

   return (fSample[index]);
 }

 void AdcSample::CalculateMeanAndSigma()
 {
   fMean = 0.;
   fSigma=0.;
   double sum = 0;;
   double start=fBaselineStart;
   double stop=fBaselineStop;
   if(start<0) start=0;
   if(stop>fSample.size()) stop=fSample.size();
   double num=stop-start;
   for (int i = start; i < stop; ++i)
     {
       fMean += fSample[i];
     }
   if(num)
       fMean /= num;

   for (int i = start; i < stop; ++i)
    {
      sum += pow ((fSample[i] - fMean), 2);
    }
    if(num)
      fSigma= sqrt (sum / num);

    fBaselineDone=true;
    //std::cout<<"AdcSample::CalculateMeanAndSigma - start:"<<start<<", stop:"<<stop<<", mean:"<< fMean<<", sigma:"<< fSigma<< std::endl;
 }








void  AdcSample::AddPeak(int pos, uint16_t height)
{

  fPeaks.push_back(AdcPeak(pos,height));
}

int  AdcSample::GetPeakHeight(int num)
 {
  if(!IsValid() || num >= GetNumPeaks()) return APFEL_NOVALUE;
  AdcPeak pk=fPeaks[num];
  return pk.fHeight;
 }


 int  AdcSample::GetPeakPosition(int num)
 {
   if(!IsValid() || num >= GetNumPeaks()) return APFEL_NOVALUE;
   AdcPeak pk=fPeaks[num];
   return pk.fPosition;
 }


 int AdcSample::GetNumPeaks()
 {
   return fPeaks.size();

 }




void AdcSample::FindPeaks(double deltaratio, double falldistance, bool negative)
 {
    int peak[APFEL_ADC_NUMMAXIMA];
    int pos[APFEL_ADC_NUMMAXIMA];

    int deltanextpeak=fSample.size()*deltaratio; // minimum distance to next peak. scaled by sample size(200 or 8000)
    int falldelta=falldistance; // stop peak finding if we decrease down such number of counts
    //std::cout<< "FindPeaks has deltaratio:"<<deltaratio<<", fall:"<<falldistance<<", deltanext:"<<deltanextpeak<<", fallabs:"<<falldelta<< std::endl;

    int startpos=0;
    fPeaks.clear();

  for(int p=0; p<APFEL_ADC_NUMMAXIMA;++p)
  {
    pos[p]=0;
    peak[p]=0;
    if(negative) peak[p]=-APFEL_ADC_MAXVALUE;

  for (int i = startpos; i < fSample.size(); ++i)
  {
      int val=fSample[i];
      if(negative) val*=-1.0; // just flip curve down
      if(val>peak[p]) {
         peak[p]=val;
         pos[p]=i;
       }
      if(val< peak[p] - falldelta) break; // stop peak search if we are on falling edge.

  } // for i
  //if(pos[p]==0) break;

  if(negative) peak[p]*=-1.0; // flip back to original value
  AddPeak(pos[p], peak[p]); // add in order of appereance. sort later
  //std::cout<<"FindPeaks added peak"<<pos[p]<<", "<<peak[p] << std::endl;
  startpos=pos[p] + deltanextpeak; // next peak search a bit right from last peak
  //std::cout<<"startpos="<<startpos << std::endl;

  }// for p

  // vector sort here, order depends on polarity:
  if(negative)
   std::sort(fPeaks.begin(), fPeaks.end());
  else
   std::sort(fPeaks.begin(), fPeaks.end(), std::greater<AdcPeak>());

  fPeakDelta=deltanextpeak;
  fHeightDelta=falldistance;
  fNegative=negative;

  fPeakfindDone=true;
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

