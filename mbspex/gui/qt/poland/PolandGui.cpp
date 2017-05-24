#include "PolandGui.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <stdlib.h>
#include <errno.h>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>
#include <QTimer>


#include <sstream>





// *********************************************************

/*
 *  Constructs a PolandGui which is a child of 'parent', with the
 *  name 'name'.'
 */
PolandGui::PolandGui (QWidget* parent) :
    GosipGui (parent), fTriggerOn(true)
{
  fImplementationName="POLAND";
  fVersionString="Welcome to POLAND GUI!\n\t v0.95 of 24 May-2017 by JAM (j.adamczewski@gsi.de)";
  setWindowTitle(QString("%1 GUI").arg(fImplementationName));

  fPolandWidget=new PolandWidget(this);
  Settings_scrollArea->setWidget(fPolandWidget);


  ClearOutputBtn_clicked ();


  QObject::connect (fPolandWidget->OffsetButton, SIGNAL (clicked ()), this, SLOT (OffsetBtn_clicked ()));


  QObject::connect (fPolandWidget->TriggerButton, SIGNAL (clicked ()), this, SLOT (TriggerBtn_clicked ()));

  QObject::connect(fPolandWidget->DACModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DACMode_changed(int)));

// JAM2017: some more signals for the autoapply feature:

QObject::connect(fPolandWidget->ModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(QFW_changed()));
QObject::connect (fPolandWidget->MasterTriggerBox, SIGNAL(stateChanged(int)), this, SLOT (QFW_changed()));
QObject::connect (fPolandWidget->InternalTriggerBox, SIGNAL(stateChanged(int)), this, SLOT (QFW_changed()));
QObject::connect (fPolandWidget->FesaModeBox, SIGNAL(stateChanged(int)), this, SLOT (QFW_changed()));

QObject::connect (fPolandWidget->TSLoop1lineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (fPolandWidget->TSLoop2lineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (fPolandWidget->TSLoop3lineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (fPolandWidget->TS1TimelineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (fPolandWidget->TS2TimelineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (fPolandWidget->TS3TimelineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));

QObject::connect (fPolandWidget->DACStartValueLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DACOffsetLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DACDeltaOffsetLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DACCalibTimeLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));

QObject::connect (fPolandWidget->DAClineEdit_1, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_2, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_3, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_4, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_5, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_6, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_7, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_8, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_9, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_10, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_11, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_12, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_13, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_14, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_15, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_16, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_17, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_18, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_19, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_20, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_21, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_22, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_23, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_24, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_25, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_26, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_27, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_28, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_29, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_30, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_31, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (fPolandWidget->DAClineEdit_32, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));

QObject::connect (fPolandWidget->FanDial, SIGNAL(valueChanged(int)),this,SLOT (Fan_changed()));



  GetSFPChainSetup(); // ensure that any slave has a status structure before we begin clicking...
  show();
}

PolandGui::~PolandGui ()
{
}




void PolandGui::ApplyGUISettings()
{
  // depending on activated view, we either set qfw parameters or change DAC programming
if(fPolandWidget->QFW_DAC_tabWidget->currentIndex()==0)
{
  ApplyQFWSettings();

}
else if (fPolandWidget->QFW_DAC_tabWidget->currentIndex()==1)
{
  ApplyDACSettings();
}
else if (fPolandWidget->QFW_DAC_tabWidget->currentIndex()==2)
{
  ApplyFanSettings();
}
}


void PolandGui::ApplyFanSettings()
{
  EvaluateFans(); // from gui to memory
  SetFans ();
}


void PolandGui::ApplyQFWSettings()
{
  EvaluateView(); // from gui to memory
  SetRegisters ();
}

void PolandGui::ApplyDACSettings()
{
  EvaluateDAC();
  ApplyDAC();
}

void PolandGui::QFW_changed ()
{
  //std::cout << "PolandGui::QFW_changed()"<< std::endl;
  if(!checkBox_AA->isChecked()) return;
  EvaluateSlave ();
  GOSIP_BROADCAST_ACTION(ApplyQFWSettings());

}

void PolandGui::DAC_changed ()
{
  //std::cout << "PolandGui::DAC_changed()"<< std::endl;
  if(!checkBox_AA->isChecked()) return;
  EvaluateSlave ();
  GOSIP_BROADCAST_ACTION(ApplyDACSettings());
}

void PolandGui::Fan_changed ()
{
  //std::cout << "PolandGui::Fan_changed()"<< std::endl;
  if(!checkBox_AA->isChecked()) return;
  EvaluateSlave ();
  GOSIP_BROADCAST_ACTION(ApplyFanSettings());

  // for autoapply refresh fan readout immediately:
  if(AssertNoBroadcast(false))
  {
    GetSensors();
    RefreshSensors();
  }
}








void PolandGui::ResetSlave ()
{
 WriteGosip (fSFP, fSlave, POLAND_REG_RESET, 0);
 WriteGosip (fSFP, fSlave, POLAND_REG_RESET, 1);
 printm("Did reset POLAND for SFP %d Slave %d",fSFP,fSlave);
}




void PolandGui::OffsetBtn_clicked ()
{
char buffer[1024];
EvaluateSlave ();
snprintf (buffer, 1024, "Really scan offset for SFP chain %d, Slave %d ?", fSFP, fSlave);
if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
    != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}

GOSIP_BROADCAST_ACTION(ScanOffsets());
}


void PolandGui::ScanOffsets ()
{
  char buffer[1024];
  WriteGosip (fSFP, fSlave, POLAND_REG_DO_OFFSET, 1);
  printm("--- SFP %d Poland %d : Doing offset measurement... ",fSFP, fSlave);
  QApplication::setOverrideCursor( Qt::WaitCursor );

  WriteGosip (fSFP, fSlave, POLAND_REG_DO_OFFSET, 0);
  sleep(2);
  AppendTextWindow ("    ... done. Dumping offset values:");

  snprintf (buffer, 1024, "gosipcmd -d -r -x -- %d %d 0x%x 0x%x", fSFP, fSlave, POLAND_REG_OFFSET_BASE, 32);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
  QApplication::restoreOverrideCursor();

}



void PolandGui::TriggerBtn_clicked ()
{
  //std::cout << "PolandGui::TriggerBtn_clicked"<< std::endl;
  char buffer[1024];
  fTriggerOn ?   snprintf (buffer, 1024, "Really disable Frontend Trigger acceptance?") : snprintf (buffer, 1024, "Really enable Frontend Trigger acceptance?");

  if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }

  fTriggerOn ? fTriggerOn=false: fTriggerOn=true;


  int value = (fTriggerOn ? 1 : 0);
  QString state= (fTriggerOn ? "ON" : "OFF");
  WriteGosip (-1, -1, POLAND_REG_TRIG_ON, value); // send broadcast of register to all slaves.
  QString msg="--- Set all devices trigger acceptance to ";
  AppendTextWindow (msg+state);
  RefreshTrigger();


}

