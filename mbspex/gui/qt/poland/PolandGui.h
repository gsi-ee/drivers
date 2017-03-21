#ifndef POLANDGUI_H
#define POLANDGUI_H

#include "ui_PolandGui.h"
#include <stdio.h>
#include <QProcess>
#include <QString>


/** this define will switch between direct call of mbspex lib or external shell call of gosipcmd*
 * note: we need to call "make nombspex" if we disable this define here!
 * note2: this define is enabled from top Makefile when building regular "make all"*/
//#define USE_MBSPEX_LIB 1

#ifdef USE_MBSPEX_LIB
extern "C"
{
#include "mbspex/libmbspex.h"
}
#else
// provide dummy structure although never filled by driver:
#define PEX_SFP_NUMBER 4
struct pex_sfp_links{
    int numslaves[PEX_SFP_NUMBER]; /**< contains configured number of slaves at each sfp chain. */
};

#endif


#define POLAND_REG_TRIGCOUNT 0x0

#define POLAND_REG_RESET 0x200000
#define POLAND_REG_STEPS_BASE   0x200014
#define POLAND_REG_STEPS_TS1    0x200014
#define POLAND_REG_STEPS_TS2    0x200018
#define POLAND_REG_STEPS_TS3    0x20001C

#define POLAND_REG_TIME_BASE    0x200020
#define POLAND_REG_TIME_TS1     0x200020
#define POLAND_REG_TIME_TS2     0x200024
#define POLAND_REG_TIME_TS3     0x200028
#define POLAND_TS_NUM           3

#define POLAND_REG_QFW_MODE         0x200004
#define POLAND_REG_QFW_PRG          0x200008

#define POLAND_REG_DAC_MODE         0x20002c
#define POLAND_REG_DAC_PROGRAM      0x200030
#define POLAND_REG_DAC_BASE_WRITE         0x200050
#define POLAND_REG_DAC_BASE_READ         0x200180

#define POLAND_REG_DAC_ALLVAL       0x2000d4
#define POLAND_REG_DAC_CAL_STARTVAL  0x2000d0
#define POLAND_REG_DAC_CAL_OFFSET    0x200034
#define POLAND_REG_DAC_CAL_DELTA     0x20000c
#define POLAND_REG_DAC_CAL_TIME      0x200038


#define POLAND_REG_INTERNAL_TRIGGER     0x200040
#define POLAND_REG_DO_OFFSET            0x200044
#define POLAND_REG_OFFSET_BASE          0x200100
#define POLAND_REG_MASTERMODE           0x200048
#define POLAND_REG_TRIG_ON              0x20004C

#define POLAND_ERRCOUNT_NUM             8
#define POLAND_DAC_NUM                  32

/** microsecond per time register unit*/
#define POLAND_TIME_UNIT                0.02

class PolandSetup
{
public:
  unsigned int fSteps[POLAND_TS_NUM];
  unsigned int fTimes[POLAND_TS_NUM];
  char fInternalTrigger;
  char fTriggerMode;
  char fQFWMode;

  /** toggle trigger acceptance of all frontends*/
  char fTriggerOn;

  unsigned int fEventCounter;
  unsigned int fErrorCounter[POLAND_ERRCOUNT_NUM];

  /* DAC values and settings:*/
   char fDACMode;
  unsigned int fDACValue[POLAND_DAC_NUM];
  unsigned int fDACAllValue;
  unsigned int fDACStartValue;
  unsigned int fDACOffset;
  unsigned int fDACDelta;
  unsigned int fDACCalibTime;

  PolandSetup () :
      fInternalTrigger (0), fTriggerMode (0), fQFWMode(0),fTriggerOn(0),fEventCounter (0), fDACMode(1),fDACAllValue(0), fDACStartValue(0),
      fDACOffset(0),fDACDelta(0),fDACCalibTime(0)
  {
    for (int i = 0; i < POLAND_TS_NUM; ++i)
    {
      fSteps[i] = 0;
      fTimes[i] = 0;
    }
    for (int j = 0; j < POLAND_ERRCOUNT_NUM; ++j)
    {
      fErrorCounter[j] = 0;
    }
    for (int k = 0; k < POLAND_DAC_NUM; ++k)
    {
      fDACValue[k] = 0;
    }
  }

