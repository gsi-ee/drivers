// this part of source file for ApfelGui contains everything that talks with hardware

#include "ApfelGui.h"



#include <stdlib.h>
#include <unistd.h>

#include <iostream>


#include <sstream>
#include <string.h>
#include <errno.h>

#include <QTextStream>
#include <QFile>

#include "Qt/qtcpsocket.h"

// following will use external scripts to read toellner power supply
//#define TOELLNER_POWER_USE_SCRIPT 1

// following will test access to /dev/ttyS0 with raw posix methods
//#define TOELLNER_POWER_RAWCALL 1


#ifdef TOELLNER_POWER_RAWCALL
#include <stdio.h>   /* Standard input/output definitions */
#include <fcntl.h>   /* File control definitions */
#include <termios.h> /* POSIX terminal control definitions */
#endif

void ApfelGui::EnableI2C ()
{
#ifdef APFEL_NEED_ENABLEI2C
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000080);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2000020);
#endif
}

void ApfelGui::DisableI2C ()
{
#ifdef APFEL_NEED_ENABLEI2C
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x1000000);
#endif
}

void ApfelGui::ResetSlave ()
{
  printm ("Resetting APFEL for SFP %d Slave %d...", fSFP, fSlave);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, APFEL_RESET);
  sleep (1);

#ifdef DO_APFEL_INIT
  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, DATA_FILT_CONTROL_RST);
  usleep (4000);

#ifdef USE_FEBEX4
  // new: configure ADCs first:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x22000010); // set SPI speed
  // Activating of SPI core with accss to both ADCs:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2100008f);
  // Both ADCs in standby:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x23000803);
  usleep (2000);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x23000800);
  usleep (1000);
  // Write fixed code into ADC's: (needed?)
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, ADC_FIX_CODE);
  usleep (1000);
  // ADC's coding to binary output mode (write):
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x23001400);
  usleep (1000);
  // SPI access to ADC0:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x21000083);
  // For ADC0 - phase of DCO to data:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, ADC0_CLK_PHASE);
  usleep (1000);
  // SPI access to ADC1:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2100008c);
  // For ADC1 - phase of DCO to data:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, ADC1_CLK_PHASE);
  usleep (1000);
  // SPI access to both ADCs:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2100008f);
  // Serial output data control - DDR two-lane, bitwise:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x23002120);

  usleep (1000);
  // Resolution/sample rate override - 14 bits / 105 MSPS (command 1):
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x23010055);
  usleep (1000);
  // Resolution/sample rate override - 14 bits / 105 MSPS (command 2):
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x2300ff01);
  usleep (1000);

  // For both ADCs - chip run:
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, 0x23000800);
  usleep (1000);
  //SPI de-activating:
  WriteGosip (fSFP, fSlave,  GOS_I2C_DWR, 0x21000000);



#endif

///////////// old init for febex/tum gui. to be replaced by febex4 adc inits:
////        // disable test data length
//  WriteGosip (fSFP, fSlave, REG_DATA_LEN, 0x10000000);
//
////        // specify trace length in slices
//  WriteGosip (fSFP, fSlave, REG_FEB_TRACE_LEN, FEB_TRACE_LEN);
//  // note: we skip verify read here to let this work in broadcast mode!
//
////        // specify trigger delay in slices
//  WriteGosip (fSFP, fSlave, REG_FEB_TRIG_DELAY, FEB_TRIG_DELAY);
//  // note: we skip verify read here to let this work in broadcast mode!
//
//  //        // disable trigger acceptance in febex
//  WriteGosip (fSFP, fSlave, REG_FEB_CTRL, 0);
//
//  //        // enable trigger acceptance in febex
//  WriteGosip (fSFP, fSlave, REG_FEB_CTRL, 1);
//
//
//
//
//

  //        // set channels used for self trigger signal

// JAM: the following is reduced version of mbs sample code. instead of arrays for each slave, we just
// take settings for first device at sfp 0. Should be sufficient for baseline setup until mbs configures all?
//  long l_sfp0_feb_ctrl0 = 0x01000000;
//  long l_sfp0_feb_ctrl1 = 0x0;
//  long l_sfp0_feb_ctrl2 = 0xffff;
//  long l_ev_od_or = (l_sfp0_feb_ctrl0 >> 20) & 0x1;
//  long l_pol = (l_sfp0_feb_ctrl0 >> 28) & 0x1;
//  long l_trig_mod = (l_sfp0_feb_ctrl0 >> 24) & 0xf;
//  long l_dis_cha = l_sfp0_feb_ctrl0 & 0x1ffff;
//  long l_dat_redu = l_sfp0_feb_ctrl1 & 0x1ffff;
//  long l_ena_trig = l_sfp0_feb_ctrl2 & 0xffff;
//
//  int trigenabchan = ((l_ev_od_or << 21) | (l_pol << 20) | (l_trig_mod << 16) | l_ena_trig);
//  WriteGosip (fSFP, fSlave, REG_FEB_SELF_TRIG, trigenabchan);
//
////        // set the step size for self trigger and data reduction
//  long l_thresh = 0x1ff;
//  for (int l_k = 0; l_k < APFEL_ADC_CHANNELS; l_k++)
//  {
//    WriteGosip (fSFP, fSlave, REG_FEB_STEP_SIZE, (l_k << 24) | l_thresh);
//  }
//
//  //
////        // reset the time stamp and set the clock source for time stamp counter
//  if (fSFP == 0 && fSlave == 0)    // assume clock source at first slave on first chain here
//  {
//    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x0);
//    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x7);
//  }
//  else
//  {
//    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x0);
//    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x5);
//  }
//
//  //
////        // enable/disable no hit in trace data suppression of channel
//  WriteGosip (fSFP, fSlave, REG_DATA_REDUCTION, l_dat_redu);
//
////        // set channels used for self trigger signal
//  WriteGosip (fSFP, fSlave, REG_MEM_DISABLE, l_dis_cha);
//
////        // write SFP id for channel header
//
//  WriteGosip (fSFP, fSlave, REG_HEADER, fSFP);
//
////        // set trapez parameters for trigger/hit finding
//  WriteGosip (fSFP, fSlave, TRIG_SUM_A_REG, TRIG_SUM_A);
//  WriteGosip (fSFP, fSlave, TRIG_GAP_REG, TRIG_SUM_A + TRIG_GAP);
//  WriteGosip (fSFP, fSlave, TRIG_SUM_B_REG, TRIG_SUM_A + TRIG_GAP + TRIG_SUM_B);
//
//#ifdef ENABLE_ENERGY_FILTER
//#ifdef TRAPEZ
////
////        // set trapez parameters for energy estimation
//  WriteGosip (fSFP, fSlave, ENERGY_SUM_A_REG, ENERGY_SUM_A);
//  WriteGosip (fSFP, fSlave, ENERGY_GAP_REG, ENERGY_SUM_A + ENERGY_GAP);
//  WriteGosip (fSFP, fSlave, ENERGY_SUM_B_REG, ENERGY_SUM_A + ENERGY_GAP + ENERGY_SUM_B);
//
//#endif // TRAPEZ
//#endif // ENABLE_ENERGY_FILTER




  usleep (50);
