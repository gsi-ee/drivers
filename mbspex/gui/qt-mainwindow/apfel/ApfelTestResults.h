#ifndef APFELTESTRESULTS_H
#define APFELTESTRESULTS_H



#include "ApfelDefines.h"

#include "GainSetup.h"
#include "DacWorkCurve.h"
#include "AdcSample.h"

#include <QDateTime>

#include <vector>
#include <string>

#define ASSERT_TEST_VALID   if(checkvalid && !fValid) return APFEL_NOVALUE;

//#define ASSERT_TEST_VALID ;
/** keeps the measured resulsts of a benchmark scan */
class ApfelTestResults
{
private:

  /** mark the test results of this chip valid or not.*/
  bool fValid;

  /** the label of the chip. Will correspond to the barcode value
   * sticked to the hardware*/
  QString fChipLabel;

  /** the label of the test board carrying this chip. Will correspond to the barcode value
     * sticked to the hardware*/
  QString fCarrierBoardLabel;

  /** user written information about environment temperature (in degree Centigrade)*/
  QString fTemperatureInfo;

  /** Current for ASIC measured by Keithley */
  double fCurrentASIC;

  /** Current for HV measured by Keithley */
  double fCurrentHV;

  /** Current for Diode mode measured by Keithley */
  double fCurrentDiode;

  bool fIDScanOK;

  /** true if general call  test has passed. false for failed or not yet done.*/
  bool fGeneralScanOK;

   /** true if general call  test has passed. false for failed or not yet done.*/
  bool fIDReverseScanOK;


   /** true if register scan test has passed. false for failed or not yet done.*/
  bool fRegisterScanOK;



  /** the address id of this apfel chip on the board*/
   uint8_t fAddressID;



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

   /** remember lower limit of baseline region*/
   int fBaselineLowerBound[APFEL_NUMDACS];

   /** remember Upper limit of baseline region*/
   int fBaselineUpperBound[APFEL_NUMDACS];


   /** keep minimum value of baseline sample set*/
    uint16_t fMinValue[APFEL_NUMDACS];;

    /** keep maximum value of baseline sample set*/
    uint16_t fMaxValue[APFEL_NUMDACS];;

    /** keep result of peak finding on trace for each DAC*/
    std::vector<AdcPeak> fPeaks[APFEL_NUMDACS];

    /** true if peak finding was applied for negative peaks (valleys in baseline)*/
    bool fNegativePeaks[APFEL_NUMDACS];

    /** record begin of benchmark*/
    QDateTime fStartTime;

    /** record begin of benchmark*/
    QDateTime fEndTime;


public:


  ApfelTestResults();

  virtual  ~ApfelTestResults(){
    //std::cout<< "TTTT DDD Dtor ApfrelTestResults object"<< std::endl
    ;}


  /** clear the previous results*/
  void Reset(bool invertedslope=false);

  /** reset previous results and mark start time*/
  void Begin();

  /** set valid and mark stop time*/
  void Finish();

  /** mark the test results as OK*/
  void SetValid(bool on=true)
  {
    fValid=on;
  }

  bool IsValid(){return fValid;}

  QString GetStartTime();

  QString GetEndTime();


  void SetChipDescriptor(const QString& label)
  {
    fChipLabel=label;
  }

  const QString& GetChipDescriptor()
  {
    return  fChipLabel;
  }
  void SetCarrierBoardDescriptor(const QString& label)
   {
    fCarrierBoardLabel=label;
   }

   const QString& GetCarrierBoardDescriptor()
   {
     return  fCarrierBoardLabel;
   }


   void SetTemperatureInfo(const QString& label)
   {
     fTemperatureInfo=label;
   }

   const QString& GetTemperatureInfo()
   {
     return  fTemperatureInfo;
   }




  void SetCurrentASIC(double amps){fCurrentASIC=amps;}
  void SetCurrentHV(double amps){fCurrentHV=amps;}
  void SetCurrentDiode(double amps){fCurrentDiode=amps;}


  double GetCurrentASIC(){return fCurrentASIC;}
  double GetCurrentHV(){return fCurrentHV;}
  double GetCurrentDiode(){return fCurrentDiode;}

  bool IsIDScanOK(){return fIDScanOK;}
  bool IsGeneralCallScanOK(){return fGeneralScanOK;}
  bool IsReverseIDScanOK(){return fIDReverseScanOK;}
  bool IsRegisterScanOK(){return fRegisterScanOK;}
  void SetIDScan(bool ok){fIDScanOK=ok;}
  void SetGeneralCallScan(bool ok){fGeneralScanOK=ok;}
  void SetReverseIDScan(bool ok){fIDReverseScanOK=ok;}
  void SetRegisterScan(bool ok){fRegisterScanOK=ok;}

