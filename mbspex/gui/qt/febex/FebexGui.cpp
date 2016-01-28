#include "FebexGui.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <QProcess>
#include <stdlib.h>

#include <QString>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDateTime>

#include <sstream>
#include <string.h>
#include <errno.h>


// *********************************************************


// this we need to implement for output of mbspex library, but also useful to format output without it:
FebexGui* FebexGui::fInstance = 0;


#include <stdarg.h>

void printm (char *fmt, ...)
{
  char c_str[256];
  va_list args;
  va_start(args, fmt);
  vsprintf (c_str, fmt, args);
//printf ("%s", c_str);
  FebexGui::fInstance->AppendTextWindow (c_str);

  va_end(args);
}

#ifdef USE_MBSPEX_LIB
/** this one is used to speed down direct mbspex io:*/
void FebexGui::I2c_sleep ()
{
  //usleep(300);

  usleep(900); // JAM2016 need to increase wait time since some problems with adc read?
}

#endif




/*
 *  Constructs a FebexGui which is a child of 'parent', with the
 *  name 'name'.'
 */
FebexGui::FebexGui (QWidget* parent) :
    QWidget (parent), fSetup(), fDebug (false), fSaveConfig (false), fChannel (0), fSlave (0), fChannelSave (0), fSlaveSave (0),
        fConfigFile (0)
{
  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
#endif

  fNumberBase = 10;

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
  QObject::connect (SaveConfigButton, SIGNAL (clicked ()), this, SLOT (SaveConfigBtn_clicked ()));
  QObject::connect (ClearOutputButton, SIGNAL (clicked ()), this, SLOT (ClearOutputBtn_clicked ()));

  QObject::connect(DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
  QObject::connect(HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));
  QObject::connect(SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
  QObject::connect(SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));

// here optional components for febex:





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
  show ();
}

FebexGui::~FebexGui ()
{
#ifdef USE_MBSPEX_LIB
  mbspex_close (fPexFD);
#endif
}

void FebexGui::ShowBtn_clicked ()
{
  //std::cout << "FebexGui::ShowBtn_clicked()"<< std::endl;
  EvaluateSlave ();
  GetRegisters ();
  RefreshView ();
}

void FebexGui::ApplyBtn_clicked ()
{
//std::cout << "FebexGui::ApplyBtn_clicked()"<< std::endl;

  char buffer[1024];
  EvaluateSlave ();

// JAM maybe disable confirm window ?
//  snprintf (buffer, 1024, "Really apply FEBEX settings  to SFP %d Device %d?", fChannel, fSlave);
//  if (QMessageBox::question (this, "FEBEX GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
//      != QMessageBox::Yes)
//  {
//    return;
//  }

  EvaluateView (); // from gui to memory

  SetRegisters (); // from memory to device

}

void FebexGui::SaveConfigBtn_clicked ()
{
//std::cout << "FebexGui::SaveConfigBtn_clicked()"<< std::endl;

  static char buffer[1024];
  QString gos_filter ("gosipcmd file (*.gos)");
  QStringList filters;
  filters << gos_filter;

  QFileDialog fd (this, "Write Febex configuration file");

  // ".", "nyxor setup file (*.txt);;gosipcmd file (*.gos);;context dump file (*.dmp)");
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
    std::cout << "FebexGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
  }

  // open file
  if (OpenConfigFile (fileName) != 0)
    return;

  if (fileName.endsWith (".gos"))
  {
    WriteConfigFile (QString ("#Format *.gos"));
    WriteConfigFile (QString ("#usage: gosipcmd -x -c file.gos \n"));
    WriteConfigFile (QString ("#                                         \n"));
    WriteConfigFile (QString ("#sfp slave address value\n"));
    fSaveConfig = true;    // switch to file output mode
    SetRegisters ();    // register settings are written to file
    fSaveConfig = false;
  }


  else
  {
    std::cout << "FebexGui::SaveConfigBtn_clicked( -  unknown file type, NEVER COME HERE!!!!)" << std::endl;
  }

  // close file
  CloseConfigFile ();
  snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
  AppendTextWindow (buffer);
}

int FebexGui::OpenConfigFile (const QString& fname)
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
  WriteConfigFile (QString ("# Febex configuration file saved on ") + timestring + QString ("\n"));
  return 0;
}

