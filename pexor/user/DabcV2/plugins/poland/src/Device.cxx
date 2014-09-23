/********************************************************************
 * The Data Acquisition Backbone Core (DABC)
 ********************************************************************
 * Copyright (C) 2009- 
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH 
 * Planckstr. 1
 * 64291 Darmstadt
 * Germany
 * Contact:  http://dabc.gsi.de
 ********************************************************************
 * This software can be used under the GPL license agreements as stated
 * in LICENSE.txt file which is part of the distribution.
 ********************************************************************/
#include "poland/Device.h"
#include "dabc/Command.h"
#include "dabc/Manager.h"
#include "dabc/Application.h"

#include "mbs/MbsTypeDefs.h"
#include "dabc/Pointer.h"
#include "dabc/Port.h"
#include "dabc/DataIO.h"

#include "poland/Factory.h"

#include "pexor/DMA_Buffer.h"
#include "pexor/PexorTwo.h"

// address map for slave (exploder1): this is user specific data concering frontends , so it is not available from libmbspex
#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC
#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
#define REG_MEM_FLAG_1      0xFFFFBc  // read only:
#define REG_BUF0_DATA_LEN     0xFFFD00  // buffer 0 submemory data length
#define REG_BUF1_DATA_LEN     0xFFFE00  // buffer 1 submemory data length
#define REG_DATA_REDUCTION  0xFFFFB0  // Nth bit = 1 enable data reduction of  Nth channel from block transfer readout. (bit0:time, bit1-8:adc)
#define REG_MEM_DISABLE     0xFFFFB4  // Nth bit =1  disable Nth channel from block transfer readout.(bit0:time, bit1-8:adc)
#define REG_MEM_FLAG_0      0xFFFFB8  // read only:
#define REG_MEM_FLAG_1      0xFFFFBc  // read only:
#define REG_RST 0xFFFFF4
#define REG_LED 0xFFFFF8
#define REG_VERSION 0xFFFFFC

// qfw specific registers:
#define REG_CTRL       0x200000
#define REG_QFW_OFFSET_BASE 0x200100

const char* poland::xmlOffsetTriggerType = "PolandOffsetReadTrigger";    //< trigger type to read out frontend offfset values

poland::Device::Device (const std::string& name, dabc::Command cmd) :
    pexorplugin::Device (name, cmd), fOffsetTrigType (0)

{

  DOUT1("Constructing poland::Device...\n");
  if (!InitQFWs ())
  {
    EOUT("\n\nError initializing QFWs at pexor device %d \n", fDeviceNum);
    exit (1);    // TODO: how to tell DABC to shutdown properly?
  }
  fOffsetTrigType = Cfg (poland::xmlOffsetTriggerType, cmd).AsInt (14);

  fInitDone = true;
   // initial start acquisition here, not done from transport start anymore:
  StartAcquisition();

}

poland::Device::~Device ()
{
}

