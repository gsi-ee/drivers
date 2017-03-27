#include "GosipGui.h"

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


// this we need to implement for output of mbspex library, but also useful to format output without it:
GosipGui* GosipGui::fInstance = 0;

#include <stdarg.h>





void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
//printf ("%s", c_str);
  GosipGui::fInstance->AppendTextWindow (c_str);
  GosipGui::fInstance->FlushTextWindow();
  va_end(args);
}

#ifdef USE_MBSPEX_LIB
/** this one is used to speed down direct mbspex io:*/
void GosipGui::I2c_sleep ()
{
  //usleep(300);

  usleep(900); // JAM2016 need to increase wait time since some problems with adc read?
}

#endif




// *********************************************************

/*
 *  Constructs a GosipGui which is a child of 'parent', with the
 *  name 'name'.'
 */
GosipGui::GosipGui (QWidget* parent) :
    QWidget (parent), fDebug (false), fSaveConfig(false), fBroadcasting(false),fSFP (0), fSlave (0), fSFPSave (0), fSlaveSave (0), fConfigFile(NULL)
{
  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
#endif


  fImplementationName="GOSIP";
  fVersionString="Welcome to GOSIP GUI!\n\t v0.82 of 23-March-2017 by JAM (j.adamczewski@gsi.de)";


  fNumberBase=10;

 	memset( &fSFPChains, 0, sizeof(struct pex_sfp_links));

  for(int sfp=0; sfp<4;++sfp)
    {
      fSetup[sfp].clear();
      // note: in the beginning, setup pointers are empty, no need to delete objects on heap
    }

  SFPspinBox->setValue (fSFP);
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
  QObject::connect (SaveConfigButton, SIGNAL (clicked ()), this, SLOT (SaveConfigBtn_clicked ()));
  QObject::connect (ClearOutputButton, SIGNAL (clicked ()), this, SLOT (ClearOutputBtn_clicked ()));






QObject::connect(DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
QObject::connect(HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));
QObject::connect(SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
QObject::connect(SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));

QObject::connect(ShowLogBox, SIGNAL(toggled(bool)), TextOutput, SLOT(setVisible(bool)));


// JAM2017: some more signals for the autoapply feature:



#ifdef USE_MBSPEX_LIB
// open handle to driver file:
  fPexFD = mbspex_open (0);    // we restrict to board number 0 here
  if (fPexFD < 0)
  {
    printm ("ERROR>> open /dev/pexor%d \n", 0);
    exit (1);
  }
#endif
  fInstance = this;

  // NOTE: this MUST be done in subclass constructor, otherwise factory method for setup structure will fail!
  //    (not tested, but to be expected so...)
  //GetSFPChainSetup(); // ensure that any slave has a status structure before we begin clicking...

   //show();
}

GosipGui::~GosipGui ()
{
#ifdef USE_MBSPEX_LIB
  mbspex_close (fPexFD);
#endif
}

void GosipGui::ShowBtn_clicked ()
{
//std::cout << "GosipGui::ShowBtn_clicked()"<< std::endl;
  EvaluateSlave ();
  GetSFPChainSetup();

  if(!AssertNoBroadcast(false)) return;
  if(!AssertChainConfigured()) return;
  GetRegisters ();
  RefreshView ();
}

void GosipGui::ApplyBtn_clicked ()
{
//std::cout << "GosipGui::ApplyBtn_clicked()"<< std::endl;
  EvaluateSlave ();
  if (!checkBox_AA->isChecked ())
  {
    QString message = QString("Really apply GUI Settings  to SFP %1 Device %2?").arg(fSFP).arg(fSlave);
    if (QMessageBox::question (this, fImplementationName, message, QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) != QMessageBox::Yes)
    {
      return;
    }
  }
  GetSFPChainSetup();
  // JAM: since we keep all slave set ups in vector/array, we must handle broadcast mode explicitely
  // no implicit driver broadcast via -1 indices anymore!
  GOSIP_BROADCAST_ACTION(ApplyGUISettings());
}












