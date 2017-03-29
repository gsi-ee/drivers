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

#include <kplotobject.h>
#include <kplotwidget.h>
#include <kplotaxis.h>

#include <sstream>
#include <string.h>
#include <errno.h>
#include <QTableWidget>

// *********************************************************


/*
 *  Constructs a ApfelGui which is a child of 'parent', with the
 *  name 'name'.'
 */
ApfelGui::ApfelGui (QWidget* parent) :
    GosipGui (parent), fPulserProgressCounter (0), fTestFile (0), fPlotMinDac (0),
        fPlotMaxDac (APFEL_DAC_MAXVALUE), fPlotMinAdc (0), fPlotMaxAdc (APFEL_ADC_MAXVALUE)
{
  fImplementationName="APFEL";
  fVersionString="Welcome to APFEL GUI!\n\t v0.990 of 27-March-2017 by JAM (j.adamczewski@gsi.de)\n";

  fApfelWidget=new ApfelWidget();
  Settings_scrollArea->setWidget(fApfelWidget);
  setWindowTitle(QString("%1 GUI").arg(fImplementationName));
  ClearOutputBtn_clicked ();


  QObject::connect (fApfelWidget->AutoAdjustButton, SIGNAL (clicked ()), this, SLOT (AutoAdjustBtn_clicked ()));
  QObject::connect (fApfelWidget->CalibrateADCButton, SIGNAL (clicked ()), this, SLOT (CalibrateADCBtn_clicked ()));
  QObject::connect (fApfelWidget->CalibrateResetButton, SIGNAL (clicked ()), this, SLOT (CalibrateResetBtn_clicked ()));

  QObject::connect (fApfelWidget->DAC_spinBox_all, SIGNAL(valueChanged(int)), this, SLOT(DAC_spinBox_all_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_00, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox00_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_01, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox01_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_02, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox02_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_03, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox03_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_04, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox04_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_05, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox05_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_06, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox06_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_07, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox07_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_08, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox08_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_09, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox09_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_10, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox10_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_11, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox11_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_12, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox12_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_13, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox13_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_14, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox14_changed(int)));
  QObject::connect (fApfelWidget->DAC_spinBox_15, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox15_changed(int)));

  QObject::connect (fApfelWidget->Apfel1_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_0(int)));
  QObject::connect (fApfelWidget->Apfel1_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_1(int)));
  QObject::connect (fApfelWidget->Apfel1_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_2(int)));
  QObject::connect (fApfelWidget->Apfel1_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_3(int)));
  QObject::connect (fApfelWidget->Apfel2_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_0(int)));
  QObject::connect (fApfelWidget->Apfel2_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_1(int)));
  QObject::connect (fApfelWidget->Apfel2_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_2(int)));
  QObject::connect (fApfelWidget->Apfel2_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_3(int)));
  QObject::connect (fApfelWidget->Apfel3_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_0(int)));
  QObject::connect (fApfelWidget->Apfel3_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_1(int)));
  QObject::connect (fApfelWidget->Apfel3_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_2(int)));
  QObject::connect (fApfelWidget->Apfel3_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_3(int)));
  QObject::connect (fApfelWidget->Apfel4_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_0(int)));
  QObject::connect (fApfelWidget->Apfel4_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_1(int)));
  QObject::connect (fApfelWidget->Apfel4_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_2(int)));
  QObject::connect (fApfelWidget->Apfel4_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_3(int)));
  QObject::connect (fApfelWidget->Apfel5_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_0(int)));
  QObject::connect (fApfelWidget->Apfel5_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_1(int)));
  QObject::connect (fApfelWidget->Apfel5_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_2(int)));
  QObject::connect (fApfelWidget->Apfel5_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_3(int)));
  QObject::connect (fApfelWidget->Apfel6_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_0(int)));
  QObject::connect (fApfelWidget->Apfel6_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_1(int)));
  QObject::connect (fApfelWidget->Apfel6_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_2(int)));
  QObject::connect (fApfelWidget->Apfel6_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_3(int)));
  QObject::connect (fApfelWidget->Apfel7_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_0(int)));
  QObject::connect (fApfelWidget->Apfel7_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_1(int)));
  QObject::connect (fApfelWidget->Apfel7_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_2(int)));
  QObject::connect (fApfelWidget->Apfel7_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_3(int)));
  QObject::connect (fApfelWidget->Apfel8_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_0(int)));
  QObject::connect (fApfelWidget->Apfel8_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_1(int)));
  QObject::connect (fApfelWidget->Apfel8_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_2(int)));
  QObject::connect (fApfelWidget->Apfel8_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_3(int)));

  QObject::connect (fApfelWidget->Apfel1_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_0 ()));
  QObject::connect (fApfelWidget->Apfel1_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_1 ()));
  QObject::connect (fApfelWidget->Apfel1_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_2 ()));
  QObject::connect (fApfelWidget->Apfel1_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_3 ()));
  QObject::connect (fApfelWidget->Apfel2_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_0 ()));
  QObject::connect (fApfelWidget->Apfel2_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_1 ()));
  QObject::connect (fApfelWidget->Apfel2_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_2 ()));
  QObject::connect (fApfelWidget->Apfel2_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_3 ()));
  QObject::connect (fApfelWidget->Apfel3_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_0 ()));
  QObject::connect (fApfelWidget->Apfel3_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_1 ()));
  QObject::connect (fApfelWidget->Apfel3_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_2 ()));
  QObject::connect (fApfelWidget->Apfel3_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_3 ()));
  QObject::connect (fApfelWidget->Apfel4_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_0 ()));
  QObject::connect (fApfelWidget->Apfel4_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_1 ()));
  QObject::connect (fApfelWidget->Apfel4_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_2 ()));
  QObject::connect (fApfelWidget->Apfel4_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_3 ()));
  QObject::connect (fApfelWidget->Apfel5_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_0 ()));
  QObject::connect (fApfelWidget->Apfel5_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_1 ()));
  QObject::connect (fApfelWidget->Apfel5_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_2 ()));
  QObject::connect (fApfelWidget->Apfel5_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_3 ()));
  QObject::connect (fApfelWidget->Apfel6_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_0 ()));
  QObject::connect (fApfelWidget->Apfel6_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_1 ()));
  QObject::connect (fApfelWidget->Apfel6_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_2 ()));
  QObject::connect (fApfelWidget->Apfel6_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_3 ()));
  QObject::connect (fApfelWidget->Apfel7_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_0 ()));
  QObject::connect (fApfelWidget->Apfel7_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_1 ()));
  QObject::connect (fApfelWidget->Apfel7_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_2 ()));
  QObject::connect (fApfelWidget->Apfel7_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_3 ()));
  QObject::connect (fApfelWidget->Apfel8_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_0 ()));
  QObject::connect (fApfelWidget->Apfel8_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_1 ()));
  QObject::connect (fApfelWidget->Apfel8_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_2 ()));
  QObject::connect (fApfelWidget->Apfel8_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_3 ()));

  QObject::connect (fApfelWidget->AutocalibrateButton_1, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_0 ()));
  QObject::connect (fApfelWidget->AutocalibrateButton_2, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_1 ()));
  QObject::connect (fApfelWidget->AutocalibrateButton_3, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_2 ()));
  QObject::connect (fApfelWidget->AutocalibrateButton_4, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_3 ()));
  QObject::connect (fApfelWidget->AutocalibrateButton_5, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_4 ()));
  QObject::connect (fApfelWidget->AutocalibrateButton_6, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_5 ()));
  QObject::connect (fApfelWidget->AutocalibrateButton_7, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_6 ()));
  QObject::connect (fApfelWidget->AutocalibrateButton_8, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_7 ()));

  QObject::connect (fApfelWidget->AutocalibrateButton_all, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_all ()));

  QObject::connect (fApfelWidget->PulserCheckBox_0, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_0()));
  QObject::connect (fApfelWidget->PulserCheckBox_1, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_0()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_0, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_0()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_1, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_0()));
  QObject::connect (fApfelWidget->ApfelTestPolarityBox_0, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_0()));

  QObject::connect (fApfelWidget->PulserCheckBox_2, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_1()));
  QObject::connect (fApfelWidget->PulserCheckBox_3, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_1()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_2, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_1()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_3, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_1()));
  QObject::connect (fApfelWidget->ApfelTestPolarityBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_1()));

  QObject::connect (fApfelWidget->PulserCheckBox_4, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_2()));
  QObject::connect (fApfelWidget->PulserCheckBox_5, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_2()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_4, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_2()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_5, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_2()));
  QObject::connect (fApfelWidget->ApfelTestPolarityBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_2()));

  QObject::connect (fApfelWidget->PulserCheckBox_6, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_3()));
  QObject::connect (fApfelWidget->PulserCheckBox_7, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_3()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_6, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_3()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_7, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_3()));
  QObject::connect (fApfelWidget->ApfelTestPolarityBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_3()));

  QObject::connect (fApfelWidget->PulserCheckBox_8, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_4()));
  QObject::connect (fApfelWidget->PulserCheckBox_9, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_4()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_8, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_4()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_9, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_4()));
  QObject::connect (fApfelWidget->ApfelTestPolarityBox_4, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_4()));

  QObject::connect (fApfelWidget->PulserCheckBox_10, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_5()));
  QObject::connect (fApfelWidget->PulserCheckBox_11, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_5()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_10, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_5()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_11, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_5()));
  QObject::connect (fApfelWidget->ApfelTestPolarityBox_5, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_5()));

  QObject::connect (fApfelWidget->PulserCheckBox_12, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_6()));
  QObject::connect (fApfelWidget->PulserCheckBox_13, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_6()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_12, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_6()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_13, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_6()));
  QObject::connect (fApfelWidget->ApfelTestPolarityBox_6, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_6()));

  QObject::connect (fApfelWidget->PulserCheckBox_14, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_7()));
  QObject::connect (fApfelWidget->PulserCheckBox_15, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_7()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_14, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_7()));
  QObject::connect (fApfelWidget->PulserAmpSpinBox_15, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_7()));

  QObject::connect (fApfelWidget->ApfelTestPolarityBox_7, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_7()));

  QObject::connect (fApfelWidget->gainCombo_0, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_0()));
  QObject::connect (fApfelWidget->gainCombo_1, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_1()));
  QObject::connect (fApfelWidget->gainCombo_2, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_2()));
  QObject::connect (fApfelWidget->gainCombo_3, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_3()));
  QObject::connect (fApfelWidget->gainCombo_4, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_4()));
  QObject::connect (fApfelWidget->gainCombo_5, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_5()));
  QObject::connect (fApfelWidget->gainCombo_6, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_6()));
  QObject::connect (fApfelWidget->gainCombo_7, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_7()));
  QObject::connect (fApfelWidget->gainCombo_8, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_8()));
  QObject::connect (fApfelWidget->gainCombo_9, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_9()));
  QObject::connect (fApfelWidget->gainCombo_10, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_10()));
  QObject::connect (fApfelWidget->gainCombo_11, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_11()));
  QObject::connect (fApfelWidget->gainCombo_12, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_12()));
  QObject::connect (fApfelWidget->gainCombo_13, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_13()));
  QObject::connect (fApfelWidget->gainCombo_14, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_14()));
  QObject::connect (fApfelWidget->gainCombo_15, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_15()));

  QObject::connect (fApfelWidget->ApfelRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));
  QObject::connect (fApfelWidget->LoGainRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));
  QObject::connect (fApfelWidget->StretcherOnRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));

  QObject::connect (fApfelWidget->InverseMappingCheckBox, SIGNAL(stateChanged(int)), this, SLOT (InverseMapping_changed(int)));

  QObject::connect (fApfelWidget->DoSampleButton, SIGNAL (clicked ()), this, SLOT (AcquireSamplesBtn_clicked ()));
  QObject::connect (fApfelWidget->DumpSampleButton, SIGNAL (clicked ()), this, SLOT (DumpSamplesBtn_clicked ()));

  QObject::connect (fApfelWidget->ZoomButton, SIGNAL (clicked ()), this, SLOT (ZoomSampleBtn_clicked ()));
  QObject::connect (fApfelWidget->UnzoomButton, SIGNAL (clicked ()), this, SLOT (UnzoomSampleBtn_clicked ()));
  QObject::connect (fApfelWidget->RefreshSampleButton, SIGNAL (clicked ()), this, SLOT (RefreshSampleBtn_clicked ()));

  QObject::connect (fApfelWidget->PeakFinderButton, SIGNAL (clicked ()), this, SLOT (PeakFinderBtn_clicked ()));

  QObject::connect (fApfelWidget->PulseTimerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(PulseTimer_changed(int)));
  QObject::connect (fApfelWidget->FrequencyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT (PulseFrequencyChanged(int)));

  QObject::connect (fApfelWidget->PulseBroadcastCheckBox, SIGNAL(stateChanged(int)), this, SLOT(PulseBroadcast_changed(int)));

