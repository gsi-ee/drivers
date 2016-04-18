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
#include <QTimer>

#include <sstream>
#include <string.h>
#include <errno.h>

#include "nxyterwidget.h"
#include "generalwidget.h"
#include "dacwidget.h"
#include "adcwidget.h"

// *********************************************************

NyxorGui* NyxorGui::fInstance = 0;

/*
 *  Constructs a NyxorGui which is a child of 'parent', with the
 *  name 'name'.'
 */
NyxorGui::NyxorGui (QWidget* parent) :
    QWidget (parent), fDebug (false), fSaveConfig (false), fSFP (0), fSlave (0), fSFPSave (0), fSlaveSave (0),
        fConfigFile (0)
{
  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
#endif

  fNumberBase = 16; // hexmode by default
  HexBox->setChecked(true);


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

// here components for (nxyter and other blocks) tabs:


  fGeneralTab = new GeneralNyxorWidget (Nxyter_tabWidget, this);
  Nxyter_tabWidget->addTab (fGeneralTab, QString ("Receiver"));

  fADCTab =  new NyxorADCWidget (Nxyter_tabWidget, this);
  Nxyter_tabWidget->addTab (fADCTab, QString ("ADC"));

  fDACTab =  new NyxorDACWidget (Nxyter_tabWidget, this);
  Nxyter_tabWidget->addTab (fDACTab, QString ("DACs"));


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
  GetSFPChainSetup();
  if(!AssertNoBroadcast(false)) return;
  if(!AssertChainConfigured()) return;
  GetRegisters ();
  RefreshView ();
}

