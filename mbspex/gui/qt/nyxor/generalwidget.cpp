#include "generalwidget.h"

#include "NyxorGui.h"

GeneralNyxorWidget::GeneralNyxorWidget(QWidget* parent, NyxorGui* owner):
   QWidget(parent),fxOwner(owner)
{

    setupUi(this);

    QObject::connect (TriggerPreDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT (TriggerPreSpinBox_changed(double)));
    QObject::connect (TriggerPostDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT (TriggerPostSpinBox_changed(double)));
    QObject::connect (SecondTestPulseDelayDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT (SecondTestPulseDelaySpinBox_changed(double)));
    QObject::connect (TestAcquisitionTriggerDelayDoubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT (TestAcquisitionTriggerDelaySpinBox_changed(double)));
    QObject::connect (TriggerPreLineEdit, SIGNAL(editingFinished()), this, SLOT (TriggerPreLineEdit_finished()));
    QObject::connect (TriggerPostLineEdit, SIGNAL(editingFinished()), this, SLOT (TriggerPostLineEdit_finished()));
    QObject::connect (SecondTestPulseDelayLineEdit, SIGNAL(editingFinished()), this, SLOT (SecondTestPulseDelayLineEdit_finished()));
    QObject::connect (TestAcquisitionTriggerDelayLineEdit, SIGNAL(editingFinished()), this, SLOT (TestAcquisitionTriggerDelayLineEdit_finished()));



    // here mapping of the bit checkboxes to array:
    fControlBitBoxes[0]=nXTS_RSTBox;
    fControlBitBoxes[1]=RnXRecBox;
    fControlBitBoxes[2]=EnAcqTasTTBox;
    fControlBitBoxes[3]=EnSnXRecBox;
    fControlBitBoxes[4]=EnNAcqGBox;
    fControlBitBoxes[5]=EnTPAcqGBox;
    fControlBitBoxes[6]=EnRinTMBox;
    fControlBitBoxes[7]=EnXRBox;
    fControlBitBoxes[8]=GCtrlnx1Box;
    fControlBitBoxes[9]=GCtrlnx2Box;
    fControlBitBoxes[10]=ESPulsenX1Box;
    fControlBitBoxes[11]=ESPulsenX2Box;
    fControlBitBoxes[12]=ETestCode2Box;
    fControlBitBoxes[13]=ETestCode1Box;


    for(int b=0; b<14; ++b)
    {
      QObject::connect(fControlBitBoxes[b], SIGNAL(clicked(bool)), this, SLOT(ControlBit_clicked(bool)));
    }

    QObject::connect (nxControlEdit, SIGNAL(editingFinished()), this, SLOT (nxControlEdit_finished()));



}


void GeneralNyxorWidget::GetRegisters()
{
  //printf("GeneralNyxorWidget::GetRegisters()...\n");
  //fxOwner->DisableI2C();

  fSetup.fNXControl=fxOwner->ReadNyxorAddress(NXREC_CTRL_R);
  fSetup.fTriggerPre=fxOwner->ReadNyxorAddress(NXREC_PRETRIG_R);
  fSetup.fTriggerPost=fxOwner->ReadNyxorAddress(NXREC_POSTTRIG_R);
  fSetup.fDelayTestPulse=fxOwner->ReadNyxorAddress(NXREC_DELAY1_R);
  fSetup.fDelayTrigger=fxOwner->ReadNyxorAddress(NXREC_DELAY2_R);
  fSetup.fTestCodeADC=fxOwner->ReadNyxorAddress(NXREC_TESTCODE_ADC_R);
  fSetup.fTestCode1=fxOwner->ReadNyxorAddress(NXREC_TESTCODE_1_R);
  fSetup.fTestCode2=fxOwner->ReadNyxorAddress(NXREC_TESTCODE_2_R);


  //fSetup.Dump();
}

