#ifndef POLANDSETUP_H
#define POLANDSETUP_H

//#include "../framework/GosipGui.h"
#include "GosipGui.h"

#include <math.h>


#define POLAND_REG_TRIGCOUNT 0x0

#define POLAND_REG_RESET 0x200000
#define POLAND_REG_STEPS_BASE   0x200014
#define POLAND_REG_STEPS_TS1    0x200014
#define POLAND_REG_STEPS_TS2    0x200018
#define POLAND_REG_STEPS_TS3    0x20001C

#define POLAND_REG_TIME_BASE    0x200020
#define POLAND_REG_TIME_TS1     0x200020
#define POLAND_REG_TIME_TS2     0x200024
#define POLAND_REG_TIME_TS3     0x200028
#define POLAND_TS_NUM           3

#define POLAND_REG_QFW_MODE         0x200004
#define POLAND_REG_QFW_PRG          0x200008

#define POLAND_REG_DAC_MODE         0x20002c
#define POLAND_REG_DAC_PROGRAM      0x200030
#define POLAND_REG_DAC_BASE_WRITE         0x200050
#define POLAND_REG_DAC_BASE_READ         0x200180

#define POLAND_REG_DAC_ALLVAL       0x2000d4
#define POLAND_REG_DAC_CAL_STARTVAL  0x2000d0
#define POLAND_REG_DAC_CAL_OFFSET    0x200034
#define POLAND_REG_DAC_CAL_DELTA     0x20000c
#define POLAND_REG_DAC_CAL_TIME      0x200038


#define POLAND_REG_INTERNAL_TRIGGER     0x200040
#define POLAND_REG_DO_OFFSET            0x200044
#define POLAND_REG_OFFSET_BASE          0x200100
#define POLAND_REG_MASTERMODE           0x200048
#define POLAND_REG_TRIG_ON              0x20004C


#define POLAND_REG_FAN_BASE             0x200208
#define POLAND_REG_TEMP_BASE             0x200210

#define POLAND_REG_FAN_SET             0x2000dc

#define POLAND_REG_ID_BASE 0x200220

//#define POLAND_REG_ID_MSB 0x200228
//#define POLAND_REG_ID_LSB 0x20022c

#define POLAND_REG_FIRMWARE_VERSION 0x200280

#define POLAND_FAN_NUM              4
#define POLAND_TEMP_NUM             7

// conversion factor temperature sensors to degrees centigrade:
#define POLAND_TEMP_TO_C 0.0625

// conversion factor fan speed to rpm:
#define POLAND_FAN_TO_RPM 30.0



#define POLAND_ERRCOUNT_NUM             8
#define POLAND_DAC_NUM                  32

/** microsecond per time register unit*/
#define POLAND_TIME_UNIT                0.02

class PolandSetup : public GosipSetup
{
public:
  unsigned int fSteps[POLAND_TS_NUM];
  unsigned int fTimes[POLAND_TS_NUM];
  char fInternalTrigger;
  char fTriggerMode;
  char fQFWMode;

  /** toggle trigger acceptance of all frontends*/
  char fTriggerOn;

  unsigned int fEventCounter;
  unsigned int fErrorCounter[POLAND_ERRCOUNT_NUM];

  /* DAC values and settings:*/
   char fDACMode;
  unsigned int fDACValue[POLAND_DAC_NUM];
  unsigned int fDACAllValue;
  unsigned int fDACStartValue;
  unsigned int fDACOffset;
  unsigned int fDACDelta;
  unsigned int fDACCalibTime;

  /** Sensor values*/

  unsigned short fTemperature[POLAND_TEMP_NUM]; //< Sensor temperature
  unsigned short fFanSpeed[POLAND_FAN_NUM]; //< Fan speed value
  unsigned int fFanSettings; //< setter pwm value for fan motion


  unsigned long long fSensorId[POLAND_TEMP_NUM]; //< unique id of the temperature sensors
  unsigned int fVersionId;      //< fpga software version tag


  PolandSetup () : GosipSetup(),
      fInternalTrigger (0), fTriggerMode (0), fQFWMode(0),fTriggerOn(0),fEventCounter (0), fDACMode(1),fDACAllValue(0), fDACStartValue(0),
      fDACOffset(0),fDACDelta(0),fDACCalibTime(0),fFanSettings(0),fVersionId(0)
  {
    for (int i = 0; i < POLAND_TS_NUM; ++i)
    {
      fSteps[i] = 0;
      fTimes[i] = 0;
    }
    for (int j = 0; j < POLAND_ERRCOUNT_NUM; ++j)
    {
      fErrorCounter[j] = 0;
    }
    for (int k = 0; k < POLAND_DAC_NUM; ++k)
    {
      fDACValue[k] = 0;
    }
    for (int k = 0; k < POLAND_TEMP_NUM; ++k)
    {
      fTemperature[k] = 0;
      fSensorId[k] = 0;
    }

    for (int k = 0; k < POLAND_FAN_NUM; ++k)
       {
          fFanSpeed[k] = 0;
       }


  }



