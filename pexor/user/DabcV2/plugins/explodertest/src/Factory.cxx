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
#include "explodertest/Factory.h"
#include "explodertest/Device.h"

#include "dabc/Command.h"
#include "dabc/logging.h"
#include "dabc/Manager.h"


dabc::FactoryPlugin explodertestfactory(new explodertest::Factory("explodertest"));



dabc::Device* explodertest::Factory::CreateDevice(const std::string& classname, const std::string& devname, dabc::Command cmd)
{
   DOUT0("explodertest::Factory::CreateDevice called for class:%s, device:%s", classname.c_str(), devname.c_str());

   if (strcmp(classname.c_str(),"explodertest::Device")!=0) return 0;
    dabc::Device* dev=new explodertest::Device(devname, cmd);
   return dev;
}