void NyxorGui::ApplyBtn_clicked ()
{
//std::cout << "NyxorGui::ApplyBtn_clicked()"<< std::endl;

  //char buffer[1024];
  EvaluateSlave ();
  //std::cout << "InitChainBtn_clicked()"<< std::endl;

// JAM disabled confirm window as wished by Henning H.
//  snprintf (buffer, 1024, "Really apply NYXOR settings  to SFP %d Device %d?", fSFP, fSlave);
//  if (QMessageBox::question (this, "NYXOR GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
//      != QMessageBox::Yes)
//  {
//    return;
//  }

  EvaluateView ();

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

  // TODO: loop over all configured slaves if known!
  bool writeheader=true;
#ifdef USE_MBSPEX_LIB
  int sfpinit=0, sfpmax=0;
  int devinit=0, devmax=0;
  int oldslave = fSlave;
  int oldsfp = fSFP;
  bool broadcast=false;
  if(AssertNoBroadcast (false))
    {
        // if "all devs" button is not pressed, we just want to save currently selected slave:
        sfpinit=fSFP; sfpmax=fSFP+1;
        devinit=fSlave; devmax=fSlave+1;
        broadcast=false;
    }
  else
    {
      sfpinit=0; sfpmax=PEX_SFP_NUMBER;
      devinit=0;
      broadcast=true;
    }

  for(fSFP=sfpinit; fSFP<sfpmax; ++fSFP)
    {
      if (fSFPChains.numslaves[fSFP] == 0) continue;
      if(broadcast) devmax=fSFPChains.numslaves[fSFP];
      for (fSlave = devinit; fSlave < devmax; ++fSlave)
          {
            if(broadcast) GetRegisters(); // refresh status structures from hardware before saving
#endif

  if (fileName.endsWith (".gos"))
  {
    WriteConfigFile (QString ("#Format *.gos"));
    WriteConfigFile (QString ("#usage: gosipcmd -x -c file.gos \n"));
    WriteConfigFile (QString ("#                                         \n"));
    WriteConfigFile (QString ("#sfp slave address value\n"));
    fSaveConfig = true;    // switch to file output mode
    SetRegisters (true);    // with option force: register settings are written to file
    fSaveConfig = false;
  }

  else if (fileName.endsWith (".dmp"))
  {
    // dump configuration

    // TODO: this will currently only save nxyter chip setup, not the rest of nyxor TODO:
    WriteConfigFile (QString ("#Format *.dmp - nxyter context dump output\n"));
    WriteConfigFile (QString ("#                                         \n"));

    for (int nx = 0; nx < NYXOR_NUMNX; nx++)
    {
      QString header = QString ("#sfp %1 slave %2 nxyter %3 \n").arg (fSFP).arg (fSlave).arg (nx);
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
    WriteNiksConfig (broadcast, writeheader);
    writeheader=false;
  }
  else
  {
    std::cout << "NyxorGui::SaveConfigBtn_clicked( -  unknown file type, NEVER COME HERE!!!!)" << std::endl;
  }

// TODO: end loop over configured slaves if known
#ifdef USE_MBSPEX_LIB
          } //for (int fSlave
    } // for (int fSFP

  fSlave=oldslave;
  fSFP=oldsfp;


#endif

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

int NyxorGui::WriteNiksConfig (bool globalsetup, bool writeheader)
{
  if (writeheader)
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
#ifdef USE_MBSPEX_LIB
      if (globalsetup)
      {
        for (int i = 0; i < 16; ++i)
        {
          if (i < fSFPChains.numslaves[ch])
            sfpline.append ("2  ");
          else
            sfpline.append ("0  ");
        }    // i

      }
      else
#endif
      {

        if (fSFP == ch || (fSFP < 0 && ch == 0))
        {
          for (int i = 0; i < 16; ++i)
          {
            if (fSlave == i || (fSlave < 0 && i == 0))    // old: assign broadcast mode to channel 0 for the moment
              sfpline.append ("2  ");
            else
              sfpline.append ("0  ");
          }
        }
        else
        {
          sfpline.append ("0  0  0  0  0  0  0  0  0  0  0  0  0  0  0  0");
        }
      }    // globalsetup
      sfpline.append ("\n");
      WriteConfigFile (sfpline);
    }    // for ch




  } // if writeheader
  WriteConfigFile (QString ("#\n"));
  WriteConfigFile (QString ("#\n"));
  QString pline;
  if (writeheader)
  {
    // note that this is currently only written once for all nyxors?
    WriteConfigFile (QString ("#               pre trg win   post trg win  test pulse delay  test trg delay\n"));
    uint16_t pretrig = fGeneralTab->fSetup.fTriggerPre;
    uint16_t posttrig = fGeneralTab->fSetup.fTriggerPost;
    uint16_t pulsdel = fGeneralTab->fSetup.fDelayTestPulse;
    uint16_t trigdel = fGeneralTab->fSetup.fDelayTrigger;
    pline = QString ("GLOBAL_PARAM    0x%1          0x%2          0x%3              0x%4\n").arg (pretrig, 0, 16).arg (
        posttrig, 0, 16).arg (pulsdel, 0, 16).arg (trigdel, 0, 16);
    WriteConfigFile (pline);
    //WriteConfigFile (QString ("GLOBAL_PARAM                  0x080           0x100         0x0b              0x0a\n"));
  }
  WriteConfigFile (QString ("#\n"));
  WriteConfigFile (QString ("#\n"));
  WriteConfigFile (QString ("#\n"));
  uint16_t nxctrl=fGeneralTab->fSetup.fNXControl;
  pline=QString ("NXY_CTRL SFP %1 NYX %2  \t\t0x%3\n").arg(fSFP).arg(fSlave).arg(nxctrl,0,16);
  WriteConfigFile (pline);

  // loop over nxyters for current sfp and slave:
  int iadd=0x22; // i2caddress for each nxyter on board
  int reset=0x84; // reset address
  for (int nx = 0; nx < NYXOR_NUMNX; nx++, iadd-=0x10, reset+=0x1) // JAM2016 -note swapped iadd order for nx0 and nx1!
    {
      WriteConfigFile (QString ("#\n"));
      WriteConfigFile (QString ("#\n"));
      WriteConfigFile (QString ("#\n"));

      QString line;
      const nxyter::NxContext* theContext = fNxTab[nx]->getContext ();
      line= QString("SFP %1 NYX %2 nXY %3 I2C_ADDR    \t\t0x%4 # wr\n").arg(fSFP).arg(fSlave).arg(nx).arg(iadd,0,16);
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 RESET     \t\t0x%4 \n").arg(fSFP).arg(fSlave).arg(nx).arg(reset,0,16);
      WriteConfigFile(line);
      line= QString("SFP %1 NYX %2 nXY %3 MASK      \t\t").arg(fSFP).arg(fSlave).arg(nx);
      for(int m=0; m<16;++m)
      {
        uint8_t mval=theContext->getRegister(m); // mask registers are at the start of array
        line.append(QString("0x%1 ").arg(mval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 BIAS      \t\t").arg(fSFP).arg(fSlave).arg(nx);
      for(int b=0; b<14;++b)
          {
            uint8_t bval=theContext->getRegister(b+16);
            line.append(QString("0x%1 ").arg(bval,0,16));
          }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 CONFIG    \t\t").arg(fSFP).arg(fSlave).arg(nx);
      for(int c=0; c<2;++c)
      {
        uint8_t cval=theContext->getRegister(c+32);
        line.append(QString("0x%1 ").arg(cval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 TEST_DELAY   \t\t").arg(fSFP).arg(fSlave).arg(nx);
      for(int d=0; d<2;++d)
      {
        uint8_t dval=theContext->getRegister(d+38);
        line.append(QString("0x%1 ").arg(dval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 CLOCK_DELAY   \t").arg(fSFP).arg(fSlave).arg(nx);
      for(int cl=0; cl<3;++cl)
      {
        uint8_t clval=theContext->getRegister(cl+43);
        line.append(QString("0x%1 ").arg(clval,0,16));
      }
      line.append ("\n");
      WriteConfigFile(line);

      line= QString("SFP %1 NYX %2 nXY %3 THR_TEST         \t").arg(fSFP).arg(fSlave).arg(nx);
      uint8_t testval=theContext->getTrimRegister(128);
      line.append(QString("0x%1 ").arg(testval,0,16));
      line.append ("\n");
      WriteConfigFile(line);


      for(int row=0; row<8;++row)
           {
              int tstart=row*16;
              int tend=(row+1)*16 -1;
              line= QString("SFP %1 NYX %2 nXY %3 THR_%4_%5      \t").arg(fSFP).arg(fSlave).arg(nx).arg(tstart).arg(tend);
              for(int t=tstart; t<tstart+16;++t)
                {
                  uint8_t tval=theContext->getTrimRegister(t);
                  line.append(QString("0x%1 ").arg(tval,0,16));
                }
              line.append ("\n");
              WriteConfigFile(line);
           }


      //uint8_t adcp=0x01;
      uint8_t adcp=fADCTab->fSetup.fDC0Phase;
      line= QString("SFP %1 NYX %2 nXY %3 ADC_DCO_PHASE   \t0x%4 \n").arg(fSFP).arg(fSlave).arg(nx).arg(adcp,0,16);
      WriteConfigFile (line);

      //SFP 0 NYX 0 nXY 0 EXT_DACS    0x100 0x101 0x102 0x103
      uint16_t dac0=fDACTab->fSetup.fRegister[nx][0];
      uint16_t dac1=fDACTab->fSetup.fRegister[nx][1];
      uint16_t dac2=fDACTab->fSetup.fRegister[nx][2];
      uint16_t dac3=fDACTab->fSetup.fRegister[nx][3];
      line= QString("SFP %1 NYX %2 nXY %3 EXT_DACS   \t\t0x%4 0x%5 0x%6 0x%7 \n")
          .arg(fSFP).arg(fSlave).arg(nx)
          .arg(dac0,0,16).arg(dac1,0,16).arg(dac2,0,16).arg(dac3,0,16);
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
  snprintf (buffer, 1024, "Please specify NUMBER OF DEVICES to initialize at SFP %d ?", fSFP);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  int numslaves = QInputDialog::getInt (this, tr ("Number of Slaves?"), tr (buffer), 1, 1, 1024, 1, &ok);
#else
  int numslaves = QInputDialog::getInteger(this, tr("Number of Slaves?"),
      tr(buffer), 1, 1, 1024, 1, &ok);

#endif
  if (!ok)
    return;
  if (fSFP < 0)
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
  GetSFPChainSetup();
  RefreshChains();
}

void NyxorGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
  snprintf (buffer, 1024, "Really reset logic on NYXOR/GEMEX device at SFP %d, Slave %d ?", fSFP, fSlave);
  if (QMessageBox::question (this, "Nyxor GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }

  AppendTextWindow ("--- Resetting logic on NYXOR... ");
  FullNyxorReset();
  ReceiverReset();
  NXTimestampReset();
}


void NyxorGui::FullNyxorReset()
{
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x7f000000);
}

void NyxorGui::ReceiverReset()
{
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x7e000000);
}

void NyxorGui::NXTimestampReset()
{
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x21000001);
}

void NyxorGui::DisableI2C()
{
  int dat = 0;
//  dat += 0x0 << 8;
//  dat += 0x0 << 16;
  dat += I2C_CTRL_A << 24;

   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
}

void NyxorGui::EnableI2CWrite (int nxid)
{
   int dat = 0;
  //dat = 0x8c; // 84 (nx0)oder 85 (nx1)
  if(nxid==0)
    dat= 0x84;
  else if (nxid==1)
    dat= 0x85;

  dat += I2C_CTRL_A << 24;

  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

}




void NyxorGui::EnableI2CRead (int nxid)
{
  int dat = 0;
  //dat = 0x8c; // 84 (nx0)oder 85 (nx1)
  if(nxid==0)
    dat= 0x80; //0x84;
  else if (nxid==1)
    dat= 0x81; //0x85;

  dat += I2C_CTRL_A << 24;

  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

}


void NyxorGui::EnableSPI()
{
  int dat=0x80;
  dat += SPI_ENABLE_ADDR<< 24;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);// enable sub core

  dat=0x44;
  dat += SPI_BAUD_ADDR << 24;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat); // set baud rate

}


void NyxorGui::DisableSPI()
{
  int dat=0;
  dat += SPI_ENABLE_ADDR<< 24;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

}




void NyxorGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "NyxorGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
  if (checked)
  {
    fSFPSave = SFPspinBox->value ();
    fSlaveSave = SlavespinBox->value ();
    SFPspinBox->setValue (-1);
    SlavespinBox->setValue (-1);
  }
  else
  {
    RefreshChains();
    SFPspinBox->setValue (fSFPSave);
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
      "Welcome to NYXOR GUI!\n\t v0.981 of 18-April-2016 by JAM (j.adamczewski@gsi.de)\n\tContains parts of ROC/nxyter GUI by Sergey Linev, GSI");

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
  ShowBtn_clicked() ;
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

//void NyxorGui::Slave_changed (int)
//{
////std::cout << "NyxorGui::Slave_changed" << std::endl;
//  EvaluateSlave ();
//  bool triggerchangeable = AssertNoBroadcast (false);
//  RefreshButton->setEnabled (triggerchangeable);
//
//}


void NyxorGui::Slave_changed (int)
{
  //std::cout << "NyxorGui::Slave_changed" << std::endl;
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
    //ShowBtn_clicked() ;
  }
}







void NyxorGui::GetSFPChainSetup()
{
//  std::cout<<"GetSFPChainSetup... "<< std::endl;
#ifdef USE_MBSPEX_LIB
    // broadcast mode: find out number of slaves and loop over all registered slaves
    mbspex_get_configured_slaves(fPexFD, &fSFPChains);

// probably later we also keep all setups as local copies?
// for the moment, this gives too much overhead due to separated subtab data....
//    // dynamically increase array of setup structures:
//    for(int sfp=0; sfp<4; ++sfp)
//    {
//      while(fSetup[sfp].size()<fSFPChains.numslaves[sfp])
//      {
//        fSetup[sfp].push_back(FebexSetup());
//        //std::cout<<"GetSFPChainSetup increased setup at sfp "<<sfp<<" to "<<fSetup[sfp].size()<<" slaves." << std::endl;
//      }
//    }
/// end example from febex gui JAM 2016
#endif

}



void NyxorGui::RefreshChains ()
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
#endif

}


void NyxorGui::RefreshView ()
{
// display setup structure to gui:
  QString text;
//  QString pre;
//  fNumberBase == 16 ? pre = "0x" : pre = "";

// note that nxyter tabs refresh themselves when updating context


  fGeneralTab->RefreshView();
  fADCTab->RefreshView();
  fDACTab->RefreshView();

  RefreshChains();

  QString statustext;
  statustext.append ("SFP ");
  statustext.append (text.setNum (fSFP));
  statustext.append (" DEV ");
  statustext.append (text.setNum (fSlave));
  statustext.append (" - Last refresh:");
  statustext.append (QDateTime::currentDateTime ().toString (Qt::TextDate));
  StatusLabel->setText (statustext);

}

void NyxorGui::EvaluateView ()
{
  fGeneralTab->EvaluateView();
  fADCTab->EvaluateView();
  fDACTab->EvaluateView();
}

void NyxorGui::EvaluateSlave ()
{
  fSFP = SFPspinBox->value ();
  fSlave = SlavespinBox->value ();
}

void NyxorGui::SetRegisters (bool force)
{
// write register values from strucure with gosipcmd
  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    if(!force && !fNxTab[nx]->needSetSubConfig()) continue;
      //EnableI2CWrite (nx);
      EnableI2CRead (nx); // read means without reset
      fNxTab[nx]->setSubConfig (force);
  }

  fGeneralTab->SetRegisters(force);
  fADCTab->SetRegisters(force);
  fDACTab->SetRegisters(force);
}

void NyxorGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    EnableI2CRead (nx);
    fNxTab[nx]->getSubConfig ();
  }
  DisableI2C();
  fGeneralTab->GetRegisters();
  fADCTab->GetRegisters();
  fDACTab->GetRegisters();
}

uint8_t NyxorGui::ReadNyxorI2c (int nxid, uint8_t address)
{
  uint8_t val = 0;

  int dat;
  int nxad = 0;
  if (nxid == 1)
    nxad = I2C_ADDR_NX0 + 1;
  else if (nxid == 0)
    nxad = I2C_ADDR_NX1 + 1;

  dat = 0;
  dat += address << 8;
  dat += nxad << 16;
  dat += I2C_COTR_A << 24;

// set i2c address to read from:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
// todo need error checking here?
  val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1);

  return val;
}


