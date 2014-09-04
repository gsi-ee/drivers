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



//const char* poland::xmlExploderSubmem = "ExploderSubmemSize";    // size of exploder submem test buffer


poland::Device::Device (const std::string& name, dabc::Command cmd) :
    pexorplugin::Device (name, cmd)

{

  DOUT1 ("Constructing poland::Device...\n");
  if (!InitQFWs())
    {
      EOUT ("\n\nError initializing QFWs at pexor device %d \n", fDeviceNum);
      return;
    }

  fInitDone = true;
}

poland::Device::~Device ()
{
}

int poland::Device::ExecuteCommand (dabc::Command cmd)
{
  DOUT1 ("poland::Device::ExecuteCommand-  %s", cmd.GetName ());
  return pexorplugin::Device::ExecuteCommand (cmd);
}

unsigned poland::Device::Read_Start (dabc::Buffer& buf)
{
  return pexorplugin::Device::Read_Start (buf);
  // here it is possible to fill different values into buffer as composed from gosip token readout

}

unsigned poland::Device::Read_Complete (dabc::Buffer& buf)
{
  // TODO: check trigger type and optionally get offset register values
// case OFFSET_TRIGGER_TYPE:
//      // for the moment we use start acq trigger 14 here
//
//      for (l_i = 0; l_i < MAX_SFP; l_i++)
//      {
//        // read registers to subevent:
//        for (l_j = 0; l_j < l_sfp_slaves[l_i]; l_j++)
//        {
//          // put header here to indicate sfp and slave:
//             printm ("oooooooo Offest readout for spf:%d slave:%d\n", l_i, l_j);
//             l_cha_head=0x42 + (l_i << 12) + (l_j << 16) + (bh_trig_typ << 8);
//              printm ("oooooooo Use data header :0x%x \n", l_cha_head);
//            *pl_dat++= l_cha_head;
//          for (l_k = 0; l_k < 32; l_k++)
//          {
//            l_stat = f_pex_slave_rd (l_i, l_j, REG_QFW_OFFSET_BASE + 4*l_k, pl_dat++);
//            printm ("oooooooo Read offset %d: 0x%x \n", l_k, *(pl_dat - 1));
//          }
//        }
//      }
// default:
  return pexorplugin::Device::Read_Complete (buf);
  // here it is possible to fill different values into buffer as composed from gosip token readout



  // TODO: check meta data




}

bool poland::Device::InitQFWs ()
{
// TODO
//  void f_qfw_init ()
//
//  {
//  #ifndef USE_MBSPEX_LIB
//    PEXOR_Port_Monitor (&sPEXOR);
//  #endif
//    for (l_i=0; l_i<MAX_SFP; l_i++)
//    {
//      if (l_sfp_slaves[l_i] != 0)
//      {
//        l_stat = f_pex_slave_init (l_i, l_sfp_slaves[l_i]);
//        if (l_stat == -1)
//        {
//          printm (RON"ERROR>>"RES" slave address initialization failed \n");
//          printm ("exiting...\n");
//          exit (-1);
//        }
//      }
//      printm ("");
//    }
//
//    //sleep (1);
//
//    if (l_first == 0)
//    {
//      l_first = 1;
//      for (l_i=0; l_i<MAX_TRIG_TYPE; l_i++)
//      {
//        l_tr_ct[l_i] = 0;
//      }
//    }
//
//    for (l_i=0; l_i<MAX_SFP; l_i++)
//    {
//      if (l_sfp_slaves[l_i] != 0)
//      {
//        for (l_j=0; l_j<l_sfp_slaves[l_i]; l_j++)
//        {
//          // needed for check of meta data, read it in any case
//          printm ("SFP: %d, OFW/EXPLODER: %d \n", l_i, l_j);
//          // get address offset of qfw buffer 0,1 for each qfw/exploder
//          l_stat = f_pex_slave_rd (l_i, l_j, REG_BUF0, &(l_qfw_buf_off[l_i][l_j][0]));
//          l_stat = f_pex_slave_rd (l_i, l_j, REG_BUF1, &(l_qfw_buf_off[l_i][l_j][1]));
//          // get nr. of channels per qfw
//          l_stat = f_pex_slave_rd (l_i, l_j, REG_SUBMEM_NUM, &(l_qfw_n_chan[l_i][l_j]));
//          // get buffer per channel offset
//          l_stat = f_pex_slave_rd (l_i, l_j, REG_SUBMEM_OFF, &(l_qfw_chan_off[l_i][l_j]));
//
//          printm ("addr offset: buf0: 0x%x, buf1: 0x%x \n",
//                  l_qfw_buf_off[l_i][l_j][0], l_qfw_buf_off[l_i][l_j][1]);
//          printm ("No. channels: %d \n", l_qfw_n_chan[l_i][l_j]);
//          printm ("channel addr offset: 0x%x \n", l_qfw_chan_off[l_i][l_j]);
//
//          // disable test data length
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_DATA_LEN, 0x10000000);
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" disabling test data length failed\n");
//            l_err_prot_ct++;
//          }
//
//          // disable trigger acceptance in exploder2a
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_CTRL, 0);
//          if (l_stat == -1)
//    {
//            printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
//            l_err_prot_ct++;
//          }
//          l_stat = f_pex_slave_rd (l_i, l_j, REG_CTRL, &l_qfw_ctrl);
//          if ( (l_qfw_ctrl & 0x1) != 0)
//          {
//            printm (RON"ERROR>>"RES" disabling trigger acceptance in qfw failed, exiting \n");
//            l_err_prot_ct++;
//            exit (0);
//          }
//
//          // enable trigger acceptance in exploder2a
//
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_CTRL, 1);
//          if (l_stat == -1)
//    {
//            printm (RON"ERROR>>"RES" PEXOR slave write REG_FEB_CTRL failed\n");
//            l_err_prot_ct++;
//          }
//          l_stat = f_pex_slave_rd (l_i, l_j, REG_CTRL, &l_qfw_ctrl);
//          if ( (l_qfw_ctrl & 0x1) != 1)
//    {
//            printm (RON"ERROR>>"RES" enabling trigger acceptance in qfw failed, exiting \n");
//            l_err_prot_ct++;
//            exit (0);
//     }
//
//          // write SFP id for channel header
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_HEADER, l_i);
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" PEXOR slave write REG_HEADER  failed\n");
//            l_err_prot_ct++;
//          }
//
//          l_stat = f_pex_slave_wr (l_i, l_j, REG_RST, 1);
//          if (l_stat == -1)
//          {
//            printm (RON"ERROR>>"RES" PEXOR slave write REG_HEADER  failed\n");
//            l_err_prot_ct++;
//          }
//        }
//      }
//    }
//  }
//

  return true;
}

