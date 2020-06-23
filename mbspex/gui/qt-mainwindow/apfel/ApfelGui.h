#ifndef APFELGUI_H
#define APFELGUI_H


#include "GosipGui.h"
#include "ApfelWidget.h"

#include <QProcess>
#include <QString>
#include <QTimer>
#include <QTime>


#include "BoardSetup.h"
#include "ApfelTest.h"
#include "ApfelTestResults.h"





class ApfelGui: public GosipGui
{
  friend class ApfelTest;


  Q_OBJECT

public:
  ApfelGui (QWidget* parent = 0);
  virtual ~ApfelGui ();


virtual GosipSetup* CreateSetup()
     {
       return new BoardSetup();
     }
  

enum ApfelTextColor_t
{
  apfel_black,
  apfel_red,
  apfel_green,
  apfel_yellow,
  apfel_blue,
  apfel_red_background,
  apfel_green_background,
  apfel_yellow_background,
  apfel_blue_background
};


protected:



  ApfelWidget* fApfelWidget;


  /** aggregate with all testing/sequencing functionality*/
  ApfelTest fBenchmark;

  

  /** timer for periodic test pulsing*/
  QTimer* fPulserTimer;

  /** timer to display status of test pulsing*/
  QTimer* fDisplayTimer;

  /** timer to perform the automatic benchmark testing in the background.*/
  QTimer* fSequencerTimer;


  /** measures the time since begin of the benchmark test*/
  QTime  fSequencerStopwatch;

  /** auxiliary references to checkboxes for baseline adjustments*/
  QCheckBox* fBaselineBoxes[16];

  /** auxiliary references to spinbox for baseline adjustment view*/
  QSpinBox* fDACSpinBoxes[16];



  /** auxiliary references to adc baseline display for refresh view*/
  QLineEdit* fADCLineEdit[16];


  /** auxiliary references to checkboxes for adc samples */
  QCheckBox* fSamplingBoxes[16];

  /** auxiliary references to adc sample mean display */
  QLineEdit* fSamplingMeanLineEdit[16];

   /** auxiliary references to adc sample sigma display */
   QLineEdit* fSamplingSigmaLineEdit[16];




  /** auxiliary references to dac value display for refresh view*/
  QSlider* fDACSlider[APFEL_NUMCHIPS][APFEL_NUMDACS];


  /** auxiliary references to dac value display for refresh view*/
  QLineEdit* fDACLineEdit[APFEL_NUMCHIPS][APFEL_NUMDACS];

  /** auxiliary references to pulser display for refresh view*/
  QComboBox* fApfelPulsePolarityCombo[APFEL_NUMCHIPS];

  /** auxiliary references to pulser display for refresh view*/
  QCheckBox* fApfelPulseEnabledCheckbox[APFEL_NUMCHIPS][APFEL_NUMCHANS];

  /** auxiliary references to pulser display for refresh view*/
  QSpinBox* fApfelPulseAmplitudeSpin[APFEL_NUMCHIPS][APFEL_NUMCHANS];


  QComboBox* fApfelGainCombo[APFEL_NUMCHIPS][APFEL_NUMCHANS];

  QGroupBox* fApfelPulseGroup[APFEL_NUMCHIPS];

  QGroupBox* fApfelDACGroup[APFEL_NUMCHIPS];

  QGroupBox* fApfelGainGroup[APFEL_NUMCHIPS];

  QCheckBox* fApfelPowerCheckbox[APFEL_NUMCHIPS];

  QLabel* fApfelPowerLabel[APFEL_NUMCHIPS];




  //QGroupBox* fApfelPowerGroup[APFEL_NUMCHIPS];
  QLabel* fApfelAddressLabel[APFEL_NUMCHIPS];

  QLineEdit* fApfelSerialLineEdit[APFEL_NUMCHIPS];

  QLabel* fApfelCurrentASICLabel[APFEL_NUMCHIPS];
  QLabel* fApfelCurrentHVLabel[APFEL_NUMCHIPS];
  QLabel* fApfelCurrentDiodeLabel[APFEL_NUMCHIPS];

