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

}

void GeneralNyxorWidget::SetRegisters()
{

}

void GeneralNyxorWidget::RefreshView ()
{
  QString pre;
  QString text;
  int numberbase=fxOwner->GetNumberBase();
  numberbase == 16 ? pre = "0x" : pre = "";
  nxControlEdit->setText(pre+text.setNum (fSetup.fNXControl, numberbase));
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





}

void GeneralNyxorWidget::EvaluateView ()
{
  int numberbase=fxOwner->GetNumberBase();
  fSetup.fNXControl=nxControlEdit->text ().toUInt (0, numberbase);
  fSetup.fTriggerPre=TriggerPreLineEdit->text ().toUInt (0, numberbase);
  fSetup.fTriggerPost=TriggerPostLineEdit->text ().toUInt (0, numberbase);
  fSetup.fDelayTestPulse=SecondTestPulseDelayLineEdit->text ().toUInt (0, numberbase);
  fSetup.fDelayTrigger=TestAcquisitionTriggerDelayLineEdit->text ().toUInt (0, numberbase);
  fSetup.fTestCodeADC=ADCTestCodeLineEdit->text ().toUInt (0, numberbase);
  fSetup.fTestCode1=TestCodeNX1LineEdit->text ().toUInt (0, numberbase);
  fSetup.fTestCode2=TestCodeNX2LineEdit->text ().toUInt (0, numberbase);
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


