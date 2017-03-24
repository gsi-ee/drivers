#include "PolandGui.h"

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
PolandGui* PolandGui::fInstance = 0;

#include <stdarg.h>

/** JAM The following nice define handles all explicit broadcast actions depending on the currently set slave*/
#define POLAND_BROADCAST_ACTION(X) \
fBroadcasting=true;  \
int oldslave = fSlave; \
int oldchan = fSFP; \
if (AssertNoBroadcast (false)) \
 { \
   fBroadcasting=false;\
   X; \
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
         X; \
       } \
     } \
     else \
     { \
       X;\
     }\
   }\
 } \
 else if (fSlave< 0) \
 { \
   for (int feb = 0; feb < fSFPChains.numslaves[fSFP]; ++feb) \
       { \
         fSlave = feb; \
         X; \
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
  PolandGui::fInstance->AppendTextWindow (c_str);
  PolandGui::fInstance->FlushTextWindow();
  va_end(args);
}

#ifdef USE_MBSPEX_LIB
/** this one is used to speed down direct mbspex io:*/
void PolandGui::I2c_sleep ()
{
  //usleep(300);

  usleep(900); // JAM2016 need to increase wait time since some problems with adc read?
}

#endif




// *********************************************************

/*
 *  Constructs a PolandGui which is a child of 'parent', with the
 *  name 'name'.'
 */
PolandGui::PolandGui (QWidget* parent) :
    QWidget (parent), fDebug (false), fSaveConfig(false), fBroadcasting(false),fSFP (0), fSlave (0), fSFPSave (0), fSlaveSave (0), fTriggerOn(true), fConfigFile(NULL)
{
  setupUi (this);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  fEnv = QProcessEnvironment::systemEnvironment ();    // get PATH to gosipcmd from parent process
#endif


  fImplementationName="POLAND";
  fVersionString="Welcome to POLAND GUI!\n\t v0.83 of 24-March-2017 by JAM (j.adamczewski@gsi.de)";


  fNumberBase=10;

 	memset( &fSFPChains, 0, sizeof(struct pex_sfp_links));

  for(int sfp=0; sfp<4;++sfp)
    fSetup[sfp].clear();


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
  QObject::connect (OffsetButton, SIGNAL (clicked ()), this, SLOT (OffsetBtn_clicked ()));
  QObject::connect (TriggerButton, SIGNAL (clicked ()), this, SLOT (TriggerBtn_clicked ()));

QObject::connect(DebugBox, SIGNAL(stateChanged(int)), this, SLOT(DebugBox_changed(int)));
QObject::connect(HexBox, SIGNAL(stateChanged(int)), this, SLOT(HexBox_changed(int)));
QObject::connect(SFPspinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));
QObject::connect(SlavespinBox, SIGNAL(valueChanged(int)), this, SLOT(Slave_changed(int)));

QObject::connect(DACModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(DACMode_changed(int)));

// JAM2017: some more signals for the autoapply feature:

QObject::connect(ModeComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(QFW_changed()));
QObject::connect (MasterTriggerBox, SIGNAL(stateChanged(int)), this, SLOT (QFW_changed()));
QObject::connect (InternalTriggerBox, SIGNAL(stateChanged(int)), this, SLOT (QFW_changed()));
QObject::connect (FesaModeBox, SIGNAL(stateChanged(int)), this, SLOT (QFW_changed()));

QObject::connect (TSLoop1lineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (TSLoop2lineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (TSLoop3lineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (TS1TimelineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (TS2TimelineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));
QObject::connect (TS3TimelineEdit, SIGNAL(returnPressed()),this,SLOT (QFW_changed()));

QObject::connect (DACStartValueLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DACOffsetLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DACDeltaOffsetLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DACCalibTimeLineEdit, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));

QObject::connect (DAClineEdit_1, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_2, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_3, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_4, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_5, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_6, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_7, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_8, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_9, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_10, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_11, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_12, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_13, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_14, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_15, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_16, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_17, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_18, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_19, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_20, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_21, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_22, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_23, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_24, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_25, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_26, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_27, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_28, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_29, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_30, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_31, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));
QObject::connect (DAClineEdit_32, SIGNAL(returnPressed()),this,SLOT (DAC_changed()));

QObject::connect (FanDial, SIGNAL(valueChanged(int)),this,SLOT (Fan_changed()));


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

  GetSFPChainSetup(); // ensure that any slave has a status structure before we begin clicking...

   show();
}

PolandGui::~PolandGui ()
{
#ifdef USE_MBSPEX_LIB
  mbspex_close (fPexFD);
#endif
}

void PolandGui::ShowBtn_clicked ()
{
//std::cout << "PolandGui::ShowBtn_clicked()"<< std::endl;
  EvaluateSlave ();
  GetSFPChainSetup();

  if(!AssertNoBroadcast(false)) return;
  if(!AssertChainConfigured()) return;
  GetRegisters ();
  RefreshView ();
}

void PolandGui::ApplyBtn_clicked ()
{
//std::cout << "PolandGui::ApplyBtn_clicked()"<< std::endl;



  EvaluateSlave ();
  if (!checkBox_AA->isChecked ())
  {
    char buffer[1024];
    char description[32];
    (QFW_DAC_tabWidget->currentIndex()==0) ? snprintf (description, 32, "QFW") : snprintf (description, 32, "DAC");

    snprintf (buffer, 1024, "Really apply %s settings  to SFP %d Device %d?", description, fSFP, fSlave);
    if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No,
        QMessageBox::Yes) != QMessageBox::Yes)
    {
      return;
    }
  }
