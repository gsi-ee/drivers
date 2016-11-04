#include "ApfelGui.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <QProcess>
#include <stdlib.h>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>

#include <sstream>
#include <string.h>
#include <errno.h>


// *********************************************************


// this we need to implement for output of mbspex library, but also useful to format output without it:
ApfelGui* ApfelGui::fInstance = 0;


#include <stdarg.h>


/** JAM The following nice define handles all explicit broadcast actions depending on the currently set slave*/
#define APFEL_BROADCAST_ACTION(X) \
fBroadcasting=true;  \
int oldslave = fSlave; \
int oldchan = fSFP; \
if (AssertNoBroadcast (false)) \
 { \
   X; \
 } \
 else if (fSFP < 0) \
 { \
   for (int sfp = 0; sfp < 4; ++sfp) \
   {\
     if (fSFPChains.numslaves[sfp] == 0) \
       continue; \
     fSFP = sfp; \
     if (fSlave < 0) \
     { \
       for (int feb = 0; feb < fSFPChains.numslaves[sfp]; ++feb) \
       { \
         fSlave = feb; \
         X; \
       } \
     } \
     else \
     { \
       X;\
     }\
   }\
 } \
 else if (fSlave< 0) \
 { \
   for (int feb = 0; feb < fSFPChains.numslaves[fSFP]; ++feb) \
       { \
         fSlave = feb; \
         X; \
       } \
 } \
 else \
 { \
   AppendTextWindow ("--- NEVER COME HERE: undefined broadcast mode ---:"); \
 } \
fSlave= oldslave;\
fSFP= oldchan; \
fBroadcasting=false;




void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
//printf ("%s", c_str);
  ApfelGui::fInstance->AppendTextWindow (c_str);
  ApfelGui::fInstance->FlushTextWindow();
  va_end(args);
}

#ifdef USE_MBSPEX_LIB
/** this one is used to speed down direct mbspex io:*/
void ApfelGui::I2c_sleep ()
{
  //usleep(300);

  usleep(1000); // JAM2016 2x time specified in docu
}

#endif




/*
 *  Constructs a ApfelGui which is a child of 'parent', with the
 *  name 'name'.'
 */