// enabling after "ini" of all registers (Ivan - 16.01.2013):
  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, DATA_FILT_CONTROL_DAT);
  sleep (1);
  printm ("Did Initialize APFEL for SFP %d Slave %d", fSFP, fSlave);
#else
  printm("Did NOT Initializing APFEL for SFP %d Slave %d, feature is disabled!",fSFP,fSlave);
#endif





}



void ApfelGui::SetRegisters ()
{
  QApplication::setOverrideCursor (Qt::WaitCursor);
  EnableI2C ();    // must be done since mbs setup program may shut i2c off at the end

   theSetup_GET_FOR_SLAVE(BoardSetup);    // check for indices is done in broadcast action macro that calls this function

  SetIOSwitch ();
  for (uint8_t apf = 0; apf < APFEL_NUMCHIPS; ++apf)
  {
    for (uint8_t dac = 0; dac < APFEL_NUMDACS; ++dac)
    {
      int val = theSetup->GetDACValue (apf, dac);
      WriteDAC_ApfelI2c (apf, dac, val);
      //std::cout << "SetRegisters DAC(" << apf <<"," << dac << ") val=" <<  val << std::endl;
    }

    for (uint8_t ch = 0; ch < APFEL_NUMCHANS; ++ch)
    {
      // here set gain factors for each channel:
      SetGain (apf, ch, theSetup->GetLowGain (apf, ch));

    }

    // no use to apply pulser here, since it will start a single pulse
    //SetPulser(apf);
  }

  DisableI2C ();
  QApplication::restoreOverrideCursor ();

}

void ApfelGui::SetIOSwitch ()
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  //std::cout << "SetIOSwitch: apfel=" << theSetup->IsApfelInUse() <<", highgain=" << theSetup->IsHighGain() << ", stretcher="<< theSetup->IsStretcherInUse()<<", pandatest:"<<theSetup->IsUsePandaTestBoard()<<")"<< std::endl;
  SetSwitches (theSetup->IsApfelInUse (), theSetup->IsHighGain (), theSetup->IsStretcherInUse (), theSetup->IsUsePandaTestBoard());

}

void ApfelGui::SetPulser (uint8_t apf)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  // here set test pulser properties. we must use both channels simultaneously:
  uint8_t amp1 = 0, amp2 = 0;
  bool on_1 = theSetup->GetTestPulseEnable (apf, 0);
  bool on_2 = theSetup->GetTestPulseEnable (apf, 1);
  bool on_any = on_1 || on_2;

  if (on_1)
    amp1 = theSetup->GetTestPulseAmplitude (apf, 0);
  if (on_2)
    amp2 = theSetup->GetTestPulseAmplitude (apf, 1);
  SetTestPulse (apf, on_any, amp1, amp2, theSetup->GetTestPulsePositive (apf));
}

void ApfelGui::SetBroadcastPulser ()
{
  bool on = fApfelWidget->PulserCheckBox_all->isChecked ();
  uint8_t amp = fApfelWidget->PulserAmpSpinBox_all->value ();
  bool positive = (fApfelWidget->ApfelTestPolarityBox_all->currentIndex () == 0);
  SetTestPulse (0xFF, on, amp, amp, positive);

}

void ApfelGui::GetDACs (int chip)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);

  for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
  {

    int val = ReadDAC_ApfelI2c (chip, dac);
    //std::cout << "GetDACs(" << chip <<"," << dac << ") val=" << val << std::endl;

    if (val < 0)
    {
      AppendTextWindow ("GetDacs has error!");
      return;    // TODO error message
    }
    //  Note: value=Read Data is one word of 32-bits, where:
      //  Read_Data [31 downto 24] - GOSIP Status register.
      //  Read_Data [23 downto 16] - APFEL Chip ID.
      //  Read_Data [15 downto 14] - APFEL Status Bits.
      //  Read_Data [13 downto 10] - All bits are zeros.
      //  Read_Data [9 downto 0]   - requested data.
    // 2017: if status bits are invalid, disable this chip:
    bool present= (((val >> 14) & 0x3) == 0);
    theSetup->SetApfelPresent(chip, present); // any not responding DAQ will deactivate the chip.
    theSetup->SetDACValue (chip, dac, (val & 0x3ff));
  }
}