GetSFPChainSetup();

// JAM2017: this statement is not useful, will exclude broadcasting mode
//if(AssertNoBroadcast(false) && !AssertChainConfigured()) return;

  // JAM: since we keep all slave set ups in vector/array, we must handle broadcast mode explicitely
  // no implicit driver broadcast via -1 indices anymore!
  POLAND_BROADCAST_ACTION(ApplyGUISettings());


}







void PolandGui::ApplyGUISettings()
{
  // depending on activated view, we either set qfw parameters or change DAC programming
if(QFW_DAC_tabWidget->currentIndex()==0)
{
  ApplyQFWSettings();

}
else if (QFW_DAC_tabWidget->currentIndex()==1)
{
  ApplyDACSettings();
}
else if (QFW_DAC_tabWidget->currentIndex()==2)
{
  ApplyFanSettings();
}
}


void PolandGui::ApplyFanSettings()
{
  EvaluateFans(); // from gui to memory
  SetFans ();
}


void PolandGui::ApplyQFWSettings()
{
  EvaluateView(); // from gui to memory
  SetRegisters ();
}

void PolandGui::ApplyDACSettings()
{
  EvaluateDAC();
  ApplyDAC();
}

void PolandGui::QFW_changed ()
{
  //std::cout << "PolandGui::QFW_changed()"<< std::endl;
  if(!checkBox_AA->isChecked()) return;
  EvaluateSlave ();
  POLAND_BROADCAST_ACTION(ApplyQFWSettings());

}

void PolandGui::DAC_changed ()
{
  //std::cout << "PolandGui::DAC_changed()"<< std::endl;
  if(!checkBox_AA->isChecked()) return;
  EvaluateSlave ();
  POLAND_BROADCAST_ACTION(ApplyDACSettings());
}

void PolandGui::Fan_changed ()
{
  //std::cout << "PolandGui::Fan_changed()"<< std::endl;
  if(!checkBox_AA->isChecked()) return;
  EvaluateSlave ();
  POLAND_BROADCAST_ACTION(ApplyFanSettings());

  // for autoapply refresh fan readout immediately:
  if(AssertNoBroadcast(false))
  {
    GetSensors();
    RefreshSensors();
  }
}





void PolandGui::InitChainBtn_clicked ()
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

void PolandGui::ResetBoardBtn_clicked ()
{
//std::cout << "PolandGui::ResetBoardBtn_clicked"<< std::endl;
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

void PolandGui::ResetSlaveBtn_clicked ()
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
  POLAND_BROADCAST_ACTION(ResetPoland());
}

void PolandGui::ResetPoland ()
{
 WriteGosip (fSFP, fSlave, POLAND_REG_RESET, 0);
 WriteGosip (fSFP, fSlave, POLAND_REG_RESET, 1);
 printm("Did reset POLAND for SFP %d Slave %d",fSFP,fSlave);
}




void PolandGui::OffsetBtn_clicked ()
{
char buffer[1024];
EvaluateSlave ();
snprintf (buffer, 1024, "Really scan offset for SFP chain %d, Slave %d ?", fSFP, fSlave);
if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
    != QMessageBox::Yes)
{
  //std::cout <<"QMessageBox does not return yes! "<< std::endl;
  return;
}

POLAND_BROADCAST_ACTION(ScanOffsets());
}


void PolandGui::ScanOffsets ()
{
  char buffer[1024];
  WriteGosip (fSFP, fSlave, POLAND_REG_DO_OFFSET, 1);
  printm("--- SFP %d Poland %d : Doing offset measurement... ",fSFP, fSlave);
  QApplication::setOverrideCursor( Qt::WaitCursor );

  WriteGosip (fSFP, fSlave, POLAND_REG_DO_OFFSET, 0);
  sleep(2);
  AppendTextWindow ("    ... done. Dumping offset values:");

  snprintf (buffer, 1024, "gosipcmd -d -r -x -- %d %d 0x%x 0x%x", fSFP, fSlave, POLAND_REG_OFFSET_BASE, 32);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
  QApplication::restoreOverrideCursor();

}



