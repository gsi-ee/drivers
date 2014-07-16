#ifndef POLANDGUI_H
#define POLANDGUI_H

#include "ui_PolandGui.h"
//#include <QGo4Widget.h>
#include <stdio.h>
#include <QProcess>
#include <QString>

#define POLAND_REG_TRIGCOUNT 0x0

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
#define POLAND_REG_ERRCOUNT_BASE        0x200
#define POLAND_ERRCOUNT_NUM             8
#define POLAND_DAC_NUM                  32

/* microsecond per time register unit*/
#define POLAND_TIME_UNIT                0.02

class PolandSetup
{
public:
  unsigned int fSteps[POLAND_TS_NUM];
  unsigned int fTimes[POLAND_TS_NUM];
  char fInternalTrigger;
  char fTriggerMode;
  char fQFWMode;

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
      fInternalTrigger (0), fTriggerMode (0), fQFWMode(0),fEventCounter (0), fDACMode(0),fDACAllValue(0), fDACStartValue(0),
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
    printf ("Trigger Master:%d, FESA:%d, Internal Trigger:%d \n", IsTriggerMaster (), IsFesaMode (),
        IsInternalTrigger ());
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

protected:

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  QProcessEnvironment fEnv;
#endif
  /* text debug mode*/
  bool fDebug;

  /* base for number display (10 or 16)*/
  int fNumberBase;

  /* index of sfp channel,   -1 for broadcast */
  int fChannel;
  /* index of slave device , -1 for broadcast*/
  int fSlave;

  PolandSetup fSetup;
  /* update register display*/
  void RefreshView ();

  /* copy values from gui to internal status object*/
  void EvaluateView ();

  /* copy sfp and slave from gui to variables*/
  void EvaluateSlave ();

  /* find out measurement mode from selected combobox entry*/
  void EvaluateMode();

  /* update measurement range in combobox entry*/
    void RefreshMode();


  /* set register from status structure*/
  void SetRegisters ();

  /* get register contents to status structure*/
  void GetRegisters ();

  /* Apply DAC setup to frontends*/
  void ApplyDAC();

  /* Refresh view of DAC contents*/
  void RefreshDAC();

  /* Refresh view of DAC mode*/
  void RefreshDACMode();

  /* copy gui contents of DAC tab to setup structure*/
  void EvaluateDAC();

  /* Read from address from sfp and slave, returns value*/
  int ReadGosip (int sfp, int slave, int address);

  /* Write value to address from sfp and slave*/
  int WriteGosip (int sfp, int slave, int address, int value);

  /* execute gosip command in shell. Return value is output of command*/
  QString ExecuteGosipCmd (QString& command);

  void AppendTextWindow (const QString& text);

  void AppendTextWindow (const char* txt)
  {
    QString buf (txt);
    AppendTextWindow (buf);
  }

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
  /* Check if broadast mode is not set. If set, returns false and prints error message if verbose is true*/
  bool AssertNoBroadcast (bool verbose=true);

public slots:
  virtual void ShowBtn_clicked();
  virtual void ApplyBtn_clicked ();
  virtual void InitChainBtn_clicked ();
  virtual void ResetBoardBtn_clicked ();
  virtual void BroadcastBtn_clicked ();
  virtual void DumpBtn_clicked ();
  virtual void ClearOutputBtn_clicked ();
  virtual void ConfigBtn_clicked ();
  virtual void OffsetBtn_clicked ();
  virtual void DebugBox_changed (int on);
  virtual void HexBox_changed(int on);
  virtual void Slave_changed(int val);
  virtual void DACMode_changed(int ix);
};

#endif