ApfelGui::ApfelGui (QWidget* parent) :
    QWidget (parent), fDebug (false), fSaveConfig (false), fBroadcasting(false), fSFP (0), fSlave (0), fSFPSave (0), fSlaveSave (0),
        fConfigFile (0)
{
  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
#endif

  fNumberBase = 10;

  memset( &fSFPChains, 0, sizeof(struct pex_sfp_links));

  for(int sfp=0; sfp<4;++sfp)
    fSetup[sfp].clear();


  SFPspinBox->setValue (fSFP);
  SlavespinBox->setValue (fSlave);
  DAC_spinBox_all->setValue (500);
  TextOutput->setCenterOnScroll (false);
  ClearOutputBtn_clicked ();

  QObject::connect (RefreshButton, SIGNAL (clicked ()), this, SLOT (ShowBtn_clicked ()));
  QObject::connect (ApplyButton, SIGNAL (clicked ()), this, SLOT (ApplyBtn_clicked ()));

  QObject::connect (InitChainButton, SIGNAL (clicked ()), this, SLOT (InitChainBtn_clicked ()));
  QObject::connect (ResetBoardButton, SIGNAL (clicked ()), this, SLOT (ResetBoardBtn_clicked ()));
  QObject::connect (ResetSlaveButton, SIGNAL (clicked ()), this, SLOT (ResetSlaveBtn_clicked ()));
  QObject::connect (BroadcastButton, SIGNAL (clicked (bool)), this, SLOT (BroadcastBtn_clicked (bool)));
  QObject::connect (DumpButton, SIGNAL (clicked ()), this, SLOT (DumpBtn_clicked ()));
  QObject::connect (ConfigButton, SIGNAL (clicked ()), this, SLOT (ConfigBtn_clicked ()));
  QObject::connect (SaveConfigButton, SIGNAL (clicked ()), this, SLOT (SaveConfigBtn_clicked ()));
  QObject::connect (ClearOutputButton, SIGNAL (clicked ()), this, SLOT (ClearOutputBtn_clicked ()));

  QObject::connect (AutoAdjustButton, SIGNAL (clicked ()), this, SLOT (AutoAdjustBtn_clicked ()));
  QObject::connect (CalibrateADCButton, SIGNAL (clicked ()), this, SLOT (CalibrateADCBtn_clicked ()));
  QObject::connect (CalibrateResetButton, SIGNAL (clicked ()), this, SLOT (CalibrateResetBtn_clicked ()));


  QObject::connect (DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
  QObject::connect (HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));
  QObject::connect (SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
  QObject::connect (SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
  QObject::connect (DAC_spinBox_all, SIGNAL(valueChanged(int)), this, SLOT(DAC_spinBox_all_changed(int)));
  QObject::connect (DAC_spinBox_00, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox00_changed(int)));
  QObject::connect (DAC_spinBox_01, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox01_changed(int))); 
  QObject::connect (DAC_spinBox_02, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox02_changed(int)));
  QObject::connect (DAC_spinBox_03, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox03_changed(int)));
  QObject::connect (DAC_spinBox_04, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox04_changed(int)));
  QObject::connect (DAC_spinBox_05, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox05_changed(int)));
  QObject::connect (DAC_spinBox_06, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox06_changed(int)));
  QObject::connect (DAC_spinBox_07, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox07_changed(int)));
  QObject::connect (DAC_spinBox_08, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox08_changed(int)));
  QObject::connect (DAC_spinBox_09, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox09_changed(int)));
  QObject::connect (DAC_spinBox_10, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox10_changed(int)));
  QObject::connect (DAC_spinBox_11, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox11_changed(int)));
  QObject::connect (DAC_spinBox_12, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox12_changed(int)));
  QObject::connect (DAC_spinBox_13, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox13_changed(int)));
  QObject::connect (DAC_spinBox_14, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox14_changed(int)));
  QObject::connect (DAC_spinBox_15, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox15_changed(int)));


  QObject::connect (Apfel1_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_0(int)));
  QObject::connect (Apfel1_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_1(int)));
  QObject::connect (Apfel1_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_2(int)));
  QObject::connect (Apfel1_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_3(int)));
  QObject::connect (Apfel2_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_0(int)));
  QObject::connect (Apfel2_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_1(int)));
  QObject::connect (Apfel2_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_2(int)));
  QObject::connect (Apfel2_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_3(int)));
  QObject::connect (Apfel3_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_0(int)));
  QObject::connect (Apfel3_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_1(int)));
  QObject::connect (Apfel3_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_2(int)));
  QObject::connect (Apfel3_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_3(int)));
  QObject::connect (Apfel4_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_0(int)));
  QObject::connect (Apfel4_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_1(int)));
  QObject::connect (Apfel4_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_2(int)));
  QObject::connect (Apfel4_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_3(int)));
  QObject::connect (Apfel5_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_0(int)));
  QObject::connect (Apfel5_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_1(int)));
  QObject::connect (Apfel5_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_2(int)));
  QObject::connect (Apfel5_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_3(int)));
  QObject::connect (Apfel6_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_0(int)));
  QObject::connect (Apfel6_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_1(int)));
  QObject::connect (Apfel6_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_2(int)));
  QObject::connect (Apfel6_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_3(int)));
  QObject::connect (Apfel7_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_0(int)));
  QObject::connect (Apfel7_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_1(int)));
  QObject::connect (Apfel7_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_2(int)));
  QObject::connect (Apfel7_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_3(int)));
  QObject::connect (Apfel8_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_0(int)));
  QObject::connect (Apfel8_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_1(int)));
  QObject::connect (Apfel8_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_2(int)));
  QObject::connect (Apfel8_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_3(int)));


  QObject::connect (Apfel1_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_0_0( )));
  QObject::connect (Apfel1_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_0_1( )));
  QObject::connect (Apfel1_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_0_2( )));
  QObject::connect (Apfel1_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_0_3( )));
  QObject::connect (Apfel2_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_1_0( )));
  QObject::connect (Apfel2_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_1_1( )));
  QObject::connect (Apfel2_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_1_2( )));
  QObject::connect (Apfel2_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_1_3( )));
  QObject::connect (Apfel3_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_2_0( )));
  QObject::connect (Apfel3_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_2_1( )));
  QObject::connect (Apfel3_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_2_2( )));
  QObject::connect (Apfel3_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_2_3( )));
  QObject::connect (Apfel4_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_3_0( )));
  QObject::connect (Apfel4_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_3_1( )));
  QObject::connect (Apfel4_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_3_2( )));
  QObject::connect (Apfel4_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_3_3( )));
  QObject::connect (Apfel5_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_4_0( )));
  QObject::connect (Apfel5_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_4_1( )));
  QObject::connect (Apfel5_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_4_2( )));
  QObject::connect (Apfel5_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_4_3( )));
  QObject::connect (Apfel6_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_5_0( )));
  QObject::connect (Apfel6_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_5_1( )));
  QObject::connect (Apfel6_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_5_2( )));
  QObject::connect (Apfel6_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_5_3( )));
  QObject::connect (Apfel7_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_6_0( )));
  QObject::connect (Apfel7_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_6_1( )));
  QObject::connect (Apfel7_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_6_2( )));
  QObject::connect (Apfel7_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_6_3( )));
  QObject::connect (Apfel8_DAClineEdit_1, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_7_0( )));
  QObject::connect (Apfel8_DAClineEdit_2, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_7_1( )));
  QObject::connect (Apfel8_DAClineEdit_3, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_7_2( )));
  QObject::connect (Apfel8_DAClineEdit_4, SIGNAL(editingFinished()), this, SLOT (DAC_enterText_7_3( )));



  QObject::connect (AutocalibrateButton_1, SIGNAL(pressed()), this, SLOT (AutoCalibrate_0()));
  QObject::connect (AutocalibrateButton_2, SIGNAL(pressed()), this, SLOT (AutoCalibrate_1()));
  QObject::connect (AutocalibrateButton_3, SIGNAL(pressed()), this, SLOT (AutoCalibrate_2()));
  QObject::connect (AutocalibrateButton_4, SIGNAL(pressed()), this, SLOT (AutoCalibrate_3()));
  QObject::connect (AutocalibrateButton_5, SIGNAL(pressed()), this, SLOT (AutoCalibrate_4()));
  QObject::connect (AutocalibrateButton_6, SIGNAL(pressed()), this, SLOT (AutoCalibrate_5()));
  QObject::connect (AutocalibrateButton_7, SIGNAL(pressed()), this, SLOT (AutoCalibrate_6()));
  QObject::connect (AutocalibrateButton_8, SIGNAL(pressed()), this, SLOT (AutoCalibrate_7()));

  QObject::connect (AutocalibrateButton_all, SIGNAL(pressed()), this, SLOT (AutoCalibrate_all()));

  QObject::connect (PulserCheckBox_0, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_0()));
  QObject::connect (PulserCheckBox_1, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_0()));
  QObject::connect (ApfelTestPolarityBox_0, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_0()));
  QObject::connect (PulserCheckBox_2, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_1()));
  QObject::connect (PulserCheckBox_3, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_1()));
  QObject::connect (ApfelTestPolarityBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_1()));
  QObject::connect (PulserCheckBox_4, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_2()));
  QObject::connect (PulserCheckBox_5, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_2()));
  QObject::connect (ApfelTestPolarityBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_2()));
  QObject::connect (PulserCheckBox_6, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_3()));
  QObject::connect (PulserCheckBox_7, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_3()));
  QObject::connect (ApfelTestPolarityBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_3()));
  QObject::connect (PulserCheckBox_8, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_4()));
  QObject::connect (PulserCheckBox_9, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_4()));
  QObject::connect (ApfelTestPolarityBox_4, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_4()));
  QObject::connect (PulserCheckBox_10, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_5()));
  QObject::connect (PulserCheckBox_11, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_5()));
  QObject::connect (ApfelTestPolarityBox_5, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_5()));
  QObject::connect (PulserCheckBox_12, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_6()));
  QObject::connect (PulserCheckBox_13, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_6()));
  QObject::connect (ApfelTestPolarityBox_6, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_6()));
  QObject::connect (PulserCheckBox_14, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_7()));
  QObject::connect (PulserCheckBox_15, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_7()));
  QObject::connect (ApfelTestPolarityBox_7, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_7()));


  QObject::connect (gainCombo_0, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_0()));
  QObject::connect (gainCombo_1, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_1()));
  QObject::connect (gainCombo_2, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_2()));
  QObject::connect (gainCombo_3, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_3()));
  QObject::connect (gainCombo_4, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_4()));
  QObject::connect (gainCombo_5, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_5()));
  QObject::connect (gainCombo_6, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_6()));
  QObject::connect (gainCombo_7, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_7()));
  QObject::connect (gainCombo_8, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_8()));
  QObject::connect (gainCombo_9, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_9()));
  QObject::connect (gainCombo_10, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_10()));
  QObject::connect (gainCombo_11, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_11()));
  QObject::connect (gainCombo_12, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_12()));
  QObject::connect (gainCombo_13, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_13()));
  QObject::connect (gainCombo_14, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_14()));
  QObject::connect (gainCombo_15, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_15()));



  QObject::connect (ApfelRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));
  QObject::connect (LoGainRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));
  QObject::connect (StretcherOnRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));

  QObject::connect (InverseMappingCheckBox, SIGNAL(stateChanged(int)), this, SLOT (InverseMapping_changed(int)));

  /** JAM put references to designer checkboxes into array to be handled later easily: */
  fBaselineBoxes[0]=Baseline_Box_00;
  fBaselineBoxes[1]=Baseline_Box_01;
  fBaselineBoxes[2]=Baseline_Box_02;
  fBaselineBoxes[3]=Baseline_Box_03;
  fBaselineBoxes[4]=Baseline_Box_04;
  fBaselineBoxes[5]=Baseline_Box_05;
  fBaselineBoxes[6]=Baseline_Box_06;
  fBaselineBoxes[7]=Baseline_Box_07;
  fBaselineBoxes[8]=Baseline_Box_08;
  fBaselineBoxes[9]=Baseline_Box_09;
  fBaselineBoxes[10]=Baseline_Box_10;
  fBaselineBoxes[11]=Baseline_Box_11;
  fBaselineBoxes[12]=Baseline_Box_12;
  fBaselineBoxes[13]=Baseline_Box_13;
  fBaselineBoxes[14]=Baseline_Box_14;
  fBaselineBoxes[15]=Baseline_Box_15;

  fDACSpinBoxes[0] = DAC_spinBox_00;
  fDACSpinBoxes[1] = DAC_spinBox_01;
  fDACSpinBoxes[2] = DAC_spinBox_02;
  fDACSpinBoxes[3] = DAC_spinBox_03;
  fDACSpinBoxes[4] = DAC_spinBox_04;
  fDACSpinBoxes[5] = DAC_spinBox_05;
  fDACSpinBoxes[6] = DAC_spinBox_06;
  fDACSpinBoxes[7] = DAC_spinBox_07;
  fDACSpinBoxes[8] = DAC_spinBox_08;
  fDACSpinBoxes[9] = DAC_spinBox_09;
  fDACSpinBoxes[10] = DAC_spinBox_10;
  fDACSpinBoxes[11] = DAC_spinBox_11;
  fDACSpinBoxes[12] = DAC_spinBox_12;
  fDACSpinBoxes[13] = DAC_spinBox_13;
  fDACSpinBoxes[14] = DAC_spinBox_14;
  fDACSpinBoxes[15] = DAC_spinBox_15;

  fADCLineEdit[0] = ADC_Value_00;
  fADCLineEdit[1] = ADC_Value_01;
  fADCLineEdit[2] = ADC_Value_02;
  fADCLineEdit[3] = ADC_Value_03;
  fADCLineEdit[4] = ADC_Value_04;
  fADCLineEdit[5] = ADC_Value_05;
  fADCLineEdit[6] = ADC_Value_06;
  fADCLineEdit[7] = ADC_Value_07;
  fADCLineEdit[8] = ADC_Value_08;
  fADCLineEdit[9] = ADC_Value_09;
  fADCLineEdit[10] = ADC_Value_10;
  fADCLineEdit[11] = ADC_Value_11;
  fADCLineEdit[12] = ADC_Value_12;
  fADCLineEdit[13] = ADC_Value_13;
  fADCLineEdit[14] = ADC_Value_14;
  fADCLineEdit[15] = ADC_Value_15;


  fDACSlider[0][0] = Apfel1_DACSlider_1;
  fDACSlider[0][1] = Apfel1_DACSlider_2;
  fDACSlider[0][2] = Apfel1_DACSlider_3;
  fDACSlider[0][3] = Apfel1_DACSlider_4;
  fDACSlider[1][0] = Apfel2_DACSlider_1;
  fDACSlider[1][1] = Apfel2_DACSlider_2;
  fDACSlider[1][2] = Apfel2_DACSlider_3;
  fDACSlider[1][3] = Apfel2_DACSlider_4;
  fDACSlider[2][0] = Apfel3_DACSlider_1;
  fDACSlider[2][1] = Apfel3_DACSlider_2;
  fDACSlider[2][2] = Apfel3_DACSlider_3;
  fDACSlider[2][3] = Apfel3_DACSlider_4;
  fDACSlider[3][0] = Apfel4_DACSlider_1;
  fDACSlider[3][1] = Apfel4_DACSlider_2;
  fDACSlider[3][2] = Apfel4_DACSlider_3;
  fDACSlider[3][3] = Apfel4_DACSlider_4;
  fDACSlider[4][0] = Apfel5_DACSlider_1;
  fDACSlider[4][1] = Apfel5_DACSlider_2;
  fDACSlider[4][2] = Apfel5_DACSlider_3;
  fDACSlider[4][3] = Apfel5_DACSlider_4;
  fDACSlider[5][0] = Apfel6_DACSlider_1;
  fDACSlider[5][1] = Apfel6_DACSlider_2;
  fDACSlider[5][2] = Apfel6_DACSlider_3;
  fDACSlider[5][3] = Apfel6_DACSlider_4;
  fDACSlider[6][0] = Apfel7_DACSlider_1;
  fDACSlider[6][1] = Apfel7_DACSlider_2;
  fDACSlider[6][2] = Apfel7_DACSlider_3;
  fDACSlider[6][3] = Apfel7_DACSlider_4;
  fDACSlider[7][0] = Apfel8_DACSlider_1;
  fDACSlider[7][1] = Apfel8_DACSlider_2;
  fDACSlider[7][2] = Apfel8_DACSlider_3;
  fDACSlider[7][3] = Apfel8_DACSlider_4;

  fDACLineEdit[0][0]= Apfel1_DAClineEdit_1;
  fDACLineEdit[0][1]= Apfel1_DAClineEdit_2;
  fDACLineEdit[0][2]= Apfel1_DAClineEdit_3;
  fDACLineEdit[0][3]= Apfel1_DAClineEdit_4;
  fDACLineEdit[1][0]= Apfel2_DAClineEdit_1;
  fDACLineEdit[1][1]= Apfel2_DAClineEdit_2;
  fDACLineEdit[1][2]= Apfel2_DAClineEdit_3;
  fDACLineEdit[1][3]= Apfel2_DAClineEdit_4;
  fDACLineEdit[2][0]= Apfel3_DAClineEdit_1;
  fDACLineEdit[2][1]= Apfel3_DAClineEdit_2;
  fDACLineEdit[2][2]= Apfel3_DAClineEdit_3;
  fDACLineEdit[2][3]= Apfel3_DAClineEdit_4;
  fDACLineEdit[3][0]= Apfel4_DAClineEdit_1;
  fDACLineEdit[3][1]= Apfel4_DAClineEdit_2;
  fDACLineEdit[3][2]= Apfel4_DAClineEdit_3;
  fDACLineEdit[3][3]= Apfel4_DAClineEdit_4;
  fDACLineEdit[4][0]= Apfel5_DAClineEdit_1;
  fDACLineEdit[4][1]= Apfel5_DAClineEdit_2;
  fDACLineEdit[4][2]= Apfel5_DAClineEdit_3;
  fDACLineEdit[4][3]= Apfel5_DAClineEdit_4;
  fDACLineEdit[5][0]= Apfel6_DAClineEdit_1;
  fDACLineEdit[5][1]= Apfel6_DAClineEdit_2;
  fDACLineEdit[5][2]= Apfel6_DAClineEdit_3;
  fDACLineEdit[5][3]= Apfel6_DAClineEdit_4;
  fDACLineEdit[6][0]= Apfel7_DAClineEdit_1;
  fDACLineEdit[6][1]= Apfel7_DAClineEdit_2;
  fDACLineEdit[6][2]= Apfel7_DAClineEdit_3;
  fDACLineEdit[6][3]= Apfel7_DAClineEdit_4;
  fDACLineEdit[7][0]= Apfel8_DAClineEdit_1;
  fDACLineEdit[7][1]= Apfel8_DAClineEdit_2;
  fDACLineEdit[7][2]= Apfel8_DAClineEdit_3;
  fDACLineEdit[7][3]= Apfel8_DAClineEdit_4;

  fApfelPulsePolarityCombo[0] = ApfelTestPolarityBox_0;
  fApfelPulsePolarityCombo[1] = ApfelTestPolarityBox_1;
  fApfelPulsePolarityCombo[2] = ApfelTestPolarityBox_2;
  fApfelPulsePolarityCombo[3] = ApfelTestPolarityBox_3;
  fApfelPulsePolarityCombo[4] = ApfelTestPolarityBox_4;
  fApfelPulsePolarityCombo[5] = ApfelTestPolarityBox_5;
  fApfelPulsePolarityCombo[6] = ApfelTestPolarityBox_6;
  fApfelPulsePolarityCombo[7] = ApfelTestPolarityBox_7;

  fApfelPulseEnabledCheckbox[0][0] = PulserCheckBox_0;
  fApfelPulseEnabledCheckbox[0][1] = PulserCheckBox_1;
  fApfelPulseEnabledCheckbox[1][0] = PulserCheckBox_2;
  fApfelPulseEnabledCheckbox[1][1] = PulserCheckBox_3;
  fApfelPulseEnabledCheckbox[2][0] = PulserCheckBox_4;
  fApfelPulseEnabledCheckbox[2][1] = PulserCheckBox_5;
  fApfelPulseEnabledCheckbox[3][0] = PulserCheckBox_6;
  fApfelPulseEnabledCheckbox[3][1] = PulserCheckBox_7;
  fApfelPulseEnabledCheckbox[4][0] = PulserCheckBox_8;
  fApfelPulseEnabledCheckbox[4][1] = PulserCheckBox_9;
  fApfelPulseEnabledCheckbox[5][0] = PulserCheckBox_10;
  fApfelPulseEnabledCheckbox[5][1] = PulserCheckBox_11;
  fApfelPulseEnabledCheckbox[6][0] = PulserCheckBox_12;
  fApfelPulseEnabledCheckbox[6][1] = PulserCheckBox_13;
  fApfelPulseEnabledCheckbox[7][0] = PulserCheckBox_14;
  fApfelPulseEnabledCheckbox[7][1] = PulserCheckBox_15;

  fApfelGainCombo[0][0] = gainCombo_0;
  fApfelGainCombo[0][1] = gainCombo_1;
  fApfelGainCombo[1][0] = gainCombo_2;
  fApfelGainCombo[1][1] = gainCombo_3;
  fApfelGainCombo[2][0] = gainCombo_4;
  fApfelGainCombo[2][1] = gainCombo_5;
  fApfelGainCombo[3][0] = gainCombo_6;
  fApfelGainCombo[3][1] = gainCombo_7;
  fApfelGainCombo[4][0] = gainCombo_8;
  fApfelGainCombo[4][1] = gainCombo_9;
  fApfelGainCombo[5][0] = gainCombo_10;
  fApfelGainCombo[5][1] = gainCombo_11;
  fApfelGainCombo[6][0] = gainCombo_12;
  fApfelGainCombo[6][1] = gainCombo_13;
  fApfelGainCombo[7][0] = gainCombo_14;
  fApfelGainCombo[7][1] = gainCombo_15;




