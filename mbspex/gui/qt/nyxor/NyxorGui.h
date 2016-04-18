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
class NyxorDACWidget;
class NyxorADCWidget;

/** number of nxyters on nyxor board. may not change so soon...*/
#define NYXOR_NUMNX 2

#define GOS_I2C_DWR  0x8010  // i2c data write reg.   addr
#define GOS_I2C_DRR1 0x8020  // i2c data read  reg. 1 addr
#define GOS_I2C_DWR2 0x8040  // i2c data read  reg. 2 addr
#define GOS_I2C_SR   0x8080  // i2c status     reg.   addr

#define I2C_CTRL_A   0x01
#define I2C_COTR_A   0x03
#define I2C_RECEIVE  0x84
#define I2C_STATUS   0x85

#define CHECK_DAT    0xa9000000


// write address modifiers for different nxyters on nyxor
#define I2C_ADDR_NX0 0x12
#define I2C_ADDR_NX1 0x22

// external DAC base address on i2c:
#define I2C_DAC_BASE_R 0xBC00000
#define I2C_DAC_BASE_W 0xB430000


// spi related definitions:
#define SPI_ENABLE_ADDR 0x11        // SPI enable register
#define SPI_BAUD_ADDR   0x12        // SPI baud rate register
#define SPI_TRANS_ADDR  0x15        // SPI transfer register
#define SPI_WRITE       0x00        // SPI write mode slave
#define SPI_READ        0x80        // SPI read mode slave

#define SPI_ADC_DC0PHASE    0x16  // pointer to ADC DC0 phase register
#define SPI_ADC_PATTERNBASE 0x19  // ADC transmit pattern registers base pointer (0x19-0x1C)

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
#define NXREC_TESTCODE_2_W 0x33 //nxyter    Test Code 1 (NYXOR Self-Test Mode) write
#define NXREC_TESTCODE_2_R 0xB3 //nxyter    Test Code 1 (NYXOR Self-Test Mode) read


class NyxorGui: public QWidget, public Ui::NyxorGui
{
  Q_OBJECT

public:
  NyxorGui (QWidget* parent = 0);
  virtual ~NyxorGui ();



  /** Perform board reset*/
  void FullNyxorReset();

  /** reset of nyxor receiver core*/
  void ReceiverReset();

  /** reset of nxyter time stamp counters*/
  void NXTimestampReset();


  /** enable i2c write for given nxyter id, or writes such commands to .gos file*/
  void EnableI2CWrite(int nxid);

  /** enable i2c read for given nxyter id, or writes such commands to .gos file*/
   void EnableI2CRead(int nxid);

  /** send disable i2c core */
  void DisableI2C();

  /** enable spi core (access external adc)*/
  void EnableSPI();


  /** enable spi core (access external adc)*/
   void DisableSPI();




   /** Write value to i2c bus address of currently selected slave. NXYTER is specified by nxid*/
   int WriteNyxorI2c (int nxid, uint8_t address, uint8_t value, bool veri=false);

   /** Read value from i2c bus address of currently selected slave. NXYTER is specified by nxid*/
   uint8_t ReadNyxorI2c (int nxid, uint8_t address);


   /** Write data value via spi interface to address*/
   int WriteNyxorSPI (uint8_t address, uint8_t value);

   /** Read data value from spi interface address*/
   uint8_t ReadNyxorSPI (uint8_t address);

   /** read back value via i2c interface from external dac of nxid and dacid*/
   uint16_t ReadNyxorDAC(int nxid, uint8_t dacid);

   /** write value via i2c interface to external dac of nxid and dacid*/
   int WriteNyxorDAC(int nxid, uint8_t dacid, uint16_t value);



   /** Write value to register address on nyxor board. Note that only least 24 bits of value are transferred.*/
   int WriteNyxorAddress (uint8_t address, uint32_t value);


   /** Read value from register address on nyxor board. Note that return value has only 24 bits.*/
   uint32_t ReadNyxorAddress (uint8_t address);


   void AppendTextWindow (const QString& text);

     void AppendTextWindow (const char* txt)
     {
       QString buf (txt);
       AppendTextWindow (buf);
     }

     int GetNumberBase(){return fNumberBase;}


     bool IsAutoApply(){return checkBox_AA->isChecked();}

#ifdef USE_MBSPEX_LIB

   /** singleton pointer to forward mbspex output*/
     static NyxorGui* fInstance;

     /** contains currently configured slaves at the chains.*/
     struct pex_sfp_links fSFPChains;

#endif

protected:

#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  QProcessEnvironment fEnv;
#endif




  NxyterWidget* fNxTab[NYXOR_NUMNX];

  GeneralNyxorWidget* fGeneralTab;
  NyxorDACWidget* fDACTab;
  NyxorADCWidget* fADCTab;

  /** text debug mode*/
  bool fDebug;

  /** save configuration file instead of setting device values*/
  bool fSaveConfig;

  /** base for number display (10 or 16)*/
  int fNumberBase;

  /** index of sfp channel,   -1 for broadcast */
  int fSFP;
  /** index of slave device , -1 for broadcast*/
  int fSlave;

  /** remember sfp channel to recover after broadcast*/
  int fSFPSave;

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

  /** update initilized chain display and slave limit*/
   void RefreshChains();

  /** set register from status structure. If force is true, ignore
   * what has changed and write complete setup (important for writing config files!)*/
  void SetRegisters (bool force=false);

  /** get register contents to status structure*/
  void GetRegisters ();

  /** Evaluate from driver how many slaves are connected to sfps*/
  void GetSFPChainSetup();

  /** Read from address from sfp and slave, returns value*/
  int ReadGosip (int sfp, int slave, int address);

  /** Write value to address from sfp and slave*/
  int WriteGosip (int sfp, int slave, int address, int value);

  /** Save value to currently open *.gos configuration file*/
  int SaveGosip(int sfp, int slave, int address, int value);

  /** execute (gosip) command in shell. Return value is output of command*/
  QString ExecuteGosipCmd (QString& command,  int timeout=5000);






  /** open configuration file for writing*/
  int OpenConfigFile(const QString& fname);

  /** guess what...*/
  int CloseConfigFile();

  /** append text to currently open config file*/
  int WriteConfigFile(const QString& text);


  /** convert current context values into gemex/nyxor file format by N.Kurz
   * flag globalsetup will evaluate complete sfp chains instead of locally selecte nyxor
   * flag writeheader specifies if header shall be written (suppressed for broadcast setup)*/
  int WriteNiksConfig(bool globalsetup=false, bool writeheader=true);


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


  /** Check if chain for given sfp and slave index is configured correctly*/
  bool AssertChainConfigured (bool verbose=true);



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
