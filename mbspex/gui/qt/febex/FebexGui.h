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

/** number of dac/adc channels per chip*/
#define FEBEX_MCP433_NUMCHAN 4


/** adress to read actual adc value. adc id and channel must be
 * written to this address first*/
#define FEBEX_ADC_PORT  0x20001c

/** number of adc units per febex*/
#define FEBEX_ADC_NUMADC 2

/** number of channels per adc unit*/
#define FEBEX_ADC_NUMCHAN 8



/** this is a class (structure) to remember the previous setup read, and the
 * next setup to apply on the currently selected febex device:*/
class FebexSetup
{
public:

  int fDAC; // remember currently set dac chip (for beginners gui)
  int fChannel; // remember currently set dac channel (for beginners gui)

  /** the (relative) baseline values set on the dacs*/
  uint8_t fDACValueSet[FEBEX_MCP433_NUMCHIPS][FEBEX_MCP433_NUMCHAN];

  /** TODO: probably keep the real adc values also here and display them...*/


  /* all initialization here:*/
  FebexSetup (): fDAC(0),fChannel(0)
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
  virtual void DebugBox_changed (int on);
  virtual void HexBox_changed(int on);
  virtual void Slave_changed(int val);
};

#endif