int poland::Device::User_Readout (dabc::Buffer& buf, uint8_t trigtype)
{
  //int res = dabc::di_Ok;
  int retsize = 0;
  if (trigtype == fOffsetTrigType)
  {
    // to do: special treatment of this trigger.
    DOUT1("poland::Device::User_Readout finds offset trigger :%d !!", trigtype);
    fBoard->ResetTrigger ();

    // this is nasty workaround since polands do not issue interrupts anymore
    // after stop acquisition interrupt. Note that mbs f_user.c for poland does the same!
    if (!InitQFWs ())
    {
      EOUT("\n\nError initializing QFWs  after start acquisition\n", fDeviceNum);
      exit (1);    // TODO: how to tell DABC to shutdown properly?
    }

    if (fMbsFormat)
    {
      unsigned int filled_size = 0, used_size = 0;
      mbs::EventHeader* evhdr = 0;
      mbs::SubeventHeader* subhdr = 0;

      // as default, we deliver empty event and subevent just marking trigger type and ids:
      dabc::Pointer ptr (buf);
      evhdr = PutMbsEventHeader (ptr, fNumEvents, trigtype);    // TODO: get current trigger type from trixor and set
      if (evhdr == 0)
        return dabc::di_SkipBuffer;    // buffer too small error
      used_size += sizeof(mbs::EventHeader);
      subhdr = PutMbsSubeventHeader (ptr, fSubeventSubcrate, fSubeventControl, fSubeventProcid);
      if (subhdr == 0)
        return dabc::di_SkipBuffer;    // buffer too small error
      used_size += sizeof(mbs::SubeventHeader);
      filled_size += sizeof(mbs::SubeventHeader);

      // here readout of offset registers:
      for (int sfp = 0; sfp < PEXORPLUGIN_NUMSFP; sfp++)
      {

        if (!fEnabledSFP[sfp])
          continue;
        for (unsigned int sl = 0; sl < fNumSlavesSFP[sfp]; ++sl)
        {
          DOUT0("oooooooo Offest readout for spf:%d slave:%d\n", sfp, sl);
          int l_cha_head = 0x42 + (sfp << 12) + (sl << 16) + (trigtype << 8);
          DOUT0("oooooooo Use data header :0x%x \n", l_cha_head);
          int* pl_dat = (int*) ptr ();
          *pl_dat = l_cha_head;
          ptr.shift (sizeof(int));
          used_size += sizeof(int);
          filled_size += sizeof(int);
          for (int l_k = 0; l_k < 32; l_k++)
          {
            unsigned long value = 0;

            if (fBoard->ReadBus (REG_QFW_OFFSET_BASE + 4 * l_k, value, sfp, sl)!=0)
            {
              EOUT("\n\nError in ReadBus: sfp %x slave %x\n", sfp, sl, REG_QFW_OFFSET_BASE + 4*l_k);
              return dabc::di_SkipBuffer;
            }

            DOUT0("oooooooo Read offset %d: 0x%x \n", l_k, value);
            pl_dat = (int*) ptr ();
            *pl_dat = value;
            ptr.shift (sizeof(int));
            used_size += sizeof(int);
            filled_size += sizeof(int);
          }                                        // value loop
        }                                        // slave loop
      }                                        // sfp loop
      subhdr->SetRawDataSize (filled_size - sizeof(mbs::SubeventHeader));
      evhdr->SetSubEventsSize (filled_size);
      buf.SetTypeId (mbs::mbt_MbsEvents);
      buf.SetTotalSize (used_size);
      retsize = used_size;
    }
    else
    {
      return dabc::di_SkipBuffer;
    }
  }
  else
  {
    // all other triggers are read out as default
    retsize = pexorplugin::Device::User_Readout (buf, trigtype);
    // optionally add here validity check on filled buffer as in mbs!
  }
  return retsize;

}

//int poland::Device::ExecuteCommand (dabc::Command cmd)
//{
//  DOUT1("poland::Device::ExecuteCommand-  %s", cmd.GetName ());
//  return pexorplugin::Device::ExecuteCommand (cmd);
//}
//
//unsigned poland::Device::Read_Start (dabc::Buffer& buf)
//{
//  return pexorplugin::Device::Read_Start (buf);
//  // here it is possible to fill different values into buffer as composed from gosip token readout
//
//}
//
//unsigned poland::Device::Read_Complete (dabc::Buffer& buf)
//{
//  // TODO: check trigger type and optionally get offset register values
//// case OFFSET_TRIGGER_TYPE:
////      // for the moment we use start acq trigger 14 here
////
////      for (l_i = 0; l_i < MAX_SFP; l_i++)
////      {
////        // read registers to subevent:
////        for (l_j = 0; l_j < l_sfp_slaves[l_i]; l_j++)
////        {
////          // put header here to indicate sfp and slave:
////             printm ("oooooooo Offest readout for spf:%d slave:%d\n", l_i, l_j);
////             l_cha_head=0x42 + (l_i << 12) + (l_j << 16) + (bh_trig_typ << 8);
////              printm ("oooooooo Use data header :0x%x \n", l_cha_head);
////            *pl_dat++= l_cha_head;
////          for (l_k = 0; l_k < 32; l_k++)
////          {
////            l_stat = f_pex_slave_rd (l_i, l_j, REG_QFW_OFFSET_BASE + 4*l_k, pl_dat++);
////            printm ("oooooooo Read offset %d: 0x%x \n", l_k, *(pl_dat - 1));
////          }
////        }
////      }
//// default:
//  return pexorplugin::Device::Read_Complete (buf);
//  // here it is possible to fill different values into buffer as composed from gosip token readout
//
//  // TODO: check meta data
//
//}

