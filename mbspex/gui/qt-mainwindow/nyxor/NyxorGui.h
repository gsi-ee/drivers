#ifndef NYXORGUI_H
#define NYXORGUI_H


#include "GosipGui.h"
#include "NyxorWidget.h"
#include "NyxorSetup.h"




class NxyterWidget;
class GeneralNyxorWidget;
class NyxorDACWidget;
class NyxorADCWidget;



class NyxorGui: public GosipGui
{
  Q_OBJECT

public:
  NyxorGui (QWidget* parent = 0 , bool oldsetup=false);
  virtual ~NyxorGui ();


 virtual GosipSetup* CreateSetup()
     {
       return new NyxorSetup();
     }


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


  

//     int GetNumberBase(){return fNumberBase;}
//
//
//     bool IsAutoApply(){return checkBox_AA->isChecked();}



protected:

  NyxorWidget* fNyxorWidget;

  NxyterWidget* fNxTab[NYXOR_NUMNX];

  GeneralNyxorWidget* fGeneralTab;
  NyxorDACWidget* fDACTab;
  NyxorADCWidget* fADCTab;

  /* this flag specifies if we use register backward compatibility JAM2016*/
  bool fOldSetup;

  
  /** update register display*/
  void RefreshView ();

  /** copy values from gui to internal status object*/
  void EvaluateView ();

 

  /** get register contents to status structure*/
  void GetRegisters ();

 /** interface method*/
  virtual void SetRegisters ();


   /* get registers and write them to gosipcmd config file*/
   virtual void SaveRegisters();


/** set register from status structure. If force is true, ignore
   * what has changed and write complete setup (important for writing config files!)*/
  void DoSetRegisters (bool force=false);


   /** save registers in roclib dump format*/
   void SaveRegistersDump();



   /** save to Nik Kurz format*/
   void SaveRegistersNik();


   /** generic reset/init call for sfp slave*/
   virtual void ResetSlave();

   /** generic reset/init call for dumpint sfp slave registers*/
   virtual void DumpSlave();

   /** save configuration to a file*/
   virtual void SaveConfig();


   /** apply configuration from a file to the hardware.
    * Subclass implementation may call this functions with gosipwait  time in microseconds
    * for crucial frontend timing*/
   virtual void ApplyFileConfig(int gosipwait=0);

  /** convert current context values into gemex/nyxor file format by N.Kurz
   * flag globalsetup will evaluate complete sfp chains instead of locally selecte nyxor
   * flag writeheader specifies if header shall be written (suppressed for broadcast setup)*/
  int WriteNiksConfig();

  /** helper for niks config file*/
  int WriteNiksConfigHeader (bool globalsetup);





public slots:



};

#endif
