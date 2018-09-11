#include "TamexPadiGui.h"

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
#include <math.h>

// *********************************************************

#define TAMEX_PRINT_DUMP(args...) \
if(fTamexDumpMode) printm( args );




std::map<Tamex_Module_type, std::map< int, QString > > TamexPadiGui::fgClockSourceText=
{
    {   TamMod_TAMEX2,
        {
            {0, QString("0x20 -> External CLK via 2 pin lemo (200 MHz)")},
            {1, QString("0x21 -> CLK from TRBus (25 MHz) via on-board PLL")},
            {2, QString("0x22 -> CLK from TRBus + Module 0 feeds 25 MHz CLK to TRBus")},
            {3, QString("0x24 -> On-board oscillator (200 MHz)")},
            {4, QString("")}
        }
    },
    {  TamMod_TAMEXPADI1,
        {
                {0, QString("0x0 -> CLK from previous module via backplane (feed in 200 MHz on crate interface)")},
                {1, QString("0x1 -> CLK from on-board oscillator")},
                {2, QString("0x2 -> CLK from 2-pin LEMO on front (feed in 200 MHz)")},
                {3, QString("0x4 -> CLK from previous module via backplane (first module CLK-master with local CLK)")},
                {4, QString("0x8 -> CLK from previous module via backplane (first module CLK-master with external CLK from front)")}

        }
     },
     {  TamMod_TAMEX3,
         {
                 {0, QString("0x26 -> External CLK from backplane (200 MHz)")},
                 {1, QString("0x2a -> On-board oscillator (200 MHz)")},
                 {2, QString(" 0x22 -> CLK from TRBus (25 MHz) via on-board PLL (Module 0 feeds 25 MHz CLK to TRBus)")},
                 {3, QString("0x20 -> External CLK from backplane (25 MHz)")},
                 {4, QString("")}
         }
     }

};

/*
 *  Constructs a TamexPadiGui which is a child of 'parent', with the
 *  name 'name'.'
 */