void PolandGui::TriggerBtn_clicked ()
{
  //std::cout << "PolandGui::TriggerBtn_clicked"<< std::endl;
  char buffer[1024];
  fTriggerOn ?   snprintf (buffer, 1024, "Really disable Frontend Trigger acceptance?") : snprintf (buffer, 1024, "Really enable Frontend Trigger acceptance?");

  if (QMessageBox::question (this, fImplementationName, QString (buffer), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
      != QMessageBox::Yes)
  {
    //std::cout <<"QMessageBox does not return yes! "<< std::endl;
    return;
  }

  fTriggerOn ? fTriggerOn=false: fTriggerOn=true;


  int value = (fTriggerOn ? 1 : 0);
  QString state= (fTriggerOn ? "ON" : "OFF");
  WriteGosip (-1, -1, POLAND_REG_TRIG_ON, value); // send broadcast of register to all slaves.
  QString msg="--- Set all devices trigger acceptance to ";
  AppendTextWindow (msg+state);
  RefreshTrigger();


}

void PolandGui::RefreshTrigger()
{
   // todo: modify label
  QString labelprefix="<html><head/><body><p>Trigger";
  QString labelstate = fTriggerOn ? " <span style=\" font-weight:600; color:#00ff00;\">ON </span></p></body></html>" :
      " <span style=\" font-weight:600; color:#ff0000;\">OFF</span></p></body></html>" ;
  TriggerLabel->setText(labelprefix+labelstate);

}


void PolandGui::BroadcastBtn_clicked (bool checked)
{
//std::cout << "PolandGui::BroadcastBtn_clicked with checked="<<checked<< std::endl;
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

void PolandGui::DumpBtn_clicked ()
{
//std::cout << "PolandGui::DumpBtn_clicked"<< std::endl;
// dump register contents from gosipcmd into TextOutput (QPlainText)
EvaluateSlave ();
AppendTextWindow ("--- Register Dump ---:");
POLAND_BROADCAST_ACTION(DumpSamples());
}


void PolandGui::DumpSamples ()
{
  char buffer[1024];

  GetRegisters();
  printm("dump registers of sfp:%d device %d",fSFP,fSlave);
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);
  int numwords = 32 + theSetup.fSteps[0] * 32 + theSetup.fSteps[1] * 32 + theSetup.fSteps[2] * 32 + 32;
  // todo: note this will not work for broadcast mode or if show was not clicked before
  // we can live with it for the moment.

  snprintf (buffer, 1024, "gosipcmd -d -r -x -- %d %d 0 0x%x", fSFP, fSlave, numwords);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  AppendTextWindow (result);
}


void PolandGui::ClearOutputBtn_clicked ()
{
//std::cout << "PolandGui::ClearOutputBtn_clicked()"<< std::endl;
TextOutput->clear ();
TextOutput->setPlainText (fVersionString);

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
RefreshChains();
 //if(checkBox_AA->isChecked() && refreshable)
 if(triggerchangeable)
 {
   // JAM note that we had a problem of prelling spinbox here (arrow buttons only, keyboard arrows are ok)
   // probably caused by too long response time of this slot?
   // workaround is to refresh the view delayed per single shot timer:
   //std::cout << "Timer started" << std::endl;
   QTimer::singleShot(10, this, SLOT(ShowBtn_clicked()));
   //std::cout << "Timer end" << std::endl;
 }







}


void PolandGui::DACMode_changed(int ix)
{
  EvaluateSlave ();
  //std::cout << "PolandGui::DACMode_changed to index:"<< ix << std::endl;
  //std::cout << "PolandGui::DACMode_changed with sfp:"<< fSFP<<", slave:"<<fSlave << std::endl;
  if(AssertNoBroadcast (false))
  {
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);
  theSetup.fDACMode= ix+1;
  //GetRegisters();
  RefreshDAC();
  }

  if(checkBox_AA->isChecked())

    QTimer::singleShot(10, this, SLOT(DAC_changed())); // again we delay to avoid prelling signals?


}


void PolandGui::RefreshChains ()
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


void PolandGui::RefreshStatus ()
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



void PolandGui::RefreshView ()
{
// display setup structure to gui:
QString text;
QString pre;
fNumberBase==16? pre="0x" : pre="";
PolandSetup& theSetup = fSetup[fSFP].at (fSlave);

RefreshMode();


TSLoop1lineEdit->setText (pre+text.setNum (theSetup.fSteps[0], fNumberBase));
TSLoop2lineEdit->setText (pre+text.setNum (theSetup.fSteps[1], fNumberBase));
TSLoop3lineEdit->setText (pre+text.setNum (theSetup.fSteps[2], fNumberBase));
TS1TimelineEdit->setText (text.setNum (theSetup.GetStepTime(0)));
TS2TimelineEdit->setText (text.setNum (theSetup.GetStepTime(1)));
TS3TimelineEdit->setText (text.setNum (theSetup.GetStepTime(2)));
MasterTriggerBox->setChecked (theSetup.IsTriggerMaster ());
FesaModeBox->setChecked (theSetup.IsFesaMode ());
InternalTriggerBox->setChecked (theSetup.IsInternalTrigger ());


EventCounterNumber->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter1->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter2->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter3->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter4->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter5->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter6->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter7->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
ErrorCounter8->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);


