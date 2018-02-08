#ifndef FEBEXGUI_H
#define FEBEXGUI_H


#include "GosipGui.h"
#include "TamexPadiWidget.h"
#include "TamexPadiSetup.h"




class TamexPadiGui:  public GosipGui
{
  Q_OBJECT

public:
  TamexPadiGui (QWidget* parent = 0);
  virtual ~TamexPadiGui ();

  virtual GosipSetup* CreateSetup()
     {
       return new TamexPadiSetup();
     }

  


protected:


 /** reference to the embedded widget with all the special controls*/
  TamexPadiWidget* fTamexPadiWidget;






  /** auxiliary references to threshold register value display for refresh view*/
  QLineEdit* fThresholdLineEdit[TAMEX_TDC_NUMCHAN];

  /** auxiliary references to spinbox for threshold setup*/
  QSlider* fThresholdSlider[TAMEX_TDC_NUMCHAN];



  /** auxiliary references to spinbox for threshold setup*/
  QDoubleSpinBox* fThresholdSpinBoxes[TAMEX_TDC_NUMCHAN];

  /** auxiliary references to channel leading edge enabled flags*/
  QRadioButton* fChannelLeadingRadio[TAMEX_TDC_NUMCHAN];

  /** auxiliary references to channel trailing edge enabled flags*/
   QRadioButton* fChannelTrailingRadio[TAMEX_TDC_NUMCHAN];

   /** auxiliary references to channel complete enabled flags*/
  QGroupBox* fChannelEnabledBox[TAMEX_TDC_NUMCHAN];


  /** Switches between amplified voltages (+- 600mV) and non amplified (+- 3mV)*/
  bool fShowAmplifiedVoltages;

 
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


  /** set threshold register value for global channel*/
  void SetThreshold(uint8_t globalchannel, uint16_t value);

  /** set the enabled status of tdc channels*/
  void SetTDCsEnabledChannels();

  /** set the mode of the lemo trigger outputs*/
  void SetLemoTriggerOut();

  /** define TDC clock source */
  void SetClockSource();

  /** define trigger time windows */
 void SetTriggerWindow();



  /** get register contents to status structure*/
  void GetRegisters ();

  /** enable spi core, start*/
   void EnableSPI();


  /** enable spi core. stop*/
   void DisableSPI();



   /** introduce some extra delay for communication via spi bus*/
   void PadiSPISleep();


  /** Write values to SPI bus address of currently selected slave.
   * Note that we write values for the channels of both padi chips simultaneously in one operation.
   *  return value is false on failure*/
    bool WriteDAC_Padi (uint8_t chan, uint16_t value_padi[TAMEX_PADI_NUMCHIPS]);

    /** Read values of currently selected slave
     *  Note that we read values for the channels of both padi chips simultaneously in one operation from the shift register.
     *  The channel to read must be selected before by method PrepareReadDAC_Padi
     *  return value is false on failure*/
    bool ReadDAC_Padi(uint16_t (&value_padi)[TAMEX_PADI_NUMCHIPS]);


    /** Prepare to transfer threshold of given channel*/
    bool PrepareReadDAC_Padi(uint8_t chan);




  /** dump current ADC values of currently set FEBEX*/
  void DumpSlave();

 



  /** apply threshold for trigger.
   * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
  void ApplyThreshold(int channel, int val);



  /** evaluate change of threshold voltage spinbox for global  channel channel*/
  void Threshold_doubleSpinBox_changed(int channel, double val);

  /** evaluate change of threshold text line for global  channel channel*/
  void Threshold_Text_changed(int channel);

  /** evaluate change of threshold text line for global  channel channel*/
   void Threshold_Slider_changed(int channel, int val);


  /** apply change of enable tamex channel channel
   * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
  void ApplyChannelEnabled(int channel, int leading=-1, int trailing=-1);


  /** enable/or disable all tamex channels
     * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
  void ApplyChannelEnabledAll(bool on);

  /** enable/or disable all tamex channel leading edge TDC
       * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
  void  ApplyLeadingEnabledAll(bool on);

  /** enable/or disable all tamex channel trailing edge TDC
       * This function is capable of usage in GOSIP_BROADCAST_ACTION macro*/
  void ApplyTrailingEnabledAll(bool on);


    /** evaluate change of enabled febex channel channel*/
  void ChannelEnabled_toggled(int channel, bool on);


   /** evaluate change of disabled febex channel channel*/
  void LeadingEnabled_toggled(int channel, bool on);

  /** evaluate change of disabled febex channel channel*/
  void TrailingEnabled_toggled(int channel, bool on);



  /** helper to evaluate displayed voltage according to amp setup*/
double GetVoltageFromRegister(unsigned int regval);

/** helper to evaluate register value from displayed voltage according to amp setup*/
unsigned int GetRegisterFromVoltage(double voltage);


 

public slots:
 


  virtual void VoltageModeCheckBoxChanged(int val);

  virtual void PreTriggerSpinBox_changed(int val);
  virtual void PostTriggerSpinBox_changed(int val);

  virtual void Threshold_doubleSpinBox_all_changed(double val);