void ApfelGui::GetRegisters ()
{
// read register values into structure with gosipcmd

  if (!AssertNoBroadcast ())
    return;
  QApplication::setOverrideCursor (Qt::WaitCursor);
  EnableI2C ();
  for (int chip = 0; chip < APFEL_NUMCHIPS; ++chip)
  {
    GetDACs (chip);

    // todo: here read back amplification settings - not possible!

    // todo: here read back test pulse settings - not possible!

    // note that adc values are not part of the setup structure and sampled in refreshview

  }
  DisableI2C ();
  QApplication::restoreOverrideCursor ();
}



int ApfelGui::ReadDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac)
{
//  int val = 0;
  if (apfelchip >= APFEL_NUMCHIPS)
  {
    AppendTextWindow ("#Error: ReadDAC_ApfelI2c with illegal chip number!");
    return -1;
  }
  int apid = GetApfelId (fSFP, fSlave, apfelchip);
  return (ReadDAC_ApfelI2c_FromID(apid,dac));
}


int ApfelGui::ReadDAC_ApfelI2c_FromID (uint8_t apid, uint8_t dac)
{
  int val = 0;
  // first: read transfer request from apfel chip with id to core register
  int dat = APFEL_TRANSFER_BASE_RD + (dac + 1) * APFEL_TRANSFER_DAC_OFFSET + (apid & 0xFF);
  // mind that dac index starts with 0 for dac1 here!

  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  // note that WriteGosip already contains i2csleep
  //I2c_sleep (); // give additional delay for apfel?
  // second: read request from core registers
  dat = APFEL_DAC_REQUEST_BASE_RD + (dac) * APFEL_CORE_REQUEST_DAC_OFFSET;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  //I2c_sleep (); // give additional delay for apfel?
  //I2c_sleep ();
  // third: actually read requested value
  val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1);    // read out the value
  //I2c_sleep (); // give additional delay for apfel?
  if (val < 0)
    return val;    // error case, propagate it upwards
///////////
//  Note: val=Read Data is one word of 32-bits, where:
//  Read_Data [31 downto 24] - GOSIP Status register.
//  Read_Data [23 downto 16] - APFEL Chip ID.
//  Read_Data [15 downto 14] - APFEL Status Bits.
//  Read_Data [13 downto 10] - All bits are zeros.
//  Read_Data [9 downto 0]   - requested data.
//
/////////////

  //return (val & APFEL_DAC_MAXVALUE);    // mask to use only data part
  return val; // 2017: do not mask out return status bits, evaluate them above !
}





int ApfelGui::ReadADC_Apfel (uint8_t adc, uint8_t chan)
{
  if (adc > APFEL_ADC_NUMADC || chan > APFEL_ADC_NUMCHAN)
    return -1;

  // test: always enable core to read data first:
  //WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, 0x80);

  int val = 0;
  int dat = (adc << 3) + chan;    //l_wr_d  = (l_k*4) + l_l;

  WriteGosip (fSFP, fSlave, APFEL_ADC_PORT, dat);    // first specify channel number

  val = ReadGosip (fSFP, fSlave, APFEL_ADC_PORT);    // read back the value

  // check if channel id matches the requested ones:
  if (((val >> 24) & 0xf) != dat)
  {
    printm ("#Error: ReadADC_Apfel channel id mismatch, requested 0x%x, received 0x%x", dat, (val >> 24));
    return -1;
  }

  //printm("ReadADC_Apfel(%d,%d) reads value=0x%x, return:0x%x",(int) adc, (int) chan, val, (val & APFEL_ADC_MAXVALUE));
  return (val & APFEL_ADC_MAXVALUE);

}



int ApfelGui::WriteDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac, uint16_t value)
{

  //std::cout << "WriteDAC_ApfelI2c(" << apfelchip <<"," << dac << ") value=" << value << std::endl;
  //(0:  shift to max adc value)
  if (apfelchip >= APFEL_NUMCHIPS)
  {
    AppendTextWindow ("#Error: WriteDAC_ApfelI2c with illegal chip number!");
    return -1;
  }
  int apfelid = GetApfelId (fSFP, fSlave, apfelchip);
  return (WriteDAC_ApfelI2c_ToID (apfelid, dac, value));
}

int  ApfelGui::WriteDAC_ApfelI2c_ToID (uint8_t apfelid, uint8_t dac, uint16_t value)
{
// first write value to core register:
int dat = APFEL_CORE_REQUEST_BASE_WR + dac * APFEL_CORE_REQUEST_DAC_OFFSET;
dat |= (value & 0x3FF);
WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
//I2c_sleep (); // give additional delay for apfel?
// then request for data transfer:
dat = APFEL_TRANSFER_BASE_WR + (dac + 1) * APFEL_TRANSFER_DAC_OFFSET + (apfelid & 0xFF);
// mind that dac index starts with 0 for dac1 here!
WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
//I2c_sleep (); // give additional delay for apfel?
return 0;
}

void ApfelGui::SetGain (uint8_t apfelchip, uint8_t chan, bool useGain16)
{
  int apid = GetApfelId (fSFP, fSlave, apfelchip);
  //std::cout << "SetGain DAC(" << (int) apfelchip <<", id:"<<apid << (int) chan << ") gain16=" << useGain16 << std::endl;
  // first set gain to core:
  int mask = 0;
  mask |= apid & 0xFF;
  if (chan == 0)
  {
    // CH1
    if (useGain16)
      mask |= 0x100;
    else
      mask |= 0x000;    // just for completeness..
  }
  else
  {
    // CH2
    if (useGain16)
      mask |= 0x300;
    else
      mask |= 0x200;
  }

  int dat = APFEL_GAIN_BASE_WR | mask;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

}

