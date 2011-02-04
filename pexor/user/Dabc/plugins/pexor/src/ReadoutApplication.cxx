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
#include "pexorplugin/ReadoutApplication.h"


#include "dabc/Parameter.h"
#include "dabc/Command.h"
#include "dabc/timing.h"
#include "dabc/CommandsSet.h"
#include "dabc/Device.h"

#include "mbs/MbsTypeDefs.h"
#include "mbs/Factory.h"





pexorplugin::ReadoutApplication::ReadoutApplication() :
   dabc::Application(pexorplugin::nameReadoutAppClass)
{
   CreateParInt(xmlPexorID, 0);
   for (int nr=0; nr<PEXORPLUGIN_NUMSFP; nr++)
	   {
			CreateParInt(FORMAT(("%s%d",xmlPexorSFPSlaves, nr)), 0);
	   }
   CreateParInt(xmlDMABufLen, 65536);
   CreateParInt(xmlDMABufNum, 20);
   CreateParBool(xmlFormatMbs, true);
   CreateParBool(xmlSyncRead, true);
   CreateParBool(xmlParallelRead, true);
   CreateParBool(xmlTriggeredRead, false);
   CreateParInt(xmlTrixorConvTime, 0x500);
   CreateParInt(xmlTrixorFastClearTime, 0x400);
   CreateParInt(dabc::xmlBufferSize, 65536);
   CreateParInt(dabc::xmlNumBuffers, 100);
   CreateParInt(xmlExploderSubmem, 1024);
   CreateParStr(dabc::xmlInputPoolName, nameInputPool);
   CreateParStr(dabc::xmlOutputPoolName, nameOutputPool);

   CreateParStr(xmlModuleName, "PEXOR-Module");
   CreateParStr(xmlModuleThread, "PEXOR-ModThread");
   CreateParStr(xmlDeviceName, "PEXOR-Device");
   CreateParStr(xmlDeviceThread,   "PEXOR-DevThread");


   CreateParStr(mbs::xmlServerKind, mbs::ServerKindToStr(mbs::StreamServer));
   CreateParStr(xmlRawFile, "");
   CreateParInt(mbs::xmlSizeLimit, 0);

   DOUT1(("!!!! Data server plugin created %s !!!!", GetName()));
}

int pexorplugin::ReadoutApplication::DataServerKind() const
{
   return mbs::StrToServerKind(GetParStr(mbs::xmlServerKind).c_str());
}


