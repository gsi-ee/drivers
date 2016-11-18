#include "ApfelTest.h"

#include "ApfelGui.h"
#include "ApfelSetup.h"

ApfelTest::ApfelTest () :
    fOwner (0), fCurrentSetup (0), fTestLength (0), fCurrentGain(0)
{

  fReferenceValues[1]=ApfelTestResults();
  fReferenceValues[16]=ApfelTestResults();
  fReferenceValues[32]=ApfelTestResults();

  InitReferenceValues();
  ResetSequencerList();
}


void ApfelTest::InitReferenceValues()
{


  // gain 1, dac2
  //559   2790    575     2833    591     2823    607     2835    623     2873    639     2826    655     2804    671     3381    687     4459    703     5547    719     9407    735     7824    751     8950    767     10102   783     10493   799     12311   815     13345   831     14311   847     14970   863     15215   879     15231   895     15243   911     15247   927     15258

  int valDAC_1[APFEL_DAC_CURVEPOINTS]=
  {559, 575, 591, 607, 623, 639, 655, 671, 687, 703, 719, 735, 751, 767, 783, 799, 815, 831, 847, 863, 879, 895, 911, 927};
  int valADC_1[APFEL_DAC_CURVEPOINTS]=
  {2790, 2833, 2823, 2835, 2873, 2826, 2804, 3381,4459, 5547, 6407, 7824, 8950, 10102, 10493, 12311, 13345, 14311, 14970, 15215, 15231,15243,15247,15258   };

  for(int i=0; i<APFEL_DAC_CURVEPOINTS; ++i)
  {
    fReferenceValues[1].AddDacSample(2, valDAC_1[i], valADC_1[i]);
    //std::cout<< "InitReferenceValues for gain1: i:"<<i<<" ("<<valDAC_1[i]<<","<<valADC_1[i]<<"="<< std::endl;
  }




  // gain 16, dac0 and dac1
  //780   12222   782     13743   784     11987   786     11142   788     9712    790     9606    792     8817    794     7959    796     7179    798     7119    800     4409    802     3606    804     2909    806     2464    808     1615    810     1379    812     974 814     781 816     680 818     669 820     678 822     673 824     665 826     701
  int valDAC_16[APFEL_DAC_CURVEPOINTS]=
    {780, 782, 784, 786, 788, 790, 792, 794, 796, 798, 800, 802, 804, 806, 808, 810, 812, 814, 816, 818, 820, 822, 824, 826};

  int valADC_16[APFEL_DAC_CURVEPOINTS]=
  {14222,13743,11987,11142,9712,9606,8817,7959,7179,7119,4409,3606,2909,2464,1615,1379,974,781,680,669,678,673,665,701};




  for(int i=0; i<APFEL_DAC_CURVEPOINTS; ++i)
    {
      fReferenceValues[16].AddDacSample(0, valDAC_16[i], valADC_16[i]);
      fReferenceValues[16].AddDacSample(1, valDAC_16[i], valADC_16[i]);
      //std::cout<< "InitReferenceValues for gain16: i:"<<i<<" ("<<valDAC_16[i]<<","<<valADC_16[i]<<"="<< std::endl;
    }

  // gain 32, dac0 and dac1
  //821   7626    822     6724    823     6152    824     5507    825     4819    826     3966    827     7243    828     2811    829     4279    830     3663    831     1400    832     766 833     735 834     718 835     749 836     784 837     656 838     741 839     611 840     710 841     766 842     709 843     707 844     722


  int valDAC_32[APFEL_DAC_CURVEPOINTS]=
  { 821,822,823,824,825,826,827,828,829,830,831,832,833,834,835,836,837,838,839,840,841,842,843,844};

  int valADC_32[APFEL_DAC_CURVEPOINTS]=
  {
     7626,6724,6152,5507,4819,3966,3243,2811,2279,1663,1400,766,735,718,749,784,656,741,611,710,766,709,707,722
  };

  for(int i=0; i<APFEL_DAC_CURVEPOINTS; ++i)
      {
        fReferenceValues[32].AddDacSample(0, valDAC_32[i], valADC_32[i]);
        fReferenceValues[32].AddDacSample(1, valDAC_32[i], valADC_32[i]);
        //std::cout<< "InitReferenceValues for gain32: i:"<<i<<" ("<<valDAC_32[i]<<","<<valADC_32[i]<<"="<< std::endl;
      }


}


void ApfelTest::LoadReferenceValues(const QString& file)
{
  printm("Loading reference values from file %s not implemented yet.",file.toLatin1().constData());
}




ApfelTestResults& ApfelTest::GetReferenceValues(int gain)
{
   return  fReferenceValues[gain];
}


void ApfelTest::AddSequencerCommand (SequencerCommand com)
{
  fTestSequence.push (com);
}

SequencerCommand ApfelTest::NextSequencerCommand ()
{
  if (fTestSequence.empty ())
    return SEQ_NONE;
  SequencerCommand com = fTestSequence.front ();
  fTestSequence.pop ();
  return com;
}

void ApfelTest::FinalizeSequencerList ()
{
  fTestLength = fTestSequence.size ();
}

void ApfelTest::ResetSequencerList ()
{
  while (!fTestSequence.empty ())
    fTestSequence.pop ();
}

int ApfelTest::GetSequencerProgress ()
{
  double prozent = 100.0 * (double) (fTestLength - fTestSequence.size ()) / (double) fTestLength;
  return prozent;
}