//  QObject::connect (BenchmarkButtonBox, SIGNAL(accepted()), this, SLOT(StartBenchmarkPressed()));
//  QObject::connect (BenchmarkButtonBox, SIGNAL(rejected()), this, SLOT(CancelBenchmarkPressed()));

  QObject::connect (fApfelWidget->BenchmarkButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(BenchmarkPressed(QAbstractButton*)));

  QObject::connect (fApfelWidget->ReferenceLoadButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ChangeReferenceDataPressed(QAbstractButton*)));


  QObject::connect (fApfelWidget->PlotTabWidget, SIGNAL(currentChanged(int)), this, SLOT (PlotTabChanged(int)));

  QObject::connect (fApfelWidget->MaximaTableWidget, SIGNAL(cellDoubleClicked(int, int )), this, SLOT (MaximaCellDoubleClicked(int,int)));


  QObject::connect (fApfelWidget->BaselineLowerSlider, SIGNAL(valueChanged(int)), this, SLOT (RefreshBaselines()));
  QObject::connect (fApfelWidget->BaselineUpperSlider, SIGNAL(valueChanged(int)), this, SLOT (RefreshBaselines()));
  QObject::connect (fApfelWidget->ReadoutRadioButton, SIGNAL(toggled(bool)), this, SLOT (RefreshBaselines()));

  /** JAM put references to designer checkboxes into array to be handled later easily: */
  fBaselineBoxes[0] = fApfelWidget->Baseline_Box_00;
  fBaselineBoxes[1] = fApfelWidget->Baseline_Box_01;
  fBaselineBoxes[2] = fApfelWidget->Baseline_Box_02;
  fBaselineBoxes[3] = fApfelWidget->Baseline_Box_03;
  fBaselineBoxes[4] = fApfelWidget->Baseline_Box_04;
  fBaselineBoxes[5] = fApfelWidget->Baseline_Box_05;
  fBaselineBoxes[6] = fApfelWidget->Baseline_Box_06;
  fBaselineBoxes[7] = fApfelWidget->Baseline_Box_07;
  fBaselineBoxes[8] = fApfelWidget->Baseline_Box_08;
  fBaselineBoxes[9] = fApfelWidget->Baseline_Box_09;
  fBaselineBoxes[10] = fApfelWidget->Baseline_Box_10;
  fBaselineBoxes[11] = fApfelWidget->Baseline_Box_11;
  fBaselineBoxes[12] = fApfelWidget->Baseline_Box_12;
  fBaselineBoxes[13] = fApfelWidget->Baseline_Box_13;
  fBaselineBoxes[14] = fApfelWidget->Baseline_Box_14;
  fBaselineBoxes[15] = fApfelWidget->Baseline_Box_15;

  fDACSpinBoxes[0] = fApfelWidget->DAC_spinBox_00;
  fDACSpinBoxes[1] = fApfelWidget->DAC_spinBox_01;
  fDACSpinBoxes[2] = fApfelWidget->DAC_spinBox_02;
  fDACSpinBoxes[3] = fApfelWidget->DAC_spinBox_03;
  fDACSpinBoxes[4] = fApfelWidget->DAC_spinBox_04;
  fDACSpinBoxes[5] = fApfelWidget->DAC_spinBox_05;
  fDACSpinBoxes[6] = fApfelWidget->DAC_spinBox_06;
  fDACSpinBoxes[7] = fApfelWidget->DAC_spinBox_07;
  fDACSpinBoxes[8] = fApfelWidget->DAC_spinBox_08;
  fDACSpinBoxes[9] = fApfelWidget->DAC_spinBox_09;
  fDACSpinBoxes[10] = fApfelWidget->DAC_spinBox_10;
  fDACSpinBoxes[11] = fApfelWidget->DAC_spinBox_11;
  fDACSpinBoxes[12] = fApfelWidget->DAC_spinBox_12;
  fDACSpinBoxes[13] = fApfelWidget->DAC_spinBox_13;
  fDACSpinBoxes[14] = fApfelWidget->DAC_spinBox_14;
  fDACSpinBoxes[15] = fApfelWidget->DAC_spinBox_15;

  fADCLineEdit[0] = fApfelWidget->ADC_Value_00;
  fADCLineEdit[1] = fApfelWidget->ADC_Value_01;
  fADCLineEdit[2] = fApfelWidget->ADC_Value_02;
  fADCLineEdit[3] = fApfelWidget->ADC_Value_03;
  fADCLineEdit[4] = fApfelWidget->ADC_Value_04;
  fADCLineEdit[5] = fApfelWidget->ADC_Value_05;
  fADCLineEdit[6] = fApfelWidget->ADC_Value_06;
  fADCLineEdit[7] = fApfelWidget->ADC_Value_07;
  fADCLineEdit[8] = fApfelWidget->ADC_Value_08;
  fADCLineEdit[9] = fApfelWidget->ADC_Value_09;
  fADCLineEdit[10] = fApfelWidget->ADC_Value_10;
  fADCLineEdit[11] = fApfelWidget->ADC_Value_11;
  fADCLineEdit[12] = fApfelWidget->ADC_Value_12;
  fADCLineEdit[13] = fApfelWidget->ADC_Value_13;
  fADCLineEdit[14] = fApfelWidget->ADC_Value_14;
  fADCLineEdit[15] = fApfelWidget->ADC_Value_15;

  fDACSlider[0][0] = fApfelWidget->Apfel1_DACSlider_1;
  fDACSlider[0][1] = fApfelWidget->Apfel1_DACSlider_2;
  fDACSlider[0][2] = fApfelWidget->Apfel1_DACSlider_3;
  fDACSlider[0][3] = fApfelWidget->Apfel1_DACSlider_4;
  fDACSlider[1][0] = fApfelWidget->Apfel2_DACSlider_1;
  fDACSlider[1][1] = fApfelWidget->Apfel2_DACSlider_2;
  fDACSlider[1][2] = fApfelWidget->Apfel2_DACSlider_3;
  fDACSlider[1][3] = fApfelWidget->Apfel2_DACSlider_4;
  fDACSlider[2][0] = fApfelWidget->Apfel3_DACSlider_1;
  fDACSlider[2][1] = fApfelWidget->Apfel3_DACSlider_2;
  fDACSlider[2][2] = fApfelWidget->Apfel3_DACSlider_3;
  fDACSlider[2][3] = fApfelWidget->Apfel3_DACSlider_4;
  fDACSlider[3][0] = fApfelWidget->Apfel4_DACSlider_1;
  fDACSlider[3][1] = fApfelWidget->Apfel4_DACSlider_2;
  fDACSlider[3][2] = fApfelWidget->Apfel4_DACSlider_3;
  fDACSlider[3][3] = fApfelWidget->Apfel4_DACSlider_4;
  fDACSlider[4][0] = fApfelWidget->Apfel5_DACSlider_1;
  fDACSlider[4][1] = fApfelWidget->Apfel5_DACSlider_2;
  fDACSlider[4][2] = fApfelWidget->Apfel5_DACSlider_3;
  fDACSlider[4][3] = fApfelWidget->Apfel5_DACSlider_4;
  fDACSlider[5][0] = fApfelWidget->Apfel6_DACSlider_1;
  fDACSlider[5][1] = fApfelWidget->Apfel6_DACSlider_2;
  fDACSlider[5][2] = fApfelWidget->Apfel6_DACSlider_3;
  fDACSlider[5][3] = fApfelWidget->Apfel6_DACSlider_4;
  fDACSlider[6][0] = fApfelWidget->Apfel7_DACSlider_1;
  fDACSlider[6][1] = fApfelWidget->Apfel7_DACSlider_2;
  fDACSlider[6][2] = fApfelWidget->Apfel7_DACSlider_3;
  fDACSlider[6][3] = fApfelWidget->Apfel7_DACSlider_4;
  fDACSlider[7][0] = fApfelWidget->Apfel8_DACSlider_1;
  fDACSlider[7][1] = fApfelWidget->Apfel8_DACSlider_2;
  fDACSlider[7][2] = fApfelWidget->Apfel8_DACSlider_3;
  fDACSlider[7][3] = fApfelWidget->Apfel8_DACSlider_4;

  fDACLineEdit[0][0] = fApfelWidget->Apfel1_DAClineEdit_1;
  fDACLineEdit[0][1] = fApfelWidget->Apfel1_DAClineEdit_2;
  fDACLineEdit[0][2] = fApfelWidget->Apfel1_DAClineEdit_3;
  fDACLineEdit[0][3] = fApfelWidget->Apfel1_DAClineEdit_4;
  fDACLineEdit[1][0] = fApfelWidget->Apfel2_DAClineEdit_1;
  fDACLineEdit[1][1] = fApfelWidget->Apfel2_DAClineEdit_2;
  fDACLineEdit[1][2] = fApfelWidget->Apfel2_DAClineEdit_3;
  fDACLineEdit[1][3] = fApfelWidget->Apfel2_DAClineEdit_4;
  fDACLineEdit[2][0] = fApfelWidget->Apfel3_DAClineEdit_1;
  fDACLineEdit[2][1] = fApfelWidget->Apfel3_DAClineEdit_2;
  fDACLineEdit[2][2] = fApfelWidget->Apfel3_DAClineEdit_3;
  fDACLineEdit[2][3] = fApfelWidget->Apfel3_DAClineEdit_4;
  fDACLineEdit[3][0] = fApfelWidget->Apfel4_DAClineEdit_1;
  fDACLineEdit[3][1] = fApfelWidget->Apfel4_DAClineEdit_2;
  fDACLineEdit[3][2] = fApfelWidget->Apfel4_DAClineEdit_3;
  fDACLineEdit[3][3] = fApfelWidget->Apfel4_DAClineEdit_4;
  fDACLineEdit[4][0] = fApfelWidget->Apfel5_DAClineEdit_1;
  fDACLineEdit[4][1] = fApfelWidget->Apfel5_DAClineEdit_2;
  fDACLineEdit[4][2] = fApfelWidget->Apfel5_DAClineEdit_3;
  fDACLineEdit[4][3] = fApfelWidget->Apfel5_DAClineEdit_4;
  fDACLineEdit[5][0] = fApfelWidget->Apfel6_DAClineEdit_1;
  fDACLineEdit[5][1] = fApfelWidget->Apfel6_DAClineEdit_2;
  fDACLineEdit[5][2] = fApfelWidget->Apfel6_DAClineEdit_3;
  fDACLineEdit[5][3] = fApfelWidget->Apfel6_DAClineEdit_4;
  fDACLineEdit[6][0] = fApfelWidget->Apfel7_DAClineEdit_1;
  fDACLineEdit[6][1] = fApfelWidget->Apfel7_DAClineEdit_2;
  fDACLineEdit[6][2] = fApfelWidget->Apfel7_DAClineEdit_3;
  fDACLineEdit[6][3] = fApfelWidget->Apfel7_DAClineEdit_4;
  fDACLineEdit[7][0] = fApfelWidget->Apfel8_DAClineEdit_1;
  fDACLineEdit[7][1] = fApfelWidget->Apfel8_DAClineEdit_2;
  fDACLineEdit[7][2] = fApfelWidget->Apfel8_DAClineEdit_3;
  fDACLineEdit[7][3] = fApfelWidget->Apfel8_DAClineEdit_4;

  fApfelPulsePolarityCombo[0] = fApfelWidget->ApfelTestPolarityBox_0;
  fApfelPulsePolarityCombo[1] = fApfelWidget->ApfelTestPolarityBox_1;
  fApfelPulsePolarityCombo[2] = fApfelWidget->ApfelTestPolarityBox_2;
  fApfelPulsePolarityCombo[3] = fApfelWidget->ApfelTestPolarityBox_3;
  fApfelPulsePolarityCombo[4] = fApfelWidget->ApfelTestPolarityBox_4;
  fApfelPulsePolarityCombo[5] = fApfelWidget->ApfelTestPolarityBox_5;
  fApfelPulsePolarityCombo[6] = fApfelWidget->ApfelTestPolarityBox_6;
  fApfelPulsePolarityCombo[7] = fApfelWidget->ApfelTestPolarityBox_7;

  fApfelPulseEnabledCheckbox[0][0] = fApfelWidget->PulserCheckBox_0;
  fApfelPulseEnabledCheckbox[0][1] = fApfelWidget->PulserCheckBox_1;
  fApfelPulseEnabledCheckbox[1][0] = fApfelWidget->PulserCheckBox_2;
  fApfelPulseEnabledCheckbox[1][1] = fApfelWidget->PulserCheckBox_3;
  fApfelPulseEnabledCheckbox[2][0] = fApfelWidget->PulserCheckBox_4;
  fApfelPulseEnabledCheckbox[2][1] = fApfelWidget->PulserCheckBox_5;
  fApfelPulseEnabledCheckbox[3][0] = fApfelWidget->PulserCheckBox_6;
  fApfelPulseEnabledCheckbox[3][1] = fApfelWidget->PulserCheckBox_7;
  fApfelPulseEnabledCheckbox[4][0] = fApfelWidget->PulserCheckBox_8;
  fApfelPulseEnabledCheckbox[4][1] = fApfelWidget->PulserCheckBox_9;
  fApfelPulseEnabledCheckbox[5][0] = fApfelWidget->PulserCheckBox_10;
  fApfelPulseEnabledCheckbox[5][1] = fApfelWidget->PulserCheckBox_11;
  fApfelPulseEnabledCheckbox[6][0] = fApfelWidget->PulserCheckBox_12;
  fApfelPulseEnabledCheckbox[6][1] = fApfelWidget->PulserCheckBox_13;
  fApfelPulseEnabledCheckbox[7][0] = fApfelWidget->PulserCheckBox_14;
  fApfelPulseEnabledCheckbox[7][1] = fApfelWidget->PulserCheckBox_15;

  fApfelPulseAmplitudeSpin[0][0] = fApfelWidget->PulserAmpSpinBox_0;
  fApfelPulseAmplitudeSpin[0][1] = fApfelWidget->PulserAmpSpinBox_1;
  fApfelPulseAmplitudeSpin[1][0] = fApfelWidget->PulserAmpSpinBox_2;
  fApfelPulseAmplitudeSpin[1][1] = fApfelWidget->PulserAmpSpinBox_3;
  fApfelPulseAmplitudeSpin[2][0] = fApfelWidget->PulserAmpSpinBox_4;
  fApfelPulseAmplitudeSpin[2][1] = fApfelWidget->PulserAmpSpinBox_5;
  fApfelPulseAmplitudeSpin[3][0] = fApfelWidget->PulserAmpSpinBox_6;
  fApfelPulseAmplitudeSpin[3][1] = fApfelWidget->PulserAmpSpinBox_7;
  fApfelPulseAmplitudeSpin[4][0] = fApfelWidget->PulserAmpSpinBox_8;
  fApfelPulseAmplitudeSpin[4][1] = fApfelWidget->PulserAmpSpinBox_9;
  fApfelPulseAmplitudeSpin[5][0] = fApfelWidget->PulserAmpSpinBox_10;
  fApfelPulseAmplitudeSpin[5][1] = fApfelWidget->PulserAmpSpinBox_11;
  fApfelPulseAmplitudeSpin[6][0] = fApfelWidget->PulserAmpSpinBox_12;
  fApfelPulseAmplitudeSpin[6][1] = fApfelWidget->PulserAmpSpinBox_13;
  fApfelPulseAmplitudeSpin[7][0] = fApfelWidget->PulserAmpSpinBox_14;
  fApfelPulseAmplitudeSpin[7][1] = fApfelWidget->PulserAmpSpinBox_15;

  fApfelPulseGroup[0] = fApfelWidget->ApfelPulseBox_1;
  fApfelPulseGroup[1] = fApfelWidget->ApfelPulseBox_2;
  fApfelPulseGroup[2] = fApfelWidget->ApfelPulseBox_3;
  fApfelPulseGroup[3] = fApfelWidget->ApfelPulseBox_4;
  fApfelPulseGroup[4] = fApfelWidget->ApfelPulseBox_5;
  fApfelPulseGroup[5] = fApfelWidget->ApfelPulseBox_6;
  fApfelPulseGroup[6] = fApfelWidget->ApfelPulseBox_7;
  fApfelPulseGroup[7] = fApfelWidget->ApfelPulseBox_8;

  fApfelGainCombo[0][0] = fApfelWidget->gainCombo_0;
  fApfelGainCombo[0][1] = fApfelWidget->gainCombo_1;
  fApfelGainCombo[1][0] = fApfelWidget->gainCombo_2;
  fApfelGainCombo[1][1] = fApfelWidget->gainCombo_3;
  fApfelGainCombo[2][0] = fApfelWidget->gainCombo_4;
  fApfelGainCombo[2][1] = fApfelWidget->gainCombo_5;
  fApfelGainCombo[3][0] = fApfelWidget->gainCombo_6;
  fApfelGainCombo[3][1] = fApfelWidget->gainCombo_7;
  fApfelGainCombo[4][0] = fApfelWidget->gainCombo_8;
  fApfelGainCombo[4][1] = fApfelWidget->gainCombo_9;
  fApfelGainCombo[5][0] = fApfelWidget->gainCombo_10;
  fApfelGainCombo[5][1] = fApfelWidget->gainCombo_11;
  fApfelGainCombo[6][0] = fApfelWidget->gainCombo_12;
  fApfelGainCombo[6][1] = fApfelWidget->gainCombo_13;
  fApfelGainCombo[7][0] = fApfelWidget->gainCombo_14;
  fApfelGainCombo[7][1] = fApfelWidget->gainCombo_15;

  fSamplingBoxes[0] = fApfelWidget->Sampling_Box_0;
  fSamplingBoxes[1] = fApfelWidget->Sampling_Box_1;
  fSamplingBoxes[2] = fApfelWidget->Sampling_Box_2;
  fSamplingBoxes[3] = fApfelWidget->Sampling_Box_3;
  fSamplingBoxes[4] = fApfelWidget->Sampling_Box_4;
  fSamplingBoxes[5] = fApfelWidget->Sampling_Box_5;
  fSamplingBoxes[6] = fApfelWidget->Sampling_Box_6;
  fSamplingBoxes[7] = fApfelWidget->Sampling_Box_7;
  fSamplingBoxes[8] = fApfelWidget->Sampling_Box_8;
  fSamplingBoxes[9] = fApfelWidget->Sampling_Box_9;
  fSamplingBoxes[10] = fApfelWidget->Sampling_Box_10;
  fSamplingBoxes[11] = fApfelWidget->Sampling_Box_11;
  fSamplingBoxes[12] = fApfelWidget->Sampling_Box_12;
  fSamplingBoxes[13] = fApfelWidget->Sampling_Box_13;
  fSamplingBoxes[14] = fApfelWidget->Sampling_Box_14;
  fSamplingBoxes[15] = fApfelWidget->Sampling_Box_15;

  fSamplingMeanLineEdit[0] = fApfelWidget->ADC_SampleMean_0;
  fSamplingMeanLineEdit[1] = fApfelWidget->ADC_SampleMean_1;
  fSamplingMeanLineEdit[2] = fApfelWidget->ADC_SampleMean_2;
  fSamplingMeanLineEdit[3] = fApfelWidget->ADC_SampleMean_3;
  fSamplingMeanLineEdit[4] = fApfelWidget->ADC_SampleMean_4;
  fSamplingMeanLineEdit[5] = fApfelWidget->ADC_SampleMean_5;
  fSamplingMeanLineEdit[6] = fApfelWidget->ADC_SampleMean_6;
  fSamplingMeanLineEdit[7] = fApfelWidget->ADC_SampleMean_7;
  fSamplingMeanLineEdit[8] = fApfelWidget->ADC_SampleMean_8;
  fSamplingMeanLineEdit[9] = fApfelWidget->ADC_SampleMean_9;
  fSamplingMeanLineEdit[10] = fApfelWidget->ADC_SampleMean_10;
  fSamplingMeanLineEdit[11] = fApfelWidget->ADC_SampleMean_11;
  fSamplingMeanLineEdit[12] = fApfelWidget->ADC_SampleMean_12;
  fSamplingMeanLineEdit[13] = fApfelWidget->ADC_SampleMean_13;
  fSamplingMeanLineEdit[14] = fApfelWidget->ADC_SampleMean_14;
  fSamplingMeanLineEdit[15] = fApfelWidget->ADC_SampleMean_15;

  fSamplingSigmaLineEdit[0] = fApfelWidget->ADC_SampleSigma_0;
  fSamplingSigmaLineEdit[1] = fApfelWidget->ADC_SampleSigma_1;
  fSamplingSigmaLineEdit[2] = fApfelWidget->ADC_SampleSigma_2;
  fSamplingSigmaLineEdit[3] = fApfelWidget->ADC_SampleSigma_3;
  fSamplingSigmaLineEdit[4] = fApfelWidget->ADC_SampleSigma_4;
  fSamplingSigmaLineEdit[5] = fApfelWidget->ADC_SampleSigma_5;
  fSamplingSigmaLineEdit[6] = fApfelWidget->ADC_SampleSigma_6;
  fSamplingSigmaLineEdit[7] = fApfelWidget->ADC_SampleSigma_7;
  fSamplingSigmaLineEdit[8] = fApfelWidget->ADC_SampleSigma_8;
  fSamplingSigmaLineEdit[9] = fApfelWidget->ADC_SampleSigma_9;
  fSamplingSigmaLineEdit[10] = fApfelWidget->ADC_SampleSigma_10;
  fSamplingSigmaLineEdit[11] = fApfelWidget->ADC_SampleSigma_11;
  fSamplingSigmaLineEdit[12] = fApfelWidget->ADC_SampleSigma_12;
  fSamplingSigmaLineEdit[13] = fApfelWidget->ADC_SampleSigma_13;
  fSamplingSigmaLineEdit[14] = fApfelWidget->ADC_SampleSigma_14;
  fSamplingSigmaLineEdit[15] = fApfelWidget->ADC_SampleSigma_15;

  fPlotWidget[0] = fApfelWidget->PlotWidget_0;
  fPlotWidget[1] = fApfelWidget->PlotWidget_1;
  fPlotWidget[2] = fApfelWidget->PlotWidget_2;
  fPlotWidget[3] = fApfelWidget->PlotWidget_3;
  fPlotWidget[4] = fApfelWidget->PlotWidget_4;
  fPlotWidget[5] = fApfelWidget->PlotWidget_5;
  fPlotWidget[6] = fApfelWidget->PlotWidget_6;
  fPlotWidget[7] = fApfelWidget->PlotWidget_7;
  fPlotWidget[8] = fApfelWidget->PlotWidget_8;
  fPlotWidget[9] = fApfelWidget->PlotWidget_9;
  fPlotWidget[10] = fApfelWidget->PlotWidget_10;
  fPlotWidget[11] = fApfelWidget->PlotWidget_11;
  fPlotWidget[12] = fApfelWidget->PlotWidget_12;
  fPlotWidget[13] = fApfelWidget->PlotWidget_13;
  fPlotWidget[14] = fApfelWidget->PlotWidget_14;
  fPlotWidget[15] = fApfelWidget->PlotWidget_15;

  // labels for plot area:
  for (int i = 0; i < 16; ++i)
  {
    fPlotWidget[i]->axis (KPlotWidget::BottomAxis)->setLabel ("Time (#samples)");
    fPlotWidget[i]->axis (KPlotWidget::LeftAxis)->setLabel ("ADC value ");
  }

  // timers for frequent test pulse:
  fPulserTimer = new QTimer (this);
  QObject::connect (fPulserTimer, SIGNAL (timeout ()), this, SLOT (PulserTimeout ()));

  fDisplayTimer = new QTimer (this);
  fDisplayTimer->setInterval (500);
  QObject::connect (fDisplayTimer, SIGNAL (timeout ()), this, SLOT (PulserDisplayTimeout ()));

  // benchmark testing procedure related:
  fApfelWidget->BenchmarkButtonBox->button (QDialogButtonBox::Apply)->setDefault (true);

  fSequencerTimer = new QTimer (this);
  fSequencerTimer->setInterval (20);
  QObject::connect (fSequencerTimer, SIGNAL (timeout ()), this, SLOT (BenchmarkTimerCallback ()));


  fBenchmark.SetOwner (this);
  //fBenchmark.LoadReferenceValues (QString ("default.apf"));

  show ();

  // start with preferred situation:
  ShowBtn_clicked ();
  checkBox_AA->setChecked (true);

}