void GosipGui::InitChainBtn_clicked ()
{
char buffer[1024];
EvaluateSlave ();
//std::cout << "InitChainBtn_clicked()"<< std::endl;
bool ok;
snprintf (buffer, 1024, "Please specify NUMBER OF DEVICES to initialize at SFP %d ?", fSFP);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
int numslaves = QInputDialog::getInt(this, tr("Number of Slaves?"),
                                 tr(buffer), 1, 1, 1024, 1, &ok);
#else
int numslaves = QInputDialog::getInteger(this, tr("Number of Slaves?"),
                                 tr(buffer), 1, 1, 1024, 1, &ok);

#endif
if (!ok) return;
if(fSFP<0)
{
    AppendTextWindow ("--- Error: Broadcast not allowed for init chain!");
    return;
}

#ifdef USE_MBSPEX_LIB
  int rev = mbspex_slave_init (fPexFD, fSFP, numslaves);

#else


snprintf (buffer, 1024, "gosipcmd -i  %d %d", fSFP, numslaves);
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);
#endif

 GetSFPChainSetup();
 RefreshChains();

}

void GosipGui::ResetBoardBtn_clicked ()
{
//std::cout << "GosipGui::ResetBoardBtn_clicked"<< std::endl;
if (QMessageBox::question (this, fImplementationName, "Really Reset gosip on pex board?", QMessageBox::Yes | QMessageBox::No,
    QMessageBox::Yes) != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}


#ifdef USE_MBSPEX_LIB
  mbspex_reset (fPexFD);
  AppendTextWindow ("Reset PEX board with mbspex_reset()");

#else

char buffer[1024];
snprintf (buffer, 1024, "gosipcmd -z");
QString com (buffer);
QString result = ExecuteGosipCmd (com);
AppendTextWindow (result);

#endif

  GetSFPChainSetup();
  RefreshChains();
}

void GosipGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
  snprintf (buffer, 1024, "Really reset %s device at SFP %d, Slave %d ?", fImplementationName.toLatin1 ().constData () ,fSFP, fSlave);
  if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }
  GOSIP_BROADCAST_ACTION(ResetSlave());
}

void GosipGui::ResetSlave ()
{

 printm("ResetSlave() for sfp:%d slave:%d - not yet implemented, please design a subclass of GosipGui!\n",fSFP,fSlave);
}

void GosipGui::EvaluateView()
{
 printm("EvaluateView() for sfp:%d slave:%d - not yet implemented, please design a subclass of GosipGui!\n",fSFP,fSlave);
}

void GosipGui::SetRegisters ()
{

  printm("SetRegisters() for sfp:%d slave:%d - not yet implemented, please design a subclass of GosipGui!\n",fSFP,fSlave);
}

void GosipGui::GetRegisters ()
{
  printm("GetRegisters() for sfp:%d slave:%d - not yet implemented, please design a subclass of GosipGui!\n",fSFP,fSlave);
}



void GosipGui::ApplyGUISettings()
{
    // default behaviour, may be overwritten
    EvaluateView(); // from gui to memory
    SetRegisters(); // from memory to device
}








void GosipGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "GosipGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
  if (checked)
  {
    fSFPSave = SFPspinBox->value ();
    fSlaveSave = SlavespinBox->value ();
    SFPspinBox->setValue (-1);
    SlavespinBox->setValue (-1);
  }
  else
  {
    SFPspinBox->setValue (fSFPSave);
    SlavespinBox->setValue (fSlaveSave);

  }
}

void GosipGui::DumpBtn_clicked ()
{
//std::cout << "GosipGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
EvaluateSlave ();
AppendTextWindow ("--- Register Dump ---:");
GOSIP_BROADCAST_ACTION(DumpSlave());
}


