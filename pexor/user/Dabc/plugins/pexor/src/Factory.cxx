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


pexorplugin::Factory pexorpluginfactory("pexorplugin");


dabc::Application* pexorplugin::Factory::CreateApplication(const char* classname, dabc::Command* cmd)
{
   if (strcmp(classname, pexorplugin::nameReadoutAppClass)==0)
      return new pexorplugin::ReadoutApplication();

   return dabc::Factory::CreateApplication(classname, cmd);
}


dabc::Module* pexorplugin::Factory::CreateModule(const char* classname, const char* modulename, dabc::Command* cmd)
{
    DOUT1(("pexorplugin::Factory::CreateModule called for class:%s, module:%s", classname, modulename));

    if ((classname==0) || (cmd==0)) return 0;
    if (strcmp(classname,"pexorplugin::ReadoutModule")==0)
       {
           dabc::Module* mod= new pexorplugin::ReadoutModule(modulename, cmd);
           unsigned int boardnum=0; //cmd->GetInt(ABB_PAR_BOARDNUM, 0);
           DOUT1(("pexorplugin::Factory::CreateModule - Created PEXOR Readout module %s for /dev/pexor%d", modulename, boardnum));
           return mod;
         }
//    else if (strcmp(classname,"pexorplugin::WriterModule")==0)
//       {
//           dabc::Module* mod = new pexorplugin::WriterModule(modulename, cmd);
//           unsigned int boardnum=cmd->GetInt(ABB_PAR_BOARDNUM, 0);
//           DOUT1(("pexorplugin::Factory::CreateModule - Created ABBWriter module %s for /dev/fpga%d", modulename, boardnum));
//           return mod;
//         }
    return 0;
}


dabc::Device* pexorplugin::Factory::CreateDevice(const char* classname, const char* devname, dabc::Command* cmd)
{
   if (strcmp(classname,"pexorplugin::Device")!=0) return 0;

    dabc::Device* dev=new pexorplugin::Device(dabc::mgr()->GetDevicesFolder(true), devname, cmd);
    dabc::mgr()->MakeThreadFor(dev, cmd->GetStr(xmlDeviceThread, "devicethread"));

   return dev;
}