EventCounterNumber->display ((int) theSetup.fEventCounter);
ErrorCounter1->display ((int) theSetup.fErrorCounter[0]);
ErrorCounter2->display ((int) theSetup.fErrorCounter[1]);
ErrorCounter3->display ((int) theSetup.fErrorCounter[2]);
ErrorCounter4->display ((int) theSetup.fErrorCounter[3]);
ErrorCounter5->display ((int) theSetup.fErrorCounter[4]);
ErrorCounter6->display ((int) theSetup.fErrorCounter[5]);
ErrorCounter7->display ((int) theSetup.fErrorCounter[6]);
ErrorCounter8->display ((int) theSetup.fErrorCounter[7]);

RefreshDACMode();
RefreshDAC(); // probably this is already triggered by signal
RefreshTrigger(); // show real trigger register as read back from actual device

RefreshSensors();
RefreshChains();
RefreshStatus();
}

void PolandGui::EvaluateView ()
{
//EvaluateSlave (); // bug if used in broadcast macro
EvaluateMode  ();

PolandSetup& theSetup = fSetup[fSFP].at (fSlave);

// copy widget values to structure
theSetup.fSteps[0] = TSLoop1lineEdit->text ().toUInt (0, fNumberBase);
theSetup.fSteps[1] = TSLoop2lineEdit->text ().toUInt (0, fNumberBase);
theSetup.fSteps[2] = TSLoop3lineEdit->text ().toUInt (0, fNumberBase);

theSetup.SetStepTime(TS1TimelineEdit->text ().toDouble (),0);
theSetup.SetStepTime(TS2TimelineEdit->text ().toDouble (),1);
theSetup.SetStepTime(TS3TimelineEdit->text ().toDouble (),2);


theSetup.SetTriggerMaster (MasterTriggerBox->isChecked ());
theSetup.SetFesaMode (FesaModeBox->isChecked ());
theSetup.SetInternalTrigger (InternalTriggerBox->isChecked ());

}


void PolandGui::EvaluateFans()
{
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);
  theSetup.SetFanSettings(FanDial->value());

}


void PolandGui::EvaluateMode()
{
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);

  int index=ModeComboBox->currentIndex();
  switch(index)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      theSetup.fQFWMode= index;
      break;
    case 4:
    case 5:
    case 6:
    case 7:
      theSetup.fQFWMode= index -4  + 16;
    break;
    default:
      std::cout << "!!! Never come here - undefined mode index"<< index << std::endl;
    break;
  };
}
void PolandGui::RefreshMode()
{
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);
  int index=-1;
  switch(theSetup.fQFWMode)
  {
    case 0:
    case 1:
    case 2:
    case 3:
      index=theSetup.fQFWMode;
      break;
    case 16:
    case 17:
    case 18:
    case 19:
      index= theSetup.fQFWMode- 16 + 4;
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
  if(fBroadcasting) return;
  fSFP = SFPspinBox->value ();
  fSlave = SlavespinBox->value ();
}



void PolandGui::EvaluateDAC()
{
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);

  theSetup.fDACMode=DACModeComboBox->currentIndex()+1;


  if(theSetup.fDACMode==4)
  {
  theSetup.fDACAllValue=DACStartValueLineEdit->text ().toUInt (0, fNumberBase);

  }
  else
  {
    theSetup.fDACStartValue=DACStartValueLineEdit->text ().toUInt (0, fNumberBase);
  }


  theSetup.fDACOffset=DACOffsetLineEdit->text ().toUInt (0, fNumberBase);
  theSetup.fDACDelta=DACDeltaOffsetLineEdit->text ().toUInt (0, fNumberBase);
  theSetup.SetCalibrationTime(DACCalibTimeLineEdit->text ().toDouble ());



if(theSetup.fDACMode==1)
{
  // only manual mode will refresh DAQ structure here
  theSetup.fDACValue[0]=DAClineEdit_1->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[1]=DAClineEdit_2->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[2]=DAClineEdit_3->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[3]=DAClineEdit_4->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[4]=DAClineEdit_5->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[5]=DAClineEdit_6->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[6]=DAClineEdit_7->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[7]=DAClineEdit_8->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[8]=DAClineEdit_9->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[9]=DAClineEdit_10->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[10]=DAClineEdit_11->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[11]=DAClineEdit_12->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[12]=DAClineEdit_13->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[13]=DAClineEdit_14->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[14]=DAClineEdit_15->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[15]=DAClineEdit_16->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[16]=DAClineEdit_17->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[17]=DAClineEdit_18->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[18]=DAClineEdit_19->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[19]=DAClineEdit_20->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[20]=DAClineEdit_21->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[21]=DAClineEdit_22->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[22]=DAClineEdit_23->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[23]=DAClineEdit_24->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[24]=DAClineEdit_25->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[25]=DAClineEdit_26->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[26]=DAClineEdit_27->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[27]=DAClineEdit_28->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[28]=DAClineEdit_29->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[29]=DAClineEdit_30->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[30]=DAClineEdit_31->text ().toUInt (0, fNumberBase);
theSetup.fDACValue[31]=DAClineEdit_32->text ().toUInt (0, fNumberBase);
}

}


