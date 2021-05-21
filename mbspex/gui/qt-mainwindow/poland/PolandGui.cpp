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
#include <QMdiSubWindow>


#include <kplotobject.h>
#include <kplotwidget.h>
#include <kplotaxis.h>


#include <sstream>


// enable this define to show original tabbed febex gui as single mdi subwindow
//#define POLAND_USE_SINGLE_SUBWINDOW 1



// *********************************************************

/*
 *  Constructs a PolandGui which is a child of 'parent', with the
 *  name 'name'.'
 */
PolandGui::PolandGui (QWidget* parent) :
    GosipGui (parent), fTriggerOn(true), fDoResetQFW(false)
{
  fImplementationName="POLAND";
  fVersionString="Welcome to POLAND GUI!\n\t v0.992 of 21-May-2021 by JAM (j.adamczewski@gsi.de)";
  setWindowTitle(QString("%1 GUI").arg(fImplementationName));


  fSettings=new QSettings("GSI", fImplementationName);

#ifdef POLAND_USE_SINGLE_SUBWINDOW
fPolandWidget=new PolandWidget(this);
mdiArea->addSubWindow(fPolandWidget); // complete febex widget in one window

#else

  fPolandWidget=new PolandWidget(0);
  fPolandViewpanelWidget = new PolandViewpanelWidget(this);
  fPolandCSAWidget = new PolandCSAWidget(this);

  QWidget* qfwtab=fPolandWidget->QFW_DAC_tabWidget->widget(0);
  QWidget* dactab=fPolandWidget->QFW_DAC_tabWidget->widget(1);
  QWidget* sensortab=fPolandWidget->QFW_DAC_tabWidget->widget(2);
  QWidget* counterbox=fPolandWidget->CounterBox;
  QWidget* triggerframe=fPolandWidget->triggerframe;
  Qt::WindowFlags wflags= Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint;
  if(qfwtab)
    {
      fPolandWidget->QFW_DAC_tabWidget->removeTab(0);
      qfwtab->setWindowTitle("QFW");
      qfwtab->show();
      QMdiSubWindow* subqfw=mdiArea->addSubWindow(qfwtab,wflags);
      subqfw->setAttribute(Qt::WA_DeleteOnClose, false);
    }

  if(dactab)
    {
    fPolandWidget->QFW_DAC_tabWidget->removeTab(0);
    dactab->setWindowTitle("DAC");
    dactab->show();
    QMdiSubWindow* subdac=mdiArea->addSubWindow(dactab, wflags);
    subdac->setAttribute(Qt::WA_DeleteOnClose, false);
    }

  if(sensortab)
  {
    fPolandWidget->QFW_DAC_tabWidget->removeTab(0);
    sensortab->setWindowTitle("Sensors");
    sensortab->show();
    QMdiSubWindow* subsens=mdiArea->addSubWindow(sensortab, wflags);
    subsens->setAttribute(Qt::WA_DeleteOnClose, false);
  }

  if(counterbox)
   {
     counterbox->setWindowTitle("Counters");
     counterbox->show();
     QMdiSubWindow* subcount=mdiArea->addSubWindow(counterbox, wflags);
     subcount->setAttribute(Qt::WA_DeleteOnClose, false);
   }
  if(triggerframe)
     {
       triggerframe->setWindowTitle("Trigger");
       triggerframe->show();
       QMdiSubWindow* subtrig=mdiArea->addSubWindow(triggerframe, wflags);
       subtrig->setAttribute(Qt::WA_DeleteOnClose, false);
     }

  if(fPolandViewpanelWidget)
       {
	  	 fPolandViewpanelWidget->setWindowTitle("Sample View");
         fPolandViewpanelWidget->show();
         QMdiSubWindow* subtrig=mdiArea->addSubWindow(fPolandViewpanelWidget, wflags);
         subtrig->setAttribute(Qt::WA_DeleteOnClose, false);
       }

  if(fPolandCSAWidget)
        {
          fPolandCSAWidget->setWindowTitle("CSA control");
          fPolandCSAWidget->show();
          QMdiSubWindow* subtrig=mdiArea->addSubWindow(fPolandCSAWidget, wflags);
          subtrig->setAttribute(Qt::WA_DeleteOnClose, false);
        }

#endif




  ClearOutputBtn_clicked ();


  QObject::connect (fPolandWidget->OffsetButton, SIGNAL (clicked ()), this, SLOT (OffsetBtn_clicked ()));


  QObject::connect (fPolandWidget->TriggerButton, SIGNAL (clicked ()), this, SLOT (TriggerBtn_clicked ()));

  QObject::connect (fPolandWidget->QFWResetButton, SIGNAL (clicked ()), this, SLOT (QFWResetBtn_clicked ()));





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


QObject::connect (fPolandViewpanelWidget->SampleButton, SIGNAL (clicked ()), this, SLOT (ShowSample ()));

QObject::connect (fPolandCSAWidget->CSA_inswitch_tocsa_radioButton, SIGNAL (toggled (bool)), this, SLOT (CSA_changed()));
// TODO CSA
QObject::connect (fPolandCSAWidget->CSA_inswitch_bypass_radioButton, SIGNAL (toggled (bool)), this, SLOT (CSA_changed()));

QObject::connect (fPolandCSAWidget->CSA_autorange_auto_radioButton, SIGNAL (toggled (bool)), this, SLOT (CSA_changed()));

QObject::connect (fPolandCSAWidget->CSA_autorange_manual_radioButton, SIGNAL (toggled (bool)), this, SLOT (CSA_changed()));

QObject::connect (fPolandCSAWidget->CSA_outswitch_fromcsa_radioButton, SIGNAL (toggled (bool)), this, SLOT (CSA_changed()));

QObject::connect (fPolandCSAWidget->CSA_outswitch_bypass_radioButton, SIGNAL (toggled (bool)), this, SLOT (CSA_changed()));

QObject::connect (fPolandCSAWidget->CSA_feedback_spinBox, SIGNAL (valueChanged(int)), this, SLOT (CSA_changed()));

ReadSettings();

  GetSFPChainSetup(); // ensure that any slave has a status structure before we begin clicking...
  show();
}