ApfelGui::~ApfelGui ()
{

}




void ApfelGui::ApplyFileConfig(int )
{
    GosipGui::ApplyFileConfig(900); // adjust bus wait time to 900 us
}


int ApfelGui::OpenTestFile (const QString& fname)
{
  fTestFile = fopen (fname.toLatin1 ().constData (), "w");
  if (fTestFile == NULL)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error opening Characterization result File '%s': %s\n", fname.toLatin1 ().constData (),
        strerror (errno));
    AppendTextWindow (buffer);
    return -1;
  }
  QString timestring = QDateTime::currentDateTime ().toString ("ddd dd.MM.yyyy hh:mm:ss");
  WriteConfigFile (QString ("# Apfel characterization test file saved on ") + timestring + QString ("\n"));
  return 0;
}

int ApfelGui::CloseTestFile ()
{
  int rev = 0;
  if (fTestFile == NULL)
    return 0;
  if (fclose (fTestFile) != 0)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error closing characterization test file! (%s)\n", strerror (errno));
    AppendTextWindow (buffer);
    rev = -1;
  }
  fTestFile = NULL;    // must not use handle again even if close fails
  return rev;
}

int ApfelGui::WriteTestFile (const QString& text)
{
  if (fTestFile == NULL)
    return -1;
  if (fprintf (fTestFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}






void ApfelGui::AutoAdjust ()
{
  if (!AssertChainConfigured ())
    return;
  QString targetstring = fApfelWidget->ADCAdjustValue->text ();
  unsigned targetvalue = targetstring.toUInt (0, fNumberBase);
  //std::cout <<"string="<<targetstring.toLatin1 ().constData ()<< ", targetvalue="<< targetvalue<< std::endl;
  for (int channel = 0; channel < 16; ++channel)
  {
    if (fBaselineBoxes[channel]->isChecked ())
    {
      int dac = AdjustBaseline (channel, targetvalue);
      fDACSpinBoxes[channel]->setValue (dac);
      AutoApplyRefresh (channel, dac);    // once again apply dac settings to immediately see the baseline on gui
      printm ("--- Auto adjusted baselines of sfp:%d board:%d channel:%d to value:%d =>%d permille DAC", fSFP, fSlave,
          channel, targetvalue, dac);
    }
  }
}

int ApfelGui::AdjustBaseline (int channel, int adctarget)
{
  int dac = 500;    // dac setup in per mille here, start in the middle
  int dacstep = 250;
  int validdac = -1;
  int adc = 0;
  int escapecounter = 10;
  bool upwards = true;    // scan direction up or down
  bool changed = false;    // do we have changed scan direction?
  bool initial = true;    // supress evaluation of scan direction at first cycle
  //std::cout << "ApfelGui::AdjustBaseline of channel "<<channel<<" to "<<adctarget<< std::endl;

  double resolution = 1.0 / APFEL_DAC_MAXVALUE * 0x3FFF / 2;    // for 14 bit febex ADC

  do
  {
    adc = autoApply (channel, dac);    // this gives already mean value of 3 adc samples
    if (adc < 0)
      break;    // avoid broadcast
    validdac = dac;
    //std::cout << "ApfelGui::AdjustBaseline after autoApply of dac:"<<dac<<" gets adc:"<<adc<<", resolution:"<<resolution<< std::endl;
    if (adc < adctarget)
    {
      dac += dacstep;
      changed = (!upwards ? true : false);
      upwards = true;
      if (dac > 1000)
        dac = 1000;
    }
    else if (adc > adctarget)
    {
      dac -= dacstep;
      changed = (upwards ? true : false);
      upwards = false;
      if (dac < 0)
        dac = 0;
    }
    else
      break;
    if (changed && !initial && dacstep > 1)
      dacstep = dacstep >> 1;
    if (dacstep < 1)
      break;
    if (!changed || dacstep == 1)
      escapecounter--;    // get us out of loop if user wants to reach value outside adc range, or if we oscillate around target value
    initial = false;
  } while (fabs (adc - adctarget) >= resolution && escapecounter);
  //std::cout << "   ApfelGui::AdjustBaseline after loop dac:"<<validdac<<" adc:"<<adc<<", resolution:"<<resolution<< std::endl;
  return validdac;
}


void ApfelGui::CalibrateSelectedADCs ()
{
  if (!AssertChainConfigured ())
    return;
  for (int channel = 0; channel < 16; ++channel)
  {
    if (fBaselineBoxes[channel]->isChecked ())
    {
      CalibrateADC (channel);
    }
  }
}

int ApfelGui::CalibrateADC (int channel)
{

  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);

  int apfel = 0, dac = 0;
  theSetup->EvaluateDACIndices (channel, apfel, dac);

  // DO NOT first autocalibrate DAC that belongs to selected channel
  // we decouple chip autocalibration from channel calibration curve now
  //DoAutoCalibrate(apfel);

  // measure slope of DAC kennlinie by differential variation:
  int gain = theSetup->GetGain (apfel, dac);
  int valDAC = theSetup->GetDACValue (apfel, dac);    // current value after automatic DAC calibration
  int valADC = AcquireBaselineSample (channel);    // fetch a sample

  EnableI2C ();
#ifdef  APFEL_DAC_LOCALCALIB
  int deltaDAC=APFEL_DAC_DELTACALIB;

  int valDAC_upper=valDAC+deltaDAC;

  // now do variation and measure new adc value:

  WriteDAC_ApfelI2c (apfel, dac, valDAC_upper);
  int valADC_upper=AcquireBaselineSample(channel);
  int deltaADC=valADC_upper-valADC;
#endif

  int valADC_max = 0, valADC_min = 0, valDAC_min = 0, valDAC_max = 0, valADC_sample = 0;

  if (gain == 1)
  {
    // special situation for gain 1: slope is inverted, need to do different procedure:

    // get minimum ADC value by setting DAC to min:
    WriteDAC_ApfelI2c (apfel, dac, 0);
    valADC_min = AcquireBaselineSample (channel);

    int valDAC_min = 0;
    int valADC_deltasaturation = APFEL_ADC_SATURATIONDELTA;

    // shift DAC up ADCmax changes significantly. this gives effective DAC min
    for (valDAC_min = 0; valDAC_min < APFEL_DAC_MAXVALUE; valDAC_min += APFEL_DAC_DELTASTEP)
    {
      WriteDAC_ApfelI2c (apfel, dac, valDAC_min);
      int samp = AcquireBaselineSample (channel);
      if (samp - valADC_min > valADC_deltasaturation)
        break;
      valADC_sample = samp;
      //std::cout <<"Searching effective maximum  DAC:"<<valDAC_max<<", ADC:"<<valADC_sample<< std::endl;
    }
    valDAC_min -= APFEL_DAC_DELTASTEP;    // rewind to point that is still in pedestal region

    //shift DAC farther upwards until we find ADC saturation:
    valDAC_max = 0;
    int valADC_step = 0;
    for (valDAC_max = valDAC_min; valDAC_max < APFEL_DAC_MAXVALUE; valDAC_max += APFEL_DAC_DELTASTEP)
    {
      WriteDAC_ApfelI2c (apfel, dac, valDAC_max);
      valADC_step = AcquireBaselineSample (channel);
      if (valADC_step >= APFEL_ADC_MAXSATURATION)
        break;
      //std::cout <<"Searching ADC saturation: DAC:"<<valDAC_max<<", ADC:"<<valADC_step<< std::endl;

    }
    printm ("found ADC_min=%d, DAC_min=%d, DAC_max=%d", valADC_min, valDAC_min, valDAC_max);
    theSetup->SetDACmin (gain, channel, valDAC_min);
    theSetup->SetDACmax (gain, channel, valDAC_max);
    theSetup->SetADCmin (gain, channel, valADC_min);

    // linear calibration only in non saturated ADC range:
    int deltaDAC = valDAC_max - valDAC_min;
    int deltaADC = APFEL_ADC_MAXSATURATION - valADC_min;
    theSetup->EvaluateCalibration (gain, channel, deltaDAC, deltaADC, valDAC_min, valADC_sample);
  }
  else
  {
    // evaluate range boundaries:
    // get minimum ADC value by setting DAC to max:
    WriteDAC_ApfelI2c (apfel, dac, APFEL_DAC_MAXVALUE);
    int valADC_min = AcquireBaselineSample (channel);

    valDAC_max = APFEL_DAC_MAXVALUE;
    //int valADC_sample;
    int valADC_deltasaturation = APFEL_ADC_SATURATIONDELTA;

    // shift DAC down until ADCmin changes significantly. this gives effective DAC max
    for (valDAC_max = APFEL_DAC_MAXVALUE; valDAC_max > 0; valDAC_max -= APFEL_DAC_DELTASTEP)
    {
      WriteDAC_ApfelI2c (apfel, dac, valDAC_max);
      int samp = AcquireBaselineSample (channel);
      if (samp - valADC_min > valADC_deltasaturation)
        break;
      valADC_sample = samp;
      //std::cout <<"Searching effective maximum  DAC:"<<valDAC_max<<", ADC:"<<valADC_sample<< std::endl;
    }
    valDAC_max += APFEL_DAC_DELTASTEP;    // rewind to point that is still in pedestal region

    //shift DAC farther downwards until we find ADC saturation:
    valDAC_min = 0;
    int valADC_step = 0;
    for (valDAC_min = valDAC_max; valDAC_min > 0; valDAC_min -= APFEL_DAC_DELTASTEP)
    {
      WriteDAC_ApfelI2c (apfel, dac, valDAC_min);
      valADC_step = AcquireBaselineSample (channel);
      if (valADC_step >= APFEL_ADC_MAXSATURATION)
        break;
      //std::cout <<"Searching ADC saturation: DAC:"<<valDAC_min<<", ADC:"<<valADC_step<< std::endl;

    }
    printm ("found ADC_min=%d, DAC_min=%d, DAC_max=%d", valADC_min, valDAC_min, valDAC_max);
    theSetup->SetDACmin (gain, channel, valDAC_min);
    theSetup->SetDACmax (gain, channel, valDAC_max);
    theSetup->SetADCmin (gain, channel, valADC_min);

    // linear calibration only in non saturated ADC range:
    int deltaDAC = valDAC_max - valDAC_min;
    int deltaADC = valADC_min - APFEL_ADC_MAXSATURATION;
    theSetup->EvaluateCalibration (gain, channel, deltaDAC, deltaADC, valDAC_max, valADC_sample);

  }

  // shift back to middle of calibration:
  WriteDAC_ApfelI2c (apfel, dac, valDAC);
  DisableI2C ();
  printm ("--- Calibrated DAC->ADC slider for sfp:%d board:%d channel:%d apfel:%d dac:%d -", fSFP, fSlave, channel,
      apfel, dac);

#ifdef  APFEL_DAC_LOCALCALIB
  theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC, valADC);
#else

// trial and error of slider calibration in the following:
//
// in this mode, we make linearcalibration over full range
//  int deltaDAC=APFEL_DAC_MAXVALUE-valDAC_min;
//       int deltaADC=valADC_min - APFEL_ADC_MAXSATURATION;
//       theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, APFEL_DAC_MAXVALUE, valADC_min);

// linear calibration only in non saturated ADC range:
//      int deltaDAC=valDAC_max-valDAC_min;
//      int deltaADC=valADC_min - APFEL_ADC_MAXSATURATION;
//      theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC_max, valADC_sample);

// test: semi-range linear calibration
//     int deltaDAC=APFEL_DAC_MAXVALUE-valDAC;
//      int deltaADC=valADC_min - valADC;
//      theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC, valADC);
///////////
//      int deltaDAC=valDAC-valDAC_min;
//      int deltaADC=APFEL_ADC_MAXSATURATION - valADC;
//      theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC, valADC);

#endif

  // finally, refresh display of currently calibrated adc channel:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  valADC = AcquireBaselineSample (channel);
  fADCLineEdit[channel]->setText (pre + text.setNum (valADC, fNumberBase));
  RefreshStatus ();
  return 0;
}