#ifdef USE_MBSPEX_LIB
// open handle to driver file:
  fPexFD = mbspex_open (0);    // we restrict to board number 0 here
  if (fPexFD < 0)
  {
    printm ("ERROR>> open /dev/pexor%d \n", 0);
    exit (1);
  }
#endif
  fInstance = this;
  show ();


  // start with preferred situation:
  ShowBtn_clicked();
  checkBox_AA->setChecked (true);

}

ApfelGui::~ApfelGui ()
{
#ifdef USE_MBSPEX_LIB
  mbspex_close (fPexFD);
#endif
}

void ApfelGui::ShowBtn_clicked ()
{
  //std::cout << "ApfelGui::ShowBtn_clicked()"<< std::endl;
  EvaluateSlave ();
  GetSFPChainSetup();

  if(!AssertNoBroadcast(false)) return;
  if(!AssertChainConfigured()) return;
  GetRegisters ();
  RefreshView ();
}

void ApfelGui::ApplyBtn_clicked ()
{
//std::cout << "ApfelGui::ApplyBtn_clicked()"<< std::endl;


  EvaluateSlave ();

// JAM maybe disable confirm window ?
//  char buffer[1024];
//  snprintf (buffer, 1024, "Really apply APFEL settings  to SFP %d Device %d?", fSFP, fSlave);
//  if (QMessageBox::question (this, "APFEL GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
//      != QMessageBox::Yes)
//  {
//    return;
//  }


  GetSFPChainSetup();
  if(AssertNoBroadcast(false) && !AssertChainConfigured()) return;

  // JAM: since we keep all slave set ups in vector/array, we must handle broadcast mode explicitely
  // no implicit driver broadcast via -1 indices anymore!
  APFEL_BROADCAST_ACTION(ApplyGUISettings());


}

void ApfelGui::ApplyGUISettings()
{
  EvaluateView(); // from gui to memory
  SetRegisters(); // from memory to device
}

void ApfelGui::SaveConfigBtn_clicked ()
{
//std::cout << "ApfelGui::SaveConfigBtn_clicked()"<< std::endl;

  static char buffer[1024];
  QString gos_filter ("gosipcmd file (*.gos)");
  QStringList filters;
  filters << gos_filter;

  QFileDialog fd (this, "Write Apfel configuration file");

  // ".", "nyxor setup file (*.txt);;gosipcmd file (*.gos);;context dump file (*.dmp)");
  fd.setNameFilters (filters);
  fd.setFileMode (QFileDialog::AnyFile);
  fd.setAcceptMode (QFileDialog::AcceptSave);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  QString fileName = flst[0];

  // complete suffix if user did not
  if (fd.selectedNameFilter () == gos_filter)
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");
  }
  else
  {
    std::cout << "ApfelGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
  }

  // open file
  if (OpenConfigFile (fileName) != 0)
    return;

  if (fileName.endsWith (".gos"))
  {
    WriteConfigFile (QString ("#Format *.gos"));
    WriteConfigFile (QString ("#usage: gosipcmd -x -c file.gos \n"));
    WriteConfigFile (QString ("#                                         \n"));
    WriteConfigFile (QString ("#sfp slave address value\n"));
    APFEL_BROADCAST_ACTION(SaveRegisters()); // refresh actual setup from hardware and write it to open file
  }


  else
  {
    std::cout << "ApfelGui::SaveConfigBtn_clicked( -  unknown file type, NEVER COME HERE!!!!)" << std::endl;
  }

  // close file
  CloseConfigFile ();
  snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
  AppendTextWindow (buffer);
}

int ApfelGui::OpenConfigFile (const QString& fname)
{
  fConfigFile = fopen (fname.toLatin1 ().constData (), "w");
  if (fConfigFile == NULL)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error opening Configuration File '%s': %s\n", fname.toLatin1 ().constData (),
        strerror (errno));
    AppendTextWindow (buffer);
    return -1;
  }
  QString timestring = QDateTime::currentDateTime ().toString ("ddd dd.MM.yyyy hh:mm:ss");
  WriteConfigFile (QString ("# Apfel configuration file saved on ") + timestring + QString ("\n"));
  return 0;
}

int ApfelGui::CloseConfigFile ()
{
  int rev = 0;
  if (fConfigFile == NULL)
    return 0;
  if (fclose (fConfigFile) != 0)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error closing Configuration File! (%s)\n", strerror (errno));
    AppendTextWindow (buffer);
    rev = -1;
  }
  fConfigFile = NULL;    // must not use handle again even if close fails
  return rev;
}

int ApfelGui::WriteConfigFile (const QString& text)
{
  if (fConfigFile == NULL)
    return -1;
  if (fprintf (fConfigFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}


void ApfelGui::InitChainBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
//std::cout << "InitChainBtn_clicked()"<< std::endl;
  bool ok;
  snprintf (buffer, 1024, "Please specify NUMBER OF DEVICES to initialize at SFP %d ?", fSFP);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  int numslaves = QInputDialog::getInt (this, tr ("Number of Slaves?"), tr (buffer), 1, 1, 1024, 1, &ok);
#else
  int numslaves = QInputDialog::getInteger(this, tr("Number of Slaves?"),
      tr(buffer), 1, 1, 1024, 1, &ok);

#endif
  if (!ok)
    return;
  if (fSFP < 0)
  {
    AppendTextWindow ("--- Error: Broadcast not allowed for init chain!");
    return;
  }
#ifdef USE_MBSPEX_LIB
  int rev = mbspex_slave_init (fPexFD, fSFP, numslaves);

#else
  snprintf (buffer, 1024, "gosipcmd -i  %d %d", fSFP, numslaves);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
#endif

  GetSFPChainSetup();
  RefreshChains();
}

void ApfelGui::ResetBoardBtn_clicked ()
{
//std::cout << "ApfelGui::ResetBoardBtn_clicked"<< std::endl;
  if (QMessageBox::question (this, "APFEL GUI", "Really Reset gosip on pex board?", QMessageBox::Yes | QMessageBox::No,
      QMessageBox::Yes) != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }
#ifdef USE_MBSPEX_LIB
  mbspex_reset (fPexFD);
  AppendTextWindow ("Reset PEX board with mbspex_reset()");

#else

  char buffer[1024];
  snprintf (buffer, 1024, "gosipcmd -z");
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);

#endif

  GetSFPChainSetup();
  RefreshChains();
}

void ApfelGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
  snprintf (buffer, 1024, "Really initialize APFEL device at SFP %d, Slave %d ?", fSFP, fSlave);
  if (QMessageBox::question (this, "Apfel GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }
  APFEL_BROADCAST_ACTION(InitApfel());


}
void ApfelGui::EnableI2C ()
{
#ifdef APFEL_NEED_ENABLEI2C
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000080);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2000020);
#endif
}

void ApfelGui::DisableI2C ()
{
#ifdef APFEL_NEED_ENABLEI2C
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000000);
#endif
}


void ApfelGui::InitApfel()
{
  printm("Resetting APFEL for SFP %d Slave %d...",fSFP,fSlave);
  WriteGosip (fSFP, fSlave,GOS_I2C_DWR,APFEL_RESET);
  sleep(1);


#ifdef DO_APFEL_INIT
  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, 0x00);
  usleep (4000);

//        // disable test data length
  WriteGosip (fSFP, fSlave, REG_DATA_LEN, 0x10000000);

//        // specify trace length in slices
  WriteGosip (fSFP, fSlave, REG_FEB_TRACE_LEN, FEB_TRACE_LEN);
  // note: we skip verify read here to let this work in broadcast mode!




//        // specify trigger delay in slices
  WriteGosip (fSFP, fSlave,  REG_FEB_TRIG_DELAY, FEB_TRIG_DELAY);
  // note: we skip verify read here to let this work in broadcast mode!


  //        // disable trigger acceptance in febex
  WriteGosip (fSFP, fSlave,   REG_FEB_CTRL, 0);


  //        // enable trigger acceptance in febex
  WriteGosip (fSFP, fSlave,   REG_FEB_CTRL, 1);


//        // set channels used for self trigger signal

  // JAM: the following is reduced version of mbs sample code. instead of arrays for each slave, we just
  // take settings for first device at sfp 0. Should be sufficient for baseline setup until mbs configures all?
  long l_sfp0_feb_ctrl0= 0x01000000;
  long l_sfp0_feb_ctrl1 = 0x0;
  long l_sfp0_feb_ctrl2= 0xffff;
  long l_ev_od_or = (l_sfp0_feb_ctrl0 >> 20) & 0x1;
  long l_pol = (l_sfp0_feb_ctrl0 >> 28) & 0x1;
  long l_trig_mod = (l_sfp0_feb_ctrl0 >> 24) & 0xf;
  long l_dis_cha  =  l_sfp0_feb_ctrl0       & 0x1ffff;
  long l_dat_redu =  l_sfp0_feb_ctrl1       & 0x1ffff;
  long l_ena_trig =  l_sfp0_feb_ctrl2       & 0xffff;


  int trigenabchan= ((l_ev_od_or<<21)|(l_pol<<20)|(l_trig_mod<<16)|l_ena_trig);
  WriteGosip (fSFP, fSlave,  REG_FEB_SELF_TRIG, trigenabchan);



//        // set the step size for self trigger and data reduction
  long l_thresh=0x1ff;
  for (int l_k=0; l_k < APFEL_ADC_CHANNELS ; l_k++){
    WriteGosip (fSFP, fSlave, REG_FEB_STEP_SIZE, ( l_k<<24 ) | l_thresh );
  }




  //
//        // reset the time stamp and set the clock source for time stamp counter
  if(fSFP==0 && fSlave==0) // assume clock source at first slave on first chain here
  {
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x0 );
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x7 );
  }
  else
  {
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x0 );
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x5 );
  }




  //
//        // enable/disable no hit in trace data suppression of channel
  WriteGosip (fSFP, fSlave,  REG_DATA_REDUCTION, l_dat_redu);

//        // set channels used for self trigger signal
  WriteGosip (fSFP, fSlave,  REG_MEM_DISABLE, l_dis_cha );

//        // write SFP id for channel header

  WriteGosip (fSFP, fSlave,  REG_HEADER, fSFP);



//        // set trapez parameters for trigger/hit finding
  WriteGosip (fSFP, fSlave,  TRIG_SUM_A_REG, TRIG_SUM_A);
  WriteGosip (fSFP, fSlave,   TRIG_GAP_REG, TRIG_SUM_A + TRIG_GAP);
  WriteGosip (fSFP, fSlave,   TRIG_SUM_B_REG, TRIG_SUM_A  + TRIG_GAP + TRIG_SUM_B );