void ApfelGui::SetTestPulse (uint8_t apfelchip, bool on, uint8_t amp1, uint8_t amp2, bool positive)
{
  int apid = GetApfelId (fSFP, fSlave, apfelchip);
  //std::cout << "SetTestPulse(" << (int) apfelchip <<", id:"<<apid<<"): on=" << on << ", ch1="<<(int) amp1<<", ch2="<< (int) amp2 << std::endl;

  int dat = 0;
  int apfelid = apid & 0xFF;
  if (!on)
  {
    // test pulse is not continuous, so we never need to reset it?
    //dat=APFEL_TESTPULSE_FLAG_WR | apfelid;
    //WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  }
  else
  {
    // first set channel amplitudes for ch1 and ch2:
    dat = APFEL_TESTPULSE_CHAN_WR | apfelid;
    if (amp1)
      dat |= ((amp1 & 0xF) << 8);
    if (amp2)
      dat |= ((amp2 & 0xF) << 12);
    WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

    // activate correct polarity:
    dat = APFEL_TESTPULSE_FLAG_WR | apfelid;
    if (positive)
      dat |= 0x100;
    else
      dat |= 0x300;
    WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  }

}

void ApfelGui::DoAutoCalibrate (uint8_t apfelchip)
{
  QApplication::setOverrideCursor (Qt::WaitCursor);

  int apid = GetApfelId (fSFP, fSlave, apfelchip);
  printm ("Doing Autocalibration of apfel chip %d (id:%d) on sfp:%d, board:%d...", apfelchip, apid, fSFP, fSlave);
  int apfelid = apid & 0xFF;
  int dat = APFEL_AUTOCALIBRATE_BASE_WR | apfelid;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // transfer to apfel chip- not neceesary here!
//   dat=APFEL_TRANSFER_BASE_WR | apfelid;
//   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
 APFEL_ADDRESSTEST_SLEEP
  printm ("...done!\n");
  //Note: The auto calibration of the APFELchip takes not more that 8 ms.
  UpdateAfterAutoCalibrate (apfelchip);

  QApplication::restoreOverrideCursor ();
}

void ApfelGui::DoAutoCalibrateAll ()
{
  QApplication::setOverrideCursor (Qt::WaitCursor);
  printm ("Doing Broadcast Autocalibration of apfel chips on sfp:%d, board:%d...", fSFP, fSlave);
  int apfelid = 0xFF;
  int dat = APFEL_AUTOCALIBRATE_BASE_WR | apfelid;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
 APFEL_ADDRESSTEST_SLEEP
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::SetSwitches (bool useApfel, bool useHighGain, bool useStretcher, bool isPandatest)
{

  int dat = APFEL_IO_CONTROL_WR;
  int mask = 0;

  if(!isPandatest)
  {
    // APFELSEM hardware
  if (!useApfel)
    mask |= APFEL_SW_NOINPUT;
  if (!useHighGain)
    mask |= APFEL_SW_HIGAIN;
  if (useStretcher)
    mask |= APFEL_SW_STRETCH;
  dat |= mask;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  dat = APFEL_IO_SET;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // read back switches
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, APFEL_IO_CONTROL_RD);
  int val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1);
  int swmask = ((val >> 12) & 0x7);
  //printm("SetInputSwitch mask=0x%x, read back switch mask=0x%x", mask, swmask);
  if (((swmask & mask) != mask))
    printm ("#Error SetInputSwitch(apfel=%d, high=%d, stretch=%d) - read back switch mask is 0x%x", useApfel,
        useHighGain, useStretcher, swmask);
  // todo: advanced error handling?

  }
  else
  {
    // new PANDATEST hardware
    // instead of 3 bits, we use the 32 bit control register here:
    int lo=APFEL_IO_DATA_LO_WR;
    int hi=APFEL_IO_DATA_HI_WR;
    hi |= (0xFFFF); // default setup: all enabled, apfel ids defined to 1,...,8
    // TODO: check here which apfels should be enabled!
    if(useHighGain)
      lo |= (0x000c);
    else
      lo |= (0x000d);
    WriteGosip (fSFP, fSlave, GOS_I2C_DWR, hi);
    WriteGosip (fSFP, fSlave, GOS_I2C_DWR, lo);

    dat |= (0x18); // default setup: all enabled
    WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat); // activate hi/lo data register with this

    // TODO: may readback switche settings here?


    // TODO: change configuration of apfel addressing here?


  }

}

void ApfelGui::SetPower (int powermask, bool highgain)
{
//printf("SetPower mask:0x%x, highgain:%d \n",powermask, highgain);

// this switching only works for new pandatest hardware

int dat = APFEL_IO_CONTROL_WR;
int mask = 0;
int lo=APFEL_IO_DATA_LO_WR;
int hi=APFEL_IO_DATA_HI_WR;

// following words of high control word:
int sel_VddAsic=0; // 8bit
int sel_InExt=0;   // 8bit

// following words of low control word:
int sel_DataASIC=0xFF; // 8bit - high is off
int chipID=0; // 4bit



sel_InExt=0xFF; // all externals to ground

// note that we have to preserve the complete state of power switches here
sel_VddAsic |=powermask;
sel_DataASIC  &= ~powermask;

// the common part:
hi |= (sel_VddAsic & 0xFF) <<8;
hi |= sel_InExt & 0xFF;
lo |=(sel_DataASIC & 0xFF) <<8;

// lsb for gain setup
if(highgain)
  lo |= (0x000c);
else
  lo |= (0x000d);



WriteGosip (fSFP, fSlave, GOS_I2C_DWR, hi);
WriteGosip (fSFP, fSlave, GOS_I2C_DWR, lo);

 dat |= (0x18); // default setup: all enabled
 WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);



}