void ApfelGui::CalibrateResetSelectedADCs ()
{
  if (!AssertChainConfigured ())
    return;
  for (int channel = 0; channel < 16; ++channel)
  {
    if (fBaselineBoxes[channel]->isChecked ())
    {
      CalibrateResetADC (channel);
    }
  }
}

int ApfelGui::CalibrateResetADC (int channel)
{
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  // TODO: first autocalibrate DAC that belongs to selected channel
  int apfel = 0, dac = 0;
  theSetup->EvaluateDACIndices (channel, apfel, dac);
  int gain = theSetup->GetGain (apfel, dac);
  theSetup->ResetCalibration (gain, channel);
  printm ("--- Reset Calibration of DAC->ADC slider for sfp:%d board:%d channel:%d apfel:%d dac:%d", fSFP, fSlave,
      channel, apfel, dac);
  return 0;

}

int ApfelGui::AcquireSample (int channel)
{

  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  bool usemonitorport = fApfelWidget->MonitorRadioButton->isChecked ();
  double val = 0;
  theSetup->ResetADCSample (channel);

  if (usemonitorport)
  {
    printm ("AcquiringSample of ADC channel %d from monitoring port.", channel);
    for (int i = 0; i < APFEL_ADC_SAMPLEVALUES; ++i)
    {
      // evaluate  a single sample from ADC monitor port (no averaging like in baseline setup!)
      val = AcquireBaselineSample (channel, 1);
      theSetup->AddADCSample (channel,val);
    }
  }
  else
  {
    // read out MBS buffer
    printm ("AcquiringSample of ADC channel %d from MBS buffer.", channel);
    //first read complete DAQ buffer of channel into memory // single operation
    AcquireMbsSample (channel);

    // then skip optional header words
    int cursor = 0;
    while ((fData[cursor] & 0xAA00) == 0xAA00)
    {
      //std::cout << "skip header word #"<< cursor<<"of mbs buffer:"<< std::hex << fData[cursor]<< std::dec<< std::endl;
      if ((cursor += 2) >= APFEL_MBS_TRACELEN)
        break;
    }
       int i = 0;
       for (i = 0; i < APFEL_MBS_TRACELEN; ++i)
       {
         double value = (fData[cursor] & 0x3FFF);
         cursor++;
         //std::cout <<"got value"<<value<< "at position "<< i <<", cursor="<<cursor<<", sum="<<sum << std::endl;
         theSetup->AddADCSample (channel, value);
       }
    //std::cout << "Filled "<<i<< "adc samples from mbs trace up to position #"<< cursor<< std::endl;
  }
  EvaluateBaseline(channel);
  FindPeaks(channel);
  RefreshLastADCSample (channel);
  return 0;
}


