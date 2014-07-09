
#ifndef POLANDGUI_H
#define POLANDGUI_H

#include "ui_PolandGui.h"
//#include <QGo4Widget.h>
#include <stdio.h>
#include <QProcess>
#include <QString>

#define POLAND_REG_TRIGCOUNT 0x0

#define POLAND_REG_STEPS_BASE 0x200014
#define POLAND_REG_STEPS_TS1 0x200014
#define POLAND_REG_STEPS_TS2 0x200018
#define POLAND_REG_STEPS_TS3 0x20001C

#define POLAND_REG_TIME_BASE  0x200020
#define POLAND_REG_TIME_TS1  0x200020
#define POLAND_REG_TIME_TS2  0x200024
#define POLAND_REG_TIME_TS3  0x200028
#define POLAND_TS_NUM 3

#define POLAND_REG_INTERNAL_TRIGGER  0x200040
#define POLAND_REG_MASTERMODE  0x200048
#define POLAND_REG_ERRCOUNT_BASE  0x200
#define POLAND_ERRCOUNT_NUM  8

class PolandSetup
{
  public:
    unsigned int fSteps[POLAND_TS_NUM];
    unsigned int fTimes[POLAND_TS_NUM];
    char fInternalTrigger;
    char fTriggerMode;
    unsigned int fEventCounter;
    unsigned int fErrorCounter[POLAND_ERRCOUNT_NUM];

    PolandSetup(): fInternalTrigger(0),fTriggerMode(0), fEventCounter(0)
    {
      for(int i=0; i<POLAND_TS_NUM;++i)
      {
        fSteps[i]=0;
        fTimes[i]=0;
      }
      for(int j=0; j<POLAND_ERRCOUNT_NUM;++j)
            {
              fErrorCounter[j]=0;
            }
    }

    void SetTriggerMaster(bool on)
    {
      on? (fTriggerMode |=2) : (fTriggerMode &= ~2);
    }

    bool IsTriggerMaster()
    {
        return ((fTriggerMode & 2) == 2);
    }

    void SetFesaMode(bool on)
    {
      on? (fTriggerMode |=1) : (fTriggerMode &= ~1);
    }

    bool IsFesaMode()
      {
          return ((fTriggerMode & 1) == 1);
      }

    void SetInternalTrigger(bool on)
    {
      on? (fInternalTrigger |=1) : (fTriggerMode &= ~1);
    }

    bool IsInternalTrigger()
    {
      return ((fInternalTrigger & 1) == 1);
    }


    void Dump()
    {
      printf("-----POLAND device status dump:");
      printf("Trigger Master:%d, FESA:%d, Internal Trigger:%d \n",IsTriggerMaster(), IsFesaMode(), IsInternalTrigger());
      for(int i=0; i<POLAND_TS_NUM;++i)
           {
             printf("Steps[%d]=0x%x\n ",i,fSteps[i]);
             printf("Times[%d]=0x%x\n ",i,fTimes[i]);
           }
      printf("Trigger count: %d \n",fEventCounter);
      for(int j=0; j<POLAND_ERRCOUNT_NUM;++j)
                 {
                   printf("Errors[%d]=%d\n ",j,fErrorCounter[j]);
                 }
    }

};



class PolandGui : public QWidget, public Ui::PolandGui
{
    Q_OBJECT

public:
    PolandGui( QWidget* parent = 0);
    virtual ~PolandGui();

protected:


    QProcessEnvironment fEnv;

    /* text debug mode*/
    bool fDebug;

    /* index of sfp channel,   -1 for broadcast */
    int fChannel;
    /* index of slave device , -1 for broadcast*/
    int fSlave;

    PolandSetup fSetup;
    /* update register display*/
    void RefreshView();

    /* copy values from gui to internal status object*/
    void EvaluateView();

    /* copy sfp and slave from gui to variables*/
    void EvaluateSlave();

    /* set register from status structure*/
    void SetRegisters();

    /* get register contents to status structure*/
     void GetRegisters();

    /* Read from address from sfp and slave, returns value*/
    int ReadGosip(int sfp, int slave, int address);

    /* Write value to address from sfp and slave*/
    int WriteGosip(int sfp, int slave, int address, int value);

    /* execute gosip command in shell. Return value is output of command*/
    QString ExecuteGosipCmd(QString& command);

    void AppendTextWindow(const QString& text);

    void AppendTextWindow(const char* txt)
    {
      QString buf(txt);
      AppendTextWindow(buf);
    }


    void DebugTextWindow(const char*txt)
    {
      if(fDebug) AppendTextWindow(txt);
    }
    void DebugTextWindow(const QString& text)
    {
      if(fDebug) AppendTextWindow(text);
    }
    /* Check if broadast mode is not set. If set, returns false and prints error message*/
    bool AssertNoBroadcast();

public slots:
    virtual void ShowBtn_clicked();
    virtual void ApplyBtn_clicked();
    virtual void InitChainBtn_clicked();
    virtual void ResetBoardBtn_clicked();
    virtual void BroadcastBtn_clicked();
    virtual void DumpBtn_clicked();
    virtual void ClearOutputBtn_clicked();
    virtual void ConfigBtn_clicked();
    virtual void DebugBox_changed(int on);
};

#endif
