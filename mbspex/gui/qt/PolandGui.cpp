#include "PolandGui.h"

#include <stdlib.h>
#include <unistd.h>

//#include <stdio.h>
#include <iostream>
//#include <QProcess>
#include <stdlib.h>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>

//#include "Riostream.h"
//#include "Rstrstream.h"
//#include "TClass.h"
//#include "TCanvas.h"
//#include "TGo4Proxy.h"
//#include "QRootCanvas.h"

#include <sstream>

//// this function called by Go4 GUI to start user panel
//extern "C" Q_DECL_EXPORT void* StartUserPanel(void* parent)
//{
//
//   return new PolandGui((QWidget*) parent);
//
////  Use this code to hide main go4 window and show only user gui
//
////  QWidget* w = (QWidget*) parent;
////  w->parentWidget()->hide();
////   return new PolandGui(0);
//
//}

// *********************************************************

/*
 *  Constructs a PolandGui which is a child of 'parent', with the
 *  name 'name'.'
 */
PolandGui::PolandGui (QWidget* parent) :
    QWidget (parent), fDebug (false), fChannel (0), fSlave (0)
{
  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
#endif

  fNumberBase=10;

  SFPspinBox->setValue (fChannel);
  SlavespinBox->setValue (fSlave);

  TextOutput->setCenterOnScroll (false);
  ClearOutputBtn_clicked ();

  QObject::connect (RefreshButton, SIGNAL (clicked ()), this, SLOT (ShowBtn_clicked ()));
  QObject::connect (ApplyButton, SIGNAL (clicked ()), this, SLOT (ApplyBtn_clicked ()));

  QObject::connect (InitChainButton, SIGNAL (clicked ()), this, SLOT (InitChainBtn_clicked ()));
  QObject::connect (ResetBoardButton, SIGNAL (clicked ()), this, SLOT (ResetBoardBtn_clicked ()));
  QObject::connect (BroadcastButton, SIGNAL (clicked ()), this, SLOT (BroadcastBtn_clicked ()));
  QObject::connect (DumpButton, SIGNAL (clicked ()), this, SLOT (DumpBtn_clicked ()));
  QObject::connect (ConfigButton, SIGNAL (clicked ()), this, SLOT (ConfigBtn_clicked ()));
  QObject::connect (ClearOutputButton, SIGNAL (clicked ()), this, SLOT (ClearOutputBtn_clicked ()));
  QObject::connect (OffsetButton, SIGNAL (clicked ()), this, SLOT (OffsetBtn_clicked ()));
QObject::connect(DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
QObject::connect(HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));
QObject::connect(SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
QObject::connect(SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));

QObject::connect(DACModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DACMode_changed(int)));


}

PolandGui::~PolandGui ()
{

}

void PolandGui::ShowBtn_clicked ()
{
//std::cout << "PolandGui::ShowBtn_clicked()"<< std::endl;
EvaluateSlave ();
GetRegisters ();
RefreshView ();
}