void PolandGui::RefreshTrigger()
{
   // todo: modify label
  QString labelprefix="<html><head/><body><p>Trigger";
  QString labelstate = fTriggerOn ? " <span style=\" font-weight:600; color:#00ff00;\">ON </span></p></body></html>" :
      " <span style=\" font-weight:600; color:#ff0000;\">OFF</span></p></body></html>" ;
  fPolandWidget->TriggerLabel->setText(labelprefix+labelstate);

}




void PolandGui::DumpSlave ()
{
  char buffer[1024];

  GetRegisters();
  printm("dump registers of sfp:%d device %d",fSFP,fSlave);
  theSetup_GET_FOR_SLAVE(PolandSetup);
  int numwords = 32 + theSetup->fSteps[0] * 32 + theSetup->fSteps[1] * 32 + theSetup->fSteps[2] * 32 + 32;
  // todo: note this will not work for broadcast mode or if show was not clicked before
  // we can live with it for the moment.

  snprintf (buffer, 1024, "gosipcmd -d -r -x -- %d %d 0 0x%x", fSFP, fSlave, numwords);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
}





void PolandGui::DACMode_changed(int ix)
{
  EvaluateSlave ();
  //std::cout << "PolandGui::DACMode_changed to index:"<< ix << std::endl;
  //std::cout << "PolandGui::DACMode_changed with sfp:"<< fSFP<<", slave:"<<fSlave << std::endl;
  if(AssertNoBroadcast (false))
  {
  theSetup_GET_FOR_SLAVE(PolandSetup);
  theSetup->fDACMode= ix+1;
  //GetRegisters();
  RefreshDAC();
  }

  if(checkBox_AA->isChecked())

    QTimer::singleShot(10, this, SLOT(DAC_changed())); // again we delay to avoid prelling signals?


}




