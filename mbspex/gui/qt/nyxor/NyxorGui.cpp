#include "NyxorGui.h"

#include <stdlib.h>
#include <unistd.h>

//#include <stdio.h>
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

#include "nxyterwidget.h"

// *********************************************************

NyxorGui* NyxorGui::fInstance = 0;

/*
 *  Constructs a NyxorGui which is a child of 'parent', with the
 *  name 'name'.'
 */
NyxorGui::NyxorGui (QWidget* parent) :
    QWidget (parent), fDebug (false), fSaveConfig (false), fChannel (0), fSlave (0), fChannelSave (0), fSlaveSave (0),
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

// here components for nxyter tabs:

  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    NxyterWidget* nxw = new NxyterWidget (Nxyter_tabWidget, this, nx);
    Nxyter_tabWidget->addTab (nxw, QString ("NX%1").arg (nx));
    fNxTab[nx] = nxw;
  }

#ifdef USE_MBSPEX_LIB
// open handle to driver file:
  fPexFD = mbspex_open (0);    // we restrict to board number 0 here
  if (fPexFD < 0)
  {
    printm ("ERROR>> open /dev/pexor%d \n", 0);
    exit (1);
  }

  fInstance = this;
#endif

  show ();
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

void NyxorGui::SaveConfigBtn_clicked ()
{
//std::cout << "NyxorGui::SaveConfigBtn_clicked()"<< std::endl;

  static char buffer[1024];
  QString txt_filter ("nyxor setup file (*.txt)");
  QString gos_filter ("gosipcmd file (*.gos)");
  QString dmp_filter ("context dump file (*.dmp)");
  QStringList filters;
  filters << txt_filter << gos_filter << dmp_filter;

  QFileDialog fd (this, "Write NYXOR configuration file");

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
  if (fd.selectedNameFilter () == txt_filter)
  {
    if (!fileName.endsWith (".txt"))
      fileName.append (".txt");
  }
  else if (fd.selectedNameFilter () == gos_filter)
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");
  }
  else if (fd.selectedNameFilter () == dmp_filter)
  {
    if (!fileName.endsWith (".dmp"))
      fileName.append (".dmp");
  }
  else
  {
    std::cout << "NyxorGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
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

  else if (fileName.endsWith (".dmp"))
  {
    // dump configuration
    WriteConfigFile (QString ("#Format *.dmp - nxyter context dump output\n"));
    WriteConfigFile (QString ("#                                         \n"));

    for (int nx = 0; nx < NYXOR_NUMNX; nx++)
    {
      QString header = QString ("#sfp %1 slave %2 nxyter %3 \n").arg (fChannel).arg (fSlave).arg (nx);
      WriteConfigFile (header);
      const nxyter::NxContext* theContext = fNxTab[nx]->getContext ();
      std::stringstream buf;
      theContext->print (buf);
      WriteConfigFile (QString (buf.str ().c_str ()));
      WriteConfigFile (QString ("#                                         \n"));
    }
  }
  else if (fileName.endsWith (".txt"))
  {
    // here function to export context values into Niks format
    WriteNiksConfig ();
  }
  else
  {
    std::cout << "NyxorGui::SaveConfigBtn_clicked( -  unknown file type, NEVER COME HERE!!!!)" << std::endl;
  }

  // close file
  CloseConfigFile ();
  snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
  AppendTextWindow (buffer);
}

int NyxorGui::OpenConfigFile (const QString& fname)
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
  WriteConfigFile (QString ("# Nyxor configuration file saved on ") + timestring + QString ("\n"));
  return 0;
}

int NyxorGui::CloseConfigFile ()
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