void ApfelGui::EvaluateBaseline(int channel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  bool ok=false;
  int startbase=fApfelWidget->BaselineLowerLineEdit->text().toInt(&ok,fNumberBase);
  int stopbase=fApfelWidget->BaselineUpperLineEdit->text().toInt(&ok,fNumberBase);

 //std::cout<< "EvaluateBaseline for channel "<<channel<<" has start:"<<startbase<<", stop="<<stopbase << std::endl;

  theSetup->EvaluateBaseline(channel,startbase, stopbase);
}


void ApfelGui::FindPeaks(int channel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  double deltaratio= fApfelWidget->PeakDeltaDoubleSpinBox->value()/100.0;
  double fall=fApfelWidget->PeakFallDoubleSpinBox->value();
  bool negative=fApfelWidget->PeakNegaitveCheckBox->isChecked();
  theSetup->EvaluatePeaks(channel,deltaratio,fall,negative);


}

void ApfelGui::SetPeakfinderPolarityNegative(bool on)
{
  fApfelWidget->PeakNegaitveCheckBox->setChecked(on);

}




void ApfelGui::AcquireSelectedSamples ()
{
  if (!AssertChainConfigured ())
    return;
  bool changed = false;
  for (int channel = 0; channel < 16; ++channel)
  {
    if (fSamplingBoxes[channel]->isChecked ())
    {
      changed = true;
      AcquireSample (channel);
    }
  }
  if (changed)
    RefreshStatus ();
}



void ApfelGui::ZoomSampleToPeak(int channel, int peak)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  double nmax=theSetup->GetSamplePeakHeight(channel,peak);
  double pos=theSetup->GetSamplePeakPosition(channel,peak);
  double window=theSetup->GetSamplePeaksPositionDelta(channel);
  double headroom=theSetup->GetSamplePeaksHeightDelta(channel);
  //bool negative=theSetup->IsSamplePeaksNegative(channel);
  double xmin=pos-window;
  double xmax=pos+window;
  double ymin=nmax-headroom*0.8;
  double ymax=nmax+headroom*0.8;
  fPlotWidget[channel]->setLimits (xmin, xmax, ymin, ymax);
  fPlotWidget[channel]->update ();

}


void ApfelGui::RefreshSampleMaxima (int febexchannel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  int numpeaks = theSetup->NumSamplePeaks (febexchannel);
  for (int i = 0; i < APFEL_ADC_NUMMAXIMA; ++i)
  {
    uint16_t height = 0;
    int pos = 0;
    if (i < numpeaks)
    {
      height = theSetup->GetSamplePeakHeight (febexchannel, i);
      pos = theSetup->GetSamplePeakPosition (febexchannel, i);
    }
    QTableWidgetItem * pitem = fApfelWidget->MaximaTableWidget->item (i, 0);
    QTableWidgetItem * hitem = fApfelWidget->MaximaTableWidget->item (i, 1);
    if(pitem) pitem->setText (pre + text.setNum (pos, fNumberBase));
    if(hitem )hitem->setText (pre + text.setNum (height, fNumberBase));
  }

}





int ApfelGui::ShowSample (int channel, bool benchmarkdisplay)
{
  //std::cout <<"ShowSample for channel:"<<channel<< std::endl;
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  //theSetup->ShowADCSample(channel); // todo: dump sample on different knob

  KPlotWidget* canvas = fPlotWidget[channel];
  if (benchmarkdisplay)
    canvas = fApfelWidget->BenchmarkPlotwidget;
  // first fill plotobject with samplepoints
  QColor col;
  KPlotObject::PointStyle pstyle = KPlotObject::Circle;
  switch (channel)
  {
    case 0:
    case 8:
    default:
      col = Qt::red;
      pstyle = KPlotObject::Circle;
      break;
    case 1:
    case 9:
      col = Qt::green;
      pstyle = KPlotObject::Letter;
      break;
    case 2:
    case 10:
      col = Qt::blue;
      pstyle = KPlotObject::Triangle;
      break;
    case 3:
    case 11:
      col = Qt::cyan;
      pstyle = KPlotObject::Square;
      break;

    case 4:
    case 12:
      col = Qt::magenta;
      pstyle = KPlotObject::Pentagon;
      break;
    case 5:
    case 13:
      col = Qt::yellow;
      pstyle = KPlotObject::Hexagon;
      break;
    case 6:
    case 14:
      col = Qt::gray;
      pstyle = KPlotObject::Asterisk;
      break;
    case 7:
    case 15:
      col = Qt::darkGreen;
      pstyle = KPlotObject::Star;
      break;

//        Letter = 2, Triangle = 3,
//         Square = 4, Pentagon = 5, Hexagon = 6, Asterisk = 7,
//         Star = 8

  };

  // TODO: put this in special functions
  canvas->resetPlot ();
  // labels for plot area:
  canvas->setAntialiasing (true);
  canvas->axis (KPlotWidget::BottomAxis)->setLabel ("Time (#samples)");
  canvas->axis (KPlotWidget::LeftAxis)->setLabel ("ADC value ");

  //KPlotObject *sampleplot = new KPlotObject(col, KPlotObject::Points, 1, pstyle);
  KPlotObject *sampleplot = new KPlotObject (col, KPlotObject::Lines, 2);
  QString label = QString ("channel:%1").arg (channel);
  sampleplot->addPoint (0, theSetup->GetADCSample (channel, 0), label);



  int samplength=theSetup->GetADCSampleLength(channel);
  for (int i = 1; i < samplength; ++i)
  {
    sampleplot->addPoint (i, theSetup->GetADCSample (channel, i));
  }

  // add it to the plot area
  canvas->addPlotObject (sampleplot);


   if (benchmarkdisplay)
  {
    canvas->setLimits (0, samplength, 0.0, 17000);
    canvas->update ();
  }
  else
  {
    UnzoomSample (channel);
    RefreshSampleMaxima(channel);
  }

  return 0;
}



void ApfelGui::ShowSelectedSamples ()
{
  if (!AssertChainConfigured ())
    return;
  int lastchecked = 0;
  for (int channel = 0; channel < 16; ++channel)
  {
    if (fSamplingBoxes[channel]->isChecked ())
    {
      ShowSample (channel);
      lastchecked = channel;
    }
  }
  fApfelWidget->ApfelTabWidget->setCurrentIndex (5);
  fApfelWidget->PlotTabWidget->setCurrentIndex (lastchecked);
}

