#include "AwagsGui.h"

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
#include <QMdiSubWindow>

#include <kplotobject.h>
#include <kplotwidget.h>
#include <kplotaxis.h>

#include <sstream>
#include <string.h>
#include <errno.h>
#include <QTableWidget>

// *********************************************************


// enable this define to show original tabbed febex gui as single mdi subwindow
//#define AWAGS_USE_SINGLE_SUBWINDOW 1


/*
 *  Constructs a AwagsGui which is a child of 'parent', with the
 *  name 'name'.'
 */
AwagsGui::AwagsGui (QWidget* parent) :
    GosipGui (parent), fTestFile (0), fPlotMinDac (0),
        fPlotMaxDac (AWAGS_DAC_MAXVALUE), fPlotMinAdc (0), fPlotMaxAdc (AWAGS_ADC_MAXVALUE), fUseSimpleSwitchAddressing(false)
{
  fImplementationName="AWAGS";
  fVersionString="Welcome to AWAGS GUI!\n\t v0.421 of 13-October-2022 by JAM (j.adamczewski@gsi.de)\n";

  fSettings=new QSettings("GSI", fImplementationName);



#ifdef AWAGS_USE_SINGLE_SUBWINDOW
fAwagsWidget=new AwagsWidget(this);
mdiArea->addSubWindow(fAwagsWidget); // complete febex widget in one window

#else

  fAwagsWidget=new AwagsWidget(0);

  QWidget* iotab=fAwagsWidget->AwagsTabWidget->widget(0);
  QWidget* gaintab=fAwagsWidget->AwagsTabWidget->widget(1);
  QWidget* dactab=fAwagsWidget->AwagsTabWidget->widget(2);
  //QWidget* pulsertab=fAwagsWidget->AwagsTabWidget->widget(3);
  QWidget* basetab=fAwagsWidget->AwagsTabWidget->widget(3);
  QWidget* sampletab=fAwagsWidget->AwagsTabWidget->widget(4);
  QWidget* plottab=fAwagsWidget->AwagsTabWidget->widget(5);
  //QWidget* addresstab=fAwagsWidget->AwagsTabWidget->widget(6);
  QWidget* currenttab=fAwagsWidget->AwagsTabWidget->widget(6);
  QWidget* charactertab=fAwagsWidget->AwagsTabWidget->widget(7);


  Qt::WindowFlags wflags= Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint;
  if(iotab)
    {
      fAwagsWidget->AwagsTabWidget->removeTab(0);
      iotab->setWindowTitle("IO Config");
      iotab->show();
      QMdiSubWindow* subio=mdiArea->addSubWindow(iotab,wflags);
      subio->setAttribute(Qt::WA_DeleteOnClose, false);
    }

  if(gaintab)
    {
    fAwagsWidget->AwagsTabWidget->removeTab(0);
    gaintab->setWindowTitle("Gain");
    gaintab->show();
    QMdiSubWindow* subchannel=mdiArea->addSubWindow(gaintab, wflags);
    subchannel->setAttribute(Qt::WA_DeleteOnClose, false);
    }

  if(dactab)
  {
    fAwagsWidget->AwagsTabWidget->removeTab(0);
    dactab->setWindowTitle("DACs");
    dactab->show();
    QMdiSubWindow* subthres=mdiArea->addSubWindow(dactab, wflags);
    subthres->setAttribute(Qt::WA_DeleteOnClose, false);
  }


//  if(pulsertab)
//  {
//    fAwagsWidget->AwagsTabWidget->removeTab(0);
//    pulsertab->setWindowTitle("Pulser");
//    pulsertab->show();
//    QMdiSubWindow* subbase= mdiArea->addSubWindow(pulsertab, wflags);
//    subbase->setAttribute(Qt::WA_DeleteOnClose, false);
//  }

  if(basetab)
   {
     fAwagsWidget->AwagsTabWidget->removeTab(0);
     basetab->setWindowTitle("Baseline");
     basetab->show();
     QMdiSubWindow* subbase= mdiArea->addSubWindow(basetab, wflags);
     subbase->setAttribute(Qt::WA_DeleteOnClose, false);
   }

  if(sampletab)
   {
     fAwagsWidget->AwagsTabWidget->removeTab(0);
     sampletab->setWindowTitle("Sample");
     sampletab->show();
     QMdiSubWindow* subbase= mdiArea->addSubWindow(sampletab, wflags);
     subbase->setAttribute(Qt::WA_DeleteOnClose, false);
   }

  if(plottab)
   {
     fAwagsWidget->AwagsTabWidget->removeTab(0);
     plottab->setWindowTitle("Plot");
     plottab->show();
     QMdiSubWindow* subbase= mdiArea->addSubWindow(plottab, wflags);
     subbase->setAttribute(Qt::WA_DeleteOnClose, false);
   }

//  if(addresstab)
//   {
//     fAwagsWidget->AwagsTabWidget->removeTab(0);
//     addresstab->setWindowTitle("Addresses");
//     addresstab->show();
//     QMdiSubWindow* subbase= mdiArea->addSubWindow(addresstab, wflags);
//     subbase->setAttribute(Qt::WA_DeleteOnClose, false);
//   }

  if(currenttab)
   {
     fAwagsWidget->AwagsTabWidget->removeTab(0);
     currenttab->setWindowTitle("Current");
     currenttab->show();
     QMdiSubWindow* subbase= mdiArea->addSubWindow(currenttab, wflags);
     subbase->setAttribute(Qt::WA_DeleteOnClose, false);
   }

  if(charactertab)
   {
     fAwagsWidget->AwagsTabWidget->removeTab(0);
     charactertab->setWindowTitle("Characterization");
     charactertab->show();
     QMdiSubWindow* subbase= mdiArea->addSubWindow(charactertab, wflags);
     subbase->setAttribute(Qt::WA_DeleteOnClose, false);
   }

#endif





  setWindowTitle(QString("%1 GUI").arg(fImplementationName));
  ClearOutputBtn_clicked ();


  QObject::connect (fAwagsWidget->AutoAdjustButton, SIGNAL (clicked ()), this, SLOT (AutoAdjustBtn_clicked ()));
  QObject::connect (fAwagsWidget->CalibrateADCButton, SIGNAL (clicked ()), this, SLOT (CalibrateADCBtn_clicked ()));
  QObject::connect (fAwagsWidget->CalibrateResetButton, SIGNAL (clicked ()), this, SLOT (CalibrateResetBtn_clicked ()));

  QObject::connect (fAwagsWidget->DAC_spinBox_all, SIGNAL(valueChanged(int)), this, SLOT(DAC_spinBox_all_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_00, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox00_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_01, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox01_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_02, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox02_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_03, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox03_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_04, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox04_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_05, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox05_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_06, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox06_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_07, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox07_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_08, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox08_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_09, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox09_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_10, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox10_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_11, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox11_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_12, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox12_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_13, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox13_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_14, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox14_changed(int)));
  QObject::connect (fAwagsWidget->DAC_spinBox_15, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox15_changed(int)));

  QObject::connect (fAwagsWidget->Awags1_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_0(int)));

//  QObject::connect (fAwagsWidget->Awags1_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_1(int)));
//  QObject::connect (fAwagsWidget->Awags1_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_2(int)));
//  QObject::connect (fAwagsWidget->Awags1_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_0_3(int)));

  QObject::connect (fAwagsWidget->Awags2_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_0(int)));

//  QObject::connect (fAwagsWidget->Awags2_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_1(int)));
//  QObject::connect (fAwagsWidget->Awags2_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_2(int)));
//  QObject::connect (fAwagsWidget->Awags2_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_1_3(int)));

  QObject::connect (fAwagsWidget->Awags3_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_0(int)));
//  QObject::connect (fAwagsWidget->Awags3_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_1(int)));
//  QObject::connect (fAwagsWidget->Awags3_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_2(int)));
//  QObject::connect (fAwagsWidget->Awags3_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_2_3(int)));

  QObject::connect (fAwagsWidget->Awags4_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_0(int)));

//  QObject::connect (fAwagsWidget->Awags4_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_1(int)));
//  QObject::connect (fAwagsWidget->Awags4_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_2(int)));
//  QObject::connect (fAwagsWidget->Awags4_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_3_3(int)));
//  QObject::connect (fAwagsWidget->Awags5_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_0(int)));
//  QObject::connect (fAwagsWidget->Awags5_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_1(int)));
//  QObject::connect (fAwagsWidget->Awags5_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_2(int)));
//  QObject::connect (fAwagsWidget->Awags5_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_4_3(int)));
//  QObject::connect (fAwagsWidget->Awags6_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_0(int)));
//  QObject::connect (fAwagsWidget->Awags6_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_1(int)));
//  QObject::connect (fAwagsWidget->Awags6_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_2(int)));
//  QObject::connect (fAwagsWidget->Awags6_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_5_3(int)));
//  QObject::connect (fAwagsWidget->Awags7_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_0(int)));
//  QObject::connect (fAwagsWidget->Awags7_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_1(int)));
//  QObject::connect (fAwagsWidget->Awags7_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_2(int)));
//  QObject::connect (fAwagsWidget->Awags7_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_6_3(int)));
//  QObject::connect (fAwagsWidget->Awags8_DACSlider_1, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_0(int)));
//  QObject::connect (fAwagsWidget->Awags8_DACSlider_2, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_1(int)));
//  QObject::connect (fAwagsWidget->Awags8_DACSlider_3, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_2(int)));
//  QObject::connect (fAwagsWidget->Awags8_DACSlider_4, SIGNAL(valueChanged(int)), this, SLOT (DAC_changed_7_3(int)));

  QObject::connect (fAwagsWidget->Awags1_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_0 ()));
//  QObject::connect (fAwagsWidget->Awags1_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_1 ()));
//  QObject::connect (fAwagsWidget->Awags1_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_2 ()));
//  QObject::connect (fAwagsWidget->Awags1_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_0_3 ()));

  QObject::connect (fAwagsWidget->Awags2_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_0 ()));
//  QObject::connect (fAwagsWidget->Awags2_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_1 ()));
//  QObject::connect (fAwagsWidget->Awags2_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_2 ()));
//  QObject::connect (fAwagsWidget->Awags2_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_1_3 ()));

  QObject::connect (fAwagsWidget->Awags3_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_0 ()));
//  QObject::connect (fAwagsWidget->Awags3_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_1 ()));
//  QObject::connect (fAwagsWidget->Awags3_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_2 ()));
//  QObject::connect (fAwagsWidget->Awags3_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_2_3 ()));

  QObject::connect (fAwagsWidget->Awags4_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_0 ()));
//  QObject::connect (fAwagsWidget->Awags4_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_1 ()));
//  QObject::connect (fAwagsWidget->Awags4_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_2 ()));
//  QObject::connect (fAwagsWidget->Awags4_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_3_3 ()));
//  QObject::connect (fAwagsWidget->Awags5_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_0 ()));
//  QObject::connect (fAwagsWidget->Awags5_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_1 ()));
//  QObject::connect (fAwagsWidget->Awags5_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_2 ()));
//  QObject::connect (fAwagsWidget->Awags5_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_4_3 ()));
//  QObject::connect (fAwagsWidget->Awags6_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_0 ()));
//  QObject::connect (fAwagsWidget->Awags6_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_1 ()));
//  QObject::connect (fAwagsWidget->Awags6_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_2 ()));
//  QObject::connect (fAwagsWidget->Awags6_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_5_3 ()));
//  QObject::connect (fAwagsWidget->Awags7_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_0 ()));
//  QObject::connect (fAwagsWidget->Awags7_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_1 ()));
//  QObject::connect (fAwagsWidget->Awags7_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_2 ()));
//  QObject::connect (fAwagsWidget->Awags7_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_6_3 ()));
//  QObject::connect (fAwagsWidget->Awags8_DAClineEdit_1, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_0 ()));
//  QObject::connect (fAwagsWidget->Awags8_DAClineEdit_2, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_1 ()));
//  QObject::connect (fAwagsWidget->Awags8_DAClineEdit_3, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_2 ()));
//  QObject::connect (fAwagsWidget->Awags8_DAClineEdit_4, SIGNAL (editingFinished ()), this, SLOT (DAC_enterText_7_3 ()));

  QObject::connect (fAwagsWidget->AutocalibrateButton_1, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_0 ()));
  QObject::connect (fAwagsWidget->AutocalibrateButton_2, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_1 ()));
  QObject::connect (fAwagsWidget->AutocalibrateButton_3, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_2 ()));
  QObject::connect (fAwagsWidget->AutocalibrateButton_4, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_3 ()));

//  QObject::connect (fAwagsWidget->AutocalibrateButton_5, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_4 ()));
//  QObject::connect (fAwagsWidget->AutocalibrateButton_6, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_5 ()));
//  QObject::connect (fAwagsWidget->AutocalibrateButton_7, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_6 ()));
//  QObject::connect (fAwagsWidget->AutocalibrateButton_8, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_7 ()));

  QObject::connect (fAwagsWidget->AutocalibrateButton_all, SIGNAL (pressed ()), this, SLOT (AutoCalibrate_all ()));

//  QObject::connect (fAwagsWidget->PulserCheckBox_0, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_0()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_1, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_0()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_0, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_0()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_1, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_0()));
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_0, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_0()));
//
//  QObject::connect (fAwagsWidget->PulserCheckBox_2, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_1()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_3, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_1()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_2, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_1()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_3, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_1()));
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_1, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_1()));
//
//  QObject::connect (fAwagsWidget->PulserCheckBox_4, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_2()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_5, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_2()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_4, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_2()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_5, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_2()));
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_2()));
//
//  QObject::connect (fAwagsWidget->PulserCheckBox_6, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_3()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_7, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_3()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_6, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_3()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_7, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_3()));
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_3()));
//
//  QObject::connect (fAwagsWidget->PulserCheckBox_8, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_4()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_9, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_4()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_8, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_4()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_9, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_4()));
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_4, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_4()));
//
//  QObject::connect (fAwagsWidget->PulserCheckBox_10, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_5()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_11, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_5()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_10, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_5()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_11, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_5()));
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_5, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_5()));
//
//  QObject::connect (fAwagsWidget->PulserCheckBox_12, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_6()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_13, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_6()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_12, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_6()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_13, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_6()));
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_6, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_6()));
//
//  QObject::connect (fAwagsWidget->PulserCheckBox_14, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_7()));
//  QObject::connect (fAwagsWidget->PulserCheckBox_15, SIGNAL(stateChanged(int)), this, SLOT (PulserChanged_7()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_14, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_7()));
//  QObject::connect (fAwagsWidget->PulserAmpSpinBox_15, SIGNAL(valueChanged(int)), this, SLOT (PulserChanged_7()));
//
//  QObject::connect (fAwagsWidget->AwagsTestPolarityBox_7, SIGNAL(currentIndexChanged(int)), this, SLOT (PulserChanged_7()));
//
  QObject::connect (fAwagsWidget->gainCombo_0, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_0()));
  QObject::connect (fAwagsWidget->gainCombo_1, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_1()));
  QObject::connect (fAwagsWidget->gainCombo_2, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_2()));
  QObject::connect (fAwagsWidget->gainCombo_3, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_3()));