int FebexGui::CloseConfigFile ()
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

int FebexGui::WriteConfigFile (const QString& text)
{
  if (fConfigFile == NULL)
    return -1;
  if (fprintf (fConfigFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}


void FebexGui::InitChainBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
//std::cout << "InitChainBtn_clicked()"<< std::endl;
  bool ok;
  snprintf (buffer, 1024, "Please specify NUMBER OF DEVICES to initialize at SFP %d ?", fChannel);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  int numslaves = QInputDialog::getInt (this, tr ("Number of Slaves?"), tr (buffer), 1, 1, 1024, 1, &ok);
#else
  int numslaves = QInputDialog::getInteger(this, tr("Number of Slaves?"),
      tr(buffer), 1, 1, 1024, 1, &ok);

#endif
  if (!ok)
    return;
  if (fChannel < 0)
  {
    AppendTextWindow ("--- Error: Broadcast not allowed for init chain!");
    return;
  }
#ifdef USE_MBSPEX_LIB
  int rev = mbspex_slave_init (fPexFD, fChannel, numslaves);

#else
  snprintf (buffer, 1024, "gosipcmd -i  %d %d", fChannel, numslaves);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
#endif
}

void FebexGui::ResetBoardBtn_clicked ()
{
//std::cout << "FebexGui::ResetBoardBtn_clicked"<< std::endl;
  if (QMessageBox::question (this, "Poland GUI", "Really Reset gosip on pex board?", QMessageBox::Yes | QMessageBox::No,
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

}

void FebexGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
  snprintf (buffer, 1024, "Really reset logic on FEBEX device at SFP %d, Slave %d ?", fChannel, fSlave);
  if (QMessageBox::question (this, "Febex GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }

  AppendTextWindow ("--- Resetting logic on NYXOR... ");
  EnableI2C ();
}
void FebexGui::EnableI2C ()
{
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, 0x1000080);
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, 0x2000020);
}

void FebexGui::DisableI2C ()
{
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, 0x1000000);
}


void FebexGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "FebexGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
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

void FebexGui::DumpBtn_clicked ()
{
//std::cout << "FebexGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
  EvaluateSlave ();

  if (!AssertNoBroadcast ())
    return;    // for febex we can not dump all connected frontends at once, NxI2c works on current slave only

  char buffer[1024];
  AppendTextWindow ("--- ADC Dump ---:");

  // JAM 2016 first demonstration how to get the actual adc values:
  printm("SFP %d DEV:%d :)",fChannel, fSlave);
  for(int adc=0; adc<FEBEX_ADC_NUMADC; ++adc){
    for (int chan=0; chan<FEBEX_ADC_NUMCHAN; ++chan){
      int val=ReadADC_Febex(adc,chan);
      if(val<0)
        printm("Read error for adc:%d chan:%d",adc,chan);
      else
        {
          if(fNumberBase==16)
            printm("Val (adc:0x%x chan:0x%x)=0x%x",adc,chan,val);
          else
            printm("Val (adc:%d chan:%d)=%d",adc,chan,val);
      }
    }

  }





}

void FebexGui::ClearOutputBtn_clicked ()
{
//std::cout << "FebexGui::ClearOutputBtn_clicked()"<< std::endl;
  TextOutput->clear ();
  TextOutput->setPlainText ("Welcome to FEBEX GUI!\n\t v0.11 of 28-Januar-2016 by JAM (j.adamczewski@gsi.de)\n");

}

void FebexGui::ConfigBtn_clicked ()
{
//std::cout << "FebexGui::ConfigBtn_clicked" << std::endl;

// here file requester and application of set up via gosipcmd
  QFileDialog fd (this, "Select FEBEX configuration file", ".", "gosipcmd file (*.gos)");
  fd.setFileMode (QFileDialog::ExistingFile);
  if (fd.exec () != QDialog::Accepted)
    return;
  QStringList flst = fd.selectedFiles ();
  if (flst.isEmpty ())
    return;
  QString fileName = flst[0];
  char buffer[1024];
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");
    snprintf (buffer, 1024, "gosipcmd -x -c %s ", fileName.toLatin1 ().constData ());

  }
  QString com (buffer);
  QString result = ExecuteGosipCmd (com, 10000);    // this will just execute the command in shell, gosip or not
  AppendTextWindow (result);

}

void FebexGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
  fDebug = on;
}

void FebexGui::HexBox_changed (int on)
{
  fNumberBase = (on ? 16 : 10);
//std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;
  RefreshView ();

}

void FebexGui::Slave_changed (int)
{
//std::cout << "FebexGui::Slave_changed" << std::endl;
  EvaluateSlave ();
  bool triggerchangeable = AssertNoBroadcast (false);
  RefreshButton->setEnabled (triggerchangeable);

}

void FebexGui::RefreshView ()
{
// display setup structure to gui:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";

// here refresh of number displays:

  // the beginning gui can only display the channel that is currently set:
  int theDAC= dacBox->value ();
  int theChannel= chanBox->value ();
  int val=fSetup.GetDACValue(theDAC,theChannel);
  //std::cout<< "RefreshView with dac:"<<theDAC<<", chan:"<<theChannel<<"val="<<val << std::endl;
  DACvalue_lineEdit->setText (pre+text.setNum (val, fNumberBase));

// JAM 2016 TODO: add more elements to show all channels simultaneously!

  // status header text with refresh time etc.:
  QString statustext;
  statustext.append ("SFP ");
  statustext.append (text.setNum (fChannel));
  statustext.append (" DEV ");
  statustext.append (text.setNum (fSlave));
  statustext.append (" - Last refresh:");
  statustext.append (QDateTime::currentDateTime ().toString (Qt::TextDate));
  StatusLabel->setText (statustext);

}

void FebexGui::EvaluateView ()
{
  // here the current gui display is just copied to setup structure in local memory

  int value=DACvalue_lineEdit->text ().toUInt (0, fNumberBase);
  int theDAC= dacBox->value ();
  int theChannel= chanBox->value ();
  fSetup.SetDACValue(theDAC,theChannel, value);
  fSetup.fDAC=theDAC;
  fSetup.fChannel=theChannel; // remember the last visible indices to apply them.

}

void FebexGui::EvaluateSlave ()
{
  fChannel = SFPspinBox->value ();
  fSlave = SlavespinBox->value ();

  fSetup.fDAC=dacBox->value ();
  fSetup.fChannel=chanBox->value (); // remember the last visible indices to show them.
}

void FebexGui::SetRegisters ()
{
  EnableI2C ();    // must be done since mbs setup program may shut i2c off at the end
// write register values from strucure with gosipcmd

  // beginners gui will only write the currently set index:
  WriteDAC_FebexI2c (fSetup.fDAC, fSetup.fChannel, fSetup.GetDACValue(fSetup.fDAC, fSetup.fChannel));

// JAM2016 TODO loop over complete structure to set alltogether




  DisableI2C ();
}

void FebexGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  EnableI2C ();

  // beginners gui will only read the currently set index:
  int val=ReadDAC_FebexI2c (fSetup.fDAC, fSetup.fChannel);
  if(val<0){
    AppendTextWindow("GetRegisters has error!");
    return; // TODO error message
  }
  fSetup.SetDACValue(fSetup.fDAC, fSetup.fChannel,val);
  // JAM2016 TODO loop over complete structure to get alltogether


  DisableI2C ();

}


int FebexGui::ReadDAC_FebexI2c (uint8_t mcpchip, uint8_t chan)
{
  int val = 0;
  if(mcpchip>4){
     AppendTextWindow ("#Error: ReadDAC_FebexI2c with illegal chip number!");
     return -1;
   }
  int channeloffset=GetChannelOffsetDAC(chan);
  if(channeloffset<0) return -2;

  int dat=FEBEX_MCP433_BASE_READ + mcpchip* FEBEX_MCP433_OFFSET + channeloffset;
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, dat); // first send read request address
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, FEBEX_MCP433_REQUEST_READ); // read request command

  val = ReadGosip (fChannel, fSlave, GOS_I2C_DRR1); // read out the value
  if(val < 0) return val; // error case, propagate it upwards
  return (val & 0xFF); // mask to use only l.s. byte
}