/** Clear display of benchmark DAC curve*/
void ApfelGui::ResetBenchmarkCurve ()
{
  fPlotMinDac = 0;
  fPlotMaxDac = APFEL_DAC_MAXVALUE;
  fPlotMinAdc = 0;
  fPlotMaxAdc = APFEL_ADC_MAXVALUE;

  KPlotWidget* canvas = fApfelWidget->BenchmarkPlotwidget;
  canvas->resetPlot ();
  // labels for plot area:
  canvas->setAntialiasing (true);
  canvas->axis (KPlotWidget::BottomAxis)->setLabel ("DAC value");
  canvas->axis (KPlotWidget::LeftAxis)->setLabel ("ADC value ");
  canvas->setLimits (0, APFEL_DAC_MAXVALUE, 0, APFEL_ADC_MAXVALUE);
  canvas->update ();

}




void ApfelGui::ShowLimitsCurve (int gain, int apfel, int dac)
{
  QColor col = Qt::red;
  KPlotObject *upper = new KPlotObject (col, KPlotObject::Lines, 3);
  KPlotObject *lower = new KPlotObject (col, KPlotObject::Lines, 3);
  //std::cout<<"ShowLimitsCurve: gain:"<<gain<<", apfel:"<<apfel<<", dac:"<<dac << std::endl;

  theSetup_GET_FOR_SLAVE(BoardSetup);
  ApfelTestResults& theResults = theSetup->AccessTestResults (gain, apfel);
  ApfelTestResults& reference = fBenchmark.GetReferenceValues (gain);

  double tolerance = fApfelWidget->ToleranceSpinBox->value () / 100.0;
  bool relativeMode=fApfelWidget->RelativeComparisonBox->isChecked();
  int ashift=0, dshift=0;
  if(relativeMode)
  {
      // get coordinates of sample and shift reference onto autocalibration centre:
      int autoix = (APFEL_DAC_CURVEPOINTS/2 -1); // should be 11 from 24
      DacSample middleSample (0, 0);
      theResults.AccessDacSample (middleSample, dac, autoix);
      DacSample middleReference (0, 0);
      reference.AccessDacSample (middleReference, dac, autoix);
      ashift= (int) middleSample.GetADCValue () - (int) middleReference.GetADCValue ();
      dshift= (int) middleSample.GetDACValue() - (int) middleReference.GetDACValue();
      //std::cout<<"ShowLimitsCurve for apfel:"<<apfel<<", dac:"<<dac<<" has adc shift:"<<(int) ashift<<", dac shift:"<< (int) dshift << std::endl;
  }

  for (int i = 0; i < APFEL_DAC_CURVEPOINTS; ++i)
  {
    DacSample point (0, 0);
    reference.AccessDacSample (point, dac, i);    // if this fails, point is just not touched -> default 0 values are saved
    int dval=point.GetDACValue() + dshift;
    int aval = point.GetADCValue () + ashift;

    double adcup = aval * (1.0 + tolerance);
    double adclow = aval * (1.0 - tolerance);
    upper->addPoint (dval, adcup);
    lower->addPoint (dval, adclow);

    // evaluate zoom range
    if (fPlotMinDac == 0 || dval < fPlotMinDac)
      fPlotMinDac = dval;
    if (fPlotMaxDac == APFEL_DAC_MAXVALUE || dval > fPlotMaxDac)
      fPlotMaxDac = dval;
    if (fPlotMinAdc == 0 || adclow < fPlotMinAdc)
      fPlotMinAdc = adclow;
    if (fPlotMaxAdc == APFEL_DAC_MAXVALUE || adcup > fPlotMaxAdc)
      fPlotMaxAdc = adcup;

    //std::cout<<"ShowLimitsCurve: i:"<<i<<", dacval:"<<dval<<"up:"<<adcup<<", lo:"<<adclow << std::endl;

  }
  fApfelWidget->BenchmarkPlotwidget->addPlotObject (upper);
  fApfelWidget->BenchmarkPlotwidget->addPlotObject (lower);
  fApfelWidget->BenchmarkPlotwidget->setLimits (fPlotMinDac, fPlotMaxDac, fPlotMinAdc, fPlotMaxAdc);
  fApfelWidget->BenchmarkPlotwidget->update ();
}

void ApfelGui::ShowBenchmarkCurve (int gain, int apfel, int dac)
{
   theSetup_GET_FOR_SLAVE(BoardSetup);

  QColor col;
  KPlotObject::PointStyle pstyle = KPlotObject::Circle;
  switch (apfel)
  {
    case 0:
    case 8:
    default:
      col = Qt::red;
      break;
    case 1:
    case 9:
      col = Qt::green;
      break;
    case 2:
    case 10:
      col = Qt::blue;
      break;
    case 3:
    case 11:
      col = Qt::cyan;
      break;

    case 4:
    case 12:
      col = Qt::magenta;
      break;
    case 5:
    case 13:
      col = Qt::yellow;
      break;
    case 6:
    case 14:
      col = Qt::gray;
      break;
    case 7:
    case 15:
      col = Qt::darkGreen;
      break;

//        Letter = 2, Triangle = 3,
//         Square = 4, Pentagon = 5, Hexagon = 6, Asterisk = 7,
//         Star = 8

  };

  KPlotObject *curveplot = new KPlotObject (col, KPlotObject::Points, 3, pstyle);
  ApfelTestResults& theResults = theSetup->AccessTestResults (gain, apfel);

  QString label = QString ("gain:%1 apfel:%2 dac:%3").arg (gain).arg (apfel).arg (dac);
  for (int i = 0; i < APFEL_DAC_CURVEPOINTS; ++i)
  {
    DacSample point (0, 0);
    theResults.AccessDacSample (point, dac, i);    // if this fails, point is just not touched -> default 0 values are saved
    uint16_t dval = point.GetDACValue ();
    uint16_t aval = point.GetADCValue ();
    // this is for zooming:
    if (fPlotMinDac == 0 || dval < fPlotMinDac)
      fPlotMinDac = dval;
    if (fPlotMaxDac == APFEL_DAC_MAXVALUE || dval > fPlotMaxDac)
      fPlotMaxDac = dval;
    if (fPlotMinAdc == 0 || aval < fPlotMinAdc)
      fPlotMinAdc = aval;
    if (fPlotMaxAdc == APFEL_DAC_MAXVALUE || aval > fPlotMaxAdc)
      fPlotMaxAdc = aval;

    if (i == APFEL_DAC_CURVEPOINTS / 2)
      curveplot->addPoint (dval, aval, label);
    else
      curveplot->addPoint (dval, aval);

    //std::cout<<"ShowBenchmarkCurve: i:"<<i<<", dacval="<< (int) dval<<", adcval="<< (int) aval << std::endl;
  }

  // add it to the plot area
  fApfelWidget->BenchmarkPlotwidget->addPlotObject (curveplot);

  //if(gain!=1)
  fApfelWidget->BenchmarkPlotwidget->setLimits (fPlotMinDac, fPlotMaxDac, fPlotMinAdc, fPlotMaxAdc);
  //else
  //  BenchmarkPlotwidget->setLimits (0, APFEL_DAC_MAXVALUE, 0, APFEL_ADC_MAXVALUE);
  //std::cout<<"ShowBenchmarkCurve limits: dmin:"<<fPlotMinDac<<", dmax:"<<fPlotMaxDac<<", amin:"<< fPlotMinAdc<<", amax:"<<fPlotMaxAdc<< std::endl;

  fApfelWidget->BenchmarkPlotwidget->update ();

}


void ApfelGui::ZoomSample (int channel)
{
  //std::cout <<"ZoomSample for channel"<< channel<< std::endl;
  // evaluate minimum and maximum value of current sample:
  theSetup_GET_FOR_SLAVE(BoardSetup);
  double minimum = theSetup->GetADCMiminum (channel);
  double maximum = theSetup->GetADCMaximum (channel);
  double xmax=theSetup->GetADCSampleLength(channel);
  fPlotWidget[channel]->setLimits (0, xmax, minimum, maximum);
  fPlotWidget[channel]->update ();
}

void ApfelGui::UnzoomSample (int channel)
{
  //std::cout <<"UnzoomSample for channel"<< channel<< std::endl;
  theSetup_GET_FOR_SLAVE(BoardSetup);
  double xmax=theSetup->GetADCSampleLength(channel);
  fPlotWidget[channel]->setLimits (0, xmax, 0.0, 17000);
  fPlotWidget[channel]->update ();
}


void ApfelGui::RefreshLastADCSample (int febexchannel)
{
  QString text;
  theSetup_GET_FOR_SLAVE(BoardSetup);
  double mean = theSetup->GetADCMean (febexchannel);
  double sigma = theSetup->GetADCSigma (febexchannel);
  if (fNumberBase == 16)
  {
    QString pre = "0x";
    fSamplingMeanLineEdit[febexchannel]->setText (pre + text.setNum ((int) mean, fNumberBase));
    fSamplingSigmaLineEdit[febexchannel]->setText (pre + text.setNum ((int) sigma, fNumberBase));
  }
  else
  {

    fSamplingMeanLineEdit[febexchannel]->setText (text.setNum (mean, 'f', 1));
    fSamplingSigmaLineEdit[febexchannel]->setText (text.setNum (sigma, 'f', 3));
  }

}


void ApfelGui::DumpSlave ()
{
  DumpADCs();
  DumpCalibrations ();    // later put to separate button
}


void ApfelGui::DumpADCs ()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if (!AssertChainConfigured ())
    return;

  printm ("SFP %d DEV:%d :)", fSFP, fSlave);
  for (int adc = 0; adc < APFEL_ADC_NUMADC; ++adc)
  {
    for (int chan = 0; chan < APFEL_ADC_NUMCHAN; ++chan)
    {
      int val = ReadADC_Apfel (adc, chan);
      if (val < 0)
        printm ("Read error for adc:%d chan:%d", adc, chan);
      else
      {
        if (fNumberBase == 16)
          printm ("Val (adc:0x%x chan:0x%x)=0x%x", adc, chan, val);
        else
          printm ("Val (adc:%d chan:%d)=%d", adc, chan, val);
      }
    }
  }
}

void ApfelGui::DumpCalibrations ()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if (!AssertChainConfigured ())
    return;
  theSetup_GET_FOR_SLAVE(BoardSetup);
  printm ("SFP %d DEV:%d : Dump calibration)", fSFP, fSlave);
  int apfel = 0, dac = 0;
  for (int febchan = 0; febchan < APFEL_ADC_CHANNELS; ++febchan)
  {
    theSetup->EvaluateDACIndices (febchan, apfel, dac);
    int gain = theSetup->GetGain (apfel, dac);
    theSetup->DumpCalibration (gain, febchan);
  }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////7
//////////////////////////////////////////////////////////////////////////////////////////////////////

void ApfelGui::AutoApplySwitch ()
{
  EvaluateIOSwitch ();
  SetIOSwitch ();
}


void ApfelGui::AutoApplyPulser (int apfel)
{
  EvaluatePulser (apfel);
  SetPulser (apfel);
}


void ApfelGui::AutoApplyGain (int apfel, int channel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  EvaluateGain (apfel, channel);
  //std::cout << "AutoApplyGain apfel=" << apfel << ", channel=" << channel << ", lowgain:"
  //    << theSetup->GetLowGain (apfel, channel) << std::endl;
  SetGain (apfel, channel, theSetup->GetLowGain (apfel, channel));
}


void ApfelGui::AutoApplyDAC (int apfel, int dac, int val)
{
  // keep setup structure always consistent:
  theSetup_GET_FOR_SLAVE(BoardSetup);
  theSetup->SetDACValue (apfel, dac, val);
  WriteDAC_ApfelI2c (apfel, dac, theSetup->GetDACValue (apfel, dac));
  RefreshADC_Apfel (apfel, dac);
}