void PolandGui::RefreshView ()
{
// display setup structure to gui:
QString text;
QString pre;
fNumberBase==16? pre="0x" : pre="";
theSetup_GET_FOR_SLAVE(PolandSetup);
RefreshMode();


fPolandWidget->TSLoop1lineEdit->setText (pre+text.setNum (theSetup->fSteps[0], fNumberBase));
fPolandWidget->TSLoop2lineEdit->setText (pre+text.setNum (theSetup->fSteps[1], fNumberBase));
fPolandWidget->TSLoop3lineEdit->setText (pre+text.setNum (theSetup->fSteps[2], fNumberBase));
fPolandWidget->TS1TimelineEdit->setText (text.setNum (theSetup->GetStepTime(0)));
fPolandWidget->TS2TimelineEdit->setText (text.setNum (theSetup->GetStepTime(1)));
fPolandWidget->TS3TimelineEdit->setText (text.setNum (theSetup->GetStepTime(2)));
fPolandWidget->MasterTriggerBox->setChecked (theSetup->IsTriggerMaster ());
fPolandWidget->FesaModeBox->setChecked (theSetup->IsFesaMode ());
fPolandWidget->InternalTriggerBox->setChecked (theSetup->IsInternalTrigger ());


fPolandWidget->EventCounterNumber->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter1->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter2->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter3->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter4->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter5->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter6->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter7->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
fPolandWidget->ErrorCounter8->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);


fPolandWidget->EventCounterNumber->display ((int) theSetup->fEventCounter);
fPolandWidget->ErrorCounter1->display ((int) theSetup->fErrorCounter[0]);
fPolandWidget->ErrorCounter2->display ((int) theSetup->fErrorCounter[1]);
fPolandWidget->ErrorCounter3->display ((int) theSetup->fErrorCounter[2]);
fPolandWidget->ErrorCounter4->display ((int) theSetup->fErrorCounter[3]);
fPolandWidget->ErrorCounter5->display ((int) theSetup->fErrorCounter[4]);
fPolandWidget->ErrorCounter6->display ((int) theSetup->fErrorCounter[5]);
fPolandWidget->ErrorCounter7->display ((int) theSetup->fErrorCounter[6]);
fPolandWidget->ErrorCounter8->display ((int) theSetup->fErrorCounter[7]);

RefreshDACMode();
RefreshDAC(); // probably this is already triggered by signal
RefreshTrigger(); // show real trigger register as read back from actual device

RefreshSensors();
RefreshChains();
RefreshStatus();
}

void PolandGui::EvaluateView ()
{
//EvaluateSlave (); // bug if used in broadcast macro
EvaluateMode  ();

theSetup_GET_FOR_SLAVE(PolandSetup);


// copy widget values to structure
theSetup->fSteps[0] = fPolandWidget->TSLoop1lineEdit->text ().toUInt (0, fNumberBase);
theSetup->fSteps[1] = fPolandWidget->TSLoop2lineEdit->text ().toUInt (0, fNumberBase);
theSetup->fSteps[2] = fPolandWidget->TSLoop3lineEdit->text ().toUInt (0, fNumberBase);

theSetup->SetStepTime(fPolandWidget->TS1TimelineEdit->text ().toDouble (),0);
theSetup->SetStepTime(fPolandWidget->TS2TimelineEdit->text ().toDouble (),1);
theSetup->SetStepTime(fPolandWidget->TS3TimelineEdit->text ().toDouble (),2);


theSetup->SetTriggerMaster (fPolandWidget->MasterTriggerBox->isChecked ());
theSetup->SetFesaMode (fPolandWidget->FesaModeBox->isChecked ());
theSetup->SetInternalTrigger (fPolandWidget->InternalTriggerBox->isChecked ());

}


void PolandGui::EvaluateFans()
{
  theSetup_GET_FOR_SLAVE(PolandSetup);
  theSetup->SetFanSettings(fPolandWidget->FanDial->value());

}


void PolandGui::EvaluateMode()
{
  theSetup_GET_FOR_SLAVE(PolandSetup);

  int index=fPolandWidget->ModeComboBox->currentIndex();
  switch(index)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      theSetup->fQFWMode= index;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      theSetup->fQFWMode= index -4  + 16;
    break;
    default:
      std::cout << "!!! Never come here - undefined mode index"<< index << std::endl;
    break;
  };
}
void PolandGui::RefreshMode()
{
  theSetup_GET_FOR_SLAVE(PolandSetup);
  int index=-1;
  switch(theSetup->fQFWMode)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      index=theSetup->fQFWMode;
      break;
    case 16:
    case 17:
    case 18:
    case 19:
      index= theSetup->fQFWMode- 16 + 4;
      break;
    default:
          std::cout << "!!! Never come here - undefined mode index"<< index << std::endl;
          break;
  };
  if(index>=0)
    fPolandWidget->ModeComboBox->setCurrentIndex(index);

}