int  FebexGui::ReadADC_Febex (uint8_t adc, uint8_t chan)
{
  if(adc>FEBEX_ADC_NUMADC || chan > FEBEX_ADC_NUMCHAN) return -1;

  int val=0;
  int dat=(adc << 2) + chan; //l_wr_d  = (l_k*4) + l_l;

  WriteGosip (fChannel, fSlave, FEBEX_ADC_PORT, dat); // first specify channel number

  val = ReadGosip (fChannel, fSlave, FEBEX_ADC_PORT); // read back the value

  // check if channel id matches the requested ones:
  if ( ((val >> 24) & 0xf) != dat)
      {
         printm ("#Error: ReadADC_Febex channel id mismatch, requested 0x%x, received 0x%x",dat, (val>>24));
         return -1;
      }


  return (val & 0x3fff);


}




int FebexGui::ReadGosip (int sfp, int slave, int address)
{
  int value = -1;
#ifdef USE_MBSPEX_LIB
  int rev = 0;
  long int dat = 0;
  QApplication::setOverrideCursor (Qt::WaitCursor);
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
  QApplication::restoreOverrideCursor ();
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


int FebexGui::GetChannelOffsetDAC(uint8_t chan)
{
  int channeloffset;
  switch(chan)
   {
     case 0:
       channeloffset=0x1000; // JAM note that order of first channels is swapped (verified with go4 readout)
       break;
     case 1:
       channeloffset=0x0000;
     break;
     case 2:
       channeloffset=0x6000;
       break;
     case 3:
       channeloffset=0x7000;
     break;
     default:
       AppendTextWindow ("#Error: GetChannelOffsetDAC with illegal channel number!");
       return -2;
   };
  return channeloffset;

}


int FebexGui::WriteDAC_FebexI2c (uint8_t mcpchip, uint8_t chan, uint8_t value)
{
  // value byte: 80: middle, max: ff, min:0
  //(0:  shift to max adc value)
  if(mcpchip>4){
    AppendTextWindow ("#Error: WriteDAC_FebexI2c with illegal chip number!");
    return -1;
  }
  int channeloffset=GetChannelOffsetDAC(chan);
  if(channeloffset<0) return -2;

  int dat=FEBEX_MCP433_BASE_WRITE + mcpchip* FEBEX_MCP433_OFFSET + channeloffset;
  dat+=(value & 0xFF);

  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, dat);
  return 0;
}

int FebexGui::SaveGosip (int sfp, int slave, int address, int value)
{
//std::cout << "# SaveGosip" << std::endl;
  static char buffer[1024] = { };
  snprintf (buffer, 1024, "%d %d %x %x \n", sfp, slave, address, value);
  QString line (buffer);
  return (WriteConfigFile (line));
}

int FebexGui::WriteGosip (int sfp, int slave, int address, int value)
{
  int rev = 0;
//std::cout << "#WriteGosip" << std::endl;
  if (fSaveConfig)
    return SaveGosip (sfp, slave, address, value);

#ifdef USE_MBSPEX_LIB
  QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = mbspex_slave_wr (fPexFD, sfp, slave, address, value);
  I2c_sleep ();
  if (fDebug)
  {
    char buffer[1024];
    snprintf (buffer, 1024, "mbspex_slave_wr(%d,%d 0x%x 0x%x)", sfp, slave, address, value);
    QString msg (buffer);
    AppendTextWindow (msg);
  }
  QApplication::restoreOverrideCursor ();
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

QString FebexGui::ExecuteGosipCmd (QString& com, int timeout)
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
  QApplication::setOverrideCursor (Qt::WaitCursor);

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
    std::cout << " FebexGui: " << buf.str ().c_str ();
    AppendTextWindow (buf.str ().c_str ());
    result = "ERROR";
  }
  QApplication::restoreOverrideCursor ();
  return result;
}

void FebexGui::AppendTextWindow (const QString& text)
{
  TextOutput->appendPlainText (text);
  TextOutput->update ();
}

bool FebexGui::AssertNoBroadcast (bool verbose)
{
  if (fChannel < 0 || fSlave < 0)
  {
    //std::cerr << "# FebexGui Error: broadcast not supported here!" << std::endl;
    if (verbose)
      AppendTextWindow ("#Error: broadcast not supported here!");
    return false;
  }
  return true;
}


