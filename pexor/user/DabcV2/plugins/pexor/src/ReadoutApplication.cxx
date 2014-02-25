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
//#include "pexorplugin/ReadoutApplication.h"
//
//
//#include "dabc/Parameter.h"
//#include "dabc/Command.h"
//#include "dabc/timing.h"
//#include "dabc/CommandsSet.h"
//#include "dabc/Device.h"
//
//#include "mbs/MbsTypeDefs.h"
//#include "mbs/Factory.h"
//
//
//
//
//
//pexorplugin::ReadoutApplication::ReadoutApplication() :
//   dabc::Application(pexorplugin::nameReadoutAppClass)
//{
//   CreatePar(xmlPexorID).DfltInt(0);
//   for (int nr=0; nr<PEXORPLUGIN_NUMSFP; nr++)
//	   {
//			CreatePar(FORMAT(("%s%d",xmlPexorSFPSlaves, nr))).DfltInt(0);
//	   }
//   CreatePar(xmlDMABufLen).DfltInt(65536);
//   CreatePar(xmlDMABufNum).DfltInt(20);
//   CreatePar(xmlDMAScatterGatherMode).DfltBool(false);
//   CreatePar(xmlDMAZeroCopy).DfltBool(false);
//   CreatePar(xmlFormatMbs).DfltBool(true);
//   CreatePar(xmlSyncRead).DfltBool(true);
//   CreatePar(xmlParallelRead).DfltBool(true);
//   CreatePar(xmlTriggeredRead).DfltBool(false);
//
//   CreatePar(xmlTrixorConvTime).DfltInt(0x500);
//   CreatePar(xmlTrixorFastClearTime).DfltInt(0x400);
//   CreatePar(dabc::xmlBufferSize).DfltInt(65536);
//   CreatePar(dabc::xmlNumBuffers).DfltInt(100);
//   CreatePar(xmlExploderSubmem).DfltInt(1024);
//   CreatePar(dabc::xmlInputPoolName).DfltStr(nameInputPool);
//   CreatePar(dabc::xmlOutputPoolName).DfltStr(nameOutputPool);
//
//   CreatePar(xmlModuleName).DfltStr("PEXOR-Module");
//   CreatePar(xmlModuleThread).DfltStr("PEXOR-ModThread");
//   CreatePar(xmlDeviceName).DfltStr("PEXOR-Device");
//   CreatePar(xmlDeviceThread).DfltStr("PEXOR-DevThread");
//
//
//   CreatePar(mbs::xmlServerKind).DfltStr(mbs::ServerKindToStr(mbs::StreamServer));
//   CreatePar(xmlRawFile).DfltStr("");
//   CreatePar(mbs::xmlSizeLimit).DfltInt(0);
//
//   DOUT1(("!!!! Data server plugin created %s !!!!", GetName()));
//}
//
//int pexorplugin::ReadoutApplication::DataServerKind() const
//{
//   return mbs::StrToServerKind(Par(mbs::xmlServerKind).AsStr());
//}
//
//
//bool pexorplugin::ReadoutApplication::CreateAppModules()
//{
//   DOUT1(("CreateAppModules starts..."));
//   bool res = false;
//
//   dabc::lgr()->SetLogLimit(10000000);
//
//   dabc::mgr.CreateMemoryPool( Par(dabc::xmlInputPoolName).AsStr("dummy pool"),
//                                 Par(dabc::xmlBufferSize).AsInt(8192),
//                                 Par(dabc::xmlNumBuffers).AsInt(100));
//
//
//   //dabc::mgr.CreateMemoryPool(roc::xmlRocPool, bufsize, Par(dabc::xmlNumBuffers).AsInt(100));
//
//   // Readout module with memory pools:
//   dabc::CmdCreateModule cmd1(nameReadoutModuleClass, Par(xmlModuleName).AsStr(), Par(xmlModuleThread).AsStr());
//   cmd1.SetStr(dabc::xmlInputPoolName, Par(dabc::xmlInputPoolName).AsStr("dummy pool"));
//   cmd1.SetStr(dabc::xmlOutputPoolName, Par(dabc::xmlInputPoolName).AsStr("dummy pool"));
//   cmd1.SetInt(dabc::xmlNumOutputs, 2); // one for lmd, one for stream server
//   res = dabc::mgr.Execute(cmd1);
//   DOUT1(("Create PEXOR Readout module = %s", DBOOL(res)));
//   if(!res) return false;
//
//
//   // the PEXOR device
//   dabc::CmdCreateDevice cmd2(nameDeviceClass, Par(xmlDeviceName).AsStr(), Par(xmlDeviceThread).AsStr());
//   cmd2.SetInt(xmlPexorID, Par(xmlPexorID).AsInt(0));
//   cmd2.SetInt(xmlDMABufLen, Par(xmlDMABufLen).AsInt(16384));
//   cmd2.SetInt(xmlDMABufNum, Par(xmlDMABufNum).AsInt(50));
//   cmd2.SetBool(xmlDMAScatterGatherMode, Par(xmlDMAScatterGatherMode).AsBool(false));
//   cmd2.SetBool(xmlDMAZeroCopy, Par(xmlDMAZeroCopy).AsBool(false));
//   cmd2.SetInt(xmlExploderSubmem, Par(xmlExploderSubmem).AsInt(2048));
//   cmd2.SetBool(xmlFormatMbs, Par(xmlFormatMbs).AsBool(false));
//   cmd2.SetBool(xmlSyncRead, Par(xmlSyncRead).AsBool(false));
//   cmd2.SetBool(xmlParallelRead, Par(xmlParallelRead).AsBool(false));
//   cmd2.SetBool(xmlTriggeredRead, Par(xmlTriggeredRead).AsBool(false));
//   cmd2.SetInt(xmlTrixorConvTime, Par(xmlTrixorConvTime).AsInt(0x30));
//   cmd2.SetInt(xmlTrixorFastClearTime, Par(xmlTrixorFastClearTime).AsInt(0x20));
//   //cmd->SetStr(xmlDeviceThread, GetParStr(xmlDeviceThread,"device thread"));
//   for (int nr=0; nr<PEXORPLUGIN_NUMSFP; nr++)
//	   {
//		   cmd2.SetInt(FORMAT(("%s%d",xmlPexorSFPSlaves, nr)), Par(FORMAT(("%s%d",xmlPexorSFPSlaves, nr))).AsInt(0));
//	   }
//
//
//   if (!dabc::mgr.Execute(cmd2))
//	   {
//		   EOUT(("Cannot create device %s for  pexorplugin", nameDeviceClass));
//		   return false;
//	   }
//
//
//   // connect module with device and mbs outputs:
//   if (!dabc::mgr.CreateTransport(FORMAT(("%s/Input0",Par(xmlModuleName).AsStr() )),  Par(xmlDeviceName).AsStr(), Par(xmlModuleThread).AsStr()) )
//	   {
//		   EOUT(("Cannot connect readout module to device %s", Par(xmlDeviceName).AsStr() ));
//		   return false;
//	   }
//
//	// connect file and mbs server outputs:
//if (OutputFileName().length()>0) {
//      dabc::CmdCreateTransport cmd3(FORMAT(("%s/Output1",Par(xmlModuleName).AsStr() )), mbs::typeLmdOutput);
//      cmd3.SetStr(mbs::xmlFileName, OutputFileName().c_str());
//      res = dabc::mgr()->Execute(cmd3);
//      DOUT1(("Create raw lmd output file %s , result = %s",OutputFileName().c_str() , DBOOL(res)));
//      if(!res) return false;
//}
//
//if (DataServerKind() != mbs::NoServer) {
//
//   ///// connect module to mbs server:
//      dabc::CmdCreateTransport cmd4(FORMAT(("%s/Output0",Par(xmlModuleName).AsStr())), mbs::typeServerTransport, "MbsServerThrd");
//
//      // no need to set extra parameters - they will be taken from application !!!
////      cmd->SetStr(mbs::xmlServerKind, mbs::ServerKindToStr(DataServerKind())); //mbs::StreamServer ,mbs::TransportServer
////      cmd->SetInt(dabc::xmlBufferSize, GetParInt(dabc::xmlBufferSize, 8192));
//
//      res = dabc::mgr.Execute(cmd4);
//      DOUT1(("Connected readout module output to Mbs server = %s", DBOOL(res)));
//      if(!res) return false;
//   }
//
//
//
//
//
//
//   return true;
//}
//
//
//int pexorplugin::ReadoutApplication::ExecuteCommand(dabc::Command cmd)
//{
//   int res = dabc::cmd_false;
//
////   if (cmd->IsName("StartTriggerMode")) {
////      DOUT0(("StartTrigger %5.2f", TimeStamp()*1e-6));
////
////      for(int t=0; t<NumRocs(); t++)
////         SwitchTriggerMode(GetRoc(t), true);
////
//////      DOUT0(("StartTrigger %5.2f done", TimeStamp()*1e-6));
////   } else
////   if (cmd->IsName("StopTriggerMode")) {
////      DOUT0(("StopTrigger  %5.2f", TimeStamp()*1e-6));
////
////      for(int t=0; t<NumRocs(); t++) {
////         SwitchTriggerMode(GetRoc(t), false);
////      }
////
//////      DOUT0(("StopTrigger %5.2f done", TimeStamp()*1e-6));
////
////   } else
//
//
//      res = dabc::Application::ExecuteCommand(cmd);
//
//   return res;
//}
//
