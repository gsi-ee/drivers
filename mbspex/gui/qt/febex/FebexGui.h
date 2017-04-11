#ifndef FEBEXGUI_H
#define FEBEXGUI_H


#include "GosipGui.h"
#include "FebexWidget.h"
#include "FebexSetup.h"




class FebexGui:  public GosipGui
{
  Q_OBJECT

public:
  FebexGui (QWidget* parent = 0);
  virtual ~FebexGui ();

  virtual GosipSetup* CreateSetup()
     {
       return new FebexSetup();
     }

  


protected:


 /** reference to the embedded widget with all the special controls*/
  FebexWidget* fFebexWidget;





  /** auxiliary references to checkboxes for baseline adjustments*/
  QCheckBox* fBaselineBoxes[16];

  /** auxiliary references to spinbox for baseline adjustment view*/
  QSpinBox* fDACSpinBoxes[16];

  /** auxiliary references to adc baseline display for refresh view*/
  QLineEdit* fADCLineEdit[16];



  /** auxiliary references to spinbox for threshold setup*/
  QSpinBox* fThresholdSpinBoxes[16];

  /** auxiliary references to channel disabled flags*/
  QRadioButton* fChannelDisabledRadio[16];

  /** auxiliary references to channel sparsifying flags*/
   QRadioButton* fChannelSparseRadio[16];

   /** auxiliary references to channel internal trigger flags*/
   QRadioButton* fChannelTriggerRadio[16];


 
 /** reset current febex slave, i.e. initialize it to defaults*/
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
    int WriteDAC_FebexI2c (uint8_t mcpchip, uint8_t chan, uint8_t value);

    /** Read value to i2c bus address of currently selected slave. mcp433 chip id and local channel id are specified*/
    int ReadDAC_FebexI2c (uint8_t mcpchip, uint8_t chan);


    /** evaluate i2c channel adress offset on mcpchip for given channel number*/
    int GetChannelOffsetDAC(uint8_t chan);


    /** Read value from adc channel of currently selected slave. adc unit id and local channel id are specified*/
    int ReadADC_Febex (uint8_t adc, uint8_t chan);

    /** sample adc baseline of global channel febexchan
     *  by avering over several readouts of ADC. Baseline value is returned.*/
    int AcquireBaselineSample(uint8_t febexchan);




  /** helper function that either does enable i2c on board, or writes such commands to .gos file*/
  void EnableI2C();

  /** helper function that either does disable i2c on board, or writes such commands to .gos file*/
  void DisableI2C ();

  /** dump current ADC values of currently set FEBEX*/
  void DumpSlave();

 

  /** Set relativ DAC value dac to FEBEXchannel, returns ADC value*/
  int autoApply(int channel, int dac);


  /** apply relative DAC value dac and refresh gui from ADC sample.
   * This function is capable of usage in FEBEX_BROADCAST_ACTION macro*/
  void AutoApplyRefresh(int channel, int dac);


 /** evaluate change of TUM addon baseline spinbox for febex channel channel*/
  void DAC_spinBox_changed(int channel, int val);


  /** apply threshold for trigger.
   * This function is capable of usage in FEBEX_BROADCAST_ACTION macro*/
  void ApplyThreshold(int channel, int val);



  /** evaluate change of threshold spinbox for febex channel channel*/
  void Threshold_spinBox_changed(int channel, int val);


  /** apply change of disabled febex channel channel
   * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
  void ApplyDisabled(int channel, bool on);

    /** evaluate change of disabled febex channel channel*/
  void Disabled_toggled(int channel, bool on);


  /** apply change of sparsifying data reduction febex channel channel
     * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
  void ApplySparsy(int channel, bool on);

      /** evaluate change of sparsifying febex channel channel*/
  void Sparsy_toggled(int channel, bool on);


  /** apply change of internal trigger for febex channel channel
       * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
    void ApplyIntTrigger(int channel, bool on);

        /** evaluate change of internal triggerfebex channel channel*/
    void IntTrigger_toggled(int channel, bool on);



  /** Automatic adjustment of adc baseline to adctarget value for global febex channel.
   * will return final dac setup value or -1 in case of error*/
  int AdjustBaseline(int channel, int adctarget);

  /** Adjust baselines of the currently selected febex device.*/
  void AutoAdjust();

 

public slots:
 
  virtual void AutoAdjustBtn_clicked ();
 
  virtual void DAC_spinBox_all_changed(int val);
  virtual void Any_spinBox00_changed(int val);
  virtual void Any_spinBox01_changed(int val);
  virtual void Any_spinBox02_changed(int val);
  virtual void Any_spinBox03_changed(int val);
  virtual void Any_spinBox04_changed(int val);
  virtual void Any_spinBox05_changed(int val);
  virtual void Any_spinBox06_changed(int val);
  virtual void Any_spinBox07_changed(int val);
  virtual void Any_spinBox08_changed(int val);
  virtual void Any_spinBox09_changed(int val);
  virtual void Any_spinBox10_changed(int val);
  virtual void Any_spinBox11_changed(int val);
  virtual void Any_spinBox12_changed(int val);
  virtual void Any_spinBox13_changed(int val);
  virtual void Any_spinBox14_changed(int val);
  virtual void Any_spinBox15_changed(int val);