void PolandGui::ApplyBtn_clicked ()
{
//std::cout << "PolandGui::ApplyBtn_clicked()"<< std::endl;

  char buffer[1024];
  char description[32];
  (QFW_DAC_tabWidget->currentIndex()==0) ? snprintf (description, 32, "QFW") : snprintf (description, 32, "DAC");
  EvaluateSlave ();
  //std::cout << "InitChainBtn_clicked()"<< std::endl;
  snprintf (buffer, 1024, "Really apply %s settings  to SFP %d Device %d?", description, fChannel, fSlave);
  if (QMessageBox::question (this, "Poland GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    return;
  }
EvaluateView ();
EvaluateDAC();

// depending on activated view, we either set qfw parameters or change DAC programming
if(QFW_DAC_tabWidget->currentIndex()==0)
{
  SetRegisters ();
}
else if (QFW_DAC_tabWidget->currentIndex()==1)
{
  ApplyDAC();
}

}

void PolandGui::InitChainBtn_clicked ()
{
char buffer[1024];
EvaluateSlave ();
//std::cout << "InitChainBtn_clicked()"<< std::endl;
snprintf (buffer, 1024, "Really initialize SFP chain %d with %d Slaves?", fChannel, fSlave);
if (QMessageBox::question (this, "Poland GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
    != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}
snprintf (buffer, 1024, "gosipcmd -i %d %d", fChannel, fSlave);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);

//     QProcess proc;
//     proc.setProcessEnvironment(fEnv);
//     std::cout << "PolandGui::ResetBoardBtn() command:  "<< buffer << std::endl;
//     int rev=proc.execute(com);
//      if(rev<0)
//      {
//        std::cerr << "# PolandGui::WriteGosip() Error "<< rev <<" on executing "<< buffer <<" #!" << std::endl;
//      }

}

void PolandGui::ResetBoardBtn_clicked ()
{
//std::cout << "PolandGui::ResetBoardBtn_clicked"<< std::endl;
if (QMessageBox::question (this, "Poland GUI", "Really Reset gosip on pex board?", QMessageBox::Yes | QMessageBox::No,
    QMessageBox::Yes) != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}

char buffer[1024];
snprintf (buffer, 1024, "gosipcmd -z");
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);


}

void PolandGui::OffsetBtn_clicked ()
{
char buffer[1024];
EvaluateSlave ();
snprintf (buffer, 1024, "Really scan offset for SFP chain %d, Slave %d ?", fChannel, fSlave);
if (QMessageBox::question (this, "Poland GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
    != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}

WriteGosip (fChannel, fSlave, POLAND_REG_DO_OFFSET, 1);
AppendTextWindow ("--- Doing offset measurement... ");
QApplication::setOverrideCursor( Qt::WaitCursor );

WriteGosip (fChannel, fSlave, POLAND_REG_DO_OFFSET, 0);
sleep(2);
AppendTextWindow ("    ... done. Dumping offset values:");

snprintf (buffer, 1024, "gosipcmd -d -r -x -- %d %d 0x%x 0x%x", fChannel, fSlave, POLAND_REG_OFFSET_BASE, 32);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);
QApplication::restoreOverrideCursor();




}

void PolandGui::BroadcastBtn_clicked ()
{
//std::cout << "PolandGui::BroadcastBtn_clicked"<< std::endl;
SFPspinBox->setValue (-1);
SlavespinBox->setValue (-1);
}

void PolandGui::DumpBtn_clicked ()
{
//std::cout << "PolandGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
EvaluateSlave ();
char buffer[1024];
AppendTextWindow ("--- Register Dump ---:");

int numwords = 32 + fSetup.fSteps[0] * 32 + fSetup.fSteps[1] * 32 + fSetup.fSteps[2] * 32 + 32;
// todo: note this will not work for broadcast mode or if show was not clicked before
// we can live with it for the moment.

snprintf (buffer, 1024, "gosipcmd -d -r -x -- %d %d 0 0x%x", fChannel, fSlave, numwords);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);

}

void PolandGui::ClearOutputBtn_clicked ()
{
//std::cout << "PolandGui::ClearOutputBtn_clicked()"<< std::endl;
TextOutput->clear ();
TextOutput->setPlainText ("Welcome to POLAND GUI!\n\t v0.3 of 16-July-2014 by JAM (j.adamczewski@gsi.de)");

}

void PolandGui::ConfigBtn_clicked ()
{
//std::cout << "PolandGui::ConfigBtn_clicked" << std::endl;

// here file requester and application of set up via gosipcmd
QFileDialog fd (this, "Select POLAND configuration file", ".", "gosipcmd file (*.gos)");
fd.setFileMode (QFileDialog::ExistingFile);
if (fd.exec () != QDialog::Accepted)
  return;
QStringList flst = fd.selectedFiles ();
if (flst.isEmpty ())
  return;
QString fileName = flst[0];
if (!fileName.endsWith (".gos"))
  fileName.append (".gos");
char buffer[1024];
snprintf (buffer, 1024, "gosipcmd -x -c %s ", fileName.toLatin1 ().constData ());
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);


}

void PolandGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
fDebug = on;
}

void PolandGui::HexBox_changed(int on)
{
  fNumberBase= (on ? 16 :10);
  //std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;
  RefreshView ();
}


void PolandGui::Slave_changed (int)
{
//std::cout << "PolandGui::Slave_changed" << std::endl;
EvaluateSlave ();
bool triggerchangeable = AssertNoBroadcast (false);
MasterTriggerBox->setEnabled (triggerchangeable);
InternalTriggerBox->setEnabled (triggerchangeable);
FesaModeBox->setEnabled (triggerchangeable);
RefreshButton->setEnabled (triggerchangeable);
//if(triggerchangeable) ShowBtn_clicked (); // automatic update of values?


}


void PolandGui::DACMode_changed(int ix)
{
  //std::cout << "PolandGui::DACMode_changed to index:"<< ix << std::endl;
  fSetup.fDACMode= ix+1;
  //GetRegisters();
  RefreshDAC();


}

void PolandGui::RefreshView ()
{
// display setup structure to gui:
QString text;
//text.setNum(fSetup.fSteps[0]);
RefreshMode();


TSLoop1lineEdit->setText (text.setNum (fSetup.fSteps[0], fNumberBase));
TSLoop2lineEdit->setText (text.setNum (fSetup.fSteps[1], fNumberBase));
TSLoop3lineEdit->setText (text.setNum (fSetup.fSteps[2], fNumberBase));
TS1TimelineEdit->setText (text.setNum (fSetup.GetStepTime(0)));
TS2TimelineEdit->setText (text.setNum (fSetup.GetStepTime(1)));
TS3TimelineEdit->setText (text.setNum (fSetup.GetStepTime(2)));
MasterTriggerBox->setChecked (fSetup.IsTriggerMaster ());
FesaModeBox->setChecked (fSetup.IsFesaMode ());
InternalTriggerBox->setChecked (fSetup.IsInternalTrigger ());


EventCounterNumber->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter1->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter2->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter3->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter4->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter5->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter6->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter7->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter8->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);


EventCounterNumber->display ((int) fSetup.fEventCounter);
ErrorCounter1->display ((int) fSetup.fErrorCounter[0]);
ErrorCounter2->display ((int) fSetup.fErrorCounter[1]);
ErrorCounter3->display ((int) fSetup.fErrorCounter[2]);
ErrorCounter4->display ((int) fSetup.fErrorCounter[3]);
ErrorCounter5->display ((int) fSetup.fErrorCounter[4]);
ErrorCounter6->display ((int) fSetup.fErrorCounter[5]);
ErrorCounter7->display ((int) fSetup.fErrorCounter[6]);
ErrorCounter8->display ((int) fSetup.fErrorCounter[7]);

RefreshDACMode();
RefreshDAC(); // probably this is already triggered by signal

}

void PolandGui::EvaluateView ()
{
EvaluateSlave ();
EvaluateMode  ();
// copy widget values to structure
fSetup.fSteps[0] = TSLoop1lineEdit->text ().toUInt (0, fNumberBase);
fSetup.fSteps[1] = TSLoop2lineEdit->text ().toUInt (0, fNumberBase);
fSetup.fSteps[2] = TSLoop3lineEdit->text ().toUInt (0, fNumberBase);

fSetup.SetStepTime(TS1TimelineEdit->text ().toDouble (),0);
fSetup.SetStepTime(TS2TimelineEdit->text ().toDouble (),1);
fSetup.SetStepTime(TS3TimelineEdit->text ().toDouble (),2);


fSetup.SetTriggerMaster (MasterTriggerBox->isChecked ());
fSetup.SetFesaMode (FesaModeBox->isChecked ());
fSetup.SetInternalTrigger (InternalTriggerBox->isChecked ());

}