void GeneralNyxorWidget::SetRegisters()
{
  //printf("GeneralNyxorWidget::SetRegisters()...\n");
  //fSetup.Dump();
  if(!fSetup.fAnything_Changed) return;

  fxOwner->ReceiverReset();
  fxOwner->NXTimestampReset();
  //Note: The first step (The resetting of nXyter chip) has to be always executed.
  if(fSetup.fNXControl_Changed)
    fxOwner->WriteNyxorAddress(NXREC_CTRL_W, fSetup.fNXControl);
  if(fSetup.fTriggerPre_Changed)
    fxOwner->WriteNyxorAddress(NXREC_PRETRIG_W, fSetup.fTriggerPre);
  if(fSetup.fTriggerPost_Changed)
    fxOwner->WriteNyxorAddress(NXREC_POSTTRIG_W,fSetup.fTriggerPost);
  if(fSetup.fDelayTestPulse_Changed)
    fxOwner->WriteNyxorAddress(NXREC_DELAY1_W, fSetup.fDelayTestPulse);
  if(fSetup.fDelayTrigger_Changed)
    fxOwner->WriteNyxorAddress(NXREC_DELAY2_W, fSetup.fDelayTrigger);
  if(fSetup.fTestCodeADC_Changed)
    fxOwner->WriteNyxorAddress(NXREC_TESTCODE_ADC_W, fSetup.fTestCodeADC);
  if(fSetup.fTestCode1_Changed)
    fxOwner->WriteNyxorAddress(NXREC_TESTCODE_1_W, fSetup.fTestCode1);
  if(fSetup.fTestCode2_Changed)
    fxOwner->WriteNyxorAddress(NXREC_TESTCODE_2_W, fSetup.fTestCode2);

  fSetup.ResetChanged();
}

void GeneralNyxorWidget::RefreshView ()
{
  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  nxControlEdit->setText(pre+text.setNum (fSetup.fNXControl, numberbase));
  RefreshControlBits();


  TriggerPreLineEdit->setText(pre+text.setNum (fSetup.fTriggerPre, numberbase));
  TriggerPreDoubleSpinBox->setValue(NYXOR_TIME_UNIT_NS * fSetup.fTriggerPre);
  TriggerPostLineEdit->setText(pre+text.setNum (fSetup.fTriggerPost, numberbase));
  TriggerPostDoubleSpinBox->setValue(NYXOR_TIME_UNIT_NS * fSetup.fTriggerPost);
  SecondTestPulseDelayLineEdit->setText(pre+text.setNum (fSetup.fDelayTestPulse, numberbase));
  SecondTestPulseDelayDoubleSpinBox->setValue(NYXOR_TIME_UNIT_NS * fSetup.fDelayTestPulse);
  TestAcquisitionTriggerDelayLineEdit->setText(pre+text.setNum (fSetup.fDelayTrigger, numberbase));
  TestAcquisitionTriggerDelayDoubleSpinBox->setValue(NYXOR_TIME_UNIT_NS * fSetup.fDelayTrigger);


  ADCTestCodeLineEdit->setText(pre+text.setNum (fSetup.fTestCodeADC, numberbase));
  TestCodeNX1LineEdit->setText(pre+text.setNum (fSetup.fTestCode1, numberbase));
  TestCodeNX2LineEdit->setText(pre+text.setNum (fSetup.fTestCode2, numberbase));


  fSetup.ResetChanged();


}

void GeneralNyxorWidget::EvaluateView ()
{
  int numberbase=fxOwner->GetNumberBase();
  int val=0;

  val=nxControlEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fNXControl)
    {
      fSetup.fAnything_Changed=true;
      fSetup.fNXControl_Changed=true;
      fSetup.fNXControl=val;
    }
  val=TriggerPreLineEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fTriggerPre)
  {
      fSetup.fAnything_Changed=true;
      fSetup.fTriggerPre_Changed=true;
      fSetup.fTriggerPre=val;
  }
  val=TriggerPostLineEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fTriggerPost)
  {
      fSetup.fAnything_Changed=true;
      fSetup.fTriggerPost_Changed=true;
      fSetup.fTriggerPost=val;

  }
  val=SecondTestPulseDelayLineEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fDelayTestPulse)
  {
      fSetup.fAnything_Changed=true;
      fSetup.fDelayTestPulse_Changed=true;
      fSetup.fDelayTestPulse=val;
  }
  val=TestAcquisitionTriggerDelayLineEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fDelayTrigger)
  {
    fSetup.fAnything_Changed=true;
    fSetup.fDelayTestPulse_Changed=true;
    fSetup.fDelayTrigger=val;
  }
  val=ADCTestCodeLineEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fTestCodeADC)
  {
    fSetup.fAnything_Changed=true;
    fSetup.fTestCodeADC_Changed=true;
    fSetup.fTestCodeADC=val;
  }
  val=TestCodeNX1LineEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fTestCode1)
  {
    fSetup.fAnything_Changed=true;
    fSetup.fTestCode1_Changed=true;
    fSetup.fTestCode1=val;
  }
  val=TestCodeNX2LineEdit->text ().toUInt (0, numberbase);
  if(val!=fSetup.fTestCode2)
  {
      fSetup.fAnything_Changed=true;
      fSetup.fTestCode2_Changed=true;
    fSetup.fTestCode2=val;
  }

}