void ApfelGui::InitKeithley()
{
 
  QString askconnect("*IDN?;\n");
  QString reset("*RST;\n");
  QString clearstatus("*CLS;\n");

  std::vector<QString> trigsetup;
    //init device:
  QTextStream tty;
  QFile file;
  QFile infile;
  QTcpSocket sock;
  bool useSerial=fApfelWidget->CurrentSerialCheckBox->isChecked();
  bool useSocket=fApfelWidget->CurrentSocketCheckBox->isChecked();

  trigsetup.push_back(QString (":INIT:CONT OFF;:ABOR;\n"));
  trigsetup.push_back(QString (":TRIG:SOUR IMM;\n"));
  trigsetup.push_back(QString (":TRIG:COUN INF;\n"));
  trigsetup.push_back(QString (":TRIG:DEL:AUTO OFF;\n"));
  trigsetup.push_back(QString (":TRIG:DEL 0.000000;\n"));
  trigsetup.push_back(QString (":INIT:CONT ON;\n"));
  if(useSerial)
  {
    QString dev=fApfelWidget->SerialDeviceName->text();
    printm ("InitKeithley access of %s...",dev.toLatin1 ().constData ());
    file.setFileName(dev);
  if (!file.open(QIODevice::WriteOnly))
    {
        printm ("InitKeithley error when opening %s for writing", dev.toLatin1 ().constData ());
        return;
    }
  infile.setFileName(dev);
  if (!infile.open(QIODevice::ReadOnly))// | QIODevice::Unbuffered ))// | QIODevice::Text))
  {
    printm ("InitKeithley error when opening %s reading", dev.toLatin1 ().constData ());
    return;
  } 
  tty.setDevice(&file);
  }
  else if(useSocket)
  {

   QString host=fApfelWidget->CurrentNodeName->text();
   quint16 port=fApfelWidget->CurrentPortSpinBox->value();

   printm ("InitKeithley access of %s:%d...",host.toLatin1 ().constData (),port);
   sock.connectToHost(host, port, QIODevice::ReadWrite | QIODevice::Text);
   if (sock.waitForConnected(1000))
   {
       printm("Connected!");
   }
   else
   {
     printm("Could not reach device! Error: %d", sock.error());
     return;
   }
   tty.setDevice(&sock);


  }
  else
  {
    return; // fake mode
  }

     
      printm ("InitKeithley will send:%s",askconnect.toLatin1 ().constData ());
      tty << askconnect;
      tty.flush();
      usleep(50000);

      QString answer("");
      if(useSerial)
        answer=infile.readLine();
      //infile.readLine(); // skip blank line after value! ???
        else
          answer=sock.readLine();
      
      
      printm("InitKeithley answer was:%s", answer.toLatin1 ().constData ());

      // TODO: error handling if not connected
      // TODO: send init procedure

      printm ("InitKeithley will send:%s",reset.toLatin1 ().constData ());
      tty << reset;
      tty.flush();
      sleep(1);

      printm ("InitKeithley will send:%s",clearstatus.toLatin1 ().constData ());
      tty << clearstatus;
      tty.flush();
      sleep(1);

      for(int i=0; i<trigsetup.size(); ++i)
           {
              QString com=trigsetup[i];
              printm("sending %s",com.toLatin1 ().constData ());
              tty << com;
              tty.flush();
              usleep(50000);
           }


    // put all  initialization stuff here:


  std::vector<QString> setup;
  //init device:
  setup.push_back(QString (":FUNC 'CURR:DC';\n"));
  setup.push_back(QString (":CURR:DC:DIG 7;\n"));
  setup.push_back(QString (":CURR:DC:NPLC 1.000000;\n"));
  setup.push_back(QString (":CURR:DC:RANG:AUTO ON;\n"));
  setup.push_back(QString (":CURR:DC:AVER:STAT OFF;\n"));
  setup.push_back(QString (":CURR:DC:REF:STAT OFF;\n"));

  // output format
  setup.push_back(QString (":FORM ASC;\n"));
  //setup.push_back(QString (":FORM:ELEM READ,UNIT,CHAN;\n"));

  setup.push_back(QString (":FORM:ELEM READ;\n"));

  for(int i=0; i<setup.size(); ++i)
        {
           QString com=setup[i];
           printm("sending %s",com.toLatin1 ().constData ());
           tty << com;
           tty.flush();
           usleep(50000);
        }

}



