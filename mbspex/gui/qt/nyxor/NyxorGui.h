#ifndef NYXORGUI_H
#define NYXORGUI_H

#include "ui_NyxorGui.h"
#include <stdio.h>
#include <stdint.h>

#include <QProcess>
#include <QString>

/** this define will switch between direct call of mbspex lib or external shell call of gosipcmd*/
#define USE_MBSPEX_LIB 1

#ifdef USE_MBSPEX_LIB
extern "C"
{
#include "mbspex/libmbspex.h"
}
#endif




class NxyterWidget;
class GeneralNyxorWidget;

/** number of nxyters on nyxor board. may not change so soon...*/
#define NYXOR_NUMNX 2

#define GOS_I2C_DWR  0x8010  // i2c data write reg.   addr
#define GOS_I2C_DRR1 0x8020  // i2c data read  reg. 1 addr
#define GOS_I2C_DWR2 0x8040  // i2c data read  reg. 2 addr
#define GOS_I2C_SR   0x8080  // i2c status     reg.   addr

#define I2C_CTRL_A   0x01
#define I2C_COTR_A   0x03

#define CHECK_DAT    0xa9000000


// write address modifiers for different nxyters on nyxor
#define I2C_ADDR_NX0 0x12
#define I2C_ADDR_NX1 0x22


// nxyter receiver sub core registers:

#define NXREC_CTRL_W 0x21 // nxyter control register write
#define NXREC_CTRL_R 0xA1 // nxyter control register read
#define NXREC_PRETRIG_W 0x22 //nxyter pre trigger window write
#define NXREC_PRETRIG_R 0xA2 //nxyter pre trigger window read
#define NXREC_POSTTRIG_W 0x23 //nxyter post trigger window write
#define NXREC_POSTTRIG_R 0xA3 //nxyter post trigger window read
#define NXREC_DELAY1_W 0x24 //nxyter Delay Register 1 (Second Test Pulse Delay) write
#define NXREC_DELAY1_R 0xA4 //nxyter Delay Register 1 (Second Test Pulse Delay) read
#define NXREC_DELAY2_W 0x25 //nxyter  Delay Register 2 (Test Acquisition Trigger Delay) write
#define NXREC_DELAY2_R 0xA5 //nxyter  Delay Register 2 (Test Acquisition Trigger Delay) read

#define NXREC_TESTCODE_ADC_W 0x31 //nxyter   ADCs Test Code (NYXOR Self-Test Mode) write
#define NXREC_TESTCODE_ADC_R 0xB1 //nxyter   ADCs Test Code (NYXOR Self-Test Mode) read
#define NXREC_TESTCODE_1_W 0x32 //nxyter    Test Code 1 (NYXOR Self-Test Mode) write
#define NXREC_TESTCODE_1_R 0xB2 //nxyter    Test Code 1 (NYXOR Self-Test Mode) read
#define NXREC_TESTCODE_2_W 0x32 //nxyter    Test Code 1 (NYXOR Self-Test Mode) write
#define NXREC_TESTCODE_2_R 0xB2 //nxyter    Test Code 1 (NYXOR Self-Test Mode) read


class NyxorGui: public QWidget, public Ui::NyxorGui
{
  Q_OBJECT

public:
  NyxorGui (QWidget* parent = 0);
  virtual ~NyxorGui ();


  /** Write value to i2c bus address of currently selected slave. NXYTER is specified by nxid*/
   int WriteNyxorI2c (int nxid, uint8_t address, uint8_t value, bool veri=false);

   /** Read value from i2c bus address of currently selected slave. NXYTER is specified by nxid*/
   uint8_t ReadNyxorI2c (int nxid, uint8_t address);


   void AppendTextWindow (const QString& text);

     void AppendTextWindow (const char* txt)
     {
       QString buf (txt);
       AppendTextWindow (buf);
     }



#ifdef USE_MBSPEX_LIB

   /** singleton pointer to forward mbspex output*/
     static NyxorGui* fInstance;
#endif

protected:

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  QProcessEnvironment fEnv;
#endif


  NxyterWidget* fNxTab[NYXOR_NUMNX];

  GeneralNyxorWidget* fGeneralTab;


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



  /** Perform board reset*/
  void FullNyxorReset();

  /** enable i2c for given nxyter id, or writes such commands to .gos file*/
  void EnableI2C(int nxid);

  /** send disable i2c core */
  void DisableI2C();



  /** open configuration file for writing*/
  int OpenConfigFile(const QString& fname);

  /** guess what...*/
  int CloseConfigFile();

  /** append text to currently open config file*/
  int WriteConfigFile(const QString& text);


  /** convert current context values into gemex/nyxor file format by N.Kurz*/
  int WriteNiksConfig();


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



public:

  int GetNumberBase(){return fNumberBase;}


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