#ifdef ENABLE_ENERGY_FILTER
#ifdef TRAPEZ
//
//        // set trapez parameters for energy estimation
  WriteGosip (fSFP, fSlave,  ENERGY_SUM_A_REG, ENERGY_SUM_A);
  WriteGosip (fSFP, fSlave,  ENERGY_GAP_REG, ENERGY_SUM_A + ENERGY_GAP);
  WriteGosip (fSFP, fSlave,  ENERGY_SUM_B_REG, ENERGY_SUM_A  + ENERGY_GAP + ENERGY_SUM_B );


#endif // TRAPEZ
#endif // ENABLE_ENERGY_FILTER
  usleep(50);
// enabling after "ini" of all registers (Ivan - 16.01.2013):
  WriteGosip (fSFP, fSlave,   DATA_FILT_CONTROL_REG, DATA_FILT_CONTROL_DAT);
  sleep (1);
  printm("Did Initialize APFEL for SFP %d Slave %d",fSFP,fSlave);
#else
  printm("Did NOT Initializing APFEL for SFP %d Slave %d, feature is disabled!",fSFP,fSlave);
#endif
}



void ApfelGui::AutoAdjustBtn_clicked ()
{
  //std::cout <<"AutoAdjustBtn_clicked "<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  APFEL_BROADCAST_ACTION(AutoAdjust());
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::AutoAdjust()
{
  if(!AssertChainConfigured()) return;
   QString targetstring=ADCAdjustValue->text ();
   unsigned targetvalue =targetstring.toUInt (0, fNumberBase);
   //std::cout <<"string="<<targetstring.toLatin1 ().constData ()<< ", targetvalue="<< targetvalue<< std::endl;
   for(int channel=0; channel<16;++channel)
     {
       if(fBaselineBoxes[channel]->isChecked())
       {
           int dac=AdjustBaseline(channel,targetvalue);
           fDACSpinBoxes[channel]->setValue (dac);
           AutoApplyRefresh(channel, dac); // once again apply dac settings to immediately see the baseline on gui
           printm("--- Auto adjusted baselines of sfp:%d board:%d channel:%d to value:%d =>%d permille DAC",fSFP, fSlave,channel, targetvalue, dac);
       }
    }
}





int ApfelGui::AdjustBaseline(int channel, int adctarget)
{
  int dac=500; // dac setup in per mille here, start in the middle
  int dacstep=250;
  int validdac=-1;
  int adc=0;
  int escapecounter=10;
  bool upwards=true; // scan direction up or down
  bool changed=false; // do we have changed scan direction?
  bool initial=true; // supress evaluation of scan direction at first cycle
  //std::cout << "ApfelGui::AdjustBaseline of channel "<<channel<<" to "<<adctarget<< std::endl;

  double resolution= 1.0/APFEL_DAC_MAXVALUE * 0x3FFF /2; // for 14 bit febex ADC

  do{
     adc=autoApply(channel, dac); // this gives already mean value of 3 adc samples
     if(adc<0) break; // avoid broadcast
     validdac=dac;
     //std::cout << "ApfelGui::AdjustBaseline after autoApply of dac:"<<dac<<" gets adc:"<<adc<<", resolution:"<<resolution<< std::endl;
     if(adc<adctarget){
       dac+=dacstep;
       changed=(!upwards ? true : false);
       upwards=true;
       if(dac>1000) dac = 1000;
     }
     else if (adc>adctarget){
       dac-=dacstep;
       changed=(upwards ? true : false);
       upwards=false;
       if(dac<0) dac=0;
     }
     else break;
     if(changed && !initial && dacstep > 1) dacstep = dacstep >> 1;
     if(dacstep <1) break;
     if(!changed || dacstep==1) escapecounter--; // get us out of loop if user wants to reach value outside adc range, or if we oscillate around target value
     initial=false;
  } while (fabs(adc-adctarget) >= resolution && escapecounter);
  //std::cout << "   ApfelGui::AdjustBaseline after loop dac:"<<validdac<<" adc:"<<adc<<", resolution:"<<resolution<< std::endl;
  return validdac;
}



void ApfelGui::CalibrateADCBtn_clicked()
{
  //std::cout <<"CalibrateADCBtn_clicked"<< std::endl;
   EvaluateSlave ();
   QApplication::setOverrideCursor (Qt::WaitCursor);
   APFEL_BROADCAST_ACTION(CalibrateSelectedADCs());
   QApplication::restoreOverrideCursor ();
}

void ApfelGui::CalibrateSelectedADCs()
{
  if(!AssertChainConfigured()) return;
   QString targetstring=ADCAdjustValue->text ();
   unsigned targetvalue =targetstring.toUInt (0, fNumberBase);
   //std::cout <<"string="<<targetstring.toLatin1 ().constData ()<< ", targetvalue="<< targetvalue<< std::endl;
   for(int channel=0; channel<16;++channel)
     {
       if(fBaselineBoxes[channel]->isChecked())
       {
           CalibrateADC(channel);
       }
    }
}


int ApfelGui::CalibrateADC(int channel)
{
  BoardSetup& theSetup=fSetup[fSFP].at (fSlave);

  int apfel=0, dac=0;
  theSetup.EvaluateDACIndices(channel, apfel, dac);

  // DO NOT first autocalibrate DAC that belongs to selected channel
  // we decouple chip autocalibration from channel calibration curve now
  //DoAutoCalibrate(apfel);

  // measure slope of DAC kennlinie by differential variation:
  int gain=theSetup.GetGain(apfel,dac);
  int deltaDAC=APFEL_DAC_DELTACALIB;
  int valDAC=theSetup.GetDACValue(apfel, dac); // current value after automatic DAC calibration
  int valADC=  AcquireBaselineSample(channel); // fetch a sample
  int valDAC_upper=valDAC+deltaDAC;

  // now do variation and measure new adc value:
  EnableI2C ();
  WriteDAC_ApfelI2c (apfel, dac, valDAC_upper);
  int valADC_upper=AcquireBaselineSample(channel);
  int deltaADC=valADC_upper-valADC;

  // shift back to middle of calibration:
  WriteDAC_ApfelI2c (apfel, dac, valDAC);
  DisableI2C ();
  printm("--- Calibrated DAC->ADC slider for sfp:%d board:%d channel:%d apfel:%d dac:%d -",fSFP, fSlave, channel, apfel, dac);
  theSetup.EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC, valADC);

  // finally, refresh display of currently calibrated adc channel:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  valADC=  AcquireBaselineSample(channel);
  fADCLineEdit[channel]->setText (pre + text.setNum (valADC, fNumberBase));
  RefreshStatus ();
  return 0;
}


void ApfelGui::CalibrateResetBtn_clicked()
{
    //std::cout <<"CalibrateResetBtn_clicked"<< std::endl;
    EvaluateSlave ();
    QApplication::setOverrideCursor (Qt::WaitCursor);
    APFEL_BROADCAST_ACTION(CalibrateResetSelectedADCs());
    QApplication::restoreOverrideCursor ();
}

void ApfelGui::CalibrateResetSelectedADCs()
{
  if(!AssertChainConfigured()) return;
   QString targetstring=ADCAdjustValue->text ();
   unsigned targetvalue =targetstring.toUInt (0, fNumberBase);
   //std::cout <<"string="<<targetstring.toLatin1 ().constData ()<< ", targetvalue="<< targetvalue<< std::endl;
   for(int channel=0; channel<16;++channel)
     {
       if(fBaselineBoxes[channel]->isChecked())
       {
           CalibrateResetADC(channel);
       }
    }
}

int ApfelGui::CalibrateResetADC(int channel)
{
  BoardSetup& theSetup=fSetup[fSFP].at (fSlave);
  // TODO: first autocalibrate DAC that belongs to selected channel
 int apfel=0, dac=0;
 theSetup.EvaluateDACIndices(channel, apfel, dac);
 int gain=theSetup.GetGain(apfel,dac);
 theSetup.ResetCalibration(gain,channel);
 printm("--- Reset Calibration of DAC->ADC slider for sfp:%d board:%d channel:%d apfel:%d dac:%d",fSFP, fSlave, channel, apfel, dac);
 return 0;

}


void ApfelGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "ApfelGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
  if (checked)
  {
    fSFPSave = SFPspinBox->value ();
    fSlaveSave = SlavespinBox->value ();
    SFPspinBox->setValue (-1);
    SlavespinBox->setValue (-1);
  }
  else
  {
    RefreshChains();
    SFPspinBox->setValue (fSFPSave);
    SlavespinBox->setValue (fSlaveSave);

  }
}

void ApfelGui::DumpADCs()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if(!AssertChainConfigured()) return;

    printm("SFP %d DEV:%d :)",fSFP, fSlave);
    for(int adc=0; adc<APFEL_ADC_NUMADC; ++adc){
      for (int chan=0; chan<APFEL_ADC_NUMCHAN; ++chan){
        int val=ReadADC_Apfel(adc,chan);
        if(val<0)
          printm("Read error for adc:%d chan:%d",adc,chan);
        else
          {
            if(fNumberBase==16)
              printm("Val (adc:0x%x chan:0x%x)=0x%x",adc,chan,val);
            else
              printm("Val (adc:%d chan:%d)=%d",adc,chan,val);
        }
      }
    }

    DumpCalibrations(); // later put to separate button
}

void ApfelGui::DumpCalibrations()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if(!AssertChainConfigured()) return;
  BoardSetup& theSetup= fSetup[fSFP].at(fSlave);
    printm("SFP %d DEV:%d : Dump calibration)",fSFP, fSlave);
    int apfel=0, dac=0;
      for (int febchan=0; febchan<APFEL_ADC_CHANNELS; ++febchan){
        theSetup.EvaluateDACIndices(febchan, apfel, dac);
        int gain=theSetup.GetGain(apfel,dac);
        theSetup.DumpCalibration(gain,febchan);
      }

}

void ApfelGui::DumpBtn_clicked ()
{
//std::cout << "ApfelGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
  EvaluateSlave ();
  AppendTextWindow ("--- ADC Dump ---:");
  APFEL_BROADCAST_ACTION(DumpADCs());

}



void ApfelGui::ClearOutputBtn_clicked ()
{
//std::cout << "ApfelGui::ClearOutputBtn_clicked()"<< std::endl;
  TextOutput->clear ();
  TextOutput->setPlainText ("Welcome to APFEL GUI!\n\t v0.900 of 4-November-2016 by JAM (j.adamczewski@gsi.de)\n");

}

void ApfelGui::ConfigBtn_clicked ()
{
//std::cout << "ApfelGui::ConfigBtn_clicked" << std::endl;

// here file requester and application of set up via gosipcmd
  QFileDialog fd (this, "Select APFEL configuration file", ".", "gosipcmd file (*.gos)");
  fd.setFileMode (QFileDialog::ExistingFile);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  char buffer[1024];
  // JAM: need to increase default bus wait time to 900us first for febex i2c!
  snprintf (buffer, 1024, "setGosipwait.sh 900"); // output redirection inside QProcess does not work, use helper script
  QString tcom (buffer);
  QString tresult=ExecuteGosipCmd (tcom, 10000);
  AppendTextWindow (tresult);
  QString fileName = flst[0];
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");
    snprintf (buffer, 1024, "gosipcmd -x -c %s ", fileName.toLatin1 ().constData ());
  }
  QString com (buffer);
  QString result = ExecuteGosipCmd (com, 10000);    // this will just execute the command in shell, gosip or not
  AppendTextWindow (result);

  snprintf (buffer, 1024, "setGosipwait.sh 0"); // set back to zero bus wait since we have explicit i2c_sleep elsewhere!
  QString zcom (buffer);
  QString zresult=ExecuteGosipCmd (zcom, 10000);
  AppendTextWindow (zresult);

  ShowBtn_clicked() ;
}

void ApfelGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
  fDebug = on;
}

void ApfelGui::HexBox_changed (int on)
{

  unsigned adjustvalue = ADCAdjustValue->text ().toUInt (0, fNumberBase);    // save value in auto adjust field
  fNumberBase = (on ? 16 : 10);
//std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;

  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  ADCAdjustValue->setText (pre + text.setNum (adjustvalue, fNumberBase));    // recover with new base

  int oldsfp = fSFP;
  int oldslave = fSlave;

  if (!AssertNoBroadcast (false))
  {
    fSFP = fSFPSave;
    fSlave = fSlaveSave;
  }
  RefreshView (); // need to workaround the case that any broadcast was set. however, we do not need full APFEL_BROADCAST_ACTION
  if (!AssertNoBroadcast (false))
  {
    fSFP = oldsfp;
    fSlave = oldslave;
  }

}

void ApfelGui::Slave_changed (int)
{
  //std::cout << "ApfelGui::Slave_changed" << std::endl;
  EvaluateSlave ();
  bool refreshable = AssertNoBroadcast (false);
  RefreshButton->setEnabled (refreshable);

  RefreshChains();
  //if(checkBox_AA->isChecked() && refreshable)
  if(refreshable)
  {
    // JAM note that we had a problem of prelling spinbox here (arrow buttons only, keyboard arrows are ok)
    // probably caused by too long response time of this slot?
    // workaround is to refresh the view delayed per single shot timer:
    //std::cout << "Timer started" << std::endl;
    QTimer::singleShot(10, this, SLOT(ShowBtn_clicked()));
    //std::cout << "Timer end" << std::endl;
    //ShowBtn_clicked() ;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////7
//////////////////////////////////////////////////////////////////////////////////////////////////////


void ApfelGui::AutoApplySwitch()
{
  EvaluateIOSwitch();
  SetIOSwitch();
}


 void ApfelGui::SwitchChanged ()
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplySwitch());
  }
}


void ApfelGui::AutoApplyPulser(int apfel)
{
  EvaluatePulser(apfel);
  SetPulser(apfel);
}

void ApfelGui::PulserChanged(int apfel)
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplyPulser(apfel));
  }

}


void ApfelGui::PulserChanged_0()
{
  PulserChanged(0);
}

void ApfelGui::PulserChanged_1()
{
  PulserChanged(1);
}

void ApfelGui::PulserChanged_2()
{
  PulserChanged(2);
}

void ApfelGui::PulserChanged_3()
{
  PulserChanged(3);
}

void ApfelGui::PulserChanged_4()
{
  PulserChanged(4);
}

void ApfelGui::PulserChanged_5()
{
  PulserChanged(5);
}

void ApfelGui::PulserChanged_6()
{
  PulserChanged(6);
}

void ApfelGui::PulserChanged_7()
{
  PulserChanged(7);
}




void ApfelGui::AutoApplyGain(int apfel, int channel)
 {
    BoardSetup& theSetup= fSetup[fSFP].at(fSlave);
    EvaluateGain(apfel, channel);
    //std::cout << "AutoApplyGain apfel="<<apfel<<", channel="<<channel<<", lowgain:"<< theSetup.GetLowGain (apfel, channel)<<std::endl;
    SetGain (apfel, channel, theSetup.GetLowGain (apfel, channel));
 }


void ApfelGui::GainChanged (int apfel, int channel)
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplyGain(apfel,channel));
  }
}


void  ApfelGui::GainChanged_0()
{
  GainChanged (0, 0);
}

void ApfelGui::GainChanged_1()
{
  GainChanged(0, 1);
}

void ApfelGui::GainChanged_2()
{
  GainChanged(1, 0);
}

void ApfelGui::GainChanged_3()
{
  GainChanged(1, 1);
}


void ApfelGui::GainChanged_4()
{
  GainChanged(2, 0);
}


void ApfelGui::GainChanged_5()
{
  GainChanged(2, 1);
}

void ApfelGui::GainChanged_6()
{
  GainChanged(3, 0);
}


void ApfelGui::GainChanged_7()
{
  GainChanged(3, 1);
}


void ApfelGui::GainChanged_8()
{
  GainChanged(4, 0);
}


void ApfelGui::GainChanged_9()
{
  GainChanged(4, 1);
}


void ApfelGui::GainChanged_10()
{
  GainChanged(5, 0);
}

void ApfelGui::GainChanged_11()
{
  GainChanged(5, 1);
}


void ApfelGui::GainChanged_12()
{
  GainChanged(6, 0);
}

void ApfelGui::GainChanged_13()
{
  GainChanged(6, 1);
}


void ApfelGui::GainChanged_14()
{
  GainChanged(7, 0);
}


void ApfelGui::GainChanged_15()
{
  GainChanged(7, 1);
}




void ApfelGui::AutoApplyDAC(int apfel, int dac, int val)
{
  // keep setup structure always consistent:
  BoardSetup& theSetup= fSetup[fSFP].at(fSlave);
  theSetup.SetDACValue(apfel,dac,val);
  WriteDAC_ApfelI2c (apfel, dac, theSetup.GetDACValue (apfel, dac));
  RefreshADC_Apfel(apfel, dac);
}


void ApfelGui::DAC_enterText(int apfel, int dac)
{
  // catch signal editingFinished() from Apfel1_DAClineEdit_1 etc.
  // need to synchronize with the sliders anyway:
  int val=fDACLineEdit[apfel][dac]->text ().toUInt (0, fNumberBase);
  fDACSlider[apfel][dac]->setValue(val & 0x3FF);

  std::cout << "ApfelGui::DAC_enterText="<<apfel<<", dac="<<dac<<", val="<<val << std::endl;
  if (checkBox_AA->isChecked () && !fBroadcasting)
     {
       EvaluateSlave ();
       APFEL_BROADCAST_ACTION(AutoApplyDAC(apfel,dac, val));
     }
}

void ApfelGui::DAC_enterText_0_0 ()
{
  DAC_enterText (0, 0);
}

void ApfelGui::DAC_enterText_0_1 ()
{
  DAC_enterText (0, 1);
}

void ApfelGui::DAC_enterText_0_2 ()
{
  DAC_enterText (0, 2);
}

void ApfelGui::DAC_enterText_0_3 ()
{
  DAC_enterText (0, 3);
}

void ApfelGui::DAC_enterText_1_0 ()
{
  DAC_enterText (1, 0);
}

void ApfelGui::DAC_enterText_1_1 ()
{
  DAC_enterText (1, 1);
}

void ApfelGui::DAC_enterText_1_2 ()
{
  DAC_enterText (1, 2);
}

void ApfelGui::DAC_enterText_1_3 ()
{
  DAC_enterText (1, 3);
}

void ApfelGui::DAC_enterText_2_0 ()
{
  DAC_enterText (2, 0);
}
void ApfelGui::DAC_enterText_2_1 ()
{
  DAC_enterText (2, 1);
}

void ApfelGui::DAC_enterText_2_2 ()
{
  DAC_enterText (2, 2);
}

void ApfelGui::DAC_enterText_2_3 ()
{
  DAC_enterText (2, 3);
}

void ApfelGui::DAC_enterText_3_0 ()
{
  DAC_enterText (3, 0);
}

void ApfelGui::DAC_enterText_3_1 ()
{
  DAC_enterText (3, 1);
}

void ApfelGui::DAC_enterText_3_2 ()
{
  DAC_enterText (3, 2);
}

void ApfelGui::DAC_enterText_3_3 ()
{
  DAC_enterText (3, 3);
}

void ApfelGui::DAC_enterText_4_0 ()
{
  DAC_enterText (4, 0);
}

void ApfelGui::DAC_enterText_4_1 ()
{
  DAC_enterText (4, 1);
}
void ApfelGui::DAC_enterText_4_2 ()
{
  DAC_enterText (4, 2);
}

void ApfelGui::DAC_enterText_4_3 ()
{
  DAC_enterText (4, 3);
}

void ApfelGui::DAC_enterText_5_0 ()
{
  DAC_enterText (5, 0);
}

void ApfelGui::DAC_enterText_5_1 ()
{
  DAC_enterText (5, 1);
}

void ApfelGui::DAC_enterText_5_2 ()
{
  DAC_enterText (5, 2);
}

void ApfelGui::DAC_enterText_5_3 ()
{
  DAC_enterText (5, 3);
}

void ApfelGui::DAC_enterText_6_0 ()
{
  DAC_enterText (6, 0);
}

void ApfelGui::DAC_enterText_6_1 ()
{
  DAC_enterText (6, 1);
}

void ApfelGui::DAC_enterText_6_2 ()
{
  DAC_enterText (6, 2);
}
void ApfelGui::DAC_enterText_6_3 ()
{
  DAC_enterText (6, 3);
}

void ApfelGui::DAC_enterText_7_0 ()
{
  DAC_enterText (7, 0);
}

void ApfelGui::DAC_enterText_7_1 ()
{
  DAC_enterText (7, 1);
}
void ApfelGui::DAC_enterText_7_2 ()
{
  DAC_enterText (7, 2);
}
void ApfelGui::DAC_enterText_7_3 ()
{
  DAC_enterText (7, 3);
}






void ApfelGui::DAC_changed(int apfel, int dac, int val)
{
  //std::cout << "ApfelGui::DAC__changed, apfel="<<apfel<<", dac="<<dac<<", val="<<val << std::endl;
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
 // need to synchronize the text view with the slider positions:
  fDACLineEdit[apfel][dac]->setText (pre+text.setNum (val, fNumberBase));

  // if autoapply mode, immediately set to structure and

  if (checkBox_AA->isChecked () && !fBroadcasting)
   {
     EvaluateSlave ();
     APFEL_BROADCAST_ACTION(AutoApplyDAC(apfel,dac, val));
   }

}






void ApfelGui::DAC_changed_0_0(int val)
{
  DAC_changed(0, 0, val);
}

void ApfelGui::DAC_changed_0_1(int val)
{
  DAC_changed(0, 1, val);
}

void ApfelGui::DAC_changed_0_2(int val)
{
  DAC_changed(0, 2, val);
}

void ApfelGui::DAC_changed_0_3(int val)
{
  DAC_changed(0, 3, val);
}

void ApfelGui::DAC_changed_1_0(int val)
{
  DAC_changed(1, 0, val);
}

void ApfelGui::DAC_changed_1_1(int val)
{
  DAC_changed(1, 1, val);
}

void ApfelGui::DAC_changed_1_2(int val)
{
  DAC_changed(1, 2, val);
}

void ApfelGui::DAC_changed_1_3(int val)
{
  DAC_changed(1, 3, val);
}

void ApfelGui::DAC_changed_2_0(int val)
{
  DAC_changed(2, 0, val);
}
void ApfelGui::DAC_changed_2_1(int val)
{
  DAC_changed(2, 1, val);
}

void ApfelGui::DAC_changed_2_2(int val)
{
  DAC_changed(2, 2, val);
}


void ApfelGui::DAC_changed_2_3(int val)
{
  DAC_changed(2, 3, val);
}

void ApfelGui::DAC_changed_3_0(int val)
{
  DAC_changed(3, 0, val);
}

void ApfelGui::DAC_changed_3_1(int val)
{
  DAC_changed(3, 1, val);
}

void ApfelGui::DAC_changed_3_2(int val)
{
  DAC_changed(3, 2, val);
}

