#include "AwagsSetup.h"

#include <iostream>

////////////////////////////////////////////////
//////// the setup data class of a single awags chip:

AwagsSetup::AwagsSetup () :
fChipPresent(true), fPoweredOn(true), fAddressID (0),
fCurrentAsic(1.0), fCurrentHV(1.0), fCurrentDiode(1.0)
{
  for (int c = 0; c < AWAGS_NUMDACS; ++c)
  {
    fDACValueSet[c] = 0;
    fGain[c]=1;
  }

//  for (int c = 0; c < AWAGS_NUMCHANS; ++c)
//  {
//    fLowGainSet[c] = true;
////    fTestPulsEnable[c] = false;
//
//  }
  //fTestPulsPositive = true;

}

/** getter and setter methods to avoid possible segfaults at wrong indices: */
int AwagsSetup::GetDACValue (int dac)
{
  ASSERT_DAC_VALID(dac)
  //std::cout << "GetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<< std::endl;
  return (fDACValueSet[dac] & 0x3FF);
}

int AwagsSetup::SetDACValue (int dac, uint16_t value)
{
  ASSERT_DAC_VALID(dac)
  fDACValueSet[dac] = (value & 0x3FF);
  //std::cout << "SetDACValue ("<<dac<<")="<< (int)(fDACValueSet[dac])<<", val="<<(int) (value  & 0x3FF)<< std::endl;
  return 0;
}


int AwagsSetup::GetGain (int dac)
{
  ASSERT_DAC_VALID(dac)
  //std::cout << "GetGain ("<<dac<<")="<< (int)(fGain[dac])<< std::endl;
  return (fGain[dac] & 0xFF);
}

int AwagsSetup::SetGain (int dac, uint8_t value)
{
  ASSERT_DAC_VALID(dac)
  fGain[dac] = value;
  //std::cout << "SetGain ("<<dac<<")="<< (int)(fGain[dac])<<", val="<<(int) (value  & 0xFF)<< std::endl;
  return 0;
}




void AwagsSetup::SetAddressID (uint8_t address)
{
  fAddressID = address;
}

uint8_t AwagsSetup::GetAddressID ()
{
  return fAddressID;
}

