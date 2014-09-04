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
#ifndef PEXORPLUGIN_ReadoutModule
#define PEXORPLUGIN_ReadoutModule

#include "dabc/ModuleAsync.h"

#include "dabc/statistic.h"

namespace pexorplugin
{

class ReadoutModule: public dabc::ModuleAsync
{

public:

  ReadoutModule (const std::string name, dabc::Command cmd);

  virtual void BeforeModuleStart ();
  virtual void AfterModuleStop ();

  virtual void ProcessInputEvent (unsigned port);
  virtual void ProcessOutputEvent (unsigned port);

protected:

  void DoPexorReadout ();
  std::string fEventRateName;
  std::string fDataRateName;

};
}

#endif
