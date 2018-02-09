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



/*
 *  Constructs a NyxorGui which is a child of 'parent', with the
 *  name 'name'.'
 */
NyxorGui::NyxorGui (QWidget* parent, bool oldsetup) :
    GosipGui (parent), fOldSetup(oldsetup)
{

  fImplementationName="NYXOR";
  QString msg=QString( "Welcome to NYXOR GUI!\n\t v0.92 of 17-Jan-2018 by JAM (j.adamczewski@gsi.de)\n\tContains parts of ROC/nxyter GUI by Sergey Linev, GSI\n\t%1").
         arg(fOldSetup ? "Using old register adresses.": "Using new register addresses.");

  fVersionString=msg;

  setWindowTitle(QString("%1 GUI").arg(fImplementationName));

  fNyxorWidget=new NyxorWidget(this);
  Settings_scrollArea->setMinimumHeight(550); // JAM2018 - adjust default container for large nxyter stuff
  Settings_scrollArea->setWidget(fNyxorWidget);

 
// here components for (nxyter and other blocks) tabs:


  fGeneralTab = new GeneralNyxorWidget (fNyxorWidget->Nxyter_tabWidget, this);
  fNyxorWidget->Nxyter_tabWidget->addTab (fGeneralTab, QString ("Receiver"));

  fADCTab =  new NyxorADCWidget (fNyxorWidget->Nxyter_tabWidget, this);
  fNyxorWidget->Nxyter_tabWidget->addTab (fADCTab, QString ("ADC"));

  fDACTab =  new NyxorDACWidget (fNyxorWidget->Nxyter_tabWidget, this);
  fNyxorWidget->Nxyter_tabWidget->addTab (fDACTab, QString ("DACs"));


  for (int nx = 0; nx < NYXOR_NUMNX; nx++)
  {
    NxyterWidget* nxw = new NxyterWidget (fNyxorWidget->Nxyter_tabWidget, this, nx);
    fNyxorWidget->Nxyter_tabWidget->addTab (nxw, QString ("NX%1").arg (nx));
    fNxTab[nx] = nxw;
  }



  GetSFPChainSetup(); // ensure that any slave has a status structure before we begin clicking...
  ClearOutputBtn_clicked (); // print correct program name!
  show ();
}

NyxorGui::~NyxorGui ()
{

}





void NyxorGui::SaveConfig()
{
//std::cout << "NyxorGui::SaveConfig()"<< std::endl;

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
    GOSIP_BROADCAST_ACTION(SaveRegisters()); // refresh actual setup from hardware and write it to open file
  }

  else if (fileName.endsWith (".dmp"))
  {
    // dump configuration
    // TODO: this will currently only save nxyter chip setup, not the rest of nyxor TODO:
    WriteConfigFile (QString ("#Format *.dmp - nxyter context dump output\n"));
    WriteConfigFile (QString ("#                                         \n"));
    GOSIP_BROADCAST_ACTION(SaveRegistersDump());

  }
  else if (fileName.endsWith (".txt"))
  {
    // here functions to export context values into Niks format
    WriteNiksConfigHeader(!AssertNoBroadcast (false));
    GOSIP_BROADCAST_ACTION(WriteNiksConfig());

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



int NyxorGui::WriteNiksConfigHeader (bool globalsetup)
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

     WriteConfigFile (QString ("#\n"));
     WriteConfigFile (QString ("#\n"));


     QString pline;
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



int NyxorGui::WriteNiksConfig ()
{
  QString pline;
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


  GosipGui::RefreshView ();
   // ^this handles the refresh of chains and status. better use base class function here! JAM2018

//  RefreshChains();
//
//  QString statustext;
//  statustext.append ("SFP ");
//  statustext.append (text.setNum (fSFP));
//  statustext.append (" DEV ");
//  statustext.append (text.setNum (fSlave));
//  statustext.append (" - Last refresh:");
//  statustext.append (QDateTime::currentDateTime ().toString (Qt::TextDate));
//  StatusLabel->setText (statustext);

}

void NyxorGui::EvaluateView ()
{
  fGeneralTab->EvaluateView();
  fADCTab->EvaluateView();
  fDACTab->EvaluateView();
}



void NyxorGui::SetRegisters ()
{
  DoSetRegisters(false);

}




void NyxorGui::ApplyFileConfig(int )
{
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


void NyxorGui::SaveRegisters()
{
  GetRegisters();
  fSaveConfig = true;    // switch to file output mode
  DoSetRegisters (true);    // with option force: register settings are written to file
  fSaveConfig = false;
}

void NyxorGui::SaveRegistersDump()
{
  GetRegisters();
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


void NyxorGui::SaveRegistersNik()
{
  GetRegisters();
  WriteNiksConfig();
}



void NyxorGui::ResetSlave()
{
   AppendTextWindow ("--- Resetting logic on NYXOR... ");
   FullNyxorReset();
   ReceiverReset();
   NXTimestampReset();
}

void NyxorGui::DumpSlave()
{
  if (fBroadcasting)
     return;    // for nyxor we can not dump all connected frontends at once, NxI2c works on current slave only
   std::stringstream buf;
   for (int nx = 0; nx < NYXOR_NUMNX; nx++)
   {
     fNxTab[nx]->dumpConfig (buf);
   }
   AppendTextWindow (buf.str ().c_str ());
}





void NyxorGui::DoSetRegisters (bool force)
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
  if(fOldSetup)
  {
    if (nxid == 1)
      nxad = I2C_ADDR_NX0 + 1;
    else if (nxid == 0)
      nxad = I2C_ADDR_NX1 + 1;
  }
  else
  {
    nxad = I2C_ADDR_NX1 + 1;
  }

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
  int dat=0;
  if(fOldSetup)
    dat=I2C_DAC_BASE_R + (0x100000 * nxid) + (0x1000 << dacid);
  else
    dat=I2C_DAC_BASE_R + (0x1000 << dacid);

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






int NyxorGui::WriteNyxorI2c (int nxid, uint8_t address, uint8_t value, bool veri)
{
  int dat = 0;    // data word to send
  int nxad = 0;
  if(fOldSetup)
  {
    if (nxid == 1)
      nxad = I2C_ADDR_NX0;
    else if (nxid == 0)
      nxad = I2C_ADDR_NX1;
  }
  else
  {
    nxad = I2C_ADDR_NX1;
  }

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
  int dat=0;
  if(fOldSetup)
    dat= I2C_DAC_BASE_W + (0x100000 * nxid) + (0x1000 << dacid) + (value & 0xFFF);
  else
    dat= I2C_DAC_BASE_W + (0x1000 << dacid) + (value & 0xFFF);

  return WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
}

int NyxorGui::WriteNyxorAddress (uint8_t address, uint32_t value)
{
  //printf("WriteNyxorAddress(0x%x, 0x%x)\n",address,value);
  int dat = (address << 24) | (value & 0xFFF);
  return WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
}