void ApfelGui::DAC_changed_3_3(int val)
{
  DAC_changed(3, 3, val);
}


void ApfelGui::DAC_changed_4_0(int val)
{
  DAC_changed(4, 0, val);
}


void ApfelGui::DAC_changed_4_1(int val)
{
  DAC_changed(4, 1, val);
}
void ApfelGui::DAC_changed_4_2(int val)
{
  DAC_changed(4, 2, val);
}


void ApfelGui::DAC_changed_4_3(int val)
{
  DAC_changed(4, 3, val);
}

void ApfelGui::DAC_changed_5_0(int val)
{
  DAC_changed(5, 0, val);
}


void ApfelGui::DAC_changed_5_1(int val)
{
  DAC_changed(5, 1, val);
}


void ApfelGui::DAC_changed_5_2(int val)
{
  DAC_changed(5, 2, val);
}

void ApfelGui::DAC_changed_5_3(int val)
{
  DAC_changed(5, 3, val);
}

void ApfelGui::DAC_changed_6_0(int val)
{
  DAC_changed(6, 0, val);
}

void ApfelGui::DAC_changed_6_1(int val)
{
  DAC_changed(6, 1, val);
}


void ApfelGui::DAC_changed_6_2(int val)
{
  DAC_changed(6, 2, val);
}
void ApfelGui::DAC_changed_6_3(int val)
{
  DAC_changed(6, 3, val);
}

void ApfelGui::DAC_changed_7_0(int val)
{
  DAC_changed(7, 0, val);
}

void ApfelGui::DAC_changed_7_1(int val)
{
  DAC_changed(7, 1, val);
}
void ApfelGui::DAC_changed_7_2(int val)
{
  DAC_changed(7, 2, val);
}
void ApfelGui::DAC_changed_7_3(int val)
{
  DAC_changed(7, 3, val);
}


void ApfelGui::AutoCalibrate(int apfel)
{

  if (!checkBox_AA->isChecked ())
  {
     // first show confirm window if not running in auto apply mode:
    char buffer[1024];
    snprintf (buffer, 1024, "Really Do APFEL DAC %d autocalibration for SFP %d Device %d?", apfel, fSFP, fSlave);
    if (QMessageBox::question (this, "APFEL GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
        != QMessageBox::Yes)
    {
      return;
    }
  }
  if (!fBroadcasting)
    {
      EvaluateSlave ();
      APFEL_BROADCAST_ACTION(DoAutoCalibrate(apfel));
    }
}


void ApfelGui::AutoCalibrate_0()
{
  AutoCalibrate(0);
}
void ApfelGui::AutoCalibrate_1()
{
  AutoCalibrate(1);
}
void ApfelGui::AutoCalibrate_2()
{
  AutoCalibrate(2);
}
void ApfelGui::AutoCalibrate_3()
{
  AutoCalibrate(3);
}
void ApfelGui::AutoCalibrate_4()
{
  AutoCalibrate(4);
}
void ApfelGui::AutoCalibrate_5()
{
  AutoCalibrate(5);
}
void ApfelGui::AutoCalibrate_6()
{
  AutoCalibrate(6);
}
void ApfelGui::AutoCalibrate_7()
{
  AutoCalibrate(7);
}


void ApfelGui::AutoCalibrate_all()
{
  if (!checkBox_AA->isChecked ())
   {
      // first show confirm window if not running in auto apply mode:
     char buffer[1024];
     snprintf (buffer, 1024, "Really Do ALL APFEL DAC autocalibration for SFP %d Device %d?", fSFP, fSlave);
     if (QMessageBox::question (this, "APFEL GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
         != QMessageBox::Yes)
     {
       return;
     }
   }
   if (!fBroadcasting)
     {
       EvaluateSlave ();
       // TODO: later use calibrate broadcast command of board
       for(int apfel=0; apfel<APFEL_NUMCHIPS; ++apfel)
       {
         APFEL_BROADCAST_ACTION(DoAutoCalibrate(apfel));
       }
     }
}










 void ApfelGui::DAC_spinBox_all_changed(int val)
{
  std::cout << "ApfelGui::DAC_spinBox_all_changed, val="<<val << std::endl;
   for(int chan=0;chan<16;++chan)
     fDACSpinBoxes[chan]->setValue (val);
  
}


 void ApfelGui::DAC_spinBox_changed (int channel, int val)
{
  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(AutoApplyRefresh(channel, val));
  }

}


 void ApfelGui::Any_spinBox00_changed(int val)
{
   DAC_spinBox_changed (0,val);
 }

void ApfelGui::Any_spinBox01_changed(int val)
{
  DAC_spinBox_changed (1,val);
}

void ApfelGui::Any_spinBox02_changed(int val)
{
  DAC_spinBox_changed (2,val);
}

void ApfelGui::Any_spinBox03_changed(int val)
{
  DAC_spinBox_changed (3,val);
}

void ApfelGui::Any_spinBox04_changed(int val)
{
  DAC_spinBox_changed (4,val);
}

void ApfelGui::Any_spinBox05_changed(int val)
{
  DAC_spinBox_changed (5,val);
}

void ApfelGui::Any_spinBox06_changed(int val)
{
  DAC_spinBox_changed (6,val);
}

void ApfelGui::Any_spinBox07_changed(int val)
{
  DAC_spinBox_changed (7,val);
}

void ApfelGui::Any_spinBox08_changed(int val)
{
  DAC_spinBox_changed (8,val);
}

void ApfelGui::Any_spinBox09_changed(int val)
{
  DAC_spinBox_changed (9,val);
}

void ApfelGui::Any_spinBox10_changed(int val)
{
  DAC_spinBox_changed (10,val);
}

void ApfelGui::Any_spinBox11_changed(int val)
{
  DAC_spinBox_changed (11,val);
}

void ApfelGui::Any_spinBox12_changed(int val)
{
  DAC_spinBox_changed (12,val);
}

void ApfelGui::Any_spinBox13_changed(int val)
{
  DAC_spinBox_changed (13,val);
}

void ApfelGui::Any_spinBox14_changed(int val)
{
  DAC_spinBox_changed (14,val);
}

void ApfelGui::Any_spinBox15_changed(int val)
{
  DAC_spinBox_changed (15,val);
}


void ApfelGui::AutoApplyRefresh(int channel, int dac)
{
     QString text;
     QString pre;
     fNumberBase == 16 ? pre = "0x" : pre = "";
     int Adc = autoApply (channel, dac);
     fADCLineEdit[channel]->setText (pre + text.setNum (Adc, fNumberBase));
     RefreshStatus ();
}




int ApfelGui::autoApply(int channel, int permillevalue)

{ 
  int apfel=0, dac=0;
  BoardSetup& theSetup=fSetup[fSFP].at(fSlave);
  theSetup.EvaluateDACIndices(channel, apfel, dac);
  int gain=theSetup.GetGain(apfel,dac);
  //int value=theSetup.EvaluateDACvalueAbsolute(permillevalue,-1,gain);
  int value=theSetup.EvaluateDACvalueAbsolute(permillevalue,channel,gain);


  theSetup.SetDACValue(apfel, dac, value);
   
   EnableI2C ();  
   WriteDAC_ApfelI2c (apfel, dac, theSetup.GetDACValue(apfel, dac));
   DisableI2C ();
   if (!AssertNoBroadcast ())
      return -1;
   int Adc=AcquireBaselineSample(channel);
   //std::cout << "ApfelGui::autoApply channel="<<channel<<", permille="<<permillevalue<<", apfel="<<apfel<<", dac="<<dac<<", DACvalue="<<value<<", ADC="<<Adc << std::endl;

   return Adc;
  
}


int ApfelGui::AcquireBaselineSample(uint8_t febexchan)
{
  if(febexchan >= APFEL_ADC_NUMADC*APFEL_ADC_NUMCHAN) return -1;
  int adcchip= febexchan/APFEL_ADC_NUMCHAN;
  int adcchannel= febexchan-adcchip * APFEL_ADC_NUMCHAN ;
  int Adc=0;
  for(int t=0; t<APFEL_ADC_BASELINESAMPLES;++t)
    {
      Adc+=ReadADC_Apfel(adcchip,adcchannel);
    }
  Adc=Adc/APFEL_ADC_BASELINESAMPLES;
  return Adc;
}



void ApfelGui::RefreshDAC(int apfel)
{
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  BoardSetup& theSetup=fSetup[fSFP].at(fSlave);
  for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
  {
    int value =  theSetup.GetDACValue (apfel, dac);
    fDACSlider[apfel][dac]->setValue (value);
    fDACLineEdit[apfel][dac]->setText (pre+text.setNum (value, fNumberBase));
  }
}

  void ApfelGui::RefreshADC_channel(int channel, int gain)
  {
    QString text;
    QString pre;
    fNumberBase == 16 ? pre = "0x" : pre = "";
     BoardSetup& theSetup=fSetup[fSFP].at(fSlave);
    int val=theSetup.GetDACValue(channel);
    int permille=theSetup.EvaluateADCvaluePermille(val,channel,gain);
    //std::cout << "RefreshADC_channel(" << (int) channel <<","<<gain<<") - val="<<val<<" permille=" << permille<< std::endl;
    fDACSpinBoxes[channel]->setValue(permille);
    int adc=AcquireBaselineSample(channel);
    fADCLineEdit[channel]->setText (pre+text.setNum (adc, fNumberBase));
  }

  void ApfelGui::RefreshADC_Apfel(int apfelchip, int dac)
  {
    BoardSetup& theSetup=fSetup[fSFP].at(fSlave);
    int chan=theSetup.EvaluateADCChannel(apfelchip, dac);
    //std::cout << "RefreshADC(" << (int) apfelchip <<"):  dac:"<<dac<<", chan=" << chan<< std::endl;
    if(chan>=0) {
      // only refresh adc channels once for active dacs
      int gain=theSetup.GetGain(apfelchip,dac);
      RefreshADC_channel(chan, gain);
      if(!theSetup.IsHighGain())  RefreshADC_channel(chan+1, gain); // kludge to cover both adc channels set by dac2 for gain 1
    }

  }


void ApfelGui::RefreshView ()
{
// display setup structure to gui:
//  QString text;
//  QString pre;
//  fNumberBase == 16 ? pre = "0x" : pre = "";
  BoardSetup& theSetup=fSetup[fSFP].at(fSlave);

//////////////////////////////////////////////////////
// first io configuration and gain:
ApfelRadioButton->setChecked(theSetup.IsApfelInUse());
PolandRadioButton->setChecked(!theSetup.IsApfelInUse()); // probably we do not need this because of autoExclusive flag
LoGainRadioButton->setChecked(!theSetup.IsHighGain());
HiGainRadioButton->setChecked(theSetup.IsHighGain()); // probably we do not need this because of autoExclusive flag
StretcherOnRadioButton->setChecked(theSetup.IsStretcherInUse());
StretcherOffRadioButton->setChecked(!theSetup.IsStretcherInUse());


InverseMappingCheckBox->setChecked(!theSetup.IsRegularMapping());


if (theSetup.IsHighGain())
  {
  // only refresh gain entries if we are in high gain mode
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
      {
        bool logain =  theSetup.GetLowGain (apfel, chan);
        if(logain)
          fApfelGainCombo[apfel][chan]->setCurrentIndex (0);
        else
          fApfelGainCombo[apfel][chan]->setCurrentIndex (1);
      }
    }
  }

///////////////////////////////////////////////////////
// show DAC values:

for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
   {
      RefreshDAC(apfel);
   }

///////////////////////////////////////////////////////
//show pulser setup:
for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
   {
      bool positive = theSetup.GetTestPulsePositive(apfel);
      if(positive)
        fApfelPulsePolarityCombo[apfel]->setCurrentIndex (0);
      else
        fApfelPulsePolarityCombo[apfel]->setCurrentIndex (1);
        for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
          {
            bool on= theSetup.GetTestPulseEnable(apfel, chan);
            fApfelPulseEnabledCheckbox[apfel][chan]->setChecked(on);
          }
   }

//////////////////////////////////////////////////////////
// dac relative baseline settings and adc sample:
  int apfel=0, dac=0;
  for(int channel=0; channel<16;++channel)
     {
          theSetup.EvaluateDACIndices(channel, apfel, dac);
          int gain=theSetup.GetGain(apfel,dac);
          RefreshADC_channel(channel, gain);
     }


  RefreshChains();
  RefreshStatus();
}


