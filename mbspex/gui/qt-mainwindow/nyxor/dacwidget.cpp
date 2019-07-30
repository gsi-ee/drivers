#include "dacwidget.h"

#include "NyxorGui.h"

NyxorDACWidget::NyxorDACWidget(QWidget* parent, NyxorGui* owner):
   QWidget(parent),fxOwner(owner),fSupressSignals(false)
{

    setupUi(this);

    /** here map indexed references to designer widget names:*/
    fDACSpinBox[0][0]=NX0_DAC0_spinBox;
    fDACSpinBox[0][1]=NX0_DAC1_spinBox;
    fDACSpinBox[0][2]=NX0_DAC2_spinBox;
    fDACSpinBox[0][3]=NX0_DAC3_spinBox;
    fDACSpinBox[1][0]=NX1_DAC0_spinBox;
    fDACSpinBox[1][1]=NX1_DAC1_spinBox;
    fDACSpinBox[1][2]=NX1_DAC2_spinBox;
    fDACSpinBox[1][3]=NX1_DAC3_spinBox;

    fDACLineEdit[0][0]=NX0_DAC0_lineEdit;
    fDACLineEdit[0][1]=NX0_DAC1_lineEdit;
    fDACLineEdit[0][2]=NX0_DAC2_lineEdit;
    fDACLineEdit[0][3]=NX0_DAC3_lineEdit;
    fDACLineEdit[1][0]=NX1_DAC0_lineEdit;
    fDACLineEdit[1][1]=NX1_DAC1_lineEdit;
    fDACLineEdit[1][2]=NX1_DAC2_lineEdit;
    fDACLineEdit[1][3]=NX1_DAC3_lineEdit;

    for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
        {
          for (int i = 0; i < NUM_NYXORDACS; ++i)
          {
            QObject::connect (fDACSpinBox[nx][i], SIGNAL(valueChanged(int)), this, SLOT (DACSpinBox_changed(int)));
            QObject::connect (fDACLineEdit[nx][i], SIGNAL(editingFinished()), this, SLOT (DACLineEdit_finished()));
            // NOTE: we cannot pass the indices to the slots here? for the moment, all elements end in same slot and everything is refreshed on change.
          }
        }





}


void NyxorDACWidget::GetRegisters()
{
  //printf("NyxorDACWidget::GetRegisters()...\n");

  for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
       {
         fxOwner->EnableI2CRead(nx);
         for (int i = 0; i < NUM_NYXORDACS; ++i)
         {
           fSetup.fRegister[nx][i]=fxOwner->ReadNyxorDAC(nx, i);
         }
       }

  fxOwner->DisableI2C();



  //fSetup.Dump();
}

void NyxorDACWidget::SetRegisters(bool force)
{
  //printf("NyxorDACWidget::SetRegisters()...\n");
  //fSetup.Dump();
  if(!fSetup.fAnyChange && !force) return;
  for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
     {
       fxOwner->EnableI2CRead(nx); // note: also for writing the dacs, the nx chip "read mode" is used!
       for (int i = 0; i < NUM_NYXORDACS; ++i)
       {
         if(force || fSetup.fChanged[nx][i])
           fxOwner->WriteNyxorDAC(nx, i, fSetup.fRegister[nx][i]);
       }
     }

  fxOwner->DisableI2C();
  fSetup.ResetChanged();
}

void NyxorDACWidget::RefreshView ()
{
  //printf ("NyxorDACWidget::RefreshView ()\n");

  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  fSupressSignals=true;
  for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
   {
     for (int i = 0; i < NUM_NYXORDACS; ++i)
     {
       fDACLineEdit[nx][i]->setText (pre + text.setNum (fSetup.fRegister[nx][i], numberbase));
       fDACSpinBox[nx][i]->setValue (fSetup.fRegister[nx][i]);
     }
   }
  fSupressSignals=false;
  fSetup.ResetChanged();
}

void NyxorDACWidget::EvaluateView ()
{
  int numberbase=fxOwner->GetNumberBase();

  for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
   {
     for (int i = 0; i < NUM_NYXORDACS; ++i)
     {
       int val=fDACLineEdit[nx][i]->text ().toUInt (0, numberbase);
       if(fSetup.fRegister[nx][i]!=val)
         {
           fSetup.fRegister[nx][i] = val;
           fSetup.fChanged[nx][i] = true;
           fSetup.fAnyChange=true;
         }

     }
   }

}


void NyxorDACWidget::DACSpinBox_changed (int val)
{
  if (fSupressSignals)
    return;
  //printf ("DACSpinBox_changed for val:%d\n",val);
  QString pre;
  QString text;
  int numberbase = fxOwner->GetNumberBase ();
  numberbase == 16 ? pre = "0x" : pre = "";
  for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
  {
    for (int i = 0; i < NUM_NYXORDACS; ++i)
    {
      int val = fDACSpinBox[nx][i]->value ();
      if (fSetup.fRegister[nx][i] != val)
      {
        fSetup.fRegister[nx][i] = val;
        fSetup.fChanged[nx][i] = true;
        fSetup.fAnyChange = true;
        fSupressSignals = true;    // unneccessary here, since edit finished will not be send when text is changed programmatically. anyhow...
        fDACLineEdit[nx][i]->setText (pre + text.setNum (fSetup.fRegister[nx][i], numberbase));
        fSupressSignals = false;
      }
    }
  }
  if(fxOwner->IsAutoApply()) SetRegisters();

}

void NyxorDACWidget::DACLineEdit_finished ()
{
  if (fSupressSignals)
    return;
  //printf ("DACLineEdit_finished\n");
  int numberbase = fxOwner->GetNumberBase ();
  for (int nx = 0; nx < NYXOR_NUMNX; ++nx)
  {
    for (int i = 0; i < NUM_NYXORDACS; ++i)
    {
      int val = fDACLineEdit[nx][i]->text ().toUInt (0, numberbase);
      if (fSetup.fRegister[nx][i] != val)
      {
        fSetup.fRegister[nx][i] = val;
        fSetup.fChanged[nx][i] = true;
        fSetup.fAnyChange = true;

        fSupressSignals = true;
        fDACSpinBox[nx][i]->setValue (fSetup.fRegister[nx][i]);
        fSupressSignals = false;
      }
    }
  }
  if(fxOwner->IsAutoApply()) SetRegisters();

}