  /** set local address id of this chip on the board*/
  void SetAddressId(uint8_t ad);

  /** get local address id of this chip on the board*/
  int GetAddressId();

  /** set the DAC value val after APFEL chip autocalibration for dac unit 0...3*/
  int SetDacValueCalibrate(int dac, uint16_t val);

  /** get the DAC value after APFEL chip autocalibration for dac 0...3.
   * checkvalid flag checks if test was completely finished**/
  int GetDacValueCalibrate(int dac, bool checkvalid=false);

  /** set the sampled mean ADC value after APFEL chip autocalibration for dac 0...3*/
  int SetAdcSampleMean(int dac, double val);

  /** get the sampled mean Adc value after APFEL chip autocalibration for dac 0...3*/
  double GetAdcSampleMean(int dac, bool checkvalid=false);

  /** set the sampled ADC sigma value after APFEL chip autocalibration for dac 0...3*/
  int SetAdcSampleSigma(int dac, double val);

  /** get the sampled ADC sigma value after APFEL chip autocalibration for dac 0...3*/
  double GetAdcSampleSigma(int dac, bool checkvalid=false);


  /** set lower limit of baseline region used for sigma and mean calculation*/
  int SetAdcBaselineLowerBound(int dac, int val);

  /** set upper limit of baseline region used for sigma and mean calculation*/
  int SetAdcBaselineUpperBound(int dac, int val);

  /** get lower limit of baseline region used for sigma and mean calculation*/
  int GetAdcBaselineLowerBound(int dac, bool checkvalid=false);

  /** get upper limit of baseline region used for sigma and mean calculation*/
  int GetAdcBaselineUpperBound(int dac, bool checkvalid=false);


  /** set the minimum ADC value of the acuired samples for dac 0...3*/
  int SetAdcSampleMinimum(int dac, uint16_t val);

  /** get the minimum ADC value of the acuired samples for dac 0...3*/
  int GetAdcSampleMinimum(int dac, bool checkvalid=false);

  /** set the maximum ADC value of the acuired samples for dac 0...3*/
  int SetAdcSampleMaximum(int dac, uint16_t val);

  /** get the maximum ADC value of the acuired samples for dac 0...3*/
  int GetAdcSampleMaximum(int dac, bool checkvalid=false);

  /** save the DAC-ADC calibration  GainSetup structure into the result object. for dac unit 0...3*/
int SetGainParameter(int dac, GainSetup structure);

/** get the slope parameter of the DAC-ADC calibration functionfor dac unit 0...3*/
double GetSlope (int dac, bool checkvalid=false);

/** get the D0 parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetD0 (int dac, bool checkvalid=false);

 /** get the DACmin parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetDACmin (int dac, bool checkvalid=false);

 /** get the DACmax parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetDACmax (int dac, bool checkvalid=false);

 /** get the ADCmin parameter of the DAC-ADC calibration function for dac unit 0...3*/
 double GetADCmin (int dac, bool checkvalid=false);


 /** Add another sampled point (valdac,valadc) of the DAC working curve, for dac unit 0...3*/
 int AddDacSample(int dac, uint16_t valdac, uint16_t valadc);

 /** Get reference to sampled DacSample point (valdac, valadc) of the DAC working curve at index x, for dac unit 0...3*/
 int AccessDacSample(DacSample& samp, int dac, int ix);

 /** clear samples of the DAC working curve */
 int ResetDacSample(int dac);

 /** add another found peak in the adc trace for a given dac*/
 int AddAdcPeak(int dac, int position, uint16_t height);

 /** remove all found peaks of adc trace for dac*/
 int ResetAdcPeaks(int dac);

 /** number of adc trace peaks found.*/
 int NumAdcPeaks(int dac);

 /** positon of the indexth adc trace peak provided by dac */
 int GetAdcPeakPosition(int dac, int index, bool checkvalid=false);

 /** height of the indexth adc trace peak provided by dac */
 int GetAdcPeakHeight(int dac, int index, bool checkvalid=false);

  /** returns true if peak finding was done with positive polarity. otherwise the peaks are valleys.*/
  bool HasNegativeAdcPeaks(int dac);

  /** mark trace peaks as valleys*/
  int SetNegativeAdcPeaks(int dac, bool negative);






};






#endif
