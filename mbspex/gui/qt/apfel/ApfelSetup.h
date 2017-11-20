#ifndef APFELSETUP_H
#define APFELSETUP_H

#include "ApfelDefines.h"

#include <QString>


/** this is a class (structure) to remember the setup of individual APFEL chip*/
class ApfelSetup
{

private:

  /** true if this chip is actually present and sends response*/
  bool fChipPresent;

  /** true if power to apfel chip has been switched on*/
  bool fPoweredOn;

  /** the chip id as taken from QR code tag*/
  QString fChipID;

  /** the address id of this apfel chip on the board*/
  uint8_t fAddressID;

  /** the absolute values of the APFEL dacs*/
  uint16_t fDACValueSet[APFEL_NUMDACS];

  /** low gain setting for high amplification mode (16 or 32). Default is 32*/
  bool fLowGainSet[APFEL_NUMCHANS];

  /** Enabled test pulser for channel*/
  bool fTestPulsEnable[APFEL_NUMCHANS];

  /** amplitude value for test pulse (0-F)*/
  uint8_t fTestPulseAmplitude[APFEL_NUMCHANS];

  /** True if test pulser with positive polarity. False for negative*/
  bool fTestPulsPositive;

  /** most recent measurment of asic current*/
  double fCurrentAsic;

  /** most recent measurment of HVcurrent*/
  double fCurrentHV;

  /** most recent measurment of diode current*/
  double fCurrentDiode;


public:

  /* all initialization here:*/
  ApfelSetup ();

  /** getter and setter methods to avoid possible segfaults at wrong indices: */
  int GetDACValue (int dac);

  int SetDACValue (int dac, uint16_t value);

  int SetLowGain (int chan, bool low = true);

  int GetLowGain (int chan);

  int SetTestPulseEnable (int chan, bool on = true);

  int GetTestPulseEnable (int chan);

  int SetTestPulseAmplitude (int chan, uint8_t amp);

  uint8_t GetTestPulseAmplitude (int chan);

  int SetTestPulsePostive (bool pos = true);

  int GetTestPulsePositive ();

  void SetAddressID (uint8_t address);

  uint8_t GetAddressID ();

   bool IsPresent(){return fChipPresent;}

   int SetPresent(bool on){fChipPresent=on; return 0;}


   bool HasPower(){return fPoweredOn;}

   int SetPower(bool on){fPoweredOn=on; return 0;}

   const QString& GetChipID() {return fChipID;}

   int SetChipID(const QString& tag){fChipID=tag; return 0;}

    void SetCurrentASIC(double val){fCurrentAsic=val;}

   double GetCurrentASIC(){return fCurrentAsic;}

   void SetCurrentHV(double val){fCurrentHV=val;}

   double GetCurrentHV(){return fCurrentHV;}

   void SetCurrentDiode(double val){fCurrentDiode=val;}

   double GetCurrentDiode(){return fCurrentDiode;}






};

#endif