bool ApfelTest::ProcessBenchmark ()
{
  if(fOwner==0 || fCurrentSetup==0) return false;

  SequencerCommand com=NextSequencerCommand();
  int febexchannel=com.GetChannel();
  switch(com.GetAction())
  {
    case SEQ_NONE:

      printm("ApfelTest has reached end of sequencer list!");
      return false;
      //fSequencerTimer->stop();
      break;
    case SEQ_GAIN_1:
      printm("ApfelTest Sets to gain 1.");
      // always keep setup consistent with the applied values:
      fCurrentSetup->SetHighGain(false); // apply gain 1 by this.
      fOwner->SetSwitches(fCurrentSetup->IsApfelInUse(), fCurrentSetup->IsHighGain(), fCurrentSetup->IsStretcherInUse());
      fCurrentGain=1;
      break;
    case SEQ_GAIN_16:
       printm("ApfelTest Sets to gain 16.");
       // always keep setup consistent with the applied values:
       fCurrentSetup->SetHighGain(true); // apply gain 1 by this.
       fOwner->SetSwitches(fCurrentSetup->IsApfelInUse(), fCurrentSetup->IsHighGain(), fCurrentSetup->IsStretcherInUse());

       for(int apfel=0; apfel<APFEL_NUMCHIPS; ++apfel)
       {
           for(int channel=0; channel<APFEL_NUMCHANS;++channel)
           {
             fCurrentSetup->SetLowGain (apfel, channel, true);
             fOwner->SetGain(apfel, channel, true);
           }

       }
       fCurrentGain=16;
       break;

    case SEQ_GAIN_32:
           printm("ApfelTest Sets to gain 32.");
           // always keep setup consistent with the applied values:
           fCurrentSetup->SetHighGain(true); // apply gain 1 by this.
           // note that we do not change other switches for the moment.
           fOwner->SetSwitches(fCurrentSetup->IsApfelInUse(), fCurrentSetup->IsHighGain(), fCurrentSetup->IsStretcherInUse());
           // now need to change channel gains:

           for(int apfel=0; apfel<APFEL_NUMCHIPS; ++apfel)
           {
               for(int channel=0; channel<APFEL_NUMCHANS;++channel)
               {
                 fCurrentSetup->SetLowGain (apfel, channel, false);
                 fOwner->SetGain(apfel, channel, false);
               }

           }
           fCurrentGain=32;
           break;
    case  SEQ_AUTOCALIB:
      printm("ApfelTest is doing DAC Autocalibration.");
      fOwner->AutoCalibrate_all();

      // now save DAC settings to structure:
      for(int apfel=0; apfel<APFEL_NUMCHIPS; ++apfel)
      {
        ApfelTestResults& theResults=fCurrentSetup->AccessTestResults(fCurrentGain, apfel);
        for(int dac=0; dac<APFEL_NUMDACS; ++dac)
          {
            theResults.SetDACValueCalibrate(dac, fCurrentSetup->GetDACValue(apfel,dac));
          }

      }

      break;

    case  SEQ_NOISESAMPLE:
      printm("ApfelTest is measuring Baseline noise samples of channel %d",febexchannel);
      {
        fOwner->AcquireSample(febexchannel);
        fOwner->ShowSample(febexchannel,true);
      double mean=fCurrentSetup->GetADCMean(febexchannel);
      double sigma=fCurrentSetup->GetADCSigma(febexchannel);
      double minimum=fCurrentSetup->GetADCMiminum(febexchannel);
      double maximum=fCurrentSetup->GetADCMaximum(febexchannel);
      // print here channelwise. TODO: evaluate mean and sigma only once, not for every refresh
      int apfel=0, dac=0;
      fCurrentSetup->EvaluateDACIndices(febexchannel,apfel,dac);
      ApfelTestResults& theResults=fCurrentSetup->AccessTestResults(fCurrentGain, apfel);
      theResults.SetDACSampleMean(dac,mean);
      theResults.SetDACSampleSigma(dac,sigma);
      theResults.SetDACSampleMinimum(dac,minimum);
      theResults.SetDACSampleMinimum(dac,maximum);
      printm("\tChannel %d : mean=%f sigma=%f minimum=%f maximum=%f", febexchannel, mean,sigma,minimum,maximum);
      }
      break;

    case  SEQ_BASELINE:
      printm("Benchmark Timer is doing baseline calibration of channel %d", febexchannel);
      {
        fOwner->CalibrateADC(febexchannel);
        int apfel=0, dac=0;
        fCurrentSetup->EvaluateDACIndices(febexchannel,apfel,dac);
        ApfelTestResults& theResults=fCurrentSetup->AccessTestResults(fCurrentGain, apfel);
        GainSetup gainSetup=fCurrentSetup->AccessGainSetup(fCurrentGain,febexchannel);
        theResults.SetGainParameter(dac, gainSetup);
      }
      break;

    case SEQ_CURVE:
      // to do
      printm("Benchmark Timer is evaluating DAC curve of channel %d", febexchannel);
      fOwner->ScanDACCurve(fCurrentGain, febexchannel);
      break;
    default:
      printm("Benchmark Timer will NOT execute unknown command %d for channel %d",com.GetAction(), com.GetChannel());
      return false;
      break;
  };
  return true;
}

