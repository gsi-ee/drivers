// $Id: Factory.h 3372 2015-11-12 13:47:24Z linev $

/************************************************************
 * The Data Acquisition Backbone Core (DABC)                *
 ************************************************************
 * Copyright (C) 2009 -                                     *
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH      *
 * Planckstr. 1, 64291 Darmstadt, Germany                   *
 * Contact:  http://dabc.gsi.de                             *
 ************************************************************
 * This software can be used under the GPL license          *
 * agreements as stated in LICENSE.txt file                 *
 * which is part of the distribution.                       *
 ************************************************************/

#ifndef PEXORNET_Factory
#define PEXORNET_Factory

#ifndef DABC_Factory
#include "dabc/Factory.h"
#endif

/** \brief Test plugin for pexornet driver udp readout  */

namespace pexornet {

   /** \brief %Factory for pexornet test classes  */

   class Factory : public dabc::Factory {
      public:
         Factory(const std::string& name) : dabc::Factory(name) {}


         virtual dabc::Module* CreateTransport(const dabc::Reference& port, const std::string& typ, dabc::Command cmd);


         virtual dabc::Module* CreateModule(const std::string& classname, const std::string& modulename, dabc::Command cmd);

         virtual void Initialize();

      protected:

   };

}

#endif
