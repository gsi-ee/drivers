#include "ApfelSetup.h"

#include <iostream>

////////////////////////////////////////////////
//////// the setup data class of a single apfel chip:

ApfelSetup::ApfelSetup () :
fChipPresent(true), fPoweredOn(true), fAddressID (0),
fCurrentAsic(1.0), fCurrentHV(1.0), fCurrentDiode(1.0),
fIDScanOK(false), fGeneralScanOK(false),fIDReverseScanOK(false),fRegisterScanOK(false)

{
  for (int c = 0; c < APFEL_NUMDACS; ++c)
  {
    fDACValueSet[c] = 0;
  }

  for (int c = 0; c < APFEL_NUMCHANS; ++c)
  {
    fLowGainSet[c] = true;
    fTestPulsEnable[c] = false;

  }
  fTestPulsPositive = true;

}

/** getter and setter methods to avoid possible segfaults at wrong indices: */
int ApfelSetup::GetDACValue (int dac)
{
  ASSERT_DAC_VALID(dac)
  //std::cout << "GetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<< std::endl;
  return (fDACValueSet[dac] & 0x3FF);
}

int ApfelSetup::SetDACValue (int dac, uint16_t value)
{
  ASSERT_DAC_VALID(dac)
  fDACValueSet[dac] = (value & 0x3FF);
  //std::cout << "SetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<<", val="<<(int) (value  & 0x3FF)<< std::endl;
  return 0;
}

int ApfelSetup::SetLowGain (int chan, bool low)
{
  ASSERT_CHAN_VALID(chan);
  fLowGainSet[chan] = low;
  return 0;
}

int ApfelSetup::GetLowGain (int chan)
{
  ASSERT_CHAN_VALID(chan);
  return (fLowGainSet[chan] ? 1 : 0);
}
int ApfelSetup::SetTestPulseEnable (int chan, bool on)
{
  ASSERT_CHAN_VALID(chan);
  fTestPulsEnable[chan] = on;
  return 0;
}

int ApfelSetup::GetTestPulseEnable (int chan)
{
  ASSERT_CHAN_VALID(chan);
  return (fTestPulsEnable[chan] ? 1 : 0);
}

int ApfelSetup::SetTestPulseAmplitude (int chan, uint8_t amp)
{
  ASSERT_CHAN_VALID(chan);
  fTestPulseAmplitude[chan] = (amp & 0xF);
  return 0;
}

uint8_t ApfelSetup::GetTestPulseAmplitude (int chan)
{
  ASSERT_CHAN_VALID(chan);
  return fTestPulseAmplitude[chan];
}

int ApfelSetup::SetTestPulsePostive (bool pos)
{
  fTestPulsPositive = pos;
  return 0;
}

int ApfelSetup::GetTestPulsePositive ()
{
  return (fTestPulsPositive ? 1 : 0);
}

void ApfelSetup::SetAddressID (uint8_t address)
{
  fAddressID = address;
}

uint8_t ApfelSetup::GetAddressID ()
{
  return fAddressID;
}