uint8_t NyxorGui::ReadNyxorSPI (uint8_t address)
{
  uint8_t val=0;
  int dat = 0;
  dat += address << 8;
  dat += SPI_READ << 16;
  dat += SPI_TRANS_ADDR << 24;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1);
  return val;
}


uint16_t NyxorGui::ReadNyxorDAC(int nxid, uint8_t dacid)
{
  uint16_t val=0;


  // read back external dacs
//           for (l_l=0; l_l<4; l_l++)
//           {
//             l_wr_d = 0xbc00000 + (0x100000 * l_k) + (0x1000 << l_l);
//             l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, l_wr_d);
//             debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
//                                     l_i, l_j, l_k, l_l, GOS_I2C_DWR, l_wr_d));
//             l_stat = f_pex_slave_wr (l_i, l_j, GOS_I2C_DWR, 0x84000000);
//             debug3 (("write SFP: %1d, NYX: %1d, nXY: %1d => %3d  A: 0x%x, D: 0x%x\n",
//                                     l_i, l_j, l_k, l_l, GOS_I2C_DWR, 0x84000000));
//
//             l_stat = f_pex_slave_rd (l_i, l_j, GOS_I2C_DRR1, &l_data);
//             debug3 (("read                         => %3d  A: 0x%x, D: 0x%x\n",
//                                                     l_l, GOS_I2C_DRR1, l_data));
//             l_data    = (l_data >> 6) & 0x3ff;
//             l_check_d = l_ext_dac[l_i][l_j][l_k][l_l];

   int dat=I2C_DAC_BASE_R + (0x100000 * nxid) + (0x1000 << dacid); // ?? really shift by dacid 0..3?

  // set i2c address to read from:
    WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // enable i2c receiver?
   dat =  I2C_RECEIVE << 24;
   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);


    val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1);


    return (val >> 6) & 0x3ff;
}



