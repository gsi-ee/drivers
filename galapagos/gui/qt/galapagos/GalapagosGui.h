#ifndef GAPGGUI_H
#define GAPGGUI_H


#include "BasicGui.h"
#include "GalChannelWidget.h"
#include "GalSequenceWidget.h"
#include "GalapagosSetup.h"
#include "GalapagosMacros.h"
#include <kled.h>


/** define*/











class GalapagosGui:  public BasicGui
{
  Q_OBJECT

public:
  GalapagosGui (QWidget* parent = 0);
  virtual ~GalapagosGui ();

  virtual BasicSetup* CreateSetup();


protected:



  GalChannelWidget* fGalChannelWidget;

  GalSequenceWidget* fGalSequenceWidget;

  /** auxiliary references to channel enabled flags*/
   QRadioButton* fChannelEnabledRadio[GAPG_CHANNELS];

   /** auxiliary references to channel simulation flags*/
   QRadioButton* fChannelSimulatedRadio[GAPG_CHANNELS];

   KLed* fChannelActiveLED[GAPG_CHANNELS];

   QComboBox* fChannelSequenceCombo[GAPG_CHANNELS];
 //

   QString fLastFileDir;

 /** reset current slave, i.e. initialize it to defaults*/
  virtual void ResetSlave ();


  /** update register display*/
  void RefreshView ();


  /** overwrite base class method to adjust waittime*/
  virtual void ApplyFileConfig(int gosipwait=0);


  /** copy values from gui to internal status object*/
  void EvaluateView ();


  /** set register from status structure*/
  void SetRegisters ();

  /** get register contents to status structure*/
  void GetRegisters ();



  /** Write value to i2c bus address of currently selected slave. mcp433 chip id and local channel id are specified*/
    int WriteDAC_GalapagosI2c (uint8_t mcpchip, uint8_t chan, uint8_t value);

    /** Read value to i2c bus address of currently selected slave. mcp433 chip id and local channel id are specified*/
    int ReadDAC_GalapagosI2c (uint8_t mcpchip, uint8_t chan);


  /** helper function that either does enable i2c on board, or writes such commands to .gos file*/
  void EnableI2C();

  /** helper function that either does disable i2c on board, or writes such commands to .gos file*/
  void DisableI2C ();

  /** dump current ADC values of currently set GAPG*/
  void Dump();

 

  /** Set relativ DAC value dac to GAPGchannel, returns ADC value*/
  int autoApply(int channel, int dac);


  /** set pattern generator hardware active.
   * This function is capable of usage in GAPG_AUTOAPPLY macro*/
  void ApplyGeneratorActive(bool on);

  /** apply change of enabled pattern generator channel  channel
   * This function is capable of usage in GAPG_AUTOAPPLY macro*/
  void ApplyChannelEnabled(int channel, bool on);

    /** evaluate change of enabled pattern generator channel channel*/
  void ChannelEnabled_toggled(int channel, bool on);

  /** apply simulation modechannel channel
    * This function is capable of usage in GAPG_AUTOAPPLY macro*/
   void ApplyChannelSimulated(int channel, bool on);

     /** evaluate simulation mode of channel channel*/
   void ChannelSimulated_toggled(int channel, bool on);

   void ApplyChannelSequence(int channel, int ix);


   /** evaluate change of disabled febex channel channel*/
   void ChannelSequence_changed(int channel, int ix);

   /** load sequence from file fullname. Returns false if no success*/
   bool LoadSequence(const QString& fullname);

   /** save sequence from setup to file fullname. Returns false if no success*/
   bool SaveSequence(const QString& fullname, GalapagosSequence* seq);

public slots:



  virtual void ChannelEnabled_toggled_all(bool on);

  GALAGUI_DEFINE_MULTICHANNEL_TOGGLED_16(ChannelEnabled);


  virtual void ChannelSimulated_toggled_all(bool on);

  GALAGUI_DEFINE_MULTICHANNEL_TOGGLED_16(ChannelSimulated);


  virtual void ChannelSequence_changed_all(int ix);

  GALAGUI_DEFINE_MULTICHANNEL_CHANGED_16(ChannelSequence);

virtual void GeneratorActive_clicked(bool checked);


virtual void SequenceIDChanged (int ix);

virtual void SequenceNew_clicked();
virtual void SequenceEdit_clicked();
virtual void SequenceLoad_clicked();
virtual void SequenceSave_clicked();
virtual void SequenceApply_clicked();
virtual void SequenceEditCancel_clicked();

virtual void ReadSettings();
virtual void WriteSettings();

};

#endif
