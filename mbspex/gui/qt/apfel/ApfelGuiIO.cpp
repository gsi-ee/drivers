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

void ApfelGui::InitApfel ()
{
  printm ("Resetting APFEL for SFP %d Slave %d...", fSFP, fSlave);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, APFEL_RESET);
  sleep (1);

#ifdef DO_APFEL_INIT
  WriteGosip (fSFP, fSlave, DATA_FILT_CONTROL_REG, 0x00);
  usleep (4000);

//        // disable test data length
  WriteGosip (fSFP, fSlave, REG_DATA_LEN, 0x10000000);

//        // specify trace length in slices
  WriteGosip (fSFP, fSlave, REG_FEB_TRACE_LEN, FEB_TRACE_LEN);
  // note: we skip verify read here to let this work in broadcast mode!

//        // specify trigger delay in slices
  WriteGosip (fSFP, fSlave, REG_FEB_TRIG_DELAY, FEB_TRIG_DELAY);
  // note: we skip verify read here to let this work in broadcast mode!

  //        // disable trigger acceptance in febex
  WriteGosip (fSFP, fSlave, REG_FEB_CTRL, 0);

  //        // enable trigger acceptance in febex
  WriteGosip (fSFP, fSlave, REG_FEB_CTRL, 1);

//        // set channels used for self trigger signal

  // JAM: the following is reduced version of mbs sample code. instead of arrays for each slave, we just
  // take settings for first device at sfp 0. Should be sufficient for baseline setup until mbs configures all?
  long l_sfp0_feb_ctrl0 = 0x01000000;
  long l_sfp0_feb_ctrl1 = 0x0;
  long l_sfp0_feb_ctrl2 = 0xffff;
  long l_ev_od_or = (l_sfp0_feb_ctrl0 >> 20) & 0x1;
  long l_pol = (l_sfp0_feb_ctrl0 >> 28) & 0x1;
  long l_trig_mod = (l_sfp0_feb_ctrl0 >> 24) & 0xf;
  long l_dis_cha = l_sfp0_feb_ctrl0 & 0x1ffff;
  long l_dat_redu = l_sfp0_feb_ctrl1 & 0x1ffff;
  long l_ena_trig = l_sfp0_feb_ctrl2 & 0xffff;

  int trigenabchan = ((l_ev_od_or << 21) | (l_pol << 20) | (l_trig_mod << 16) | l_ena_trig);
  WriteGosip (fSFP, fSlave, REG_FEB_SELF_TRIG, trigenabchan);

//        // set the step size for self trigger and data reduction
  long l_thresh = 0x1ff;
  for (int l_k = 0; l_k < APFEL_ADC_CHANNELS; l_k++)
  {
    WriteGosip (fSFP, fSlave, REG_FEB_STEP_SIZE, (l_k << 24) | l_thresh);
  }

  //
//        // reset the time stamp and set the clock source for time stamp counter
  if (fSFP == 0 && fSlave == 0)    // assume clock source at first slave on first chain here
  {
    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x0);
    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x7);
  }
  else
  {
    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x0);
    WriteGosip (fSFP, fSlave, REG_FEB_TIME, 0x5);
  }

  //
//        // enable/disable no hit in trace data suppression of channel
  WriteGosip (fSFP, fSlave, REG_DATA_REDUCTION, l_dat_redu);

//        // set channels used for self trigger signal
  WriteGosip (fSFP, fSlave, REG_MEM_DISABLE, l_dis_cha);

//        // write SFP id for channel header

  WriteGosip (fSFP, fSlave, REG_HEADER, fSFP);

//        // set trapez parameters for trigger/hit finding
  WriteGosip (fSFP, fSlave, TRIG_SUM_A_REG, TRIG_SUM_A);
  WriteGosip (fSFP, fSlave, TRIG_GAP_REG, TRIG_SUM_A + TRIG_GAP);
  WriteGosip (fSFP, fSlave, TRIG_SUM_B_REG, TRIG_SUM_A + TRIG_GAP + TRIG_SUM_B);

#ifdef ENABLE_ENERGY_FILTER
#ifdef TRAPEZ
//
//        // set trapez parameters for energy estimation
  WriteGosip (fSFP, fSlave, ENERGY_SUM_A_REG, ENERGY_SUM_A);
  WriteGosip (fSFP, fSlave, ENERGY_GAP_REG, ENERGY_SUM_A + ENERGY_GAP);
  WriteGosip (fSFP, fSlave, ENERGY_SUM_B_REG, ENERGY_SUM_A + ENERGY_GAP + ENERGY_SUM_B);