  void SetTriggerMaster (bool on)
  {
    on ? (fTriggerMode |= 2) : (fTriggerMode &= ~2);
  }

  bool IsTriggerMaster ()
  {
    return ((fTriggerMode & 2) == 2);
  }

  void SetTriggerOn (bool on)
   {
     on ? (fTriggerOn = 1) : (fTriggerOn =0);
   }

   bool IsTriggerOn ()
   {
     return (fTriggerOn == 1);
   }

  void SetFesaMode (bool on)
  {
    on ? (fTriggerMode |= 1) : (fTriggerMode &= ~1);
  }

  bool IsFesaMode ()
  {
    return ((fTriggerMode & 1) == 1);
  }

  void SetInternalTrigger (bool on)
  {
    on ? (fInternalTrigger |= 1) : (fInternalTrigger &= ~1);
  }

  bool IsInternalTrigger ()
  {
    return ((fInternalTrigger & 1) == 1);
  }

  /* calculate time in us from setup value of loop */
  double GetStepTime(int loop)
    {
      return ((double)  (fTimes[loop]*POLAND_TIME_UNIT));
    }

  void SetStepTime(double us, int loop)
  {
    fTimes[loop]=round(us/POLAND_TIME_UNIT);
  }

  /* calculate calibration time in milliseconds from register value*/
  double GetCalibrationTime()
    {
      return ((double)  (fDACCalibTime*POLAND_TIME_UNIT)/1000);
    }

  void SetCalibrationTime(double ms)
   {

    fDACCalibTime=1000* ms /POLAND_TIME_UNIT;
   }


///////////////////////////////////////////////////////////////////////
  // following was copied from the working go4 unpacker code JAM2017


  /** Temperature sensor getter raw value*/
         unsigned short GetTempRaw(unsigned int t)
         {
           if (t >= POLAND_TEMP_NUM)
             return 0;
           return fTemperature[t];
         }

  /** Temperature sensor setter raw value*/
   void SetTempRaw(unsigned int t, unsigned short value)
   {
     if (t < POLAND_TEMP_NUM)
       fTemperature[t] = value;
   }

   /** Temperature sensor temperature in degrees centigrade*/
   double GetTempCelsius(unsigned int t)
   {
     if (t >= POLAND_TEMP_NUM)
       return -1000.0;
     unsigned short raw = GetTempRaw(t);
     double sign = ((raw & 0x800) == 0x800) ? -1.0 : 1.0;
     raw=(raw & 0x7FF);
     if(sign<0) raw =0x7FF-raw;
     double rev = (raw* POLAND_TEMP_TO_C) * sign;
     return rev;
   }

   /* mapping of indices to sensor positions is done here: */
  double GetTemp_Base ()
  {
    return GetTempCelsius (1);
  }

  double GetTemp_LogicUnit ()
  {
    return GetTempCelsius (0);
  }

  double GetTemp_Stretcher ()
  {
    return GetTempCelsius (2);
  }

  double GetTemp_Piggy_1 ()
  {
    return GetTempCelsius (3);
  }

  double GetTemp_Piggy_2 ()
  {
    return GetTempCelsius (4);
  }
  double GetTemp_Piggy_3 ()
  {
    return GetTempCelsius (5);
  }

  double GetTemp_Piggy_4 ()
  {
    return GetTempCelsius (6);
  }





  /* end mapping of indices to sensor positions*/


  void SetSensorId(unsigned int t, unsigned long long id)
   {
      if (t < POLAND_TEMP_NUM)
          fSensorId[t]=id;
   }

   unsigned long long GetSensorId(unsigned int t)
     {
         if (t >= POLAND_TEMP_NUM)
            return 0;
       return fSensorId[t];
   }

   unsigned long long GetSensorId_Base ()
    {
      return GetSensorId(1);
    }

   unsigned long long GetSensorId_LogicUnit ()
    {
      return GetSensorId(0);
    }

   unsigned long long GetSensorId_Stretcher ()
    {
      return GetSensorId (2);
    }