void PolandGui::EvaluateMode()
{
  int index=ModeComboBox->currentIndex();
  switch(index)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      fSetup.fQFWMode= index;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      fSetup.fQFWMode= index -4  + 16;
    break;
    default:
      std::cout << "!!! Never come here - undefined mode index"<< index << std::endl;
    break;
  };
}
void PolandGui::RefreshMode()
{
  int index=-1;
  switch(fSetup.fQFWMode)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      index=fSetup.fQFWMode;
      break;
    case 16:
    case 17:
    case 18:
    case 19:
      index= fSetup.fQFWMode- 16 + 4;
      break;
    default:
          std::cout << "!!! Never come here - undefined mode index"<< index << std::endl;
          break;
  };
  if(index>=0)
    ModeComboBox->setCurrentIndex(index);

}

void PolandGui::EvaluateSlave ()
{
fChannel = SFPspinBox->value ();
fSlave = SlavespinBox->value ();
}



void PolandGui::EvaluateDAC()
{

  if(fSetup.fDACMode==4)
  {
  fSetup.fDACAllValue=DACStartValueLineEdit->text ().toUInt (0, fNumberBase);

  }
  else
  {
    fSetup.fDACStartValue=DACStartValueLineEdit->text ().toUInt (0, fNumberBase);
  }


  fSetup.fDACOffset=DACOffsetLineEdit->text ().toUInt (0, fNumberBase);
  fSetup.fDACDelta=DACDeltaOffsetLineEdit->text ().toUInt (0, fNumberBase);
  fSetup.SetCalibrationTime(DACCalibTimeLineEdit->text ().toDouble ());



if(fSetup.fDACMode==1)
{
  // only manual mode will refresh DAQ structure here
  fSetup.fDACValue[0]=DAClineEdit_1->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[1]=DAClineEdit_2->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[2]=DAClineEdit_3->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[3]=DAClineEdit_4->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[4]=DAClineEdit_5->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[5]=DAClineEdit_6->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[6]=DAClineEdit_7->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[7]=DAClineEdit_8->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[8]=DAClineEdit_9->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[9]=DAClineEdit_10->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[10]=DAClineEdit_11->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[11]=DAClineEdit_12->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[12]=DAClineEdit_13->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[13]=DAClineEdit_14->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[14]=DAClineEdit_15->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[15]=DAClineEdit_16->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[16]=DAClineEdit_17->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[17]=DAClineEdit_18->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[18]=DAClineEdit_19->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[19]=DAClineEdit_20->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[20]=DAClineEdit_21->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[21]=DAClineEdit_22->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[22]=DAClineEdit_23->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[23]=DAClineEdit_24->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[24]=DAClineEdit_25->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[25]=DAClineEdit_26->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[26]=DAClineEdit_27->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[27]=DAClineEdit_28->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[28]=DAClineEdit_29->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[29]=DAClineEdit_30->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[30]=DAClineEdit_31->text ().toUInt (0, fNumberBase);
fSetup.fDACValue[31]=DAClineEdit_32->text ().toUInt (0, fNumberBase);
}

}


void  PolandGui::ApplyDAC()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  WriteGosip (fChannel, fSlave, POLAND_REG_DAC_MODE , fSetup.fDACMode);

  switch((int) fSetup.fDACMode)
  {
    case 1:
      // manual settings:
      for (int d = 0; d < POLAND_DAC_NUM; ++d)
      {
        WriteGosip (fChannel, fSlave, POLAND_REG_DAC_BASE_WRITE + 4 * d, fSetup.fDACValue[d]);
      }
      break;
    case 2:
      // test structure:
      // no more actions needed
      break;
    case 3:
      // issue calibration:
      WriteGosip (fChannel, fSlave, POLAND_REG_DAC_CAL_STARTVAL , fSetup.fDACStartValue);
      WriteGosip (fChannel, fSlave, POLAND_REG_DAC_CAL_OFFSET ,  fSetup.fDACOffset);
      WriteGosip (fChannel, fSlave, POLAND_REG_DAC_CAL_DELTA ,  fSetup.fDACDelta);
      WriteGosip (fChannel, fSlave, POLAND_REG_DAC_CAL_TIME ,  fSetup.fDACCalibTime);

      break;
    case 4:
      // all same constant value mode:
      WriteGosip (fChannel, fSlave, POLAND_REG_DAC_ALLVAL , fSetup.fDACAllValue);
      break;

    default:
      std::cout << "!!! ApplyDAC Never come here - undefined DAC mode"<<  fSetup.fDACMode << std::endl;
      break;

  };


  WriteGosip (fChannel, fSlave, POLAND_REG_DAC_PROGRAM , 1);
  WriteGosip (fChannel, fSlave, POLAND_REG_DAC_PROGRAM , 0);
  sleep(2);

  QApplication::restoreOverrideCursor();
}