bool pexorplugin::ReadoutApplication::CreateAppModules()
{
   DOUT1(("CreateAppModules starts..."));
   bool res = false;
   dabc::Command* cmd;

   dabc::lgr()->SetLogLimit(10000000);

   dabc::mgr()->CreateMemoryPool( GetParStr(dabc::xmlInputPoolName,"dummy pool").c_str(),
                                 GetParInt(dabc::xmlBufferSize, 8192),
                                 GetParInt(dabc::xmlNumBuffers, 100));

   // Readout module with memory pools:
   cmd = new dabc::CmdCreateModule(nameReadoutModuleClass, GetParStr(xmlModuleName).c_str(), GetParStr(xmlModuleThread).c_str());
   cmd->SetStr(dabc::xmlInputPoolName, GetParStr(dabc::xmlInputPoolName,"dummy pool"));
   cmd->SetStr(dabc::xmlOutputPoolName, GetParStr(dabc::xmlInputPoolName,"dummy pool"));
   cmd->SetInt(dabc::xmlNumOutputs, 2); // one for lmd, one for stream server
   res = dabc::mgr()->Execute(cmd);
   DOUT1(("Create PEXOR Readout module = %s", DBOOL(res)));
   if(!res) return false;


   // the PEXOR device
   cmd = new dabc::CmdCreateDevice(nameDeviceClass, GetParStr(xmlDeviceName).c_str(), GetParStr(xmlDeviceThread).c_str());
   cmd->SetInt(xmlPexorID, GetParInt(xmlPexorID, 0));
   cmd->SetInt(xmlDMABufLen, GetParInt(xmlDMABufLen, 16384));
   cmd->SetInt(xmlDMABufNum, GetParInt(xmlDMABufNum, 50));
   cmd->SetInt(xmlExploderSubmem, GetParInt(xmlExploderSubmem, 2048));
   cmd->SetBool(xmlFormatMbs, GetParBool(xmlFormatMbs, 0));
   cmd->SetBool(xmlSyncRead, GetParBool(xmlSyncRead, 0));
   cmd->SetBool(xmlParallelRead, GetParBool(xmlParallelRead, 0));
   cmd->SetBool(xmlTriggeredRead, GetParBool(xmlTriggeredRead, 0));
   cmd->SetInt(xmlTrixorConvTime, GetParInt(xmlTrixorConvTime, 0x30));
   cmd->SetInt(xmlTrixorFastClearTime, GetParInt(xmlTrixorFastClearTime, 0x20));
   //cmd->SetStr(xmlDeviceThread, GetParStr(xmlDeviceThread,"device thread"));
   for (int nr=0; nr<PEXORPLUGIN_NUMSFP; nr++)
	   {
		   cmd->SetInt(FORMAT(("%s%d",xmlPexorSFPSlaves, nr)), GetParInt(FORMAT(("%s%d",xmlPexorSFPSlaves, nr)), 0));
	   }


   if (!dabc::mgr()->Execute(cmd))
	   {
		   EOUT(("Cannot create device %s for  pexorplugin",nameDeviceClass));
		   return false;
	   }


   // connect module with device and mbs outputs:
   if (!dabc::mgr()->CreateTransport(FORMAT(("%s/Input0",GetParStr(xmlModuleName).c_str() )),  GetParStr(xmlDeviceName).c_str(), GetParStr(xmlModuleThread).c_str()) )
	   {
		   EOUT(("Cannot connect readout module to device %s", GetParStr(xmlDeviceName).c_str() ));
		   return false;
	   }

	// connect file and mbs server outputs:
if (OutputFileName().length()>0) {
      cmd = new dabc::CmdCreateTransport(FORMAT(("%s/Output1",GetParStr(xmlModuleName).c_str() )), mbs::typeLmdOutput);
      cmd->SetStr(mbs::xmlFileName, OutputFileName().c_str());
      res = dabc::mgr()->Execute(cmd);
      DOUT1(("Create raw lmd output file %s , result = %s",OutputFileName().c_str() , DBOOL(res)));
      if(!res) return false;
}

if (DataServerKind() != mbs::NoServer) {

   ///// connect module to mbs server:
      cmd = new dabc::CmdCreateTransport(FORMAT(("%s/Output0",GetParStr(xmlModuleName).c_str())), mbs::typeServerTransport, "MbsServerThrd");

      // no need to set extra parameters - they will be taken from application !!!
//      cmd->SetStr(mbs::xmlServerKind, mbs::ServerKindToStr(DataServerKind())); //mbs::StreamServer ,mbs::TransportServer
//      cmd->SetInt(dabc::xmlBufferSize, GetParInt(dabc::xmlBufferSize, 8192));

      res = dabc::mgr()->Execute(cmd);
      DOUT1(("Connected readout module output to Mbs server = %s", DBOOL(res)));
      if(!res) return false;
   }






   return true;
}


int pexorplugin::ReadoutApplication::ExecuteCommand(dabc::Command* cmd)
{
   int res = dabc::cmd_false;

//   if (cmd->IsName("StartTriggerMode")) {
//      DOUT0(("StartTrigger %5.2f", TimeStamp()*1e-6));
//
//      for(int t=0; t<NumRocs(); t++)
//         SwitchTriggerMode(GetRoc(t), true);
//
////      DOUT0(("StartTrigger %5.2f done", TimeStamp()*1e-6));
//   } else
//   if (cmd->IsName("StopTriggerMode")) {
//      DOUT0(("StopTrigger  %5.2f", TimeStamp()*1e-6));
//
//      for(int t=0; t<NumRocs(); t++) {
//         SwitchTriggerMode(GetRoc(t), false);
//      }
//
////      DOUT0(("StopTrigger %5.2f done", TimeStamp()*1e-6));
//
//   } else


      res = dabc::Application::ExecuteCommand(cmd);

   return res;
}

