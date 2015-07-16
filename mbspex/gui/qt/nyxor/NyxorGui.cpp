#include "NyxorGui.h"

#include <stdlib.h>
#include <unistd.h>

//#include <stdio.h>
#include <iostream>
//#include <QProcess>
#include <stdlib.h>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>



#include <sstream>

#include "nxyterwidget.h"

// *********************************************************




/*
 *  Constructs a NyxorGui which is a child of 'parent', with the
 *  name 'name'.'
 */
NyxorGui::NyxorGui (QWidget* parent) :
    QWidget (parent), fDebug (false), fChannel (0), fSlave (0), fChannelSave (0), fSlaveSave (0)
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
  QObject::connect (ResetSlaveButton, SIGNAL (clicked ()), this, SLOT (ResetSlaveBtn_clicked ()));
  QObject::connect (BroadcastButton, SIGNAL (clicked (bool)), this, SLOT (BroadcastBtn_clicked (bool)));
  QObject::connect (DumpButton, SIGNAL (clicked ()), this, SLOT (DumpBtn_clicked ()));
  QObject::connect (ConfigButton, SIGNAL (clicked ()), this, SLOT (ConfigBtn_clicked ()));
  QObject::connect (ClearOutputButton, SIGNAL (clicked ()), this, SLOT (ClearOutputBtn_clicked ()));

QObject::connect(DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
QObject::connect(HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));
QObject::connect(SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
QObject::connect(SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));



// here components for nxyter tabs:

for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    NxyterWidget* nxw = new NxyterWidget (Nxyter_tabWidget, this, nx);
    Nxyter_tabWidget->addTab (nxw, QString ("NX%1").arg (nx));
    fNxTab[nx] = nxw;
  }





show();
}

NyxorGui::~NyxorGui ()
{

}

void NyxorGui::ShowBtn_clicked ()
{
//std::cout << "NyxorGui::ShowBtn_clicked()"<< std::endl;
EvaluateSlave ();
GetRegisters ();
RefreshView ();
}

void NyxorGui::ApplyBtn_clicked ()
{
//std::cout << "NyxorGui::ApplyBtn_clicked()"<< std::endl;

  char buffer[1024];
  EvaluateSlave ();
  //std::cout << "InitChainBtn_clicked()"<< std::endl;
  snprintf (buffer, 1024, "Really apply NYXOR settings  to SFP %d Device %d?", fChannel, fSlave);
  if (QMessageBox::question (this, "NYXOR GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    return;
  }
//EvaluateView ();


// here we call subguis for nxyters to do setup:
SetRegisters ();

}

void NyxorGui::InitChainBtn_clicked ()
{
char buffer[1024];
EvaluateSlave ();
//std::cout << "InitChainBtn_clicked()"<< std::endl;
bool ok;
snprintf (buffer, 1024, "Please specify NUMBER OF DEVICES to initialize at SFP %d ?", fChannel);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
int numslaves = QInputDialog::getInt(this, tr("Number of Slaves?"),
                                 tr(buffer), 1, 1, 1024, 1, &ok);
#else
int numslaves = QInputDialog::getInteger(this, tr("Number of Slaves?"),
                                 tr(buffer), 1, 1, 1024, 1, &ok);

#endif
if (!ok) return;
if(fChannel<0)
{
    AppendTextWindow ("--- Error: Broadcast not allowed for init chain!");
    return;
}
snprintf (buffer, 1024, "gosipcmd -i  %d %d", fChannel, numslaves);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);


}