void  PolandGui::RefreshDAC()
{
  //std::cout << "PolandGui::RefreshDAC"<< std::endl;
  QString text;






  DACOffsetLineEdit->setText (text.setNum (fSetup.fDACOffset, fNumberBase));
  DACDeltaOffsetLineEdit->setText (text.setNum (fSetup.fDACDelta, fNumberBase));
  DACCalibTimeLineEdit->setText (text.setNum (fSetup.GetCalibrationTime()));

  DAClineEdit_1->setText (text.setNum (fSetup.fDACValue[0], fNumberBase));
  DAClineEdit_2->setText (text.setNum (fSetup.fDACValue[1], fNumberBase));
  DAClineEdit_3->setText (text.setNum (fSetup.fDACValue[2], fNumberBase));
  DAClineEdit_4->setText (text.setNum (fSetup.fDACValue[3], fNumberBase));
  DAClineEdit_5->setText (text.setNum (fSetup.fDACValue[4], fNumberBase));
  DAClineEdit_6->setText (text.setNum (fSetup.fDACValue[5], fNumberBase));
  DAClineEdit_7->setText (text.setNum (fSetup.fDACValue[6], fNumberBase));
  DAClineEdit_8->setText (text.setNum (fSetup.fDACValue[7], fNumberBase));
  DAClineEdit_9->setText (text.setNum (fSetup.fDACValue[8], fNumberBase));
  DAClineEdit_10->setText (text.setNum (fSetup.fDACValue[9], fNumberBase));
  DAClineEdit_10->setText (text.setNum (fSetup.fDACValue[10], fNumberBase));
  DAClineEdit_12->setText (text.setNum (fSetup.fDACValue[11], fNumberBase));
  DAClineEdit_13->setText (text.setNum (fSetup.fDACValue[12], fNumberBase));
  DAClineEdit_14->setText (text.setNum (fSetup.fDACValue[13], fNumberBase));
  DAClineEdit_15->setText (text.setNum (fSetup.fDACValue[14], fNumberBase));
  DAClineEdit_16->setText (text.setNum (fSetup.fDACValue[15], fNumberBase));
  DAClineEdit_17->setText (text.setNum (fSetup.fDACValue[16], fNumberBase));
  DAClineEdit_18->setText (text.setNum (fSetup.fDACValue[17], fNumberBase));
  DAClineEdit_19->setText (text.setNum (fSetup.fDACValue[18], fNumberBase));
  DAClineEdit_20->setText (text.setNum (fSetup.fDACValue[19], fNumberBase));
  DAClineEdit_21->setText (text.setNum (fSetup.fDACValue[20], fNumberBase));
  DAClineEdit_22->setText (text.setNum (fSetup.fDACValue[21], fNumberBase));
  DAClineEdit_23->setText (text.setNum (fSetup.fDACValue[22], fNumberBase));
  DAClineEdit_24->setText (text.setNum (fSetup.fDACValue[23], fNumberBase));
  DAClineEdit_25->setText (text.setNum (fSetup.fDACValue[24], fNumberBase));
  DAClineEdit_26->setText (text.setNum (fSetup.fDACValue[25], fNumberBase));
  DAClineEdit_27->setText (text.setNum (fSetup.fDACValue[26], fNumberBase));
  DAClineEdit_28->setText (text.setNum (fSetup.fDACValue[27], fNumberBase));
  DAClineEdit_29->setText (text.setNum (fSetup.fDACValue[28], fNumberBase));
  DAClineEdit_30->setText (text.setNum (fSetup.fDACValue[29], fNumberBase));
  DAClineEdit_31->setText (text.setNum (fSetup.fDACValue[30], fNumberBase));
  DAClineEdit_32->setText (text.setNum (fSetup.fDACValue[31], fNumberBase));

  // depending on DAC mode different fields are writable:

  //std::cout << "!!! RefreshDAC With DAC mode="<<  (int)fSetup.fDACMode << std::endl;
  switch((int) fSetup.fDACMode)
  {
    case 1:
      // manual settings:
      DACscrollArea->setEnabled(true);
      DACCaliFrame->setEnabled(false);

      break;
    case 2:
      // test structure
      DACscrollArea->setEnabled(false);
      DACCaliFrame->setEnabled(false);
          break;
    case 3:
          // calibrate mode
      DACscrollArea->setEnabled(false);
      DACCaliFrame->setEnabled(true);
      DACStartValueLineEdit->setEnabled(true);
      DACStartValueLineEdit->setText (text.setNum (fSetup.fDACStartValue, fNumberBase));
      DACOffsetLineEdit->setEnabled(true);
      DACDeltaOffsetLineEdit->setEnabled(true);
      DACCalibTimeLineEdit->setEnabled(true);

          break;
    case 4:
          // all constant
      DACscrollArea->setEnabled(false);
      DACCaliFrame->setEnabled(true);
      DACStartValueLineEdit->setEnabled(true);
      DACStartValueLineEdit->setText (text.setNum (fSetup.fDACAllValue, fNumberBase));
      DACOffsetLineEdit->setEnabled(false);
      DACDeltaOffsetLineEdit->setEnabled(false);
      DACCalibTimeLineEdit->setEnabled(false);

          break;
    default:
      std::cout << "!!! RefreshDAC Never come here - undefined DAC mode"<<  fSetup.fDACMode << std::endl;
      break;

  };



}