//  QObject::connect (fAwagsWidget->gainCombo_4, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_4()));
//  QObject::connect (fAwagsWidget->gainCombo_5, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_5()));
//  QObject::connect (fAwagsWidget->gainCombo_6, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_6()));
//  QObject::connect (fAwagsWidget->gainCombo_7, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_7()));
//  QObject::connect (fAwagsWidget->gainCombo_8, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_8()));
//  QObject::connect (fAwagsWidget->gainCombo_9, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_9()));
//  QObject::connect (fAwagsWidget->gainCombo_10, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_10()));
//  QObject::connect (fAwagsWidget->gainCombo_11, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_11()));
//  QObject::connect (fAwagsWidget->gainCombo_12, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_12()));
//  QObject::connect (fAwagsWidget->gainCombo_13, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_13()));
//  QObject::connect (fAwagsWidget->gainCombo_14, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_14()));
//  QObject::connect (fAwagsWidget->gainCombo_15, SIGNAL(currentIndexChanged(int)), this, SLOT (GainChanged_15()));

//  QObject::connect (fAwagsWidget->AwagsRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));
//  QObject::connect (fAwagsWidget->LoGainRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));
//  QObject::connect (fAwagsWidget->StretcherOnRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));
  QObject::connect (fAwagsWidget->ProtoRadioButton, SIGNAL(toggled(bool)), this, SLOT (SwitchChanged()));

//  QObject::connect (fAwagsWidget->SimpleModeOnRadioButton, SIGNAL(toggled(bool)), this, SLOT (SetSimpleSwitches(bool)));


//  QObject::connect (fAwagsWidget->InverseMappingCheckBox, SIGNAL(stateChanged(int)), this, SLOT (InverseMapping_changed(int)));

  QObject::connect (fAwagsWidget->DoSampleButton, SIGNAL (clicked ()), this, SLOT (AcquireSamplesBtn_clicked ()));
  QObject::connect (fAwagsWidget->DumpSampleButton, SIGNAL (clicked ()), this, SLOT (DumpSamplesBtn_clicked ()));

  QObject::connect (fAwagsWidget->ZoomButton, SIGNAL (clicked ()), this, SLOT (ZoomSampleBtn_clicked ()));
  QObject::connect (fAwagsWidget->UnzoomButton, SIGNAL (clicked ()), this, SLOT (UnzoomSampleBtn_clicked ()));
  QObject::connect (fAwagsWidget->RefreshSampleButton, SIGNAL (clicked ()), this, SLOT (RefreshSampleBtn_clicked ()));
//
//  QObject::connect (fAwagsWidget->PeakFinderButton, SIGNAL (clicked ()), this, SLOT (PeakFinderBtn_clicked ()));

//  QObject::connect (fAwagsWidget->PulseTimerCheckBox, SIGNAL(stateChanged(int)), this, SLOT(PulseTimer_changed(int)));
//  QObject::connect (fAwagsWidget->FrequencyComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT (PulseFrequencyChanged(int)));
//
//  QObject::connect (fAwagsWidget->PulseBroadcastCheckBox, SIGNAL(stateChanged(int)), this, SLOT(PulseBroadcast_changed(int)));

  //QObject::connect (BenchmarkButtonBox, SIGNAL(accepted()), this, SLOT(StartBenchmarkPressed()));
  //QObject::connect (BenchmarkButtonBox, SIGNAL(rejected()), this, SLOT(CancelBenchmarkPressed()));

  QObject::connect (fAwagsWidget->BenchmarkButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(BenchmarkPressed(QAbstractButton*)));

  QObject::connect (fAwagsWidget->ReferenceLoadButtonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(ChangeReferenceDataPressed(QAbstractButton*)));


  QObject::connect (fAwagsWidget->PlotTabWidget, SIGNAL(currentChanged(int)), this, SLOT (PlotTabChanged(int)));

//  QObject::connect (fAwagsWidget->MaximaTableWidget, SIGNAL(cellDoubleClicked(int, int )), this, SLOT (MaximaCellDoubleClicked(int,int)));


  QObject::connect (fAwagsWidget->BaselineLowerSlider, SIGNAL(valueChanged(int)), this, SLOT (RefreshBaselines()));
  QObject::connect (fAwagsWidget->BaselineUpperSlider, SIGNAL(valueChanged(int)), this, SLOT (RefreshBaselines()));
  QObject::connect (fAwagsWidget->ReadoutRadioButton, SIGNAL(toggled(bool)), this, SLOT (RefreshBaselines()));

 // QObject::connect (fAwagsWidget->Baseline_Box_invert, SIGNAL(stateChanged(int)), this, SLOT(BaselineInvert_changed(int)));


  QObject::connect (fAwagsWidget->PowerCheckBox_1, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_0(int)));
  QObject::connect (fAwagsWidget->PowerCheckBox_2, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_1(int)));
  QObject::connect (fAwagsWidget->PowerCheckBox_3, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_2(int)));
  QObject::connect (fAwagsWidget->PowerCheckBox_4, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_3(int)));

//  QObject::connect (fAwagsWidget->PowerCheckBox_5, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_4(int)));
//  QObject::connect (fAwagsWidget->PowerCheckBox_6, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_5(int)));
//  QObject::connect (fAwagsWidget->PowerCheckBox_7, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_6(int)));
//  QObject::connect (fAwagsWidget->PowerCheckBox_8, SIGNAL(stateChanged(int)), this, SLOT (PowerChanged_7(int)));


  QObject::connect (fAwagsWidget->MeasureCurrentsPushButton, SIGNAL (clicked ()), this, SLOT (MeasureCurrentsPushButton_clicked ()));
  QObject::connect (fAwagsWidget->InitKeithleyPushButton, SIGNAL (clicked ()), this, SLOT (InitKeithleyPushButton_clicked ()));

 // QObject::connect (fAwagsWidget->AddressScanPushButton, SIGNAL (clicked ()), this, SLOT (AddressScanPushButton_clicked ()));




  /** JAM put references to designer checkboxes into array to be handled later easily: */
  fBaselineBoxes[0] = fAwagsWidget->Baseline_Box_00;
  fBaselineBoxes[1] = fAwagsWidget->Baseline_Box_01;
  fBaselineBoxes[2] = fAwagsWidget->Baseline_Box_02;
  fBaselineBoxes[3] = fAwagsWidget->Baseline_Box_03;
  fBaselineBoxes[4] = fAwagsWidget->Baseline_Box_04;
  fBaselineBoxes[5] = fAwagsWidget->Baseline_Box_05;
  fBaselineBoxes[6] = fAwagsWidget->Baseline_Box_06;
  fBaselineBoxes[7] = fAwagsWidget->Baseline_Box_07;
  fBaselineBoxes[8] = fAwagsWidget->Baseline_Box_08;
  fBaselineBoxes[9] = fAwagsWidget->Baseline_Box_09;
  fBaselineBoxes[10] = fAwagsWidget->Baseline_Box_10;
  fBaselineBoxes[11] = fAwagsWidget->Baseline_Box_11;
  fBaselineBoxes[12] = fAwagsWidget->Baseline_Box_12;
  fBaselineBoxes[13] = fAwagsWidget->Baseline_Box_13;
  fBaselineBoxes[14] = fAwagsWidget->Baseline_Box_14;
  fBaselineBoxes[15] = fAwagsWidget->Baseline_Box_15;

  fDACSpinBoxes[0] = fAwagsWidget->DAC_spinBox_00;
  fDACSpinBoxes[1] = fAwagsWidget->DAC_spinBox_01;
  fDACSpinBoxes[2] = fAwagsWidget->DAC_spinBox_02;
  fDACSpinBoxes[3] = fAwagsWidget->DAC_spinBox_03;
  fDACSpinBoxes[4] = fAwagsWidget->DAC_spinBox_04;
  fDACSpinBoxes[5] = fAwagsWidget->DAC_spinBox_05;
  fDACSpinBoxes[6] = fAwagsWidget->DAC_spinBox_06;
  fDACSpinBoxes[7] = fAwagsWidget->DAC_spinBox_07;
  fDACSpinBoxes[8] = fAwagsWidget->DAC_spinBox_08;
  fDACSpinBoxes[9] = fAwagsWidget->DAC_spinBox_09;
  fDACSpinBoxes[10] = fAwagsWidget->DAC_spinBox_10;
  fDACSpinBoxes[11] = fAwagsWidget->DAC_spinBox_11;
  fDACSpinBoxes[12] = fAwagsWidget->DAC_spinBox_12;
  fDACSpinBoxes[13] = fAwagsWidget->DAC_spinBox_13;
  fDACSpinBoxes[14] = fAwagsWidget->DAC_spinBox_14;
  fDACSpinBoxes[15] = fAwagsWidget->DAC_spinBox_15;

  fADCLineEdit[0] = fAwagsWidget->ADC_Value_00;
  fADCLineEdit[1] = fAwagsWidget->ADC_Value_01;
  fADCLineEdit[2] = fAwagsWidget->ADC_Value_02;
  fADCLineEdit[3] = fAwagsWidget->ADC_Value_03;
  fADCLineEdit[4] = fAwagsWidget->ADC_Value_04;
  fADCLineEdit[5] = fAwagsWidget->ADC_Value_05;
  fADCLineEdit[6] = fAwagsWidget->ADC_Value_06;
  fADCLineEdit[7] = fAwagsWidget->ADC_Value_07;
  fADCLineEdit[8] = fAwagsWidget->ADC_Value_08;
  fADCLineEdit[9] = fAwagsWidget->ADC_Value_09;
  fADCLineEdit[10] = fAwagsWidget->ADC_Value_10;
  fADCLineEdit[11] = fAwagsWidget->ADC_Value_11;
  fADCLineEdit[12] = fAwagsWidget->ADC_Value_12;
  fADCLineEdit[13] = fAwagsWidget->ADC_Value_13;
  fADCLineEdit[14] = fAwagsWidget->ADC_Value_14;
  fADCLineEdit[15] = fAwagsWidget->ADC_Value_15;

  fDACSlider[0][0] = fAwagsWidget->Awags1_DACSlider_1;
//  fDACSlider[0][1] = fAwagsWidget->Awags1_DACSlider_2;
//  fDACSlider[0][2] = fAwagsWidget->Awags1_DACSlider_3;
//  fDACSlider[0][3] = fAwagsWidget->Awags1_DACSlider_4;

  fDACSlider[1][0] = fAwagsWidget->Awags2_DACSlider_1;
//  fDACSlider[1][1] = fAwagsWidget->Awags2_DACSlider_2;
//  fDACSlider[1][2] = fAwagsWidget->Awags2_DACSlider_3;
//  fDACSlider[1][3] = fAwagsWidget->Awags2_DACSlider_4;

  fDACSlider[2][0] = fAwagsWidget->Awags3_DACSlider_1;
//  fDACSlider[2][1] = fAwagsWidget->Awags3_DACSlider_2;
//  fDACSlider[2][2] = fAwagsWidget->Awags3_DACSlider_3;
//  fDACSlider[2][3] = fAwagsWidget->Awags3_DACSlider_4;
  fDACSlider[3][0] = fAwagsWidget->Awags4_DACSlider_1;
//  fDACSlider[3][1] = fAwagsWidget->Awags4_DACSlider_2;
//  fDACSlider[3][2] = fAwagsWidget->Awags4_DACSlider_3;
//  fDACSlider[3][3] = fAwagsWidget->Awags4_DACSlider_4;
//  fDACSlider[4][0] = fAwagsWidget->Awags5_DACSlider_1;
//  fDACSlider[4][1] = fAwagsWidget->Awags5_DACSlider_2;
//  fDACSlider[4][2] = fAwagsWidget->Awags5_DACSlider_3;
//  fDACSlider[4][3] = fAwagsWidget->Awags5_DACSlider_4;
//  fDACSlider[5][0] = fAwagsWidget->Awags6_DACSlider_1;
//  fDACSlider[5][1] = fAwagsWidget->Awags6_DACSlider_2;
//  fDACSlider[5][2] = fAwagsWidget->Awags6_DACSlider_3;
//  fDACSlider[5][3] = fAwagsWidget->Awags6_DACSlider_4;
//  fDACSlider[6][0] = fAwagsWidget->Awags7_DACSlider_1;
//  fDACSlider[6][1] = fAwagsWidget->Awags7_DACSlider_2;
//  fDACSlider[6][2] = fAwagsWidget->Awags7_DACSlider_3;
//  fDACSlider[6][3] = fAwagsWidget->Awags7_DACSlider_4;
//  fDACSlider[7][0] = fAwagsWidget->Awags8_DACSlider_1;
//  fDACSlider[7][1] = fAwagsWidget->Awags8_DACSlider_2;
//  fDACSlider[7][2] = fAwagsWidget->Awags8_DACSlider_3;
//  fDACSlider[7][3] = fAwagsWidget->Awags8_DACSlider_4;

  fDACLineEdit[0][0] = fAwagsWidget->Awags1_DAClineEdit_1;
//  fDACLineEdit[0][1] = fAwagsWidget->Awags1_DAClineEdit_2;
//  fDACLineEdit[0][2] = fAwagsWidget->Awags1_DAClineEdit_3;
//  fDACLineEdit[0][3] = fAwagsWidget->Awags1_DAClineEdit_4;

  fDACLineEdit[1][0] = fAwagsWidget->Awags2_DAClineEdit_1;
//  fDACLineEdit[1][1] = fAwagsWidget->Awags2_DAClineEdit_2;
//  fDACLineEdit[1][2] = fAwagsWidget->Awags2_DAClineEdit_3;
//  fDACLineEdit[1][3] = fAwagsWidget->Awags2_DAClineEdit_4;

  fDACLineEdit[2][0] = fAwagsWidget->Awags3_DAClineEdit_1;
//  fDACLineEdit[2][1] = fAwagsWidget->Awags3_DAClineEdit_2;
//  fDACLineEdit[2][2] = fAwagsWidget->Awags3_DAClineEdit_3;
//  fDACLineEdit[2][3] = fAwagsWidget->Awags3_DAClineEdit_4;

  fDACLineEdit[3][0] = fAwagsWidget->Awags4_DAClineEdit_1;