TamexPadiGui::TamexPadiGui (QWidget* parent) :
    fShowAmplifiedVoltages (true), fTamexDumpMode (false), GosipGui (parent)
{

  fImplementationName = "TAMEX-PADI";
  fVersionString = "Welcome to TAMEX-PADI GUI!\n\t v0.93 of 11-Sep-2018 by JAM (j.adamczewski@gsi.de)";

  fTamexPadiWidget = new TamexPadiWidget (this);
  Settings_scrollArea->setWidget (fTamexPadiWidget);
  setWindowTitle (QString ("%1 GUI").arg (fImplementationName));

  ClearOutputBtn_clicked ();

  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_all, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_all_changed(double)));

  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_00, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_00_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_01, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_01_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_02, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_02_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_03, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_03_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_04, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_04_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_05, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_05_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_06, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_06_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_07, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_07_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_08, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_08_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_09, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_09_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_10, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_10_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_11, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_11_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_12, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_12_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_13, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_13_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_14, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_14_changed(double)));
  QObject::connect (fTamexPadiWidget->Threshold_doubleSpinBox_15, SIGNAL(valueChanged(double)), this, SLOT(Threshold_doubleSpinBox_15_changed(double)));

  QObject::connect (fTamexPadiWidget->Threshold_Slider_all, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_all_changed(int)));

  QObject::connect (fTamexPadiWidget->Threshold_Slider_00, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_00_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_01, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_01_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_02, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_02_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_03, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_03_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_04, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_04_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_05, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_05_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_06, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_06_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_07, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_07_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_08, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_08_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_09, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_09_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_10, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_10_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_11, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_11_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_12, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_12_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_13, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_13_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_14, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_14_changed(int)));
  QObject::connect (fTamexPadiWidget->Threshold_Slider_15, SIGNAL(valueChanged(int)), this, SLOT(Threshold_Slider_15_changed(int)));

  QObject::connect (fTamexPadiWidget->Threshold_Value_all, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_all_changed ()));

  QObject::connect (fTamexPadiWidget->Threshold_Value_00, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_00_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_01, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_01_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_02, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_02_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_03, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_03_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_04, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_04_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_05, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_05_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_06, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_06_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_07, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_07_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_08, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_08_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_09, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_09_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_10, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_10_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_11, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_11_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_12, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_12_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_13, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_13_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_14, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_14_changed ()));
  QObject::connect (fTamexPadiWidget->Threshold_Value_15, SIGNAL (editingFinished ()), this,
      SLOT (Threshold_Text_15_changed ()));

  QObject::connect (fTamexPadiWidget->channelGroupBox_all, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_all(bool)));

  QObject::connect (fTamexPadiWidget->channelGroupBox_00, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_00(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_01, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_01(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_02, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_02(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_03, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_03(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_04, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_04(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_05, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_05(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_06, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_06(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_07, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_07(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_08, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_08(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_09, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_09(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_10, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_10(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_11, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_11(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_12, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_12(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_13, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_13(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_14, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_14(bool)));
  QObject::connect (fTamexPadiWidget->channelGroupBox_15, SIGNAL(toggled(bool)), this, SLOT(ChannelEnabled_toggled_15(bool)));

  QObject::connect (fTamexPadiWidget->Channel_leading_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_all(bool)));

  QObject::connect (fTamexPadiWidget->Channel_leading_radio_00, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_00(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_01, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_01(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_02, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_02(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_03, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_03(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_04, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_04(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_05, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_05(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_06, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_06(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_07, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_07(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_08, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_08(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_09, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_09(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_10, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_10(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_11, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_11(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_12, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_12(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_13, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_13(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_14, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_14(bool)));
  QObject::connect (fTamexPadiWidget->Channel_leading_radio_15, SIGNAL(toggled(bool)), this, SLOT(ChannelLeading_toggled_15(bool)));

  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_ALL, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_all(bool)));

  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_00, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_00(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_01, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_01(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_02, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_02(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_03, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_03(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_04, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_04(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_05, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_05(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_06, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_06(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_07, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_07(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_08, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_08(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_09, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_09(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_10, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_10(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_11, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_11(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_12, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_12(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_13, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_13(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_14, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_14(bool)));
  QObject::connect (fTamexPadiWidget->Channel_trailing_radio_15, SIGNAL(toggled(bool)), this, SLOT(ChannelTrailing_toggled_15(bool)));

  //////////////////////////


  QObject::connect (fTamexPadiWidget->TriggerEnabledBoxAll, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_all(bool)));

   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh00, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_00(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh01, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_01(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh02, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_02(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh03, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_03(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh04, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_04(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh05, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_05(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh06, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_06(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh07, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_07(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh08, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_08(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh09, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_09(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh10, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_10(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh11, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_11(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh12, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_12(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh13, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_13(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh14, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_14(bool)));
   QObject::connect (fTamexPadiWidget->TriggerEnabledBoxCh15, SIGNAL(toggled(bool)), this, SLOT(TriggerEnabled_toggled_15(bool)));


   //currentIndexChanged

   QObject::connect (fTamexPadiWidget->PolarityComboBoxAll, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_all(int )));

     QObject::connect (fTamexPadiWidget->PolarityComboBox00, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_00(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox01, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_01(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox02, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_02(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox03, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_03(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox04, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_04(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox05, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_05(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox06, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_06(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox07, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_07(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox08, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_08(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox09, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_09(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox10, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_10(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox11, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_11(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox12, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_12(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox13, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_13(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox14, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_14(int )));
     QObject::connect (fTamexPadiWidget->PolarityComboBox15, SIGNAL(currentIndexChanged(int)), this, SLOT(InputPolarity_toggled_15(int )));





  //////////////////////////



  QObject::connect (fTamexPadiWidget->VoltageModeCheckBox, SIGNAL(stateChanged(int)), this, SLOT (VoltageModeCheckBoxChanged(int)));

  QObject::connect (fTamexPadiWidget->PreTriggerSpinBox, SIGNAL(valueChanged(int)), this, SLOT(PreTriggerSpinBox_changed(int)));
  QObject::connect (fTamexPadiWidget->PostTriggerSpinBox, SIGNAL(valueChanged(int)), this, SLOT(PostTriggerSpinBox_changed(int)));
  QObject::connect (fTamexPadiWidget->TriggerwindowGroupBox, SIGNAL(toggled(bool)), this, SLOT(TriggerWindowGroupBox_toggled(bool)));

  QObject::connect (fTamexPadiWidget->ClockSourceComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT (ClockSourceCurrentIndexChanged(int)));

  QObject::connect (fTamexPadiWidget->Ch0RefRadioButton, SIGNAL(toggled(bool)), this, SLOT (TriggerOutChanged()));
  QObject::connect (fTamexPadiWidget->EnableOrCheckBox, SIGNAL(toggled(bool)), this, SLOT (TriggerOutChanged()));
  QObject::connect (fTamexPadiWidget->CombineOrCheckBox, SIGNAL(toggled(bool)), this, SLOT (TriggerOutChanged()));



  QObject::connect (fTamexPadiWidget->BoardTypeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT (BoardTypeComboBoxurrentIndexChanged(int)));



  /** JAM put references to designer checkboxes into array to be handled later easily: */
  fThresholdSpinBoxes[0] = fTamexPadiWidget->Threshold_doubleSpinBox_00;
  fThresholdSpinBoxes[1] = fTamexPadiWidget->Threshold_doubleSpinBox_01;
  fThresholdSpinBoxes[2] = fTamexPadiWidget->Threshold_doubleSpinBox_02;
  fThresholdSpinBoxes[3] = fTamexPadiWidget->Threshold_doubleSpinBox_03;
  fThresholdSpinBoxes[4] = fTamexPadiWidget->Threshold_doubleSpinBox_04;
  fThresholdSpinBoxes[5] = fTamexPadiWidget->Threshold_doubleSpinBox_05;
  fThresholdSpinBoxes[6] = fTamexPadiWidget->Threshold_doubleSpinBox_06;
  fThresholdSpinBoxes[7] = fTamexPadiWidget->Threshold_doubleSpinBox_07;
  fThresholdSpinBoxes[8] = fTamexPadiWidget->Threshold_doubleSpinBox_08;
  fThresholdSpinBoxes[9] = fTamexPadiWidget->Threshold_doubleSpinBox_09;
  fThresholdSpinBoxes[10] = fTamexPadiWidget->Threshold_doubleSpinBox_10;
  fThresholdSpinBoxes[11] = fTamexPadiWidget->Threshold_doubleSpinBox_11;
  fThresholdSpinBoxes[12] = fTamexPadiWidget->Threshold_doubleSpinBox_12;
  fThresholdSpinBoxes[13] = fTamexPadiWidget->Threshold_doubleSpinBox_13;
  fThresholdSpinBoxes[14] = fTamexPadiWidget->Threshold_doubleSpinBox_14;
  fThresholdSpinBoxes[15] = fTamexPadiWidget->Threshold_doubleSpinBox_15;

  fThresholdLineEdit[0] = fTamexPadiWidget->Threshold_Value_00;
  fThresholdLineEdit[1] = fTamexPadiWidget->Threshold_Value_01;
  fThresholdLineEdit[2] = fTamexPadiWidget->Threshold_Value_02;
  fThresholdLineEdit[3] = fTamexPadiWidget->Threshold_Value_03;
  fThresholdLineEdit[4] = fTamexPadiWidget->Threshold_Value_04;
  fThresholdLineEdit[5] = fTamexPadiWidget->Threshold_Value_05;
  fThresholdLineEdit[6] = fTamexPadiWidget->Threshold_Value_06;
  fThresholdLineEdit[7] = fTamexPadiWidget->Threshold_Value_07;
  fThresholdLineEdit[8] = fTamexPadiWidget->Threshold_Value_08;
  fThresholdLineEdit[9] = fTamexPadiWidget->Threshold_Value_09;
  fThresholdLineEdit[10] = fTamexPadiWidget->Threshold_Value_10;
  fThresholdLineEdit[11] = fTamexPadiWidget->Threshold_Value_11;
  fThresholdLineEdit[12] = fTamexPadiWidget->Threshold_Value_12;
  fThresholdLineEdit[13] = fTamexPadiWidget->Threshold_Value_13;
  fThresholdLineEdit[14] = fTamexPadiWidget->Threshold_Value_14;
  fThresholdLineEdit[15] = fTamexPadiWidget->Threshold_Value_15;

  fThresholdSlider[0] = fTamexPadiWidget->Threshold_Slider_00;
  fThresholdSlider[1] = fTamexPadiWidget->Threshold_Slider_01;
  fThresholdSlider[2] = fTamexPadiWidget->Threshold_Slider_02;
  fThresholdSlider[3] = fTamexPadiWidget->Threshold_Slider_03;
  fThresholdSlider[4] = fTamexPadiWidget->Threshold_Slider_04;
  fThresholdSlider[5] = fTamexPadiWidget->Threshold_Slider_05;
  fThresholdSlider[6] = fTamexPadiWidget->Threshold_Slider_06;
  fThresholdSlider[7] = fTamexPadiWidget->Threshold_Slider_07;
  fThresholdSlider[8] = fTamexPadiWidget->Threshold_Slider_08;
  fThresholdSlider[9] = fTamexPadiWidget->Threshold_Slider_09;
  fThresholdSlider[10] = fTamexPadiWidget->Threshold_Slider_10;
  fThresholdSlider[11] = fTamexPadiWidget->Threshold_Slider_11;
  fThresholdSlider[12] = fTamexPadiWidget->Threshold_Slider_12;
  fThresholdSlider[13] = fTamexPadiWidget->Threshold_Slider_13;
  fThresholdSlider[14] = fTamexPadiWidget->Threshold_Slider_14;
  fThresholdSlider[15] = fTamexPadiWidget->Threshold_Slider_15;

  fChannelLeadingRadio[0] = fTamexPadiWidget->Channel_leading_radio_00;
  fChannelLeadingRadio[1] = fTamexPadiWidget->Channel_leading_radio_01;
  fChannelLeadingRadio[2] = fTamexPadiWidget->Channel_leading_radio_02;
  fChannelLeadingRadio[3] = fTamexPadiWidget->Channel_leading_radio_03;
  fChannelLeadingRadio[4] = fTamexPadiWidget->Channel_leading_radio_04;
  fChannelLeadingRadio[5] = fTamexPadiWidget->Channel_leading_radio_05;
  fChannelLeadingRadio[6] = fTamexPadiWidget->Channel_leading_radio_06;
  fChannelLeadingRadio[7] = fTamexPadiWidget->Channel_leading_radio_07;
  fChannelLeadingRadio[8] = fTamexPadiWidget->Channel_leading_radio_08;
  fChannelLeadingRadio[9] = fTamexPadiWidget->Channel_leading_radio_09;
  fChannelLeadingRadio[10] = fTamexPadiWidget->Channel_leading_radio_10;
  fChannelLeadingRadio[11] = fTamexPadiWidget->Channel_leading_radio_11;
  fChannelLeadingRadio[12] = fTamexPadiWidget->Channel_leading_radio_12;
  fChannelLeadingRadio[13] = fTamexPadiWidget->Channel_leading_radio_13;
  fChannelLeadingRadio[14] = fTamexPadiWidget->Channel_leading_radio_14;
  fChannelLeadingRadio[15] = fTamexPadiWidget->Channel_leading_radio_15;

  fChannelTrailingRadio[0] = fTamexPadiWidget->Channel_trailing_radio_00;
  fChannelTrailingRadio[1] = fTamexPadiWidget->Channel_trailing_radio_01;
  fChannelTrailingRadio[2] = fTamexPadiWidget->Channel_trailing_radio_02;
  fChannelTrailingRadio[3] = fTamexPadiWidget->Channel_trailing_radio_03;
  fChannelTrailingRadio[4] = fTamexPadiWidget->Channel_trailing_radio_04;
  fChannelTrailingRadio[5] = fTamexPadiWidget->Channel_trailing_radio_05;
  fChannelTrailingRadio[6] = fTamexPadiWidget->Channel_trailing_radio_06;
  fChannelTrailingRadio[7] = fTamexPadiWidget->Channel_trailing_radio_07;
  fChannelTrailingRadio[8] = fTamexPadiWidget->Channel_trailing_radio_08;
  fChannelTrailingRadio[9] = fTamexPadiWidget->Channel_trailing_radio_09;
  fChannelTrailingRadio[10] = fTamexPadiWidget->Channel_trailing_radio_10;
  fChannelTrailingRadio[11] = fTamexPadiWidget->Channel_trailing_radio_11;
  fChannelTrailingRadio[12] = fTamexPadiWidget->Channel_trailing_radio_12;
  fChannelTrailingRadio[13] = fTamexPadiWidget->Channel_trailing_radio_13;
  fChannelTrailingRadio[14] = fTamexPadiWidget->Channel_trailing_radio_14;
  fChannelTrailingRadio[15] = fTamexPadiWidget->Channel_trailing_radio_15;

  fChannelEnabledBox[0] = fTamexPadiWidget->channelGroupBox_00;
  fChannelEnabledBox[1] = fTamexPadiWidget->channelGroupBox_01;
  fChannelEnabledBox[2] = fTamexPadiWidget->channelGroupBox_02;
  fChannelEnabledBox[3] = fTamexPadiWidget->channelGroupBox_03;
  fChannelEnabledBox[4] = fTamexPadiWidget->channelGroupBox_04;
  fChannelEnabledBox[5] = fTamexPadiWidget->channelGroupBox_05;
  fChannelEnabledBox[6] = fTamexPadiWidget->channelGroupBox_06;
  fChannelEnabledBox[7] = fTamexPadiWidget->channelGroupBox_07;
  fChannelEnabledBox[8] = fTamexPadiWidget->channelGroupBox_08;
  fChannelEnabledBox[9] = fTamexPadiWidget->channelGroupBox_09;
  fChannelEnabledBox[10] = fTamexPadiWidget->channelGroupBox_10;
  fChannelEnabledBox[11] = fTamexPadiWidget->channelGroupBox_11;
  fChannelEnabledBox[12] = fTamexPadiWidget->channelGroupBox_12;
  fChannelEnabledBox[13] = fTamexPadiWidget->channelGroupBox_13;
  fChannelEnabledBox[14] = fTamexPadiWidget->channelGroupBox_14;
  fChannelEnabledBox[15] = fTamexPadiWidget->channelGroupBox_15;


  fChannelTriggerEnabCheck[0] =  fTamexPadiWidget->TriggerEnabledBoxCh00;
  fChannelTriggerEnabCheck[1] =  fTamexPadiWidget->TriggerEnabledBoxCh01;
  fChannelTriggerEnabCheck[2] =  fTamexPadiWidget->TriggerEnabledBoxCh02;
  fChannelTriggerEnabCheck[3] =  fTamexPadiWidget->TriggerEnabledBoxCh03;
  fChannelTriggerEnabCheck[4] =  fTamexPadiWidget->TriggerEnabledBoxCh04;
  fChannelTriggerEnabCheck[5] =  fTamexPadiWidget->TriggerEnabledBoxCh05;
  fChannelTriggerEnabCheck[6] =  fTamexPadiWidget->TriggerEnabledBoxCh06;
  fChannelTriggerEnabCheck[7] =  fTamexPadiWidget->TriggerEnabledBoxCh07;
  fChannelTriggerEnabCheck[8] =  fTamexPadiWidget->TriggerEnabledBoxCh08;
  fChannelTriggerEnabCheck[9] =  fTamexPadiWidget->TriggerEnabledBoxCh09;
  fChannelTriggerEnabCheck[10] =  fTamexPadiWidget->TriggerEnabledBoxCh10;
  fChannelTriggerEnabCheck[11] =  fTamexPadiWidget->TriggerEnabledBoxCh11;
  fChannelTriggerEnabCheck[12] =  fTamexPadiWidget->TriggerEnabledBoxCh12;
  fChannelTriggerEnabCheck[13] =  fTamexPadiWidget->TriggerEnabledBoxCh13;
  fChannelTriggerEnabCheck[14] =  fTamexPadiWidget->TriggerEnabledBoxCh14;
  fChannelTriggerEnabCheck[15] =  fTamexPadiWidget->TriggerEnabledBoxCh15;

  fChannelPolarityBox[0] = fTamexPadiWidget->PolarityComboBox00;
  fChannelPolarityBox[1] = fTamexPadiWidget->PolarityComboBox01;
  fChannelPolarityBox[2] = fTamexPadiWidget->PolarityComboBox02;
  fChannelPolarityBox[3] = fTamexPadiWidget->PolarityComboBox03;
  fChannelPolarityBox[4] = fTamexPadiWidget->PolarityComboBox04;
  fChannelPolarityBox[5] = fTamexPadiWidget->PolarityComboBox05;
  fChannelPolarityBox[6] = fTamexPadiWidget->PolarityComboBox06;
  fChannelPolarityBox[7] = fTamexPadiWidget->PolarityComboBox07;
  fChannelPolarityBox[8] = fTamexPadiWidget->PolarityComboBox08;
  fChannelPolarityBox[9] = fTamexPadiWidget->PolarityComboBox09;
  fChannelPolarityBox[10] = fTamexPadiWidget->PolarityComboBox10;
  fChannelPolarityBox[11] = fTamexPadiWidget->PolarityComboBox11;
  fChannelPolarityBox[12] = fTamexPadiWidget->PolarityComboBox12;
  fChannelPolarityBox[13] = fTamexPadiWidget->PolarityComboBox13;
  fChannelPolarityBox[14] = fTamexPadiWidget->PolarityComboBox14;
  fChannelPolarityBox[15] = fTamexPadiWidget->PolarityComboBox15;


  // just to update the ns labels (and to see if something changes when we show
  fTamexPadiWidget->PreTriggerSpinBox->setValue (100);
  fTamexPadiWidget->PostTriggerSpinBox->setValue (100);

  for(int i=0; i<16; ++i)
  {
      fChannelPolarityBox[i]->setItemData(0, QBrush(Qt::red),    Qt::BackgroundColorRole );
      fChannelPolarityBox[i]->setItemData(0, QBrush(Qt::black),  Qt::TextColorRole );
      fChannelPolarityBox[i]->setItemData(1, QBrush(Qt::blue),   Qt::BackgroundColorRole);
      fChannelPolarityBox[i]->setItemData(1, QBrush(Qt::white),  Qt::TextColorRole);
  }
  fTamexPadiWidget->PolarityComboBoxAll->setItemData(0, QBrush(Qt::red),  Qt::BackgroundColorRole);
  fTamexPadiWidget->PolarityComboBoxAll->setItemData(0, QBrush(Qt::black),  Qt::TextColorRole);
  fTamexPadiWidget->PolarityComboBoxAll->setItemData(1, QBrush(Qt::blue), Qt::BackgroundColorRole);
  fTamexPadiWidget->PolarityComboBoxAll->setItemData(0, QBrush(Qt::red),  Qt::TextColorRole);
  RefreshPolaritySelector( fTamexPadiWidget->PolarityComboBoxAll, true); // put default color to general pol widget
  GetSFPChainSetup ();    // ensure that any slave has a status structure before we begin clicking...
  show ();


}

TamexPadiGui::~TamexPadiGui ()
{
}

void TamexPadiGui::EnableSPI ()
{
  // TODO do we need something here?

}

void TamexPadiGui::DisableSPI ()
{
  // TODO do we need something here?

}

void TamexPadiGui::PadiSPISleep ()
{
  usleep (500);
}

void TamexPadiGui::ResetSlave ()
{

  // taken from example MBS user readout function:

  //  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, 0x00);
  //  usleep (4000);
  //

  // disable test data length
  WriteGosip (fSFP, fSlave, REG_DATA_LEN, 0x10000000);

  // write SFP id for TDC header
  WriteGosip (fSFP, fSlave, REG_HEADER, fSFP);

  // PADI default thresholds:
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_DAT_WR, PADI_DEF_TH);
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_CTL, 0x1);    // Prepare start bit
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_CTL, 0x0);    //Start

  //set default tdc clock source
  WriteGosip (fSFP, fSlave, REG_TAM_CLK_SEL, CLK_SRC_TDC_TAM2);

  // set default trigger window:
  int l_trig_wind = (TRIG_WIN_EN << 31) + (POST_TRIG_TIME << 16) + PRE_TRIG_TIME;
  WriteGosip (fSFP, fSlave, REG_TAM_TRG_WIN, l_trig_wind);

  WriteGosip (fSFP, fSlave, REG_TAM_CTRL, 0x7c20d0);    // set reset bit

  // clear reset & set CNTRL_REG (CH0 enabled)
  int l_enable_or = 1;
  int l_combine_or = 1;
  WriteGosip (fSFP, fSlave, REG_TAM_CTRL, 0x7c20c0 | l_enable_or | l_combine_or);

  printm ("Did Initialize TAMEXPADI for SFP %d Slave %d", fSFP, fSlave);
}

void TamexPadiGui::DumpSlave ()
{
  if (!AssertChainConfigured ())
    return;
  printm ("###### SFP %d DEV:%d :)", fSFP, fSlave);
  fTamexDumpMode = true;
  GetRegisters ();

  fTamexDumpMode = false;
}

void TamexPadiGui::ApplyFileConfig (int)
{
  GosipGui::ApplyFileConfig (900);    // adjust bus wait time to 900 us
}

void TamexPadiGui::ApplyThreshold (int channel, int val)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  uint16_t thres = val;
  theSetup->SetDACValue (channel, thres);    // keep setup structure consistent

  SetThreshold (channel, thres);

}

void TamexPadiGui::ApplyThresholdToAll (int val)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  uint16_t thres = val;
  for (int c = 0; c < TAMEX_TDC_NUMCHAN; ++c)
    theSetup->SetDACValue (c, thres);    // keep setup structure consistent

  uint16_t values[TAMEX_PADI_NUMCHIPS] = { 0, 0 };
  for (int p = 0; p < TAMEX_PADI_NUMCHIPS; ++p)
  {
    values[p] = val;
  }
  // broadcast the settings to the chip (with channel 8)
  if (!WriteDAC_Padi (8, values))
  {
    printm ("SetThreshold has error setting value 0x%x to all channels", val);
    return;
  }
}

void TamexPadiGui::VoltageModeCheckBoxChanged (int on)
{
  GOSIP_LOCK_SLOT
  //std::cout<< "VoltageModeCheckBoxChanged to "<<on << std::endl;
  fShowAmplifiedVoltages = on;

  // change ranges of spinboxes here:
  int min = on ? -600.0 : -3.0;
  int max = on ? 600.0 : 3.0;
  double step = on ? 1.0 : 0.005;
  int decimals = on ? 0 : 3;

  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
  {
    // adjust spinbox ranges:
    fThresholdSpinBoxes[chan]->setRange (min, max);
    fThresholdSpinBoxes[chan]->setSingleStep (step);
    fThresholdSpinBoxes[chan]->setDecimals (decimals);

    // also refresh the display depending on the slider values:
    int regval = fThresholdSlider[chan]->value ();
    double volts = Register2Voltage (regval);
    fThresholdSpinBoxes[chan]->setValue (volts);

  }

  // don't forget the all spinbox here-
  fTamexPadiWidget->Threshold_doubleSpinBox_all->setRange (min, max);
  fTamexPadiWidget->Threshold_doubleSpinBox_all->setSingleStep (step);
  fTamexPadiWidget->Threshold_doubleSpinBox_all->setDecimals (decimals);
  int allval = fTamexPadiWidget->Threshold_Slider_all->value ();
  double allvolts = Register2Voltage (allval);
  fTamexPadiWidget->Threshold_doubleSpinBox_all->setValue (allvolts);

  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::Threshold_doubleSpinBox_all_changed (double voltage)
{
  GOSIP_LOCK_SLOT
  //std::cout<<"Threshold_doubleSpinBox_all_changed with voltage "<< voltage <<std::endl;
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  int regval = Voltage2Register (voltage);
  fTamexPadiWidget->Threshold_Slider_all->setValue (regval);
  fTamexPadiWidget->Threshold_Value_all->setText (pre + text.setNum (regval, fNumberBase));

  // since we lock the qt slots, we have to handle distribution of single elements manually:
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
  {
    fThresholdSlider[chan]->setValue (regval);
    fThresholdLineEdit[chan]->setText (pre + text.setNum (regval, fNumberBase));
    fThresholdSpinBoxes[chan]->setValue (voltage);

    // too time consuming:
    //GOSIP_AUTOAPPLY(ApplyThreshold(chan, regval));
  }    // for
  // better use the padi broadcast feature:
  GOSIP_AUTOAPPLY(ApplyThresholdToAll(regval));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::Threshold_doubleSpinBox_changed (int channel, double voltage)
{
  GOSIP_LOCK_SLOT
  //std::cout<<"Threshold_doubleSpinBox_changed  for "<<channel<<" with voltage "<< voltage <<std::endl;
  // here just forward voltage change to register change (slider):
  int regval = Voltage2Register (voltage);
  fThresholdSlider[channel]->setValue (regval);
  // here forward changes to the line edit text
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  fThresholdLineEdit[channel]->setText (pre + text.setNum (regval, fNumberBase));

  // since we lock the Qt slots to avoid prelling, any element has to take care to set the autoapply
  GOSIP_AUTOAPPLY(ApplyThreshold(channel, regval));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::Threshold_doubleSpinBox_00_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (0, voltage);
}

void TamexPadiGui::Threshold_doubleSpinBox_01_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (1, voltage);
}

void TamexPadiGui::Threshold_doubleSpinBox_02_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (2, voltage);
}

void TamexPadiGui::Threshold_doubleSpinBox_03_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (3, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_04_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (4, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_05_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (5, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_06_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (6, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_07_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (7, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_08_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (8, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_09_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (9, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_10_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (10, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_11_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (11, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_12_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (12, voltage);
}

void TamexPadiGui::Threshold_doubleSpinBox_13_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (13, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_14_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (14, voltage);
}
void TamexPadiGui::Threshold_doubleSpinBox_15_changed (double voltage)
{
  Threshold_doubleSpinBox_changed (15, voltage);
}

///////////////// threshold with register settings:

void TamexPadiGui::Threshold_Slider_all_changed (int regval)
{
  GOSIP_LOCK_SLOT
  //std::cout<<"Threshold_Slider_all_changed with value "<< regval <<std::endl;

  // also keep other "all" fields up to date:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  fTamexPadiWidget->Threshold_Value_all->setText (pre + text.setNum (regval, fNumberBase));

  // forward changes to the voltage spinbox:
  double volts = Register2Voltage (regval);
  fTamexPadiWidget->Threshold_doubleSpinBox_all->setValue (volts);

  // since we lock the qt slots, we have to handle distribution of single elements manually:
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
  {
    fThresholdSlider[chan]->setValue (regval);
    fThresholdLineEdit[chan]->setText (pre + text.setNum (regval, fNumberBase));
    fThresholdSpinBoxes[chan]->setValue (volts);

    // does work, but is time consuming:
    //GOSIP_AUTOAPPLY(ApplyThreshold(chan, regval));
  }
  // better use the padi broadcast feature:
  GOSIP_AUTOAPPLY(ApplyThresholdToAll(regval));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::Threshold_Slider_changed (int channel, int regval)
{
  GOSIP_LOCK_SLOT
  //std::cout<<"Threshold_Slider_changed for "<<channel<<" with value "<< regval <<std::endl;
  // here forward changes to the line edit text
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  fThresholdLineEdit[channel]->setText (pre + text.setNum (regval, fNumberBase));
  // forward changes to the voltage spinbox:
  double volts = Register2Voltage (regval);
  fThresholdSpinBoxes[channel]->setValue (volts);
  // since we lock the Qt slots to avoid prelling, any element has to take care to set the autoapply
  GOSIP_AUTOAPPLY(ApplyThreshold(channel, regval));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::Threshold_Slider_00_changed (int regval)
{
  Threshold_Slider_changed (0, regval);
}

void TamexPadiGui::Threshold_Slider_01_changed (int regval)
{
  Threshold_Slider_changed (1, regval);
}

void TamexPadiGui::Threshold_Slider_02_changed (int regval)
{
  Threshold_Slider_changed (2, regval);
}

void TamexPadiGui::Threshold_Slider_03_changed (int regval)
{
  Threshold_Slider_changed (3, regval);
}
void TamexPadiGui::Threshold_Slider_04_changed (int regval)
{
  Threshold_Slider_changed (4, regval);
}
void TamexPadiGui::Threshold_Slider_05_changed (int regval)
{
  Threshold_Slider_changed (5, regval);
}
void TamexPadiGui::Threshold_Slider_06_changed (int regval)
{
  Threshold_Slider_changed (6, regval);
}
void TamexPadiGui::Threshold_Slider_07_changed (int regval)
{
  Threshold_Slider_changed (7, regval);
}
void TamexPadiGui::Threshold_Slider_08_changed (int regval)
{
  Threshold_Slider_changed (8, regval);
}
void TamexPadiGui::Threshold_Slider_09_changed (int regval)
{
  Threshold_Slider_changed (9, regval);
}
void TamexPadiGui::Threshold_Slider_10_changed (int regval)
{
  Threshold_Slider_changed (10, regval);
}
void TamexPadiGui::Threshold_Slider_11_changed (int regval)
{
  Threshold_Slider_changed (11, regval);
}
void TamexPadiGui::Threshold_Slider_12_changed (int regval)
{
  Threshold_Slider_changed (12, regval);
}

void TamexPadiGui::Threshold_Slider_13_changed (int regval)
{
  Threshold_Slider_changed (13, regval);
}
void TamexPadiGui::Threshold_Slider_14_changed (int regval)
{
  Threshold_Slider_changed (14, regval);
}
void TamexPadiGui::Threshold_Slider_15_changed (int regval)
{
  Threshold_Slider_changed (15, regval);
}

void TamexPadiGui::Threshold_Text_all_changed ()
{
  GOSIP_LOCK_SLOT
  QString txt = fTamexPadiWidget->Threshold_Value_all->text ();
  //std::cout<<"Threshold_Text_all_changed with text"<< txt.toLatin1 ().constData ()<<std::endl;
  bool ok;
  int val = txt.toInt (&ok, fNumberBase);
  if (!ok)
    return;
  double volts = Register2Voltage (val);

  // first keep the other "all" elements up to date:
  fTamexPadiWidget->Threshold_Slider_all->setValue (val);
  fTamexPadiWidget->Threshold_doubleSpinBox_all->setValue (volts);

  // since we lock the qt slots, we have to handle distribution of single elements manually:
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
  {
    fThresholdSlider[chan]->setValue (val);
    fThresholdLineEdit[chan]->setText (txt);
    fThresholdSpinBoxes[chan]->setValue (volts);
    // too time consuming:
    //GOSIP_AUTOAPPLY(ApplyThreshold(chan, val));
  }
  // better use the padi broadcast feature:
  GOSIP_AUTOAPPLY(ApplyThresholdToAll(val));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::Threshold_Text_changed (int channel)
{
  GOSIP_LOCK_SLOT
  QString txt = fThresholdLineEdit[channel]->text ();
  //std::cout<<"Threshold_Text_changed for "<<channel<<" with text "<< txt.toLatin1 ().constData () <<std::endl;
  // forward text as value to the register slider (will invoke the auto apply if set)
  bool ok;
  int val = txt.toInt (&ok, fNumberBase);
  //std::cout<<"Threshold_Text_changed has value "<<val<<std::endl;

  if (ok)
    fThresholdSlider[channel]->setValue (val);
  double volts = Register2Voltage (val);
  fThresholdSpinBoxes[channel]->setValue (volts);
  // since we lock the Qt slots to avoid prelling, any element has to take care to set the autoapply
  GOSIP_AUTOAPPLY(ApplyThreshold(channel, val));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::Threshold_Text_00_changed ()
{
  Threshold_Text_changed (0);
}

void TamexPadiGui::Threshold_Text_01_changed ()
{
  Threshold_Text_changed (1);
}

void TamexPadiGui::Threshold_Text_02_changed ()
{
  Threshold_Text_changed (2);
}

void TamexPadiGui::Threshold_Text_03_changed ()
{
  Threshold_Text_changed (3);
}

void TamexPadiGui::Threshold_Text_04_changed ()
{
  Threshold_Text_changed (4);
}

void TamexPadiGui::Threshold_Text_05_changed ()
{
  Threshold_Text_changed (5);
}

void TamexPadiGui::Threshold_Text_06_changed ()
{
  Threshold_Text_changed (6);
}

void TamexPadiGui::Threshold_Text_07_changed ()
{
  Threshold_Text_changed (7);
}
void TamexPadiGui::Threshold_Text_08_changed ()
{
  Threshold_Text_changed (8);
}

void TamexPadiGui::Threshold_Text_09_changed ()
{
  Threshold_Text_changed (9);
}

void TamexPadiGui::Threshold_Text_10_changed ()
{
  Threshold_Text_changed (10);
}

void TamexPadiGui::Threshold_Text_11_changed ()
{
  Threshold_Text_changed (11);
}

void TamexPadiGui::Threshold_Text_12_changed ()
{
  Threshold_Text_changed (12);
}

void TamexPadiGui::Threshold_Text_13_changed ()
{
  Threshold_Text_changed (13);
}

void TamexPadiGui::Threshold_Text_14_changed ()
{
  Threshold_Text_changed (14);
}

void TamexPadiGui::Threshold_Text_15_changed ()
{
  Threshold_Text_changed (15);
}

/////////////////////////////////////////////////////////////////////////////////////7
/////////////// Channel enable/disable section

void TamexPadiGui::ApplyChannelEnabled (int channel, int leading, int trailing)
{

  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  // -1 means do not change the existing setup for this channel. 0 means disable, 1 means enable bit.
  //std::cout<<"ApplyChannelEnabled ch:"<<channel<<", leading:"<<leading<<", trailing:"<<trailing << std::endl;
  if (leading >= 0)
    theSetup->SetChannelLeadingEnabled (channel, leading > 0);
  if (trailing >= 0)
    theSetup->SetChannelTrailingEnabled (channel, trailing > 0);
  SetTDCsEnabledChannels ();

}



void TamexPadiGui::ApplyTriggerEnabled(int channel, bool on)
{
  //std::cout<<"ApplyTriggerEnabled ch:"<<channel<<", on:"<<on << std::endl;

  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  theSetup->SetChannelTriggerEnabled (channel, on);
  SetTDCsTriggerChannels ();


}

void TamexPadiGui::ApplyInputPolarity(int channel, bool positive)
{
 //std::cout<<"ApplyInputPolarity ch:"<<channel<<", positive:"<<positive << std::endl;
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  theSetup->SetChannelPolarityPositive(channel, positive);
  SetTDCsPolarity();
}



void TamexPadiGui::ApplyChannelEnabledAll (bool on)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
  {
    bool leading = false;
    bool trailing = false;
    if (on)
    {
      leading = fChannelLeadingRadio[chan]->isChecked ();
      trailing = fChannelTrailingRadio[chan]->isChecked ();
    }
    theSetup->SetChannelLeadingEnabled (chan, leading);
    theSetup->SetChannelTrailingEnabled (chan, trailing);

  }
  SetTDCsEnabledChannels ();

}

void TamexPadiGui::ApplyLeadingEnabledAll (bool on)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
    theSetup->SetChannelLeadingEnabled (chan, on);
  SetTDCsEnabledChannels ();
}

void TamexPadiGui::ApplyTrailingEnabledAll (bool on)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
    theSetup->SetChannelTrailingEnabled (chan, on);
  SetTDCsEnabledChannels ();
}


void TamexPadiGui::ApplyTriggerEnabledAll (bool on)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  //std::cout<<"ApplyTriggerEnabledAll for on= "<< on << std::endl;
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
  {
    theSetup->SetChannelTriggerEnabled (chan, on);
  }
  SetTDCsTriggerChannels ();

}



void TamexPadiGui::ApplyInputPolarityAll (bool on)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
  {
    theSetup->SetChannelPolarityPositive(chan, on);
  }
  SetTDCsPolarity();

}



void TamexPadiGui::ChannelEnabled_toggled (int channel, bool on)
{
  GOSIP_LOCK_SLOT
  bool leading = false, trailing = false;
  if (on)
  {
    leading = fChannelLeadingRadio[channel]->isChecked ();
    trailing = fChannelTrailingRadio[channel]->isChecked ();
  }
  GOSIP_AUTOAPPLY(ApplyChannelEnabled(channel, (leading ? 1:0), (trailing ? 1:0 )));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::ChannelEnabled_toggled_00 (bool on)
{
  ChannelEnabled_toggled (0, on);
}
void TamexPadiGui::ChannelEnabled_toggled_01 (bool on)
{
  ChannelEnabled_toggled (1, on);
}
void TamexPadiGui::ChannelEnabled_toggled_02 (bool on)
{
  ChannelEnabled_toggled (2, on);
}
void TamexPadiGui::ChannelEnabled_toggled_03 (bool on)
{
  ChannelEnabled_toggled (3, on);
}
void TamexPadiGui::ChannelEnabled_toggled_04 (bool on)
{
  ChannelEnabled_toggled (4, on);
}
void TamexPadiGui::ChannelEnabled_toggled_05 (bool on)
{
  ChannelEnabled_toggled (5, on);
}
void TamexPadiGui::ChannelEnabled_toggled_06 (bool on)
{
  ChannelEnabled_toggled (6, on);
}
void TamexPadiGui::ChannelEnabled_toggled_07 (bool on)
{
  ChannelEnabled_toggled (7, on);
}
void TamexPadiGui::ChannelEnabled_toggled_08 (bool on)
{
  ChannelEnabled_toggled (8, on);
}
void TamexPadiGui::ChannelEnabled_toggled_09 (bool on)
{
  ChannelEnabled_toggled (9, on);
}
void TamexPadiGui::ChannelEnabled_toggled_10 (bool on)
{
  ChannelEnabled_toggled (10, on);
}
void TamexPadiGui::ChannelEnabled_toggled_11 (bool on)
{
  ChannelEnabled_toggled (11, on);
}
void TamexPadiGui::ChannelEnabled_toggled_12 (bool on)
{
  ChannelEnabled_toggled (12, on);
}
void TamexPadiGui::ChannelEnabled_toggled_13 (bool on)
{
  ChannelEnabled_toggled (13, on);
}
void TamexPadiGui::ChannelEnabled_toggled_14 (bool on)
{
  ChannelEnabled_toggled (14, on);
}
void TamexPadiGui::ChannelEnabled_toggled_15 (bool on)
{
  ChannelEnabled_toggled (15, on);
}

void TamexPadiGui::ChannelEnabled_toggled_all (bool on)
{
  GOSIP_LOCK_SLOT
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
    fChannelEnabledBox[chan]->setChecked (on);
  // since we lock the slots of the radiobuttons, we handle the autoapply here separately (and better):
  GOSIP_AUTOAPPLY(ApplyChannelEnabledAll(on));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::LeadingEnabled_toggled (int channel, bool on)
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyChannelEnabled(channel, on, -1));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::ChannelLeading_toggled_00 (bool on)
{
  LeadingEnabled_toggled (0, on);
}
void TamexPadiGui::ChannelLeading_toggled_01 (bool on)
{
  LeadingEnabled_toggled (1, on);
}
void TamexPadiGui::ChannelLeading_toggled_02 (bool on)
{
  LeadingEnabled_toggled (2, on);
}

void TamexPadiGui::ChannelLeading_toggled_03 (bool on)
{
  LeadingEnabled_toggled (3, on);
}

void TamexPadiGui::ChannelLeading_toggled_04 (bool on)
{
  LeadingEnabled_toggled (4, on);
}
void TamexPadiGui::ChannelLeading_toggled_05 (bool on)
{
  LeadingEnabled_toggled (5, on);
}
void TamexPadiGui::ChannelLeading_toggled_06 (bool on)
{
  LeadingEnabled_toggled (6, on);
}
void TamexPadiGui::ChannelLeading_toggled_07 (bool on)
{
  LeadingEnabled_toggled (7, on);
}
void TamexPadiGui::ChannelLeading_toggled_08 (bool on)
{
  LeadingEnabled_toggled (8, on);
}
void TamexPadiGui::ChannelLeading_toggled_09 (bool on)
{
  LeadingEnabled_toggled (9, on);
}
void TamexPadiGui::ChannelLeading_toggled_10 (bool on)
{
  LeadingEnabled_toggled (10, on);
}
void TamexPadiGui::ChannelLeading_toggled_11 (bool on)
{
  LeadingEnabled_toggled (11, on);
}
void TamexPadiGui::ChannelLeading_toggled_12 (bool on)
{
  LeadingEnabled_toggled (12, on);
}
void TamexPadiGui::ChannelLeading_toggled_13 (bool on)
{
  LeadingEnabled_toggled (13, on);
}

void TamexPadiGui::ChannelLeading_toggled_14 (bool on)
{
  LeadingEnabled_toggled (14, on);
}

void TamexPadiGui::ChannelLeading_toggled_15 (bool on)
{
  LeadingEnabled_toggled (15, on);
}

void TamexPadiGui::ChannelLeading_toggled_all (bool on)
{
  GOSIP_LOCK_SLOT
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
    fChannelLeadingRadio[chan]->setChecked (on);
  // since we lock the slots of the radiobuttons, we handle the autoapply here separately (and better):
  GOSIP_AUTOAPPLY(ApplyLeadingEnabledAll(on));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::TrailingEnabled_toggled (int channel, bool on)
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyChannelEnabled(channel, -1, on ));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::ChannelTrailing_toggled_00 (bool on)
{
  TrailingEnabled_toggled (0, on);
}
void TamexPadiGui::ChannelTrailing_toggled_01 (bool on)
{
  TrailingEnabled_toggled (1, on);
}
void TamexPadiGui::ChannelTrailing_toggled_02 (bool on)
{
  TrailingEnabled_toggled (2, on);
}
void TamexPadiGui::ChannelTrailing_toggled_03 (bool on)
{
  TrailingEnabled_toggled (3, on);
}
void TamexPadiGui::ChannelTrailing_toggled_04 (bool on)
{
  TrailingEnabled_toggled (4, on);
}
void TamexPadiGui::ChannelTrailing_toggled_05 (bool on)
{
  TrailingEnabled_toggled (5, on);
}
void TamexPadiGui::ChannelTrailing_toggled_06 (bool on)
{
  TrailingEnabled_toggled (6, on);
}
void TamexPadiGui::ChannelTrailing_toggled_07 (bool on)
{
  TrailingEnabled_toggled (7, on);
}
void TamexPadiGui::ChannelTrailing_toggled_08 (bool on)
{
  TrailingEnabled_toggled (8, on);
}
void TamexPadiGui::ChannelTrailing_toggled_09 (bool on)
{
  TrailingEnabled_toggled (9, on);
}
void TamexPadiGui::ChannelTrailing_toggled_10 (bool on)
{
  TrailingEnabled_toggled (10, on);
}
void TamexPadiGui::ChannelTrailing_toggled_11 (bool on)
{
  TrailingEnabled_toggled (11, on);
}
void TamexPadiGui::ChannelTrailing_toggled_12 (bool on)
{
  TrailingEnabled_toggled (12, on);
}
void TamexPadiGui::ChannelTrailing_toggled_13 (bool on)
{
  TrailingEnabled_toggled (13, on);
}
void TamexPadiGui::ChannelTrailing_toggled_14 (bool on)
{
  TrailingEnabled_toggled (14, on);
}
void TamexPadiGui::ChannelTrailing_toggled_15 (bool on)
{
  TrailingEnabled_toggled (15, on);
}

void TamexPadiGui::ChannelTrailing_toggled_all (bool on)
{
  GOSIP_LOCK_SLOT
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
    fChannelTrailingRadio[chan]->setChecked (on);
  // since we lock the slots of the radiobuttons, we handle the autoapply here separately (and better):
  GOSIP_AUTOAPPLY(ApplyTrailingEnabledAll(on));
  GOSIP_UNLOCK_SLOT
}

//// BEGIN COMSTRUCTION, JAM at work
////////////////////////////////////////////////////////////////////\/////////////////////////////////
void TamexPadiGui::TriggerEnabled_toggled (int channel, bool on)
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyTriggerEnabled(channel, on));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::TriggerEnabled_toggled_00 (bool on)
{
  TriggerEnabled_toggled (0, on);
}
void TamexPadiGui::TriggerEnabled_toggled_01 (bool on)
{
  TriggerEnabled_toggled (1, on);
}
void TamexPadiGui::TriggerEnabled_toggled_02 (bool on)
{
  TriggerEnabled_toggled (2, on);
}
void TamexPadiGui::TriggerEnabled_toggled_03 (bool on)
{
  TriggerEnabled_toggled (3, on);
}
void TamexPadiGui::TriggerEnabled_toggled_04 (bool on)
{
  TriggerEnabled_toggled (4, on);
}
void TamexPadiGui::TriggerEnabled_toggled_05 (bool on)
{
  TriggerEnabled_toggled (5, on);
}
void TamexPadiGui::TriggerEnabled_toggled_06 (bool on)
{
  TriggerEnabled_toggled (6, on);
}
void TamexPadiGui::TriggerEnabled_toggled_07 (bool on)
{
  TriggerEnabled_toggled (7, on);
}
void TamexPadiGui::TriggerEnabled_toggled_08 (bool on)
{
  TriggerEnabled_toggled (8, on);
}
void TamexPadiGui::TriggerEnabled_toggled_09 (bool on)
{
  TriggerEnabled_toggled (9, on);
}
void TamexPadiGui::TriggerEnabled_toggled_10 (bool on)
{
  TriggerEnabled_toggled (10, on);
}
void TamexPadiGui::TriggerEnabled_toggled_11 (bool on)
{
  TriggerEnabled_toggled (11, on);
}
void TamexPadiGui::TriggerEnabled_toggled_12 (bool on)
{
  TriggerEnabled_toggled (12, on);
}
void TamexPadiGui::TriggerEnabled_toggled_13 (bool on)
{
  TriggerEnabled_toggled (13, on);
}
void TamexPadiGui::TriggerEnabled_toggled_14 (bool on)
{
  TriggerEnabled_toggled (14, on);
}
void TamexPadiGui::TriggerEnabled_toggled_15 (bool on)
{
  TriggerEnabled_toggled (15, on);
}

void TamexPadiGui::TriggerEnabled_toggled_all (bool on)
{
  GOSIP_LOCK_SLOT
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
    fChannelTriggerEnabCheck[chan]->setChecked (on);
  // since we lock the slots of the radiobuttons, we handle the autoapply here separately (and better):
  GOSIP_AUTOAPPLY(ApplyTriggerEnabledAll(on));
  GOSIP_UNLOCK_SLOT
}


//////////////////////////////////////////////////////////////////77


void TamexPadiGui::InputPolarity_toggled (int channel, int index )
{
  GOSIP_LOCK_SLOT
  //std::cout << "TamexPadiGui::InputPolarity_toggled, channel="<<channel<<", index="<<index<<std::endl;
  RefreshPolaritySelector(fChannelPolarityBox[channel], (index==0));
  GOSIP_AUTOAPPLY(ApplyInputPolarity(channel, (index==0)));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::InputPolarity_toggled_00 (int index)
{
  InputPolarity_toggled (0, index);
}
void TamexPadiGui::InputPolarity_toggled_01 (int index)
{
  InputPolarity_toggled (1, index);
}
void TamexPadiGui::InputPolarity_toggled_02 (int index)
{
  InputPolarity_toggled (2, index);
}
void TamexPadiGui::InputPolarity_toggled_03 (int index)
{
  InputPolarity_toggled (3, index);
}
void TamexPadiGui::InputPolarity_toggled_04 (int index)
{
  InputPolarity_toggled (4, index);
}
void TamexPadiGui::InputPolarity_toggled_05 (int index)
{
  InputPolarity_toggled (5, index);
}
void TamexPadiGui::InputPolarity_toggled_06 (int index)
{
  InputPolarity_toggled (6, index);
}
void TamexPadiGui::InputPolarity_toggled_07 (int index)
{
  InputPolarity_toggled (7, index);
}
void TamexPadiGui::InputPolarity_toggled_08 (int index)
{
  InputPolarity_toggled (8, index);
}
void TamexPadiGui::InputPolarity_toggled_09 (int index)
{
  InputPolarity_toggled (9, index);
}
void TamexPadiGui::InputPolarity_toggled_10 (int index)
{
  InputPolarity_toggled (10, index);
}
void TamexPadiGui::InputPolarity_toggled_11 (int index)
{
  InputPolarity_toggled (11, index);
}
void TamexPadiGui::InputPolarity_toggled_12 (int index)
{
  InputPolarity_toggled (12, index);
}
void TamexPadiGui::InputPolarity_toggled_13 (int index)
{
  InputPolarity_toggled (13, index);
}
void TamexPadiGui::InputPolarity_toggled_14 (int index)
{
  InputPolarity_toggled (14, index);
}
void TamexPadiGui::InputPolarity_toggled_15 (int index)
{
  InputPolarity_toggled (15, index);
}

void TamexPadiGui::InputPolarity_toggled_all (int index)
{
  GOSIP_LOCK_SLOT
  RefreshPolaritySelector(fTamexPadiWidget->PolarityComboBoxAll, (index==0));
  for (int chan = 0; chan < TAMEX_TDC_NUMCHAN; ++chan)
    {
      fChannelPolarityBox[chan]->setCurrentIndex (index);
      RefreshPolaritySelector(fChannelPolarityBox[chan], (index==0));
    }
    // since we lock the slots of the radiobuttons, we handle the autoapply here separately (and better):
  GOSIP_AUTOAPPLY(ApplyInputPolarityAll(index==0));
  GOSIP_UNLOCK_SLOT
}





///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////// END CONSTRUCTION



void TamexPadiGui::ApplyTriggerwindow ()
{
  //std::cout << "TamexPadiGui::ApplyTriggerwindow"<<std::endl;
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  theSetup->SetEnabledTriggerWindow (fTamexPadiWidget->TriggerwindowGroupBox->isChecked ());
  theSetup->SetPreTriggerWindow (fTamexPadiWidget->PreTriggerSpinBox->value ());
  theSetup->SetPostTriggerWindow (fTamexPadiWidget->PostTriggerSpinBox->value ());
  SetTriggerWindow ();
}

void TamexPadiGui::PreTriggerSpinBox_changed (int val)
{
  int nanos = val * 5;
  fTamexPadiWidget->PreTriggerNanosecLabel->setText (QString ("%1 ns").arg (nanos));
  GOSIP_LOCK_SLOT
  // avoid that we auto apply again the setting when refreshview sets the ns label
  GOSIP_AUTOAPPLY(ApplyTriggerwindow());
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::PostTriggerSpinBox_changed (int val)
{
  int nanos = val * 5;
  fTamexPadiWidget->PostTriggerNanosecLabel->setText (QString ("%1 ns").arg (nanos));
  GOSIP_LOCK_SLOT
  // avoid that we auto apply again the setting when refreshview sets the ns label
  GOSIP_AUTOAPPLY(ApplyTriggerwindow());
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::TriggerWindowGroupBox_toggled (bool)
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyTriggerwindow());
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::ApplyClockSource (int index)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  int clk = ComboIndex2ClockSource (index);
  theSetup->SetClockSource (clk);
  SetClockSource ();

}

void TamexPadiGui::ClockSourceCurrentIndexChanged (int index)
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyClockSource(index));
  GOSIP_UNLOCK_SLOT
}

void TamexPadiGui::ApplyBoardType (Tamex_Module_type mod)
{
  //std::cout << "TamexPadiGui::ApplyBoardType for "<< mod <<std::endl;
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  theSetup->SetModuleType(mod);
}


void  TamexPadiGui::BoardTypeComboBoxurrentIndexChanged (int ix)
{
  GOSIP_LOCK_SLOT
  //std::cout << "TamexPadiGui::BoardTypeComboBoxurrentIndexChanged to "<<ix<<std::endl;
  Tamex_Module_type mod = BoardIndex2ModuleType(ix);
  RefreshClockSourceList(mod);
  RefreshModuleCaps(mod);
  GOSIP_AUTOAPPLY(ApplyBoardType(mod));
  GOSIP_UNLOCK_SLOT

}


void TamexPadiGui::ApplyTriggerOutAndRef ()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  theSetup->SetEnableOR (fTamexPadiWidget->EnableOrCheckBox->isChecked ());
  theSetup->SetCombineOR (fTamexPadiWidget->CombineOrCheckBox->isChecked ());
  theSetup->SetEnableTriggerReferenceChannel (fTamexPadiWidget->Ch0RefRadioButton->isChecked ());
  SetLemoTriggerOut ();
}

void TamexPadiGui::TriggerOutChanged ()
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyTriggerOutAndRef());
  GOSIP_UNLOCK_SLOT
}


 void TamexPadiGui::RefreshPolaritySelector(QComboBox* ctrl, bool ispositive)
 {
   ctrl->setStyleSheet(
            ispositive ? "QComboBox { color: black; background-color: red; }":
                "QComboBox { color: white; background-color: blue; }"
        );
 }


void TamexPadiGui::RefreshClockSourceList (Tamex_Module_type mod)

{
  for (int i=0; i<TAMEX_NUM_CLK_SRC; ++i)
    {
      QString txt=ModuleType2ComboText(mod,i);
      //if(txt.isEmpty()) continue; // better override previous entry with empty string
      fTamexPadiWidget->ClockSourceComboBox->setItemText(i, txt);
    }
}

void TamexPadiGui::RefreshModuleCaps (Tamex_Module_type mod)
{
  fTamexPadiWidget->ChannelTriggerGroupBox->setEnabled(mod==TamMod_TAMEXPADI1);
  fTamexPadiWidget->TamexPadiTabs->setTabEnabled(2,mod!=TamMod_TAMEX3);
  fTamexPadiWidget->TriggerOutgroupBox->setEnabled(mod!=TamMod_TAMEX3);
}


void TamexPadiGui::RefreshView ()
{

  //std::cout << "TamexPadiGui::RefreshView"<<std::endl;
// display setup structure to gui:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  Tamex_Module_type module =theSetup->GetModuleType ();
  RefreshModuleCaps(module);
  int bix=ModuleType2BoardIndex(module);
  fTamexPadiWidget->BoardTypeComboBox->setCurrentIndex(bix);
  for (uint8_t channel = 0; channel < TAMEX_TDC_NUMCHAN; ++channel)
  {
    uint16_t thres = theSetup->GetDACValue (channel);
    double thresvoltage = Register2Voltage (thres);

    //std::cout << "TamexPadiGui::RefreshView with thres("<< (int) channel<<")="<< thres<<" ->"<<thresvoltage<<" mV"<<std::endl;

    fThresholdLineEdit[channel]->setText (pre + text.setNum (thres, fNumberBase));
    fThresholdSlider[channel]->setValue (thres);
    fThresholdSpinBoxes[channel]->setValue (thresvoltage);

    bool leading = theSetup->IsChannelLeadingEnabled (channel);
    bool trailing = theSetup->IsChannelTrailingEnabled (channel);
    bool ispositive = theSetup->IsChannelPolarityPositive(channel);

    if (!leading && !trailing)
    {
      fChannelEnabledBox[channel]->setChecked (false);
      // channel completely disabled: leave the subradiobuttons as they were.
    }
    else
    {
      // any really set bits will set channel box to active
      fChannelEnabledBox[channel]->setChecked (true);
      fChannelLeadingRadio[channel]->setChecked (leading);
      fChannelTrailingRadio[channel]->setChecked (trailing);
      fChannelPolarityBox[channel]->setCurrentIndex(ispositive ? 0 : 1);
      RefreshPolaritySelector(fChannelPolarityBox[channel], ispositive);
    }

    bool enabtrig = theSetup->IsChannelTriggerEnabled (channel); // check of boardtype is done in setup class
    fChannelTriggerEnabCheck[channel]->setChecked(enabtrig);



  }

  // put chip version to header of the padi boxes:
  int v0 = theSetup->GetPadiVersion (0);
  fTamexPadiWidget->PADIgroupBox_0->setTitle (QString ("PADI 0 - version 0x%1").arg (v0, 0, 16));
  int v1 = theSetup->GetPadiVersion (1);
  fTamexPadiWidget->PADIgroupBox_1->setTitle (QString ("PADI 1 - version 0x%1").arg (v1, 0, 16));

  // now the tdc control registers:
  int trigpre = theSetup->GetPreTriggerWindow ();
  fTamexPadiWidget->PreTriggerSpinBox->setValue (trigpre);
  int trigpost = theSetup->GetPostTriggerWindow ();
  fTamexPadiWidget->PostTriggerSpinBox->setValue (trigpost);
  // ^hopefully the changed signal of above will refresh also the ns Labels?

  fTamexPadiWidget->TriggerwindowGroupBox->setChecked (theSetup->IsEnabledTriggerWindow ());

  fTamexPadiWidget->Ch0RefRadioButton->setChecked (theSetup->IsTriggerReferenceChannel ());
  fTamexPadiWidget->EnableOrCheckBox->setChecked (theSetup->IsEnableOR ());
  fTamexPadiWidget->CombineOrCheckBox->setChecked (theSetup->IsCombineOR ());

  RefreshClockSourceList(module);
  int ix = ClockSource2ComboIndex (theSetup->GetClockSource ());
  fTamexPadiWidget->ClockSourceComboBox->setCurrentIndex (ix);
  // when switching from hex to dec mode or vice versa, have to update the all value field:
  int allval = fTamexPadiWidget->Threshold_Slider_all->value ();
  fTamexPadiWidget->Threshold_Value_all->setText (pre + text.setNum (allval, fNumberBase));




  GosipGui::RefreshView ();
  // ^this handles the refresh of chains and status. better use base class function here! JAM2018
}

void TamexPadiGui::EvaluateView ()
{
  //std::cout << "TamexPadiGui::EvaluateView"<<std::endl;

  // here the current gui display is just copied to setup structure in local memory
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);

  for (uint8_t channel = 0; channel < TAMEX_TDC_NUMCHAN; ++channel)
  {
    uint16_t thres = fThresholdSlider[channel]->value ();
    theSetup->SetDACValue (channel, thres);
    theSetup->SetChannelLeadingEnabled (channel, fChannelLeadingRadio[channel]->isChecked ());
    theSetup->SetChannelTrailingEnabled (channel, fChannelTrailingRadio[channel]->isChecked ());
    theSetup->SetChannelPolarityPositive(channel, (fChannelPolarityBox[channel]->currentIndex() == 0) );

  }

  theSetup->SetEnabledTriggerWindow (fTamexPadiWidget->TriggerwindowGroupBox->isChecked ());
  theSetup->SetPreTriggerWindow (fTamexPadiWidget->PreTriggerSpinBox->value ());
  theSetup->SetPostTriggerWindow (fTamexPadiWidget->PostTriggerSpinBox->value ());

  int bix=fTamexPadiWidget->BoardTypeComboBox->currentIndex();
  Tamex_Module_type mod = BoardIndex2ModuleType(bix);
  theSetup->SetModuleType (mod); // TODO Later, when the board type is evaluated by register read, this needs to removed JAM

  int index = fTamexPadiWidget->ClockSourceComboBox->currentIndex ();
  int clk = ComboIndex2ClockSource (index);
  theSetup->SetClockSource (clk);

  theSetup->SetEnableOR (fTamexPadiWidget->EnableOrCheckBox->isChecked ());
  theSetup->SetCombineOR (fTamexPadiWidget->CombineOrCheckBox->isChecked ());
  theSetup->SetEnableTriggerReferenceChannel (fTamexPadiWidget->Ch0RefRadioButton->isChecked ());

}

void TamexPadiGui::SetRegisters ()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);

  QApplication::setOverrideCursor (Qt::WaitCursor);

  // apply PADI thresholds:
  uint16_t values[TAMEX_PADI_NUMCHIPS] = { 0, 0 };
  for (int c = 0; c < TAMEX_PADI_NUMCHAN; ++c)
  {
    //printm ("TamexPadiGui::GetRegisters channel %d sees thresholds 0x%x and 0x%x\n", c, values[0], values[1]);
    for (int p = 0; p < TAMEX_PADI_NUMCHIPS; ++p)
    {
      values[p] = theSetup->GetDACValue (p, c);
    }
    if (!WriteDAC_Padi (c, values))
    {
      printm ("SetRegisters has error writing PADI channels %d", c);
      return;
    }
  }

  SetClockSource ();

  SetTriggerWindow ();

  SetLemoTriggerOut ();

  SetTDCsEnabledChannels ();

  SetTDCsTriggerChannels();

  SetTDCsPolarity();

  QApplication::restoreOverrideCursor ();

}

void TamexPadiGui::SetThreshold (uint8_t globalchannel, uint16_t value)
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  int chip = 0, chan = 0;
  uint16_t values[TAMEX_PADI_NUMCHIPS] = { 0, 0 };
  theSetup->EvaluateDACIndices (globalchannel, chip, chan);
  for (int p = 0; p < TAMEX_PADI_NUMCHIPS; ++p)
  {
    values[p] = theSetup->GetDACValue (p, chan);    // get current settings of both chip
  }
  values[chip] = value;    // override changed value of current chip
  if (!WriteDAC_Padi (chan, values))
  {
    printm ("SetThreshold has error writing channel %d of PADI %d", chan, chip);
    return;
  }

}

void TamexPadiGui::SetTDCsEnabledChannels ()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  // set tdc channels enabled/disabled:
  int regenabled = theSetup->GetEnabledRegister ();
  //printm("SetTDCsEnabledChannels writes enab:0x%x",regenabled);
  WriteGosip (fSFP, fSlave, REG_TAM_EN_1, regenabled);





}

void TamexPadiGui::SetTDCsTriggerChannels ()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  if (theSetup->GetModuleType () == TamMod_TAMEXPADI1)
  {
    int regenabled = theSetup->GetTriggerEnabledRegister ();
    printm ("SetTDCsTriggerChannels writes 0x%x", regenabled);
    WriteGosip (fSFP, fSlave, REG_TAM_EN_TR, regenabled);
  }
  else
  {
    printm ("SetTDCsTriggerChannels is only supported for TAMEX-PADI1!");
  }

}

void TamexPadiGui::SetTDCsPolarity()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  int regenabled = theSetup->GetPolarityRegister ();
  printm("SetTDCsPolarity writes 0x%x",regenabled);
  WriteGosip (fSFP, fSlave, REG_TAM_POLARITY, regenabled);
}



void TamexPadiGui::SetLemoTriggerOut ()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  int reset = theSetup->IsTriggerReferenceChannel () ? COM_CTRL_REFCHAN_RESET : COM_CTRL_NOREF_RESET;
  int apply = theSetup->IsTriggerReferenceChannel () ? COM_CTRL_REFCHAN_APPLY : COM_CTRL_NOREF_APPLY;
  Tamex_Module_type module =theSetup->GetModuleType ();
  if(module != TamMod_TAMEX3)
  {
    int enable_or = theSetup->IsEnableOR () ? COM_CTRL_ENABLE_OR_BIT : 0;
    int combine_or = theSetup->IsCombineOR () ? COM_CTRL_COMBINE_OR_BIT : 0;
    apply = apply | enable_or | combine_or;
  }
  WriteGosip (fSFP, fSlave, REG_TAM_CTRL, reset);    // reset TDC
  WriteGosip (fSFP, fSlave, REG_TAM_CTRL, apply);    // set TDC control register

}

void TamexPadiGui::SetClockSource ()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  Tamex_Module_type module =theSetup->GetModuleType ();
  int clksrc = theSetup->GetClockSource ();
  switch(module)
  {
    case TamMod_TAMEX2:
        if (clksrc == 0x22)
        {
          WriteGosip (fSFP, fSlave, REG_TAM_CLK_SEL, clksrc - 1);    // // set tdc clock source
          if (fSlave == 0)    // If clock from TRBus used
          {
            WriteGosip (fSFP, fSlave, REG_TAM_BUS_EN, 0x80);    // Enable feeding clock to TRBus on slave 0
          }
        }
        else
        {
          WriteGosip (fSFP, fSlave, REG_TAM_CLK_SEL, clksrc);
          if (fSlave == 0)
          {
            // we should disable the clock feeding to trbus here when we switch back?
            int busregister = ReadGosip (fSFP, fSlave, REG_TAM_BUS_EN);
            if ((busregister & 0x80) == 0x80)
            {
              busregister &= ~0x80;
              WriteGosip (fSFP, fSlave, REG_TAM_BUS_EN, busregister);
              // probably too detailed, but we do not know what else is set in this register...
            }
          }
        }
   break;

   case TamMod_TAMEXPADI1:
    default:
      if ((clksrc == 0x4) && (fSlave == 0))
      {
        WriteGosip (fSFP, fSlave, REG_TAM_CLK_SEL, 0x1);    // // Enable local clock oscillator on slave 0 only
      }
      else  if ((clksrc == 0x8) && (fSlave == 0))
      {
        WriteGosip (fSFP, fSlave, REG_TAM_CLK_SEL, 0x2);    // Enable ext clock from front on slave 0 only
      }
      else
      {
        WriteGosip (fSFP, fSlave, REG_TAM_CLK_SEL, clksrc);    // set TAMEX-PADI1 clock source
      }
    break;

    case TamMod_TAMEX3:

      WriteGosip (fSFP, fSlave, REG_TAM_CLK_SEL, clksrc);    // set TAMEX3 clock source
      if ((clksrc == 0x22))// If clock from TRBus used
           {
              if((fSlave == 0))
              WriteGosip (fSFP, fSlave, REG_TAM_BUS_EN, 0x80);    // Enable feeding clock to TRBus on slave 0
           }
      else
      {
        if (fSlave == 0)
        {
          // we should disable the clock feeding to trbus here when we switch back?
          int busregister = ReadGosip (fSFP, fSlave, REG_TAM_BUS_EN);
          if ((busregister & 0x80) == 0x80)
          {
            busregister &= ~0x80;
            WriteGosip (fSFP, fSlave, REG_TAM_BUS_EN, busregister);
            // probably too detailed, but we do not know what else is set in this register...
          }
        }
      }
      break;

  };

///////////////////////////////////////////////////////////////////////////////////////////////////
//  if ( (l_sfp_tam_mode[l_i] == 1) || (l_sfp_tam_mode[l_i] == 2) ) // Set TAMEX2 clock source
//      {
//
//        if (CLK_SRC_TDC_TAM2 == 0x22) // special case clock distribution via TRBus
//        {
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_CLK_SEL, 0x21);
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" Setting clock source failed\n");
//            l_err_prot_ct++;
//          }
//        }
//        else
//        {
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_CLK_SEL, CLK_SRC_TDC_TAM2);
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" Setting clock source failed\n");
//            l_err_prot_ct++;
//          }
//        }
//
//        if ((CLK_SRC_TDC_TAM2 == 0x22) && (l_j == 0)) // If clock from TRBus used
//        {
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_BUS_EN, 0x80); // Enable feeding clock to TRBus on slave 0
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" Enabling TRBus CLK on slave 0 failed\n");
//            l_err_prot_ct++;
//          }
//        }
//      }
//      else if (l_sfp_tam_mode[l_i] == 3) // Set TAMEX-PADI1 clock source
//      {
//        if ((CLK_SRC_TDC_TAM_PADI1 == 0x4) && (l_j == 0))
//        {
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_CLK_SEL, 0x1); // Enable local clock oscillator on slave 0 only
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" Setting clock source failed\n");
//            l_err_prot_ct++;
//          }
//        }
//        else if ((CLK_SRC_TDC_TAM_PADI1 == 0x8) && (l_j == 0))
//        {
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_CLK_SEL, 0x2); // Enable ext clock from front on slave 0 only
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" Setting clock source failed\n");
//            l_err_prot_ct++;
//          }
//        }
//        else
//        {
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_CLK_SEL, CLK_SRC_TDC_TAM_PADI1); // Set TAMEX-PADI1 clock source
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" Setting clock source failed\n");
//            l_err_prot_ct++;
//          }
//        }
//      }
//      else if (l_sfp_tam_mode[l_i] == 10) // Set TAMEX3 clock source
//      {
//        l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_CLK_SEL, CLK_SRC_TDC_TAM3);
//        if (l_stat == -1)
//        {
//          printm (RON"ERROR>>"RES" Setting clock source failed\n");
//          l_err_prot_ct++;
//        }
//        if ((CLK_SRC_TDC_TAM3 == 0x22) && (l_j == 0)) // If clock from TRBus used
//        {
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_BUS_EN, 0x80); // Enable feeding clock to TRBus on slave 0
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" Enabling TRBus CLK on slave 0 failed\n");
//            l_err_prot_ct++;
//          }
//        }
//      }
//////////////////////////////////////////////////////////////////////////////////////////////////7


}

void TamexPadiGui::SetTriggerWindow ()
{
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);

//////////////////////////////7
  // static  long  l_trig_wind = (TRIG_WIN_EN << 31) + (POST_TRIG_TIME << 16) + PRE_TRIG_TIME;
//  printm ("trigger window: 0x%x \n", l_trig_wind); // set trigger window
//         l_stat = f_pex_slave_wr (l_i, l_j, REG_TAM_TRG_WIN, l_trig_wind);
//         if (l_stat == -1)
//         {
//           printm (RON"ERROR>>"RES" Setting TDC trigger window failed\n");
//           l_err_prot_ct++;
//         }
/////////////////////////

  int windowenabled = theSetup->IsEnabledTriggerWindow () ? (1 << 31) : 0;
  int pre = theSetup->GetPreTriggerWindow () & 0xFFFF;
  int post = (theSetup->GetPostTriggerWindow () & 0xFFFF) << 16;

  WriteGosip (fSFP, fSlave, REG_TAM_TRG_WIN, (windowenabled | pre | post));
}

void TamexPadiGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  theSetup_GET_FOR_SLAVE(TamexPadiSetup);
  Tamex_Module_type boardtype=theSetup->GetModuleType(); // TODO - get actual board type from hardware register!!!!

  QApplication::setOverrideCursor (Qt::WaitCursor);
  //std::cout << "TamexPadiGui::GetRegisters()"<<std::endl;

  //////////////////////////////////////////////
  // here read channel enabled status register:
  int regenabled = ReadGosip (fSFP, fSlave, REG_TAM_EN_1);
  TAMEX_PRINT_DUMP("Channel enabled register \t0x%x", regenabled);
  theSetup->SetEnabledRegister (regenabled);


  //////////////////////////////////////////////
   // here read channel trigger enabled register:
  if(boardtype == TamMod_TAMEXPADI1)
  {
   int trigenabled = ReadGosip (fSFP, fSlave, REG_TAM_EN_TR);
   TAMEX_PRINT_DUMP("Channel trigger enabled register \t0x%x", trigenabled);
   theSetup->SetTriggerEnabledRegister(trigenabled);
  }
  else
  {
    TAMEX_PRINT_DUMP("Channel trigger enabled register NOT READ, no TAMEX-PADI1 board!");
  }

  //////////////////////////////////////////////
  // here read input polarity register:
  int polar = ReadGosip (fSFP, fSlave, REG_TAM_POLARITY);
  TAMEX_PRINT_DUMP("Channel polarity register \t0x%x", polar);
  theSetup->SetPolarityRegister (polar);


  //////////////////////////////////////////////
  // trigger windows:
  int trigwinreg = ReadGosip (fSFP, fSlave, REG_TAM_TRG_WIN);
  TAMEX_PRINT_DUMP("Trigger window register \t0x%x", trigwinreg);

  bool windowenabled = ((trigwinreg & (1 << 31)) == (1 << 31));
  int pretime = trigwinreg & 0x7FFF;
  int posttime = (trigwinreg >> 16) & 0x7FFF;
  theSetup->SetEnabledTriggerWindow (windowenabled);
  theSetup->SetPreTriggerWindow (pretime);
  theSetup->SetPostTriggerWindow (posttime);

  /////////////////////////////////////////////////////////////////////////////////7
  // clock source:
  int clockreg = ReadGosip (fSFP, fSlave, REG_TAM_CLK_SEL);
  TAMEX_PRINT_DUMP("Clock source register  \t0x%x", clockreg);
  int clk = clockreg & 0xFF;
  switch(boardtype)
  {
    case TamMod_TAMEX2:
      if (clk == 0x21)
      {
        // check here if we have setup with trbus clock feed
        int busclock = ReadGosip (fSFP, 0, REG_TAM_BUS_EN);

        bool istrbus = (busclock & 0x80) == 0x80 ? true : false;
        if (istrbus)
          clk = 0x22;
      }

    break;
    default:

    break;

  }
  theSetup->SetClockSource (clk);

  //////////////////////////////////////////////////////////////////////////
  // lemo output and reference channel:
  int control = ReadGosip (fSFP, fSlave, REG_TAM_CTRL);
  TAMEX_PRINT_DUMP("Tamex control register \t0x%x", control);

  bool hasrefchannel = ((control & COM_CTRL_REFCHAN_APPLY) == COM_CTRL_REFCHAN_APPLY ? true : false);
  bool enable_or = ((control & COM_CTRL_ENABLE_OR_BIT)== COM_CTRL_ENABLE_OR_BIT ? true : false);bool
  combine_or = ((control & COM_CTRL_COMBINE_OR_BIT)== COM_CTRL_COMBINE_OR_BIT ? true : false);

  theSetup
  ->SetEnableTriggerReferenceChannel (hasrefchannel);


  theSetup->SetEnableOR (enable_or);
  theSetup->SetCombineOR (combine_or);




  if (boardtype != TamMod_TAMEX3)
  {

    /////////////////////////////////////////////////////////////////////////////
    // threshold set via PADI dacs:
    EnableSPI ();
    TAMEX_PRINT_DUMP("______________________________");
    TAMEX_PRINT_DUMP("   DAC threshold settings:    ");
    TAMEX_PRINT_DUMP("Channel \t| PADI-0 \t| PADI-1 ");
    TAMEX_PRINT_DUMP("--------\t+------  \t+--------");

    // we read the channels of both padis in parallel:
    uint16_t values[TAMEX_PADI_NUMCHIPS] = { 0, 0 };
    PrepareReadDAC_Padi (0);

    for (int c = 0; c < TAMEX_PADI_NUMCHAN; ++c)
    {
      PrepareReadDAC_Padi (c + 1);
      if (!ReadDAC_Padi (values))
      {
        printm ("GetRegisters has error reading PADI channels %d", c);
        return;
      }
      TAMEX_PRINT_DUMP("  %d    \t| 0x%x    \t| 0x%x ", c, values[0], values[1]);
      for (int p = 0; p < TAMEX_PADI_NUMCHIPS; ++p)
      {
        theSetup->SetDACValue (p, c, values[p]);
      }
    }
    // after last channel, we read the chip version which is available as channel 8:
    PrepareReadDAC_Padi (0);    // dummy to shift result out. prepares next channel 0, but should not harm
    ReadDAC_Padi (values);

    TAMEX_PRINT_DUMP("Version\t| 0x%x    \t| 0x%x", values[0], values[1]);
    for (int p = 0; p < TAMEX_PADI_NUMCHIPS; ++p)
    {
      theSetup->SetPadiVersion (p, values[p]);
    }
    DisableSPI ();
  }
  else
  {
    TAMEX_PRINT_DUMP("TAMEX3 does not support PADI thresholds.");
  }

  QApplication::restoreOverrideCursor ();
}

bool TamexPadiGui::WriteDAC_Padi (uint8_t chan, uint16_t value_padi[TAMEX_PADI_NUMCHIPS])
{
  if (chan > TAMEX_PADI_NUMCHAN)
    return false;
  // note that for chan==TAMEX_PADI_NUMCHAN, we will broadcast settings to all DACs
  int com = COM_TAM_PADI_WRITE;
  com |= ((chan & 0xF) << 10);
  com |= ((chan & 0xF) << 26);
  com |= (value_padi[0] & 0x3FF);
  com |= (value_padi[1] & 0x3FF) << 16;
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_DAT_WR, com);    //  Load data register for transmission
  PadiSPISleep ();
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_CTL, 0x1);    // Prepare start bit
  PadiSPISleep ();
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_CTL, 0x0);    //Start
  PadiSPISleep ();
  return true;
}

bool TamexPadiGui::PrepareReadDAC_Padi (uint8_t chan)
{
  if (chan > TAMEX_PADI_NUMCHAN)
    return false;
  // note that for chan==TAMEX_PADI_NUMCHAN, we read out chipversion
  int com = COM_TAM_PADI_READ;
  com |= ((chan & 0xF) << 10);
  com |= ((chan & 0xF) << 26);

  WriteGosip (fSFP, fSlave, REG_TAM_PADI_DAT_WR, com);    //  Prepare reading channel data
  PadiSPISleep ();    // give additional delay
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_CTL, 0x1);    // Prepare start bit
  PadiSPISleep ();
  WriteGosip (fSFP, fSlave, REG_TAM_PADI_CTL, 0x0);    //Start
  PadiSPISleep ();
  return true;
}

bool TamexPadiGui::ReadDAC_Padi (uint16_t (&value_padi)[TAMEX_PADI_NUMCHIPS])
{
  int val = ReadGosip (fSFP, fSlave, REG_TAM_PADI_DAT_RD);    // read back the current shift register value
  PadiSPISleep ();
  if (val == -1)
    return false;
  value_padi[0] = val & 0x3FF;
  value_padi[1] = (val >> 16) & 0x3FF;
  //printm("ReadDAC_Padi gets values 0x%x and 0x%x", value_padi[0],  value_padi[1]);

  return true;
}

double TamexPadiGui::Register2Voltage (unsigned int regval)
{
  double rev = 0;
  double m = 1200.0 / 1024.0;    // mV per reg count
  rev = -600.0 + m * regval;
  if (!fShowAmplifiedVoltages)
    rev /= 200.0;
  //std::cout<<"Register2Voltage converts "<<regval<<" to "<< rev<<" mV" <<std::endl;
  return rev;
}

unsigned int TamexPadiGui::Voltage2Register(double voltage)
{
  unsigned int rev = 0;
  double m = 1024.0 / 1200.0;    // reg count per mV
  if (!fShowAmplifiedVoltages)
    m *= 200.0;
  rev = m * voltage + 511;
  //std::cout<<"Voltage2Register converts "<<voltage<<" mV to "<< rev <<std::endl;
  return rev;
}

int TamexPadiGui::ClockSource2ComboIndex (int clk)
{
  // TODO: handle all these assignments with one global maps and pairs later

  theSetup_GET_FOR_SLAVE_RETURN(TamexPadiSetup);
  int ix = 0;
  Tamex_Module_type mod=theSetup->GetModuleType();
  switch(mod)
  {
    case TamMod_TAMEX2:
      switch (clk)
        {
          case 0x20:
            ix = 0;
            break;
          case 0x21:
            ix = 1;
            break;
          case 0x22:
            ix = 2;
            break;
          case 0x24:
          default:
            ix = 3;
            break;
        };
    break;
    case TamMod_TAMEXPADI1:
    default:
       switch (clk)
       {
         case 0x0:
           ix = 0;
           break;
         case 0x1:
           ix = 1;
           break;
         case 0x2:
           ix = 2;
           break;
         case 0x4:
           ix = 3;
           break;
         case 0x8:
         default:
           ix = 4;
           break;
       };
    break;

   case TamMod_TAMEX3:
          switch (clk)
          {
            case 0x26:
              ix = 0;
              break;
            case 0x2a:
            default:
              ix = 1;
              break;
            case 0x22:
              ix = 2;
              break;
            case 0x20:
              ix = 3;
              break;
          };
   break;
  };
  return ix;
}

int TamexPadiGui::ComboIndex2ClockSource (int index)
{
  // TODO: handle all these assignments with one global maps and pairs later
  int clk = 0;
  theSetup_GET_FOR_SLAVE_RETURN(TamexPadiSetup);
  Tamex_Module_type mod = theSetup->GetModuleType ();

  switch (mod)
  {
    case TamMod_TAMEX2:
      switch (index)
      {
        case 0:
          clk = 0x20;
          break;
        case 1:
          clk = 0x21;
          break;
        case 2:
          clk = 0x22;
          break;
        case 3:
        default:
          clk = 0x24;
          break;
      }
      ;
      break;
    case TamMod_TAMEXPADI1:
    default:
      switch (index)
      {
        case 0:
          clk = 0x0;
          break;
        case 1:
          clk = 0x1;
          break;
        case 2:
          clk = 0x2;
          break;
        case 3:
        default:
          clk = 0x4;
          break;
        case 4:
          clk = 0x8;
          break;

      }
      ;
      break;
    case TamMod_TAMEX3:
      switch (index)
      {
        case 0:
          clk = 0x26;
          break;
        case 1:
          clk = 0x2a;
          break;
        case 2:
          clk = 0x22;
          break;
        case 3:
        default:
          clk = 0x20;
          break;

      }
      ;
      break;
  };
  return clk;
}


int TamexPadiGui::ModuleType2BoardIndex (Tamex_Module_type mod)
{
  // TODO: handle this with global map later
  int ix = 0;
  switch (mod)
  {
    case TamMod_TAMEX2:
      ix = 0;
      break;
    case TamMod_TAMEXPADI1:
      ix = 1;
      break;
    case TamMod_TAMEX3:
      ix = 2;
      break;
    default:
      ix = 1;
      break;
  }
  return ix;
}

Tamex_Module_type TamexPadiGui::BoardIndex2ModuleType (int index)
{
  // TODO: handle this with global map later
  Tamex_Module_type mod = TamMod_NONE;
  switch (index)
  {
    case 0:
      mod = TamMod_TAMEX2;
      break;
    case 1:
      mod = TamMod_TAMEXPADI1;
      break;
    case 2:
      mod = TamMod_TAMEX3;
      break;
    default:
      mod = TamMod_NONE;
      break;
  };
  return mod;
}


QString TamexPadiGui::ModuleType2ComboText(Tamex_Module_type mod, int ix)
{
  return fgClockSourceText[mod][ix];
}