double ApfelGui::ReadKeithleyCurrent()
{

 double rev=-1.0;
 QTextStream tty;
 QFile file;
 QFile infile;
 QTcpSocket sock;
 bool useSerial=fApfelWidget->CurrentSerialCheckBox->isChecked();
 bool useSocket=fApfelWidget->CurrentSocketCheckBox->isChecked();
  //Read a single Measurement:
  QString getData(":SENS:DATA? \n");
  if(useSerial)
   {
    QString dev=fApfelWidget->SerialDeviceName->text();
    printm ("ReadKeithleyCurrent access of %s...",dev.toLatin1 ().constData ());
 
  file.setFileName(dev);
   if (!file.open(QIODevice::WriteOnly))
     {
         printm ("ReadKeithleyCurrent error when opening %s for writing", dev.toLatin1 ().constData ());
         return -12.0;;
     }
   infile.setFileName(dev);
   if (!infile.open(QIODevice::ReadOnly))// | QIODevice::Unbuffered ))// | QIODevice::Text))
   {
     printm ("ReadKeithleyCurrent error when opening %s for reading",  dev.toLatin1 ().constData ());
     return -13.0;
   }

       tty.setDevice(&file);
   }
  else if(useSocket)
   {

  QString host=fApfelWidget->CurrentNodeName->text();
  quint16 port=fApfelWidget->CurrentPortSpinBox->value();

   printm ("ReadKeithleyCurrent access of %s:%d...",host.toLatin1 ().constData (),port);
   sock.connectToHost(host, port); //  QIODevice::ReadWrite | QIODevice::Text
   if (sock.waitForConnected(1000))
     {
         printm("Connected!");
     }
     else
     {
       printm("Could not reach device! Error: %d", sock.error());
       return -11.0;
     }

   tty.setDevice(&sock);

   }
  else
  {
    return -22; // fake mode nop

  }
       printm ("sending:%s",getData.toLatin1 ().constData ());
       tty << getData;
       tty.flush();
       usleep(50000);

       QString answer("");
if(useSerial)
       answer=infile.readLine();
      //infile.readLine(); // skip blank line after value! ???
  else
    answer=sock.readLine();
          
       rev=answer.toDouble();
       printm("Got answer string:%s", answer.toLatin1 ().constData ());
       return rev;
}




void ApfelGui::ReadToellnerPower(double& u, double& i)
{
#ifdef   APFEL_USE_TOELLNER_POWER



#ifdef  TOELLNER_POWER_USE_SCRIPT
  // we just use Peters skripts for the moment
  // TODO: communicate with /dev/ttyS0 directly
   QString ucom ("measure_serial.sh V");
   QString uresult = ExecuteGosipCmd (ucom, 10000);
   u=uresult.toDouble();
   printm("Read Toellner Voltage=%f V",u);

   sleep(1);
   QString icom ("measure_serial.sh C");
   QString iresult = ExecuteGosipCmd (icom, 10000);
   i=iresult.toDouble();
   printm("Read Toellner Current=%f A",i);

#elif defined(TOELLNER_POWER_RAWCALL)
   printm ("ReadToellnerPower raw access of /dev/ttyS0...");
   QString setremote("SYST:REM\n");
   QString askvoltage("MV?\n");
   QString askampere("MC?\n");

   // open for write test:

   int fd; /* File descriptor for the port */

   fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1)
     {
      /* Could not open the port. */
       printm("ReadToellnerPower: Unable to open /dev/ttyS0 - ");
     }
     else
     {
       //fcntl(fd, F_SETFL, 0);
        printm("ReadToellnerPower writes %s ", askvoltage.toLatin1 ().constData ());
       int bytes= askvoltage.size() +1;
       int n = write(fd, askvoltage.toLatin1 ().constData (), bytes);
       //int n = write(fd, "MV?\n", 4);
         printm("ReadToellnerPower wanted to write %d bytes, returned:%d",bytes,n);
         sleep(1);
         bytes= askampere.size() +1;
       printm("ReadToellnerPower writes %s ", askampere.toLatin1 ().constData ());
       n = write(fd, askampere.toLatin1 ().constData (), bytes);
       //n = write(fd, "MC?\n", 4);
       printm("ReadToellnerPower wanted to write %d bytes, returned:%d",bytes,n);

       ::close(fd);
     }

   // note that this mode was only used for testing command sending. DEBUG by external cat



#else


   printm ("ReadToellnerPower accesses /dev/ttyS0...");
   QString setremote("SYST:REM\n");
   QString askvoltage("MV?\n");
   QString askampere("MC?\n"); // 2017 works with backslash n
   QFile file("/dev/ttyS0");
   if (!file.open(QIODevice::WriteOnly))
   {
       printm ("ReadToellnerPower error when opening /dev/ttyS0 for writing");
       return;
   }
   QFile infile("/dev/ttyS0");
      if (!infile.open(QIODevice::ReadOnly))// | QIODevice::Unbuffered ))// | QIODevice::Text))
      {
          printm ("ReadToellnerPower error when opening /dev/ttyS0 for reading");
          return;
      }

     QTextStream tty(&file);
     tty << setremote;
     tty.flush();
     sleep(1);
     //usleep(1000);
     printm ("ReadToellnerPower has send:%s",setremote.toLatin1 ().constData ());
     tty <<  askvoltage;
     tty.flush();
     sleep(1); // need sleep when working without printm, otherwise response is highly crunched
     printm ("ReadToellnerPower has send:%s",askvoltage.toLatin1 ().constData ());
     // note that sending works, we can receive response in cat /dev/ttyS0 process!

     QString answer("");
     answer=infile.readLine();
     infile.readLine(); // skip blank line after value!
     u=answer.toDouble();
     printm("ReadToellnerPower answer was:%s", answer.toLatin1 ().constData ());

     //sleep(1);
     tty <<  askampere;
     tty.flush();
     sleep(1);
     printm ("ReadToellnerPower has send:%s",askampere.toLatin1 ().constData ());
     answer="";
     answer=infile.readLine();
     infile.readLine(); // skip blank line after value!
     i=answer.toDouble();
     printm("ReadToellnerPower answer was %s",answer.toLatin1 ().constData ());

     printm("ReadToellnerPower gets Voltage=%f V, Current=%f A",u, i);
#endif

#endif

}