void GosipGui::DumpSlave ()
{
  // most generic form, just uses dump function of setup structure
  // for detailed data fifo dump etc. please overwrite this method!
  GetRegisters();
  printm("Dump setup of sfp:%d device %d",fSFP,fSlave);
  theSetup_GET_FOR_SLAVE(GosipSetup);
  theSetup->Dump();

}


void GosipGui::ClearOutputBtn_clicked ()
{
//std::cout << "GosipGui::ClearOutputBtn_clicked()"<< std::endl;
TextOutput->clear ();
TextOutput->setPlainText (fVersionString);

}

void GosipGui::ConfigBtn_clicked ()
{
//std::cout << "GosipGui::ConfigBtn_clicked" << std::endl;
  ApplyFileConfig();
}


void GosipGui::ApplyFileConfig(int gosipwait)
{
  // most generic form
  // TODO: probably put here argument to set gosip waiting time before executing the script
  QFileDialog fd (this, "Select GOSIP configuration file", ".", "gosipcmd file (*.gos)");
  fd.setFileMode (QFileDialog::ExistingFile);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  if (gosipwait > 0)
  {
    QString tcom=QString("setGosipwait.sh %1").arg(gosipwait);
    QString tresult = ExecuteGosipCmd (tcom, 10000);
    AppendTextWindow (tresult);
  }

  QString fileName = flst[0];
  if (!fileName.endsWith (".gos"))
    fileName.append (".gos");
  char buffer[1024];
  snprintf (buffer, 1024, "gosipcmd -x -c %s ", fileName.toLatin1 ().constData ());
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
  if(gosipwait>0)
   {
    QString zcom="setGosipwait.sh 0";
    QString zresult=ExecuteGosipCmd (zcom, 10000);
    AppendTextWindow (zresult);
   }

}



void GosipGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
fDebug = on;
}

void GosipGui::HexBox_changed(int on)
{
  fNumberBase= (on ? 16 :10);
  //std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;
  RefreshView ();
}


void GosipGui::Slave_changed (int)
{
//std::cout << "GosipGui::Slave_changed" << std::endl;
EvaluateSlave ();
bool refreshable = AssertNoBroadcast (false);
RefreshButton->setEnabled (refreshable);
RefreshChains();
 //if(checkBox_AA->isChecked() && refreshable)
 if(refreshable)
 {
   // JAM note that we had a problem of prelling spinbox here (arrow buttons only, keyboard arrows are ok)
   // probably caused by too long response time of this slot?
   // workaround is to refresh the view delayed per single shot timer:
   //std::cout << "Timer started" << std::endl;
   QTimer::singleShot(10, this, SLOT(ShowBtn_clicked()));
   //std::cout << "Timer end" << std::endl;
 }

}




void GosipGui::RefreshChains ()
{

#ifdef USE_MBSPEX_LIB
  // show status of configured chains:
  Chain0_Box->setValue (fSFPChains.numslaves[0]);
  Chain1_Box->setValue (fSFPChains.numslaves[1]);
  Chain2_Box->setValue (fSFPChains.numslaves[2]);
  Chain3_Box->setValue (fSFPChains.numslaves[3]);

  // set maximum value of device spinbox according to init chains:

  if (fSFP >= 0) // only for non broadcast mode of slaves
  {
    if (fSFPChains.numslaves[fSFP] > 0) // configured chains
    {
      SlavespinBox->setMaximum (fSFPChains.numslaves[fSFP] - 1);
      SlavespinBox->setEnabled (true);
    }
    else // non configured chains
    {
      SlavespinBox->setEnabled (false);
    }
  }
#else
    Chain0_Box->setEnabled (false);
    Chain1_Box->setEnabled (false);
    Chain2_Box->setEnabled (false);
    Chain3_Box->setEnabled (false);

#endif

}