uint32_t NyxorGui::ReadNyxorAddress (uint8_t address)
{
  int dat = (address << 24);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  uint32_t val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1);
  // error handling?
  //printf("ReadNyxorAddress(0x%x) returns 0x%x\n",address,val);
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
  if (nxid == 1)
    nxad = I2C_ADDR_NX0;
  else if (nxid == 0)
    nxad = I2C_ADDR_NX1;

  dat = value;
  dat += address << 8;
  dat += nxad << 16;
  dat += I2C_COTR_A << 24;
  return WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
}


int NyxorGui::WriteNyxorSPI (uint8_t address, uint8_t value)
{
  int dat = 0;
  dat = value;
  dat += address << 8;
  dat += SPI_WRITE << 16;
  dat += SPI_TRANS_ADDR << 24;
  return WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
}


int NyxorGui::WriteNyxorDAC(int nxid, uint8_t dacid, uint16_t value)
{

      //write external dac values
       //         l_wr_d = 0xb430000 + (0x100000 * l_k) + (0x1000 << l_l) + l_ext_dac[l_i][l_j][l_k][l_l];
  int dat= I2C_DAC_BASE_W + (0x100000 * nxid) + (0x1000 << dacid) + (value & 0xFFF);
  return WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
}

int NyxorGui::WriteNyxorAddress (uint8_t address, uint32_t value)
{
  //printf("WriteNyxorAddress(0x%x, 0x%x)\n",address,value);
  int dat = (address << 24) | (value & 0xFFF);
  return WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
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

bool NyxorGui::AssertChainConfigured (bool verbose)
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



bool NyxorGui::AssertNoBroadcast (bool verbose)
{
  if (fSFP < 0 || fSlave < 0)
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
  //usleep(500);
  usleep(800);
// JAM: test avoid arbirtrary loop
//#define N_LOOP 500000
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