void  PolandGui::ApplyDAC()
{
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);
  QApplication::setOverrideCursor( Qt::WaitCursor );
  WriteGosip (fSFP, fSlave, POLAND_REG_DAC_MODE , theSetup.fDACMode);

  switch((int) theSetup.fDACMode)
  {
    case 1:
      // manual settings:
      for (int d = 0; d < POLAND_DAC_NUM; ++d)
      {
        WriteGosip (fSFP, fSlave, POLAND_REG_DAC_BASE_WRITE + 4 * d, theSetup.fDACValue[d]);
      }
      break;
    case 2:
      // test structure:
      // no more actions needed
      break;
    case 3:
      // issue calibration:
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_STARTVAL , theSetup.fDACStartValue);
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_OFFSET ,  theSetup.fDACOffset);
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_DELTA ,  theSetup.fDACDelta);
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_TIME ,  theSetup.fDACCalibTime);

      break;
    case 4:
      // all same constant value mode:
      WriteGosip (fSFP, fSlave, POLAND_REG_DAC_ALLVAL , theSetup.fDACAllValue);
      break;

    default:
      std::cout << "!!! ApplyDAC Never come here - undefined DAC mode"<<  theSetup.fDACMode << std::endl;
      break;

  };


  WriteGosip (fSFP, fSlave, POLAND_REG_DAC_PROGRAM , 1);
  WriteGosip (fSFP, fSlave, POLAND_REG_DAC_PROGRAM , 0);
  sleep(2);

  QApplication::restoreOverrideCursor();
}

void  PolandGui::RefreshDAC()
{
  //std::cout << "PolandGui::RefreshDAC"<< std::endl;
  QString text;
QString pre;
fNumberBase==16? pre="0x" : pre="";

PolandSetup& theSetup = fSetup[fSFP].at (fSlave);



  DACOffsetLineEdit->setText (pre+text.setNum (theSetup.fDACOffset, fNumberBase));
  DACDeltaOffsetLineEdit->setText (pre+text.setNum (theSetup.fDACDelta, fNumberBase));
  DACCalibTimeLineEdit->setText (text.setNum (theSetup.GetCalibrationTime()));

  DAClineEdit_1->setText (pre+text.setNum (theSetup.fDACValue[0], fNumberBase));
  DAClineEdit_2->setText (pre+text.setNum (theSetup.fDACValue[1], fNumberBase));
  DAClineEdit_3->setText (pre+text.setNum (theSetup.fDACValue[2], fNumberBase));
  DAClineEdit_4->setText (pre+text.setNum (theSetup.fDACValue[3], fNumberBase));
  DAClineEdit_5->setText (pre+text.setNum (theSetup.fDACValue[4], fNumberBase));
  DAClineEdit_6->setText (pre+text.setNum (theSetup.fDACValue[5], fNumberBase));
  DAClineEdit_7->setText (pre+text.setNum (theSetup.fDACValue[6], fNumberBase));
  DAClineEdit_8->setText (pre+text.setNum (theSetup.fDACValue[7], fNumberBase));
  DAClineEdit_9->setText (pre+text.setNum (theSetup.fDACValue[8], fNumberBase));
  DAClineEdit_10->setText (pre+text.setNum (theSetup.fDACValue[9], fNumberBase));
  DAClineEdit_11->setText (pre+text.setNum (theSetup.fDACValue[10], fNumberBase));
  DAClineEdit_12->setText (pre+text.setNum (theSetup.fDACValue[11], fNumberBase));
  DAClineEdit_13->setText (pre+text.setNum (theSetup.fDACValue[12], fNumberBase));
  DAClineEdit_14->setText (pre+text.setNum (theSetup.fDACValue[13], fNumberBase));
  DAClineEdit_15->setText (pre+text.setNum (theSetup.fDACValue[14], fNumberBase));
  DAClineEdit_16->setText (pre+text.setNum (theSetup.fDACValue[15], fNumberBase));
  DAClineEdit_17->setText (pre+text.setNum (theSetup.fDACValue[16], fNumberBase));
  DAClineEdit_18->setText (pre+text.setNum (theSetup.fDACValue[17], fNumberBase));
  DAClineEdit_19->setText (pre+text.setNum (theSetup.fDACValue[18], fNumberBase));
  DAClineEdit_20->setText (pre+text.setNum (theSetup.fDACValue[19], fNumberBase));
  DAClineEdit_21->setText (pre+text.setNum (theSetup.fDACValue[20], fNumberBase));
  DAClineEdit_22->setText (pre+text.setNum (theSetup.fDACValue[21], fNumberBase));
  DAClineEdit_23->setText (pre+text.setNum (theSetup.fDACValue[22], fNumberBase));
  DAClineEdit_24->setText (pre+text.setNum (theSetup.fDACValue[23], fNumberBase));
  DAClineEdit_25->setText (pre+text.setNum (theSetup.fDACValue[24], fNumberBase));
  DAClineEdit_26->setText (pre+text.setNum (theSetup.fDACValue[25], fNumberBase));
  DAClineEdit_27->setText (pre+text.setNum (theSetup.fDACValue[26], fNumberBase));
  DAClineEdit_28->setText (pre+text.setNum (theSetup.fDACValue[27], fNumberBase));
  DAClineEdit_29->setText (pre+text.setNum (theSetup.fDACValue[28], fNumberBase));
  DAClineEdit_30->setText (pre+text.setNum (theSetup.fDACValue[29], fNumberBase));
  DAClineEdit_31->setText (pre+text.setNum (theSetup.fDACValue[30], fNumberBase));
  DAClineEdit_32->setText (pre+text.setNum (theSetup.fDACValue[31], fNumberBase));

  // depending on DAC mode different fields are writable:

  //std::cout << "!!! RefreshDAC With DAC mode="<<  (int)theSetup.fDACMode << std::endl;
  switch((int) theSetup.fDACMode)
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
      DACStartValueLineEdit->setText (pre+text.setNum (theSetup.fDACStartValue, fNumberBase));
      DACOffsetLineEdit->setEnabled(true);
      DACDeltaOffsetLineEdit->setEnabled(true);
      DACCalibTimeLineEdit->setEnabled(true);

          break;
    case 4:
          // all constant
      DACscrollArea->setEnabled(false);
      DACCaliFrame->setEnabled(true);
      DACStartValueLineEdit->setEnabled(true);
      DACStartValueLineEdit->setText (pre+text.setNum (theSetup.fDACAllValue, fNumberBase));
      DACOffsetLineEdit->setEnabled(false);
      DACDeltaOffsetLineEdit->setEnabled(false);
      DACCalibTimeLineEdit->setEnabled(false);

          break;
    default:
      std::cout << "!!! RefreshDAC Never come here - undefined DAC mode"<<  theSetup.fDACMode << std::endl;
      break;

  };



}


