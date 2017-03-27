#ifndef POLANDGUI_H
#define POLANDGUI_H

//#include "../framework/GosipGui.h"
#include "GosipGui.h"
#include "PolandWidget.h"
#include "PolandSetup.h"
#include <stdio.h>
#include <QProcess>
#include <QString>



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


  /** toggle general trigger state*/
  bool fTriggerOn;


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



  /** refresh view of temp/fan sensors from status structure*/
   void RefreshSensors();


  /** read temp sensors into status structure*/
   void GetSensors ();

   /** set fan speed value*/
   void SetFans ();




  /** get registers and write them to config file*/
  void SaveRegisters();




  /** Apply DAC setup to frontends*/
  void ApplyDAC();

  /** Refresh view of DAC contents*/
  void RefreshDAC();

  /** Refresh view of DAC mode*/
  void RefreshDACMode();

  /** copy gui contents of DAC tab to setup structure*/
  void EvaluateDAC();



public slots:



  virtual void OffsetBtn_clicked ();
  virtual void DACMode_changed(int ix);
  virtual void TriggerBtn_clicked ();
  virtual void QFW_changed ();
  virtual void DAC_changed ();
  virtual void Fan_changed ();

};

#endif
