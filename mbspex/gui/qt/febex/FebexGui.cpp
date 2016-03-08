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
#include <QTimer>

#include <sstream>
#include <string.h>
#include <errno.h>
#include <math.h>

// *********************************************************


// this we need to implement for output of mbspex library, but also useful to format output without it:
FebexGui* FebexGui::fInstance = 0;


#include <stdarg.h>


/** JAM The following nice define handles all explicit broadcast actions depending on the currently set slave*/
#define FEBEX_BROADCAST_ACTION(X) \
fBroadcasting=true;  \
int oldslave = fSlave; \
int oldchan = fSFP; \
if (AssertNoBroadcast (false)) \
 { \
   X(); \
 } \
 else if (fSFP < 0) \
 { \
   for (int sfp = 0; sfp < 4; ++sfp) \
   {\
     if (fSFPChains.numslaves[sfp] == 0) \
       continue; \
     fSFP = sfp; \
     if (fSlave < 0) \
     { \
       for (int feb = 0; feb < fSFPChains.numslaves[sfp]; ++feb) \
       { \
         fSlave = feb; \
         X(); \
       } \
     } \
     else \
     { \
       X();\
     }\
   }\
 } \
 else if (fSlave< 0) \
 { \
   for (int feb = 0; feb < fSFPChains.numslaves[fSFP]; ++feb) \
       { \
         fSlave = feb; \
         X(); \
       } \
 } \
 else \
 { \
   AppendTextWindow ("--- NEVER COME HERE: undefined broadcast mode ---:"); \
 } \
fSlave= oldslave;\
fSFP= oldchan; \
fBroadcasting=false;




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
    QWidget (parent), fDebug (false), fSaveConfig (false), fBroadcasting(false), fSFP (0), fSlave (0), fSFPSave (0), fSlaveSave (0),
        fConfigFile (0)
{
  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
#endif

  fNumberBase = 10;

  memset( &fSFPChains, 0, sizeof(struct pex_sfp_links));

  for(int sfp=0; sfp<4;++sfp)
    fSetup[sfp].clear();


  SFPspinBox->setValue (fSFP);
  SlavespinBox->setValue (fSlave);
  DAC_spinBox_all->setValue (500);
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

  QObject::connect (AutoAdjustButton, SIGNAL (clicked ()), this, SLOT (AutoAdjustBtn_clicked ()));


  QObject::connect (DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
  QObject::connect (HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));
  QObject::connect (SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
  QObject::connect (SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
  QObject::connect (DAC_spinBox_all, SIGNAL(valueChanged(int)), this, SLOT(DAC_spinBox_all_changed(int)));
  QObject::connect (DAC_spinBox_00, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox00_changed(int)));
  QObject::connect (DAC_spinBox_01, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox01_changed(int))); 
  QObject::connect (DAC_spinBox_02, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox02_changed(int)));
  QObject::connect (DAC_spinBox_03, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox03_changed(int)));
  QObject::connect (DAC_spinBox_04, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox04_changed(int)));
  QObject::connect (DAC_spinBox_05, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox05_changed(int)));
  QObject::connect (DAC_spinBox_06, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox06_changed(int)));
  QObject::connect (DAC_spinBox_07, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox07_changed(int)));
  QObject::connect (DAC_spinBox_08, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox08_changed(int)));
  QObject::connect (DAC_spinBox_09, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox09_changed(int)));
  QObject::connect (DAC_spinBox_10, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox10_changed(int)));
  QObject::connect (DAC_spinBox_11, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox11_changed(int)));
  QObject::connect (DAC_spinBox_12, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox12_changed(int)));
  QObject::connect (DAC_spinBox_13, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox13_changed(int)));
  QObject::connect (DAC_spinBox_14, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox14_changed(int)));
  QObject::connect (DAC_spinBox_15, SIGNAL(valueChanged(int)), this, SLOT (Any_spinBox15_changed(int)));



  /** JAM put references to designer checkboxes into array to be handled later easily: */
  fBaselineBoxes[0]=Baseline_Box_00;
  fBaselineBoxes[1]=Baseline_Box_01;
  fBaselineBoxes[2]=Baseline_Box_02;
  fBaselineBoxes[3]=Baseline_Box_03;
  fBaselineBoxes[4]=Baseline_Box_04;
  fBaselineBoxes[5]=Baseline_Box_05;
  fBaselineBoxes[6]=Baseline_Box_06;
  fBaselineBoxes[7]=Baseline_Box_07;
  fBaselineBoxes[8]=Baseline_Box_08;
  fBaselineBoxes[9]=Baseline_Box_09;
  fBaselineBoxes[10]=Baseline_Box_10;
  fBaselineBoxes[11]=Baseline_Box_11;
  fBaselineBoxes[12]=Baseline_Box_12;
  fBaselineBoxes[13]=Baseline_Box_13;
  fBaselineBoxes[14]=Baseline_Box_14;
  fBaselineBoxes[15]=Baseline_Box_15;

  fDACSpinBoxes[0] = DAC_spinBox_00;
  fDACSpinBoxes[1] = DAC_spinBox_01;
  fDACSpinBoxes[2] = DAC_spinBox_02;
  fDACSpinBoxes[3] = DAC_spinBox_03;
  fDACSpinBoxes[4] = DAC_spinBox_04;
  fDACSpinBoxes[5] = DAC_spinBox_05;
  fDACSpinBoxes[6] = DAC_spinBox_06;
  fDACSpinBoxes[7] = DAC_spinBox_07;
  fDACSpinBoxes[8] = DAC_spinBox_08;
  fDACSpinBoxes[9] = DAC_spinBox_09;
  fDACSpinBoxes[10] = DAC_spinBox_10;
  fDACSpinBoxes[11] = DAC_spinBox_11;
  fDACSpinBoxes[12] = DAC_spinBox_12;
  fDACSpinBoxes[13] = DAC_spinBox_13;
  fDACSpinBoxes[14] = DAC_spinBox_14;
  fDACSpinBoxes[15] = DAC_spinBox_15;

  fADCLineEdit[0] = ADC_Value_00;
  fADCLineEdit[1] = ADC_Value_01;
  fADCLineEdit[2] = ADC_Value_02;
  fADCLineEdit[3] = ADC_Value_03;
  fADCLineEdit[4] = ADC_Value_04;
  fADCLineEdit[5] = ADC_Value_05;
  fADCLineEdit[6] = ADC_Value_06;
  fADCLineEdit[7] = ADC_Value_07;
  fADCLineEdit[8] = ADC_Value_08;
  fADCLineEdit[9] = ADC_Value_09;
  fADCLineEdit[10] = ADC_Value_10;
  fADCLineEdit[11] = ADC_Value_11;
  fADCLineEdit[12] = ADC_Value_12;
  fADCLineEdit[13] = ADC_Value_13;
  fADCLineEdit[14] = ADC_Value_14;
  fADCLineEdit[15] = ADC_Value_15;

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
  GetSFPChainSetup();
  if(!AssertChainConfigured()) return;
  GetRegisters ();
  RefreshView ();
}

