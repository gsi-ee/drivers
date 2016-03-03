#ifndef FEBEXGUI_H
#define FEBEXGUI_H

#include "ui_FebexGui.h"
#include <stdio.h>
#include <stdint.h>

#include <QProcess>
#include <QString>

/** this define will switch between direct call of mbspex lib or external shell call of gosipcmd*
 * note that we need to call "make nombspex" if we disable this define here!  */
#define USE_MBSPEX_LIB 1

#ifdef USE_MBSPEX_LIB
extern "C"
{
#include "mbspex/libmbspex.h"
}
#endif



#include <iostream>


#define GOS_I2C_DWR  0x208010  // i2c data write reg.   addr
#define GOS_I2C_DRR1 0x208020  // i2c data read  reg. 1 addr


/** i2c address of first mcp443x/5x/ chip on febex for writing values. Used as base
 * to evaluate values for all 4 chips on board with 4 channels each:*/
#define FEBEX_MCP433_BASE_WRITE 0x62580000

/** i2c address of first mcp443x/5x/ chip on febex for read request. Used as base
 * to evaluate values for all 4 chips on board with 4 channels each:*/
#define FEBEX_MCP433_BASE_READ  0xe2580c00

/** this value is i2c adressing offset between mcp chips*/
#define FEBEX_MCP433_OFFSET 0x20000

/** i2c command value to request a data read from mcp433 */
#define FEBEX_MCP433_REQUEST_READ 0x86000000

/** number of dac chips on febex */
#define FEBEX_MCP433_NUMCHIPS 4

/** number of dac  channels per chip*/
#define FEBEX_MCP433_NUMCHAN 4

/** maximum value to set for DAC*/
#define FEBEX_MCP433_MAXVAL 0xFF

/** adress to read actual adc value. adc id and channel must be
 * written to this address first*/
#define FEBEX_ADC_PORT  0x20001c

/** number of adc units per febex*/
#define FEBEX_ADC_NUMADC 2

/** number of channels per adc unit*/
#define FEBEX_ADC_NUMCHAN 8


/* number of samples to evaluate average adc baseline value*/
#define FEBEX_ADC_BASELINESAMPLES 3

/** this is a class (structure) to remember the previous setup read, and the
 * next setup to apply on the currently selected febex device:*/
class FebexSetup
{
public:

//  int fDAC; // remember currently set dac chip (for beginners gui)
//  int fChannel; // remember currently set dac channel (for beginners gui)

  /** the (relative) baseline values set on the dacs*/
  uint8_t fDACValueSet[FEBEX_MCP433_NUMCHIPS][FEBEX_MCP433_NUMCHAN];

  /** TODO: probably keep the real adc values also here and display them...*/


  /* all initialization here:*/
  FebexSetup ()
  {
    for (int m = 0; m < FEBEX_MCP433_NUMCHIPS; ++m)
    {
      for (int c = 0; c < FEBEX_MCP433_NUMCHAN; ++c)
       {
         fDACValueSet[m][c]=0;
       }
    }
  }

  /** getter and setter methods to avoid possible segfaults at wrong indices: */
  int GetDACValue(int chip, int chan)
  {
    if(chip<0 || chip>=FEBEX_MCP433_NUMCHIPS || chan <0 || chan >=FEBEX_MCP433_NUMCHAN) return -1; // error handling
    //std::cout << "GetDACValue ("<<chip<<","<<chan<<")="<< (int)(fDACValueSet[chip][chan])<< std::endl;
    return fDACValueSet[chip][chan];
  }

  int SetDACValue(int chip, int chan, uint8_t value)
    {
      if(chip<0 || chip>=FEBEX_MCP433_NUMCHIPS || chan <0 || chan >=FEBEX_MCP433_NUMCHAN) return -1; // error handling
      fDACValueSet[chip][chan]=value;
      //std::cout << "SetDACValue ("<<chip<<","<<chan<<")="<< (int)(fDACValueSet[chip][chan])<<", val="<<(int) value<< std::endl;
      return 0;
    }

  /** convert febex channel to DAC indices*/
   void EvaluateDACIndices(int febexchannel, int& chip, int& chan)
     {
           chip= febexchannel/FEBEX_MCP433_NUMCHAN ;
           chan= febexchannel-chip*FEBEX_MCP433_NUMCHAN;
     }