PolandGui::~PolandGui ()
{
}




void PolandGui::ApplyGUISettings()
{
  //std::cout << "PolandGui::ApplyGUISettings()"<< std::endl;
  // depending on activated view, we either set qfw parameters or change DAC programming

// TODO JAM2019 - for subwindow mode, always apply everything
  //if(fPolandWidget->QFW_DAC_tabWidget->currentIndex()==0)
{
  ApplyQFWSettings();

}
//else if (fPolandWidget->QFW_DAC_tabWidget->currentIndex()==1)
{
  ApplyDACSettings();
}
//else if (fPolandWidget->QFW_DAC_tabWidget->currentIndex()==2)
{
  ApplyFanSettings();
}

{
  ApplyCSASettings();
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
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyQFWSettings());
  GOSIP_UNLOCK_SLOT
}

void PolandGui::DAC_changed ()
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyDACSettings());
  GOSIP_UNLOCK_SLOT
}

void PolandGui::Fan_changed ()
{
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyFanSettings());

  // for autoapply refresh fan readout immediately:
  if(AssertNoBroadcast(false))
  {
    GetSensors();
    RefreshSensors();
  }
  GOSIP_UNLOCK_SLOT
}


void PolandGui::CSA_changed ()
{
  //std::cout << "PolandGui::CSA_changed()"<< std::endl;
  GOSIP_LOCK_SLOT
  GOSIP_AUTOAPPLY(ApplyCSASettings());
  GOSIP_UNLOCK_SLOT
}


void PolandGui::ApplyCSASettings()
{
  EvaluateCSA();
  ApplyCSA();
}