//  fDACLineEdit[3][1] = fAwagsWidget->Awags4_DAClineEdit_2;
//  fDACLineEdit[3][2] = fAwagsWidget->Awags4_DAClineEdit_3;
//  fDACLineEdit[3][3] = fAwagsWidget->Awags4_DAClineEdit_4;
//  fDACLineEdit[4][0] = fAwagsWidget->Awags5_DAClineEdit_1;
//  fDACLineEdit[4][1] = fAwagsWidget->Awags5_DAClineEdit_2;
//  fDACLineEdit[4][2] = fAwagsWidget->Awags5_DAClineEdit_3;
//  fDACLineEdit[4][3] = fAwagsWidget->Awags5_DAClineEdit_4;
//  fDACLineEdit[5][0] = fAwagsWidget->Awags6_DAClineEdit_1;
//  fDACLineEdit[5][1] = fAwagsWidget->Awags6_DAClineEdit_2;
//  fDACLineEdit[5][2] = fAwagsWidget->Awags6_DAClineEdit_3;
//  fDACLineEdit[5][3] = fAwagsWidget->Awags6_DAClineEdit_4;
//  fDACLineEdit[6][0] = fAwagsWidget->Awags7_DAClineEdit_1;
//  fDACLineEdit[6][1] = fAwagsWidget->Awags7_DAClineEdit_2;
//  fDACLineEdit[6][2] = fAwagsWidget->Awags7_DAClineEdit_3;
//  fDACLineEdit[6][3] = fAwagsWidget->Awags7_DAClineEdit_4;
//  fDACLineEdit[7][0] = fAwagsWidget->Awags8_DAClineEdit_1;
//  fDACLineEdit[7][1] = fAwagsWidget->Awags8_DAClineEdit_2;
//  fDACLineEdit[7][2] = fAwagsWidget->Awags8_DAClineEdit_3;
//  fDACLineEdit[7][3] = fAwagsWidget->Awags8_DAClineEdit_4;

//  fAwagsPulsePolarityCombo[0] = fAwagsWidget->AwagsTestPolarityBox_0;
//  fAwagsPulsePolarityCombo[1] = fAwagsWidget->AwagsTestPolarityBox_1;
//  fAwagsPulsePolarityCombo[2] = fAwagsWidget->AwagsTestPolarityBox_2;
//  fAwagsPulsePolarityCombo[3] = fAwagsWidget->AwagsTestPolarityBox_3;
//  fAwagsPulsePolarityCombo[4] = fAwagsWidget->AwagsTestPolarityBox_4;
//  fAwagsPulsePolarityCombo[5] = fAwagsWidget->AwagsTestPolarityBox_5;
//  fAwagsPulsePolarityCombo[6] = fAwagsWidget->AwagsTestPolarityBox_6;
//  fAwagsPulsePolarityCombo[7] = fAwagsWidget->AwagsTestPolarityBox_7;

//  fAwagsPulseEnabledCheckbox[0][0] = fAwagsWidget->PulserCheckBox_0;
//  fAwagsPulseEnabledCheckbox[0][1] = fAwagsWidget->PulserCheckBox_1;
//  fAwagsPulseEnabledCheckbox[1][0] = fAwagsWidget->PulserCheckBox_2;
//  fAwagsPulseEnabledCheckbox[1][1] = fAwagsWidget->PulserCheckBox_3;
//  fAwagsPulseEnabledCheckbox[2][0] = fAwagsWidget->PulserCheckBox_4;
//  fAwagsPulseEnabledCheckbox[2][1] = fAwagsWidget->PulserCheckBox_5;
//  fAwagsPulseEnabledCheckbox[3][0] = fAwagsWidget->PulserCheckBox_6;
//  fAwagsPulseEnabledCheckbox[3][1] = fAwagsWidget->PulserCheckBox_7;
//  fAwagsPulseEnabledCheckbox[4][0] = fAwagsWidget->PulserCheckBox_8;
//  fAwagsPulseEnabledCheckbox[4][1] = fAwagsWidget->PulserCheckBox_9;
//  fAwagsPulseEnabledCheckbox[5][0] = fAwagsWidget->PulserCheckBox_10;
//  fAwagsPulseEnabledCheckbox[5][1] = fAwagsWidget->PulserCheckBox_11;
//  fAwagsPulseEnabledCheckbox[6][0] = fAwagsWidget->PulserCheckBox_12;
//  fAwagsPulseEnabledCheckbox[6][1] = fAwagsWidget->PulserCheckBox_13;
//  fAwagsPulseEnabledCheckbox[7][0] = fAwagsWidget->PulserCheckBox_14;
//  fAwagsPulseEnabledCheckbox[7][1] = fAwagsWidget->PulserCheckBox_15;
//
//  fAwagsPulseAmplitudeSpin[0][0] = fAwagsWidget->PulserAmpSpinBox_0;
//  fAwagsPulseAmplitudeSpin[0][1] = fAwagsWidget->PulserAmpSpinBox_1;
//  fAwagsPulseAmplitudeSpin[1][0] = fAwagsWidget->PulserAmpSpinBox_2;
//  fAwagsPulseAmplitudeSpin[1][1] = fAwagsWidget->PulserAmpSpinBox_3;
//  fAwagsPulseAmplitudeSpin[2][0] = fAwagsWidget->PulserAmpSpinBox_4;
//  fAwagsPulseAmplitudeSpin[2][1] = fAwagsWidget->PulserAmpSpinBox_5;
//  fAwagsPulseAmplitudeSpin[3][0] = fAwagsWidget->PulserAmpSpinBox_6;
//  fAwagsPulseAmplitudeSpin[3][1] = fAwagsWidget->PulserAmpSpinBox_7;
//  fAwagsPulseAmplitudeSpin[4][0] = fAwagsWidget->PulserAmpSpinBox_8;
//  fAwagsPulseAmplitudeSpin[4][1] = fAwagsWidget->PulserAmpSpinBox_9;
//  fAwagsPulseAmplitudeSpin[5][0] = fAwagsWidget->PulserAmpSpinBox_10;
//  fAwagsPulseAmplitudeSpin[5][1] = fAwagsWidget->PulserAmpSpinBox_11;
//  fAwagsPulseAmplitudeSpin[6][0] = fAwagsWidget->PulserAmpSpinBox_12;
//  fAwagsPulseAmplitudeSpin[6][1] = fAwagsWidget->PulserAmpSpinBox_13;
//  fAwagsPulseAmplitudeSpin[7][0] = fAwagsWidget->PulserAmpSpinBox_14;
//  fAwagsPulseAmplitudeSpin[7][1] = fAwagsWidget->PulserAmpSpinBox_15;
//
//  fAwagsPulseGroup[0] = fAwagsWidget->AwagsPulseBox_1;
//  fAwagsPulseGroup[1] = fAwagsWidget->AwagsPulseBox_2;
//  fAwagsPulseGroup[2] = fAwagsWidget->AwagsPulseBox_3;
//  fAwagsPulseGroup[3] = fAwagsWidget->AwagsPulseBox_4;
//  fAwagsPulseGroup[4] = fAwagsWidget->AwagsPulseBox_5;
//  fAwagsPulseGroup[5] = fAwagsWidget->AwagsPulseBox_6;
//  fAwagsPulseGroup[6] = fAwagsWidget->AwagsPulseBox_7;
//  fAwagsPulseGroup[7] = fAwagsWidget->AwagsPulseBox_8;


  fAwagsGainGroup[0] = fAwagsWidget->AwagsGainBox_1;
  fAwagsGainGroup[1] = fAwagsWidget->AwagsGainBox_2;
  fAwagsGainGroup[2] = fAwagsWidget->AwagsGainBox_3;
  fAwagsGainGroup[3] = fAwagsWidget->AwagsGainBox_4;

//  fAwagsGainGroup[4] = fAwagsWidget->AwagsGainBox_5;
//  fAwagsGainGroup[5] = fAwagsWidget->AwagsGainBox_6;
//  fAwagsGainGroup[6] = fAwagsWidget->AwagsGainBox_7;
//  fAwagsGainGroup[7] = fAwagsWidget->AwagsGainBox_8;


   fAwagsDACGroup[0] = fAwagsWidget->AwagsBox1;
   fAwagsDACGroup[1] = fAwagsWidget->AwagsBox2;
   fAwagsDACGroup[2] = fAwagsWidget->AwagsBox3;
   fAwagsDACGroup[3] = fAwagsWidget->AwagsBox4;
//   fAwagsDACGroup[4] = fAwagsWidget->AwagsBox5;
//   fAwagsDACGroup[5] = fAwagsWidget->AwagsBox6;
//   fAwagsDACGroup[6] = fAwagsWidget->AwagsBox7;
//   fAwagsDACGroup[7] = fAwagsWidget->AwagsBox8;


//  fAwagsGainCombo[0][0] = fAwagsWidget->gainCombo_0;
//  fAwagsGainCombo[0][1] = fAwagsWidget->gainCombo_1;
//  fAwagsGainCombo[1][0] = fAwagsWidget->gainCombo_2;
//  fAwagsGainCombo[1][1] = fAwagsWidget->gainCombo_3;
//  fAwagsGainCombo[2][0] = fAwagsWidget->gainCombo_4;
//  fAwagsGainCombo[2][1] = fAwagsWidget->gainCombo_5;
//  fAwagsGainCombo[3][0] = fAwagsWidget->gainCombo_6;
//  fAwagsGainCombo[3][1] = fAwagsWidget->gainCombo_7;
//  fAwagsGainCombo[4][0] = fAwagsWidget->gainCombo_8;
//  fAwagsGainCombo[4][1] = fAwagsWidget->gainCombo_9;
//  fAwagsGainCombo[5][0] = fAwagsWidget->gainCombo_10;
//  fAwagsGainCombo[5][1] = fAwagsWidget->gainCombo_11;
//  fAwagsGainCombo[6][0] = fAwagsWidget->gainCombo_12;
//  fAwagsGainCombo[6][1] = fAwagsWidget->gainCombo_13;
//  fAwagsGainCombo[7][0] = fAwagsWidget->gainCombo_14;
//  fAwagsGainCombo[7][1] = fAwagsWidget->gainCombo_15;


   fAwagsGainCombo[0] = fAwagsWidget->gainCombo_0;
    fAwagsGainCombo[1] = fAwagsWidget->gainCombo_1;
    fAwagsGainCombo[2] = fAwagsWidget->gainCombo_2;
    fAwagsGainCombo[3] = fAwagsWidget->gainCombo_3;




  fSamplingBoxes[0] = fAwagsWidget->Sampling_Box_0;
  fSamplingBoxes[1] = fAwagsWidget->Sampling_Box_1;
  fSamplingBoxes[2] = fAwagsWidget->Sampling_Box_2;
  fSamplingBoxes[3] = fAwagsWidget->Sampling_Box_3;
  fSamplingBoxes[4] = fAwagsWidget->Sampling_Box_4;
  fSamplingBoxes[5] = fAwagsWidget->Sampling_Box_5;
  fSamplingBoxes[6] = fAwagsWidget->Sampling_Box_6;
  fSamplingBoxes[7] = fAwagsWidget->Sampling_Box_7;
  fSamplingBoxes[8] = fAwagsWidget->Sampling_Box_8;
  fSamplingBoxes[9] = fAwagsWidget->Sampling_Box_9;
  fSamplingBoxes[10] = fAwagsWidget->Sampling_Box_10;
  fSamplingBoxes[11] = fAwagsWidget->Sampling_Box_11;
  fSamplingBoxes[12] = fAwagsWidget->Sampling_Box_12;
  fSamplingBoxes[13] = fAwagsWidget->Sampling_Box_13;
  fSamplingBoxes[14] = fAwagsWidget->Sampling_Box_14;
  fSamplingBoxes[15] = fAwagsWidget->Sampling_Box_15;

  fSamplingMeanLineEdit[0] = fAwagsWidget->ADC_SampleMean_0;
  fSamplingMeanLineEdit[1] = fAwagsWidget->ADC_SampleMean_1;
  fSamplingMeanLineEdit[2] = fAwagsWidget->ADC_SampleMean_2;
  fSamplingMeanLineEdit[3] = fAwagsWidget->ADC_SampleMean_3;
  fSamplingMeanLineEdit[4] = fAwagsWidget->ADC_SampleMean_4;
  fSamplingMeanLineEdit[5] = fAwagsWidget->ADC_SampleMean_5;
  fSamplingMeanLineEdit[6] = fAwagsWidget->ADC_SampleMean_6;
  fSamplingMeanLineEdit[7] = fAwagsWidget->ADC_SampleMean_7;
  fSamplingMeanLineEdit[8] = fAwagsWidget->ADC_SampleMean_8;
  fSamplingMeanLineEdit[9] = fAwagsWidget->ADC_SampleMean_9;
  fSamplingMeanLineEdit[10] = fAwagsWidget->ADC_SampleMean_10;
  fSamplingMeanLineEdit[11] = fAwagsWidget->ADC_SampleMean_11;
  fSamplingMeanLineEdit[12] = fAwagsWidget->ADC_SampleMean_12;
  fSamplingMeanLineEdit[13] = fAwagsWidget->ADC_SampleMean_13;
  fSamplingMeanLineEdit[14] = fAwagsWidget->ADC_SampleMean_14;
  fSamplingMeanLineEdit[15] = fAwagsWidget->ADC_SampleMean_15;

  fSamplingSigmaLineEdit[0] = fAwagsWidget->ADC_SampleSigma_0;
  fSamplingSigmaLineEdit[1] = fAwagsWidget->ADC_SampleSigma_1;
  fSamplingSigmaLineEdit[2] = fAwagsWidget->ADC_SampleSigma_2;
  fSamplingSigmaLineEdit[3] = fAwagsWidget->ADC_SampleSigma_3;
  fSamplingSigmaLineEdit[4] = fAwagsWidget->ADC_SampleSigma_4;
  fSamplingSigmaLineEdit[5] = fAwagsWidget->ADC_SampleSigma_5;
  fSamplingSigmaLineEdit[6] = fAwagsWidget->ADC_SampleSigma_6;
  fSamplingSigmaLineEdit[7] = fAwagsWidget->ADC_SampleSigma_7;
  fSamplingSigmaLineEdit[8] = fAwagsWidget->ADC_SampleSigma_8;
  fSamplingSigmaLineEdit[9] = fAwagsWidget->ADC_SampleSigma_9;
  fSamplingSigmaLineEdit[10] = fAwagsWidget->ADC_SampleSigma_10;
  fSamplingSigmaLineEdit[11] = fAwagsWidget->ADC_SampleSigma_11;
  fSamplingSigmaLineEdit[12] = fAwagsWidget->ADC_SampleSigma_12;
  fSamplingSigmaLineEdit[13] = fAwagsWidget->ADC_SampleSigma_13;
  fSamplingSigmaLineEdit[14] = fAwagsWidget->ADC_SampleSigma_14;
  fSamplingSigmaLineEdit[15] = fAwagsWidget->ADC_SampleSigma_15;

  fAwagsPowerGroup[0]=fAwagsWidget->PowerGroupBox_1;
  fAwagsPowerGroup[1]=fAwagsWidget->PowerGroupBox_2;