void ApfelGui::AutoApplyRefresh (int channel, int dac)
{
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  int Adc = autoApply (channel, dac);
  fADCLineEdit[channel]->setText (pre + text.setNum (Adc, fNumberBase));
  RefreshStatus ();
}

int ApfelGui::autoApply (int channel, int permillevalue)

{
  int apfel = 0, dac = 0;
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  theSetup->EvaluateDACIndices (channel, apfel, dac);
  int gain = theSetup->GetGain (apfel, dac);
  //int value=theSetup->EvaluateDACvalueAbsolute(permillevalue,-1,gain);
  int value = theSetup->EvaluateDACvalueAbsolute (permillevalue, channel, gain);

  theSetup->SetDACValue (apfel, dac, value);

  EnableI2C ();
  WriteDAC_ApfelI2c (apfel, dac, theSetup->GetDACValue (apfel, dac));
  DisableI2C ();

  RefreshDAC (apfel);    //  immediately update DAC sliders when shifting baseline!
  if (!AssertNoBroadcast ())
    return -1;
  int Adc = AcquireBaselineSample (channel);
  //std::cout << "ApfelGui::autoApply channel="<<channel<<", permille="<<permillevalue<<", apfel="<<apfel<<", dac="<<dac<<", DACvalue="<<value<<", ADC="<<Adc << std::endl;

  return Adc;

}

int ApfelGui::AcquireBaselineSample (uint8_t febexchan, int numsamples)
{
  if (febexchan >= APFEL_ADC_NUMADC * APFEL_ADC_NUMCHAN)
    return -1;
  int adcchip = febexchan / APFEL_ADC_NUMCHAN;
  int adcchannel = febexchan - adcchip * APFEL_ADC_NUMCHAN;
  int Adc = 0;
  if (numsamples <= 0)
    numsamples = APFEL_ADC_BASELINESAMPLES;
  for (int t = 0; t < numsamples; ++t)
  {
    Adc += ReadADC_Apfel (adcchip, adcchannel);
  }
  Adc = Adc / numsamples;
  return Adc;
}

int ApfelGui::AcquireMbsSample (uint8_t febexchan)
{
  if (febexchan >= APFEL_ADC_NUMADC * APFEL_ADC_NUMCHAN)
    return -1;
  // issue read request:
  EnableI2C ();
  WriteGosip (fSFP, fSlave, APFEL_ADC_DAQBUFFER_REQ_PORT, 0x80);
  int readaddress = APFEL_ADC_DAQBUFFER_BASE * (febexchan + 1);
  for (int cursor = 0; cursor < APFEL_MBS_TRACELEN; cursor += 2)
  {
    int value = ReadGosip (fSFP, fSlave, readaddress);
    fData[cursor] = (value >> 16) & 0xFFFF;
    fData[cursor + 1] = (value & 0xFFFF);    // check the order here?
//          if(cursor<10 || APFEL_MBS_TRACELEN -cursor < 10)
//            printf("AcquireMbsSample val=0x%x dat[%d]=0x%x dat[%d]=0x%x\n",value,cursor,fData[cursor],cursor+1, fData[cursor+1]);
    readaddress += 4;
  }
  DisableI2C ();
  return 0;
}

void ApfelGui::RefreshDAC (int apfel)
{
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
   theSetup_GET_FOR_SLAVE(BoardSetup);
  for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
  {
    int value = theSetup->GetDACValue (apfel, dac);
    fDACSlider[apfel][dac]->setValue (value);
    fDACLineEdit[apfel][dac]->setText (pre + text.setNum (value, fNumberBase));
  }
}

void ApfelGui::RefreshADC_channel (int channel, int gain)
{
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
   theSetup_GET_FOR_SLAVE(BoardSetup);
  int val = theSetup->GetDACValue (channel);
  int permille = theSetup->EvaluateADCvaluePermille (val, channel, gain);
  //std::cout << "RefreshADC_channel(" << (int) channel <<","<<gain<<") - val="<<val<<" permille=" << permille<< std::endl;
  fDACSpinBoxes[channel]->setValue (permille);
  int adc = AcquireBaselineSample (channel);
  fADCLineEdit[channel]->setText (pre + text.setNum (adc, fNumberBase));
}

void ApfelGui::RefreshADC_Apfel (int apfelchip, int dac)
{
   theSetup_GET_FOR_SLAVE(BoardSetup);
  int chan = theSetup->EvaluateADCChannel (apfelchip, dac);
  //std::cout << "RefreshADC(" << (int) apfelchip <<"):  dac:"<<dac<<", chan=" << chan<< std::endl;
  if (chan >= 0)
  {
    // only refresh adc channels once for active dacs
    int gain = theSetup->GetGain (apfelchip, dac);
    RefreshADC_channel (chan, gain);
    if (!theSetup->IsHighGain ())
      RefreshADC_channel (chan + 1, gain);    // kludge to cover both adc channels set by dac2 for gain 1
  }

}

void ApfelGui::RefreshView ()
{
// display setup structure to gui:
//  QString text;
//  QString pre;
//  fNumberBase == 16 ? pre = "0x" : pre = "";
  theSetup_GET_FOR_SLAVE(BoardSetup);

//////////////////////////////////////////////////////
// first io configuration and gain:
  fApfelWidget->ApfelRadioButton->setChecked (theSetup->IsApfelInUse ());
  fApfelWidget->PolandRadioButton->setChecked (!theSetup->IsApfelInUse ());    // probably we do not need this because of autoExclusive flag
  fApfelWidget->LoGainRadioButton->setChecked (!theSetup->IsHighGain ());
  fApfelWidget->HiGainRadioButton->setChecked (theSetup->IsHighGain ());    // probably we do not need this because of autoExclusive flag
  fApfelWidget->StretcherOnRadioButton->setChecked (theSetup->IsStretcherInUse ());
  fApfelWidget->StretcherOffRadioButton->setChecked (!theSetup->IsStretcherInUse ());

  fApfelWidget->InverseMappingCheckBox->setChecked (!theSetup->IsRegularMapping ());

  if (theSetup->IsHighGain ())
  {
    // only refresh gain entries if we are in high gain mode
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
      {
        bool logain = theSetup->GetLowGain (apfel, chan);
        if (logain)
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
    RefreshDAC (apfel);
  }

///////////////////////////////////////////////////////
//show pulser setup:
  for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
  {
    bool positive = theSetup->GetTestPulsePositive (apfel);
    if (positive)
      fApfelPulsePolarityCombo[apfel]->setCurrentIndex (0);
    else
      fApfelPulsePolarityCombo[apfel]->setCurrentIndex (1);
    for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
    {
      bool on = theSetup->GetTestPulseEnable (apfel, chan);
      fApfelPulseEnabledCheckbox[apfel][chan]->setChecked (on);
    }
  }

  RefreshBaselines();

//////////////////////////////////////////////////////////
// dac relative baseline settings and adc sample:
  int apfel = 0, dac = 0;
  for (int channel = 0; channel < 16; ++channel)
  {
    theSetup->EvaluateDACIndices (channel, apfel, dac);
    int gain = theSetup->GetGain (apfel, dac);
    RefreshADC_channel (channel, gain);

    // also put most recent sample parameters to display:
    RefreshLastADCSample (channel);
  }

  RefreshChains ();
  RefreshStatus ();
}





void ApfelGui::EvaluatePulser (int apfel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  bool positive = (fApfelPulsePolarityCombo[apfel]->currentIndex () == 0);
  theSetup->SetTestPulsePostive (apfel, positive);
  for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
  {
    bool on = fApfelPulseEnabledCheckbox[apfel][chan]->isChecked ();
    theSetup->SetTestPulseEnable (apfel, chan, on);
    int amplitude = fApfelPulseAmplitudeSpin[apfel][chan]->value ();
    theSetup->SetTestPulseAmplitude (apfel, chan, amplitude);
  }
}

void ApfelGui::EvaluateGain (int apfel, int channel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  bool logain = (fApfelGainCombo[apfel][channel]->currentIndex () == 0);
  theSetup->SetLowGain (apfel, channel, logain);
}

void ApfelGui::EvaluateIOSwitch ()
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  // get io config from gui
  theSetup->SetApfelInUse (fApfelWidget->ApfelRadioButton->isChecked ());
  theSetup->SetHighGain (fApfelWidget->HiGainRadioButton->isChecked ());
  theSetup->SetStretcherInUse (fApfelWidget->StretcherOnRadioButton->isChecked ());

}

void ApfelGui::EvaluateView ()
{
  // here the current gui display is just copied to setup structure in local memory
  theSetup_GET_FOR_SLAVE(BoardSetup);
//std::cout<<"ApfelGui::EvaluateView ()" << std::endl;

  theSetup->SetApfelMapping (!fApfelWidget->InverseMappingCheckBox->isChecked ());

  EvaluateIOSwitch ();

  if (theSetup->IsHighGain ())
  {
    // only apply gain entries if we are in high gain mode
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      for (int chan = 0; chan < APFEL_NUMCHANS; ++chan)
      {
        EvaluateGain (apfel, chan);
      }
    }
  }
// here baseline sliders for dacs
// todo: prevent different settings from DAC and ADC tabs; check which tab is active?
  if (fApfelWidget->ApfelTabWidget->currentIndex () == 3)
  {
    // only apply the adc sliders when visible
    for (int channel = 0; channel < 16; ++channel)
    {
      int permille = fDACSpinBoxes[channel]->value ();
      int value = theSetup->EvaluateDACvalueAbsolute (permille);
      //std::cout<<"EvaluateView for channel:"<<channel<<", permille:"<<permille<<" - val="<<value<< std::endl;
      theSetup->SetDACValue (channel, value);
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
        //std::cout<<"EvaluateView for apfel:"<<apfel<<", dac:"<<dac<<" - val="<<value<< std::endl;
        theSetup->SetDACValue (apfel, dac, value);
      }

    }
  }    //if(ApfelTabWidget->currentIndex()==3)

// pulser config from gui
  for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
  {
    EvaluatePulser (apfel);
  }
}






uint8_t ApfelGui::GetApfelId (int sfp, int slave, uint8_t apfelchip)
{
  if (sfp < 0 || sfp >= PEX_SFP_NUMBER)
    return 0xFF;
  if (slave < 0 || slave >= fSFPChains.numslaves[sfp])
    return 0xFF;
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  return theSetup->GetApfelID (apfelchip);
}


int ApfelGui::ScanDACCurve (int gain, int channel)
{
  QApplication::setOverrideCursor (Qt::WaitCursor);
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  int apfel = 0, dac = 0;
  theSetup->EvaluateDACIndices (channel, apfel, dac);
  ApfelTestResults& theResults = theSetup->AccessTestResults (gain, apfel);
  theResults.ResetDacSample (dac);
  int points = APFEL_DAC_CURVEPOINTS;
  // depending on gain, we have different stepsizes
  int step = 0;
  switch (gain)
  {
    case 1:
      step = 16;
      dac = 2;
      break;
    case 16:
      step = 2;
      break;
    case 32:
      step = 1;
      break;
  }

  // we start in the middle of the autocalibration point:
  uint16_t dac_mid = theResults.GetDacValueCalibrate (dac);
  if (dac_mid == 0)
  {
    // no calibration done yet, do it now
    DoAutoCalibrate (apfel);
    dac_mid = theSetup->GetDACValue (apfel, dac);
  }
  //std::cout<<"ScanDACCurve for gain:"<<gain<<", step:"<<step<<", channel:"<<channel<<" - DAC middle point is "<<dac_mid << std::endl;
  EnableI2C ();
  uint16_t d0 = dac_mid - step * points / 2;
  for (int p = 0; p < points; ++p)
  {
    uint16_t dacval = d0 + p * step;
    theSetup->SetDACValue (apfel, dac, dacval);
    WriteDAC_ApfelI2c (apfel, dac, theSetup->GetDACValue (apfel, dac));
    int adcval = AcquireBaselineSample (channel);
   // std::cout<<"   ScanDACCurve got d:"<<dacval<<", adc:"<<adcval << std::endl;
    theResults.AddDacSample (dac, dacval, adcval);
  }
  DisableI2C ();
  ResetBenchmarkCurve ();
  ShowLimitsCurve (gain, apfel, dac);
  ShowBenchmarkCurve (gain, apfel, dac);

  QApplication::restoreOverrideCursor ();
}