void PolandGui::RefreshDACMode()
{
  //std::cout << "PolandGui::RefreshDACMode for mode "<< (int) fSetup.fDACMode << std::endl;
  DACModeComboBox->setCurrentIndex((int) fSetup[fSFP].at (fSlave).fDACMode -1);
}


void PolandGui::SetRegisters ()
{
// write register values from strucure with gosipcmd
  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);
//if (AssertNoBroadcast (false)) // NOTE: after change to broadcast action, this is always true JAM2017
if(!fBroadcasting) // use macro flag instead!
{
  // update trigger modes only in single device
  WriteGosip (fSFP, fSlave, POLAND_REG_INTERNAL_TRIGGER, theSetup.fInternalTrigger);
  WriteGosip (fSFP, fSlave, POLAND_REG_MASTERMODE, theSetup.fTriggerMode);
}

WriteGosip (fSFP, fSlave, POLAND_REG_QFW_MODE, theSetup.fQFWMode);

// following is required to really activate qfw mode (thanks Sven Loechner for fixing):
WriteGosip (fSFP, fSlave, POLAND_REG_QFW_PRG, 1);
WriteGosip (fSFP, fSlave, POLAND_REG_QFW_PRG, 0);



// WriteGosip(fSFP, fSlave, POLAND_REG_TRIGCOUNT, theSetup.fEventCounter);

for (int i = 0; i < POLAND_TS_NUM; ++i)
{
  WriteGosip (fSFP, fSlave, POLAND_REG_STEPS_BASE + 4 * i, theSetup.fSteps[i]);
  WriteGosip (fSFP, fSlave, POLAND_REG_TIME_BASE + 4 * i, theSetup.fTimes[i]);
}
//    for(int e=0; e<POLAND_ERRCOUNT_NUM;++e)
//     {
//       WriteGosip(fSFP, fSlave, POLAND_REG_ERRCOUNT_BASE + 4*e, theSetup.fErrorCounter[e]);
//     }

// TODO: error handling with exceptions?

}

void PolandGui::GetRegisters ()
{
// read register values into structure with gosipcmd

if (!AssertNoBroadcast ())
  return;

PolandSetup& theSetup = fSetup[fSFP].at (fSlave);

theSetup.fInternalTrigger = ReadGosip (fSFP, fSlave, POLAND_REG_INTERNAL_TRIGGER);
theSetup.fTriggerMode = ReadGosip (fSFP, fSlave, POLAND_REG_MASTERMODE);
theSetup.fEventCounter = ReadGosip (fSFP, fSlave, POLAND_REG_TRIGCOUNT);
theSetup.fQFWMode = ReadGosip (fSFP, fSlave, POLAND_REG_QFW_MODE);

for (int i = 0; i < POLAND_TS_NUM; ++i)
{
  theSetup.fSteps[i] = ReadGosip (fSFP, fSlave, POLAND_REG_STEPS_BASE + 4 * i);
  theSetup.fTimes[i] = ReadGosip (fSFP, fSlave, POLAND_REG_TIME_BASE + 4 * i);
}

// for errorcounters we have to scan token payload:
int errcountstart = 4*(32 + theSetup.fSteps[0] * 32 + theSetup.fSteps[1] * 32 + theSetup.fSteps[2] * 32);

for (int e = 0; e < POLAND_ERRCOUNT_NUM; ++e)
{
  theSetup.fErrorCounter[e] = ReadGosip (fSFP, fSlave, errcountstart + 4 * e);
}

theSetup.fDACMode=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_MODE);
theSetup.fDACCalibTime=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_TIME);
theSetup.fDACOffset=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_OFFSET);
theSetup.fDACStartValue=ReadGosip (fSFP, fSlave, POLAND_REG_DAC_CAL_STARTVAL);