  QDoubleSpinBox* fApfelCurrentASICSpin[APFEL_NUMCHIPS];
  QDoubleSpinBox* fApfelCurrentHVSpin[APFEL_NUMCHIPS];
  QDoubleSpinBox* fApfelCurrentDiodeSpin[APFEL_NUMCHIPS];


  QLabel* fApfelIDScanLabel[APFEL_NUMCHIPS];
  QLabel* fApfelGeneralCallLabel[APFEL_NUMCHIPS];
  QLabel* fApfelReverseIDScanLabel[APFEL_NUMCHIPS];
  QLabel* fApfelRegisterTestLabel[APFEL_NUMCHIPS];

  KPlotWidget* fPlotWidget[16];



  unsigned fPulserProgressCounter;

 

  /** test data output file handle*/
  FILE* fTestFile;

  /** temporary data field for mbs readout buffer samples*/
  uint16_t fData[APFEL_MBS_TRACELEN];


  /** keeps range of current benchmark plot window*/
  int fPlotMinDac;

  /** keeps range of current benchmark plot window*/
  int fPlotMaxDac;

  /** keeps range of current benchmark plot window*/
  int fPlotMinAdc;

  /** keeps range of current benchmark plot window*/
  int fPlotMaxAdc;


  /** general mode flag for different addressing modes of pandatest switches*/
  bool fUseSimpleSwitchAddressing;

  /** update register display*/
  void RefreshView ();


  /** change apfel address labels depending on panda or pasem hardware*/
  void RefreshApfelLabels(bool ispandatest);

  /** udpate display of dac settings for apfel chip with given index */
  void RefreshDAC(int apfel);

  /** udpate display of current measurements for apfel chip with given index.
   * Return value is false if any of the tests shows a failure */
   bool RefreshCurrents(int apfel, bool reset=false);

   /** udpate display of  id scans for apfel chip with given index. if reset is true, put startup colors.
    *  Return value is false if any of the tests shows a failure*/
   bool RefreshIDScan(int apfel, bool reset=false);

  /** udpate display of adc value of channel. specify gain to set relative dac slider from calibration */
   void RefreshADC_channel(int channel, int gain);

   /** udpate display of adc  that currently belongs to apfel and dac indices*/
   void RefreshADC_Apfel(int apfel, int dac);


   /** udpate display of most recent adc  sample from chanmnel*/
   void RefreshLastADCSample(int febexchannel);



   /** show the sample maxima for febexchannel in table*/
   void RefreshSampleMaxima(int febexchannel);



   /** helper function to put coloured text into any label on gui.*/
   void RefreshColouredLabel(QLabel* label, const QString text, ApfelTextColor_t color=apfel_black);


   /** apply configuration from file*/
   void ApplyFileConfig(int );


  /** copy values from gui to internal status object*/
  void EvaluateView ();


  /** put io switch settings for apfel chip from gui into setup structure*/
  void EvaluateIOSwitch();

  /** put test pulser settings for apfel chip from gui into setup structure*/
  void EvaluatePulser(int apfel);

  /** decode pulser interval from frequency box index*/
  int EvaluatePulserInterval(int index);

  /** put gain settings for apfel chip and channel from gui into setup structure*/
  void EvaluateGain(int apfel, int channel);

  /** set register from status structure*/
  void SetRegisters ();

  /** apply test pulser settings for apfel chip from setup structure to device*/
  void SetPulser(uint8_t apfel);

  /** apply test pulser broadcast settings to all apfel chips*/
  void SetBroadcastPulser();

  /** set io switch from setup structures to device */
  void SetIOSwitch();


  /** set apfel addon boards to inverted mount mode
   * (apfel9-12 first, apfel1-4 second) */
  void SetInverseMapping(int on);


  /** set DAC to ADC baseline slope for gain 16/32 into inverted mode (old boards)*/
  void SetBaselineInverted(int on);


  /** get register contents to status structure*/
  void GetRegisters ();

  /** get DAC settings of apfel into status structure*/
  void GetDACs (int apfel);



  /** entry point when save button is clicked*/
  void SaveConfig();

  /** the actual saving with requester menu*/
  void DoSaveConfig(const char* name=0);

  /** save results of benchmark tests*/
  void SaveTestResults();

  /** Map index of apfel chip on board to addressing id number*/
  uint8_t GetApfelId(int sfp, int slave, uint8_t apfelchip);