//  fAwagsPowerGroup[2]=fAwagsWidget->PowerGroupBox_3;
//  fAwagsPowerGroup[3]=fAwagsWidget->PowerGroupBox_4;
//  fAwagsPowerGroup[4]=fAwagsWidget->PowerGroupBox_5;
//  fAwagsPowerGroup[5]=fAwagsWidget->PowerGroupBox_6;
//  fAwagsPowerGroup[6]=fAwagsWidget->PowerGroupBox_7;
//  fAwagsPowerGroup[7]=fAwagsWidget->PowerGroupBox_8;

  fAwagsAddressLabel[0]=fAwagsWidget->AwagsAdressLabel_1;
  fAwagsAddressLabel[1]=fAwagsWidget->AwagsAdressLabel_2;
  fAwagsAddressLabel[2]=fAwagsWidget->AwagsAdressLabel_3;
  fAwagsAddressLabel[3]=fAwagsWidget->AwagsAdressLabel_4;
//  fAwagsAddressLabel[4]=fAwagsWidget->AwagsAdressLabel_5;
//  fAwagsAddressLabel[5]=fAwagsWidget->AwagsAdressLabel_6;
//  fAwagsAddressLabel[6]=fAwagsWidget->AwagsAdressLabel_7;
//  fAwagsAddressLabel[7]=fAwagsWidget->AwagsAdressLabel_8;



  fAwagsPowerCheckbox[0]=fAwagsWidget->PowerCheckBox_1;
  fAwagsPowerCheckbox[1]=fAwagsWidget->PowerCheckBox_2;
  fAwagsPowerCheckbox[2]=fAwagsWidget->PowerCheckBox_3;
  fAwagsPowerCheckbox[3]=fAwagsWidget->PowerCheckBox_4;
//  fAwagsPowerCheckbox[4]=fAwagsWidget->PowerCheckBox_5;
//  fAwagsPowerCheckbox[5]=fAwagsWidget->PowerCheckBox_6;
//  fAwagsPowerCheckbox[6]=fAwagsWidget->PowerCheckBox_7;
//  fAwagsPowerCheckbox[7]=fAwagsWidget->PowerCheckBox_8;


  fAwagsPowerLabel[0]=fAwagsWidget->OnLabel_1;
  fAwagsPowerLabel[1]=fAwagsWidget->OnLabel_2;
  fAwagsPowerLabel[2]=fAwagsWidget->OnLabel_3;
  fAwagsPowerLabel[3]=fAwagsWidget->OnLabel_4;
//  fAwagsPowerLabel[4]=fAwagsWidget->OnLabel_5;
//  fAwagsPowerLabel[5]=fAwagsWidget->OnLabel_6;
//  fAwagsPowerLabel[6]=fAwagsWidget->OnLabel_7;
//  fAwagsPowerLabel[7]=fAwagsWidget->OnLabel_8;




   fAwagsSerialLineEdit[0]=fAwagsWidget->AwagsSerialNum_1;
   fAwagsSerialLineEdit[1]=fAwagsWidget->AwagsSerialNum_2;
   fAwagsSerialLineEdit[2]=fAwagsWidget->AwagsSerialNum_3;
   fAwagsSerialLineEdit[3]=fAwagsWidget->AwagsSerialNum_4;
//   fAwagsSerialLineEdit[4]=fAwagsWidget->AwagsSerialNum_5;
//   fAwagsSerialLineEdit[5]=fAwagsWidget->AwagsSerialNum_6;
//   fAwagsSerialLineEdit[6]=fAwagsWidget->AwagsSerialNum_7;
//   fAwagsSerialLineEdit[7]=fAwagsWidget->AwagsSerialNum_8;


   fAwagsCurrentASICLabel[0]=fAwagsWidget->CurrentASIC_Label_1;
   fAwagsCurrentASICLabel[1]=fAwagsWidget->CurrentASIC_Label_2;
   fAwagsCurrentASICLabel[2]=fAwagsWidget->CurrentASIC_Label_3;
   fAwagsCurrentASICLabel[3]=fAwagsWidget->CurrentASIC_Label_4;

//   fAwagsCurrentASICLabel[4]=fAwagsWidget->CurrentASIC_Label_5;
//   fAwagsCurrentASICLabel[5]=fAwagsWidget->CurrentASIC_Label_6;
//   fAwagsCurrentASICLabel[6]=fAwagsWidget->CurrentASIC_Label_7;
//   fAwagsCurrentASICLabel[7]=fAwagsWidget->CurrentASIC_Label_8;




//   fAwagsIDScanLabel[0]=fAwagsWidget->AdressIDScan_Label_1;
//   fAwagsIDScanLabel[1]=fAwagsWidget->AdressIDScan_Label_2;
//   fAwagsIDScanLabel[2]=fAwagsWidget->AdressIDScan_Label_3;
//   fAwagsIDScanLabel[3]=fAwagsWidget->AdressIDScan_Label_4;
//   fAwagsIDScanLabel[4]=fAwagsWidget->AdressIDScan_Label_5;
//   fAwagsIDScanLabel[5]=fAwagsWidget->AdressIDScan_Label_6;
//   fAwagsIDScanLabel[6]=fAwagsWidget->AdressIDScan_Label_7;
//   fAwagsIDScanLabel[7]=fAwagsWidget->AdressIDScan_Label_8;
//
//   fAwagsGeneralCallLabel[0]=fAwagsWidget->AdressGeneralCall_Label_1;
//   fAwagsGeneralCallLabel[1]=fAwagsWidget->AdressGeneralCall_Label_2;
//   fAwagsGeneralCallLabel[2]=fAwagsWidget->AdressGeneralCall_Label_3;
//   fAwagsGeneralCallLabel[3]=fAwagsWidget->AdressGeneralCall_Label_4;
//   fAwagsGeneralCallLabel[4]=fAwagsWidget->AdressGeneralCall_Label_5;
//   fAwagsGeneralCallLabel[5]=fAwagsWidget->AdressGeneralCall_Label_6;
//   fAwagsGeneralCallLabel[6]=fAwagsWidget->AdressGeneralCall_Label_7;
//   fAwagsGeneralCallLabel[7]=fAwagsWidget->AdressGeneralCall_Label_8;



//   fAwagsReverseIDScanLabel[0]=fAwagsWidget->AdressReverseID_Label_1;
//   fAwagsReverseIDScanLabel[1]=fAwagsWidget->AdressReverseID_Label_2;
//   fAwagsReverseIDScanLabel[2]=fAwagsWidget->AdressReverseID_Label_3;
//   fAwagsReverseIDScanLabel[3]=fAwagsWidget->AdressReverseID_Label_4;
//   fAwagsReverseIDScanLabel[4]=fAwagsWidget->AdressReverseID_Label_5;
//   fAwagsReverseIDScanLabel[5]=fAwagsWidget->AdressReverseID_Label_6;
//   fAwagsReverseIDScanLabel[6]=fAwagsWidget->AdressReverseID_Label_7;
//   fAwagsReverseIDScanLabel[7]=fAwagsWidget->AdressReverseID_Label_8;
//
//
//   fAwagsRegisterTestLabel[0]=fAwagsWidget->AdressRegisterTest_Label_1;
//   fAwagsRegisterTestLabel[1]=fAwagsWidget->AdressRegisterTest_Label_2;
//   fAwagsRegisterTestLabel[2]=fAwagsWidget->AdressRegisterTest_Label_3;
//   fAwagsRegisterTestLabel[3]=fAwagsWidget->AdressRegisterTest_Label_4;
//   fAwagsRegisterTestLabel[4]=fAwagsWidget->AdressRegisterTest_Label_5;
//   fAwagsRegisterTestLabel[5]=fAwagsWidget->AdressRegisterTest_Label_6;
//   fAwagsRegisterTestLabel[6]=fAwagsWidget->AdressRegisterTest_Label_7;
//   fAwagsRegisterTestLabel[7]=fAwagsWidget->AdressRegisterTest_Label_8;


   fAwagsCurrentHVLabel[0]=fAwagsWidget->CurrentHV_Label_1;
   fAwagsCurrentHVLabel[1]=fAwagsWidget->CurrentHV_Label_2;
   fAwagsCurrentHVLabel[2]=fAwagsWidget->CurrentHV_Label_3;
   fAwagsCurrentHVLabel[3]=fAwagsWidget->CurrentHV_Label_4;
//   fAwagsCurrentHVLabel[4]=fAwagsWidget->CurrentHV_Label_5;
//   fAwagsCurrentHVLabel[5]=fAwagsWidget->CurrentHV_Label_6;
//   fAwagsCurrentHVLabel[6]=fAwagsWidget->CurrentHV_Label_7;
//   fAwagsCurrentHVLabel[7]=fAwagsWidget->CurrentHV_Label_8;


   fAwagsCurrentDiodeLabel[0]=fAwagsWidget->CurrentDiode_Label_1;
   fAwagsCurrentDiodeLabel[1]=fAwagsWidget->CurrentDiode_Label_2;
   fAwagsCurrentDiodeLabel[2]=fAwagsWidget->CurrentDiode_Label_3;
   fAwagsCurrentDiodeLabel[3]=fAwagsWidget->CurrentDiode_Label_4;
//   fAwagsCurrentDiodeLabel[4]=fAwagsWidget->CurrentDiode_Label_5;
//   fAwagsCurrentDiodeLabel[5]=fAwagsWidget->CurrentDiode_Label_6;
//   fAwagsCurrentDiodeLabel[6]=fAwagsWidget->CurrentDiode_Label_7;
//   fAwagsCurrentDiodeLabel[7]=fAwagsWidget->CurrentDiode_Label_8;

   fAwagsCurrentASICSpin[0]=fAwagsWidget->CurrentASIC_DoubleSpinBox_1;
   fAwagsCurrentASICSpin[1]=fAwagsWidget->CurrentASIC_DoubleSpinBox_2;
   fAwagsCurrentASICSpin[2]=fAwagsWidget->CurrentASIC_DoubleSpinBox_3;
   fAwagsCurrentASICSpin[3]=fAwagsWidget->CurrentASIC_DoubleSpinBox_4;
//   fAwagsCurrentASICSpin[4]=fAwagsWidget->CurrentASIC_DoubleSpinBox_5;
//   fAwagsCurrentASICSpin[5]=fAwagsWidget->CurrentASIC_DoubleSpinBox_6;
//   fAwagsCurrentASICSpin[6]=fAwagsWidget->CurrentASIC_DoubleSpinBox_7;
//   fAwagsCurrentASICSpin[7]=fAwagsWidget->CurrentASIC_DoubleSpinBox_8;

   fAwagsCurrentHVSpin[0]=fAwagsWidget->CurrentHV_DoubleSpinBox_1;
   fAwagsCurrentHVSpin[1]=fAwagsWidget->CurrentHV_DoubleSpinBox_2;
   fAwagsCurrentHVSpin[2]=fAwagsWidget->CurrentHV_DoubleSpinBox_3;
   fAwagsCurrentHVSpin[3]=fAwagsWidget->CurrentHV_DoubleSpinBox_4;
//   fAwagsCurrentHVSpin[4]=fAwagsWidget->CurrentHV_DoubleSpinBox_5;
//   fAwagsCurrentHVSpin[5]=fAwagsWidget->CurrentHV_DoubleSpinBox_6;
//   fAwagsCurrentHVSpin[6]=fAwagsWidget->CurrentHV_DoubleSpinBox_7;
//   fAwagsCurrentHVSpin[7]=fAwagsWidget->CurrentHV_DoubleSpinBox_8;

   fAwagsCurrentDiodeSpin[0]=fAwagsWidget->CurrentDiode_DoubleSpinBox_1;
   fAwagsCurrentDiodeSpin[1]=fAwagsWidget->CurrentDiode_DoubleSpinBox_2;
   fAwagsCurrentDiodeSpin[2]=fAwagsWidget->CurrentDiode_DoubleSpinBox_3;
   fAwagsCurrentDiodeSpin[3]=fAwagsWidget->CurrentDiode_DoubleSpinBox_4;
//   fAwagsCurrentDiodeSpin[4]=fAwagsWidget->CurrentDiode_DoubleSpinBox_5;
//   fAwagsCurrentDiodeSpin[5]=fAwagsWidget->CurrentDiode_DoubleSpinBox_6;
//   fAwagsCurrentDiodeSpin[6]=fAwagsWidget->CurrentDiode_DoubleSpinBox_7;
//   fAwagsCurrentDiodeSpin[7]=fAwagsWidget->CurrentDiode_DoubleSpinBox_8;





  fPlotWidget[0] = fAwagsWidget->PlotWidget_0;
  fPlotWidget[1] = fAwagsWidget->PlotWidget_1;
  fPlotWidget[2] = fAwagsWidget->PlotWidget_2;
  fPlotWidget[3] = fAwagsWidget->PlotWidget_3;
  fPlotWidget[4] = fAwagsWidget->PlotWidget_4;
  fPlotWidget[5] = fAwagsWidget->PlotWidget_5;
  fPlotWidget[6] = fAwagsWidget->PlotWidget_6;
  fPlotWidget[7] = fAwagsWidget->PlotWidget_7;
  fPlotWidget[8] = fAwagsWidget->PlotWidget_8;
  fPlotWidget[9] = fAwagsWidget->PlotWidget_9;
  fPlotWidget[10] = fAwagsWidget->PlotWidget_10;
  fPlotWidget[11] = fAwagsWidget->PlotWidget_11;
  fPlotWidget[12] = fAwagsWidget->PlotWidget_12;
  fPlotWidget[13] = fAwagsWidget->PlotWidget_13;
  fPlotWidget[14] = fAwagsWidget->PlotWidget_14;
  fPlotWidget[15] = fAwagsWidget->PlotWidget_15;

  // labels for plot area:
  for (int i = 0; i < 16; ++i)
  {
    fPlotWidget[i]->axis (KPlotWidget::BottomAxis)->setLabel ("Time (#samples)");
    fPlotWidget[i]->axis (KPlotWidget::LeftAxis)->setLabel ("ADC value ");
  }
//
//  // timers for frequent test pulse:
//  fPulserTimer = new QTimer (this);
//  QObject::connect (fPulserTimer, SIGNAL (timeout ()), this, SLOT (PulserTimeout ()));

//  fDisplayTimer = new QTimer (this);
//  fDisplayTimer->setInterval (500);
//  QObject::connect (fDisplayTimer, SIGNAL (timeout ()), this, SLOT (PulserDisplayTimeout ()));

  // benchmark testing procedure related:
  fAwagsWidget->BenchmarkButtonBox->button (QDialogButtonBox::Apply)->setDefault (true);

  fSequencerTimer = new QTimer (this);
  fSequencerTimer->setInterval (20);
  QObject::connect (fSequencerTimer, SIGNAL (timeout ()), this, SLOT (BenchmarkTimerCallback ()));


  fBenchmark.SetOwner (this);
  //fBenchmark.LoadReferenceValues (QString ("default.apf"));

  ReadSettings();


  show ();

  // start with preferred situation:
  ShowBtn_clicked ();
  //checkBox_AA->setChecked (true); // handled by settings now

}