void FebexGui::ApplyBtn_clicked ()
{
//std::cout << "FebexGui::ApplyBtn_clicked()"<< std::endl;


  EvaluateSlave ();

// JAM maybe disable confirm window ?
//  char buffer[1024];
//  snprintf (buffer, 1024, "Really apply FEBEX settings  to SFP %d Device %d?", fSFP, fSlave);
//  if (QMessageBox::question (this, "FEBEX GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
//      != QMessageBox::Yes)
//  {
//    return;
//  }


  GetSFPChainSetup();
  if(AssertNoBroadcast(false) && !AssertChainConfigured()) return;

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
    FEBEX_BROADCAST_ACTION(SaveRegisters); // refresh actual setup from hardware and write it to open file
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

void FebexGui::ResetBoardBtn_clicked ()
{
//std::cout << "FebexGui::ResetBoardBtn_clicked"<< std::endl;
  if (QMessageBox::question (this, "FEBEX GUI", "Really Reset gosip on pex board?", QMessageBox::Yes | QMessageBox::No,
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

void FebexGui::ResetSlaveBtn_clicked ()
{
  char buffer[1024];
  EvaluateSlave ();
  snprintf (buffer, 1024, "Really initialize FEBEX device at SFP %d, Slave %d ?", fSFP, fSlave);
  if (QMessageBox::question (this, "Febex GUI", QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }
  FEBEX_BROADCAST_ACTION(InitFebex);


}
void FebexGui::EnableI2C ()
{
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000080);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2000020);
}

void FebexGui::DisableI2C ()
{
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000000);
}


void FebexGui::InitFebex()
{
  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, 0x00);
  usleep (4000);

//        // disable test data length
  WriteGosip (fSFP, fSlave, REG_DATA_LEN, 0x10000000);

//        // specify trace length in slices
  WriteGosip (fSFP, fSlave, REG_FEB_TRACE_LEN, FEB_TRACE_LEN);
  // note: we skip verify read here to let this work in broadcast mode!




//        // specify trigger delay in slices
  WriteGosip (fSFP, fSlave,  REG_FEB_TRIG_DELAY, FEB_TRIG_DELAY);
  // note: we skip verify read here to let this work in broadcast mode!


  //        // disable trigger acceptance in febex
  WriteGosip (fSFP, fSlave,   REG_FEB_CTRL, 0);


  //        // enable trigger acceptance in febex
  WriteGosip (fSFP, fSlave,   REG_FEB_CTRL, 1);


//        // set channels used for self trigger signal

  // JAM: the following is reduced version of mbs sample code. instead of arrays for each slave, we just
  // take settings for first device at sfp 0. Should be sufficient for baseline setup until mbs configures all?
  long l_sfp0_feb_ctrl0= 0x01000000;
  long l_sfp0_feb_ctrl1 = 0x0;
  long l_sfp0_feb_ctrl2= 0xffff;
  long l_ev_od_or = (l_sfp0_feb_ctrl0 >> 20) & 0x1;
  long l_pol = (l_sfp0_feb_ctrl0 >> 28) & 0x1;
  long l_trig_mod = (l_sfp0_feb_ctrl0 >> 24) & 0xf;
  long l_dis_cha  =  l_sfp0_feb_ctrl0       & 0x1ffff;
  long l_dat_redu =  l_sfp0_feb_ctrl1       & 0x1ffff;
  long l_ena_trig =  l_sfp0_feb_ctrl2       & 0xffff;


  int trigenabchan= ((l_ev_od_or<<21)|(l_pol<<20)|(l_trig_mod<<16)|l_ena_trig);
  WriteGosip (fSFP, fSlave,  REG_FEB_SELF_TRIG, trigenabchan);



//        // set the step size for self trigger and data reduction
  long l_thresh=0x1ff;
  for (int l_k=0; l_k < FEBEX_CH ; l_k++){
    WriteGosip (fSFP, fSlave, REG_FEB_STEP_SIZE, ( l_k<<24 ) | l_thresh );
  }




  //
//        // reset the time stamp and set the clock source for time stamp counter
  if(fSFP==0 && fSlave==0) // assume clock source at first slave on first chain here
  {
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x0 );
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x7 );
  }
  else
  {
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x0 );
    WriteGosip (fSFP, fSlave,   REG_FEB_TIME,0x5 );
  }




  //