  /** Write value to i2c bus address of currently selected slave. apfel slot index  and local dac id are specified*/
    int WriteDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac, uint16_t value);


    /** Write value to i2c bus address of currently selected slave. apfel address id and local dac id are specified*/
    int WriteDAC_ApfelI2c_ToID (uint8_t apid, uint8_t dac, uint16_t value);

    /** Read value to i2c bus address of currently selected slave. apfel slot index position and local dac id are specified*/
    int ReadDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac);

    /** Read value to i2c bus address of currently selected slave. Use specified apfel bus address id and local dac id are specified*/
    int ReadDAC_ApfelI2c_FromID (uint8_t apid, uint8_t dac);


    /** evaluate i2c channel adress offset on apfel for given channel number*/
    int GetChannelOffsetDAC(uint8_t chan);


    /** Read value from adc channel of currently selected slave. adc unit id and local channel id are specified*/
    int ReadADC_Apfel (uint8_t adc, uint8_t chan);

    /** sample adc baseline of global channel febexchan
     *  by avering over several readouts of ADC.
     *  numsamples may specify how many samples to average. default is APFEL_ADC_BASELINESAMPLES=3
     *  Baseline value is returned.*/
    int AcquireBaselineSample(uint8_t febexchan, int numsamples=-1);


    /** read a trace from the mbs buffer of febexchan into the fData field*/
    int AcquireMbsSample(uint8_t febexchan);

    /** set gain factor for each apfel channel on board. High gain switch must be enabled for board.
     *  gain is 16 if useGain16=true, or 32 if useGain16=false (default)
     * */
    void SetGain(uint8_t apfelchip, uint8_t chan, bool useGain16);

    /** set test pulser properties for each apfel channel on board.
         *  flag on=true switches pulser on
         *  amp1 and amp2 specify channel amplitudes to activate, both must be set with a single call
         *  flag positive=true: positive polarity, =false: negative pulse
         * */
    void SetTestPulse(uint8_t apfelchip, bool on, uint8_t amp1, uint8_t amp2, bool positive);


    /** Perform automatic calibration of specified apfel chip*/
    void DoAutoCalibrate(uint8_t apfelchip);

    /** Send autocalibration broadcast to current board*/
    void DoAutoCalibrateAll();

    /** Fetch DAC and ADC values related to apfelchip after autocalibratio*/
    void UpdateAfterAutoCalibrate(uint8_t apfelchip);


    /** helper function to handle different febex fpga features to access power switches*/
    void WriteSwitchRegister(int lo, int hi, bool simplemode=true);

    /** set switch register of currently selected slave (apfel input on/off), gain 1 or 16/32, stretcher on/off)
     * different behaviour depending on actual hardware (pandatest setup or pasem)*/
    void SetSwitches(bool useApfel, bool useHighGain, bool useStretcher, bool isPandatest=false);


    /** set power state after apfel chip index on/off bitmask. take into account the current board gain setting*/
    void SetPower (int powermask, bool highgain);

    /** back to default config after the address tests.*/
    void SetDefaultIOConfig();


   /** Initialize febex after power up*/
   void ResetSlave();


  /** helper function that either does enable i2c on board, or writes such commands to .gos file*/
  void EnableI2C();

  /** helper function that either does disable i2c on board, or writes such commands to .gos file*/
  void DisableI2C ();


  /** dump current ADC values of currently set FEBEX*/
  void DumpSlave();


  /** dump current ADC values of currently set APFEL*/
  void DumpADCs();


  /** *dump dac2 channel calibrations */
  void DumpCalibrations();

 

  /** open test characteristics file for writing*/
   int OpenTestFile(const QString& fname);

   /** guess what...*/
   int CloseTestFile();

   /** append text to currently open test file*/
   int WriteTestFile(const QString& text);



  /** Set relativ DAC value permille to APFELchannel, returns ADC value*/
  int autoApply(int channel, int permille);


  /** apply relative DAC value permille and refresh gui from ADC sample.
   * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyRefresh(int channel, int permille);

  /** evaluate change of spinbox for febex channel channel*/
  void DAC_spinBox_changed(int channel, int val);


  /** apply io switch settings directly.
   * * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplySwitch();


  /** send general pulser settings to all apfel channels*/
  void BroadcastPulser();


  /** apply pulser settings directly
    * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyPulser(int apfel);

 /** slot forward when change of pulser settings on gui*/
  void PulserChanged(int apfel);


  /** apply chip power connection settings directly
     * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
   void AutoApplyPower(int apfel, int state);

  /** slot forward when change of power button checkbox on gui*/
  void PowerChanged(int apfel, int state);

  /** apply gain settings directly
      * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyGain(int apfel, int channel);


  /** slot forward when change of pulser settings on gui*/
   void GainChanged(int apfel, int channel);





   /** apply absolute DAC value val directly
  * This function is capable of usage in APFEL_BROADCAST_ACTION macro*/
  void AutoApplyDAC(int apfel, int dac, int val);


  /** slot forward when change of dacslider for apfelchip and dac
   * refresh display of textline
   * may do automatic apply*/
  void DAC_changed(int apfel, int dac, int val);


  /** slot forward when  input of dac textline for apfelchip and dac
   * also refresh display of slider here
   *  may do automatic apply**/
  void DAC_enterText(int apfel, int dac);


  /** start interactive autocalibration of apfel chip dacs.*/
  void AutoCalibrate(int apfel);



  /** Automatic adjustment of adc baseline to adctarget value for global febex channel.
   * will return final dac setup value or -1 in case of error*/
  int AdjustBaseline(int channel, int adctarget);

  /** Adjust baselines of the currently selected febex device.*/
  void AutoAdjust();

  /** Set baseline of febex ADC channel to the targetvalue.*/
  void AutoAdjustChannel (int channel, unsigned targetvalue);


  /** perform a scan of the DAC-ADC curve of gain and channel.
   * gain:1,16,32 */
  int ScanDACCurve(int gain, int channel);


  /** Automatic calibration of DAC->ADC relation for febex channel.
   * Will AutoCalibrate corresponding apfel first*/
    int CalibrateADC(int channel);

    /** Calibrate DAC->ADC for ADC channels with set checkbox checked.*/
    void CalibrateSelectedADCs();

    /** Reset calibration of DAC->ADC relation for febex channel.
      * Default is linear falling curve betwen adc and dac*/
    int CalibrateResetADC(int channel);

    /** Reset Calibration  DAC->ADC for ADC channels with set checkbox checked.*/
    void CalibrateResetSelectedADCs();


    /** get a sample from febex readout buffer for specified channel.
     *   peakfinder polarity may specify the expected peak polarity: 0: negative, 1: positive
     * default: use setup in gui checkbox*/
    int AcquireSample(int channel, int peakfinderpolarity=-1);

    /** get sample from febex  for ADC channels with acquire checkbox checked.*/
    void AcquireSelectedSamples();


    /** Clear display of benchmark DAC curve*/
    void ResetBenchmarkCurve();

    /* show DAC curve in benchmark display for allowed range*/
    void ShowLimitsCurve(int gain, int apfel, int dac);

    /* show DAC curve in benchmark display for gain, chip and dacl*/
    void ShowBenchmarkCurve(int gain, int apfel, int dac);

    /** request for file to load reference data*/
    void LoadBenchmarkReferences();

    /** dump most recent acquired adc sample for specified channel.
     * if benchmarkdisplay is set, plot to general benchmark pad */
    int ShowSample(int channel, bool benchmarkdisplay=false);

    /** dump most recent acquired adc sample for ADC channels with acquire checkbox checked.*/
    void ShowSelectedSamples();

    /** zoom into sampled plot*/
    void ZoomSample(int channel);

    /** show full range of sample plot*/
    void UnzoomSample(int channel);

    /** zoom display of sample for channel to the vicinity of peak number peak*/
    void ZoomSampleToPeak(int channel, int peak);

    /** do peak finding on current sample
     * polarity may specify the expected peak polarity: 0: negative, 1: positive
     * default: use setup in gui checkbox*/
    void FindPeaks(int channel,int usepolarity=-1);

    /** Switch peak finder polarity to negative or positive. */
    void SetPeakfinderPolarityNegative(bool on);

    /** read voltage and current via serial connection from toellner power supply*/
    void ReadToellnerPower(double& u, double& i);


    /** init keithley multimeter*/
    void InitKeithley();

    /** Read current via serial connection from keithley device*/
    double ReadKeithleyCurrent();

    /** calculate mean and sigma of sampled baseline for channel.
     * Baseline region may be cut with gui elements*/
    void EvaluateBaseline(int channel);


    /** loop over all connected apfel chips and perform address scan.*/
    void DoIdScan();

    /** Perform ID Scan for apfel chip at given slot */
    void ExecuteIDScanTest(int apfel);


    /** Enable single chip communication and set local id*/
    void SetSingleChipCommID(int apfel, int id);


    /** Activate single chip and set the switch bits for different current measurements */
    void SetSingleChipCurrentMode(int apfel, bool selectHV, bool selectDiode);





    /** loop over all connected apfel chips and perform current scan with keithley.*/
      void DoCurrentScan();

      /** Perform ampere Scan for apfel chip at given slot */
      void ExecuteCurrentScan(int apfel);

 