int NyxorGui::WriteConfigFile (const QString& text)
{
  if (fConfigFile == NULL)
    return -1;
  if (fprintf (fConfigFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}

int NyxorGui::WriteNiksConfig ()
{
  WriteConfigFile (QString ("# -------------------------------------------------------------\n"));
  WriteConfigFile (QString ("# specify nr of nXYters in use (as function of GEMEX/NYXOR id).\n"));
  WriteConfigFile (QString ("# must be filled consecutively beginning with GEMEX/NYXOR index 0.\n"));
  WriteConfigFile (
      QString ("# if 0 specified for GEMEX/NYXOR index n, the last GEMEX/NYXOR used for a given SFP id is n-1.\n"));
  WriteConfigFile (QString ("# if for GEMEX/NYXOR index 0 the number nXYters is 0, this SFP is not used.\n"));
  WriteConfigFile (QString ("# max 2 nXYter per GEMEX/NYXOR !\n"));
  WriteConfigFile (QString ("#\n"));
  WriteConfigFile (QString ("#  GEMEX/NYXOR id:  0  1  2  3  4  5  6  7  8  9 10 10 12 13 14 15\n"));
  WriteConfigFile (QString ("#                   |  |  |  |  |  |  |  |  |  |  |  |  |  |  |  |\n"));
  for (int ch = 0; ch < 4; ++ch)
  {
    QString sfpline = QString ("SFP%1_NYX_IN_USE     ").arg (ch);
    if (fChannel == ch || (fChannel < 0 && ch == 0))
    {
      for (int i = 0; i < 16; ++i)
      {
        if (fSlave == i || (fSlave<0 && i==0)) // assign broadcast mode to channel 0 for the moment
          sfpline.append ("2  ");
        else
          sfpline.append ("0  ");
      }
    }
    else
    {
      sfpline.append ("0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0");
    }
    sfpline.append ("\n");
    WriteConfigFile (sfpline);
  }

  WriteConfigFile (QString ("#\n"));
  WriteConfigFile (QString ("#\n"));
  WriteConfigFile (QString ("#                             pre trg win   post trg win  test pulse delay  test trg delay\n"));
  WriteConfigFile (QString ("GLOBAL_PARAM                  0x080           0x100         0x0b              0x0a\n"));
  // TODO: set these parameters from GUI?


  // loop over nxyters for current sfp and slave:
  int iadd=0x12; // i2caddress for each nxyter on board
  for (int nx = 0; nx < NYXOR_NUMNX; nx++, iadd+=0x10)
    {
      WriteConfigFile (QString ("#\n"));
      WriteConfigFile (QString ("#\n"));
      WriteConfigFile (QString ("#\n"));

      QString line;
      const nxyter::NxContext* theContext = fNxTab[nx]->getContext ();
      line= QString("SFP %1 NYX %2 nXY %3 I2C_ADDR    \t\t0x%4 # wr\n").arg(fChannel).arg(fSlave).arg(nx).arg(iadd,0,16);
      WriteConfigFile(line);
      line= QString("SFP %1 NYX %2 nXY %3 RESET    \t\t0x8c \n").arg(fChannel).arg(fSlave).arg(nx);
      WriteConfigFile(line);
      line= QString("SFP %1 NYX %2 nXY %3 MASK   \t\t").arg(fChannel).arg(fSlave).arg(nx);
      for(int m=0; m<16;++m)
      {
        uint8_t mval=theContext->getRegister(m); // mask registers are at the start of array
        line.append(QString("0x%1 ").arg(mval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 BIAS   \t\t").arg(fChannel).arg(fSlave).arg(nx);
      for(int b=0; b<14;++b)
          {
            uint8_t bval=theContext->getRegister(b+16);
            line.append(QString("0x%1 ").arg(bval,0,16));
          }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 CONFIG   \t\t").arg(fChannel).arg(fSlave).arg(nx);
      for(int c=0; c<2;++c)
      {
        uint8_t cval=theContext->getRegister(c+32);
        line.append(QString("0x%1 ").arg(cval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 TEST_DELAY   \t\t").arg(fChannel).arg(fSlave).arg(nx);
      for(int d=0; d<2;++d)
      {
        uint8_t dval=theContext->getRegister(d+38);
        line.append(QString("0x%1 ").arg(dval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 CLOCK_DELAY   \t").arg(fChannel).arg(fSlave).arg(nx);
      for(int cl=0; cl<2;++cl)
      {
        uint8_t clval=theContext->getRegister(cl+43);
        line.append(QString("0x%1 ").arg(clval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 THR_TEST         \t\t").arg(fChannel).arg(fSlave).arg(nx);
      uint8_t testval=theContext->getTrimRegister(128);
      line.append(QString("0x%1 ").arg(testval,0,16));
      line.append ("\n");
      WriteConfigFile(line);


      for(int row=0; row<8;++row)
           {
              int tstart=row*16;
              int tend=(row+1)*16 -1;
              line= QString("SFP %1 NYX %2 nXY %3 THR_%4_%5     \t").arg(fChannel).arg(fSlave).arg(nx).arg(tstart).arg(tend);
              for(int t=tstart; t<tstart+16;++t)
                {
                  uint8_t tval=theContext->getTrimRegister(t);
                  line.append(QString("0x%1 ").arg(tval,0,16));
                }
              line.append ("\n");
              WriteConfigFile(line);
           }


      uint8_t adcp=0x01;
      line= QString("SFP %1 NYX %2 nXY %3 ADC_DCO_PHASE   \t0x%4 \n").arg(fChannel).arg(fSlave).arg(nx).arg(adcp,0,16);
      WriteConfigFile (line);
    } // for nx
  WriteConfigFile (QString ("#\n"));
  return 0;
}

void NyxorGui::InitChainBtn_clicked ()
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
  EnableI2C ();
}
void NyxorGui::EnableI2C ()
{
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, 0x7f000000);

  int dat = 0;
  dat = 0x8c;
  dat += 0x0 << 8;
  dat += 0x0 << 16;
  dat += I2C_CTRL_A << 24;

  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, dat);

//  // has to be changed a bit if more than one nxy are connected to
//          // one GEMEX/NYXOR

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
    return;    // for nyxor we can not dump all connected frontends at once, NxI2c works on current slave only

  char buffer[1024];
  AppendTextWindow ("--- Register Dump ---:");

  std::stringstream buf;
  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    fNxTab[nx]->dumpConfig (buf);
  }

  AppendTextWindow (buf.str ().c_str ());

}

void NyxorGui::ClearOutputBtn_clicked ()
{
//std::cout << "NyxorGui::ClearOutputBtn_clicked()"<< std::endl;
  TextOutput->clear ();
  TextOutput->setPlainText (
      "Welcome to NYXOR GUI!\n\t v0.7 of 27-July-2015 by JAM (j.adamczewski@gsi.de)\n\tContains parts of ROC/nxyter GUI by Sergey Linev, GSI");

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
  if (fileName.endsWith (".txt"))
  {
    snprintf (buffer, 1024, "bash -c \"m_set_nxy %s \" ", fileName.toLatin1 ().constData ());
  }
  else
  {
    if (!fileName.endsWith (".gos"))
      fileName.append (".gos");
    snprintf (buffer, 1024, "gosipcmd -x -c %s ", fileName.toLatin1 ().constData ());

  }
  QString com (buffer);
  QString result = ExecuteGosipCmd (com, 10000);    // this will just execute the command in shell, gosip or not
  AppendTextWindow (result);

}

void NyxorGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
  fDebug = on;
}

void NyxorGui::HexBox_changed (int on)
{
  fNumberBase = (on ? 16 : 10);
//std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;
  RefreshView ();
// optionally refresh display of subdisplays:
  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    fNxTab[nx]->showContext ();
  }
}

void NyxorGui::Slave_changed (int)
{
//std::cout << "NyxorGui::Slave_changed" << std::endl;
  EvaluateSlave ();
  bool triggerchangeable = AssertNoBroadcast (false);
  RefreshButton->setEnabled (triggerchangeable);

}

void NyxorGui::RefreshView ()
{
// display setup structure to gui:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";

// note that nxyter tabs refresh themselves when updating context

  QString statustext;
  statustext.append ("SFP ");
  statustext.append (text.setNum (fChannel));
  statustext.append (" DEV ");
  statustext.append (text.setNum (fSlave));
  statustext.append (" - Last refresh:");
  statustext.append (QDateTime::currentDateTime ().toString (Qt::TextDate));
  StatusLabel->setText (statustext);

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
  EnableI2C ();    // must be done since mbs setup program may shut i2c off at the end
// write register values from strucure with gosipcmd
  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    fNxTab[nx]->setSubConfig ();
  }

}

void NyxorGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  EnableI2C ();
  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    fNxTab[nx]->getSubConfig ();
  }

}

uint8_t NyxorGui::ReadNyxorI2c (int nxid, uint8_t address)
{
  uint8_t val = 0;

  int dat;
  int nxad = 0;
  if (nxid == 0)
    nxad = I2C_ADDR_NX0 + 1;
  else if (nxid == 1)
    nxad = I2C_ADDR_NX1 + 1;

  dat = 0;
  dat += address << 8;
  dat += nxad << 16;
  dat += I2C_COTR_A << 24;

// set i2c address to read from:
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, dat);
// todo need error checking here?
  val = ReadGosip (fChannel, fSlave, GOS_I2C_DRR1);
  return val;
}

int NyxorGui::ReadGosip (int sfp, int slave, int address)
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

int NyxorGui::WriteNyxorI2c (int nxid, uint8_t address, uint8_t value, bool veri)
{
  int dat = 0;    // data word to send
  int nxad = 0;
  if (nxid == 0)
    nxad = I2C_ADDR_NX0;
  else if (nxid == 1)
    nxad = I2C_ADDR_NX1;

  dat = value;
  dat += address << 8;
  dat += nxad << 16;
  dat += I2C_COTR_A << 24;
  WriteGosip (fChannel, fSlave, GOS_I2C_DWR, dat);
}

int NyxorGui::SaveGosip (int sfp, int slave, int address, int value)
{
//std::cout << "# SaveGosip" << std::endl;
  static char buffer[1024] = { };
  snprintf (buffer, 1024, "%d %d %x %x \n", sfp, slave, address, value);
  QString line (buffer);
  return (WriteConfigFile (line));
}

int NyxorGui::WriteGosip (int sfp, int slave, int address, int value)
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
    std::cout << " NyxorGui: " << buf.str ().c_str ();
    AppendTextWindow (buf.str ().c_str ());
    result = "ERROR";
  }
  QApplication::restoreOverrideCursor ();
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
  NyxorGui::fInstance->AppendTextWindow (c_str);

  va_end(args);
}

/** this one from Nik to speed down direct mbspex io*/
void NyxorGui::I2c_sleep ()
{
  usleep(300);

// JAM: test avoid arbirtrary loop
//#define N_LOOP 300000
//
//  int l_ii;
//  int volatile l_depp = 0;
//
//  for (l_ii = 0; l_ii < N_LOOP; l_ii++)
//  {
//    l_depp++;
//  }
}

#endif