void PolandGui::EvaluateSlave ()
{
  if(fBroadcasting) return;
  fSFP = SFPspinBox->value ();
  fSlave = SlavespinBox->value ();
}



void PolandGui::EvaluateDAC()
{
  theSetup_GET_FOR_SLAVE(PolandSetup);

  theSetup->fDACMode=fPolandWidget->DACModeComboBox->currentIndex()+1;


  if(theSetup->fDACMode==4)
  {
  theSetup->fDACAllValue=fPolandWidget->DACStartValueLineEdit->text ().toUInt (0, fNumberBase);

  }
  else
  {
    theSetup->fDACStartValue=fPolandWidget->DACStartValueLineEdit->text ().toUInt (0, fNumberBase);
  }


  theSetup->fDACOffset=fPolandWidget->DACOffsetLineEdit->text ().toUInt (0, fNumberBase);
  theSetup->fDACDelta=fPolandWidget->DACDeltaOffsetLineEdit->text ().toUInt (0, fNumberBase);
  theSetup->SetCalibrationTime(fPolandWidget->DACCalibTimeLineEdit->text ().toDouble ());



if(theSetup->fDACMode==1)
{
  // only manual mode will refresh DAQ structure here
theSetup->fDACValue[0]=fPolandWidget->DAClineEdit_1->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[1]=fPolandWidget->DAClineEdit_2->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[2]=fPolandWidget->DAClineEdit_3->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[3]=fPolandWidget->DAClineEdit_4->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[4]=fPolandWidget->DAClineEdit_5->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[5]=fPolandWidget->DAClineEdit_6->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[6]=fPolandWidget->DAClineEdit_7->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[7]=fPolandWidget->DAClineEdit_8->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[8]=fPolandWidget->DAClineEdit_9->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[9]=fPolandWidget->DAClineEdit_10->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[10]=fPolandWidget->DAClineEdit_11->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[11]=fPolandWidget->DAClineEdit_12->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[12]=fPolandWidget->DAClineEdit_13->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[13]=fPolandWidget->DAClineEdit_14->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[14]=fPolandWidget->DAClineEdit_15->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[15]=fPolandWidget->DAClineEdit_16->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[16]=fPolandWidget->DAClineEdit_17->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[17]=fPolandWidget->DAClineEdit_18->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[18]=fPolandWidget->DAClineEdit_19->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[19]=fPolandWidget->DAClineEdit_20->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[20]=fPolandWidget->DAClineEdit_21->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[21]=fPolandWidget->DAClineEdit_22->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[22]=fPolandWidget->DAClineEdit_23->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[23]=fPolandWidget->DAClineEdit_24->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[24]=fPolandWidget->DAClineEdit_25->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[25]=fPolandWidget->DAClineEdit_26->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[26]=fPolandWidget->DAClineEdit_27->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[27]=fPolandWidget->DAClineEdit_28->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[28]=fPolandWidget->DAClineEdit_29->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[29]=fPolandWidget->DAClineEdit_30->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[30]=fPolandWidget->DAClineEdit_31->text ().toUInt (0, fNumberBase);
theSetup->fDACValue[31]=fPolandWidget->DAClineEdit_32->text ().toUInt (0, fNumberBase);
}

}


void  PolandGui::ApplyDAC()
{
  theSetup_GET_FOR_SLAVE(PolandSetup);
  QApplication::setOverrideCursor( Qt::WaitCursor );
  WriteGosip (fSFP, fSlave, POLAND_REG_DAC_MODE , theSetup->fDACMode);

  switch((int) theSetup->fDACMode)
  {
    case 1:
      // manual settings:
      for (int d = 0; d < POLAND_DAC_NUM; ++d)
      {
        WriteGosip (fSFP, fSlave, POLAND_REG_DAC_BASE_WRITE + 4 * d, theSetup->fDACValue[d]);
      }
      break;
    case 2:
      // test structure:
      // no more actions needed
      break;
    case 3:
      // issue calibration:
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_STARTVAL , theSetup->fDACStartValue);
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_OFFSET ,  theSetup->fDACOffset);
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_DELTA ,  theSetup->fDACDelta);
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_TIME ,  theSetup->fDACCalibTime);

      break;
    case 4:
      // all same constant value mode:
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_ALLVAL , theSetup->fDACAllValue);
      break;

    default:
      std::cout << "!!! ApplyDAC Never come here - undefined DAC mode"<<  theSetup->fDACMode << std::endl;
      break;

  };


  WriteGosip (fSFP, fSlave, POLAND_REG_DAC_PROGRAM , 1);
  WriteGosip (fSFP, fSlave, POLAND_REG_DAC_PROGRAM , 0);
  sleep(2);

  QApplication::restoreOverrideCursor();
}