public slots:
  
  virtual void AutoAdjustBtn_clicked ();
  virtual void CalibrateADCBtn_clicked();
  virtual void CalibrateResetBtn_clicked();

  virtual void AcquireSamplesBtn_clicked();
  virtual void DumpSamplesBtn_clicked();

  virtual void ZoomSampleBtn_clicked();
  virtual void UnzoomSampleBtn_clicked();
  virtual void RefreshSampleBtn_clicked();

  virtual void PeakFinderBtn_clicked();


 
  virtual void DAC_spinBox_all_changed(int val);
  virtual void Any_spinBox00_changed(int val);
  virtual void Any_spinBox01_changed(int val);
  virtual void Any_spinBox02_changed(int val);
  virtual void Any_spinBox03_changed(int val);
  virtual void Any_spinBox04_changed(int val);
  virtual void Any_spinBox05_changed(int val);
  virtual void Any_spinBox06_changed(int val);
  virtual void Any_spinBox07_changed(int val);
  virtual void Any_spinBox08_changed(int val);
  virtual void Any_spinBox09_changed(int val);
  virtual void Any_spinBox10_changed(int val);
  virtual void Any_spinBox11_changed(int val);
  virtual void Any_spinBox12_changed(int val);
  virtual void Any_spinBox13_changed(int val);
  virtual void Any_spinBox14_changed(int val);
  virtual void Any_spinBox15_changed(int val);



  virtual void DAC_changed_0_0(int val);
  virtual void DAC_changed_0_1(int val);
  virtual void DAC_changed_0_2(int val);
  virtual void DAC_changed_0_3(int val);
  virtual void DAC_changed_1_0(int val);
  virtual void DAC_changed_1_1(int val);
  virtual void DAC_changed_1_2(int val);
  virtual void DAC_changed_1_3(int val);
  virtual void DAC_changed_2_0(int val);
  virtual void DAC_changed_2_1(int val);
  virtual void DAC_changed_2_2(int val);
  virtual void DAC_changed_2_3(int val);
  virtual void DAC_changed_3_0(int val);
  virtual void DAC_changed_3_1(int val);
  virtual void DAC_changed_3_2(int val);
  virtual void DAC_changed_3_3(int val);
  virtual void DAC_changed_4_0(int val);
  virtual void DAC_changed_4_1(int val);
  virtual void DAC_changed_4_2(int val);
  virtual void DAC_changed_4_3(int val);
  virtual void DAC_changed_5_0(int val);
  virtual void DAC_changed_5_1(int val);
  virtual void DAC_changed_5_2(int val);
  virtual void DAC_changed_5_3(int val);
  virtual void DAC_changed_6_0(int val);
  virtual void DAC_changed_6_1(int val);
  virtual void DAC_changed_6_2(int val);
  virtual void DAC_changed_6_3(int val);
  virtual void DAC_changed_7_0(int val);
  virtual void DAC_changed_7_1(int val);
  virtual void DAC_changed_7_2(int val);
  virtual void DAC_changed_7_3(int val);


  virtual void DAC_enterText_0_0 ();
  virtual void DAC_enterText_0_1 ();
  virtual void DAC_enterText_0_2 ();
  virtual void DAC_enterText_0_3 ();
  virtual void DAC_enterText_1_0 ();
  virtual void DAC_enterText_1_1 ();
  virtual void DAC_enterText_1_2 ();
  virtual void DAC_enterText_1_3 ();
  virtual void DAC_enterText_2_0 ();
  virtual void DAC_enterText_2_1 ();
  virtual void DAC_enterText_2_2 ();
  virtual void DAC_enterText_2_3 ();
  virtual void DAC_enterText_3_0 ();
  virtual void DAC_enterText_3_1 ();
  virtual void DAC_enterText_3_2 ();
  virtual void DAC_enterText_3_3 ();
  virtual void DAC_enterText_4_0 ();
  virtual void DAC_enterText_4_1 ();
  virtual void DAC_enterText_4_2 ();
  virtual void DAC_enterText_4_3 ();
  virtual void DAC_enterText_5_0 ();
  virtual void DAC_enterText_5_1 ();
  virtual void DAC_enterText_5_2 ();
  virtual void DAC_enterText_5_3 ();
  virtual void DAC_enterText_6_0 ();
  virtual void DAC_enterText_6_1 ();
  virtual void DAC_enterText_6_2 ();
  virtual void DAC_enterText_6_3 ();
  virtual void DAC_enterText_7_0 ();
  virtual void DAC_enterText_7_1 ();
  virtual void DAC_enterText_7_2 ();
  virtual void DAC_enterText_7_3 ();


  virtual void AutoCalibrate_0();
  virtual void AutoCalibrate_1();
  virtual void AutoCalibrate_2();
  virtual void AutoCalibrate_3();
  virtual void AutoCalibrate_4();
  virtual void AutoCalibrate_5();
  virtual void AutoCalibrate_6();
  virtual void AutoCalibrate_7();
  virtual void AutoCalibrate_all();

  virtual void PulserChanged_0();
  virtual void PulserChanged_1();
  virtual void PulserChanged_2();
  virtual void PulserChanged_3();
  virtual void PulserChanged_4();
  virtual void PulserChanged_5();
  virtual void PulserChanged_6();
  virtual void PulserChanged_7();


  virtual void GainChanged_0();
  virtual void GainChanged_1();
  virtual void GainChanged_2();
  virtual void GainChanged_3();
  virtual void GainChanged_4();
  virtual void GainChanged_5();
  virtual void GainChanged_6();
  virtual void GainChanged_7();
  virtual void GainChanged_8();
  virtual void GainChanged_9();
  virtual void GainChanged_10();
  virtual void GainChanged_11();
  virtual void GainChanged_12();
  virtual void GainChanged_13();
  virtual void GainChanged_14();
  virtual void GainChanged_15();


  virtual void PowerChanged_0(int checkstate);
  virtual void PowerChanged_1(int checkstate);
  virtual void PowerChanged_2(int checkstate);
  virtual void PowerChanged_3(int checkstate);
  virtual void PowerChanged_4(int checkstate);
  virtual void PowerChanged_5(int checkstate);
  virtual void PowerChanged_6(int checkstate);
  virtual void PowerChanged_7(int checkstate);


  virtual void SwitchChanged();

  virtual void SetSimpleSwitches(bool on);

  virtual void InverseMapping_changed (int on);

  virtual void BaselineInvert_changed (int on);

  virtual void PlotTabChanged (int num);

  virtual void PulseTimer_changed(int on);
  virtual void PulseFrequencyChanged(int period);
  virtual void PulseBroadcast_changed(int on);

  virtual void PulserTimeout();
  virtual void PulserDisplayTimeout();


  virtual void StartBenchmarkPressed();
  virtual void CancelBenchmarkPressed();
  virtual void ContinueBenchmarkPressed();
  virtual void SaveBenchmarkPressed();

  virtual void BenchmarkPressed(QAbstractButton* but);
  virtual void ChangeReferenceDataPressed(QAbstractButton*);

  virtual void BenchmarkTimerCallback();

  virtual void MaximaCellDoubleClicked(int row, int column);

  virtual void RefreshBaselines();



  virtual void MeasureCurrentsPushButton_clicked ();
  virtual void InitKeithleyPushButton_clicked ();
  virtual void AddressScanPushButton_clicked ();



};


#endif
