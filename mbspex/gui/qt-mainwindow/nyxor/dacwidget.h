#ifndef DACWidget_H
#define DACWidget_H

#include <QWidget>

//#include <QSpinBox>
//#include <QSlider>
//#include <QCheckBox>
//#include <QLineEdit>

#include <stdint.h>
#include <stdio.h>

#include "ui_dacwidget.h"



//class NyxorGui;
//
//#define NYXOR_NUMNX 2

#include "NyxorGui.h"

#define NUM_NYXORDACS 4

class NyxorDACRegisters
{
  public:

  uint16_t   fRegister[NYXOR_NUMNX][NUM_NYXORDACS]; //< external DAC registers
  bool       fChanged[NYXOR_NUMNX][NUM_NYXORDACS]; //< mark what has changed since last show
  bool       fAnyChange; //< true if anything has changed at all

  NyxorDACRegisters()
  {
    for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
    {
      for (int i = 0; i < NUM_NYXORDACS; ++i)
      {
        fRegister[nx][i] = 0;
      }
    }
    ResetChanged();
  }

  void Dump ()
  {
    printf ("-----Nyxor DAC register dump:\n");
    for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
    {
      for (int i = 0; i < NUM_NYXORDACS; ++i)
      {
        printf ("DAC_%d=0x%x\n", i, fRegister[nx][i]);
      }
    }
  }

  void ResetChanged()
  {
    fAnyChange=false;
    for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
        {
          for (int i = 0; i < NUM_NYXORDACS; ++i)
          {
            fChanged[nx][i] = false;
          }
        }
  }

};


class NyxorDACWidget : public QWidget , public Ui::NyxorDACWidget {
   Q_OBJECT


   protected:

   NyxorGui* fxOwner;

   QSpinBox*  fDACSpinBox[NYXOR_NUMNX][NUM_NYXORDACS];
   QLineEdit* fDACLineEdit[NYXOR_NUMNX][NUM_NYXORDACS];

   bool fSupressSignals; // avoid further signals when changing spinboxes at RefreshView

   public:

   NyxorDACRegisters fSetup;

   NyxorDACWidget(QWidget* parent, NyxorGui* owner);

     void GetRegisters();

     void SetRegisters(bool force=false);

     /** update register display, regard decimal or hex number base*/
     void RefreshView ();


     /** copy values from gui to internal status object*/
     void EvaluateView ();


   public slots:

     void DACSpinBox_changed(int );
     void DACLineEdit_finished();

};

#endif