void ApfelGui::UpdateAfterAutoCalibrate (uint8_t apfelchip)
{
  // here get registers of apfelchip only and refresh
  EnableI2C ();
  GetDACs (apfelchip);
  DisableI2C ();
  RefreshDAC (apfelchip);
  for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
  {
    RefreshADC_Apfel (apfelchip, dac);
  }
}


void ApfelGui::SetInverseMapping (int on)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  theSetup->SetApfelMapping (!on);

}


int ApfelGui::EvaluatePulserInterval (int findex)
{
  int period = 1000;
  switch (findex)
  {
    case 0:
    default:
      period = 1000;
      break;
    case 1:
      period = 500;
      break;
    case 2:
      period = 200;
      break;
    case 3:
      period = 100;
      break;
    case 4:
      period = 20;
      break;
  };
  //std::cout << "EvaluatePulserInterval gives ms period:" <<  period << std::endl;
  return period;
}



void ApfelGui::RefreshBaselines()
{
  //std::cout << "RefreshBaselines" <<  std::endl;
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";

  double lowpermil=fApfelWidget->BaselineLowerSlider->value();
  double uppermil=fApfelWidget->BaselineUpperSlider->value();
  double maxrange=APFEL_ADC_SAMPLEVALUES;
  if(fApfelWidget->ReadoutRadioButton->isChecked())
    maxrange=APFEL_MBS_TRACELEN;
  int lowvalue= lowpermil*0.001*maxrange;
  int upvalue= uppermil*0.001*maxrange;
  fApfelWidget->BaselineLowerLineEdit->setText(pre + text.setNum (lowvalue, fNumberBase));
  fApfelWidget->BaselineUpperLineEdit->setText(pre + text.setNum (upvalue, fNumberBase));

}




void ApfelGui::LoadBenchmarkReferences()
{
  QFileDialog fd (this, "Select Benchmark reference data file", ".", "apfel characterization file (*.apf)");
  fd.setFileMode (QFileDialog::ExistingFile);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  QString filename = flst[0];
  fBenchmark.LoadReferenceValues(filename);
  fApfelWidget->ReferenceLineEdit->setText(filename);
}




void ApfelGui::SaveTestResults ()
{

  printm ("Saving test results of sfp:%d slave%d.", fSFP, fSlave);
  theSetup_GET_FOR_SLAVE(BoardSetup);

// do not refer to current setup entries here, but to saved results.
//  QString apf1 = theSetup->GetBoardID (0);
//  QString apf2 = theSetup->GetBoardID (1);
//  if (apf1.isEmpty () || apf2.isEmpty ())
//  {
//    printm ("Can not save test results: full id information was not given! Please rerun test.");
//    return;
//  }
//  double current = theSetup->GetCurrent ();
//  double voltage = theSetup->GetVoltage();
//  QString header = QString ("# apfel1:").append (apf1).append (", apfel2:").append (apf2);
//  header.append (QString (" Current:%1 A Voltage:%1 A").arg (current).arg (voltage));
//  header.append ("\n");
//  WriteTestFile (header);
// information of this header is now part of table

  // header
  QString tstamp=QDateTime::currentDateTime().toString(APFEL_RESULT_TIMEFORMAT);
  WriteTestFile(QString("# This is an APFEL Test result file saved with ApfelGui on "));
  WriteTestFile(tstamp);
  WriteTestFile(QString("\n"));
  WriteTestFile(QString("#   developed for FAIR/PASEM project 2016 by JAM (j.adamczewski@gsi.de), GSI Experiment Electronics department \n"));
  WriteTestFile(QString("#\n"));

  // format
  WriteTestFile (
      QString ("# BoardID \tGain \tAPFEL \tDAC \tCalibSet \tBaseline \tSigma  \tBaseLow \tBaseUp \tdDAC/dADC \tDAC0 \tDACmin \tDACmax \tADCmin"));
  for (int i = 0; i < APFEL_DAC_CURVEPOINTS; ++i)
  {
    WriteTestFile (QString ("\tDAC_%1 \tADC_%2").arg (i).arg (i));
  }

  WriteTestFile (QString ("\tPeakPolarity"));
  for (int i = 0; i < APFEL_ADC_NUMMAXIMA; ++i)
    {
      WriteTestFile (QString ("\tPeakPos_%1 \tPeakHeight_%2").arg (i).arg (i));
    }

  WriteTestFile (QString ("\tCurrent(A) \tVoltage(V) \tStartDate \t StartTime \tStopDate \tStopTime"));
  WriteTestFile (QString ("\n"));
  // loopp over gain:
  for (int gain = 1; gain < 40; gain += 15)
  {
    if (gain == 31)
      gain = 32;    // :)
    for (int apfel = 0; apfel < APFEL_NUMCHIPS; ++apfel)
    {
      //int apfeladdress = theSetup->GetApfelID (apfel);    // index to address id, depends on setup!
      ApfelTestResults& theResult = theSetup->AccessTestResults (gain, apfel);
      for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
      {
        QString boardid=QString(theResult.GetBoardDescriptor().c_str());
        int apfeladdress=theResult.GetAddressId();
        int dacval = theResult.GetDacValueCalibrate (dac,true); // when saving, we assure that test was really done
        int baseline = theResult.GetAdcSampleMean (dac,true);
        int sigma = theResult.GetAdcSampleSigma (dac,true);
        int startbase=theResult.GetAdcBaselineLowerBound(dac,true);
        int stopbase=theResult.GetAdcBaselineUpperBound(dac,true);
        double slope = theResult.GetSlope (dac,true);
        double dac0 = theResult.GetD0 (dac,true);
        double dacmin = theResult.GetDACmin (dac,true);
        double dacmax = theResult.GetDACmax (dac,true);
        double adcmin = theResult.GetADCmin (dac,true);
        double current = theResult.GetCurrent();
        double voltage = theResult.GetVoltage();

        // here we should supress/mark as invalid the results that are not meaningful for the selected gain:

        if (gain == 1)
        {
          if (dac == 0 || dac == 1 || dac == 3)
          {
            baseline = APFEL_NOVALUE;
            sigma = APFEL_NOVALUE;
            startbase = APFEL_NOVALUE;
            stopbase = APFEL_NOVALUE;
            slope = APFEL_NOVALUE;
            dac0 = APFEL_NOVALUE;
            dacmin = APFEL_NOVALUE;
            dacmax = APFEL_NOVALUE;
            adcmin = APFEL_NOVALUE;
            current = APFEL_NOVALUE;
            voltage = APFEL_NOVALUE;
          }

        }
        else if (gain == 16 || gain == 32)
        {
          if (dac == 2 || dac == 3)
          {
            baseline = APFEL_NOVALUE;
            sigma = APFEL_NOVALUE;
            startbase = APFEL_NOVALUE;
            stopbase = APFEL_NOVALUE;
            slope = APFEL_NOVALUE;
            dac0 = APFEL_NOVALUE;
            dacmin = APFEL_NOVALUE;
            dacmax = APFEL_NOVALUE;
            adcmin = APFEL_NOVALUE;
            current = APFEL_NOVALUE;
            voltage = APFEL_NOVALUE;
          }

        }





        QString line = "\t";
        line.append(boardid);
        line.append(QString ("\t\t%1 \t\t%2 \t\t%3 \t\t%4 \t\t%5 \t\t%6 \t\t%7 \t\t%8 \t\t%9").arg (gain).arg (apfeladdress).arg (dac).arg (dacval).arg (baseline).arg (sigma).arg(startbase).arg(stopbase).arg (slope));
        line.append (QString ("\t\t%1 \t\t%2 \t\t%3 \t\t%4").arg (dac0).arg (dacmin).arg (dacmax).arg (adcmin));

        // add the results of the curve to the line:
        for (int i = 0; i < APFEL_DAC_CURVEPOINTS; ++i)
        {
          int dval=APFEL_NOVALUE, aval=APFEL_NOVALUE;
          if(theResult.IsValid())
          {
            DacSample point (0, 0);
            if(theResult.AccessDacSample (point, dac, i)==0) // if this fails, we keep the APFEL_NOVALUE
            {
              dval = point.GetDACValue ();
              aval = point.GetADCValue ();
            }
          }
          //std::cout<<"saving curve point"<<i<<", dac="<<(int) dval<<", adc="<<(int) aval << std::endl;
          line.append (QString ("\t%1 \t%2").arg (dval).arg (aval));
        }

        // put here location of peak finder
        bool isnegative=theResult.HasNegativeAdcPeaks(dac);
        line.append((isnegative ? QString("\t -1") : QString("\t 1")));
        for (int i = 0; i < APFEL_ADC_NUMMAXIMA; ++i)
                {
                  int pos = theResult.GetAdcPeakPosition(dac,i,true);
                  int max = theResult.GetAdcPeakHeight(dac,i,true);
                  line.append (QString ("\t%1 \t%2").arg (pos).arg (max));
                }

        line.append(QString ("\t%1 \t%2 \t").arg(current).arg(voltage));
        line.append(theResult.GetStartTime());
        line.append("\t");
        line.append(theResult.GetEndTime());
        line.append ("\n");
        WriteTestFile (line);
      }

    }
  }

}


void ApfelGui::SaveConfig()
  {
    DoSaveConfig();
  }


void ApfelGui::DoSaveConfig(const char* selectfile)
{
//std::cout << "ApfelGui::SaveConfigBtn_clicked()"<< std::endl;

  static char buffer[1024];
  QString gos_filter ("gosipcmd file (*.gos)");
  QString apf_filter ("apfel characterization file (*.apf)");
  QStringList filters;
  filters << gos_filter << apf_filter;

  QFileDialog fd (this, "Write Apfel configuration file");

  // ".", "nyxor setup file (*.txt);;gosipcmd file (*.gos);;context dump file (*.dmp)");
  fd.setNameFilters (filters);
  fd.selectNameFilter (apf_filter);
  fd.setFileMode (QFileDialog::AnyFile);
  fd.setAcceptMode (QFileDialog::AcceptSave);

  if (selectfile)
  {
    fd.selectFile (QString (selectfile));
  }
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

    // open file
    if (OpenConfigFile (fileName) != 0)
      return;

    WriteConfigFile (QString ("#Format *.gos"));
    WriteConfigFile (QString ("#usage: gosipcmd -x -c file.gos \n"));
    WriteConfigFile (QString ("#                                         \n"));
    WriteConfigFile (QString ("#sfp slave address value\n"));
    GOSIP_BROADCAST_ACTION(SaveRegisters());
    // refresh actual setup from hardware and write it to open file
    CloseConfigFile ();
    snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
    AppendTextWindow (buffer);
  }
  else if (fd.selectedNameFilter () == apf_filter)
  {
    if (!fileName.endsWith (".apf"))
      fileName.append (".apf");

    OpenTestFile (fileName);
    GOSIP_BROADCAST_ACTION(SaveTestResults());
    // dump figures of merit of current slave, or of all slaves
    CloseTestFile ();
    snprintf (buffer, 1024, "Saved test result to file '%s' .\n", fileName.toLatin1 ().constData ());
    AppendTextWindow (buffer);
  }

  else
  {
    std::cout << "ApfelGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
  }

}