void NyxorGui::ResetBoardBtn_clicked ()
{
//std::cout << "NyxorGui::ResetBoardBtn_clicked"<< std::endl;
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

void NyxorGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
  snprintf (buffer, 1024, "Really reset logic on NYXOR/GEMEX device at SFP %d, Slave %d ?", fChannel, fSlave);
  if (QMessageBox::question (this, "Nyxor GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }

  AppendTextWindow ("--- Resetting logic on NYXOR... ");


  WriteGosip(fChannel, fSlave, GOS_I2C_DWR, 0x7f000000);

  int dat=0;
  dat  = 0x8c;
  dat += 0x0              <<  8;
  dat  += 0x0              << 16;
  dat += I2C_CTRL_A       << 24;

  WriteGosip(fChannel, fSlave, GOS_I2C_DWR, dat);


//  // has to be changed a bit if more than one nxy are connected to
//          // one GEMEX/NYXOR
//          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, 0x7f000000);
//          if (l_stat == -1)
//          {
//            printf ("ERROR>> GEMEX/NYXOR and nXYter reset failed: \n");
//            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
//                                 l_i, l_j, GOS_I2C_DWR, 0x7f000000);
//            printf ("exiting.. \n");
//            exit (0);
//          }
//
//          // activate I2C core and reset nXYter
//          l_wr_d  = l_reset[l_i][l_j][l_k];
//          l_wr_d += 0x0              <<  8;
//          l_wr_d += 0x0              << 16;
//          l_wr_d += I2C_CTRL_A       << 24;
//
//          l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
//          if (l_stat == -1)
//          {
//            printf ("ERROR>> activating I2C core failed: \n");
//            printf ("        SFP: %d, NYX: %d => A: 0x%x, D: 0x%x\n",
//                                 l_i, l_j, GOS_I2C_DWR, l_wr_d);
//            printf ("exiting.. \n");
//            exit (0);
//          }





}






void NyxorGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "NyxorGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
  if (checked)
  {
    fChannelSave = SFPspinBox->value ();
    fSlaveSave = SlavespinBox->value ();
    SFPspinBox->setValue (-1);
    SlavespinBox->setValue (-1);
  }
  else
  {
    SFPspinBox->setValue (fChannelSave);
    SlavespinBox->setValue (fSlaveSave);

  }
}

void NyxorGui::DumpBtn_clicked ()
{
//std::cout << "NyxorGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
EvaluateSlave ();

if (!AssertNoBroadcast ())
  return; // for nyxor we can not dump all connected frontends at once, NxI2c works on current slave only

char buffer[1024];
AppendTextWindow ("--- Register Dump ---:");

std::stringstream buf;
for (int nx = 0; nx < NYXOR_NUMNX; nx++)
    {
      fNxTab[nx]->dumpConfig(buf);
    }

AppendTextWindow (buf.str().c_str());

}

void NyxorGui::ClearOutputBtn_clicked ()
{
//std::cout << "NyxorGui::ClearOutputBtn_clicked()"<< std::endl;
TextOutput->clear ();
TextOutput->setPlainText ("Welcome to NYXOR GUI!\n\t v0.3 of 16-July-2015 by JAM (j.adamczewski@gsi.de)\n\tContains parts of ROC/nxyter GUI by  Sergey Linev, GSI");

}

void NyxorGui::ConfigBtn_clicked ()
{
//std::cout << "NyxorGui::ConfigBtn_clicked" << std::endl;

// here file requester and application of set up via gosipcmd
QFileDialog fd (this, "Select NYXOR configuration file", ".", "gosipcmd file (*.gos)");
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

void NyxorGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
fDebug = on;
}

void NyxorGui::HexBox_changed(int on)
{
  fNumberBase= (on ? 16 :10);
  //std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;
  RefreshView ();
}


void NyxorGui::Slave_changed (int)
{
//std::cout << "NyxorGui::Slave_changed" << std::endl;
EvaluateSlave ();
bool triggerchangeable = AssertNoBroadcast (false);
//MasterTriggerBox->setEnabled (triggerchangeable);
//InternalTriggerBox->setEnabled (triggerchangeable);
//FesaModeBox->setEnabled (triggerchangeable);
RefreshButton->setEnabled (triggerchangeable);
//if(triggerchangeable) ShowBtn_clicked (); // automatic update of values?


}




void NyxorGui::RefreshView ()
{
// display setup structure to gui:
QString text;
QString pre;
fNumberBase==16? pre="0x" : pre="";

// note that nxyter tabs refresh themselves when updating context


QString statustext;
statustext.append("SFP ");
statustext.append(text.setNum(fChannel));
statustext.append(" DEV ");
statustext.append(text.setNum(fSlave));
statustext.append(" - Last refresh:");
statustext.append(QDateTime::currentDateTime().toString(Qt::TextDate));
StatusLabel->setText(statustext);

}

void NyxorGui::EvaluateView ()
{


}




void NyxorGui::EvaluateSlave ()
{
fChannel = SFPspinBox->value ();
fSlave = SlavespinBox->value ();
}









void NyxorGui::SetRegisters ()
{
// write register values from strucure with gosipcmd
  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
    {
      fNxTab[nx]->setSubConfig();
    }





if (AssertNoBroadcast (false))
{
  // update trigger modes only in single device
//  WriteGosip (fChannel, fSlave, POLAND_REG_INTERNAL_TRIGGER, fSetup.fInternalTrigger);
//  WriteGosip (fChannel, fSlave, POLAND_REG_MASTERMODE, fSetup.fTriggerMode);
}

//WriteGosip (fChannel, fSlave, POLAND_REG_QFW_MODE, fSetup.fQFWMode);
//
//// following is required to really activate qfw mode (thanks Sven Loechner for fixing):
//WriteGosip (fChannel, fSlave, POLAND_REG_QFW_PRG, 1);
//WriteGosip (fChannel, fSlave, POLAND_REG_QFW_PRG, 0);
//
//
//
//// WriteGosip(fChannel, fSlave, POLAND_REG_TRIGCOUNT, fSetup.fEventCounter);
//
//for (int i = 0; i < POLAND_TS_NUM; ++i)
//{
//  WriteGosip (fChannel, fSlave, POLAND_REG_STEPS_BASE + 4 * i, fSetup.fSteps[i]);
//  WriteGosip (fChannel, fSlave, POLAND_REG_TIME_BASE + 4 * i, fSetup.fTimes[i]);
//}
////    for(int e=0; e<POLAND_ERRCOUNT_NUM;++e)
////     {
////       WriteGosip(fChannel, fSlave, POLAND_REG_ERRCOUNT_BASE + 4*e, fSetup.fErrorCounter[e]);
////     }
//
//// TODO: error handling with exceptions?

}

void NyxorGui::GetRegisters ()
{
// read register values into structure with gosipcmd

if (!AssertNoBroadcast ())
  return;

for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    fNxTab[nx]->getSubConfig();
  }


}