//        // enable/disable no hit in trace data suppression of channel
  WriteGosip (fSFP, fSlave,  REG_DATA_REDUCTION, l_dat_redu);

//        // set channels used for self trigger signal
  WriteGosip (fSFP, fSlave,  REG_MEM_DISABLE, l_dis_cha );

//        // write SFP id for channel header

  WriteGosip (fSFP, fSlave,  REG_HEADER, fSFP);

// JAM the following is redundant due to new macro FEBEX_BROADCAST_ACTION
//  if(AssertNoBroadcast(false))
//  {
//    WriteGosip (fSFP, fSlave,  REG_HEADER, fSFP);
//  }
//  else
//    {
//#ifdef USE_MBSPEX_LIB
//
//    // broadcast mode: find out number of slaves and loop over all registered slaves
//    GetSFPChainSetup();
//    if (fSFP < 0)
//    {
//      for (int sfp = 0; sfp < 4; ++sfp)
//      {
//        int numslaves = fSFPChains.numslaves[sfp];
//        for (int feb = 0; feb < numslaves; ++feb)
//          WriteGosip (sfp, feb, REG_HEADER, sfp);
//      }
//    }
//    else if (fSlave < 0)
//    {
//      int numslaves = fSFPChains.numslaves[fSFP];
//      for (int feb = 0; feb < numslaves; ++feb)
//               WriteGosip (fSFP, feb, REG_HEADER, fSFP);
//    }
//
//
//#else
//      AppendTextWindow("Could not set sfp id in broadcast mode with gosipcmd interface!");
//      return;
//#endif
//    }
//////////////////////////////////////////////////// end old code