bool poland::Device::InitQFWs ()
{

  for (int sfp = 0; sfp < PEXORPLUGIN_NUMSFP; sfp++)
  {

    if (!fEnabledSFP[sfp])
      continue;
    for (unsigned int sl = 0; sl < fNumSlavesSFP[sfp]; ++sl)
    {
      // get submemory addresses to check setup:
      unsigned long base_dbuf0 = 0, base_dbuf1 = 0;
      unsigned long num_submem = 0, submem_offset = 0;
      unsigned long qfw_ctrl = 0;

      int rev = fBoard->ReadBus (REG_BUF0, base_dbuf0, sfp, sl);
      if (rev == 0)
      {
        DOUT1("Slave %x: Base address for Double Buffer 0  0x%x  \n", sl, base_dbuf0);
      }
      else
      {
        EOUT("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 0 address)\n", rev, sl, REG_BUF0);
        return false;
      }
      rev = fBoard->ReadBus (REG_BUF1, base_dbuf1, sfp, sl);
      if (rev == 0)
      {
        DOUT1("Slave %x: Base address for Double Buffer 1  0x%x  \n", sl, base_dbuf1);
      }
      else
      {
        EOUT("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 1 address)\n", rev, sl, REG_BUF1);
        return false;
      }
      rev = fBoard->ReadBus (REG_SUBMEM_NUM, num_submem, sfp, sl);
      if (rev == 0)
      {
        DOUT1("Slave %x: Number of Channels  0x%x  \n", sl, num_submem);
      }
      else
      {
        EOUT("\n\ntoken Error %d in ReadBus: slave %x addr %x (num submem)\n", rev, sl, REG_SUBMEM_NUM);
        return false;
      }
      rev = fBoard->ReadBus (REG_SUBMEM_OFF, submem_offset, sfp, sl);
      if (rev == 0)
      {
        DOUT1("Slave %x: Offset of Channel to the Base address  0x%x  \n", sl, submem_offset);
      }
      else
      {
        EOUT("\n\ncheck_token Error %d in ReadBus: slave %x addr %x (submem offset)\n", rev, sl, REG_SUBMEM_OFF);
        return false;
      }

      // disable test data length:
      rev = fBoard->WriteBus (REG_DATA_LEN, 0x10000000, sfp, sl);
      if (rev)
      {
        EOUT("\n\nError %d in WriteBus disabling test data length !\n", rev);
        return false;
      }

      // disable trigger acceptance in exploder2a

      rev = fBoard->WriteBus (REG_CTRL, 0, sfp, sl);
      if (rev)
      {
        EOUT("\n\nError %d in WriteBus disabling  trigger acceptance (sfp:%d, device:%d)!\n", rev, sfp, sl);
        return false;
      }
      rev = fBoard->ReadBus (REG_CTRL, qfw_ctrl, sfp, sl);
      if (rev == 0)
      {
        if ((qfw_ctrl & 0x1) != 0)
        {
          EOUT("Disabling trigger acceptance in qfw failed for sfp:%d device:%d!!!!!\n", sfp, sl);
          return false;
        }
      }
      else
      {
        EOUT("\n\nError %d in ReadBus: disabling  trigger acceptance (sfp:%d, device:%d)\n", rev, sfp, sl);
        return false;
      }

      // enable trigger acceptance in exploder2a:
      rev = fBoard->WriteBus (REG_CTRL, 1, sfp, sl);
      if (rev)
      {
        EOUT("\n\nError %d in WriteBus enabling  trigger acceptance (sfp:%d, device:%d)!\n", rev, sfp, sl);
        return false;
      }
      rev = fBoard->ReadBus (REG_CTRL, qfw_ctrl, sfp, sl);
      if (rev == 0)
      {
        if ((qfw_ctrl & 0x1) != 1)
        {
          EOUT("Enabling trigger acceptance in qfw failed for sfp:%d device:%d!!!!!\n", sfp, sl);
          return false;
        }
        DOUT1("Trigger acceptance is enabled for sfp:%d device:%d  \n", sfp, sl);
      }
      else
      {
        EOUT("\n\nError %d in ReadBus: enabling  trigger acceptance (sfp:%d, device:%d)\n", rev, sfp, sl);
        return false;
      }

      // write SFP id for channel header:
      rev = fBoard->WriteBus (REG_HEADER, sfp, sfp, sl);
      if (rev)
      {
        EOUT("\n\nError %d in WriteBus writing channel header (sfp:%d, device:%d)!\n", rev, sfp, sl);
        return false;
      }

      // enable frontend logic:
      // ? do we need setting to 0 before as in poland scripts?
      rev = fBoard->WriteBus (REG_RST, 1, sfp, sl);
      if (rev)
      {
        EOUT("\n\nError %d in WriteBus resetting qfw (sfp:%d, device:%d)!\n", rev, sfp, sl);
        return false;
      }
    }    // for slaves
  }    // for (int sfp = 0;

  return true;
}