AwagsGui::~AwagsGui ()
{

}




void AwagsGui::ApplyFileConfig(int )
{
    GosipGui::ApplyFileConfig(900); // adjust bus wait time to 900 us
}


int AwagsGui::OpenTestFile (const QString& fname)
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
  WriteConfigFile (QString ("# Awags characterization test file saved on ") + timestring + QString ("\n"));
  return 0;
}

int AwagsGui::CloseTestFile ()
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

int AwagsGui::WriteTestFile (const QString& text)
{
  if (fTestFile == NULL)
    return -1;
  if (fprintf (fTestFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}






void AwagsGui::AutoAdjust ()
{
  if (!AssertChainConfigured ())
    return;
  QString targetstring = fAwagsWidget->ADCAdjustValue->text ();
  unsigned targetvalue = targetstring.toUInt (0, fNumberBase);
  //std::cout <<"string="<<targetstring.toLatin1 ().constData ()<< ", targetvalue="<< targetvalue<< std::endl;
  for (int channel = 0; channel < 16; ++channel)
  {
    if (fBaselineBoxes[channel]->isChecked ())
    {
       AutoAdjustChannel (channel, targetvalue);
//      int dac = AdjustBaseline (channel, targetvalue);
//      fDACSpinBoxes[channel]->setValue (dac);
//      AutoApplyRefresh (channel, dac);    // once again apply dac settings to immediately see the baseline on gui
//      printm ("--- Auto adjusted baselines of sfp:%d board:%d channel:%d to value:%d =>%d permille DAC", fSFP, fSlave,
//          channel, targetvalue, dac);
    }
  }
}

void AwagsGui::AutoAdjustChannel (int channel, unsigned targetvalue)
{
  int dac = AdjustBaseline (channel, targetvalue);
       fDACSpinBoxes[channel]->setValue (dac);
       AutoApplyRefresh (channel, dac);    // once again apply dac settings to immediately see the baseline on gui
       printm ("--- Auto adjusted baselines of sfp:%d board:%d channel:%d to value:%d =>%d permille DAC", fSFP, fSlave,
           channel, targetvalue, dac);

}


int AwagsGui::AdjustBaseline (int channel, int adctarget)
{
  int dac = 500;    // dac setup in per mille here, start in the middle
  int dacstep = 250;
  int validdac = -1;
  int adc = 0;
  int escapecounter = 10;
  bool upwards = true;    // scan direction up or down
  bool changed = false;    // do we have changed scan direction?
  bool initial = true;    // supress evaluation of scan direction at first cycle
  //std::cout << "AwagsGui::AdjustBaseline of channel "<<channel<<" to "<<adctarget<< std::endl;

  double resolution = 1.0 / AWAGS_DAC_MAXVALUE * 0x3FFF / 2;    // for 14 bit febex ADC

  do
  {
    adc = autoApply (channel, dac);    // this gives already mean value of 3 adc samples
    if (adc < 0)
      break;    // avoid broadcast
    validdac = dac;
    //std::cout << "AwagsGui::AdjustBaseline after autoApply of dac:"<<dac<<" gets adc:"<<adc<<", resolution:"<<resolution<< std::endl;
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
  //std::cout << "   AwagsGui::AdjustBaseline after loop dac:"<<validdac<<" adc:"<<adc<<", resolution:"<<resolution<< std::endl;
  return validdac;
}


void AwagsGui::CalibrateSelectedADCs ()
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

int AwagsGui::CalibrateADC (int channel)
{

  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  printm ("Calibrate baseline slider for ADC channel %d ...", channel);
  int awags = 0, dac = 0;
  theSetup->EvaluateDACIndices (channel, awags, dac);

  // 2017: check if awags is present before doing some calibration procedures:
  if(!theSetup->IsAwagsPresent(awags))
  {
    printm ("Skipping not connected AWAGS chip of ID %d", theSetup->GetAwagsID(awags));
    return -1;
  }

  // DO NOT first autocalibrate DAC that belongs to selected channel
  // we decouple chip autocalibration from channel calibration curve now
  //DoAutoCalibrate(awags);

  // measure slope of DAC kennlinie by differential variation:
  int gain = theSetup->GetGain (awags, dac);
  int valDAC = theSetup->GetDACValue (awags, dac);    // current value after automatic DAC calibration
  int valADC = AcquireBaselineSample (channel);    // fetch a sample

  EnableI2C ();
#ifdef  AWAGS_DAC_LOCALCALIB
  int deltaDAC=AWAGS_DAC_DELTACALIB;

  int valDAC_upper=valDAC+deltaDAC;

  // now do variation and measure new adc value:

  WriteDAC_AwagsI2c (awags, dac, valDAC_upper);
  int valADC_upper=AcquireBaselineSample(channel);
  int deltaADC=valADC_upper-valADC;
#endif

  int valADC_max = 0, valADC_min = 0, valDAC_min = 0, valDAC_max = 0, valADC_sample = 0;

//#ifdef AWAGS_GAIN1_INVERTED
//  if (((gain != 1) && theSetup->IsBaselineInverted()) || ((gain == 1) && !theSetup->IsBaselineInverted()))
//#else
//  if (theSetup->IsBaselineInverted())
//#endif
//  {
//    // special situation for gain 1 or new panda boards: slope is inverted, need to do different procedure:
//    // get minimum ADC value by setting DAC to min:
//    WriteDAC_AwagsI2c (awags, dac, 0);
//    valADC_min = AcquireBaselineSample (channel);
//
//    int valDAC_min = 0;
//    int valADC_deltasaturation = AWAGS_ADC_SATURATIONDELTA;
//
//    // shift DAC up ADCmax changes significantly. this gives effective DAC min
//    for (valDAC_min = 0; valDAC_min < AWAGS_DAC_MAXVALUE; valDAC_min += AWAGS_DAC_DELTASTEP)
//    {
//      WriteDAC_AwagsI2c (awags, dac, valDAC_min);
//      int samp = AcquireBaselineSample (channel);
//      if (samp - valADC_min > valADC_deltasaturation)
//        break;
//      valADC_sample = samp;
//      //std::cout <<"Searching effective maximum  DAC:"<<valDAC_max<<", ADC:"<<valADC_sample<< std::endl;
//    }
//    valDAC_min -= AWAGS_DAC_DELTASTEP;    // rewind to point that is still in pedestal region
//
//    //shift DAC farther upwards until we find ADC saturation:
//    valDAC_max = 0;
//    int valADC_step = 0;
//    for (valDAC_max = valDAC_min; valDAC_max < AWAGS_DAC_MAXVALUE; valDAC_max += AWAGS_DAC_DELTASTEP)
//    {
//      WriteDAC_AwagsI2c (awags, dac, valDAC_max);
//      valADC_step = AcquireBaselineSample (channel);
//      if (valADC_step >= AWAGS_ADC_MAXSATURATION)
//        break;
//      //std::cout <<"Searching ADC saturation: DAC:"<<valDAC_max<<", ADC:"<<valADC_step<< std::endl;
//
//    }
//    printm ("found ADC_min=%d, DAC_min=%d, DAC_max=%d", valADC_min, valDAC_min, valDAC_max);
//    theSetup->SetDACmin (gain, channel, valDAC_min);
//    theSetup->SetDACmax (gain, channel, valDAC_max);
//    theSetup->SetADCmin (gain, channel, valADC_min);
//
//    // linear calibration only in non saturated ADC range:
//    int deltaDAC = valDAC_max - valDAC_min;
//    int deltaADC = AWAGS_ADC_MAXSATURATION - valADC_min;
//    theSetup->EvaluateCalibration (gain, channel, deltaDAC, deltaADC, valDAC_min, valADC_sample);
//  }
//  else

  {
    // evaluate range boundaries:
    // get minimum ADC value by setting DAC to max:
    WriteDAC_AwagsI2c (awags, dac, AWAGS_DAC_MAXVALUE);
    int valADC_min = AcquireBaselineSample (channel);

    valDAC_max = AWAGS_DAC_MAXVALUE;
    //int valADC_sample;
    int valADC_deltasaturation = AWAGS_ADC_SATURATIONDELTA;

    // shift DAC down until ADCmin changes significantly. this gives effective DAC max
    for (valDAC_max = AWAGS_DAC_MAXVALUE; valDAC_max > 0; valDAC_max -= AWAGS_DAC_DELTASTEP)
    {
      WriteDAC_AwagsI2c (awags, dac, valDAC_max);
      int samp = AcquireBaselineSample (channel);
      if (samp - valADC_min > valADC_deltasaturation)
        break;
      valADC_sample = samp;
      //std::cout <<"Searching effective maximum  DAC:"<<valDAC_max<<", ADC:"<<valADC_sample<< std::endl;
    }
    valDAC_max += AWAGS_DAC_DELTASTEP;    // rewind to point that is still in pedestal region

    //shift DAC farther downwards until we find ADC saturation:
    valDAC_min = 0;
    int valADC_step = 0;
    for (valDAC_min = valDAC_max; valDAC_min > 0; valDAC_min -= AWAGS_DAC_DELTASTEP)
    {
      WriteDAC_AwagsI2c (awags, dac, valDAC_min);
      valADC_step = AcquireBaselineSample (channel);
      if (valADC_step >= AWAGS_ADC_MAXSATURATION)
        break;
      //std::cout <<"Searching ADC saturation: DAC:"<<valDAC_min<<", ADC:"<<valADC_step<< std::endl;

    }
    printm ("found ADC_min=%d, DAC_min=%d, DAC_max=%d", valADC_min, valDAC_min, valDAC_max);
    theSetup->SetDACmin (gain, channel, valDAC_min);
    theSetup->SetDACmax (gain, channel, valDAC_max);
    theSetup->SetADCmin (gain, channel, valADC_min);

    // linear calibration only in non saturated ADC range:
    int deltaDAC = valDAC_max - valDAC_min;
    int deltaADC = valADC_min - AWAGS_ADC_MAXSATURATION;
    theSetup->EvaluateCalibration (gain, channel, deltaDAC, deltaADC, valDAC_max, valADC_sample);

  }

  // shift back to middle of calibration:
  WriteDAC_AwagsI2c (awags, dac, valDAC);
  DisableI2C ();
  printm ("--- Calibrated DAC->ADC slider for sfp:%d board:%d channel:%d awags:%d dac:%d -", fSFP, fSlave, channel,
      awags, dac);

#ifdef  AWAGS_DAC_LOCALCALIB
  theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC, valADC);
#else

// trial and error of slider calibration in the following:
//
// in this mode, we make linearcalibration over full range
//  int deltaDAC=AWAGS_DAC_MAXVALUE-valDAC_min;
//       int deltaADC=valADC_min - AWAGS_ADC_MAXSATURATION;
//       theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, AWAGS_DAC_MAXVALUE, valADC_min);

// linear calibration only in non saturated ADC range:
//      int deltaDAC=valDAC_max-valDAC_min;
//      int deltaADC=valADC_min - AWAGS_ADC_MAXSATURATION;
//      theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC_max, valADC_sample);

// test: semi-range linear calibration
//     int deltaDAC=AWAGS_DAC_MAXVALUE-valDAC;
//      int deltaADC=valADC_min - valADC;
//      theSetup->EvaluateCalibration(gain, channel, deltaDAC, deltaADC, valDAC, valADC);
///////////
//      int deltaDAC=valDAC-valDAC_min;
//      int deltaADC=AWAGS_ADC_MAXSATURATION - valADC;
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


void AwagsGui::CalibrateResetSelectedADCs ()
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

int AwagsGui::CalibrateResetADC (int channel)
{
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  // TODO: first autocalibrate DAC that belongs to selected channel
  int awags = 0, dac = 0;
  theSetup->EvaluateDACIndices (channel, awags, dac);
  int gain = theSetup->GetGain (awags, dac);
  theSetup->ResetCalibration (gain, channel);
  printm ("--- Reset Calibration of DAC->ADC slider for sfp:%d board:%d channel:%d awags:%d dac:%d", fSFP, fSlave,
      channel, awags, dac);
  return 0;

}

int AwagsGui::AcquireSample(int channel, int peakfinderpolarity)
{

  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  bool usemonitorport = fAwagsWidget->MonitorRadioButton->isChecked ();
  double val = 0;
  theSetup->ResetADCSample (channel);

  if (usemonitorport)
  {
    printm ("AcquiringSample of ADC channel %d from monitoring port.", channel);
    for (int i = 0; i < AWAGS_ADC_SAMPLEVALUES; ++i)
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
      if ((cursor += 2) >= AWAGS_MBS_TRACELEN)
        break;
    }
       int i = 0;
       for (i = 0; i < AWAGS_MBS_TRACELEN; ++i)
       {
         if((fData[cursor] & 0xBB00) == 0xBB00) break; // JAM2022: leave loop if trace is smaller
         double value = (fData[cursor] & 0x3FFF);
         cursor++;
         //std::cout <<"got value"<<value<< "at position "<< i <<", cursor="<<cursor<<", sum="<<sum << std::endl;
         theSetup->AddADCSample (channel, value);
       }
    //std::cout << "Filled "<<i<< "adc samples from mbs trace up to position #"<< cursor<< std::endl;
  }
  EvaluateBaseline(channel);
  //FindPeaks(channel, peakfinderpolarity);
  RefreshLastADCSample (channel);
  return 0;
}


void AwagsGui::EvaluateBaseline(int channel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  bool ok=false;
  int startbase=fAwagsWidget->BaselineLowerLineEdit->text().toInt(&ok,fNumberBase);
  int stopbase=fAwagsWidget->BaselineUpperLineEdit->text().toInt(&ok,fNumberBase);

 //std::cout<< "EvaluateBaseline for channel "<<channel<<" has start:"<<startbase<<", stop="<<stopbase << std::endl;

  theSetup->EvaluateBaseline(channel,startbase, stopbase);
}


//void AwagsGui::FindPeaks (int channel, int usepolarity)
//{
//  theSetup_GET_FOR_SLAVE(BoardSetup);
//  double deltaratio = fAwagsWidget->PeakDeltaDoubleSpinBox->value () / 100.0;
//  double fall = fAwagsWidget->PeakFallDoubleSpinBox->value ();
//  bool negative = false;
//  if(usepolarity ==0)
//    negative = true;
//  else if (usepolarity == 1)
//    negative = false;
//  else
//    negative=fAwagsWidget->PeakNegaitveCheckBox->isChecked();
//  theSetup->EvaluatePeaks (channel, deltaratio, fall, negative);
//
//}
//
//void AwagsGui::SetPeakfinderPolarityNegative(bool on)
//{
//  fAwagsWidget->PeakNegaitveCheckBox->setChecked(on);
//
//}




void AwagsGui::AcquireSelectedSamples ()
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



//void AwagsGui::ZoomSampleToPeak(int channel, int peak)
//{
//  theSetup_GET_FOR_SLAVE(BoardSetup);
//  double nmax=theSetup->GetSamplePeakHeight(channel,peak);
//  double pos=theSetup->GetSamplePeakPosition(channel,peak);
//  double window=theSetup->GetSamplePeaksPositionDelta(channel);
//  double headroom=theSetup->GetSamplePeaksHeightDelta(channel);
//  //bool negative=theSetup->IsSamplePeaksNegative(channel);
//  double xmin=pos-window;
//  double xmax=pos+window;
//  double ymin=nmax-headroom*0.8;
//  double ymax=nmax+headroom*0.8;
//  fPlotWidget[channel]->setLimits (xmin, xmax, ymin, ymax);
//  fPlotWidget[channel]->update ();
//
//}


//void AwagsGui::RefreshSampleMaxima (int febexchannel)
//{
//  theSetup_GET_FOR_SLAVE(BoardSetup);
//  QString text;
//  QString pre;
//  fNumberBase == 16 ? pre = "0x" : pre = "";
//  int numpeaks = theSetup->NumSamplePeaks (febexchannel);
//  for (int i = 0; i < AWAGS_ADC_NUMMAXIMA; ++i)
//  {
//    uint16_t height = 0;
//    int pos = 0;
//    if (i < numpeaks)
//    {
//      height = theSetup->GetSamplePeakHeight (febexchannel, i);
//      pos = theSetup->GetSamplePeakPosition (febexchannel, i);
//    }
//    QTableWidgetItem * pitem = fAwagsWidget->MaximaTableWidget->item (i, 0);
//    QTableWidgetItem * hitem = fAwagsWidget->MaximaTableWidget->item (i, 1);
//    if(pitem) pitem->setText (pre + text.setNum (pos, fNumberBase));
//    if(hitem )hitem->setText (pre + text.setNum (height, fNumberBase));
//  }
//
//}
//




int AwagsGui::ShowSample (int channel, bool benchmarkdisplay)
{
//  std::cout <<"ShowSample for channel:"<<channel<< ". benchmarkdisplay="<<benchmarkdisplay<< std::endl;
//  std::cout<< " --- ShowSample with NOP"<<std::endl;
//  return 0; // JAM2020 DEBUG
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  //theSetup->ShowADCSample(channel); // todo: dump sample on different knob

  KPlotWidget* canvas= fPlotWidget[channel];
  if (benchmarkdisplay)
    canvas = fAwagsWidget->BenchmarkPlotwidget;
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
  // only most recent kplot
  //canvas->setAutoDeletePlotObjects(true);
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
    //RefreshSampleMaxima(channel);
  }

  return 0;
}



void AwagsGui::ShowSelectedSamples ()
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
  fAwagsWidget->AwagsTabWidget->setCurrentIndex (5);
  // TODO JAM2019- bring subwindow to front instead
  fAwagsWidget->PlotTabWidget->setCurrentIndex (lastchecked);
}