//        // set trapez parameters for trigger/hit finding
  WriteGosip (fSFP, fSlave,  TRIG_SUM_A_REG, TRIG_SUM_A);
  WriteGosip (fSFP, fSlave,   TRIG_GAP_REG, TRIG_SUM_A + TRIG_GAP);
  WriteGosip (fSFP, fSlave,   TRIG_SUM_B_REG, TRIG_SUM_A  + TRIG_GAP + TRIG_SUM_B );


#ifdef ENABLE_ENERGY_FILTER
#ifdef TRAPEZ
//
//        // set trapez parameters for energy estimation
  WriteGosip (fSFP, fSlave,  ENERGY_SUM_A_REG, ENERGY_SUM_A);
  WriteGosip (fSFP, fSlave,  ENERGY_GAP_REG, ENERGY_SUM_A + ENERGY_GAP);
  WriteGosip (fSFP, fSlave,  ENERGY_SUM_B_REG, ENERGY_SUM_A  + ENERGY_GAP + ENERGY_SUM_B );


#endif // TRAPEZ
#endif // ENABLE_ENERGY_FILTER
  usleep(50);
// enabling after "ini" of all registers (Ivan - 16.01.2013):
  WriteGosip (fSFP, fSlave,   DATA_FILT_CONTROL_REG, DATA_FILT_CONTROL_DAT);
  sleep (1);
  printm("Did Initialize FEBEX for SFP %d Slave %d",fSFP,fSlave);
}



void FebexGui::AutoAdjustBtn_clicked ()
{
  //std::cout <<"AutoAdjustBtn_clicked "<< std::endl;
  EvaluateSlave ();
  QApplication::setOverrideCursor (Qt::WaitCursor);
  FEBEX_BROADCAST_ACTION(AutoAdjust);
  QApplication::restoreOverrideCursor ();
}


void FebexGui::AutoAdjust()
{
  if(!AssertChainConfigured()) return;
   QString targetstring=ADCAdjustValue->text ();
   unsigned targetvalue =targetstring.toUInt (0, fNumberBase);
   //std::cout <<"string="<<targetstring.toLatin1 ().constData ()<< ", targetvalue="<< targetvalue<< std::endl;
   for(int channel=0; channel<16;++channel)
     {
       if(fBaselineBoxes[channel]->isChecked())
       {
           int dac=AdjustBaseline(channel,targetvalue);
           fDACSpinBoxes[channel]->setValue (dac);
           printm("--- Auto adjusted baselines of sfp:%d FEBEX:%d channel:%d to value:%d =>%d permille DAC",fSFP, fSlave,channel, targetvalue, dac);
       }
    }
   if (!checkBox_AA->isChecked ())
     RefreshView(); // in auto apply mode the baselines are automatically displayed, without this we have to get it again
   else
     RefreshStatus();
}



int FebexGui::AdjustBaseline(int channel, int adctarget)
{
  int dac=500; // dac setup in per mille here, start in the middle
  int dacstep=250;
  int validdac=-1;
  int adc=0;
  int escapecounter=10;
  bool upwards=true; // scan direction up or down
  bool changed=false; // do we have changed scan direction?
  bool initial=true; // supress evaluation of scan direction at first cycle
  //std::cout << "FebexGui::AdjustBaseline of channel "<<channel<<" to "<<adctarget<< std::endl;

  double resolution= 1.0/FEBEX_MCP433_MAXVAL * 0xFFF /2; // for 12 bit
    // test if FEBEX is for 14 bit values:
  if(autoApply(channel, 1000)>0xFFF)
        resolution*=4;
  do{
     adc=autoApply(channel, dac); // this gives already mean value of 3 adc samples
     if(adc<0) break; // avoid broadcast
     validdac=dac;
     //std::cout << "FebexGui::AdjustBaseline after autoApply of dac:"<<dac<<" gets adc:"<<adc<<", resolution:"<<resolution<< std::endl;
     if(adc<adctarget){
       dac+=dacstep;
       changed=(!upwards ? true : false);
       upwards=true;
       if(dac>1000) dac = 1000;
     }
     else if (adc>adctarget){
       dac-=dacstep;
       changed=(upwards ? true : false);
       upwards=false;
       if(dac<0) dac=0;
     }
     else break;
     if(changed && !initial && dacstep > 1) dacstep = dacstep >> 1;
     if(dacstep <1) break;
     if(!changed || dacstep==1) escapecounter--; // get us out of loop if user wants to reach value outside adc range, or if we oscillate around target value
     initial=false;
  } while (fabs(adc-adctarget) >= resolution && escapecounter);
  //std::cout << "   FebexGui::AdjustBaseline after loop dac:"<<validdac<<" adc:"<<adc<<", resolution:"<<resolution<< std::endl;
  return validdac;
}







void FebexGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "FebexGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
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

void FebexGui::DumpADCs()
{
  // JAM 2016 first demonstration how to get the actual adc values:
  if(!AssertChainConfigured()) return;

    printm("SFP %d DEV:%d :)",fSFP, fSlave);
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



void FebexGui::DumpBtn_clicked ()
{
//std::cout << "FebexGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
  EvaluateSlave ();
//  int oldslave = fSlave;
//  int oldchan = fSFP;

  AppendTextWindow ("--- ADC Dump ---:");

  FEBEX_BROADCAST_ACTION(DumpADCs);


//  if (AssertNoBroadcast ())
//  {
//    DumpADCs ();    // just dump actually set FEBEX
//  }
//  else if (fSFP < 0)
//  {
//    for (int sfp = 0; sfp < 4; ++sfp)
//    {
//      if (fSFPChains.numslaves[sfp] == 0)
//        continue;
//      fSFP = sfp;
//
//      if (fSlave < 0)
//      {
//        for (int feb = 0; feb < fSFPChains.numslaves[sfp]; ++feb)
//        {
//          fSlave = feb;
//          DumpADCs ();
//        }
//      }
//      else
//      {
//        DumpADCs ();
//      }
//
//    }    // for sfp
//  }
//  else if (fSlave< 0)
//  {
//    for (int feb = 0; feb < fSFPChains.numslaves[fSFP]; ++feb)
//        {
//          fSlave = feb;
//          DumpADCs ();
//        }
//  }
//  else
//  {
//    AppendTextWindow ("--- NEVER COME HERE: ADC dump with unexpected broadcast ---:");
//  }
//
//  fSlave= oldslave;
//  fSFP= oldchan;

}

void FebexGui::ClearOutputBtn_clicked ()
{
//std::cout << "FebexGui::ClearOutputBtn_clicked()"<< std::endl;
  TextOutput->clear ();
  TextOutput->setPlainText ("Welcome to FEBEX GUI!\n\t v0.83 of 8-March-2016 by Armin Entezami and JAM (j.adamczewski@gsi.de)\n");

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
  char buffer[1024];
  // JAM: need to increase default bus wait time to 900us first for febex i2c!
  snprintf (buffer, 1024, "./setGosipwait.sh 900"); // output redirection inside QProcess does not work, use helper script
  QString tcom (buffer);
  QString tresult=ExecuteGosipCmd (tcom, 10000);
  AppendTextWindow (tresult);
  QString fileName = flst[0];
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

void FebexGui::DebugBox_changed (int on)
{
//std::cout << "DebugBox_changed to "<< on << std::endl;
  fDebug = on;
}

void FebexGui::HexBox_changed (int on)
{

  unsigned adjustvalue =ADCAdjustValue->text ().toUInt (0, fNumberBase); // save value in auto adjust field
  fNumberBase = (on ? 16 : 10);
//std::cout << "HexBox_changed set base to "<< fNumberBase << std::endl;

  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";
  ADCAdjustValue->setText (pre+text.setNum (adjustvalue, fNumberBase)); // recover with new base

  RefreshView ();

}

void FebexGui::Slave_changed (int)
{
  //std::cout << "FebexGui::Slave_changed" << std::endl;
  EvaluateSlave ();
  bool refreshable = AssertNoBroadcast (false);
  RefreshButton->setEnabled (refreshable);

  RefreshChains();
  if(checkBox_AA->isChecked() && refreshable)
    
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

 void FebexGui::DAC_spinBox_all_changed(int val)
{
  //std::cout << "FebexGui::DAC_spinBox_all_changed, val="<<val << std::endl;
   for(int chan=0;chan<16;++chan)
     fDACSpinBoxes[chan]->setValue (val);
  
}


 void FebexGui::DAC_spinBox_changed (int channel, int val)
{
  if (checkBox_AA->isChecked () && AssertNoBroadcast (false))
  {
    QString text;
    QString pre;
    fNumberBase == 16 ? pre = "0x" : pre = "";
    EvaluateSlave ();
    //int permille = fDACSpinBoxes[channel]->value ();
    int permille =val;
    int Adc = autoApply (channel, permille);

    fADCLineEdit[channel]->setText (pre + text.setNum (Adc, fNumberBase));
    RefreshStatus ();
  }

}


 void FebexGui::Any_spinBox00_changed(int val)
{
   DAC_spinBox_changed (0,val);
 }

void FebexGui::Any_spinBox01_changed(int val)
{
  DAC_spinBox_changed (1,val);
}

void FebexGui::Any_spinBox02_changed(int val)
{
  DAC_spinBox_changed (2,val);
}

void FebexGui::Any_spinBox03_changed(int val)
{
  DAC_spinBox_changed (3,val);
}

void FebexGui::Any_spinBox04_changed(int val)
{
  DAC_spinBox_changed (4,val);
}

void FebexGui::Any_spinBox05_changed(int val)
{
  DAC_spinBox_changed (5,val);
}

void FebexGui::Any_spinBox06_changed(int val)
{
  DAC_spinBox_changed (6,val);
}

void FebexGui::Any_spinBox07_changed(int val)
{
  DAC_spinBox_changed (7,val);
}

void FebexGui::Any_spinBox08_changed(int val)
{
  DAC_spinBox_changed (8,val);
}

void FebexGui::Any_spinBox09_changed(int val)
{
  DAC_spinBox_changed (9,val);
}

void FebexGui::Any_spinBox10_changed(int val)
{
  DAC_spinBox_changed (10,val);
}

void FebexGui::Any_spinBox11_changed(int val)
{
  DAC_spinBox_changed (11,val);
}

void FebexGui::Any_spinBox12_changed(int val)
{
  DAC_spinBox_changed (12,val);
}

void FebexGui::Any_spinBox13_changed(int val)
{
  DAC_spinBox_changed (13,val);
}

void FebexGui::Any_spinBox14_changed(int val)
{
  DAC_spinBox_changed (14,val);
}

void FebexGui::Any_spinBox15_changed(int val)
{
  DAC_spinBox_changed (15,val);
}



int FebexGui::autoApply(int channel, int dac)

{ 
  int dacchip,dacchannel, adcchip, adcchannel;
  int value=255-round((dac*255.0/1000.0)) ;
  dacchip= channel/FEBEX_MCP433_NUMCHAN ;
  dacchannel= channel-dacchip*FEBEX_MCP433_NUMCHAN;
  fSetup[fSFP].at(fSlave).SetDACValue(dacchip,dacchannel, value);
   
   EnableI2C ();  
   WriteDAC_FebexI2c (dacchip, dacchannel, fSetup[fSFP].at(fSlave).GetDACValue(dacchip, dacchannel));
   DisableI2C ();
   if (!AssertNoBroadcast ())
      return -1;
   int Adc=AcquireBaselineSample(channel);
   return Adc;
  
}


int FebexGui::AcquireBaselineSample(uint8_t febexchan)
{
  if(febexchan >= FEBEX_ADC_NUMADC*FEBEX_ADC_NUMCHAN) return -1;
  int adcchip= febexchan/FEBEX_ADC_NUMCHAN;
  int adcchannel= febexchan-adcchip * FEBEX_ADC_NUMCHAN ;
  int Adc=0;
  for(int t=0; t<FEBEX_ADC_BASELINESAMPLES;++t)
    {
      Adc+=ReadADC_Febex(adcchip,adcchannel);
    }
  Adc=Adc/FEBEX_ADC_BASELINESAMPLES;
  return Adc;
}







void FebexGui::RefreshView ()
{
// display setup structure to gui:
  QString text;
  QString pre;
  fNumberBase == 16 ? pre = "0x" : pre = "";

  // JAM improved this by looping over spinbox references
  for(int channel=0; channel<16;++channel)
     {
          int val=fSetup[fSFP].at(fSlave).GetDACValue(channel);
          int permille=1000 - round((val*1000.0/255.0)) ;
          fDACSpinBoxes[channel]->setValue(permille);
          int adc=AcquireBaselineSample(channel);
          fADCLineEdit[channel]->setText (pre+text.setNum (adc, fNumberBase));
     }

  RefreshChains();
  RefreshStatus();

}


void FebexGui::RefreshStatus ()
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


void FebexGui::RefreshChains ()
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


void FebexGui::EvaluateView ()
{
  // here the current gui display is just copied to setup structure in local memory


// JAM improved this by looping over spinbox references
  for(int channel=0; channel<16;++channel)
     {
          int permille=fDACSpinBoxes[channel]->value();
          int value=255 - round((permille*255.0/1000.0)) ;
          fSetup[fSFP].at(fSlave).SetDACValue(channel, value);
     }

}

void FebexGui::EvaluateSlave ()
{
  if(fBroadcasting) return;
  fSFP = SFPspinBox->value ();
  fSlave = SlavespinBox->value ();

}

void FebexGui::SetRegisters ()
{
  QApplication::setOverrideCursor (Qt::WaitCursor);
  EnableI2C ();    // must be done since mbs setup program may shut i2c off at the end

      for (int m = 0; m < FEBEX_MCP433_NUMCHIPS; ++m)
    {
      for (int c = 0; c < FEBEX_MCP433_NUMCHAN; ++c)
       {
          WriteDAC_FebexI2c (m, c, fSetup[fSFP].at(fSlave).GetDACValue(m, c));
       }
    }

  DisableI2C ();
  QApplication::restoreOverrideCursor ();

}

void FebexGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  QApplication::setOverrideCursor (Qt::WaitCursor);
 EnableI2C ();
 for (int m = 0; m < FEBEX_MCP433_NUMCHIPS; ++m)
    {
      for (int c = 0; c < FEBEX_MCP433_NUMCHAN; ++c)
       {
         
	  int val=ReadDAC_FebexI2c (m, c); 
	  //std::cout<<"GetRegisters val="<<val<<std::endl;
  
	  if(val<0){
	  AppendTextWindow("GetRegisters has error!");
	  return; // TODO error message 
	  }
	  fSetup[fSFP].at(fSlave).SetDACValue(m, c,val);
	 
       }
    }
  DisableI2C ();
  QApplication::restoreOverrideCursor ();
}


void FebexGui::SaveRegisters()

{
   GetRegisters(); // refresh actual setup from hardware
   fSaveConfig = true;    // switch to file output mode
   SetRegisters();    // register settings are written to file
   fSaveConfig = false;
}

void FebexGui::GetSFPChainSetup()
{
//  std::cout<<"GetSFPChainSetup... "<< std::endl;
#ifdef USE_MBSPEX_LIB
    // broadcast mode: find out number of slaves and loop over all registered slaves
    mbspex_get_configured_slaves(fPexFD, &fSFPChains);

    // dynamically increase array of setup structures:
    for(int sfp=0; sfp<4; ++sfp)
    {
      while(fSetup[sfp].size()<fSFPChains.numslaves[sfp])
      {
        fSetup[sfp].push_back(FebexSetup());
        //std::cout<<"GetSFPChainSetup increased setup at sfp "<<sfp<<" to "<<fSetup[sfp].size()<<" slaves." << std::endl;
      }
    }
#endif
  
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
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat); // first send read request address
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, FEBEX_MCP433_REQUEST_READ); // read request command

  val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1); // read out the value
  if(val < 0) return val; // error case, propagate it upwards
  return (val & 0xFF); // mask to use only l.s. byte
}



int  FebexGui::ReadADC_Febex (uint8_t adc, uint8_t chan)
{
  if(adc>FEBEX_ADC_NUMADC || chan > FEBEX_ADC_NUMCHAN) return -1;

  int val=0;
  int dat=(adc << 3) + chan; //l_wr_d  = (l_k*4) + l_l;

  WriteGosip (fSFP, fSlave, FEBEX_ADC_PORT, dat); // first specify channel number

  val = ReadGosip (fSFP, fSlave, FEBEX_ADC_PORT); // read back the value

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

  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
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
  if (fSFP < 0 || fSlave < 0)
  {
    //std::cerr << "# FebexGui Error: broadcast not supported here!" << std::endl;
    if (verbose)
      AppendTextWindow ("#Error: broadcast not supported here!");
    return false;
  }
  return true;
}

bool FebexGui::AssertChainConfigured (bool verbose)
{
  if (fSlave >= fSFPChains.numslaves[fSFP])
  {
    if (verbose)
      printm("#Error: device index %d not in initialized chain of length %d at SFP %d",fSlave,fSFPChains.numslaves[fSFP],fSFP);
    return false;
  }
  return true;
}
