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
#include "dabc/Manager.h"
#include "mbs/MbsTypeDefs.h"

pexorplugin::ReadoutModule::ReadoutModule (const std::string name, dabc::Command cmd) :
    dabc::ModuleAsync (name)
{
  EnsurePorts (1, 1, dabc::xmlWorkPool);
  std::string ratesprefix = "Pexor";

  fEventRateName = ratesprefix + "Events";
  fDataRateName = ratesprefix + "Data";
  fInfoName = ratesprefix + "Info";
  fFileStateName= ratesprefix + "FileOn";
  CreatePar (fEventRateName).SetRatemeter (false, 3.).SetUnits ("Ev");
  CreatePar (fDataRateName).SetRatemeter (false, 3.).SetUnits ("Mb");
  CreatePar(fInfoName, "info").SetSynchron(true, 2., false).SetDebugLevel(2);
  CreatePar(fFileStateName).Dflt(false);

  Par (fDataRateName).SetDebugLevel (1);
  Par (fEventRateName).SetDebugLevel (1);


  CreateCmdDef(mbs::comStartFile)
        .AddArg(dabc::xmlFileName, "string", true)
        .AddArg(dabc::xmlFileSizeLimit, "int", false, 1000);

     CreateCmdDef(mbs::comStopFile);

     CreateCmdDef(mbs::comStartServer)
        .AddArg(mbs::xmlServerKind, "string", true, mbs::ServerKindToStr(mbs::StreamServer))
        .AddArg(mbs::xmlServerPort, "int", false, 6901);
     CreateCmdDef(mbs::comStopServer);


  PublishPars("$CONTEXT$/PexReadout");

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
        DOUT3 ("pexorplugin::ReadoutModule::DoPexorReadout - can not send to all outputs. skip event \n");
        return;
      }
      ref = Recv ();
      if (!ref.null ())
      {
        Par (fDataRateName).SetValue ((double)(ref.GetTotalSize ()) / 1024. / 1024.);
        DOUT3 ("pexorplugin::ReadoutModule::DoPexorReadout - has buffer size: total %d bytes  \n",ref.GetTotalSize ());
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

int pexorplugin::ReadoutModule::ExecuteCommand(dabc::Command cmd)
{

  // this is section taken from mbs combiner
  if (cmd.IsName(mbs::comStartFile)) {

    std::string fname = cmd.GetStr(dabc::xmlFileName); //"filename")
    int maxsize = cmd.GetInt(dabc::xml_maxsize, 30); // maxsize
    std::string url = dabc::format("%s://%s?%s=%d", mbs::protocolLmd, fname.c_str(), dabc::xml_maxsize, maxsize);
    EnsurePorts(0, 2);
    bool res = dabc::mgr.CreateTransport(OutputName(1, true), url);
    DOUT0("Started file %s res = %d", url.c_str(), res);
    SetInfo(dabc::format("Execute StartFile for %s, result=%d",url.c_str(), res), true);
    ChangeFileState(true);
    return cmd_bool(res);
     } else
     if (cmd.IsName(mbs::comStopFile)) {
        FindPort(OutputName(1)).Disconnect();
        SetInfo("Stopped file", true);
        ChangeFileState(false);
        return dabc::cmd_true;
     } else
     if (cmd.IsName(mbs::comStartServer)) {
        if (NumOutputs()<1) {
           EOUT("No ports was created for the server");
           return dabc::cmd_false;
        }
        std::string skind = cmd.GetStr(mbs::xmlServerKind);

        int port = cmd.GetInt(mbs::xmlServerPort, 6666);
        std::string url = dabc::format("mbs://%s?%s=%d", skind.c_str(), mbs::xmlServerPort,  port);
        EnsurePorts(0, 1);
        bool res = dabc::mgr.CreateTransport(OutputName(0, true));
        DOUT0("Started server %s res = %d", url.c_str(), res);
        SetInfo(dabc::format("Execute StartServer for %s, result=%d",url.c_str(), res), true);
        return cmd_bool(res);
     } else
     if (cmd.IsName(mbs::comStopServer)) {
        FindPort(OutputName(0)).Disconnect();
        SetInfo("Stopped server", true);
        return dabc::cmd_true;
     }

   return dabc::ModuleAsync::ExecuteCommand(cmd);
}






void pexorplugin::ReadoutModule::SetInfo(const std::string& info, bool forceinfo)
{
//   DOUT0("SET INFO: %s", info.c_str());

   dabc::InfoParameter par;

   if (!fInfoName.empty()) par = Par(fInfoName);

   par.SetValue(info);
   if (forceinfo)
      par.FireModified();
}


void pexorplugin::ReadoutModule::ChangeFileState(bool on)
{
  dabc::Parameter par=Par (fFileStateName);
  par.SetValue (on);
  dabc::Hierarchy chld = fWorkerHierarchy.FindChild(fFileStateName.c_str());
  if (!chld.null())
  {
       par.ScanParamFields(&chld()->Fields());
       fWorkerHierarchy.MarkChangedItems();
       DOUT0("ChangeFileState to %d", on);
  }
  else
  {
      DOUT0("ChangeFileState Could not find parameter %s", fFileStateName.c_str());
  }





}