  void SetTriggerMaster (bool on)
  {
    on ? (fTriggerMode |= 2) : (fTriggerMode &= ~2);
  }

  bool IsTriggerMaster ()
  {
    return ((fTriggerMode & 2) == 2);
  }

  void SetTriggerOn (bool on)
   {
     on ? (fTriggerOn = 1) : (fTriggerOn =0);
   }

   bool IsTriggerOn ()
   {
     return (fTriggerOn == 1);
   }

  void SetFesaMode (bool on)
  {
    on ? (fTriggerMode |= 1) : (fTriggerMode &= ~1);
  }

  bool IsFesaMode ()
  {
    return ((fTriggerMode & 1) == 1);
  }

  void SetInternalTrigger (bool on)
  {
    on ? (fInternalTrigger |= 1) : (fInternalTrigger &= ~1);
  }

  bool IsInternalTrigger ()
  {
    return ((fInternalTrigger & 1) == 1);
  }

  /* calculate time in us from setup value of loop */
  double GetStepTime(int loop)
    {
      return ((double)  (fTimes[loop]*POLAND_TIME_UNIT));
    }

  void SetStepTime(double us, int loop)
  {
    fTimes[loop]=us/POLAND_TIME_UNIT;
  }

  /* calculate calibration time in milliseconds from register value*/
  double GetCalibrationTime()
    {
      return ((double)  (fDACCalibTime*POLAND_TIME_UNIT)/1000);
    }

  void SetCalibrationTime(double ms)
   {

    fDACCalibTime=1000* ms /POLAND_TIME_UNIT;
   }



  void Dump ()
  {
    printf ("-----POLAND device status dump:");
    printf ("Trigger Master:%d, FESA:%d, Internal Trigger:%d Trigger Enabled:%d\n", IsTriggerMaster (), IsFesaMode (),
        IsInternalTrigger (), IsTriggerOn());
    printf ("QFW Mode:0x%x", fQFWMode);
    for (int i = 0; i < POLAND_TS_NUM; ++i)
    {
      printf ("Steps[%d]=0x%x\n ", i, fSteps[i]);
      printf ("Times[%d]=0x%x\n ", i, fTimes[i]);
    }
    printf ("Trigger count: %d \n", fEventCounter);
    for (int j = 0; j < POLAND_ERRCOUNT_NUM; ++j)
    {
      printf ("Errors[%d]=%d\n ", j, fErrorCounter[j]);
    }

    printf ("DAC mode: %d \n", fDACMode);
    printf ("DAC Set all  Value: 0x%x", fDACAllValue);
    printf ("DAC Cal Start Value: 0x%x", fDACStartValue);
    printf ("DAC Offset : 0x%x", fDACOffset);
    printf ("DAC Offset Delta : 0x%x", fDACDelta);
    printf ("DAC Calibration Time : 0x%x", fDACCalibTime);



    for (int k = 0; k < POLAND_DAC_NUM; ++k)
       {
          printf ("DAC[%d]=0x%x\n",k,fDACValue[k]);
       }

  }

};

class PolandGui: public QWidget, public Ui::PolandGui
{
  Q_OBJECT

public:
  PolandGui (QWidget* parent = 0);
  virtual ~PolandGui ();


  void AppendTextWindow (const QString& text);

  void AppendTextWindow (const char* txt)
         {
           QString buf (txt);
           AppendTextWindow (buf);
         }

  void FlushTextWindow();


  /** singleton pointer to forward mbspex lib output, also useful without mbspex lib:*/
static PolandGui* fInstance;



protected:

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  QProcessEnvironment fEnv;
#endif

  /** for saving of configuration, we now have setup structures for all slaves.
   * array index is sfp, vector index is febex in chain*/
  std::vector<PolandSetup> fSetup[4];


  /** contains currently configured slaves at the chains.*/
  struct pex_sfp_links fSFPChains;


  /** text debug mode*/
  bool fDebug;

  /** save configuration file instead of setting device values*/
  bool fSaveConfig;

  /** this flag protects some slots during broadcast write mode*/
  bool fBroadcasting;

  /** base for number display (10 or 16)*/
  int fNumberBase;