/** Clear display of benchmark DAC curve*/
void AwagsGui::ResetBenchmarkCurve ()
{
//  std::cout<< "ResetBenchmarkCurve. with NOP"<<std::endl;
//  return; // JAM2020 DEBUG
  fPlotMinDac = 0;
  fPlotMaxDac = AWAGS_DAC_MAXVALUE;
  fPlotMinAdc = 0;
  fPlotMaxAdc = AWAGS_ADC_MAXVALUE;

  KPlotWidget* canvas = fAwagsWidget->BenchmarkPlotwidget;
  // only for most recent versions of kplotwidget:
  //canvas->setAutoDeletePlotObjects(true);
  canvas->resetPlot ();
  // labels for plot area:
  canvas->setAntialiasing (true);
  canvas->axis (KPlotWidget::BottomAxis)->setLabel ("DAC value");
  canvas->axis (KPlotWidget::LeftAxis)->setLabel ("ADC value ");
  canvas->setLimits (0, AWAGS_DAC_MAXVALUE, 0, AWAGS_ADC_MAXVALUE);
  canvas->update ();

}




void AwagsGui::ShowLimitsCurve (int gain, int awags, int dac)
{
//  std::cout<< "ShowLimitsCurve with NOP"<<std::endl;
//  return; // JAM2020 DEBUG
  QColor col = Qt::red;
  KPlotObject *upper = new KPlotObject (col, KPlotObject::Lines, 3);
  KPlotObject *lower = new KPlotObject (col, KPlotObject::Lines, 3);
 // std::cout<<"ShowLimitsCurve: gain:"<<gain<<", awags:"<<awags<<", dac:"<<dac << std::endl;

  theSetup_GET_FOR_SLAVE(BoardSetup);
  AwagsTestResults& theResults = theSetup->AccessTestResults (gain, awags);
  AwagsTestResults& reference = fBenchmark.GetReferenceValues (gain);

  double tolerance = fAwagsWidget->ToleranceSpinBox->value () / 100.0;
  bool relativeMode=fAwagsWidget->RelativeComparisonBox->isChecked();
  int ashift=0, dshift=0;
  if(relativeMode)
  {
      // get coordinates of sample and shift reference onto autocalibration centre:
      int autoix = (AWAGS_DAC_CURVEPOINTS/2 -1); // should be 11 from 24
      DacSample middleSample (0, 0);
      theResults.AccessDacSample (middleSample, dac, autoix);
      DacSample middleReference (0, 0);
      reference.AccessDacSample (middleReference, dac, autoix);
      ashift= (int) middleSample.GetADCValue () - (int) middleReference.GetADCValue ();
      dshift= (int) middleSample.GetDACValue() - (int) middleReference.GetDACValue();
      //std::cout<<"ShowLimitsCurve for awags:"<<awags<<", dac:"<<dac<<" has adc shift:"<<(int) ashift<<", dac shift:"<< (int) dshift << std::endl;
  }

  for (int i = 0; i < AWAGS_DAC_CURVEPOINTS; ++i)
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
    if (fPlotMaxDac == AWAGS_DAC_MAXVALUE || dval > fPlotMaxDac)
      fPlotMaxDac = dval;
    if (fPlotMinAdc == 0 || adclow < fPlotMinAdc)
      fPlotMinAdc = adclow;
    if (fPlotMaxAdc == AWAGS_DAC_MAXVALUE || adcup > fPlotMaxAdc)
      fPlotMaxAdc = adcup;

    //std::cout<<"ShowLimitsCurve: i:"<<i<<", dacval:"<<dval<<"up:"<<adcup<<", lo:"<<adclow << std::endl;

  }
  fAwagsWidget->BenchmarkPlotwidget->addPlotObject (upper);
  fAwagsWidget->BenchmarkPlotwidget->addPlotObject (lower);
  fAwagsWidget->BenchmarkPlotwidget->setLimits (fPlotMinDac, fPlotMaxDac, fPlotMinAdc, fPlotMaxAdc);
  fAwagsWidget->BenchmarkPlotwidget->update ();
}

void AwagsGui::ShowBenchmarkCurve (int gain, int awags, int dac)
{
//  std::cout<< "ShowBenchmarkCurve with NOP for gain:"<<gain<<", awags:"<<awags<<", dac:"<<dac<<std::endl;
//    return; // JAM2020 DEBUG

   theSetup_GET_FOR_SLAVE(BoardSetup);

  QColor col;
  KPlotObject::PointStyle pstyle = KPlotObject::Circle;
  switch (awags)
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
  AwagsTestResults& theResults = theSetup->AccessTestResults (gain, awags);

  QString label = QString ("gain:%1 awags:%2 dac:%3").arg (gain).arg (awags).arg (dac);
  for (int i = 0; i < AWAGS_DAC_CURVEPOINTS; ++i)
  {
    DacSample point (0, 0);
    theResults.AccessDacSample (point, dac, i);    // if this fails, point is just not touched -> default 0 values are saved
    uint16_t dval = point.GetDACValue ();
    uint16_t aval = point.GetADCValue ();
    // this is for zooming:
    if (fPlotMinDac == 0 || dval < fPlotMinDac)
      fPlotMinDac = dval;
    if (fPlotMaxDac == AWAGS_DAC_MAXVALUE || dval > fPlotMaxDac)
      fPlotMaxDac = dval;
    if (fPlotMinAdc == 0 || aval < fPlotMinAdc)
      fPlotMinAdc = aval;
    if (fPlotMaxAdc == AWAGS_DAC_MAXVALUE || aval > fPlotMaxAdc)
      fPlotMaxAdc = aval;

    if (i == AWAGS_DAC_CURVEPOINTS / 2)
      curveplot->addPoint (dval, aval, label);
    else
      curveplot->addPoint (dval, aval);

    //std::cout<<"ShowBenchmarkCurve: i:"<<i<<", dacval="<< (int) dval<<", adcval="<< (int) aval << std::endl;
  }

  // add it to the plot area
  fAwagsWidget->BenchmarkPlotwidget->addPlotObject (curveplot);

  //if(gain!=1)
  fAwagsWidget->BenchmarkPlotwidget->setLimits (fPlotMinDac, fPlotMaxDac, fPlotMinAdc, fPlotMaxAdc);
  //else
  //  BenchmarkPlotwidget->setLimits (0, AWAGS_DAC_MAXVALUE, 0, AWAGS_ADC_MAXVALUE);
 //std::cout<<"ShowBenchmarkCurve limits: dmin:"<<fPlotMinDac<<", dmax:"<<fPlotMaxDac<<", amin:"<< fPlotMinAdc<<", amax:"<<fPlotMaxAdc<< std::endl;

  fAwagsWidget->BenchmarkPlotwidget->update ();

}


void AwagsGui::ZoomSample (int channel)
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

void AwagsGui::UnzoomSample (int channel)
{
  //std::cout <<"UnzoomSample for channel"<< channel<< std::endl;
  theSetup_GET_FOR_SLAVE(BoardSetup);
  double xmax=theSetup->GetADCSampleLength(channel);
  fPlotWidget[channel]->setLimits (0, xmax, 0.0, 17000);
  fPlotWidget[channel]->update ();
}
//

void AwagsGui::RefreshLastADCSample (int febexchannel)
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


void AwagsGui::DumpSlave ()
{
  DumpADCs();
  DumpCalibrations ();    // later put to separate button
}


void AwagsGui::DumpADCs ()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if (!AssertChainConfigured ())
    return;

  printm ("SFP %d DEV:%d :)", fSFP, fSlave);
  for (int adc = 0; adc < AWAGS_ADC_NUMADC; ++adc)
  {
    for (int chan = 0; chan < AWAGS_ADC_NUMCHAN; ++chan)
    {
      int val = ReadADC_Awags (adc, chan);
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

void AwagsGui::DumpCalibrations ()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if (!AssertChainConfigured ())
    return;
  theSetup_GET_FOR_SLAVE(BoardSetup);
  printm ("SFP %d DEV:%d : Dump calibration)", fSFP, fSlave);
  int awags = 0, dac = 0;
  for (int febchan = 0; febchan < AWAGS_ADC_CHANNELS; ++febchan)
  {
    theSetup->EvaluateDACIndices (febchan, awags, dac);
    int gain = theSetup->GetGain (awags, dac);
    theSetup->DumpCalibration (gain, febchan);
  }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////7
//////////////////////////////////////////////////////////////////////////////////////////////////////

void AwagsGui::AutoApplySwitch ()
{
  EvaluateIOSwitch ();
  SetIOSwitch ();
}


//void AwagsGui::AutoApplyPulser (int awags)
//{
//  EvaluatePulser (awags);
//  SetPulser (awags);
//}


void AwagsGui::AutoApplyGain (int awags)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  EvaluateGain (awags);
  //std::cout << "AutoApplyGain awags=" << awags << ", channel=" << channel << ", lowgain:"
  //    << theSetup->GetLowGain (awags, channel) << std::endl;
  SetGain (awags, theSetup->GetGain (awags,0));
}


void AwagsGui::AutoApplyDAC (int awags, int dac, int val)
{
  // keep setup structure always consistent:
  theSetup_GET_FOR_SLAVE(BoardSetup);
  // this one is only called for dac index 0, since there are no other sliders on gui
  // we have to keep track for all existing dac indices in setup though:
  for(int d=0; d<AWAGS_NUMDACS; ++d)
  {
    theSetup->SetDACValue (awags, d, val);
    if (theSetup->IsAwagsPresent (awags))
    {
      // only access chip if it is connected
      WriteDAC_AwagsI2c (awags, d, theSetup->GetDACValue (awags, d)); // internally will address only dac 0
      RefreshADC_Awags (awags, d); // will update all febex channels connected to dac 0
    }
  }
}


void AwagsGui::AutoApplyPower(int awags, int state)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  //if(!theSetup->IsUsePandaTestBoard()) return;
  //std::cout << "AutoApplyPower awags=" << awags << ", state=" << state << std::endl;
  theSetup->SetAwagsPowered(awags, (state>0));
  SetDefaultIOConfig();
  // check if we still have connection to the awags and display non connected ones:
  GetRegisters();
  RefreshView();
  // todo: only use required calls for this?
}


void AwagsGui::SetDefaultIOConfig()
{
    theSetup_GET_FOR_SLAVE(BoardSetup);
  // here we have to keep all other awags power states as indicated in setup!
    int powermask=0;
    for(int a=0; a<AWAGS_NUMCHIPS; ++a)
    {
      if(theSetup->HasAwagsPower(a)) powermask |= (1 << a);
    }
    SetPower(powermask, false); // TODO: new functions here

}


void AwagsGui::AutoApplyRefresh (int channel, int dac)
{
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  int Adc = autoApply (channel, dac);
  fADCLineEdit[channel]->setText (pre + text.setNum (Adc, fNumberBase));
  RefreshStatus ();
}

int AwagsGui::autoApply (int channel, int permillevalue)

{
  int awags = 0, dac = 0;
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  theSetup->EvaluateDACIndices (channel, awags, dac);
  if(!theSetup->IsAwagsPresent(awags)) return -1; // exlude not connected dacs


  int gain = theSetup->GetGain (awags, dac);
  //int value=theSetup->EvaluateDACvalueAbsolute(permillevalue,-1,gain);
  int value = theSetup->EvaluateDACvalueAbsolute (permillevalue, channel, gain);

  theSetup->SetDACValue (awags, dac, value);

  EnableI2C ();
  WriteDAC_AwagsI2c (awags, dac, theSetup->GetDACValue (awags, dac));
  DisableI2C ();

  RefreshDAC (awags);    //  immediately update DAC sliders when shifting baseline!
  if (!AssertNoBroadcast ())
    return -1;
  int Adc = AcquireBaselineSample (channel);
  //std::cout << "AwagsGui::autoApply channel="<<channel<<", permille="<<permillevalue<<", awags="<<awags<<", dac="<<dac<<", DACvalue="<<value<<", ADC="<<Adc << std::endl;

  return Adc;

}