void  PolandGui::RefreshDAC()
{
  //std::cout << "PolandGui::RefreshDAC"<< std::endl;
  QString text;
QString pre;
fNumberBase==16? pre="0x" : pre="";

theSetup_GET_FOR_SLAVE(PolandSetup);



fPolandWidget->DACOffsetLineEdit->setText (pre+text.setNum (theSetup->fDACOffset, fNumberBase));
fPolandWidget->DACDeltaOffsetLineEdit->setText (pre+text.setNum (theSetup->fDACDelta, fNumberBase));
fPolandWidget->DACCalibTimeLineEdit->setText (text.setNum (theSetup->GetCalibrationTime()));

fPolandWidget->DAClineEdit_1->setText (pre+text.setNum (theSetup->fDACValue[0], fNumberBase));
fPolandWidget->DAClineEdit_2->setText (pre+text.setNum (theSetup->fDACValue[1], fNumberBase));
fPolandWidget->DAClineEdit_3->setText (pre+text.setNum (theSetup->fDACValue[2], fNumberBase));
fPolandWidget->DAClineEdit_4->setText (pre+text.setNum (theSetup->fDACValue[3], fNumberBase));
fPolandWidget->DAClineEdit_5->setText (pre+text.setNum (theSetup->fDACValue[4], fNumberBase));
fPolandWidget->DAClineEdit_6->setText (pre+text.setNum (theSetup->fDACValue[5], fNumberBase));
fPolandWidget-> DAClineEdit_7->setText (pre+text.setNum (theSetup->fDACValue[6], fNumberBase));
fPolandWidget-> DAClineEdit_8->setText (pre+text.setNum (theSetup->fDACValue[7], fNumberBase));
fPolandWidget->DAClineEdit_9->setText (pre+text.setNum (theSetup->fDACValue[8], fNumberBase));
fPolandWidget->DAClineEdit_10->setText (pre+text.setNum (theSetup->fDACValue[9], fNumberBase));
fPolandWidget->DAClineEdit_11->setText (pre+text.setNum (theSetup->fDACValue[10], fNumberBase));
fPolandWidget->DAClineEdit_12->setText (pre+text.setNum (theSetup->fDACValue[11], fNumberBase));
fPolandWidget->DAClineEdit_13->setText (pre+text.setNum (theSetup->fDACValue[12], fNumberBase));
fPolandWidget->DAClineEdit_14->setText (pre+text.setNum (theSetup->fDACValue[13], fNumberBase));
fPolandWidget->DAClineEdit_15->setText (pre+text.setNum (theSetup->fDACValue[14], fNumberBase));
fPolandWidget->DAClineEdit_16->setText (pre+text.setNum (theSetup->fDACValue[15], fNumberBase));
fPolandWidget-> DAClineEdit_17->setText (pre+text.setNum (theSetup->fDACValue[16], fNumberBase));
fPolandWidget->DAClineEdit_18->setText (pre+text.setNum (theSetup->fDACValue[17], fNumberBase));
fPolandWidget->DAClineEdit_19->setText (pre+text.setNum (theSetup->fDACValue[18], fNumberBase));
fPolandWidget->DAClineEdit_20->setText (pre+text.setNum (theSetup->fDACValue[19], fNumberBase));
fPolandWidget->DAClineEdit_21->setText (pre+text.setNum (theSetup->fDACValue[20], fNumberBase));
fPolandWidget->DAClineEdit_22->setText (pre+text.setNum (theSetup->fDACValue[21], fNumberBase));
fPolandWidget->DAClineEdit_23->setText (pre+text.setNum (theSetup->fDACValue[22], fNumberBase));
fPolandWidget->DAClineEdit_24->setText (pre+text.setNum (theSetup->fDACValue[23], fNumberBase));
fPolandWidget->DAClineEdit_25->setText (pre+text.setNum (theSetup->fDACValue[24], fNumberBase));
fPolandWidget->DAClineEdit_26->setText (pre+text.setNum (theSetup->fDACValue[25], fNumberBase));
fPolandWidget->DAClineEdit_27->setText (pre+text.setNum (theSetup->fDACValue[26], fNumberBase));
fPolandWidget->DAClineEdit_28->setText (pre+text.setNum (theSetup->fDACValue[27], fNumberBase));
fPolandWidget->DAClineEdit_29->setText (pre+text.setNum (theSetup->fDACValue[28], fNumberBase));
fPolandWidget->DAClineEdit_30->setText (pre+text.setNum (theSetup->fDACValue[29], fNumberBase));
fPolandWidget->DAClineEdit_31->setText (pre+text.setNum (theSetup->fDACValue[30], fNumberBase));
fPolandWidget->DAClineEdit_32->setText (pre+text.setNum (theSetup->fDACValue[31], fNumberBase));

  // depending on DAC mode different fields are writable:

  //std::cout << "!!! RefreshDAC With DAC mode="<<  (int)theSetup->fDACMode << std::endl;
  switch((int) theSetup->fDACMode)
  {
    case 1:
      // manual settings:
      fPolandWidget->DACscrollArea->setEnabled(true);
      fPolandWidget->DACCaliFrame->setEnabled(false);

      break;
    case 2:
      // test structure
      fPolandWidget->DACscrollArea->setEnabled(false);
      fPolandWidget->DACCaliFrame->setEnabled(false);
          break;
    case 3:
          // calibrate mode
      fPolandWidget->DACscrollArea->setEnabled(false);
      fPolandWidget->DACCaliFrame->setEnabled(true);
      fPolandWidget->DACStartValueLineEdit->setEnabled(true);
      fPolandWidget->DACStartValueLineEdit->setText (pre+text.setNum (theSetup->fDACStartValue, fNumberBase));
      fPolandWidget->DACOffsetLineEdit->setEnabled(true);
      fPolandWidget->DACDeltaOffsetLineEdit->setEnabled(true);
      fPolandWidget->DACCalibTimeLineEdit->setEnabled(true);

          break;
    case 4:
          // all constant
      fPolandWidget->DACscrollArea->setEnabled(false);
      fPolandWidget->DACCaliFrame->setEnabled(true);
      fPolandWidget->DACStartValueLineEdit->setEnabled(true);
      fPolandWidget->DACStartValueLineEdit->setText (pre+text.setNum (theSetup->fDACAllValue, fNumberBase));
      fPolandWidget->DACOffsetLineEdit->setEnabled(false);
      fPolandWidget->DACDeltaOffsetLineEdit->setEnabled(false);
      fPolandWidget->DACCalibTimeLineEdit->setEnabled(false);

          break;
    default:
      std::cout << "!!! RefreshDAC Never come here - undefined DAC mode"<<  theSetup->fDACMode << std::endl;
      break;

  };



}


