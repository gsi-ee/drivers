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
#include "dabc/PoolHandle.h"
#include "dabc/MemoryPool.h"
#include "dabc/Port.h"
#include "dabc/Pointer.h"
#include "dabc/Manager.h"
#include "dabc/records.h"

#include "bnet/common.h"

pexorplugin::ReadoutModule::ReadoutModule(const char* name, dabc::Command* cmd) :
   dabc::ModuleAsync(name),
   fInPool(0),fOutPool(0),fEventsRate(0),fBnetMode(false),fBufferSize(1024)
{

	 fBufferSize = GetCfgInt(dabc::xmlBufferSize, 16384, cmd);
	 int numoutputs = GetCfgInt(dabc::xmlNumOutputs, 2, cmd);



   fInPool = CreatePoolHandle(GetCfgStr(dabc::xmlInputPoolName, "pexorpool", cmd).c_str(), fBufferSize, 1);
   fOutPool = CreatePoolHandle(GetCfgStr(dabc::xmlOutputPoolName, "pexorpool", cmd).c_str(), fBufferSize, 1); // if not configured, we use same pool for input and output


   CreateInput("Input0", fInPool,  GetCfgInt(dabc::xmlInputQueueSize, 50, cmd)); // one input for complete pexor: later different inputs for sfps?
   for(int n=0; n<numoutputs; n++)
		  {
			 CreateOutput(FORMAT(("Output%d", n)), fOutPool, GetCfgInt(dabc::xmlOutputQueueSize, 10, cmd),
							 fBnetMode ? sizeof(bnet::EventId) : 0);
		  }

   CreateRateParameter("Device Readout", false, 1., "Input0","");
   fEventsRate = CreateRateParameter("EventsRate", false, 3., "", "", "1/s", 0., 2500.);
   //fErrorRate = CreateRateParameter("Transport Error Rate", false, 3., "", "", "1/s", 0., 2500.);

}










void pexorplugin::ReadoutModule::BeforeModuleStart()
{
    DOUT1(("\n\npexorplugin::ReadoutModule::BeforeModuleStart"));

}


void pexorplugin::ReadoutModule::AfterModuleStop()
{
   DOUT1(("\npexorplugin::ReadoutModule finished. Rate %5.1f Mb/s numoper:%7ld time:%5.1f s\n\n", fRecvRate.GetRate(), fRecvRate.GetNumOper(), fRecvRate.GetTotalTime()));
}

//
//
void pexorplugin::ReadoutModule::ProcessUserEvent(dabc::ModuleItem* , uint16_t id)
{
dabc::Buffer* ref=0;
DOUT3(("pexorplugin::ProcessUserEvent\n"));
try
   {
   if(id==dabc::evntInput || id==dabc::evntOutput)
      {
	   while(Input(0)->CanRecv())
	   {
		   if (!CanSendToAllOutputs())
			   {
				   DOUT3(("pexorplugin::ReadoutModule::ProcessUserEvent - can not send to all outputs. skip event \n"));
				   return;
			   }
			   ref = Input(0)->Recv();
		   if (ref)
		   {
			   fRecvRate.Packet(ref->GetDataSize());
			   SendToAllOutputs(ref);
			   if (fEventsRate) fEventsRate->AccountValue(1.);
		   }
	   }
      }
   else
	  {
		  DOUT3(("pexorplugin::ReadoutModule::ProcessUserEvent gets event id:%d, ignored.", id));
	  }


   }
catch(dabc::Exception& e)
   {
       DOUT1(("pexorplugin::ReadoutModule::ProcessUserEvent - raised dabc exception %s at event id=%d", e.what(), id));
       dabc::Buffer::Release(ref);
       // how do we treat this?
   }
catch(std::exception& e)
   {
       DOUT1(("pexorplugin::ReadoutModule::ProcessUserEvent - raised std exception %s at event id=%d", e.what(), id));
       dabc::Buffer::Release(ref);
   }
catch(...)
   {
       DOUT1(("pexorplugin::ReadoutModule::ProcessInputEvent - Unexpected exception!!!"));
       throw;
   }

}