void PolandGui::RefreshDACMode()
{
  //std::cout << "PolandGui::RefreshDACMode for mode "<< (int) fSetup.fDACMode << std::endl;
  DACModeComboBox->setCurrentIndex((int) fSetup.fDACMode -1);
}


void PolandGui::SetRegisters ()
{
// write register values from strucure with gosipcmd

if (AssertNoBroadcast (false))
{
  // update trigger modes only in single device
  WriteGosip (fChannel, fSlave, POLAND_REG_INTERNAL_TRIGGER, fSetup.fInternalTrigger);
  WriteGosip (fChannel, fSlave, POLAND_REG_MASTERMODE, fSetup.fTriggerMode);
}

WriteGosip (fChannel, fSlave, POLAND_REG_QFW_MODE, fSetup.fQFWMode);


// WriteGosip(fChannel, fSlave, POLAND_REG_TRIGCOUNT, fSetup.fEventCounter);

for (int i = 0; i < POLAND_TS_NUM; ++i)
{
  WriteGosip (fChannel, fSlave, POLAND_REG_STEPS_BASE + 4 * i, fSetup.fSteps[i]);
  WriteGosip (fChannel, fSlave, POLAND_REG_TIME_BASE + 4 * i, fSetup.fTimes[i]);
}
//    for(int e=0; e<POLAND_ERRCOUNT_NUM;++e)
//     {
//       WriteGosip(fChannel, fSlave, POLAND_REG_ERRCOUNT_BASE + 4*e, fSetup.fErrorCounter[e]);
//     }

// TODO: error handling with exceptions?

}