   unsigned long long GetSensorId_Piggy_1 ()
    {
      return GetSensorId (3);
    }

   unsigned long long GetSensorId_Piggy_2 ()
    {
      return GetSensorId (4);
    }
   unsigned long long GetSensorId_Piggy_3 ()
    {
      return GetSensorId (5);
    }

   unsigned long long GetSensorId_Piggy_4 ()
    {
      return GetSensorId (6);
    }


   /** Temperature sensor getter raw value*/
   unsigned short GetFanRaw(unsigned int f)
   {
     if (f >= POLAND_FAN_NUM)
       return 0;
     return fFanSpeed[f];
   }

   /** Temperature sensor setter raw value*/
   void SetFanRaw(unsigned int f, unsigned short value)
   {
     if (f < POLAND_FAN_NUM)
       fFanSpeed[f] = value;
   }

   /** Temperature sensor temperature in degrees centigrade*/
   double GetFanRPM(unsigned int f)
   {
     if (f >= POLAND_FAN_NUM)
       return -1.0;
     unsigned short raw = GetFanRaw(f);
     double rev = double(raw & 0xFFFF) * POLAND_FAN_TO_RPM;
     return rev;
   }


   /** evaluate pwm on and off words from 16 bit target frequency value*/
   void SetFanSettings(unsigned short settings)
   {
     unsigned short on=0, off=0;


     if(settings==0xffff)
     {
         on=0;
         off=0xffff;

     }
     else if (settings > 0x8000)
     {
       on= 2*(0xffff - settings +1);
       off=0xffff;

     }
     else if (settings == 0x8000)
     {
       on=0xffff;
       off=0xffff;
     }
     else
     {
       // below 0x8000
       on=0xffff;
       off=2*settings;
     }

     fFanSettings=on;
     fFanSettings=(fFanSettings<<16) | off;
   }

   /** convert existing pwm register setup into a relative rpm setup*/
   unsigned short GetFanSettings()
    {
     unsigned short rev=0;

     unsigned short on=(fFanSettings>>16) & 0xffff;
     unsigned short off= fFanSettings & 0xffff;

     if(on==0 && off==0xffff)
       rev=0xffff;
     else if(on==0xffff && off==0xffff)
       rev=0x8000;
     else if (on==0xffff)
     {
       rev= off/2;
     }
     else if(off==0xffff)
     {
      rev = 0xffff - on/2  +1;
     }
     return rev;
    }




   void SetVersionId(unsigned int id)
         {
           fVersionId=id;
         }

   unsigned int GetVersionId(){return fVersionId;}





  void Dump ()
  {
    printm ("-----POLAND device status dump:\n");
    printm ("-----Firmware version 0x%x:\n", GetVersionId());
    printm ("Trigger Master:%d, FESA:%d, Internal Trigger:%d Trigger Enabled:%d\n", IsTriggerMaster (), IsFesaMode (),
        IsInternalTrigger (), IsTriggerOn());
    printm ("QFW Mode:0x%x", fQFWMode);
    for (int i = 0; i < POLAND_TS_NUM; ++i)
    {
      printm ("Steps[%d]=0x%x\n ", i, fSteps[i]);
      printm ("Times[%d]=0x%x\n ", i, fTimes[i]);
    }
    printm ("Trigger count: %d \n", fEventCounter);
    for (int j = 0; j < POLAND_ERRCOUNT_NUM; ++j)
    {
      printm ("Errors[%d]=%d\n ", j, fErrorCounter[j]);
    }

    printm ("DAC mode: %d \n", fDACMode);
    printm ("DAC Set all  Value: 0x%x", fDACAllValue);
    printm ("DAC Cal Start Value: 0x%x", fDACStartValue);
    printm ("DAC Offset : 0x%x", fDACOffset);
    printm ("DAC Offset Delta : 0x%x", fDACDelta);
    printm ("DAC Calibration Time : 0x%x", fDACCalibTime);



    for (int k = 0; k < POLAND_DAC_NUM; ++k)
       {
          printm ("DAC[%d]=0x%x\n",k,fDACValue[k]);
       }


    for (int k = 0; k < POLAND_TEMP_NUM; ++k)
           {
              printm ("Temperature[%d]=%f Celsius\n",k,GetTempCelsius(k));
              printm ("SensorID[%d]=0x%lx \n",k, GetSensorId(k));
           }

    for (int k = 0; k < POLAND_FAN_NUM; ++k)
               {
                  printm ("Fan speed [%d]=%f RPM\n",k,GetFanRPM(k));
               }

  }

};


#endif