void ApfelGui::SetSingleChipCommID(int apfel, int id)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  int dat = APFEL_IO_CONTROL_WR;
  int mask = 0;
  int lo=APFEL_IO_DATA_LO_WR;
  int hi=APFEL_IO_DATA_HI_WR;

  // following words of high control word:
  int sel_VddAsic=0; // 8bit
  int sel_InExt=0;   // 8bit

  // following words of low control word:
  int sel_DataASIC=0xFF; // 8bit
  int chipID=id & 0xF; // 4bit



  sel_InExt=0xFF; // all externals to ground


  // note that we have to preserve the complete state of power switches here
  sel_VddAsic |= (1 << apfel);
  sel_DataASIC  &= ~(1 << apfel);

  // the common part:
  hi |= (sel_VddAsic & 0xFF) <<8;
  hi |= sel_InExt & 0xFF;
  lo |=(sel_DataASIC & 0xFF) <<8;
  lo |= (chipID & 0xF) << 4;

  // lsb for gain setup, this time we also activate bit 1 - SelIDAsic
  if(theSetup->IsHighGain())
    lo |= (0x000e);
  else
    lo |= (0x000f);



  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, hi);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, lo);

   dat |= (0x18); // default setup: all enabled
   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
   APFEL_ADDRESSTEST_SLEEP

   //sleep(1);
}



void ApfelGui::SetSingleChipCurrentMode(int apfel, bool selectHV, bool selectDiode)

{

  theSetup_GET_FOR_SLAVE(BoardSetup);

  // first find out which other chips should have power on:
  int powermask=0;
  for(int a=0; a<APFEL_NUMCHIPS; ++a)
      {
        if(theSetup->HasApfelPower(a)) powermask |= (1 << a);
      }


  int dat = APFEL_IO_CONTROL_WR;
  int mask = 0;
  int lo=APFEL_IO_DATA_LO_WR;
  int hi=APFEL_IO_DATA_HI_WR;

  // following words of high control word:
  int sel_VddAsic=0; // 8bit
  int sel_InExt=0xFF;   // 8bit - all to ground except for this apfel

  // following words of low control word:
  int sel_DataASIC=0xFF; // 8bit
  int chipID=0; // 4bit, use id 0 to compensate addressing differences on chip (Sven?)



  sel_InExt &=~(1 << apfel); // enable only this apfel


  sel_VddAsic=powermask; // always enable/disable other chips depending on power state
  if(selectHV && selectDiode)
  {
    // in this mode want to clear the relevant bit
    sel_VddAsic  &= ~(1 << apfel);
  }
  else
  {
    // maybe redundant, since our bit should be already set by powermask
    sel_VddAsic |= (1 << apfel);
  }



  sel_DataASIC  &= ~powermask; // always enable/disable other chips depending on power state
  sel_DataASIC  &= ~(1 << apfel); // also redundant?

  // the common part:
  hi |= (sel_VddAsic & 0xFF) <<8;
  hi |= sel_InExt & 0xFF;
  lo |=(sel_DataASIC & 0xFF) <<8;
  lo |= (chipID & 0xF) << 4;

  // lsb for gain setup: this time we _do_ use bit 1 - SelIDAsic, always set id 0 here because of pull up resistors on chip (?Sven)
  if(theSetup->IsHighGain())
    lo |= (0x0003);
  else
    lo |= (0x0002);

  // finally set the hv/diode switches:
  char selHV= selectHV ? 0x1 : 0x0;
  char selDiode=selectDiode ? 0x1: 0x0;
  lo |= ((selHV << 3) & 0xF);
  lo |= ((selDiode << 2) & 0xF);


  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, hi);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, lo);

   dat |= (0x18); // default setup: all enabled
   WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
   APFEL_ADDRESSTEST_SLEEP

}






  /** Perform ID Scan for apfel chip at given slot */