void GosipGui::RefreshStatus ()
{
  QString text;
  QString statustext;
   statustext.append ("SFP ");
   statustext.append (text.setNum (fSFP));
   statustext.append (" DEV ");
   statustext.append (text.setNum (fSlave));
   statustext.append (" - Last refresh:");
   statustext.append (QDateTime::currentDateTime ().toString (Qt::TextDate));
   StatusLabel->setText (statustext);
}



void GosipGui::RefreshView ()
{
// display setup structure to gui:

// for base class, we only update status messages etc.
RefreshChains();
RefreshStatus();
}



void GosipGui::EvaluateSlave ()
{
  if(fBroadcasting) return;
  fSFP = SFPspinBox->value ();
  fSlave = SlavespinBox->value ();
}



void GosipGui::SaveConfigBtn_clicked ()
{
  //std::cout << "GosipGui::SaveConfigBtn_clicked()"<< std::endl;
  SaveConfig();
}




void GosipGui::SaveConfig()
{
  // default: write a gosipcmd script file
  // this one may be reimplemented in subclass if other file formats shall be supported.
  static char buffer[1024];
  QString gos_filter ("gosipcmd file (*.gos)");
  //QString dmp_filter ("data dump file (*.dmp)");
  QStringList filters;
  filters << gos_filter;// << dmp_filter;

  QFileDialog fd (this, "Write GOSIP configuration file");

  fd.setNameFilters (filters);
  fd.setFileMode (QFileDialog::AnyFile);
  fd.setAcceptMode (QFileDialog::AcceptSave);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  QString fileName = flst[0];

  // complete suffix if user did not
   if (fd.selectedNameFilter () == gos_filter)
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");
  }
  else
  {
    std::cout << "GosipGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
  }

  // open file
  if (OpenConfigFile (fileName) != 0)
    return;

  if (fileName.endsWith (".gos"))
  {
    WriteConfigFile (QString ("# Format *.gos\n"));
    WriteConfigFile (QString ("# usage: gosipcmd -x -c file.gos \n"));
    WriteConfigFile (QString ("#                                         \n"));
    WriteConfigFile (QString ("# sfp slave address value\n"));


    GOSIP_BROADCAST_ACTION(SaveRegisters()); // refresh actual setup from hardware and write it to open file
  }
  else
  {
    std::cout << "GosipGui::SaveConfigBtn_clicked( -  unknown file type, NEVER COME HERE!!!!)" << std::endl;
  }

  // close file
  CloseConfigFile ();
  snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
  AppendTextWindow (buffer);
}





int GosipGui::OpenConfigFile (const QString& fname)
{
  fConfigFile = fopen (fname.toLatin1 ().constData (), "w");
  if (fConfigFile == NULL)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error opening Configuration File '%s': %s\n", fname.toLatin1 ().constData (),
        strerror (errno));
    AppendTextWindow (buffer);
    return -1;
  }
  QString timestring = QDateTime::currentDateTime ().toString ("ddd dd.MM.yyyy hh:mm:ss");
  WriteConfigFile (QString ("# Gosip configuration file saved on ") + timestring + QString ("\n"));
  return 0;
}

int GosipGui::CloseConfigFile ()
{
  int rev = 0;
  if (fConfigFile == NULL)
    return 0;
  if (fclose (fConfigFile) != 0)
  {
    char buffer[1024];
    snprintf (buffer, 1024, " Error closing Configuration File! (%s)\n", strerror (errno));
    AppendTextWindow (buffer);
    rev = -1;
  }
  fConfigFile = NULL;    // must not use handle again even if close fails
  return rev;
}

