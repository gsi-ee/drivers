#include "adcwidget.h"

#include "NyxorGui.h"

NyxorADCWidget::NyxorADCWidget(QWidget* parent, NyxorGui* owner):
   QWidget(parent),fxOwner(owner)
{

    setupUi(this);

}


void NyxorADCWidget::GetRegisters()
{
  //printf("NyxorADCWidget::GetRegisters()...\n");
  //fxOwner->DisableI2C();
  fxOwner->EnableSPI();

  fSetup.fDC0Phase=fxOwner->ReadNyxorSPI(SPI_ADC_DC0PHASE);
  for(int i=0; i<NUM_ADC_TRANSMITREGS;++i)
  {
    fSetup.fTransmit[i]=fxOwner->ReadNyxorSPI(SPI_ADC_PATTERNBASE + i);
  }

  fxOwner->DisableSPI();

  //fSetup.Dump();
}

void NyxorADCWidget::SetRegisters()
{
  //printf("NyxorADCWidget::SetRegisters()...\n");
  //fSetup.Dump();

  //fxOwner->DisableI2C();
  fxOwner->EnableSPI();
  fxOwner->WriteNyxorSPI(SPI_ADC_DC0PHASE, fSetup.fDC0Phase);
  for(int i=0; i<NUM_ADC_TRANSMITREGS;++i)
    {
      fxOwner->WriteNyxorSPI(SPI_ADC_PATTERNBASE + i, fSetup.fTransmit[i]);
    }


   fxOwner->DisableSPI();
}

void NyxorADCWidget::RefreshView ()
{
  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  DC0PhaselineEdit->setText(pre+text.setNum (fSetup.fDC0Phase, numberbase));
  Transmit1LineEdit->setText(pre+text.setNum (fSetup.fTransmit[0], numberbase));
  Transmit2LineEdit->setText(pre+text.setNum (fSetup.fTransmit[1], numberbase));
  Transmit3LineEdit->setText(pre+text.setNum (fSetup.fTransmit[2], numberbase));
  Transmit4LineEdit->setText(pre+text.setNum (fSetup.fTransmit[3], numberbase));






}

void NyxorADCWidget::EvaluateView ()
{
  int numberbase=fxOwner->GetNumberBase();

  fSetup.fDC0Phase=DC0PhaselineEdit->text ().toUInt (0, numberbase);

  fSetup.fTransmit[0]=Transmit1LineEdit->text ().toUInt (0, numberbase);
  fSetup.fTransmit[1]=Transmit2LineEdit->text ().toUInt (0, numberbase);
  fSetup.fTransmit[2]=Transmit3LineEdit->text ().toUInt (0, numberbase);
  fSetup.fTransmit[3]=Transmit4LineEdit->text ().toUInt (0, numberbase);
}





