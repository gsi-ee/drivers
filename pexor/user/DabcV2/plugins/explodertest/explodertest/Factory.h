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
#ifndef EXPLODERTEST_Factory
#define EXPLODERTEST_Factory

#include "dabc/Factory.h"


namespace explodertest {

   class Factory: public dabc::Factory  {
      public:

         Factory(const std::string name) : dabc::Factory(name) {}


         virtual dabc::Device* CreateDevice(const std::string& classname, const std::string& devname, dabc::Command com);
   };

}// namespace

#endif