  /** index of sfp channel,   -1 for broadcast */
  int fSFP;
  /** index of slave device , -1 for broadcast*/
  int fSlave;

  /** remember sfp channel to recover after broadcast*/
  int fSFPSave;

  /** remember slave channel to recover after broadcast*/
  int fSlaveSave;


  /** toggle general trigger state*/
  bool fTriggerOn;


#ifdef USE_MBSPEX_LIB



  /** file descriptor on mbspex device*/
  int fPexFD;

  /** speed down mbspex io with this function from Nik*/
  void I2c_sleep ();

#endif


  //PolandSetup fSetup;

  /** configuration output file handle*/
    FILE* fConfigFile;

  /** open configuration file for writing*/
    int OpenConfigFile(const QString& fname);

    /** guess what...*/
    int CloseConfigFile();

    /** append text to currently open config file*/
    int WriteConfigFile(const QString& text);




  /** update register display*/
  void RefreshView ();

//  /** update febex device index display*/
  void RefreshStatus ();

 /** update initilized chain display and slave limit*/
  void RefreshChains();

 /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
  void ApplyGUISettings();

  /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
   void ApplyQFWSettings();

   /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
     void ApplyDACSettings();

  /** helper function for broadcast: rest current poland slave*/
  void ResetPoland ();

  /** helper function for broadcast: do offset scan for current poland slave*/
   void ScanOffsets ();

   /** helper function for broadcast: dump samples for current poland slave*/
   void DumpSamples ();

  /** copy values from gui to internal status object*/
  void EvaluateView ();

  /** copy sfp and slave from gui to variables*/
  void EvaluateSlave ();

  /** find out measurement mode from selected combobox entry.*/
  void EvaluateMode();

  /** update measurement range in combobox entry*/
    void RefreshMode();

    /** refresh view of general trigger state*/
    void RefreshTrigger();

  /** set register from status structure*/
  void SetRegisters ();

  /** get register contents to status structure*/
  void GetRegisters ();


  /** get registers and write them to config file*/
  void SaveRegisters();


  /** retrieve slave configuration from driver*/
  void GetSFPChainSetup();




  /** Apply DAC setup to frontends*/
  void ApplyDAC();

  /** Refresh view of DAC contents*/
  void RefreshDAC();

  /** Refresh view of DAC mode*/
  void RefreshDACMode();

  /** copy gui contents of DAC tab to setup structure*/
  void EvaluateDAC();

  /** Read from address from sfp and slave, returns value*/
  int ReadGosip (int sfp, int slave, int address);

  /** Write value to address from sfp and slave*/
  int WriteGosip (int sfp, int slave, int address, int value);

  /** Save value to currently open *.gos configuration file*/
  int SaveGosip (int sfp, int slave, int address, int value);

  /** execute (gosip) command in shell. Return value is output of command*/
  QString ExecuteGosipCmd (QString& command,  int timeout=5000);

 

  void DebugTextWindow (const char*txt)
  {
    if (fDebug)
      AppendTextWindow (txt);
  }
  void DebugTextWindow (const QString& text)
  {
    if (fDebug)
      AppendTextWindow (text);
  }
  /** Check if broadast mode is not set. If set, returns false and prints error message if verbose is true*/
  bool AssertNoBroadcast (bool verbose=true);


  /** Check if chain for given sfp and slave index is configured correctly*/
  bool AssertChainConfigured (bool verbose=true);


public slots:
  virtual void ShowBtn_clicked();
  virtual void ApplyBtn_clicked ();
  virtual void InitChainBtn_clicked ();
  virtual void ResetBoardBtn_clicked ();
  virtual void ResetSlaveBtn_clicked ();
  virtual void BroadcastBtn_clicked (bool checked);
  virtual void DumpBtn_clicked ();
  virtual void ClearOutputBtn_clicked ();
  virtual void ConfigBtn_clicked ();
  virtual void SaveConfigBtn_clicked ();
  virtual void OffsetBtn_clicked ();
  virtual void DebugBox_changed (int on);
  virtual void HexBox_changed(int on);
  virtual void Slave_changed(int val);
  virtual void DACMode_changed(int ix);
  virtual void TriggerBtn_clicked ();

  virtual void QFW_changed ();
  virtual void DAC_changed ();

};

#endif