for (int d = 0; d < POLAND_DAC_NUM; ++d)
{
  theSetup.fDACValue[d] = ReadGosip (fSFP, fSlave, POLAND_REG_DAC_BASE_READ + 4 * d);
}

theSetup.fTriggerOn=ReadGosip (fSFP, fSlave, POLAND_REG_TRIG_ON);

fTriggerOn=theSetup.fTriggerOn;
// for the moment, we only refresh the general trigger flag from current frontend


GetSensors();


//printf("GetRegisters for sfp:%d slave:%d DUMP \n",fSFP, fSlave);
//theSetup.Dump();

// TODO: error handling with exceptions?
}



  void PolandGui::RefreshSensors()
  {

    QString text;
    QString pre;
    fNumberBase==16? pre="0x" : pre="";
    PolandSetup& theSetup = fSetup[fSFP].at (fSlave);

    QString idtext=QString("POLAND Firmware version: 0x%1 [YYMMDDVV]").arg(theSetup.GetVersionId(),0,16);
    FirmwareLabel->setText(idtext);

    TBaseLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    TLogicLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    TStretchLCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    TPiggy1LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    TPiggy2LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    TPiggy3LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    TPiggy4LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);

    Fan1_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    Fan2_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    Fan3_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);
    Fan4_LCD->setMode((fNumberBase==16) ? QLCDNumber::Hex :  QLCDNumber::Dec);

    TBaseLCD->display (theSetup.GetTemp_Base());
    TLogicLCD->display (theSetup.GetTemp_LogicUnit());
    TStretchLCD->display (theSetup.GetTemp_Stretcher());
    TPiggy1LCD->display (theSetup.GetTemp_Piggy_1());
    TPiggy2LCD->display (theSetup.GetTemp_Piggy_2());
    TPiggy3LCD->display (theSetup.GetTemp_Piggy_3());
    TPiggy4LCD->display (theSetup.GetTemp_Piggy_4());

    QString basetext=QString("0x%1").arg(qulonglong(theSetup.GetSensorId_Base()),0,16);
    IdBaseLabel->setText(basetext);

    QString logictext=QString("0x%1").arg(qulonglong(theSetup.GetSensorId_LogicUnit()),0,16);
    IdLogicLabel->setText(logictext);


    QString stretchtext=QString("0x%1").arg(qulonglong(theSetup.GetSensorId_Stretcher()),0,16);
    IdStretchLabel->setText(stretchtext);

    QString piggy1text=QString("0x%1").arg(qulonglong(theSetup.GetSensorId_Piggy_1()),0,16);
    IdPiggy1Label->setText(piggy1text);
    QString piggy2text=QString("0x%1").arg(qulonglong(theSetup.GetSensorId_Piggy_2()),0,16);
    IdPiggy2Label->setText(piggy2text);

    QString piggy3text=QString("0x%1").arg(qulonglong(theSetup.GetSensorId_Piggy_3()),0,16);
    IdPiggy3Label->setText(piggy3text);

    QString piggy4text=QString("0x%1").arg(qulonglong(theSetup.GetSensorId_Piggy_4()),0,16);
    IdPiggy4Label->setText(piggy4text);





    Fan1_LCD->display (theSetup.GetFanRPM(0));
    Fan2_LCD->display (theSetup.GetFanRPM(1));
    Fan3_LCD->display (theSetup.GetFanRPM(2));
    Fan4_LCD->display (theSetup.GetFanRPM(3));

    FanDial->setValue(theSetup.GetFanSettings());


  }


void PolandGui::GetSensors ()
  {

  if (!AssertNoBroadcast ())
    return;

  PolandSetup& theSetup = fSetup[fSFP].at (fSlave);

  unsigned int version=ReadGosip (fSFP, fSlave, POLAND_REG_FIRMWARE_VERSION);
  theSetup.SetVersionId(version);

//  unsigned long long id_lsb=ReadGosip (fSFP, fSlave, POLAND_REG_ID_LSB);
//  unsigned long long id_msb=ReadGosip (fSFP, fSlave, POLAND_REG_ID_MSB);
//  id_msb= id_msb & 0xFFFFFF; // mask out upper crc word
//  unsigned long long id= (id_msb << 32) + id_lsb;
//  theSetup.SetSensorId(id);



  unsigned int address=POLAND_REG_TEMP_BASE;
  for(int t=0; t<POLAND_TEMP_NUM;t+=2)
  {
    unsigned int data=ReadGosip (fSFP, fSlave, address);
    theSetup.SetTempRaw(t, (data & 0xffff));
    theSetup.SetTempRaw(t+1, (data>>16) & 0xffff);
    address+=4;
  }
  address=POLAND_REG_ID_BASE;
  for(int t=0; t<POLAND_TEMP_NUM;++t)
  {
    unsigned long long id_msb=ReadGosip (fSFP, fSlave, address);
    unsigned long long id_lsb=ReadGosip (fSFP, fSlave, address+4);
    id_msb= id_msb & 0xFFFFFF; // mask out upper crc word
    unsigned long long id= (id_msb << 32) + id_lsb;
    theSetup.SetSensorId(t,id);
    address+=8;
  }



  address=POLAND_REG_FAN_BASE;
  for(int t=0; t<POLAND_FAN_NUM;t+=2)
  {
    unsigned int data=ReadGosip (fSFP, fSlave, address);
    theSetup.SetFanRaw(t, (data & 0xffff));
    theSetup.SetFanRaw(t+1, (data>>16) & 0xffff);
    address+=4;
  }




 // read back fan setter value:
  theSetup.fFanSettings=ReadGosip (fSFP, fSlave, POLAND_REG_FAN_SET);




  }

  void PolandGui::SetFans ()
  {
    //std::cout << "PolandGui::SetFans()"<< std::endl;
    PolandSetup& theSetup = fSetup[fSFP].at (fSlave);
    WriteGosip (fSFP, fSlave, POLAND_REG_FAN_SET, theSetup.fFanSettings);

  }




