#ifndef GeneralNyxorWidget_H
#define GeneralNyxorWidget_H

#include <QWidget>

#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QLineEdit>

#include <stdint.h>
#include <stdio.h>

#include "ui_generalwidget.h"

// time unit (LSB) in nanoseconds for delay and windows:
#define NYXOR_TIME_UNIT_NS 31.25


class NyxorGui;

class NyxorReceiverCoreRegisters
{
  public:

  uint16_t   fNXControl; //< nxyter control register
  uint16_t   fTriggerPre; //< Pre  trigger time
  uint16_t   fTriggerPost; //< Post trigger time
  uint8_t   fDelayTestPulse; //< Test pulse delay time
  uint8_t   fDelayTrigger; //< Trigger delay time
  uint16_t   fTestCodeADC; //< ADC test code
  uint16_t   fTestCode1; //< NX value1 test code
  uint16_t   fTestCode2; //< NX value2 test code


  NyxorReceiverCoreRegisters(): fNXControl(0), fTriggerPre(0),fTriggerPost(0),fDelayTestPulse(0), fDelayTrigger(0),fTestCodeADC(0),
      fTestCode1(0),fTestCode2(0) {}

  void Dump(){
    printf ("-----Nyxor Receiver core register dump:");
      printf ("NxControl:  \t0x%x,\n", fNXControl);
      printf ("TriggerPre: \t0x%x,\n", fTriggerPre);
      printf ("TriggerPost: \t0x%x,\n", fTriggerPost);
      printf ("DelayTestPulse: \t0x%x,\n", fDelayTestPulse);
      printf ("DelayTrigger: \t0x%x,\n", fDelayTrigger);
      printf ("TestCodeADC: \t0x%x,\n", fTestCodeADC);
      printf ("TestCode1: \t0x%x,\n", fTestCode1);
      printf ("TestCode2: \t0x%x,\n", fTestCode2);

  }

};


class GeneralNyxorWidget : public QWidget , public Ui::GeneralNyxorWidget {
   Q_OBJECT


   protected:

   NyxorGui* fxOwner;



   /** auxiliary references to checkboxes for control register bits*/
    QCheckBox* fControlBitBoxes[14];


   public:

    NyxorReceiverCoreRegisters fSetup;

     GeneralNyxorWidget(QWidget* parent, NyxorGui* owner);

     void GetRegisters();

     void SetRegisters();

     /** update register display, regard decimal or hex number base*/
     void RefreshView ();

     /** refresh bits of control register*/
     void RefreshControlBits();

     /** copy values from gui to internal status object*/
     void EvaluateView ();


   public slots:


   void TriggerPreSpinBox_changed(double);
   void TriggerPostSpinBox_changed(double);
   void SecondTestPulseDelaySpinBox_changed(double);
   void TestAcquisitionTriggerDelaySpinBox_changed(double);

   void TriggerPreLineEdit_finished();
   void TriggerPostLineEdit_finished();
   void SecondTestPulseDelayLineEdit_finished();
   void TestAcquisitionTriggerDelayLineEdit_finished();


   void ControlBit_clicked(bool);
   void nxControlEdit_finished();

};

#endif