void PolandGui::RefreshDACMode()
{
  //std::cout << "PolandGui::RefreshDACMode for mode "<< (int) fSetup.fDACMode << std::endl;
  theSetup_GET_FOR_SLAVE(PolandSetup);
  fPolandWidget->DACModeComboBox->setCurrentIndex((int) theSetup->fDACMode -1);
}


void PolandGui::SetRegisters ()
{
// write register values from strucure with gosipcmd
  theSetup_GET_FOR_SLAVE(PolandSetup);
//if (AssertNoBroadcast (false)) // NOTE: after change to broadcast action, this is always true JAM2017
if(!fBroadcasting) // use macro flag instead!
{
  // update trigger modes only in single device
  WriteGosip (fSFP, fSlave, POLAND_REG_INTERNAL_TRIGGER, theSetup->fInternalTrigger);
  WriteGosip (fSFP, fSlave, POLAND_REG_MASTERMODE, theSetup->fTriggerMode);
}

WriteGosip (fSFP, fSlave, POLAND_REG_QFW_MODE, theSetup->fQFWMode);

// following is required to really activate qfw mode (thanks Sven Loechner for fixing):
WriteGosip (fSFP, fSlave, POLAND_REG_QFW_PRG, 1);
WriteGosip (fSFP, fSlave, POLAND_REG_QFW_PRG, 0);



// WriteGosip(fSFP, fSlave, POLAND_REG_TRIGCOUNT, theSetup->fEventCounter);

for (int i = 0; i < POLAND_TS_NUM; ++i)
{
  WriteGosip (fSFP, fSlave, POLAND_REG_STEPS_BASE + 4 * i, theSetup->fSteps[i]);
  WriteGosip (fSFP, fSlave, POLAND_REG_TIME_BASE + 4 * i, theSetup->fTimes[i]);
}
//    for(int e=0; e<POLAND_ERRCOUNT_NUM;++e)
//     {
//       WriteGosip(fSFP, fSlave, POLAND_REG_ERRCOUNT_BASE + 4*e, theSetup->fErrorCounter[e]);
//     }

// TODO: error handling with exceptions?

}