void PolandGui::GetRegisters ()
{
// read register values into structure with gosipcmd

if (!AssertNoBroadcast ())
  return;

fSetup.fInternalTrigger = ReadGosip (fChannel, fSlave, POLAND_REG_INTERNAL_TRIGGER);
fSetup.fTriggerMode = ReadGosip (fChannel, fSlave, POLAND_REG_MASTERMODE);
fSetup.fEventCounter = ReadGosip (fChannel, fSlave, POLAND_REG_TRIGCOUNT);
fSetup.fQFWMode = ReadGosip (fChannel, fSlave, POLAND_REG_QFW_MODE);

for (int i = 0; i < POLAND_TS_NUM; ++i)
{
  fSetup.fSteps[i] = ReadGosip (fChannel, fSlave, POLAND_REG_STEPS_BASE + 4 * i);
  fSetup.fTimes[i] = ReadGosip (fChannel, fSlave, POLAND_REG_TIME_BASE + 4 * i);
}
for (int e = 0; e < POLAND_ERRCOUNT_NUM; ++e)
{
  fSetup.fErrorCounter[e] = ReadGosip (fChannel, fSlave, POLAND_REG_ERRCOUNT_BASE + 4 * e);
}

fSetup.fDACMode=ReadGosip (fChannel, fSlave, POLAND_REG_DAC_MODE);
fSetup.fDACCalibTime=ReadGosip (fChannel, fSlave, POLAND_REG_DAC_CAL_TIME);
fSetup.fDACOffset=ReadGosip (fChannel, fSlave, POLAND_REG_DAC_CAL_OFFSET);
fSetup.fDACStartValue=ReadGosip (fChannel, fSlave, POLAND_REG_DAC_CAL_STARTVAL);

for (int d = 0; d < POLAND_DAC_NUM; ++d)
{
  fSetup.fDACValue[d] = ReadGosip (fChannel, fSlave, POLAND_REG_DAC_BASE_READ + 4 * d);
}





//printf("GetRegisters for sfp:%d slave:%d DUMP \n",fChannel, fSlave);
//fSetup.Dump();

// TODO: error handling with exceptions?
}

int PolandGui::ReadGosip (int sfp, int slave, int address)
{
int value = -1;

char buffer[1024];
//snprintf(buffer,1024,"/daq/usr/adamczew/workspace/drivers/mbspex/bin/gosipcmd -r -- %d %d 0x%x",sfp, slave, address);
snprintf (buffer, 1024, "gosipcmd -r -- %d %d 0x%x", sfp, slave, address);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
if (result != "ERROR")
{
  DebugTextWindow (result);
  value = result.toInt (0, 0);
}
else
{

  value = -1;
}


return value;
}

int PolandGui::WriteGosip (int sfp, int slave, int address, int value)
{
int rev = 0;
char buffer[1024];
snprintf (buffer, 1024, "gosipcmd -w -- %d %d 0x%x 0x%x", sfp, slave, address, value);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
if (result == "ERROR")
  rev = -1;
return rev;
}

QString PolandGui::ExecuteGosipCmd (QString& com)
{
// interface to shell gosipcmd
// TODO optionally some remote call via ssh for Go4 gui?
QString result;
QProcess proc;
DebugTextWindow (com);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
proc.setProcessEnvironment (fEnv);
#endif
proc.setReadChannel (QProcess::StandardOutput);
QApplication::setOverrideCursor( Qt::WaitCursor );

proc.start (com);
// if(proc.waitForReadyRead (1000)) // will give termination warnings after leaving this function
if (proc.waitForFinished (5000))    // after process is finished we can still read stdio buffer
{
  // read back stdout of proc here
  result = proc.readAll ();
}
else
{
  std::cout << " PolandGui::ExecuteGosipCmd(): gosipcmd not finished after 5 s error" << std::endl;
  AppendTextWindow ("! Warning: ExecuteGosipCmd not finished after 5 s timeout !!!");
  result = "ERROR";
}
QApplication::restoreOverrideCursor();
return result;
}

void PolandGui::AppendTextWindow (const QString& text)
{
TextOutput->appendPlainText (text);
TextOutput->update ();
}

bool PolandGui::AssertNoBroadcast (bool verbose)
{
if (fChannel < 0 || fSlave < 0)
{
  //std::cerr << "# PolandGui Error: broadcast not supported here!" << std::endl;
  if (verbose)
    AppendTextWindow ("#Error: broadcast not supported here!");
  return false;
}
return true;
}