void ApfelGui::RefreshStatus ()
{
  QString text;
  QString statustext;
   statustext.append ("SFP ");
   statustext.append (text.setNum (fSFP));
   statustext.append (" DEV ");
   statustext.append (text.setNum (fSlave));
   statustext.append (" - Last refresh:");
   statustext.append (QDateTime::currentDateTime ().toString (Qt::TextDate));
   StatusLabel->setText (statustext);
}


void ApfelGui::RefreshChains ()
{

#ifdef USE_MBSPEX_LIB
  // show status of configured chains:
  Chain0_Box->setValue (fSFPChains.numslaves[0]);
  Chain1_Box->setValue (fSFPChains.numslaves[1]);
  Chain2_Box->setValue (fSFPChains.numslaves[2]);
  Chain3_Box->setValue (fSFPChains.numslaves[3]);

  // set maximum value of device spinbox according to init chains:

  if (fSFP >= 0) // only for non broadcast mode of slaves
  {
    if (fSFPChains.numslaves[fSFP] > 0) // configured chains
    {
      SlavespinBox->setMaximum (fSFPChains.numslaves[fSFP] - 1);
      SlavespinBox->setEnabled (true);
    }
    else // non configured chains
    {
      SlavespinBox->setEnabled (false);
    }
  }
#else
    Chain0_Box->setEnabled (false);
    Chain1_Box->setEnabled (false);
    Chain2_Box->setEnabled (false);
    Chain3_Box->setEnabled (false);

#endif

}



void ApfelGui::EvaluatePulser (int apfel)
{
  BoardSetup& theSetup = fSetup[fSFP].at (fSlave);
  bool positive = (fApfelPulsePolarityCombo[apfel]->currentIndex () == 0);
  theSetup.SetTestPulsePostive (apfel, positive);
  for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
  {
    bool on = fApfelPulseEnabledCheckbox[apfel][chan]->isChecked ();
    theSetup.SetTestPulseEnable (apfel, chan, on);
  }
}

void ApfelGui::EvaluateGain(int apfel, int channel)
{
  BoardSetup& theSetup= fSetup[fSFP].at(fSlave);
  bool logain = (fApfelGainCombo[apfel][channel]->currentIndex () == 0);
  theSetup.SetLowGain (apfel, channel, logain);
}

void ApfelGui::EvaluateIOSwitch()
{
  BoardSetup& theSetup= fSetup[fSFP].at(fSlave);
  // get io config from gui
  theSetup.SetApfelInUse(ApfelRadioButton->isChecked());
  theSetup.SetHighGain(HiGainRadioButton->isChecked());
  theSetup.SetStretcherInUse(StretcherOnRadioButton->isChecked());

}


void ApfelGui::EvaluateView ()
{
  // here the current gui display is just copied to setup structure in local memory
BoardSetup& theSetup= fSetup[fSFP].at(fSlave);


theSetup.SetApfelMapping(!InverseMappingCheckBox->isChecked());

EvaluateIOSwitch();

if (theSetup.IsHighGain())
  {
  // only apply gain entries if we are in high gain mode
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
      {
        EvaluateGain(apfel, chan);
      }
    }
  }
// here baseline sliders for dacs
// todo: prevent different settings from DAC and ADC tabs; check which tab is active?
if(ApfelTabWidget->currentIndex()==3)
{
  // only apply the adc sliders when visible
  for(int channel=0; channel<16;++channel)
     {
          int permille=fDACSpinBoxes[channel]->value();
          int value=theSetup.EvaluateDACvalueAbsolute(permille);
          std::cout<<"EvaluateView for channel:"<<channel<<", permille:"<<permille<<" - val="<<value<< std::endl;
          theSetup.SetDACValue(channel, value);
     }
}
else
  {
    // otherwise use direct entries of DAC panel:
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
      {
        int value = fDACSlider[apfel][dac]->value () & 0x3FF;
        std::cout<<"EvaluateView for apfel:"<<apfel<<", dac:"<<dac<<" - val="<<value<< std::endl;
        theSetup.SetDACValue (apfel, dac, value);
      }

    }
  } //if(ApfelTabWidget->currentIndex()==3)


// pulser config from gui
for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
   {
      EvaluatePulser(apfel);
   }
}

void ApfelGui::EvaluateSlave ()
{
  if(fBroadcasting) return;
  fSFP = SFPspinBox->value ();
  fSlave = SlavespinBox->value ();

}

void ApfelGui::SetRegisters ()
{
  QApplication::setOverrideCursor (Qt::WaitCursor);
  EnableI2C ();    // must be done since mbs setup program may shut i2c off at the end

  BoardSetup& theSetup=fSetup[fSFP].at(fSlave); // check for indices is done in broadcast action macro that calls this function

  SetIOSwitch();
  for (uint8_t apf = 0; apf < APFEL_NUMCHIPS; ++apf)
  {
    for (uint8_t dac = 0; dac < APFEL_NUMDACS; ++dac)
    {
      int val= theSetup.GetDACValue (apf, dac);
      WriteDAC_ApfelI2c (apf, dac, val);
      std::cout << "SetRegisters DAC(" << apf <<"," << dac << ") val=" <<  val << std::endl;
    }

    for (uint8_t ch = 0; ch < APFEL_NUMCHANS; ++ch)
    {
      // here set gain factors for each channel:
      SetGain (apf, ch, theSetup.GetLowGain (apf, ch));

    }
    SetPulser(apf);
  }

  DisableI2C ();
  QApplication::restoreOverrideCursor ();

}


void ApfelGui::SetIOSwitch()
{
  BoardSetup& theSetup=fSetup[fSFP].at(fSlave);
  //std::cout << "SetIOSwitch: apfel=" << theSetup.IsApfelInUse() <<", highgain=" << theSetup.IsHighGain() << ", stretcher="<< theSetup.IsStretcherInUse()<<")"<< std::endl;
  SetSwitches(theSetup.IsApfelInUse(), theSetup.IsHighGain(), theSetup.IsStretcherInUse());

}


void ApfelGui::SetPulser(uint8_t apf)
{
  BoardSetup& theSetup=fSetup[fSFP].at(fSlave);
   // here set test pulser properties. we must use both channels simultaneously:
  bool on_1=theSetup.GetTestPulseEnable(apf,0);
  bool on_2=theSetup.GetTestPulseEnable(apf,1);
  bool on_any= on_1 || on_2;
  SetTestPulse(apf,on_any,on_1,on_2,theSetup.GetTestPulsePositive(apf));
}


void ApfelGui::GetDACs (int chip)
{
  BoardSetup& theSetup=fSetup[fSFP].at (fSlave);

  for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
     {

       int val = ReadDAC_ApfelI2c (chip, dac);
       //std::cout << "GetDACs(" << chip <<"," << dac << ") val=" << val << std::endl;

       if (val < 0)
       {
         AppendTextWindow ("GetDacs has error!");
         return;    // TODO error message
       }
       theSetup.SetDACValue (chip, dac, val);
     }
}


void ApfelGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  QApplication::setOverrideCursor (Qt::WaitCursor);
  EnableI2C ();
  for (int chip = 0; chip < APFEL_NUMCHIPS; ++chip)
  {
    GetDACs(chip);
//    for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
//    {
//
//      int val = ReadDAC_ApfelI2c (chip, dac);
//      //std::cout << "GetRegisters DAC(" << chip <<"," << dac << ") val=" << val << std::endl;
//
//      if (val < 0)
//      {
//        AppendTextWindow ("GetRegisters has error!");
//        return;    // TODO error message
//      }
//      fSetup[fSFP].at (fSlave).SetDACValue (chip, dac, val);

//    }

    // todo: here read back amplification settings - not possible!


    // todo: here read back test pulse settings - not possible!


    // note that adc values are not part of the setup structure and sampled in refreshview


  }
  DisableI2C ();
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::SaveRegisters()

{
   GetRegisters(); // refresh actual setup from hardware
   fSaveConfig = true;    // switch to file output mode
   SetRegisters();    // register settings are written to file
   fSaveConfig = false;
}

void ApfelGui::GetSFPChainSetup()
{
//  std::cout<<"GetSFPChainSetup... "<< std::endl;
#ifdef USE_MBSPEX_LIB
    // broadcast mode: find out number of slaves and loop over all registered slaves
    mbspex_get_configured_slaves(fPexFD, &fSFPChains);
#else
    // without mbspex lib, we just assume 4 devices for each chain:
    for(int sfp=0; sfp<4; ++sfp)
    {
      fSFPChains.numslaves[sfp]=4;
    }
#endif


    // dynamically increase array of setup structures:
    for(int sfp=0; sfp<4; ++sfp)
    {
      while(fSetup[sfp].size()<fSFPChains.numslaves[sfp])
      {
        fSetup[sfp].push_back(BoardSetup());
        // TODO: evaluate real mapping of apfel chips here!

        //std::cout<<"GetSFPChainSetup increased setup at sfp "<<sfp<<" to "<<fSetup[sfp].size()<<" slaves." << std::endl;
      }
    }
  
}

    
int ApfelGui::ReadDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac)
{
  int val = 0;
  if(apfelchip>=APFEL_NUMCHIPS){
     AppendTextWindow ("#Error: ReadDAC_ApfelI2c with illegal chip number!");
     return -1;
   }
  int apid=GetApfelId(fSFP, fSlave, apfelchip);

  // first: read transfer request from apfel chip with id to core register
   int dat=APFEL_TRANSFER_BASE_RD + (dac+1) * APFEL_TRANSFER_DAC_OFFSET + (apid & 0xFF);
   // mind that dac index starts with 0 for dac1 here!



   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
   // note that WriteGosip already contains i2csleep


   // second: read request from core registers
   dat=APFEL_DAC_REQUEST_BASE_RD + (dac)* APFEL_CORE_REQUEST_DAC_OFFSET;
   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // third: actually read requested value
  val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1); // read out the value
  if(val < 0) return val; // error case, propagate it upwards
///////////
//  Note: val=Read Data is one word of 32-bits, where:
//  Read_Data [31 downto 24] - GOSIP Status register.
//  Read_Data [23 downto 16] - APFEL Chip ID.
//  Read_Data [15 downto 14] - APFEL Status Bits.
//  Read_Data [13 downto 10] - All bits are zeros.
//  Read_Data [9 downto 0]   - requested data.
//
/////////////
  return (val & APFEL_DAC_MAXVALUE); // mask to use only data part
}