void ApfelGui::ExecuteIDScanTest(int apfel)
  {
    theSetup_GET_FOR_SLAVE(BoardSetup);
    if(!theSetup->IsApfelPresent(apfel)) return;
    int position=apfel+1; // same numbering as on gui
    printm("Starting Address Scan tests for position %d ...",position);
    QApplication::setOverrideCursor (Qt::WaitCursor);
    bool idscanok=true;
    for(int id=0; id<16; ++id)
    {
      SetSingleChipCommID(apfel, id);
      printm("Starting ID Scan for position %d with assigned id:%d",apfel,id);
      int val=0x010 | (id & 0xF);
      WriteDAC_ApfelI2c_ToID (id, 0, val);
     APFEL_ADDRESSTEST_SLEEP
      int rev= (ReadDAC_ApfelI2c_FromID (id, 0) & APFEL_DAC_MAXVALUE);
      if(rev!=val)
        {
          idscanok=false;
          printm("!! Failure reading back from apfel%d, assigned id:%d - wrote value 0x%x, read: 0x%x",apfel,id, val, rev);
        }

    }
    APFEL_ADDRESSTEST_SLEEP
    theSetup->SetIDScan(apfel,idscanok);

    bool generalscanok=true;
  for (int id = 0; id < 16; ++id)
  {
    SetSingleChipCommID (apfel, id);
    printm ("Starting General call test for position %d with assigned id:%d ", position, id);
    int val = 0x310 | (id & 0xF);
    WriteDAC_ApfelI2c_ToID (0xFF, 0, val);    // broadcast to id 0xFF
   APFEL_ADDRESSTEST_SLEEP
    int rev = (ReadDAC_ApfelI2c_FromID (id, 0) & APFEL_DAC_MAXVALUE);
    if (rev != val)
    {
      generalscanok = false;
      printm ("!! Failure reading back from apfel%d, assigned id:%d - wrote value 0x%x, read: 0x%x", apfel, id, val,
          rev);
    }

  }
  APFEL_ADDRESSTEST_SLEEP
  theSetup->SetGeneralScan(apfel, generalscanok);



 bool reverseidscanok=true;
 for (int id = 0; id < 16; ++id)
 {
   SetSingleChipCommID (apfel, id);
   printm("Starting reverse id scan test for position %d with assigned id:%d",position, id);
   int val = 0x2f0 | (id & 0xF);

   WriteDAC_ApfelI2c_ToID (id, 0, val);   // test value to our device
  APFEL_ADDRESSTEST_SLEEP
   for (int other = 0; other < 16; ++other)
   {
     if(other==id) continue;
     int otherval= 0x00 | (other & 0xF);
     WriteDAC_ApfelI2c_ToID (other, 0, otherval);   // try writing something to other address
    APFEL_ADDRESSTEST_SLEEP
   }
   int rev = (ReadDAC_ApfelI2c_FromID (id, 0)  & APFEL_DAC_MAXVALUE);
   if (rev != val)
   {
     reverseidscanok= false;
     printm ("!! Failure reading back from apfel %d, assigned id:%d - wrote value 0x%x, read: 0x%x", apfel, id, val,
         rev);
   }

 }
 APFEL_ADDRESSTEST_SLEEP
 theSetup->SetReverseIDScan(apfel, reverseidscanok);



 printm("Starting register access  test for position %d ...",position);
 bool registertestok=true;
 SetSingleChipCommID (apfel, 0); // use first id only
 std::vector<int> firstvals;
 firstvals.push_back(0x15c);
 firstvals.push_back(0x155);
 firstvals.push_back(0x15a);
 firstvals.push_back(0x153);
 std::vector<int> secondvals;
 secondvals.push_back(0x2a3);
 secondvals.push_back(0x2aa);
 secondvals.push_back(0x2a5);
 secondvals.push_back(0x2ac);

 for(int dac=0; dac<APFEL_NUMDACS; ++dac)
 {
   WriteDAC_ApfelI2c_ToID (0, dac, firstvals[dac]);
   APFEL_ADDRESSTEST_SLEEP
 }

 for(int dac=0; dac<APFEL_NUMDACS; ++dac)
  {

   int rev = (ReadDAC_ApfelI2c_FromID (0, dac)  & APFEL_DAC_MAXVALUE);
   if (rev != firstvals[dac])
     {
     registertestok= false;
       printm ("!! Registertest Failure reading back from apfel %d - wrote value 0x%x, read: 0x%x", apfel, firstvals[dac],
           rev);
     }

    APFEL_ADDRESSTEST_SLEEP
  }

 for(int dac=0; dac<APFEL_NUMDACS; ++dac)
  {
    WriteDAC_ApfelI2c_ToID (0, dac, secondvals[dac]);
    APFEL_ADDRESSTEST_SLEEP
  }

 for(int dac=0; dac<APFEL_NUMDACS; ++dac)
   {

    int rev = (ReadDAC_ApfelI2c_FromID (0, dac)  & APFEL_DAC_MAXVALUE);
    if (rev != secondvals[dac])
      {
      registertestok= false;
        printm ("!! Registertest Failure reading back from apfel %d - wrote value 0x%x, read: 0x%x", apfel, firstvals[dac],
            rev);
      }
     APFEL_ADDRESSTEST_SLEEP
   }



  theSetup->SetRegisterScan(apfel, registertestok);


  printm("Everything is done for position %d ...",position);

    QApplication::restoreOverrideCursor ();
    RefreshIDScan(apfel);
  }


void ApfelGui::ExecuteCurrentScan (int apfel)
{
  theSetup_GET_FOR_SLAVE(BoardSetup);
  if (!theSetup->IsApfelPresent (apfel))
    return;
  int position = apfel + 1;    // same numbering as on gui
  double val=-1;
  QApplication::setOverrideCursor (Qt::WaitCursor);
  bool fake=fApfelWidget->CurrentTestCheckBox->isChecked();
  printm ("Starting %s ASIC current measurement  for position %d ...", (fake ? "FAKE" : "Multimeter") ,position);
  SetSingleChipCurrentMode (apfel, true, true);    // hv bit on, diode bit on  -> enable asic switch
  usleep(200000);
  val = (fake ? (position * 1.0E-3) : ReadKeithleyCurrent ());
  printm ("          - got %E A", val);
  theSetup->SetCurrentASIC (apfel, val);
  //SetDefaultIOConfig ();    // back to normal operation

  printm ("Starting %s HV current measurement for position %d ...", (fake ? "FAKE" : "Multimeter"), position);
  SetSingleChipCurrentMode (apfel, false, true);    // hv bit off, diode bit on      -> enable HV switch
  usleep(200000);
  val = (fake ? (position * 1.0E-9) : ReadKeithleyCurrent ());
  printm ("          - got %E A", val);
  theSetup->SetCurrentHV(apfel, val);
  //SetDefaultIOConfig ();    // back to normal operation

  printm ("Starting %s Diode current measurement for position %d ...", (fake ? "FAKE" : "Multimeter"), position);
  SetSingleChipCurrentMode (apfel, false, false);    // hv bit off, diode bit off     -> enable Diode switch
  usleep(200000);
  //val = (fake ? (position * 1.0E-6) : ReadKeithleyCurrent ());
  val = (fake ? (position * 1.0E-9) : ReadKeithleyCurrent ());
  printm ("          - got %E A", val);
  theSetup->SetCurrentDiode(apfel, val);
  SetDefaultIOConfig ();    // back to normal operation
  QApplication::restoreOverrideCursor ();
  RefreshCurrents (apfel);
}