void PolandGui::GetRegisters ()
{
// read register values into structure with gosipcmd

if (!AssertNoBroadcast ())
  return;

theSetup_GET_FOR_SLAVE(PolandSetup);

theSetup->fInternalTrigger = ReadGosip (fSFP, fSlave, POLAND_REG_INTERNAL_TRIGGER);
theSetup->fTriggerMode = ReadGosip (fSFP, fSlave, POLAND_REG_MASTERMODE);
theSetup->fEventCounter = ReadGosip (fSFP, fSlave, POLAND_REG_TRIGCOUNT);
theSetup->fQFWMode = ReadGosip (fSFP, fSlave, POLAND_REG_QFW_MODE);

for (int i = 0; i < POLAND_TS_NUM; ++i)
{
  theSetup->fSteps[i] = ReadGosip (fSFP, fSlave, POLAND_REG_STEPS_BASE + 4 * i);
  theSetup->fTimes[i] = ReadGosip (fSFP, fSlave, POLAND_REG_TIME_BASE + 4 * i);
}

// for errorcounters we have to scan token payload:
int errcountstart = 4*(32 + theSetup->fSteps[0] * 32 + theSetup->fSteps[1] * 32 + theSetup->fSteps[2] * 32);

for (int e = 0; e < POLAND_ERRCOUNT_NUM; ++e)
{
  theSetup->fErrorCounter[e] = ReadGosip (fSFP, fSlave, errcountstart + 4 * e);
}

theSetup->fDACMode=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_MODE);
theSetup->fDACCalibTime=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_TIME);
theSetup->fDACOffset=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_OFFSET);
theSetup->fDACStartValue=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_STARTVAL);

for (int d = 0; d < POLAND_DAC_NUM; ++d)
{
  theSetup->fDACValue[d] = ReadGosip (fSFP, fSlave, POLAND_REG_DAC_BASE_READ + 4 * d);
}

theSetup->fTriggerOn=ReadGosip (fSFP, fSlave, POLAND_REG_TRIG_ON);

fTriggerOn=theSetup->fTriggerOn;
// for the moment, we only refresh the general trigger flag from current frontend


GetSensors();


//printf("GetRegisters for sfp:%d slave:%d DUMP \n",fSFP, fSlave);
//theSetup->Dump();

// TODO: error handling with exceptions?
}



  void PolandGui::RefreshSensors()
  {

    QString text;
    QString pre;
    fNumberBase==16? pre="0x" : pre="";
    theSetup_GET_FOR_SLAVE(PolandSetup);

    QString idtext=QString("POLAND Firmware version: 0x%1 [YYMMDDVV]").arg(theSetup->GetVersionId(),0,16);
    fPolandWidget->FirmwareLabel->setText(idtext);

    fPolandWidget->TBaseLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->TLogicLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->TStretchLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->TPiggy1LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->TPiggy2LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->TPiggy3LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->TPiggy4LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);

    fPolandWidget->Fan1_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->Fan2_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->Fan3_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    fPolandWidget->Fan4_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);

    fPolandWidget->TBaseLCD->display (theSetup->GetTemp_Base());
    fPolandWidget->TLogicLCD->display (theSetup->GetTemp_LogicUnit());
    fPolandWidget->TStretchLCD->display (theSetup->GetTemp_Stretcher());
    fPolandWidget->TPiggy1LCD->display (theSetup->GetTemp_Piggy_1());
    fPolandWidget->TPiggy2LCD->display (theSetup->GetTemp_Piggy_2());
    fPolandWidget->TPiggy3LCD->display (theSetup->GetTemp_Piggy_3());
    fPolandWidget->TPiggy4LCD->display (theSetup->GetTemp_Piggy_4());

    QString basetext=QString("0x%1").arg(qulonglong(theSetup->GetSensorId_Base()),0,16);
    fPolandWidget->IdBaseLabel->setText(basetext);

    QString logictext=QString("0x%1").arg(qulonglong(theSetup->GetSensorId_LogicUnit()),0,16);
    fPolandWidget->IdLogicLabel->setText(logictext);


    QString stretchtext=QString("0x%1").arg(qulonglong(theSetup->GetSensorId_Stretcher()),0,16);
    fPolandWidget->IdStretchLabel->setText(stretchtext);

    QString piggy1text=QString("0x%1").arg(qulonglong(theSetup->GetSensorId_Piggy_1()),0,16);
    fPolandWidget->IdPiggy1Label->setText(piggy1text);
    QString piggy2text=QString("0x%1").arg(qulonglong(theSetup->GetSensorId_Piggy_2()),0,16);
    fPolandWidget->IdPiggy2Label->setText(piggy2text);

    QString piggy3text=QString("0x%1").arg(qulonglong(theSetup->GetSensorId_Piggy_3()),0,16);
    fPolandWidget->IdPiggy3Label->setText(piggy3text);

    QString piggy4text=QString("0x%1").arg(qulonglong(theSetup->GetSensorId_Piggy_4()),0,16);
    fPolandWidget->IdPiggy4Label->setText(piggy4text);





    fPolandWidget->Fan1_LCD->display (theSetup->GetFanRPM(0));
    fPolandWidget->Fan2_LCD->display (theSetup->GetFanRPM(1));
    fPolandWidget->Fan3_LCD->display (theSetup->GetFanRPM(2));
    fPolandWidget->Fan4_LCD->display (theSetup->GetFanRPM(3));

    fPolandWidget->FanDial->setValue(theSetup->GetFanSettings());


  }


