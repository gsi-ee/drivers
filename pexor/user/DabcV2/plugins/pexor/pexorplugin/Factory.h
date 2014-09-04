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
#ifndef PEXOR_Factory
#define PRXOR_Factory

#include "dabc/Factory.h"
#include "pexorplugin/Commands.h"

namespace pexorplugin
{

class Factory: public dabc::Factory
{
public:

  Factory (const std::string name) :
      dabc::Factory (name)
  {
  }

  virtual dabc::Module* CreateModule (const std::string& classname, const std::string& modulename, dabc::Command cmd);

  virtual dabc::Device* CreateDevice (const std::string& classname, const std::string& devname, dabc::Command com);
};

}    // namespace

#endif