int AwagsGui::AcquireBaselineSample (uint8_t febexchan, int numsamples)
{
  if (febexchan >= AWAGS_ADC_NUMADC * AWAGS_ADC_NUMCHAN)
    return -1;
  int adcchip = febexchan / AWAGS_ADC_NUMCHAN;
  int adcchannel = febexchan - adcchip * AWAGS_ADC_NUMCHAN;
  int Adc = 0;
  if (numsamples <= 0)
    numsamples = AWAGS_ADC_BASELINESAMPLES;
  for (int t = 0; t < numsamples; ++t)
  {
    Adc += ReadADC_Awags (adcchip, adcchannel);
  }
  Adc = Adc / numsamples;
  return Adc;
}

int AwagsGui::AcquireMbsSample (uint8_t febexchan)
{
  if (febexchan >= AWAGS_ADC_NUMADC * AWAGS_ADC_NUMCHAN)
    return -1;
  // issue read request:
  EnableI2C ();
  WriteGosip (fSFP, fSlave, AWAGS_ADC_DAQBUFFER_REQ_PORT, 0x80);
  int readaddress = AWAGS_ADC_DAQBUFFER_BASE * (febexchan + 1);
  for (int cursor = 0; cursor < AWAGS_MBS_TRACELEN; cursor += 2)
  {
    int value = ReadGosip (fSFP, fSlave, readaddress);
    fData[cursor] = (value >> 16) & 0xFFFF;
    fData[cursor + 1] = (value & 0xFFFF);    // check the order here?
//          if(cursor<10 || AWAGS_MBS_TRACELEN -cursor < 10)
//            printf("AcquireMbsSample val=0x%x dat[%d]=0x%x dat[%d]=0x%x\n",value,cursor,fData[cursor],cursor+1, fData[cursor+1]);
    readaddress += 4;
  }
  DisableI2C ();
  return 0;
}

void AwagsGui::RefreshDAC (int awags)
{
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
   theSetup_GET_FOR_SLAVE(BoardSetup);
//  for (int dac = 0; dac < AWAGS_NUMDACS; ++dac)
//    {
    int dac=0; // JAM22 we have only one real dac, but 4 entries in setup
    int value = theSetup->GetDACValue (awags, dac);
    fDACSlider[awags][dac]->setValue (value);
    fDACLineEdit[awags][dac]->setText (pre + text.setNum (value, fNumberBase));
//  }

  // 2017: disable DAC widget if DAC has not responded last request
  //fAwagsPulseGroup[awags]->setEnabled(theSetup->IsAwagsPresent(awags));
  fAwagsDACGroup[awags]->setEnabled(theSetup->IsAwagsPresent(awags));
  fAwagsGainGroup[awags]->setEnabled(theSetup->IsAwagsPresent(awags));





  //change color of adress label depending on chip presence:
       // red: power off
       // yellow: not present
       // gree: power on and present
  // default: green
  //QString labelstate= " <html><head/><body><p> <span style=\" font-weight:600; color:#00cc00;\"> ON </span></p></body></html>" ;
  // if not responding and power: yellow
  QString labelstate="ON ";
  AwagsTextColor_t color=awags_green_background;
  if(!theSetup->IsAwagsPresent(awags))
    {
      color=awags_yellow_background;
      labelstate="NC ";
    }
    //labelstate=" <html><head/><body><p> <span style=\" font-weight:600; color:#cccc00;\"> NC </span></p></body></html>" ;

    // if no power: red
  if(!theSetup->HasAwagsPower(awags))
  {
    //labelstate=" <html><head/><body><p> <span style=\" font-weight:600; color:#ff0000;\"> OFF</span></p></body></html>" ;
    color=awags_red_background;
    labelstate="OFF";
  }
    //fAwagsPowerLabel[awags]->setText(labelstate);

  RefreshColouredLabel(fAwagsPowerLabel[awags],labelstate,color);


  fAwagsAddressLabel[awags]->setEnabled(theSetup->IsAwagsPresent(awags));








}


bool AwagsGui::RefreshCurrents (int awags, bool reset)
{
  bool rev=true;
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  QString asicstate= " I ASIC :";
  QString hvstate=   " I HV   :";
  QString diodestate=" I DIODE:";
  if(!theSetup->IsAwagsPresent(awags))
    {
      RefreshColouredLabel(fAwagsCurrentASICLabel[awags],asicstate,awags_yellow_background);
      RefreshColouredLabel(fAwagsCurrentHVLabel[awags],hvstate,awags_yellow_background);
      RefreshColouredLabel(fAwagsCurrentDiodeLabel[awags],diodestate,awags_yellow_background);
    }
  else if(reset)
    {
      RefreshColouredLabel(fAwagsCurrentASICLabel[awags],asicstate,awags_blue_background);
      RefreshColouredLabel(fAwagsCurrentHVLabel[awags],hvstate,awags_blue_background);
      RefreshColouredLabel(fAwagsCurrentDiodeLabel[awags],diodestate,awags_blue_background);
      RefreshColouredLabel(fAwagsWidget->CurrentMeasurementsLabel_All,"NOT DONE", awags_blue_background);
    }
  else
  {
    //  set red or green depending on alarm level.

    AwagsTextColor_t color=awags_red_background;

// if present, refresh also measured values:
  double iasic=theSetup->GetCurrentASIC(awags);
  iasic *= 1.0e+3; // displayed units: mA
  //std::cout<< "Refresh Currents for awags "<<awags<<" gets Iasic="<<iasic << std::endl;
  fAwagsCurrentASICSpin[awags]->setValue(iasic);

  if((iasic>fAwagsWidget->CurrentASIC_DoubleSpinBox_Min->value()) &&
        (iasic<fAwagsWidget->CurrentASIC_DoubleSpinBox_Max->value()))
  {
      color=awags_green_background;
  }
  else
  {
      color=awags_red_background;
      rev=false;
  }
  RefreshColouredLabel(fAwagsCurrentASICLabel[awags],asicstate,color);


  double ihv=theSetup->GetCurrentHV(awags);
  ihv *= 1.0e+9; // displayed units: nA
  //std::cout<< "Refresh Currents for awags "<<awags<<" gets Ihv="<<ihv << std::endl;
   fAwagsCurrentHVSpin[awags]->setValue(ihv);
   if((ihv>fAwagsWidget->CurrentHV_DoubleSpinBox_Min->value()) &&
         (ihv<fAwagsWidget->CurrentHV_DoubleSpinBox_Max->value()))
   {
       color=awags_green_background;
   }
   else
   {
       color=awags_red_background;
       rev=false;
   }
   RefreshColouredLabel(fAwagsCurrentHVLabel[awags],hvstate,color);


   double idiode=theSetup->GetCurrentDiode(awags);
   //idiode *= 1.0e+6; // displayed units: microAmpere
   //idiode *= 1.0e+9; // displayed units: nanoAmpere
   idiode *= 1.0e+3; // displayed units: milliAmpere
   //std::cout<< "Refresh Currents for awags "<<awags<<" gets Idiode="<<idiode << std::endl;
     fAwagsCurrentDiodeSpin[awags]->setValue(idiode);
     if((idiode>fAwagsWidget->CurrentDiode_DoubleSpinBox_Min->value()) &&
           (idiode<fAwagsWidget->CurrentDiode_DoubleSpinBox_Max->value()))
     {
         color=awags_green_background;
     }
     else
     {
         color=awags_red_background;
         rev=false;
     }
     RefreshColouredLabel(fAwagsCurrentDiodeLabel[awags],diodestate,color);
  }
  return rev;
}


//bool AwagsGui::RefreshIDScan(int awags, bool reset)
//{
//
//
//  bool rev=true;
//  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
//  QString idscan=    "ID Scan________";
//  QString general=   "General Call___";
//  QString reverse=   "Reverse ID Scan";
//  QString reg=       "Register Test__";
//
//  AwagsTextColor_t idcolor=awags_green_background;
//  AwagsTextColor_t gencolor=awags_green_background;
//  AwagsTextColor_t revcolor=awags_green_background;
//  AwagsTextColor_t regcolor=awags_green_background;
//
//   if(!theSetup->IsAwagsPresent(awags))
//    {
//    idcolor=awags_yellow_background;
//    gencolor=awags_yellow_background;
//    revcolor=awags_yellow_background;
//    regcolor=awags_yellow_background;
//    }
//  else if(reset)
//  {
//     idcolor=awags_blue_background;
//     gencolor=awags_blue_background;
//     revcolor=awags_blue_background;
//     regcolor=awags_blue_background;
//     RefreshColouredLabel(fAwagsWidget->AdressTestLabel_All,"NOT DONE", awags_blue_background);
//  }
//  else
//  {
//    if(!theSetup->IsIDScanOK(awags))
//    {
//      idcolor=awags_red_background;
//      rev=false;
//    }
//    if(!theSetup->IsGeneralScanOK(awags))
//    {
//      gencolor=awags_red_background;
//      rev=false;
//    }
//
//    if(!theSetup->IsReverseIDScanOK(awags))
//    {
//      revcolor=awags_red_background;
//      rev=false;
//    }
//
//    if(!theSetup->IsRegisterScanOK(awags))
//    {
//      regcolor=awags_red_background;
//      rev=false;
//    }
//  }
//
//  RefreshColouredLabel(fAwagsIDScanLabel[awags],idscan, idcolor);
//  RefreshColouredLabel(fAwagsGeneralCallLabel[awags],general, gencolor);
//  RefreshColouredLabel(fAwagsReverseIDScanLabel[awags],reverse , revcolor);
//  RefreshColouredLabel(fAwagsRegisterTestLabel[awags],reg , regcolor);
//
//  return rev;
//}



void AwagsGui::RefreshColouredLabel(QLabel* label, const QString text, AwagsTextColor_t color)
{
  if(label==0) return;
  QString labeltext=" <html><head/><body><p> <span style=\" font-weight:600;";
  switch (color)
  {
    case awags_red:
      labeltext.append(" color:#ff0000;\"> ");
      break;
    case awags_green:
      labeltext.append(" color:#00cc00;\"> ");
      break;
    case awags_yellow:
      labeltext.append(" color:#cccc00;\"> ");
      break;
    case awags_blue:
      labeltext.append(" color:#0000cc;\"> ");
      break;
    case awags_red_background:
      labeltext.append(" background-color:#ff0000;\"> ");
      break;
    case awags_green_background:
      labeltext.append(" background-color:#00cc00;\"> ");
      break;
    case awags_yellow_background:
      labeltext.append(" background-color:#cccc00;\"> ");
      break;
    case awags_blue_background:
      labeltext.append(" background-color:#0000dd;\"> ");
      break;


    case awags_black:
    default:
      labeltext.append(" background-color:#000000;\"> ");
      break;

  }

  labeltext.append(text);
  labeltext.append(" </span></p></body></html>");
  label->setText(labeltext);

}



void AwagsGui::RefreshADC_channel (int channel, int gain)
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

void AwagsGui::RefreshADC_Awags (int awagschip, int dac)
{
   theSetup_GET_FOR_SLAVE(BoardSetup);
  int chan = theSetup->EvaluateADCChannel (awagschip, dac);
  //std::cout << "RefreshADC(" << (int) awagschip <<"):  dac:"<<dac<<", chan=" << chan<< std::endl;
  if (chan >= 0)
  {
    // only refresh adc channels once for active dacs
    int gain = theSetup->GetGain (awagschip, dac);
    for (int c=chan; c<chan + AWAGS_NUMCHANS; ++c)
    {
      RefreshADC_channel (chan, gain);
      // the single dac of each awags works on the next AWAGS_NUMCHANS=4 FEBEX channels
      // refresh them all!
    }
  }
}




void AwagsGui::RefreshView ()
{
// display setup structure to gui:
//  QString text;
//  QString pre;
//  fNumberBase == 16 ? pre = "0x" : pre = "";
  theSetup_GET_FOR_SLAVE(BoardSetup);

//////////////////////////////////////////////////////
// io configuration and gain:

  bool isprotoboard=theSetup->IsUsePrototypeBoard();
//
  fAwagsWidget->ProtoRadioButton->setChecked (isprotoboard);

// JAM22: here refresh gains:
  int gain=1;
  for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
     {
      gain=theSetup->GetGain (awags, 0);
      int ix=0;
      int mask=gain;
      for(ix=0; ix<5; ++ix)
      {
        if((mask && 0x1) == 0x1) break;
        mask= (mask>>1);
      }
      if(ix==4)
      {
        printf("Never come here: RefreshView() could not find correct gain bit, gain=0x%x \n",gain);
        ix=0; // avoid crashing the gui, but need to debug in this case...
      }
        fAwagsGainCombo[awags]->setCurrentIndex (ix);
     }
///////////////////////////////// JAM22


///////////////////////////////////////////////////////
// show DAC and test result values:

  int idfails=0;
  int currentfails=0;
  for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
  {
    RefreshDAC (awags);
   // if(!RefreshIDScan(awags)) idfails++;
    if(!RefreshCurrents(awags)) currentfails++; // show most recent current measurments
  }
//  if(idfails==0)
//  {
//    RefreshColouredLabel(fAwagsWidget->AdressTestLabel_All,"PASSED", awags_green_background);
//  }
//  else
//  {
//    QString fails= QString("FAILED %1 chips").arg(idfails);
//    RefreshColouredLabel(fAwagsWidget->AdressTestLabel_All,fails, awags_red_background);
//  }

  if(currentfails==0)
   {
     RefreshColouredLabel(fAwagsWidget->CurrentMeasurementsLabel_All,"PASSED", awags_green_background);
   }
   else
   {
     QString fails= QString("FAILED %1 chips").arg(currentfails);
     RefreshColouredLabel(fAwagsWidget->CurrentMeasurementsLabel_All, fails, awags_red_background);
   }




  RefreshBaselines();

//////////////////////////////////////////////////////////

  //fAwagsWidget->Baseline_Box_invert->setChecked (!theSetup->IsBaselineInverted());
// dac relative baseline settings and adc sample:
  int awags = 0, dac = 0;
  for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
  {
    theSetup->EvaluateDACIndices (channel, awags, dac);
    int gain = theSetup->GetGain (awags, dac);
    RefreshADC_channel (channel, gain);

    // also put most recent sample parameters to display:
    RefreshLastADCSample (channel);
  }
  GosipGui::RefreshView ();
   // ^this handles the refresh of chains and status. better use base class function here! JAM2018
}






void AwagsGui::EvaluateGain (int awags)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  int ix = fAwagsGainCombo[awags]->currentIndex ();
  int gain=(1<<ix);
  //printf("EvaluateGain for awags %d gives gain %d from index %d\n",
  //    awags, gain, ix);
  theSetup->SetGain (awags, 0, gain);
}

void AwagsGui::EvaluateIOSwitch ()
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  // get io config from gui

  theSetup->SetUsePrototypeBoard(fAwagsWidget->ProtoRadioButton->isChecked());
}