void PolandGui::EvaluateCSA()
{
  theSetup_GET_FOR_SLAVE(PolandSetup);
  uint8_t feedback = (fPolandCSAWidget->CSA_feedback_spinBox->value() & 0x7);
  bool autorangemanual= fPolandCSAWidget->CSA_autorange_manual_radioButton->isChecked();
  bool inswitchbypass= fPolandCSAWidget->CSA_inswitch_bypass_radioButton->isChecked();
  bool outswitchbypass= fPolandCSAWidget->CSA_outswitch_bypass_radioButton->isChecked();
  //std::cout << "PolandGui::EvaluateCSA sees autorangemanual:"<<autorangemanual<<", inbypass:"<<inswitchbypass << ", outbypass:"<< outswitchbypass<<", feedback:"<< (int)feedback << std::endl;
  theSetup->SetCSASettings(autorangemanual, inswitchbypass, outswitchbypass, feedback);
  //std::cout << "PolandGui::EvaluateCSA() sets register value 0x"<< std::hex << theSetup->GetCSAControl()<< std::dec<< std::endl;
}


void PolandGui::ApplyCSA()
{
  theSetup_GET_FOR_SLAVE(PolandSetup);
  WriteGosip (fSFP, fSlave, POLAND_REG_CSA_CTRL, theSetup->GetCSAControl());
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

  snprintf (buffer, 1024, "gosipcmd -d -r -x -- 0x%x 0x%x 0x%x 0x%x", fSFP, fSlave, POLAND_REG_OFFSET_BASE, 32);
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


void PolandGui::QFWResetBtn_clicked ()
{
char buffer[1024];
EvaluateSlave ();
snprintf (buffer, 1024, "Really Reset QFWs for SFP chain %d, Slave %d ?", fSFP, fSlave);
if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
    != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}
fDoResetQFW=true;
QFW_changed ();
fDoResetQFW=false;

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

  snprintf (buffer, 1024, "gosipcmd -d -r -x -- 0x%x 0x%x  0 0x%x", fSFP, fSlave, numwords);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
}





void PolandGui::DACMode_changed(int ix)
{
  EnableDACModeWidgets(ix+1); // just update text input widgest enable/disabled
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
fPolandViewpanelWidget->RefreshEventCounter(); // hex/dec toggle here

RefreshCSA();

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

//void PolandGui::EvaluateSlave ()
//{
//  if(fBroadcasting) return;
//  fSFP = SFPspinBox->value ();
//  fSlave = SlavespinBox->value ();
//}



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
int dacmode=(int) theSetup->fDACMode;
EnableDACModeWidgets(dacmode);
  //std::cout << "!!! RefreshDAC With DAC mode="<<  (int)theSetup->fDACMode << std::endl;

// additionally, we update some fields depending on the mode:
switch(dacmode)
  {
     case 3:
          // calibrate mode
      fPolandWidget->DACStartValueLineEdit->setText (pre+text.setNum (theSetup->fDACStartValue, fNumberBase));
          break;
    case 4:
          // all constant
      fPolandWidget->DACStartValueLineEdit->setText (pre+text.setNum (theSetup->fDACAllValue, fNumberBase));
          break;
    default:
      break;

  };



}


void PolandGui::RefreshDACMode()
{
  //std::cout << "PolandGui::RefreshDACMode for mode "<< (int) fSetup.fDACMode << std::endl;
  theSetup_GET_FOR_SLAVE(PolandSetup);
  fPolandWidget->DACModeComboBox->setCurrentIndex((int) theSetup->fDACMode -1);
}


void PolandGui::EnableDACModeWidgets(int mode)
{
  switch(mode)
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
      //fPolandWidget->DACStartValueLineEdit->setText (pre+text.setNum (theSetup->fDACStartValue, fNumberBase));
      fPolandWidget->DACOffsetLineEdit->setEnabled(true);
      fPolandWidget->DACDeltaOffsetLineEdit->setEnabled(true);
      fPolandWidget->DACCalibTimeLineEdit->setEnabled(true);

          break;
    case 4:
          // all constant
      fPolandWidget->DACscrollArea->setEnabled(false);
      fPolandWidget->DACCaliFrame->setEnabled(true);
      fPolandWidget->DACStartValueLineEdit->setEnabled(true);
      //fPolandWidget->DACStartValueLineEdit->setText (pre+text.setNum (theSetup->fDACAllValue, fNumberBase));
      fPolandWidget->DACOffsetLineEdit->setEnabled(false);
      fPolandWidget->DACDeltaOffsetLineEdit->setEnabled(false);
      fPolandWidget->DACCalibTimeLineEdit->setEnabled(false);

          break;
    default:
      std::cout << "!!!EnableDACModeWidgets Never come here - undefined DAC mode "<<  mode << std::endl;
      break;

  };

}