#endif // TRAPEZ
#endif // ENABLE_ENERGY_FILTER
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

  BoardSetup& theSetup = fSetup[fSFP].at (fSlave);    // check for indices is done in broadcast action macro that calls this function

  SetIOSwitch ();
  for (uint8_t apf = 0; apf < APFEL_NUMCHIPS; ++apf)
  {
    for (uint8_t dac = 0; dac < APFEL_NUMDACS; ++dac)
    {
      int val = theSetup.GetDACValue (apf, dac);
      WriteDAC_ApfelI2c (apf, dac, val);
      //std::cout << "SetRegisters DAC(" << apf <<"," << dac << ") val=" <<  val << std::endl;
    }

    for (uint8_t ch = 0; ch < APFEL_NUMCHANS; ++ch)
    {
      // here set gain factors for each channel:
      SetGain (apf, ch, theSetup.GetLowGain (apf, ch));

    }

    // no use to apply pulser here, since it will start a single pulse
    //SetPulser(apf);
  }

  DisableI2C ();
  QApplication::restoreOverrideCursor ();

}

void ApfelGui::SetIOSwitch ()
{
  BoardSetup& theSetup = fSetup[fSFP].at (fSlave);
  //std::cout << "SetIOSwitch: apfel=" << theSetup.IsApfelInUse() <<", highgain=" << theSetup.IsHighGain() << ", stretcher="<< theSetup.IsStretcherInUse()<<")"<< std::endl;
  SetSwitches (theSetup.IsApfelInUse (), theSetup.IsHighGain (), theSetup.IsStretcherInUse ());

}

void ApfelGui::SetPulser (uint8_t apf)
{
  BoardSetup& theSetup = fSetup[fSFP].at (fSlave);
  // here set test pulser properties. we must use both channels simultaneously:
  uint8_t amp1 = 0, amp2 = 0;
  bool on_1 = theSetup.GetTestPulseEnable (apf, 0);
  bool on_2 = theSetup.GetTestPulseEnable (apf, 1);
  bool on_any = on_1 || on_2;

  if (on_1)
    amp1 = theSetup.GetTestPulseAmplitude (apf, 0);
  if (on_2)
    amp2 = theSetup.GetTestPulseAmplitude (apf, 1);
  SetTestPulse (apf, on_any, amp1, amp2, theSetup.GetTestPulsePositive (apf));
}

void ApfelGui::SetBroadcastPulser ()
{
  bool on = PulserCheckBox_all->isChecked ();
  uint8_t amp = PulserAmpSpinBox_all->value ();
  bool positive = (ApfelTestPolarityBox_all->currentIndex () == 0);
  SetTestPulse (0xFF, on, amp, amp, positive);

}