  /** helper function to access DAC value via global febex channel number*/
  int GetDACValue(int febexchannel)
    {
      int chip=0, chan=0;
      EvaluateDACIndices(febexchannel, chip, chan);
      return GetDACValue(chip, chan);
    }

  /** helper function to set DAC value via global febex channel number*/
  int SetDACValue(int febexchannel,  uint8_t value)
     {
          int chip=0, chan=0;
          EvaluateDACIndices(febexchannel, chip, chan);
          return SetDACValue(chip, chan, value);
     }




};


class FebexGui: public QWidget, public Ui::FebexGui
{
  Q_OBJECT

public:
  FebexGui (QWidget* parent = 0);
  virtual ~FebexGui ();


  void AppendTextWindow (const QString& text);

  void AppendTextWindow (const char* txt)
         {
           QString buf (txt);
           AppendTextWindow (buf);
         }



   /** singleton pointer to forward mbspex lib output, also useful without mbspex lib:*/
  static FebexGui* fInstance;

protected:

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  QProcessEnvironment fEnv;
#endif


  FebexSetup fSetup;


  /** auxiliary references to checkboxes for baseline adjustments*/
  QCheckBox* fBaselineBoxes[16];

  /** auxiliary references to spinbox for baseline adjustment view*/
  QSpinBox* fDACSpinBoxes[16];

  /** auxiliary references to adc baseline display for refresh view*/
  QLineEdit* fADCLineEdit[16];


  /** text debug mode*/
  bool fDebug;

  /** save configuration file instead of setting device values*/
  bool fSaveConfig;

  /** base for number display (10 or 16)*/
  int fNumberBase;

  /** index of sfp channel,   -1 for broadcast */
  int fChannel;
  /** index of slave device , -1 for broadcast*/
  int fSlave;

  /** remember sfp channel to recover after broadcast*/
  int fChannelSave;

  /** remember slave channel to recover after broadcast*/
  int fSlaveSave;

  /** configuration output file handle*/
  FILE* fConfigFile;

#ifdef USE_MBSPEX_LIB



  /** file descriptor on mbspex device*/
  int fPexFD;

  /** speed down mbspex io with this function from Nik*/
  void I2c_sleep ();

#endif

  /** update register display*/
  void RefreshView ();

//  /** update febex device index display*/
  void RefreshStatus ();



  /** copy values from gui to internal status object*/
  void EvaluateView ();

  /** copy sfp and slave from gui to variables*/
  void EvaluateSlave ();



  /** set register from status structure*/
  void SetRegisters ();

  /** get register contents to status structure*/
  void GetRegisters ();



  /** Read from address from sfp and slave, returns value*/
  int ReadGosip (int sfp, int slave, int address);

  /** Write value to address from sfp and slave*/
  int WriteGosip (int sfp, int slave, int address, int value);

  /** Save value to currently open *.gos configuration file*/
  int SaveGosip(int sfp, int slave, int address, int value);

  /** execute (gosip) command in shell. Return value is output of command*/
  QString ExecuteGosipCmd (QString& command,  int timeout=5000);


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

  /** open configuration file for writing*/
  int OpenConfigFile(const QString& fname);

  /** guess what...*/
  int CloseConfigFile();

  /** append text to currently open config file*/
  int WriteConfigFile(const QString& text);

  /** Set relativ DAC value dac to FEBEXchannel, returns ADC value*/
  int autoApply(int channel, int dac);


 /** evaluate change of spinbox for febex channel channel*/
  void DAC_spinBox_changed(int channel, int val);


  /** Automatic adjustment of adc baseline to adctarget value for global febex channel.
   * will return final dac setup value or -1 in case of error*/
  int AdjustBaseline(int channel, int adctarget);


  void DebugTextWindow (const char*txt)
  {
      AppendTextWindow (txt);
  }
  void DebugTextWindow (const QString& text)
  {
    if (fDebug)
      AppendTextWindow (text);
  }
  /** Check if broadast mode is not set. If set, returns false and prints error message if verbose is true*/
  bool AssertNoBroadcast (bool verbose=true);

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
  virtual void AutoAdjustBtn_clicked ();
  virtual void DebugBox_changed (int on);
  virtual void HexBox_changed(int on);
  virtual void Slave_changed(int val);
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
};

#endif