void PolandGui::SaveConfigBtn_clicked ()
{
//std::cout << "PolandGui::SaveConfigBtn_clicked()"<< std::endl;

  static char buffer[1024];
  QString gos_filter ("gosipcmd file (*.gos)");
  //QString dmp_filter ("data dump file (*.dmp)");
  QStringList filters;
  filters << gos_filter;// << dmp_filter;

  QFileDialog fd (this, "Write POLAND configuration file");

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
//  else if (fd.selectedNameFilter () == dmp_filter)
//  {
//    if (!fileName.endsWith (".dmp"))
//      fileName.append (".dmp");
//  }
  else
  {
    std::cout << "PolandGui::SaveConfigBtn_clicked( - NEVER COME HERE!!!!)" << std::endl;
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


    POLAND_BROADCAST_ACTION(SaveRegisters()); // refresh actual setup from hardware and write it to open file
  }

//  else if (fileName.endsWith (".dmp"))
//  {
//    // dump configuration
//    WriteConfigFile (QString ("#Format *.dmp - register dump output\n"));
//    WriteConfigFile (QString ("#                                         \n"));
//
//
//  }
  else
  {
    std::cout << "PolandGui::SaveConfigBtn_clicked( -  unknown file type, NEVER COME HERE!!!!)" << std::endl;
  }

  // close file
  CloseConfigFile ();
  snprintf (buffer, 1024, "Saved current slave configuration to file '%s' .\n", fileName.toLatin1 ().constData ());
  AppendTextWindow (buffer);
}

int PolandGui::OpenConfigFile (const QString& fname)
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
  WriteConfigFile (QString ("# Poland configuration file saved on ") + timestring + QString ("\n"));
  return 0;
}

int PolandGui::CloseConfigFile ()
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

int PolandGui::WriteConfigFile (const QString& text)
{
  if (fConfigFile == NULL)
    return -1;
  if (fprintf (fConfigFile, text.toLatin1 ().constData ()) < 0)
    return -2;
  return 0;
}




void PolandGui::SaveRegisters ()
{
  // this may be called in explicit broadcast mode, so it is independent of the view on gui
  GetRegisters(); // refresh actual setup from hardware
  fSaveConfig = true;    // switch to file output mode
  WriteConfigFile (QString ("# QFW Registers: \n"));
  SetRegisters();    // register settings are written to file
  WriteConfigFile (QString ("# DAC Settings: \n"));
  ApplyDAC();
  fSaveConfig = false;
}










int PolandGui::ReadGosip (int sfp, int slave, int address)
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

int PolandGui::WriteGosip (int sfp, int slave, int address, int value)
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

int PolandGui::SaveGosip (int sfp, int slave, int address, int value)
{
//std::cout << "# SaveGosip" << std::endl;
  static char buffer[1024] = { };
  snprintf (buffer, 1024, "%d %d %x %x \n", sfp, slave, address, value);
  QString line (buffer);
  return (WriteConfigFile (line));
}



QString PolandGui::ExecuteGosipCmd (QString& com, int timeout)
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
    std::cout << " PolandGui: " << buf.str ().c_str ();
    AppendTextWindow (buf.str ().c_str ());
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



void PolandGui::FlushTextWindow ()
{
  TextOutput->repaint ();
}



bool PolandGui::AssertNoBroadcast (bool verbose)
{
if (fSFP < 0 || fSlave < 0)
{
  //std::cerr << "# PolandGui Error: broadcast not supported here!" << std::endl;
  if (verbose)
    AppendTextWindow ("#Error: broadcast not supported here!");
  return false;
}
return true;
}

bool PolandGui::AssertChainConfigured (bool verbose)
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


void PolandGui::GetSFPChainSetup()
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
        fSetup[sfp].push_back(PolandSetup());
        //std::cout<<"GetSFPChainSetup increased setup at sfp "<<sfp<<" to "<<fSetup[sfp].size()<<" slaves." << std::endl;
      }
    }

}