void PolandGui::SetRegisters ()
{
  //std::cout << "PolandGui::SetRegisters()"<< std::endl;


// write register values from strucure with gosipcmd
  theSetup_GET_FOR_SLAVE(PolandSetup);
//if (AssertNoBroadcast (false)) // NOTE: after change to broadcast action, this is always true JAM2017
if(!fBroadcasting) // use macro flag instead!
{
  // update trigger modes only in single device
  WriteGosip (fSFP, fSlave, POLAND_REG_INTERNAL_TRIGGER, theSetup->fInternalTrigger);
  WriteGosip (fSFP, fSlave, POLAND_REG_MASTERMODE, theSetup->fTriggerMode);
}

if(fDoResetQFW)
{
  WriteGosip (fSFP, fSlave, POLAND_REG_QFW_RESET, 0);
  WriteGosip (fSFP, fSlave, POLAND_REG_QFW_RESET, 1);
  //std::cout << "PolandGui::SetRegisters did reset QFW for ("<<fSFP<<", "<<fSlave<<")"<< std::endl;
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

theSetup->fCSASettings=ReadGosip (fSFP, fSlave, POLAND_REG_CSA_CTRL);

//printf("GetRegisters for sfp:%d slave:%d DUMP \n",fSFP, fSlave);
//theSetup->Dump();

// TODO: error handling with exceptions?
}



  void PolandGui::RefreshSensors()
  {

    //QString text;
    //QString pre;
    //fNumberBase==16? pre="0x" : pre="";
    theSetup_GET_FOR_SLAVE(PolandSetup);

    QString idtext=QString("POLAND Firmware version: 0x%1 [YYMMDDVV]").arg(theSetup->GetVersionId(),0,16);
    fPolandWidget->FirmwareLabel->setText(idtext);

////////////// enable this if you still want the temperatures etc displayed in hex
//    fPolandWidget->TBaseLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->TLogicLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->TStretchLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->TPiggy1LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->TPiggy2LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->TPiggy3LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->TPiggy4LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//
//    fPolandWidget->Fan1_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->Fan2_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->Fan3_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
//    fPolandWidget->Fan4_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
/////////////////////////////////////

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
    id_lsb = id_lsb & 0xFFFFFFFF; // clear high bits of long long
    id_msb= id_msb & 0xFFFFFF; // mask out upper crc word
    unsigned long long id= (id_msb << 32) | id_lsb;
    //printm("GetSensors for %d got id: 0x%llx. msb:0x%llx , lsb:0x%llx\n",t, id, id_msb, id_lsb);
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



void PolandGui::RefreshCSA()
 {
   theSetup_GET_FOR_SLAVE(PolandSetup);
   uint8_t feedback = 0;
   bool autorangemanual= false;
   bool inswitchbypass= false;
   bool outswitchbypass= false;
   theSetup->GetCSASettings(autorangemanual, inswitchbypass, outswitchbypass, feedback);
   //std::cout << "PolandGui::RefreshCSA() sees autorangemanual:"<<autorangemanual<<", inbypass:"<<inswitchbypass << ", outbypass:"<< outswitchbypass<<", feedback:"<< (int) feedback << std::endl;
   fPolandCSAWidget->CSA_feedback_spinBox->setValue(feedback);
   fPolandCSAWidget->CSA_autorange_manual_radioButton->setChecked(autorangemanual);
   fPolandCSAWidget->CSA_autorange_auto_radioButton->setChecked(!autorangemanual);
   fPolandCSAWidget->CSA_inswitch_bypass_radioButton->setChecked(inswitchbypass);
   fPolandCSAWidget->CSA_inswitch_tocsa_radioButton->setChecked(!inswitchbypass);
   fPolandCSAWidget->CSA_outswitch_bypass_radioButton->setChecked(outswitchbypass);
   fPolandCSAWidget->CSA_outswitch_fromcsa_radioButton->setChecked(!outswitchbypass);
   //std::cout << "PolandGui::RefreshCSA() sets registers from value 0x"<< std::hex << theSetup->GetCSAControl()<< std::dec<< std::endl;

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



///////////////////////////////////////////////////////////////////77
//////////////////// here additional code to display data for test:

void PolandGui::ShowSample ()
{
  //std::cout <<"ShowSample for channel:"<<channel<< std::endl;
  theSetup_GET_FOR_SLAVE(PolandSetup);
  GetSample(&(theSetup->fLastSample));
  fPolandViewpanelWidget->ShowSample(&(theSetup->fLastSample));
}


void PolandGui::GetSample(PolandSample* theSample)
{

  bool externaldaqmode=fPolandViewpanelWidget->ExternDAQRadioButton->isChecked();

  if(externaldaqmode)
  {

    int addr=0;
    int eventcounter = ReadGosip (fSFP, fSlave, addr);
    //printf (" - Internal Event number 0x%x\n", eventcounter);
    theSample->SetEventCounter(eventcounter);
    // board id calculated from SFP and device id:

//    pdata += 1;
//    QFWRAW_CHECK_PDATA;
//    int QfwSetup = *pdata;
//    printf (" - QFW SEtup %d\n", QfwSetup);
//    for (int j = 0; j < 4; ++j)
//    {
//      QFWRAW_CHECK_PDATA_BREAK;
//      pdata++;
//
//    }
    // skip 5 words of qfwsetup info here:
    addr += 20;

//    QFWRAW_CHECK_PDATA;
    for (int l = 0; l < POLAND_QFWLOOPS; l++)
    {
      theSample->SetLoopsize(l, ReadGosip (fSFP, fSlave, addr));
      //printf (" - Loopsize[%d] = 0x%x\n", l, theSample->GetLoopsize(l));
      addr +=4;
    }    // first loop loop

    for (int loop = 0; loop < POLAND_QFWLOOPS; loop++)
    {
      theSample->SetLooptime(loop, ReadGosip (fSFP, fSlave, addr));
      //printf (" - Looptime[%d] = 0x%x\n", loop, theSample->GetLooptime(loop));
      addr +=4;
    }    // second loop loop

    for (int j = 0; j < 21; ++j)
    {
      addr +=4;

    }
    /** All loops X slices/loop X channels */
    for (int loop = 0; loop < POLAND_QFWLOOPS; loop++)
    {
      for (int sl = 0; sl < theSample->GetLoopsize(loop); ++sl)
        for (int ch = 0; ch < POLAND_DAC_NUM; ++ch)
        {
          int value = ReadGosip (fSFP, fSlave, addr);
          theSample->AddTraceValue(loop, ch,value);
          addr +=4;
//            printf (" -- loop %d slice %d ch %d = 0x%x\n", loop, sl, ch, value);
        }
    }    //loop

    /* errorcount values: - per QFW CHIPS*/
    for (int qfw = 0; qfw < POLAND_ERRCOUNT_NUM; ++qfw)
    {
      theSample->SetErrorScaler(qfw, (unsigned int)  ReadGosip (fSFP, fSlave, addr));
      addr +=4;
      //printf (" - ErrorScaler[%d] = 0x%x\n", qfw, theSample->GetErrorScaler(qfw));
    }

  }
  else
  {
    // internal token request from GUI: only possible if we link against pexor C++ library
#ifdef USE_PEXOR_LIB
    theSetup_GET_FOR_SLAVE(PolandSetup);

      // estimate required payload from trace settings
    // open board instance
    pexor::PexorTwo board;
      if (!board.IsOpen ())
      {
        printm ("**** PolandGui::GetSample - Could not open pexor board!\n");
        return;
      }
// init dma buffers
     int numchains=SlavespinBox->maximum()+1;
     int numwords = 32 + theSetup->fSteps[0] * 32 + theSetup->fSteps[1] * 32 + theSetup->fSteps[2] * 32 + 32;
     int bytesPerBuffer=2* numchains * numwords*sizeof(int);
     //printm("PPPPP  numchains:%d numwords:%d  bytes:%d", numchains, numwords, bytesPerBuffer);
     int Bufnum=5;
      int rev = board.Add_DMA_Buffers (bytesPerBuffer, Bufnum);
       if (rev)
       {
         printm ("**** PolandGui::GetSample - Error %d on mapping dma buffers\n", rev);
         return;
       }


// token request
       pexor::DMA_Buffer* tokbuf=0;

       int BufID=0; // TODO: later member variable that is reset on pexor rest from framework?
       for(int t=0;t<2;++t)
       {
         // always get sample of both frontend buffers to be consistent after poland reset


#ifdef  POLANDGUI_SAMPLE_SYNCREADOUT
         // first variant: synchrous with polling in kernel module
                // would require very large gosipretries settings without trigger interrupt!
                // this can be dangerous if poland does not send anything (trigger off)


           tokbuf= board.RequestToken (fSFP, BufID | 2 , true, true);
           if (tokbuf <=  0)
           {
             printm ("**** PolandGui::GetSample Error in synchronous Token Request!!\n");
                   return ;
           }


#else
           int pollcounter =0;
           board.RequestToken (fSFP, BufID | 2 , false, true); // asynchronous mode here, wait for data ready |2, directdma on
           do
           {
             tokbuf= board.WaitForToken (fSFP, true, 0, 0, false); // directdma, 0, 0, syncmode
             pollcounter++;
             usleep(POLAND_SAMPLE_WAITCYCLE);
           } while (tokbuf == (pexor::DMA_Buffer*)(-1) && pollcounter<POLAND_SAMPLE_MAXPOLLS);

                if (tokbuf <=  0 || pollcounter>=POLAND_SAMPLE_MAXPOLLS)
                   {
                     printm ("**** PolandGui::GetSample Errorin async Token Request, pollcounter=%d \n",pollcounter);
                     return;
                   }

//                printm ("**** PolandGui::GetSample after %d polls with waittime %f us, token buffer:0x%x  \n",
//                    pollcounter, POLAND_SAMPLE_WAITCYCLE, (unsigned long) tokbuf);

#endif
                if(t==0){
                    // we only use first frontend buffer
                    UnpackQfw (tokbuf, theSample);
                    board.Free_DMA_Buffer (tokbuf);
                }
                BufID = (BufID == 0 ? 1 : 0);
                // still read second double buffer to be consistent for next request!
       } // for 2


// release board , cleanup
    board.Free_DMA_Buffer (tokbuf);
// ? error handling?


#else
    printm("Fetching gosip token with GUI is not supported without pexor/FESA library. Please use external DAQ.");
#endif

  }

}

#ifdef USE_PEXOR_LIB
// this function stolen and adopted from polandtest:
int PolandGui::UnpackQfw (pexor::DMA_Buffer* tokbuf, PolandSample* theSample)
{

///////////////// this code is mostly taken from Go4 unpacker at https://subversion.gsi.de/go4/app/qfw/pexor

  int loopsize[POLAND_QFWLOOPS];
  int looptime[POLAND_QFWLOOPS];

  int *pdata = tokbuf->Data ();
  int *pdatabegin = pdata;
  int lwords = tokbuf->UsedSize ()/sizeof(int); // this is true filled size from DMA, not total buffer lenght
  // loop over single subevent data:
  while (pdata - pdatabegin < lwords)
  {

    if ((*pdata & 0xff) != 0x34)    // regular channel data
    {
//     printf ("**** unpack_qfw: Skipping Non-header format 0x%x - (0x34 are expected) ...\n",
//         (*pdata & 0xff));
      pdata++;
      continue; // we have to skip it, since the dedicated padding pattern is added by mbs and not available here!
    }

    unsigned trig_type = (*pdata & 0xf00) >> 8;
    unsigned sfp_id = (*pdata & 0xf000) >> 12;
    unsigned device_id = (*pdata & 0xff0000) >> 16;
    unsigned channel_id = (*pdata & 0xff000000) >> 24;
    int* pdatastart = pdata;
    pdata++;

    int opticlen = *pdata++;
//    if(Debugmode)
      //printf ("Token header: trigid:0x%x sfp:0x%x modid:0x%x memid:0x%x opticlen:0x%x\n", trig_type, sfp_id, device_id,
      //channel_id, opticlen);
//    //
    if (opticlen > lwords * 4)
    {
      printm ("**** unpack_qfw: Mismatch with subevent len %d and optic len %d", lwords * 4, opticlen);
      // avoid that we run second step on invalid raw event!
      return -1;
    }
    QFWRAW_CHECK_PDATA;
    int eventcounter = *pdata;

    if(fSlave==device_id)
        theSample->SetEventCounter(eventcounter);

//    if(Debugmode)
    // printm (" - Internal Event number 0x%x\n", eventcounter);
    // board id calculated from SFP and device id:

    pdata += 1;
    QFWRAW_CHECK_PDATA;
    int QfwSetup = *pdata;
//    if(Debugmode)
//    printf (" - QFW SEtup %d\n", QfwSetup);
    for (int j = 0; j < 4; ++j)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      pdata++;

    }
    QFWRAW_CHECK_PDATA;
    for (int l = 0; l < POLAND_QFWLOOPS; l++)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      loopsize[l] = *pdata++;
      if(fSlave==device_id)
        theSample->SetLoopsize(l,loopsize[l]);
//      if(Debugmode)
 //       printm (" - Loopsize[%d] = 0x%x\n", l, loopsize[l]);
    }    // first loop loop

    QFWRAW_CHECK_PDATA;
    for (int loop = 0; loop < POLAND_QFWLOOPS; loop++)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      looptime[loop] = *pdata++;
      if(fSlave==device_id)
        theSample->SetLooptime(loop, looptime[loop]);
//      if(Debugmode)
 //       printm (" - Looptime[%d] = 0x%x\n", loop, looptime[loop]);
    }    // second loop loop

    for (int j = 0; j < 21; ++j)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      pdata++;

    }
    QFWRAW_CHECK_PDATA;
    /** All loops X slices/loop X channels */
    for (int loop = 0; loop < POLAND_QFWLOOPS; loop++)
    {
      for (int sl = 0; sl < loopsize[loop]; ++sl)
        for (int ch = 0; ch < POLAND_DAC_NUM; ++ch)
        {
          QFWRAW_CHECK_PDATA_BREAK;
          int value = *pdata++;
          if(fSlave==device_id)
            theSample->AddTraceValue(loop, ch,value);
//          if(Debugmode)
//           printm (" -- loop %d slice %d ch %d = 0x%x\n", loop, sl, ch, value);
        }
    }    //loop

    QFWRAW_CHECK_PDATA;
    /* errorcount values: - per QFW CHIPS*/
    for (int qfw = 0; qfw < POLAND_ERRCOUNT_NUM; ++qfw)
    {
      QFWRAW_CHECK_PDATA_BREAK;
      if(fSlave==device_id)
        theSample->SetErrorScaler(qfw,*pdata++);
      //      ErrorScaler[qfw] = (unsigned int) (*pdata++);

//      if(Debugmode)
//        printf (" - ErrorScaler[%d] = 0x%x\n", qfw, ErrorScaler[qfw]);
    }
    QFWRAW_CHECK_PDATA;

    // skip filler words at the end of gosip payload:
    while (pdata - pdatastart <= (opticlen / 4))    // note that trailer is outside opticlen!
    {
      //if(Debugmode)
      //printf("######### skipping word 0x%x\n ",*pdata);
      pdata++;
    }

    // crosscheck if trailer word matches eventcounter header
    if (*pdata != eventcounter)
    {
      printm ("!!!!! Eventcounter 0x%x does not match trailing word 0x%x at position 0x%x!\n", eventcounter, *pdata,
          (opticlen / 4));
    }
    else
    {
//      if(Debugmode)
        //printm ("Found trailing Eventcounter 0x%x \n",*pdata);
    }
    pdata++;
  }    // while pdata - pdatastart < lwords

////////////////////////////// end go4 unpacker

  return 0;
}


#endif
