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
#include "pexorplugin/ReadoutModule.h"
#include "pexorplugin/Commands.h"

#include "dabc/logging.h"
#include "dabc/Port.h"

pexorplugin::ReadoutModule::ReadoutModule (const std::string name, dabc::Command cmd) :
    dabc::ModuleAsync (name)
{
  EnsurePorts (1, 1, dabc::xmlWorkPool);
  std::string ratesprefix = "Pexor";

  fEventRateName = ratesprefix + "Events";
  fDataRateName = ratesprefix + "Data";
  CreatePar (fEventRateName).SetRatemeter (false, 3.).SetUnits ("Ev");
  CreatePar (fDataRateName).SetRatemeter (false, 1.).SetUnits ("Mb");

  Par (fDataRateName).SetDebugLevel (1);
  Par (fEventRateName).SetDebugLevel (1);

}

void pexorplugin::ReadoutModule::BeforeModuleStart ()
{
  DOUT1 ("pexorplugin::ReadoutModule::BeforeModuleStart");

}

void pexorplugin::ReadoutModule::AfterModuleStop ()
{
  DOUT1 ("pexorplugin::ReadoutModule finished. Rate %5.1f Mb/s", Par (fDataRateName).Value ().AsDouble ());

}

void pexorplugin::ReadoutModule::ProcessInputEvent (unsigned port)
{
  DoPexorReadout ();
}

void pexorplugin::ReadoutModule::ProcessOutputEvent (unsigned port)
{
  DoPexorReadout ();
}

void pexorplugin::ReadoutModule::DoPexorReadout ()
{
  dabc::Buffer ref;
  DOUT3 ("pexorplugin::DoPexorReadout\n");
  try
  {
    while (CanRecv ())
    {
      if (!CanSendToAllOutputs ())
      {
        DOUT3 (("pexorplugin::ReadoutModule::DoPexorReadout - can not send to all outputs. skip event \n"));
        return;
      }
      ref = Recv ();
      if (!ref.null ())
      {
        Par (fDataRateName).SetValue (ref.GetTotalSize () / 1024. / 1024.);
        SendToAllOutputs (ref);
        Par (fEventRateName).SetValue (1);
      }
    }

  }
  catch (dabc::Exception& e)
  {
    DOUT1 ("pexorplugin::ReadoutModule::DoPexorReadout - raised dabc exception %s", e.what ());
    ref.Release ();
    // how do we treat this?
  }
  catch (std::exception& e)
  {
    DOUT1 ("pexorplugin::ReadoutModule::DoPexorReadout - raised std exception %s ", e.what ());
    ref.Release ();
  }
  catch (...)
  {
    DOUT1 ("pexorplugin::ReadoutModule::DoPexorReadout - Unexpected exception!!!");
    throw;
  }

}