void PolandGui::GetSensors ()
  {

  if (!AssertNoBroadcast ())
    return;

  theSetup_GET_FOR_SLAVE(PolandSetup);

  unsigned int version=ReadGosip (fSFP, fSlave, POLAND_REG_FIRMWARE_VERSION);
  theSetup->SetVersionId(version);

//  unsigned long long id_lsb=ReadGosip (fSFP, fSlave, POLAND_REG_ID_LSB);
//  unsigned long long id_msb=ReadGosip (fSFP, fSlave, POLAND_REG_ID_MSB);
//  id_msb= id_msb & 0xFFFFFF; // mask out upper crc word
//  unsigned long long id= (id_msb << 32) + id_lsb;
//  theSetup->SetSensorId(id);



  unsigned int address=POLAND_REG_TEMP_BASE;
  for(int t=0; t<POLAND_TEMP_NUM;t+=2)
  {
    unsigned int data=ReadGosip (fSFP, fSlave, address);
    theSetup->SetTempRaw(t, (data & 0xffff));
    theSetup->SetTempRaw(t+1, (data>>16) & 0xffff);
    address+=4;
  }
  address=POLAND_REG_ID_BASE;
  for(int t=0; t<POLAND_TEMP_NUM;++t)
  {
    unsigned long long id_msb=ReadGosip (fSFP, fSlave, address);
    unsigned long long id_lsb=ReadGosip (fSFP, fSlave, address+4);
    id_msb= id_msb & 0xFFFFFF; // mask out upper crc word
    unsigned long long id= (id_msb << 32) + id_lsb;
    theSetup->SetSensorId(t,id);
    address+=8;
  }



  address=POLAND_REG_FAN_BASE;
  for(int t=0; t<POLAND_FAN_NUM;t+=2)
  {
    unsigned int data=ReadGosip (fSFP, fSlave, address);
    theSetup->SetFanRaw(t, (data & 0xffff));
    theSetup->SetFanRaw(t+1, (data>>16) & 0xffff);
    address+=4;
  }




 // read back fan setter value:
  theSetup->fFanSettings=ReadGosip (fSFP, fSlave, POLAND_REG_FAN_SET);


  }



void PolandGui::SetFans ()
  {
    //std::cout << "PolandGui::SetFans()"<< std::endl;
    theSetup_GET_FOR_SLAVE(PolandSetup);
    WriteGosip (fSFP, fSlave, POLAND_REG_FAN_SET, theSetup->fFanSettings);

  }






void PolandGui::SaveRegisters ()
{
  // this may be called in explicit broadcast mode, so it is independent of the view on gui
  GetRegisters(); // refresh actual setup from hardware
  fSaveConfig = true;    // switch to file output mode
  WriteConfigFile (QString ("# QFW Registers: \n"));
  SetRegisters();    // register settings are written to file
  WriteConfigFile (QString ("# DAC Settings: \n"));
  ApplyDAC();
  fSaveConfig = false;
}












