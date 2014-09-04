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
#include "pexorplugin/Device.h"

#include "dabc/Command.h"
#include "dabc/logging.h"
#include "dabc/Manager.h"

dabc::FactoryPlugin pexorpluginfactory (new pexorplugin::Factory ("pexorplugin"));


dabc::Module* pexorplugin::Factory::CreateModule (const std::string& classname, const std::string& modulename,
    dabc::Command cmd)
{
  DOUT0 ("pexorplugin::Factory::CreateModule called for class:%s, module:%s", classname.c_str (), modulename.c_str ());

  if (strcmp (classname.c_str (), "pexorplugin::ReadoutModule") == 0)
  {
    dabc::Module* mod = new pexorplugin::ReadoutModule (modulename, cmd);
    unsigned int boardnum = 0;    //cmd->GetInt(ABB_PAR_BOARDNUM, 0);
    DOUT1 ("pexorplugin::Factory::CreateModule - Created PEXOR Readout module %s for /dev/pexor-%d",
        modulename.c_str (), boardnum);
    return mod;
  }
  return dabc::Factory::CreateModule (classname, modulename, cmd);
}

dabc::Device* pexorplugin::Factory::CreateDevice (const std::string& classname, const std::string& devname,
    dabc::Command cmd)
{
  DOUT0 ("pexorplugin::Factory::CreateDevicee called for class:%s, device:%s", classname.c_str (), devname.c_str ());

  if (strcmp (classname.c_str (), "pexorplugin::Device") != 0)
    return 0;

  dabc::Device* dev = new pexorplugin::Device (devname, cmd);

  return dev;
}