void ApfelGui::GetDACs (int chip)
{
  BoardSetup& theSetup = fSetup[fSFP].at (fSlave);

  for (int dac = 0; dac < APFEL_NUMDACS; ++dac)
  {

    int val = ReadDAC_ApfelI2c (chip, dac);
    //std::cout << "GetDACs(" << chip <<"," << dac << ") val=" << val << std::endl;

    if (val < 0)
    {
      AppendTextWindow ("GetDacs has error!");
      return;    // TODO error message
    }
    theSetup.SetDACValue (chip, dac, val);
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


void ApfelGui::GetSFPChainSetup ()
{
//  std::cout<<"GetSFPChainSetup... "<< std::endl;
#ifdef USE_MBSPEX_LIB
  // broadcast mode: find out number of slaves and loop over all registered slaves
  mbspex_get_configured_slaves(fPexFD, &fSFPChains);
#else
  // without mbspex lib, we just assume 4 devices for each chain:
  for (int sfp = 0; sfp < 4; ++sfp)
  {
    fSFPChains.numslaves[sfp] = 4;
  }
#endif

  // dynamically increase array of setup structures:
  for (int sfp = 0; sfp < 4; ++sfp)
  {
    while (fSetup[sfp].size () < fSFPChains.numslaves[sfp])
    {
      fSetup[sfp].push_back (BoardSetup ());
      // TODO: evaluate real mapping of apfel chips here!

      //std::cout<<"GetSFPChainSetup increased setup at sfp "<<sfp<<" to "<<fSetup[sfp].size()<<" slaves." << std::endl;
    }
  }

}

int ApfelGui::ReadDAC_ApfelI2c (uint8_t apfelchip, uint8_t dac)
{
  int val = 0;
  if (apfelchip >= APFEL_NUMCHIPS)
  {
    AppendTextWindow ("#Error: ReadDAC_ApfelI2c with illegal chip number!");
    return -1;
  }
  int apid = GetApfelId (fSFP, fSlave, apfelchip);

  // first: read transfer request from apfel chip with id to core register
  int dat = APFEL_TRANSFER_BASE_RD + (dac + 1) * APFEL_TRANSFER_DAC_OFFSET + (apid & 0xFF);
  // mind that dac index starts with 0 for dac1 here!

  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);
  // note that WriteGosip already contains i2csleep

  // second: read request from core registers
  dat = APFEL_DAC_REQUEST_BASE_RD + (dac) * APFEL_CORE_REQUEST_DAC_OFFSET;
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // third: actually read requested value
  val = ReadGosip (fSFP, fSlave, GOS_I2C_DRR1);    // read out the value
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
  return (val & APFEL_DAC_MAXVALUE);    // mask to use only data part
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

  // first write value to core register:
  int dat = APFEL_CORE_REQUEST_BASE_WR + dac * APFEL_CORE_REQUEST_DAC_OFFSET;
  dat |= (value & 0x3FF);
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

  // then request for data transfer:
  dat = APFEL_TRANSFER_BASE_WR + (dac + 1) * APFEL_TRANSFER_DAC_OFFSET + (apfelid & 0xFF);
  // mind that dac index starts with 0 for dac1 here!
  WriteGosip (fSFP, fSlave, GOS_I2C_DWR, dat);

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
  usleep (8000);
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
  usleep (8000);
  QApplication::restoreOverrideCursor ();
}


void ApfelGui::SetSwitches (bool useApfel, bool useHighGain, bool useStretcher)
{
  int dat = APFEL_IO_CONTROL_WR;
  int mask = 0;
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




void ApfelGui::ReadToellnerPower(double& u, double& i)
{

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

}

int ApfelGui::ReadGosip (int sfp, int slave, int address)
{
  int value = -1;
#ifdef USE_MBSPEX_LIB
  int rev = 0;
  long int dat = 0;
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = mbspex_slave_rd (fPexFD, sfp, slave, address, &dat);
  I2c_sleep ();
  value = dat;
  if (fDebug)
  {
    char buffer[1024];
    if (rev == 0)
    {
      snprintf (buffer, 1024, "mbspex_slave_rd(%d,%d 0x%x) -> 0x%x", sfp, slave, address, value);
    }
    else
    {
      snprintf (buffer, 1024, "ERROR %d from mbspex_slave_rd(%d,%d 0x%x)", rev, sfp, slave, address);
    }
    QString msg (buffer);
    AppendTextWindow (msg);

  }
  //QApplication::restoreOverrideCursor ();
#else
  char buffer[1024];
//snprintf(buffer,1024,"/daq/usr/adamczew/workspace/drivers/mbspex/bin/gosipcmd -r -- %d %d 0x%x",sfp, slave, address);
  snprintf (buffer, 1024, "gosipcmd -r -- %d %d 0x%x", sfp, slave, address);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  if (result != "ERROR")
  {
    QString pre, valtext;
    fNumberBase == 16 ? pre = "0x" : pre = "";
    //DebugTextWindow (result);
    value = result.toInt (0, 0);
    valtext = pre + valtext.setNum (value, fNumberBase);
    DebugTextWindow (valtext);

  }
  else
  {

    value = -1;
  }
#endif

  return value;
}




int ApfelGui::WriteGosip (int sfp, int slave, int address, int value)
{
  int rev = 0;
//std::cout << "#WriteGosip" << std::endl;
  if (fSaveConfig)
    return SaveGosip (sfp, slave, address, value);

#ifdef USE_MBSPEX_LIB
  //QApplication::setOverrideCursor (Qt::WaitCursor);
  rev = mbspex_slave_wr (fPexFD, sfp, slave, address, value);
  I2c_sleep ();
  if (fDebug)
  {
    char buffer[1024];
    snprintf (buffer, 1024, "mbspex_slave_wr(%d,%d 0x%x 0x%x)", sfp, slave, address, value);
    QString msg (buffer);
    AppendTextWindow (msg);
  }
  //QApplication::restoreOverrideCursor ();
#else
  char buffer[1024];
  snprintf (buffer, 1024, "gosipcmd -w -- %d %d 0x%x 0x%x", sfp, slave, address, value);
  QString com (buffer);
  QString result = ExecuteGosipCmd (com);
  if (result == "ERROR")
    rev = -1;
#endif

  return rev;
}

QString ApfelGui::ExecuteGosipCmd (QString& com, int timeout)
{
// interface to shell gosipcmd
// TODO optionally some remote call via ssh for Go4 gui?
  QString result;
  QProcess proc;
  DebugTextWindow (com);
#if QT_VERSION >= QT_VERSION_CHECK(4,6,0)
  proc.setProcessEnvironment (fEnv);
#endif
  proc.setReadChannel (QProcess::StandardOutput);
  QApplication::setOverrideCursor (Qt::WaitCursor);

  proc.start (com);
// if(proc.waitForReadyRead (1000)) // will give termination warnings after leaving this function
  if (proc.waitForFinished (timeout))    // after process is finished we can still read stdio buffer
  {
    // read back stdout of proc here
    result = proc.readAll ();
  }
  else
  {
    std::stringstream buf;
    buf << "! Warning: ExecuteGosipCmd not finished after " << timeout / 1000 << " s timeout !!!" << std::endl;
    std::cout << " ApfelGui: " << buf.str ().c_str ();
    AppendTextWindow (buf.str ().c_str ());
    result = "ERROR";
  }
  QApplication::restoreOverrideCursor ();
  return result;
}