  virtual void TriggerUseWindowChecked(bool);

  virtual void Threshold_spinBox_all_changed(int val);


  virtual void Threshold_spinBox_00_changed (int val);
  virtual void Threshold_spinBox_01_changed (int val);
  virtual void Threshold_spinBox_02_changed (int val);
  virtual void Threshold_spinBox_03_changed (int val);
  virtual void Threshold_spinBox_04_changed (int val);
  virtual void Threshold_spinBox_05_changed (int val);
  virtual void Threshold_spinBox_06_changed (int val);
  virtual void Threshold_spinBox_07_changed (int val);
  virtual void Threshold_spinBox_08_changed (int val);
  virtual void Threshold_spinBox_09_changed (int val);
  virtual void Threshold_spinBox_10_changed (int val);
  virtual void Threshold_spinBox_11_changed (int val);
  virtual void Threshold_spinBox_12_changed (int val);
  virtual void Threshold_spinBox_13_changed (int val);
  virtual void Threshold_spinBox_14_changed (int val);
  virtual void Threshold_spinBox_15_changed (int val);


  virtual void ChannelDisabled_toggled_all(bool on);

  virtual void ChannelDisabled_toggled_00 (bool on);
  virtual void ChannelDisabled_toggled_01 (bool on);
  virtual void ChannelDisabled_toggled_02 (bool on);
  virtual void ChannelDisabled_toggled_03 (bool on);
  virtual void ChannelDisabled_toggled_04 (bool on);
  virtual void ChannelDisabled_toggled_05 (bool on);
  virtual void ChannelDisabled_toggled_06 (bool on);
  virtual void ChannelDisabled_toggled_07 (bool on);
  virtual void ChannelDisabled_toggled_08 (bool on);
  virtual void ChannelDisabled_toggled_09 (bool on);
  virtual void ChannelDisabled_toggled_10 (bool on);
  virtual void ChannelDisabled_toggled_11 (bool on);
  virtual void ChannelDisabled_toggled_12 (bool on);
  virtual void ChannelDisabled_toggled_13 (bool on);
  virtual void ChannelDisabled_toggled_14 (bool on);
  virtual void ChannelDisabled_toggled_15 (bool on);

  virtual void  ChannelDisabled_toggled_special(bool on);


  virtual void ChannelSparsy_toggled_all(bool on);

  virtual void ChannelSparsy_toggled_00 (bool on);
  virtual void ChannelSparsy_toggled_01 (bool on);
  virtual void ChannelSparsy_toggled_02 (bool on);
  virtual void ChannelSparsy_toggled_03 (bool on);
  virtual void ChannelSparsy_toggled_04 (bool on);
  virtual void ChannelSparsy_toggled_05 (bool on);
  virtual void ChannelSparsy_toggled_06 (bool on);
  virtual void ChannelSparsy_toggled_07 (bool on);
  virtual void ChannelSparsy_toggled_08 (bool on);
  virtual void ChannelSparsy_toggled_09 (bool on);
  virtual void ChannelSparsy_toggled_10 (bool on);
  virtual void ChannelSparsy_toggled_11 (bool on);
  virtual void ChannelSparsy_toggled_12 (bool on);
  virtual void ChannelSparsy_toggled_13 (bool on);
  virtual void ChannelSparsy_toggled_14 (bool on);
  virtual void ChannelSparsy_toggled_15 (bool on);

  virtual void ChannelSparsy_toggled_special (bool on);

  virtual void ChannelTrigger_toggled_all(bool on);


  virtual void ChannelTrigger_toggled_00 (bool on);
  virtual void ChannelTrigger_toggled_01 (bool on);
  virtual void ChannelTrigger_toggled_02 (bool on);
  virtual void ChannelTrigger_toggled_03 (bool on);
  virtual void ChannelTrigger_toggled_04 (bool on);
  virtual void ChannelTrigger_toggled_05 (bool on);
  virtual void ChannelTrigger_toggled_06 (bool on);
  virtual void ChannelTrigger_toggled_07 (bool on);
  virtual void ChannelTrigger_toggled_08 (bool on);
  virtual void ChannelTrigger_toggled_09 (bool on);
  virtual void ChannelTrigger_toggled_10 (bool on);
  virtual void ChannelTrigger_toggled_11 (bool on);
  virtual void ChannelTrigger_toggled_12 (bool on);
  virtual void ChannelTrigger_toggled_13 (bool on);
  virtual void ChannelTrigger_toggled_14 (bool on);
  virtual void ChannelTrigger_toggled_15 (bool on);




};

#endif
