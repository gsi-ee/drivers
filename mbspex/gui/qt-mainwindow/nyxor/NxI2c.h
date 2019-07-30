//============================================================================
/*! \file NxI2c.h
 *  \author W.F.J.Mueller/GSI, based on earlier work of Norbert Abel/KIP
 *
 *  JAM taken from CBM roclib and modified to be used with gosipcmd in Nyxor GUI 15-July-2015
 */
//============================================================================

#ifndef NXYTER_NXI2C_H
#define NXYTER_NXI2C_H

//#include "roc/I2cDevice.h"
#include "NxContext.h"
#include "NyxorGui.h"

namespace nxyter {

   //! Names for nXYTER I2C Registers
   // they are directly in nxyter name space for ease of usage !!
   enum nxRegs {
      kNxRegMask00  =  0,
      kNxRegMask01  =  1,
      kNxRegMask02  =  2,
      kNxRegMask03  =  3,
      kNxRegMask04  =  4,
      kNxRegMask05  =  5,
      kNxRegMask06  =  6,
      kNxRegMask07  =  7,
      kNxRegMask08  =  8,
      kNxRegMask09  =  9,
      kNxRegMask10  = 10,
      kNxRegMask11  = 11,
      kNxRegMask12  = 12,
      kNxRegMask13  = 13,
      kNxRegMask14  = 14,
      kNxRegMask15  = 15,
      kNxRegVcg     = 16,
      kNxRegIcgfoll = 17,
      kNxRegVth     = 18,
      kNxRegVbfb    = 19,
      kNxRegVbiasF  = 20,
      kNxRegVbiasS  = 21,
      kNxRegVbiasS2 = 22,
      kNxRegVcm     = 23,
      kNxRegcal     = 24,
      kNxRegiCOMP   = 25,
      kNxRegiDUR    = 26,
      kNxRegiINV    = 27,
      kNxRegiPDH    = 28,
      kNxRegiTWC    = 29,
      kNxRegSpare30 = 30,
      kNxRegSpare31 = 31,
      kNxRegConfig0 = 32,
      kNxRegConfig1 = 33,
      kNxRegOverflowLSB   = 34,
      kNxRegOverflowMSB   = 35,
      kNxRegMissTokenLSB  = 36,
      kNxRegMissTokenMSB  = 37,
      kNxRegdelayTestPuls = 38,
      kNxRegdelayTestTrig = 39,
      kNxRegSpare40 = 40,
      kNxRegSpare41 = 41,
      kNxRegTrimDAQPower  = 42,
      kNxRegdelayClock1   = 43,
      kNxRegdelayClock2   = 44,
      kNxRegdelayClock3   = 45
   };

   static const uint8_t kNxC0TestPulsEnable    = 1<<0;
   static const uint8_t kNxC0TestPulsSync      = 1<<1;
   static const uint8_t kNxC0TestPulsPolarity  = 1<<2;
   static const uint8_t kNxC0TestTrigEnable    = 1<<3;
   static const uint8_t kNxC0TestTrigSync      = 1<<4;
   static const uint8_t kNxC0Disable32MHz      = 1<<5;
   static const uint8_t kNxC0Disable128MHz     = 1<<6;
   static const uint8_t kNxC0TsLsbClockSelect  = 1<<7;
   static const uint8_t kNxC1CalibSelectMask   = 0x3;
   static const uint8_t kNxC1FrontEndPolarity  = 1<<2;
   static const uint8_t kNxC1ReadClockSelect   = 1<<3;

   class NxI2c {
      public:

         explicit NxI2c(NyxorGui* parent, uint8_t id);
         virtual ~NxI2c();

         uint8_t getId(){return fNxId;}

         int setContext(NxContext& cntx, int domask=nxyter::kDoAll,
                        bool veri=false); // JAM turn off verify by default
         int getContext(NxContext& cntx, int domask=nxyter::kDoAll);

         int getCounters(uint16_t& overflow, uint16_t& tokenmiss);
         int setTestModes(bool testpuls, bool testtrig, int calselect); 

         int probe();
         int setToDefault(bool ispos=false, 
                          int maskon=128, int poweron=128);
         void printRegisters(std::ostream& os, int domask=nxyter::kDoAll);

         int setRegMask(const uint8_t *val);
         int setRegCore(const uint8_t *val);
         int setRegTrim(const uint8_t *val);
         int getRegMask(uint8_t *val);
         int getRegCore(uint8_t *val);
         int getRegTrim(uint8_t *val);

         static const char* registerName(int reg);
         static const char* configurationBitName(int bit);

         static uint8_t delayToSetting(uint8_t delay);
         static uint8_t settingToDelay(uint8_t val);

         // following methods taken from former base class i2cDevice.
         // we will implement this by plain gosipcmd calls of NyxorGui:



         int setRegister(uint8_t reg, uint8_t val, bool veri=false);
         int getRegister(uint8_t reg, uint8_t& val);
         int getRegister16(uint8_t reg, uint16_t& val);
         int setRegisterVerify(uint8_t reg, uint8_t valset, uint8_t& valget);

         int setRegisterArray(uint8_t reg, const uint8_t *val, int nreg,
             bool veri=false);
         int getRegisterArray(uint8_t reg, uint8_t *val, int nreg);

         int setMailboxRegister(uint8_t reg, const uint8_t *val, int nval);
         int getMailboxRegister(uint8_t reg, uint8_t *val, int nval);


         // JAM2016: these are forwards to activate i2c cores directly from widget slots:
         void enableI2C();
         void disableI2C();


      protected:

         /** back pointer to owning gui that still keeps gosipcmd interface*/
         NyxorGui* fOwner;

         /** id number of nxyter on Nyxor board*/
         uint8_t fNxId;

   };

}

//! ostream insertion for nxyter::NxI2c
/*!
 * Just calls nxyter::NxI2c::printRegisters()
 * \relates nxyter::NxI2c
 */
inline std::ostream& operator<<(std::ostream& os, nxyter::NxI2c& obj)
  { obj.printRegisters(os); return os; }

#endif