  virtual void Threshold_doubleSpinBox_00_changed (double val);
  virtual void Threshold_doubleSpinBox_01_changed (double val);
  virtual void Threshold_doubleSpinBox_02_changed (double val);
  virtual void Threshold_doubleSpinBox_03_changed (double val);
  virtual void Threshold_doubleSpinBox_04_changed (double val);
  virtual void Threshold_doubleSpinBox_05_changed (double val);
  virtual void Threshold_doubleSpinBox_06_changed (double val);
  virtual void Threshold_doubleSpinBox_07_changed (double val);
  virtual void Threshold_doubleSpinBox_08_changed (double val);
  virtual void Threshold_doubleSpinBox_09_changed (double val);
  virtual void Threshold_doubleSpinBox_10_changed (double val);
  virtual void Threshold_doubleSpinBox_11_changed (double val);
  virtual void Threshold_doubleSpinBox_12_changed (double val);
  virtual void Threshold_doubleSpinBox_13_changed (double val);
  virtual void Threshold_doubleSpinBox_14_changed (double val);
  virtual void Threshold_doubleSpinBox_15_changed (double val);


  virtual void Threshold_Slider_all_changed(int val);

  virtual void Threshold_Slider_00_changed (int val);
    virtual void Threshold_Slider_01_changed (int val);
    virtual void Threshold_Slider_02_changed (int val);
    virtual void Threshold_Slider_03_changed (int val);
    virtual void Threshold_Slider_04_changed (int val);
    virtual void Threshold_Slider_05_changed (int val);
    virtual void Threshold_Slider_06_changed (int val);
    virtual void Threshold_Slider_07_changed (int val);
    virtual void Threshold_Slider_08_changed (int val);
    virtual void Threshold_Slider_09_changed (int val);
    virtual void Threshold_Slider_10_changed (int val);
    virtual void Threshold_Slider_11_changed (int val);
    virtual void Threshold_Slider_12_changed (int val);
    virtual void Threshold_Slider_13_changed (int val);
    virtual void Threshold_Slider_14_changed (int val);
    virtual void Threshold_Slider_15_changed (int val);

    virtual void Threshold_Text_all_changed();


    virtual void Threshold_Text_00_changed();
    virtual void Threshold_Text_01_changed();
    virtual void Threshold_Text_02_changed();
    virtual void Threshold_Text_03_changed();
    virtual void Threshold_Text_04_changed();
    virtual void Threshold_Text_05_changed();
    virtual void Threshold_Text_06_changed();
    virtual void Threshold_Text_07_changed();
    virtual void Threshold_Text_08_changed();
    virtual void Threshold_Text_09_changed();
    virtual void Threshold_Text_10_changed();
    virtual void Threshold_Text_11_changed();
    virtual void Threshold_Text_12_changed();
    virtual void Threshold_Text_13_changed();
    virtual void Threshold_Text_14_changed();
    virtual void Threshold_Text_15_changed();



  virtual void ChannelLeading_toggled_all(bool on);

  virtual void ChannelLeading_toggled_00 (bool on);
  virtual void ChannelLeading_toggled_01 (bool on);
  virtual void ChannelLeading_toggled_02 (bool on);
  virtual void ChannelLeading_toggled_03 (bool on);
  virtual void ChannelLeading_toggled_04 (bool on);
  virtual void ChannelLeading_toggled_05 (bool on);
  virtual void ChannelLeading_toggled_06 (bool on);
  virtual void ChannelLeading_toggled_07 (bool on);
  virtual void ChannelLeading_toggled_08 (bool on);
  virtual void ChannelLeading_toggled_09 (bool on);
  virtual void ChannelLeading_toggled_10 (bool on);
  virtual void ChannelLeading_toggled_11 (bool on);
  virtual void ChannelLeading_toggled_12 (bool on);
  virtual void ChannelLeading_toggled_13 (bool on);
  virtual void ChannelLeading_toggled_14 (bool on);
  virtual void ChannelLeading_toggled_15 (bool on);

  virtual void ChannelTrailing_toggled_all(bool on);

    virtual void ChannelTrailing_toggled_00 (bool on);
    virtual void ChannelTrailing_toggled_01 (bool on);
    virtual void ChannelTrailing_toggled_02 (bool on);
    virtual void ChannelTrailing_toggled_03 (bool on);
    virtual void ChannelTrailing_toggled_04 (bool on);
    virtual void ChannelTrailing_toggled_05 (bool on);
    virtual void ChannelTrailing_toggled_06 (bool on);
    virtual void ChannelTrailing_toggled_07 (bool on);
    virtual void ChannelTrailing_toggled_08 (bool on);
    virtual void ChannelTrailing_toggled_09 (bool on);
    virtual void ChannelTrailing_toggled_10 (bool on);
    virtual void ChannelTrailing_toggled_11 (bool on);
    virtual void ChannelTrailing_toggled_12 (bool on);
    virtual void ChannelTrailing_toggled_13 (bool on);
    virtual void ChannelTrailing_toggled_14 (bool on);
    virtual void ChannelTrailing_toggled_15 (bool on);

    virtual void ChannelEnabled_toggled_all(bool on);

       virtual void ChannelEnabled_toggled_00 (bool on);
       virtual void ChannelEnabled_toggled_01 (bool on);
       virtual void ChannelEnabled_toggled_02 (bool on);
       virtual void ChannelEnabled_toggled_03 (bool on);
       virtual void ChannelEnabled_toggled_04 (bool on);
       virtual void ChannelEnabled_toggled_05 (bool on);
       virtual void ChannelEnabled_toggled_06 (bool on);
       virtual void ChannelEnabled_toggled_07 (bool on);
       virtual void ChannelEnabled_toggled_08 (bool on);
       virtual void ChannelEnabled_toggled_09 (bool on);
       virtual void ChannelEnabled_toggled_10 (bool on);
       virtual void ChannelEnabled_toggled_11 (bool on);
       virtual void ChannelEnabled_toggled_12 (bool on);
       virtual void ChannelEnabled_toggled_13 (bool on);
       virtual void ChannelEnabled_toggled_14 (bool on);
       virtual void ChannelEnabled_toggled_15 (bool on);


};

#endif