void AwagsGui::EvaluateView ()
{
  // here the current gui display is just copied to setup structure in local memory
  theSetup_GET_FOR_SLAVE(BoardSetup);
//std::cout<<"AwagsGui::EvaluateView ()" << std::endl;

  EvaluateIOSwitch ();


    for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
    {
        EvaluateGain (awags);
    }

// here baseline sliders for dacs:


// prevent different settings from DAC and ADC tabs; check which tab is active?
// TODO JAM2019 - handle this depending on visible subwindow?
//  if (fAwagsWidget->AwagsTabWidget->currentIndex () == 3)
  {
    // only apply the adc sliders when visible
    for (int channel = 0; channel < AWAGS_ADC_CHANNELS; ++channel)
    {
      int permille = fDACSpinBoxes[channel]->value ();
      int value = theSetup->EvaluateDACvalueAbsolute (permille);
      //std::cout<<"EvaluateView for channel:"<<channel<<", permille:"<<permille<<" - val="<<value<< std::endl;
      theSetup->SetDACValue (channel, value);
    }
  }
//  else
//  {
//    // otherwise use direct entries of DAC panel:
//    for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
//    {
//      for (int dac = 0; dac < AWAGS_NUMDACS; ++dac)
//      {
//        int value = fDACSlider[awags][dac]->value () & 0x3FF;
//        //std::cout<<"EvaluateView for awags:"<<awags<<", dac:"<<dac<<" - val="<<value<< std::endl;
//        theSetup->SetDACValue (awags, dac, value);
//      }
//
//    }
//  }    //if(AwagsTabWidget->currentIndex()==3)



}






uint8_t AwagsGui::GetAwagsId (int sfp, int slave, uint8_t awagschip)
{
  if (sfp < 0 || sfp >= PEX_SFP_NUMBER)
    return 0xFF;
  if (slave < 0 || slave >= fSFPChains.numslaves[sfp])
    return 0xFF;
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  return theSetup->GetAwagsID (awagschip);
}


int AwagsGui::ScanDACCurve (int gain, int channel)
{
  QApplication::setOverrideCursor (Qt::WaitCursor);
  theSetup_GET_FOR_SLAVE_RETURN(BoardSetup);
  int awags = 0, dac = 0, dacrecord=0;
  theSetup->EvaluateDACIndices (channel, awags, dac);
  AwagsTestResults& theResults = theSetup->AccessTestResults (gain, awags);
  dacrecord=dac;

  // JAM22: note that with this we will repeat measurments for each awags dac 4 times
  // (for each of the awags channels), but only record the last curve
  // to account differences of the 4 output channels, should work with separate dac indices here?



  // kludge to get also results from second adc for dac3: we record it for dac4
//  if (gain == 1)
//  {
//    if ((channel % 2) != 0)
//    {
//      dacrecord++;
//      printm ("\tScanDACCurve for channel %d  at gain 1: -shifted dac to index %d to record results", channel, dacrecord);
//    }
//  }
  ////////////////////////

  theResults.ResetDacSample (dacrecord);
  int points = AWAGS_DAC_CURVEPOINTS;
  // depending on gain, we have different stepsizes
  //int step = 2/gain * 8; // JAM22 preliminary
  int step=8;

//  switch (gain)
//  {
//    case 1:
//      step = 16;
//      //dac = 2;
//      break;
//    case 16:
//      step = 2;
//      break;
//    case 32:
//      step = 1;
//      break;
//  }

  // we start in the middle of the autocalibration point:
  uint16_t dac_mid = theResults.GetDacValueCalibrate (dac);
  if (dac_mid == 0)
  {
    // no calibration done yet, do it now
    DoAutoCalibrate (awags);
    dac_mid = theSetup->GetDACValue (awags, dac);
  }
  //std::cout<<"ScanDACCurve for gain:"<<gain<<", step:"<<step<<", channel:"<<channel<<" - DAC middle point is "<<dac_mid << std::endl;
  EnableI2C ();
  uint16_t d0 = dac_mid - step * points / 2;
  for (int p = 0; p < points; ++p)
  {
    uint16_t dacval = d0 + p * step;
    theSetup->SetDACValue (awags, dac, dacval);
    WriteDAC_AwagsI2c (awags, dac, theSetup->GetDACValue (awags, dac));
    int adcval = AcquireBaselineSample (channel);
   //std::cout<<"   ScanDACCurve got d:"<<dacval<<", adc:"<<adcval << std::endl;
    theResults.AddDacSample (dacrecord, dacval, adcval);
  }
  DisableI2C ();
  ResetBenchmarkCurve ();
  ShowLimitsCurve (gain, awags, dac);
  ShowBenchmarkCurve (gain, awags, dac);

  QApplication::restoreOverrideCursor ();
}

void AwagsGui::UpdateAfterAutoCalibrate (uint8_t awagschip)
{
  // here get registers of awagschip only and refresh
  EnableI2C ();
  GetDACs (awagschip);
  DisableI2C ();
  RefreshDAC (awagschip);
  for (int dac = 0; dac < AWAGS_NUMDACS; ++dac)
  {
    RefreshADC_Awags (awagschip, dac);
  }
}


//void AwagsGui::SetInverseMapping (int on)
//{
//  theSetup_GET_FOR_SLAVE(BoardSetup);
//  theSetup->SetAwagsMapping (!on);
//
//}


//void AwagsGui::SetBaselineInverted(int on)
//{
//  theSetup_GET_FOR_SLAVE(BoardSetup);
//  //std::cout<< "SetBaselineInverted with "<<on << std::endl;
//  theSetup->SetBaselineInverted (on);
//  for (int channel = 0; channel < 16; ++channel)
//   {
//       CalibrateResetADC (channel);
//   }
//}




//int AwagsGui::EvaluatePulserInterval (int findex)
//{
//  int period = 1000;
//  switch (findex)
//  {
//    case 0:
//    default:
//      period = 1000;
//      break;
//    case 1:
//      period = 500;
//      break;
//    case 2:
//      period = 200;
//      break;
//    case 3:
//      period = 100;
//      break;
//    case 4:
//      period = 20;
//      break;
//  };
//  //std::cout << "EvaluatePulserInterval gives ms period:" <<  period << std::endl;
//  return period;
//}



void AwagsGui::RefreshBaselines()
{
  //std::cout << "RefreshBaselines" <<  std::endl;
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";

  double lowpermil=fAwagsWidget->BaselineLowerSlider->value();
  double uppermil=fAwagsWidget->BaselineUpperSlider->value();
  double maxrange=AWAGS_ADC_SAMPLEVALUES;
  if(fAwagsWidget->ReadoutRadioButton->isChecked())
    maxrange=AWAGS_MBS_TRACELEN;
  int lowvalue= lowpermil*0.001*maxrange;
  int upvalue= uppermil*0.001*maxrange;
  fAwagsWidget->BaselineLowerLineEdit->setText(pre + text.setNum (lowvalue, fNumberBase));
  fAwagsWidget->BaselineUpperLineEdit->setText(pre + text.setNum (upvalue, fNumberBase));

}


//void AwagsGui::DoIdScan()
//{
//  for(int a=0; a<AWAGS_NUMCHIPS; ++a)
//   {
//      RefreshIDScan(a,true); // reset colors
//   }
//  //RefreshColouredLabel(fAwagsWidget->AdressTestLabel_All,QString("New Test..."), awags_blue_background);
//  for(int a=0; a<AWAGS_NUMCHIPS; ++a)
//  {
//    ExecuteIDScanTest(a);
//    RefreshIDScan(a);
//  }
//  SetDefaultIOConfig(); // back to normal operation
//
//}



void AwagsGui::DoCurrentScan ()
{

  //  reset to blue for new scan:
  for(int a=0; a<AWAGS_NUMCHIPS; ++a)
     {
        RefreshCurrents(a,true); // reset colors
     }


  //RefreshColouredLabel(fAwagsWidget->CurrentMeasurementsLabel_All,QString("New Test..."), awags_blue_background);
  for (int a = 0; a < AWAGS_NUMCHIPS; ++a)
  {
    ExecuteCurrentScan (a);
    RefreshCurrents (a);
  }
  SetDefaultIOConfig ();    // back to normal operation
}





void AwagsGui::LoadBenchmarkReferences()
{
  QFileDialog fd (this, "Select Benchmark reference data file", ".", "awags characterization file (*.apf)");
  fd.setFileMode (QFileDialog::ExistingFile);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  QString filename = flst[0];
  fBenchmark.LoadReferenceValues(filename);
  fAwagsWidget->ReferenceLineEdit->setText(filename);
}




void AwagsGui::SaveTestResults ()
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
//  QString header = QString ("# awags1:").append (apf1).append (", awags2:").append (apf2);
//  header.append (QString (" Current:%1 A Voltage:%1 A").arg (current).arg (voltage));
//  header.append ("\n");
//  WriteTestFile (header);
// information of this header is now part of table

  // header
  QString tstamp=QDateTime::currentDateTime().toString(AWAGS_RESULT_TIMEFORMAT);
  WriteTestFile(QString("# This is an AWAGS Test result file saved with AwagsGui on "));
  WriteTestFile(tstamp);
  WriteTestFile(QString("\n"));
  WriteTestFile(QString("#   developed for FAIR/PASEM project and PANDA 2016-2017 by JAM (j.adamczewski@gsi.de), GSI Experiment Electronics department \n"));
  WriteTestFile(QString("#\n"));

  // format
  WriteTestFile (
      QString ("#  CarrierBoardID\t ChipID \tGain \tAWAGS \tDAC \tCalibSet \tBaseline \tSigma  \tBaseLow \tBaseUp \tdDAC/dADC \tDAC0 \tDACmin \tDACmax \tADCmin"));
  for (int i = 0; i < AWAGS_DAC_CURVEPOINTS; ++i)
  {
    WriteTestFile (QString ("\tDAC_%1 \tADC_%2").arg (i).arg (i));
  }

//  WriteTestFile (QString ("\tPeakPolarity"));
//  for (int i = 0; i < AWAGS_ADC_NUMMAXIMA; ++i)
//    {
//      WriteTestFile (QString ("\tPeakPos_%1 \tPeakHeight_%2").arg (i).arg (i));
//    }

  WriteTestFile (QString ("\tI_ASIC(A) \tI_HV(A)  \tI_Diode(A)  \tTemp (C) \t\tStartDate \t StartTime \tStopDate \tStopTime"));
  WriteTestFile (QString ("\n"));
  // loop over gain:
  for (int gain = 1; gain < 16; gain *=2) // JAM22 TTODO: check for real gain values later
  {
    for (int awags = 0; awags < AWAGS_NUMCHIPS; ++awags)
    {
      AwagsTestResults& theResult = theSetup->AccessTestResults (gain, awags);
      for (int dac = 0; dac < AWAGS_NUMDACS; ++dac)
      {
        QString carrierid = theResult.GetCarrierBoardDescriptor ();
        QString chipid = theResult.GetChipDescriptor ();
        int awagsaddress = theResult.GetAddressId ();
        int dacval = theResult.GetDacValueCalibrate (dac, true);    // when saving, we assure that test was really done
        int baseline = theResult.GetAdcSampleMean (dac, true);
        int sigma = theResult.GetAdcSampleSigma (dac, true);
        int startbase = theResult.GetAdcBaselineLowerBound (dac, true);
        int stopbase = theResult.GetAdcBaselineUpperBound (dac, true);
        double slope = theResult.GetSlope (dac, true);
        double dac0 = theResult.GetD0 (dac, true);
        double dacmin = theResult.GetDACmin (dac, true);
        double dacmax = theResult.GetDACmax (dac, true);
        double adcmin = theResult.GetADCmin (dac, true);
        double currentasic = theResult.GetCurrentASIC ();
        double currenthv = theResult.GetCurrentHV ();
        double currentdiode = theResult.GetCurrentDiode ();
//        bool idscanok=theResult.IsIDScanOK();
//        bool generalcallok=theResult.IsGeneralCallScanOK();
//        bool reverseidok=theResult.IsReverseIDScanOK();
//        bool registerok=theResult.IsRegisterScanOK();

        // here we should supress/mark as invalid the results that are not meaningful for the selected gain:




        QString line = "\t";
        line.append(carrierid);
        line.append("\t");
        line.append(chipid);
        line.append(QString ("\t\t%1 \t\t%2 \t\t%3 \t\t%4 \t\t%5 \t\t%6 \t\t%7 \t\t%8 \t\t%9").arg (gain).arg (awagsaddress).arg (dac).arg (dacval).arg (baseline).arg (sigma).arg(startbase).arg(stopbase).arg (slope));
        line.append (QString ("\t\t%1 \t\t%2 \t\t%3 \t\t%4").arg (dac0).arg (dacmin).arg (dacmax).arg (adcmin));

        // add the results of the curve to the line:
        for (int i = 0; i < AWAGS_DAC_CURVEPOINTS; ++i)
        {
          int dval=AWAGS_NOVALUE, aval=AWAGS_NOVALUE;
          if(theResult.IsValid())
          {
            DacSample point (0, 0);
            if(theResult.AccessDacSample (point, dac, i)==0) // if this fails, we keep the AWAGS_NOVALUE
            {
              dval = point.GetDACValue ();
              aval = point.GetADCValue ();
            }
          }
          //std::cout<<"saving curve point"<<i<<", dac="<<(int) dval<<", adc="<<(int) aval << std::endl;
          line.append (QString ("\t%1 \t%2").arg (dval).arg (aval));
        }

        // put here location of peak finder
//        bool isnegative=theResult.HasNegativeAdcPeaks(dac);
//        line.append((isnegative ? QString("\t -1") : QString("\t 1")));
//        for (int i = 0; i < AWAGS_ADC_NUMMAXIMA; ++i)
//                {
//                  int pos = theResult.GetAdcPeakPosition(dac,i,true);
//                  int max = theResult.GetAdcPeakHeight(dac,i,true);
//                  line.append (QString ("\t%1 \t%2").arg (pos).arg (max));
//                }

        line.append(QString ("\t%1 \t%2 \t%3\t").arg(currentasic).arg(currenthv).arg(currentdiode));
//        line.append(QString ("\t%1 \t%2 \t%3 \t%4 \t").arg(idscanok).arg(generalcallok).arg(reverseidok).arg(registerok));
        line.append("\t");
        line.append(theResult.GetTemperatureInfo());
        line.append("\t");
        line.append(theResult.GetStartTime());
        line.append("\t");
        line.append(theResult.GetEndTime());
        line.append ("\n");
        WriteTestFile (line);
      }

    }
  }

}


void AwagsGui::SaveConfig()
  {
    DoSaveConfig();
  }


void AwagsGui::DoSaveConfig(const char* selectfile)
{
//std::cout << "AwagsGui::SaveConfigBtn_clicked()"<< std::endl;

  static char buffer[1024];
  QString gos_filter ("gosipcmd file (*.gos)");
  QString apf_filter ("awags characterization file (*.apf)");
  QStringList filters;
  filters << gos_filter << apf_filter;

  QFileDialog fd (this, "Write Awags configuration file");

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
    std::cout << "AwagsGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
  }

}




