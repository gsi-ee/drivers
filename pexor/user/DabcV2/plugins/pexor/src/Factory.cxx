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
#include "pexorplugin/Factory.h"
#include "pexorplugin/ReadoutModule.h"
#include "pexorplugin/ReadoutApplication.h"
#include "pexorplugin/Device.h"

#include "dabc/Command.h"
#include "dabc/logging.h"
#include "dabc/Manager.h"


dabc::FactoryPlugin pexorpluginfactory(new pexorplugin::Factory("pexorplugin"));



//dabc::Application* pexorplugin::Factory::CreateApplication(const std::string classname, dabc::Command cmd)
//{
//   if (strcmp(classname, pexorplugin::nameReadoutAppClass)==0)
//      return new pexorplugin::ReadoutApplication();
//
//   return dabc::Factory::CreateApplication(classname, cmd);
//}


dabc::Module* pexorplugin::Factory::CreateModule(const std::string& classname, const std::string& modulename, dabc::Command cmd)
{
    DOUT0("pexorplugin::Factory::CreateModule called for class:%s, module:%s", classname.c_str(), modulename.c_str());

    if (strcmp(classname.c_str(),"pexorplugin::ReadoutModule")==0)
       {
           dabc::Module* mod= new pexorplugin::ReadoutModule(modulename, cmd);
           unsigned int boardnum=0; //cmd->GetInt(ABB_PAR_BOARDNUM, 0);
           DOUT1("pexorplugin::Factory::CreateModule - Created PEXOR Readout module %s for /dev/pexor%d", modulename.c_str(), boardnum);
           return mod;
         }
    return dabc::Factory::CreateModule(classname, modulename, cmd);
}


dabc::Device* pexorplugin::Factory::CreateDevice(const std::string& classname, const std::string& devname, dabc::Command cmd)
{
   DOUT0("pexorplugin::Factory::CreateDevicee called for class:%s, device:%s", classname.c_str(), devname.c_str());

   if (strcmp(classname.c_str(),"pexorplugin::Device")!=0) return 0;

    dabc::Device* dev=new pexorplugin::Device(devname, cmd);
    //dabc::mgr()->MakeThreadFor(dev, cmd.GetStr(xmlDeviceThread, "devicethread"));

   return dev;
}


//dabc::DataInput* pexorplugin::Factory::CreateDataInput(const std::string& typ)
//{
//   dabc::Url url(typ);
//   if (url.GetProtocol()=="pexor") {
//      DOUT0("pexor data input name %s", url.GetFullName().c_str());
//      // TODO: find or create device helper object here
//      std::string devname = url.GetOptionStr("device", "pexor0")
//
//      pexorplugin::Device* dev=dabc::mgr.CreateDevice("pexorplugin::Device",devname);
//      // this will look for and use existing device of that name if any
//      return new pexorplugin::Transport(dev);
//   }
//
//   return 0;
//}
//
//
//
//
//dabc::Transport* pexorplugin::Factory::CreateTransport(const dabc::Reference& port, const std::string& typ, dabc::Command cmd)
//{
//   dabc::Url url(typ);
//
//   dabc::PortRef portref = port;
//
//   if (portref.IsInput() && (url.GetProtocol()=="pexor") && !url.GetHostName().empty()) {
//
//// TODO: evaluate parameters from connection url here
//     std::string devname = url.GetOptionStr("device", "pexor0")
//
//
//
//
////      int nport = url.GetPort();
////      int rcvbuflen = url.GetOptionInt("udpbuf", 2000000);
////      int mtu = url.GetOptionInt("mtu", 64512);
////      double flush = url.GetOptionDouble(dabc::xml_flush, 1.);
////      bool observer = url.GetOptionBool("observer", false);
////
////      if (nport>0) {
////
////         int fd = DataSocketAddon::OpenUdp(nport, rcvbuflen);
////
////         if (fd>0) {
////            DataSocketAddon* addon = new DataSocketAddon(fd, nport, mtu, flush);
////
////            return new hadaq::DataTransport(cmd, portref, addon, observer);
////         }
////      }
//      return new pexorplugin::Transport(cmd, portref);
//
//   }
//
//   return dabc::Factory::CreateTransport(port, typ, cmd);
//}





