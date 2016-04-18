#include "adcwidget.h"

#include "NyxorGui.h"

NyxorADCWidget::NyxorADCWidget(QWidget* parent, NyxorGui* owner):
   QWidget(parent),fxOwner(owner)
{

    setupUi(this);

    // map to designer widget names:
    fADCTransmitLineEdit[0]= Transmit1LineEdit;
    fADCTransmitLineEdit[1]= Transmit2LineEdit;
    fADCTransmitLineEdit[2]= Transmit3LineEdit;
    fADCTransmitLineEdit[3]= Transmit4LineEdit;


    QObject::connect (DC0PhaselineEdit, SIGNAL(editingFinished()), this, SLOT (AnyLineEdit_finished()));
    for (int i = 0; i < NUM_ADC_TRANSMITREGS; ++i)
      {
        QObject::connect (fADCTransmitLineEdit[i], SIGNAL(editingFinished()), this, SLOT (AnyLineEdit_finished()));
      }


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

  if(!fSetup.fAnyChange) return;

  //fxOwner->DisableI2C();
  fxOwner->EnableSPI();
  if(fSetup.fDC0Phase_Changed)
    fxOwner->WriteNyxorSPI(SPI_ADC_DC0PHASE, fSetup.fDC0Phase);
  for(int i=0; i<NUM_ADC_TRANSMITREGS;++i)
    {
      if(fSetup.fTransmit_Changed[i])
        fxOwner->WriteNyxorSPI(SPI_ADC_PATTERNBASE + i, fSetup.fTransmit[i]);
    }
   fxOwner->DisableSPI();
   fSetup.ResetChanged();
}

void NyxorADCWidget::RefreshView ()
{
  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  DC0PhaselineEdit->setText(pre+text.setNum (fSetup.fDC0Phase, numberbase));
  for(int i=0; i<NUM_ADC_TRANSMITREGS;++i)
      {
        fADCTransmitLineEdit[i]->setText(pre+text.setNum (fSetup.fTransmit[i], numberbase));

      }
  fSetup.ResetChanged();
}

void NyxorADCWidget::EvaluateView ()
{
  int numberbase = fxOwner->GetNumberBase ();

  int val = 0;
  val = DC0PhaselineEdit->text ().toUInt (0, numberbase);
  if (val != fSetup.fDC0Phase)
  {
    fSetup.fAnyChange = true;
    fSetup.fDC0Phase_Changed = true;
    fSetup.fDC0Phase = val;
  }
  for (int i = 0; i < NUM_ADC_TRANSMITREGS; ++i)
  {

    val = fADCTransmitLineEdit[i]->text ().toUInt (0, numberbase);
    if (val != fSetup.fTransmit[i])
    {
      fSetup.fAnyChange = true;
      fSetup.fTransmit_Changed[i] = true;
      fSetup.fTransmit[i] = val;
    }
  }

}


void NyxorADCWidget::AnyLineEdit_finished ()
{
  if(fxOwner->IsAutoApply())
  {
      EvaluateView();
      SetRegisters();
  }
}