int  ApfelGui::ReadADC_Apfel (uint8_t adc, uint8_t chan)
{
  if(adc>APFEL_ADC_NUMADC || chan > APFEL_ADC_NUMCHAN) return -1;

  // test: always enable core to read data first:
  //WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, 0x80);

  int val=0;
  int dat=(adc << 3) + chan; //l_wr_d  = (l_k*4) + l_l;

  WriteGosip (fSFP, fSlave, APFEL_ADC_PORT, dat); // first specify channel number

  val = ReadGosip (fSFP, fSlave, APFEL_ADC_PORT); // read back the value

  // check if channel id matches the requested ones:
  if ( ((val >> 24) & 0xf) != dat)
      {
         printm ("#Error: ReadADC_Apfel channel id mismatch, requested 0x%x, received 0x%x",dat, (val>>24));
         return -1;
      }


  //printm("ReadADC_Apfel(%d,%d) reads value=0x%x, return:0x%x",(int) adc, (int) chan, val, (val & APFEL_ADC_MAXVALUE));
  return (val & APFEL_ADC_MAXVALUE);


}








int ApfelGui::ReadGosip (int sfp, int slave, int address)
{
  int value = -1;
#ifdef USE_MBSPEX_LIB
  int rev = 0;
  long int dat = 0;
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = mbspex_slave_rd (fPexFD, sfp, slave, address, &dat);
  I2c_sleep ();
  value = dat;
  if (fDebug)
  {
    char buffer[1024];
    if (rev == 0)
    {
      snprintf (buffer, 1024, "mbspex_slave_rd(%d,%d 0x%x) -> 0x%x", sfp, slave, address, value);
    }
    else
    {
      snprintf (buffer, 1024, "ERROR %d from mbspex_slave_rd(%d,%d 0x%x)", rev, sfp, slave, address);
    }
    QString msg (buffer);
    AppendTextWindow (msg);

  }
  //QApplication::restoreOverrideCursor ();
#else
  char buffer[1024];
//snprintf(buffer,1024,"/daq/usr/adamczew/workspace/drivers/mbspex/bin/gosipcmd -r -- %d %d 0x%x",sfp, slave, address);
  snprintf (buffer, 1024, "gosipcmd -r -- %d %d 0x%x", sfp, slave, address);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  if (result != "ERROR")
  {
    QString pre, valtext;
    fNumberBase==16? pre="0x" : pre="";
    //DebugTextWindow (result);
    value = result.toInt (0, 0);
    valtext=pre+valtext.setNum (value, fNumberBase);
    DebugTextWindow (valtext);

  }
  else
  {

    value = -1;
  }
#endif

  return value;
}


uint8_t ApfelGui::GetApfelId(int sfp, int slave, uint8_t apfelchip)
{
  if(sfp<0 || sfp>= PEX_SFP_NUMBER) return 0xFF;
  if (slave<0 || slave>=fSFPChains.numslaves[sfp]) return 0xFF;
  BoardSetup& theSetup=fSetup[sfp].at(slave);
  return theSetup.GetApfelID(apfelchip);
}



int ApfelGui::WriteDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac, uint16_t value)
{



  //std::cout << "WriteDAC_ApfelI2c(" << apfelchip <<"," << dac << ") value=" << value << std::endl;
  //(0:  shift to max adc value)
  if(apfelchip>=APFEL_NUMCHIPS){
    AppendTextWindow ("#Error: WriteDAC_ApfelI2c with illegal chip number!");
    return -1;
  }
  int apfelid=GetApfelId(fSFP, fSlave, apfelchip);


  // first write value to core register:
  int dat=APFEL_CORE_REQUEST_BASE_WR + dac* APFEL_CORE_REQUEST_DAC_OFFSET;
  dat|=(value & 0x3FF);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);


  // then request for data transfer:
  dat=APFEL_TRANSFER_BASE_WR + (dac+1) * APFEL_TRANSFER_DAC_OFFSET + (apfelid & 0xFF);
  // mind that dac index starts with 0 for dac1 here!
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  return 0;
}


void ApfelGui::SetGain(uint8_t apfelchip, uint8_t chan, bool useGain16)
{
  int apid=GetApfelId(fSFP, fSlave, apfelchip);
  //std::cout << "SetGain DAC(" << (int) apfelchip <<", id:"<<apid << (int) chan << ") gain16=" << useGain16 << std::endl;
  // first set gain to core:
  int mask=0;
  mask |= apid & 0xFF;
  if(chan==0)
  {
    // CH1
    if(useGain16)
      mask |=0x100;
    else
      mask |=0x000; // just for completeness..
  }
  else
  {
    // CH2
    if(useGain16)
        mask |= 0x300;
    else
        mask |= 0x200;
  }

  int dat=APFEL_GAIN_BASE_WR | mask;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // transfer to apfel chip: not required here!
//  dat=APFEL_TRANSFER_BASE_WR + ((apfelchip+1) & 0xFF);
//  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);


}

void ApfelGui::SetTestPulse(uint8_t apfelchip, bool on, bool chan1, bool chan2, bool positive)
{
  int apid=GetApfelId(fSFP, fSlave, apfelchip);
  //std::cout << "SetTestPulse(" << (int) apfelchip <<", id:"<<apid<<"): on=" << on << ", ch1="<<chan1<<", ch2="<<chan2 << std::endl;

  int dat=0;
  int apfelid = apid & 0xFF;
  if(!on)
  {
    dat=APFEL_TESTPULSE_FLAG_WR | apfelid;
    WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  }
  else
  {
  // first set channel mask for ch1 and ch2:
     dat=APFEL_TESTPULSE_CHAN_WR | apfelid;
     if(chan1) dat |= (1 << 8);
     if(chan2) dat |= (1 << 12);
     WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // activate correct polarity:
     dat=APFEL_TESTPULSE_FLAG_WR | apfelid;
     if(positive) dat |= 0x100;
     else dat |= 0x300;
     WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  }

  // transfer to apfel chip: TODO: needed here?
//   dat=APFEL_TRANSFER_BASE_WR | apfelid;
//   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

}


void ApfelGui::DoAutoCalibrate(uint8_t apfelchip)
{
  QApplication::setOverrideCursor (Qt::WaitCursor);

  int apid=GetApfelId(fSFP, fSlave, apfelchip);
  printm("Doing Autocalibration of apfel chip %d (id:%d) on sfp:%d, board:%d...",apfelchip,apid, fSFP, fSlave);
  int apfelid = apid & 0xFF;
  int dat =APFEL_AUTOCALIBRATE_BASE_WR | apfelid;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // transfer to apfel chip- not neceesary here!
//   dat=APFEL_TRANSFER_BASE_WR | apfelid;
//   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  usleep(8000);
  printm("...done!\n");
  //Note: The auto calibration of the APFELchip takes not more that 8 ms.

  // here get registers of apfelchip only and refresh
   EnableI2C ();
   GetDACs(apfelchip);
   DisableI2C ();
   RefreshDAC(apfelchip);
   BoardSetup& theSetup=fSetup[fSFP].at (fSlave);
   for(int dac=0; dac<APFEL_NUMDACS; ++dac)
   {
     RefreshADC_Apfel(apfelchip, dac);
   }

  QApplication::restoreOverrideCursor ();
}





void ApfelGui::SetSwitches(bool useApfel, bool useHighGain, bool useStretcher)
{
  int dat=APFEL_IO_CONTROL_WR;
  int mask=0;
  if(!useApfel)         mask |= APFEL_SW_NOINPUT;
  if(!useHighGain)       mask |= APFEL_SW_HIGAIN;
  if(useStretcher)      mask |= APFEL_SW_STRETCH;
  dat |= mask;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  dat=APFEL_IO_SET;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // read back switches
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, APFEL_IO_CONTROL_RD);
  int val=ReadGosip(fSFP, fSlave,GOS_I2C_DRR1);
  int swmask=((val>>12) & 0x7);
  //printm("SetInputSwitch mask=0x%x, read back switch mask=0x%x", mask, swmask);
  if (((swmask & mask) != mask))
      printm("#Error SetInputSwitch(apfel=%d, high=%d, stretch=%d) - read back switch mask is 0x%x",
          useApfel, useHighGain, useStretcher,swmask);
  // todo: advanced error handling?

}


void ApfelGui::SetInverseMapping(int on)
{
  BoardSetup& theSetup=fSetup[fSFP].at (fSlave);
  theSetup.SetApfelMapping(!on);

}

void ApfelGui::InverseMapping_changed (int on)
{
  //std::cout << "InverseMapping_changed to" <<  on << std::endl;

  if (checkBox_AA->isChecked () && !fBroadcasting)
  {
    EvaluateSlave ();
    APFEL_BROADCAST_ACTION(SetInverseMapping(on));
  }

}



int ApfelGui::SaveGosip (int sfp, int slave, int address, int value)
{
//std::cout << "# SaveGosip" << std::endl;
  static char buffer[1024] = { };
  snprintf (buffer, 1024, "%d %d %x %x \n", sfp, slave, address, value);
  QString line (buffer);
  return (WriteConfigFile (line));
}

int ApfelGui::WriteGosip (int sfp, int slave, int address, int value)
{
  int rev = 0;
//std::cout << "#WriteGosip" << std::endl;
  if (fSaveConfig)
    return SaveGosip (sfp, slave, address, value);

#ifdef USE_MBSPEX_LIB
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = mbspex_slave_wr (fPexFD, sfp, slave, address, value);
  I2c_sleep ();
  if (fDebug)
  {
    char buffer[1024];
    snprintf (buffer, 1024, "mbspex_slave_wr(%d,%d 0x%x 0x%x)", sfp, slave, address, value);
    QString msg (buffer);
    AppendTextWindow (msg);
  }
  //QApplication::restoreOverrideCursor ();
#else
  char buffer[1024];
  snprintf (buffer, 1024, "gosipcmd -w -- %d %d 0x%x 0x%x", sfp, slave, address, value);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  if (result == "ERROR")
  rev = -1;
#endif

  return rev;
}

QString ApfelGui::ExecuteGosipCmd (QString& com, int timeout)
{
// interface to shell gosipcmd
// TODO optionally some remote call via ssh for Go4 gui?
  QString result;
  QProcess proc;
  DebugTextWindow (com);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  proc.setProcessEnvironment (fEnv);
#endif
  proc.setReadChannel (QProcess::StandardOutput);
  QApplication::setOverrideCursor (Qt::WaitCursor);

  proc.start (com);
// if(proc.waitForReadyRead (1000)) // will give termination warnings after leaving this function
  if (proc.waitForFinished (timeout))    // after process is finished we can still read stdio buffer
  {
    // read back stdout of proc here
    result = proc.readAll ();
  }
  else
  {
    std::stringstream buf;
    buf << "! Warning: ExecuteGosipCmd not finished after " << timeout / 1000 << " s timeout !!!" << std::endl;
    std::cout << " ApfelGui: " << buf.str ().c_str ();
    AppendTextWindow (buf.str ().c_str ());
    result = "ERROR";
  }
  QApplication::restoreOverrideCursor ();
  return result;
}

void ApfelGui::AppendTextWindow (const QString& text)
{
  TextOutput->appendPlainText (text);
  TextOutput->update ();
}

void ApfelGui::FlushTextWindow ()
{
  TextOutput->repaint ();
}

bool ApfelGui::AssertNoBroadcast (bool verbose)
{
  if (fSFP < 0 || fSlave < 0)
  {
    //std::cerr << "# ApfelGui Error: broadcast not supported here!" << std::endl;
    if (verbose)
      AppendTextWindow ("#Error: broadcast not supported here!");
    return false;
  }
  return true;
}

bool ApfelGui::AssertChainConfigured (bool verbose)
{
#ifdef USE_MBSPEX_LIB

  if (fSlave >= fSFPChains.numslaves[fSFP])
  {
    if (verbose)
      printm("#Error: device index %d not in initialized chain of length %d at SFP %d",fSlave,fSFPChains.numslaves[fSFP],fSFP);
    return false;
  }
#endif
return true;
}
