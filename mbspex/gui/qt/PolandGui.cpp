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

  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
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
QObject::connect(SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
QObject::connect(SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));


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
EvaluateView ();
SetRegisters ();
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

//    QProcess proc;
//    proc.setProcessEnvironment(fEnv);
//    std::cout << "PolandGui::ResetBoardBtn() command:  "<< buffer << std::endl;
//    int rev=proc.execute(com);
//     if(rev<0)
//     {
//       std::cerr << "# PolandGui::WriteGosip() Error "<< rev <<" on executing "<< buffer <<" #!" << std::endl;
//     }

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

sleep(2);
WriteGosip (fChannel, fSlave, POLAND_REG_DO_OFFSET, 0);
AppendTextWindow ("    ... done.");
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
TextOutput->setPlainText ("Welcome to POLAND GUI!\n\t v0.1 10-July-2014 by JAM (j.adamczewski@gsi.de)");

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

void PolandGui::Slave_changed (int)
{
//std::cout << "PolandGui::Slave_changed" << std::endl;
EvaluateSlave ();
bool triggerchangeable = AssertNoBroadcast (false);
MasterTriggerBox->setEnabled (triggerchangeable);
InternalTriggerBox->setEnabled (triggerchangeable);
FesaModeBox->setEnabled (triggerchangeable);
RefreshButton->setEnabled (triggerchangeable);


}

void PolandGui::RefreshView ()
{
// display setup structure to gui:
QString text;
//text.setNum(fSetup.fSteps[0]);
TSLoop1lineEdit->setText (text.setNum (fSetup.fSteps[0], 10));
TSLoop2lineEdit->setText (text.setNum (fSetup.fSteps[1], 10));
TSLoop3lineEdit->setText (text.setNum (fSetup.fSteps[2], 10));
TS1TimelineEdit->setText (text.setNum (fSetup.GetStepTime(0)));
TS2TimelineEdit->setText (text.setNum (fSetup.GetStepTime(1)));
TS3TimelineEdit->setText (text.setNum (fSetup.GetStepTime(2)));
MasterTriggerBox->setChecked (fSetup.IsTriggerMaster ());
FesaModeBox->setChecked (fSetup.IsFesaMode ());
InternalTriggerBox->setChecked (fSetup.IsInternalTrigger ());

EventCounterNumber->display ((int) fSetup.fEventCounter);
ErrorCounter1->display ((int) fSetup.fErrorCounter[0]);
ErrorCounter2->display ((int) fSetup.fErrorCounter[1]);
ErrorCounter3->display ((int) fSetup.fErrorCounter[2]);
ErrorCounter4->display ((int) fSetup.fErrorCounter[3]);
ErrorCounter5->display ((int) fSetup.fErrorCounter[4]);
ErrorCounter6->display ((int) fSetup.fErrorCounter[5]);
ErrorCounter7->display ((int) fSetup.fErrorCounter[6]);
ErrorCounter8->display ((int) fSetup.fErrorCounter[7]);

}

void PolandGui::EvaluateView ()
{
EvaluateSlave ();
// copy widget values to structure
fSetup.fSteps[0] = TSLoop1lineEdit->text ().toUInt (0, 0);
fSetup.fSteps[1] = TSLoop2lineEdit->text ().toUInt (0, 0);
fSetup.fSteps[2] = TSLoop3lineEdit->text ().toUInt (0, 0);

fSetup.SetStepTime(TS1TimelineEdit->text ().toDouble (),0);
fSetup.SetStepTime(TS2TimelineEdit->text ().toDouble (),1);
fSetup.SetStepTime(TS3TimelineEdit->text ().toDouble (),2);


fSetup.SetTriggerMaster (MasterTriggerBox->isChecked ());
fSetup.SetFesaMode (FesaModeBox->isChecked ());
fSetup.SetInternalTrigger (InternalTriggerBox->isChecked ());

}
void PolandGui::EvaluateSlave ()
{
fChannel = SFPspinBox->value ();
fSlave = SlavespinBox->value ();
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

for (int i = 0; i < POLAND_TS_NUM; ++i)
{
  fSetup.fSteps[i] = ReadGosip (fChannel, fSlave, POLAND_REG_STEPS_BASE + 4 * i);
  fSetup.fTimes[i] = ReadGosip (fChannel, fSlave, POLAND_REG_TIME_BASE + 4 * i);
}
for (int e = 0; e < POLAND_ERRCOUNT_NUM; ++e)
{
  fSetup.fErrorCounter[e] = ReadGosip (fChannel, fSlave, POLAND_REG_ERRCOUNT_BASE + 4 * e);
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

///////////// OLD method:
// QProcess proc;
// printf("PolandGui::ReadGosip() buffer:%s command: %s\n",buffer, (char*) com.data());
//// std::cout << "PolandGui::ReadGosip() command:  "<< com.unicode() << std::endl;
//
// DebugTextWindow(buffer);
// proc.setProcessEnvironment(fEnv);
//
//
// proc.setReadChannel(QProcess::StandardOutput);
// proc.start(com);
//// if(proc.waitForReadyRead (1000)) // will give termination warnings after leaving this function
// if(proc.waitForFinished (5000)) // after process is finished we can still read stdio buffer
// {
//      // read back stdout of proc here
//    int retval=proc.readLine(buffer,1024);
//    if(retval<0)
//        {
//          std::cout << "PolandGui::ReadGosip() read result error "<< retval << std::endl;
//          return -1;
//        }
//    else
//    {
//      // TODO: how to handle broadcast results here? need array of values and list output in gui!
//      value=atoi(buffer);
//      std::cout << "PolandGui::ReadGosip() read value "<< value << " from buffer " << buffer << std::endl;
//      DebugTextWindow(buffer);
//
//    }
//
//
//  }
//else
//  {
//    std::cout << "PolandGui::ReadGosip(): gosipcmd not finished after 5 s error"<< std::endl;
//    DebugTextWindow("! Warning: command not finished after 5 s timeout !!!");
//    return -1;
//  }

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

// old interface:
//  QProcess proc;
//  proc.setProcessEnvironment(fEnv);
//  std::cout << "PolandGui::WriteGosip() command:  "<< buffer << std::endl;
//  DebugTextWindow(buffer);
//  rev=proc.execute(com);
//   if(rev<0)
//   {
//     std::cerr << "# PolandGui::WriteGosip() Error "<< rev <<" on executing "<< buffer <<" #!" << std::endl;
//     AppendTextWindow("! Error on executing command!!!");
//   }
return rev;
}

QString PolandGui::ExecuteGosipCmd (QString& com)
{
// interface to shell gosipcmd
// TODO optionally some remote call via ssh for Go4 gui?
QString result;
QProcess proc;
DebugTextWindow (com);
proc.setProcessEnvironment (fEnv);
proc.setReadChannel (QProcess::StandardOutput);
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

