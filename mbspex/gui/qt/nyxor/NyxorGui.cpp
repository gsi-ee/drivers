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

NyxorGui* NyxorGui::fInstance=0;

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

#ifdef USE_MBSPEX_LIB
// open handle to driver file:
fPexFD=mbspex_open (0); // we restrict to board number 0 here
  if (fPexFD< 0)
  {
    printm ("ERROR>> open /dev/pexor%d \n", 0);
    exit (1);
  }

  fInstance=this;
#endif



show();
}

NyxorGui::~NyxorGui ()
{
#ifdef USE_MBSPEX_LIB
  mbspex_close (fPexFD);
#endif
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
#ifdef USE_MBSPEX_LIB
int rev=mbspex_slave_init (fPexFD, fChannel, numslaves);

#else
snprintf (buffer, 1024, "gosipcmd -i  %d %d", fChannel, numslaves);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);
#endif
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
#ifdef USE_MBSPEX_LIB
mbspex_reset(fPexFD);
AppendTextWindow ("Reset PEX board with mbspex_reset()");

#else

char buffer[1024];
snprintf (buffer, 1024, "gosipcmd -z");
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);

#endif

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
TextOutput->setPlainText ("Welcome to NYXOR GUI!\n\t v0.5 of 20-July-2015 by JAM (j.adamczewski@gsi.de)\n\tContains parts of ROC/nxyter GUI by  Sergey Linev, GSI");

}

void NyxorGui::ConfigBtn_clicked ()
{
//std::cout << "NyxorGui::ConfigBtn_clicked" << std::endl;

// here file requester and application of set up via gosipcmd
QFileDialog fd (this, "Select NYXOR configuration file", ".", "nyxor setup file (*.txt);;gosipcmd file (*.gos)");
fd.setFileMode (QFileDialog::ExistingFile);
if (fd.exec () != QDialog::Accepted)
  return;
QStringList flst = fd.selectedFiles ();
if (flst.isEmpty ())
  return;
QString fileName = flst[0];
char buffer[1024];
if(fileName.endsWith(".txt"))
{
    QString path=fileName;
    path.truncate(path.lastIndexOf("/"));
    // for the moment, the setup exectuable does not take any setup file as argument but maybe later..
  snprintf (buffer, 1024, "bash -c \"cd %s; ./m_set_nxy %s \" ",path.toLatin1 ().constData (), fileName.toLatin1 ().constData ());
}
else
{
  if (!fileName.endsWith (".gos"))
    fileName.append (".gos");
    snprintf (buffer, 1024, "gosipcmd -x -c %s ", fileName.toLatin1 ().constData ());

}
QString com (buffer);
QString result = ExecuteGosipCmd (com, 10000); // this will just execute the command in shell, gosip or not
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
#ifdef USE_MBSPEX_LIB
int rev=0;
long int dat=0;
QApplication::setOverrideCursor( Qt::WaitCursor );
rev = mbspex_slave_rd (fPexFD, sfp, slave, address, &dat);
I2c_sleep ();
value=dat;
  if (fDebug)
  {
    char buffer[1024];
    if (rev == 0)
    {
      snprintf (buffer, 1024, "mbspex_slave_rd(%d,%d 0x%x) -> 0x%x", sfp, slave, address, value);
    }
    else
    {
      snprintf (buffer, 1024, "ERROR %d from mbspex_slave_rd(%d,%d 0x%x)", rev, sfp, slave, address);
    }
    QString msg (buffer);
    AppendTextWindow (msg);

  }
QApplication::restoreOverrideCursor();
#else
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
#endif

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

#ifdef USE_MBSPEX_LIB
QApplication::setOverrideCursor( Qt::WaitCursor );
rev = mbspex_slave_wr (fPexFD, sfp, slave, address, value);
I2c_sleep ();
if (fDebug)
  {
      char buffer[1024];
      snprintf (buffer, 1024, "mbspex_slave_wr(%d,%d 0x%x 0x%x)", sfp, slave, address, value);
      QString msg (buffer);
      AppendTextWindow (msg);
  }
QApplication::restoreOverrideCursor();
#else
char buffer[1024];
snprintf (buffer, 1024, "gosipcmd -w -- %d %d 0x%x 0x%x", sfp, slave, address, value);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
if (result == "ERROR")
  rev = -1;
#endif

return rev;
}

QString NyxorGui::ExecuteGosipCmd (QString& com, int timeout)
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
if (proc.waitForFinished (timeout))    // after process is finished we can still read stdio buffer
{
  // read back stdout of proc here
  result = proc.readAll ();
}
else
{
  std::stringstream buf;
  buf<<"! Warning: ExecuteGosipCmd not finished after "<< timeout/1000<<" s timeout !!!"<< std::endl;
  std::cout << " NyxorGui: "<<buf.str().c_str();
  AppendTextWindow (buf.str().c_str());
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





// this we need to implement for output of mbspex library:
#ifdef USE_MBSPEX_LIB
#include <stdarg.h>

void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
  //printf ("%s", c_str);
  NyxorGui::fInstance->AppendTextWindow(c_str);

  va_end(args);
}

/** this one from Nik to speed down direct mbspex io*/
void NyxorGui::I2c_sleep ()
{
  #define N_LOOP 300000

  int l_ii;
  int volatile l_depp=0;

  for (l_ii=0; l_ii<N_LOOP; l_ii++)
  {
    l_depp++;
  }
}



#endif

