#ifndef POLANDGUI_H
#define POLANDGUI_H

//#include "../framework/GosipGui.h"
#include "GosipGui.h"
#include "PolandWidget.h"
#include "PolandViewpanelWidget.h"
#include "PolandCSAWidget.h"
#include "PolandSetup.h"
#include <stdio.h>
#include <QProcess>
#include <QString>

#ifdef USE_PEXOR_LIB


// enable definition below to make synchronous redout within kernel module.
// !!! Danger of system crash when poland does not react!!! But concurrency safe
//#define POLANDGUI_SAMPLE_SYNCREADOUT 1

// number of usecs for each cycle that waits for token received
#define POLAND_SAMPLE_WAITCYCLE 1

// number of polls for triggerless readout with wait for data ready:
#define POLAND_SAMPLE_MAXPOLLS 1000000



#include "pexor/PexorTwo.h"
#include "pexor/Benchmark.h"
#include "pexor/DMA_Buffer.h"
#include "pexor/User_Buffer.h"

/* helper macro for BuildEvent to check if payload pointer is still inside delivered region:*/
/* this one to be called at top data processing loop*/
#define  QFWRAW_CHECK_PDATA                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
  printf("############ unexpected end of payload for sfp:%d slave:%d with opticlen:0x%x, skip event\n",sfp_id, device_id, opticlen);\
  return -1; \
}

/* this one just to leave internal loops*/
#define  QFWRAW_CHECK_PDATA_BREAK                                    \
if((pdata - pdatastart) > (opticlen/4)) \
{ \
 break; \
}



#endif
/* USE_PEXOR_LIB*/

class PolandGui: public GosipGui
{
  Q_OBJECT

public:
  PolandGui (QWidget* parent = 0);
  virtual ~PolandGui ();


  virtual GosipSetup* CreateSetup()
     {
       return new PolandSetup();
     }


protected:


  /** reference to the embedded poland widget with all the special controls*/
  PolandWidget* fPolandWidget;

  /** reference to the embedded poland viewpanel widget for plotting data samples JAM2020*/
  PolandViewpanelWidget* fPolandViewpanelWidget;

  /** reference to the embedded poland CSA control widget NEW JAM2021*/
   PolandCSAWidget* fPolandCSAWidget;

  /** toggle general trigger state*/
  bool fTriggerOn;

  /** this flag controls if we want to have the QFW reset action on next refresh*/
  bool fDoResetQFW;


  /** update register display*/
  void RefreshView ();



 /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
  void ApplyGUISettings();

  /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
   void ApplyQFWSettings();

   /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
   void ApplyDACSettings();


   /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
   void ApplyFanSettings();

   /** helper function for broadcast: get shown set up and put it immediately to hardware.*/
   void ApplyCSASettings();

  /** helper function for broadcast: rest current poland slave*/
  virtual void ResetSlave ();

  /** helper function for broadcast: do offset scan for current poland slave*/
   void ScanOffsets ();

   /** helper function for broadcast: dump samples for current poland slave*/
   virtual void DumpSlave ();

  /** copy values from gui to internal status object*/
  void EvaluateView ();

  /** copy gui contents of sensors tab to setup structure*/
   void EvaluateFans();

   /** copy gui contents of CSA tab to setup structure*/
   void EvaluateCSA();


  /** copy sfp and slave from gui to variables*/
  //void EvaluateSlave ();

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



  /** refresh view of temp/fan sensors from status structure*/
   void RefreshSensors();


  /** read temp sensors into status structure*/
   void GetSensors ();

   /** set fan speed value*/
   void SetFans ();

   /** set CSA properties value*/
   void ApplyCSA();

   /** Refresh view of CSA contents*/
    void RefreshCSA();

  /** get registers and write them to config file*/
  void SaveRegisters();




  /** Apply DAC setup to frontends*/
  void ApplyDAC();

  /** Refresh view of DAC contents*/
  void RefreshDAC();

  /** Refresh view of DAC mode*/
  void RefreshDACMode();

  /** Refresh widget (enable/disable) of DAC mode*/
   void EnableDACModeWidgets(int mode);


  /** copy gui contents of DAC tab to setup structure*/
  void EvaluateDAC();

  void GetSample(PolandSample* theSample);

#ifdef USE_PEXOR_LIB
// this function stolen and adopted from polandtest:
int UnpackQfw (pexor::DMA_Buffer* tokbuf, PolandSample* theSample);
#endif


public slots:



  virtual void OffsetBtn_clicked ();
  virtual void DACMode_changed(int ix);
  virtual void TriggerBtn_clicked ();
  virtual void QFWResetBtn_clicked();
  virtual void QFW_changed ();
  virtual void DAC_changed ();
  virtual void Fan_changed ();
  virtual void CSA_changed ();
  virtual void CSA_spinbox_changed (int value);
  virtual void CSA_lineEdit_changed();

  virtual void ShowSample ();
};

#endif