uint8_t NyxorGui::ReadNyxorI2c (int nxid, uint8_t address)
{
  uint8_t val=0;

  int dat;
  int nxad=0;
  if(nxid==0) nxad=I2C_ADDR_NX0 +1;
  else if(nxid == 1) nxad=I2C_ADDR_NX1 +1;

  dat  = 0;
  dat += address <<  8;
  dat += nxad << 16 ;
  dat += I2C_COTR_A << 24;


  // set i2c address to read from:
  WriteGosip(fChannel, fSlave, GOS_I2C_DWR, dat);
  // todo need error checking here?
  val=ReadGosip(fChannel, fSlave, GOS_I2C_DRR1);
  return val;
}

int NyxorGui::ReadGosip (int sfp, int slave, int address)
{
int value = -1;

char buffer[1024];
//snprintf(buffer,1024,"/daq/usr/adamczew/workspace/drivers/mbspex/bin/gosipcmd -r -- %d %d 0x%x",sfp, slave, address);
snprintf (buffer, 1024, "gosipcmd -r -- %d %d 0x%x", sfp, slave, address);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
if (result != "ERROR")
{
  QString pre, valtext;
  fNumberBase==16? pre="0x" : pre="";
  //DebugTextWindow (result);
  value = result.toInt (0, 0);
  valtext=pre+valtext.setNum (value, fNumberBase);
  DebugTextWindow (valtext);

}
else
{

  value = -1;
}


return value;
}


int NyxorGui::WriteNyxorI2c (int nxid, uint8_t address, uint8_t value, bool veri)
{
  int dat=0; // data word to send
  int nxad=0;
  if(nxid==0) nxad=I2C_ADDR_NX0;
  else if(nxid == 1) nxad=I2C_ADDR_NX1;

 dat  = value;
 dat += address <<  8;
 dat += nxad << 16 ;
 dat += I2C_COTR_A << 24;

  WriteGosip(fChannel, fSlave, GOS_I2C_DWR, dat);

}


int NyxorGui::WriteGosip (int sfp, int slave, int address, int value)
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

QString NyxorGui::ExecuteGosipCmd (QString& com)
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
  std::cout << " NyxorGui::ExecuteGosipCmd(): gosipcmd not finished after 5 s error" << std::endl;
  AppendTextWindow ("! Warning: ExecuteGosipCmd not finished after 5 s timeout !!!");
  result = "ERROR";
}
QApplication::restoreOverrideCursor();
return result;
}

void NyxorGui::AppendTextWindow (const QString& text)
{
TextOutput->appendPlainText (text);
TextOutput->update ();
}

bool NyxorGui::AssertNoBroadcast (bool verbose)
{
if (fChannel < 0 || fSlave < 0)
{
  //std::cerr << "# NyxorGui Error: broadcast not supported here!" << std::endl;
  if (verbose)
    AppendTextWindow ("#Error: broadcast not supported here!");
  return false;
}
return true;
}