void GeneralNyxorWidget::TriggerPreSpinBox_changed(double nanos)
{
  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  fSetup.fTriggerPre=nanos/NYXOR_TIME_UNIT_NS;
  TriggerPreLineEdit->setText(pre+text.setNum (fSetup.fTriggerPre, numberbase));

}

void GeneralNyxorWidget::TriggerPostSpinBox_changed(double nanos)
{
  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  fSetup.fTriggerPost=nanos/NYXOR_TIME_UNIT_NS;
  TriggerPostLineEdit->setText(pre+text.setNum (fSetup.fTriggerPost, numberbase));



}

void GeneralNyxorWidget::SecondTestPulseDelaySpinBox_changed(double nanos)
{
  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  fSetup.fDelayTestPulse=nanos/NYXOR_TIME_UNIT_NS;
  SecondTestPulseDelayLineEdit->setText(pre+text.setNum (fSetup.fDelayTestPulse, numberbase));


}

void GeneralNyxorWidget::TestAcquisitionTriggerDelaySpinBox_changed(double nanos)
{

  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  fSetup.fDelayTrigger=nanos/NYXOR_TIME_UNIT_NS;
  TestAcquisitionTriggerDelayLineEdit->setText(pre+text.setNum (fSetup.fDelayTrigger, numberbase));

}

void GeneralNyxorWidget::TriggerPreLineEdit_finished()
{
  int numberbase=fxOwner->GetNumberBase();
  fSetup.fTriggerPre=TriggerPreLineEdit->text ().toUInt (0, numberbase);
  double nanos=NYXOR_TIME_UNIT_NS * fSetup.fTriggerPre;
  TriggerPreDoubleSpinBox->setValue(nanos);
}


void GeneralNyxorWidget::TriggerPostLineEdit_finished()
{

  int numberbase=fxOwner->GetNumberBase();
  fSetup.fTriggerPost=TriggerPostLineEdit->text ().toUInt (0, numberbase);
  double nanos=NYXOR_TIME_UNIT_NS * fSetup.fTriggerPost;
  TriggerPostDoubleSpinBox->setValue(nanos);

}


void GeneralNyxorWidget::SecondTestPulseDelayLineEdit_finished()
{
  int numberbase=fxOwner->GetNumberBase();
  fSetup.fDelayTestPulse=SecondTestPulseDelayLineEdit->text ().toUInt (0, numberbase);
  double nanos=NYXOR_TIME_UNIT_NS * fSetup.fDelayTestPulse;
  SecondTestPulseDelayDoubleSpinBox->setValue(nanos);
}

void GeneralNyxorWidget::TestAcquisitionTriggerDelayLineEdit_finished()
{

  int numberbase=fxOwner->GetNumberBase();
  fSetup.fDelayTrigger=TestAcquisitionTriggerDelayLineEdit->text ().toUInt (0, numberbase);
    double nanos=NYXOR_TIME_UNIT_NS * fSetup.fDelayTrigger;
    TestAcquisitionTriggerDelayDoubleSpinBox->setValue(nanos);


}

void GeneralNyxorWidget::nxControlEdit_finished()
{
  int numberbase=fxOwner->GetNumberBase();
  fSetup.fNXControl=nxControlEdit->text ().toUInt (0, numberbase);
  RefreshControlBits();
}

void GeneralNyxorWidget::RefreshControlBits()
{
  int word=fSetup.fNXControl & 0x3FFF;
    for (int b=0; b<14; ++b)
      {
        bool on= ((word >> b) & 0x1) == 0x1? true: false;
        fControlBitBoxes[b]->setChecked(on);
      }
}


void GeneralNyxorWidget::ControlBit_clicked(bool)
{
  // if user clicks any of the control bit boxes, we recalculate complete register value here:

  int word=0;
  for (int b=0; b<14; ++b)
      {
        if(fControlBitBoxes[b]->isChecked()) word |= (0x1 << b);
      }
  fSetup.fNXControl=word;

  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";

  nxControlEdit->setText(pre+text.setNum (fSetup.fNXControl, numberbase));
}


