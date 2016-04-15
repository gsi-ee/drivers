#ifndef ADCWidget_H
#define ADCWidget_H

#include <QWidget>

//#include <QSpinBox>
//#include <QSlider>
//#include <QCheckBox>
//#include <QLineEdit>

#include <stdint.h>
#include <stdio.h>

#include "ui_adcwidget.h"



class NyxorGui;

#define NUM_ADC_TRANSMITREGS 4

class NyxorADCRegisters
{
  public:

  uint8_t   fTransmit[NUM_ADC_TRANSMITREGS]; //< ADC transmit registers
  uint8_t   fDC0Phase; // < dc0 phase register

  NyxorADCRegisters():fDC0Phase(0)
  { for(int i=0; i<NUM_ADC_TRANSMITREGS; ++i) { fTransmit[i]=0;}}

  void Dump(){
    printf ("-----Nyxor ADC register dump:\n");
    printf ("DC0 phase register = 0x%x\n", fDC0Phase);
    for(int i=0; i<NUM_ADC_TRANSMITREGS; ++i) {printf ("Transmit register%d = 0x%x\n", i, fTransmit[i]);}
  }

};


class NyxorADCWidget : public QWidget , public Ui::NyxorADCWidget {
   Q_OBJECT


   protected:

   NyxorGui* fxOwner;

   public:

   NyxorADCRegisters fSetup;

   NyxorADCWidget(QWidget* parent, NyxorGui* owner);

     void GetRegisters();

     void SetRegisters();

     /** update register display, regard decimal or hex number base*/
     void RefreshView ();


     /** copy values from gui to internal status object*/
     void EvaluateView ();


//   public slots:



};

#endif