int GosipGui::WriteConfigFile (const QString& text)
{
  if (fConfigFile == NULL)
    return -1;
  if (fprintf (fConfigFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}




void GosipGui::SaveRegisters ()
{
  // default implementation. might be overwritten by frontend subclass -
  // this may be called in explicit broadcast mode, so it is independent of the view on gui
  GetRegisters(); // refresh actual setup from hardware
  fSaveConfig = true;    // switch to file output mode
  SetRegisters();    // register settings are written to file
  fSaveConfig = false;
}




int GosipGui::ReadGosip (int sfp, int slave, int address)
{
int value = -1;
#ifdef USE_MBSPEX_LIB
  int rev = 0;
  long int dat = 0;
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = mbspex_slave_rd (fPexFD, sfp, slave, address, &dat);
  I2c_sleep ();
  value = dat;
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
  //QApplication::restoreOverrideCursor ();
#else
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
#endif

return value;
}

int GosipGui::WriteGosip (int sfp, int slave, int address, int value)
{
  int rev = 0;
  if (fSaveConfig)
      return SaveGosip (sfp, slave, address, value);

#ifdef USE_MBSPEX_LIB
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = mbspex_slave_wr (fPexFD, sfp, slave, address, value);
  I2c_sleep ();
  if (fDebug)
  {
    char buffer[1024];
    snprintf (buffer, 1024, "mbspex_slave_wr(%d,%d 0x%x 0x%x)", sfp, slave, address, value);
    QString msg (buffer);
    AppendTextWindow (msg);
  }
  //QApplication::restoreOverrideCursor ();
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

int GosipGui::SaveGosip (int sfp, int slave, int address, int value)
{
//std::cout << "# SaveGosip" << std::endl;
  static char buffer[1024] = { };
  snprintf (buffer, 1024, "%d %d %x %x \n", sfp, slave, address, value);
  QString line (buffer);
  return (WriteConfigFile (line));
}



QString GosipGui::ExecuteGosipCmd (QString& com, int timeout)
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
    buf << "! Warning: ExecuteGosipCmd not finished after " << timeout / 1000 << " s timeout !!!" << std::endl;
    std::cout << " GosipGui: " << buf.str ().c_str ();
    AppendTextWindow (buf.str ().c_str ());
    result = "ERROR";
}
QApplication::restoreOverrideCursor();
return result;
}

void GosipGui::AppendTextWindow (const QString& text)
{
TextOutput->appendPlainText (text);
TextOutput->update ();
}



void GosipGui::FlushTextWindow ()
{
  TextOutput->repaint ();
}



bool GosipGui::AssertNoBroadcast (bool verbose)
{
if (fSFP < 0 || fSlave < 0)
{
  //std::cerr << "# GosipGui Error: broadcast not supported here!" << std::endl;
  if (verbose)
    AppendTextWindow ("#Error: broadcast not supported here!");
  return false;
}
return true;
}

bool GosipGui::AssertChainConfigured (bool verbose)
{
#ifdef USE_MBSPEX_LIB

  if (fSlave >= fSFPChains.numslaves[fSFP])
  {
    if (verbose)
      printm("#Error: device index %d not in initialized chain of length %d at SFP %d",fSlave,fSFPChains.numslaves[fSFP],fSFP);
    return false;
  }
#endif
return true;
}


void GosipGui::GetSFPChainSetup()
{
//std::cout<<"GetSFPChainSetup... "<< std::endl;
#ifdef USE_MBSPEX_LIB
    // broadcast mode: find out number of slaves and loop over all registered slaves
    mbspex_get_configured_slaves(fPexFD, &fSFPChains);
#else
    // without mbspex lib, we just assume 4 devices for each chain:
    for(int sfp=0; sfp<4; ++sfp)
    {
      fSFPChains.numslaves[sfp]=4;
    }
#endif


    // dynamically increase array of setup structures:
    for(int sfp=0; sfp<4; ++sfp)
    {
      while(fSetup[sfp].size()<fSFPChains.numslaves[sfp])
      {
        fSetup[sfp].push_back(CreateSetup());
        //std::cout<<"GetSFPChainSetup increased setup at sfp "<<sfp<<" to "<<fSetup[sfp].size()<<" slaves." << std::endl;
      }
      // TODO note that we never drop/delete any structure that has been created.
      // the gui must be restarted when chain setup is reconfigured
    }


}



