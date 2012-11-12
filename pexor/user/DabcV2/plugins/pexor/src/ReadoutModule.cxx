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

pexorplugin::ReadoutModule::ReadoutModule(const char* name, dabc::Command cmd) :
   dabc::ModuleAsync(name),
   fInPool(0),fOutPool(0),fBnetMode(false),fBufferSize(1024)
{

	 fBufferSize =  Cfg(dabc::xmlBufferSize,cmd).AsInt(16384);
	 int numoutputs = Cfg(dabc::xmlNumOutputs,cmd).AsInt(2);



   fInPool = CreatePoolHandle(Cfg(dabc::xmlInputPoolName,cmd).AsStr("pexorpool"));
   fOutPool = CreatePoolHandle(Cfg(dabc::xmlOutputPoolName, cmd).AsStr("pexorpool")); // if not configured, we use same pool for input and output

   CreateInput("Input0", fInPool,  Cfg(dabc::xmlInputQueueSize,cmd).AsInt(50)); // one input for complete pexor: later different inputs for sfps?
   for(int n=0; n<numoutputs; n++)
		  {
			 CreateOutput(FORMAT(("Output%d", n)), fOutPool, Cfg(dabc::xmlOutputQueueSize, cmd).AsInt(10));
							 // deprecated? fBnetMode ? sizeof(bnet::EventId) : 0);
		  }

   std::string ratesprefix = "Pexor";

   fEventRateName = ratesprefix + "Events";
   fDataRateName = ratesprefix + "Data";

   CreatePar(fEventRateName).SetRatemeter(false, 3.).SetUnits("Ev");
   CreatePar(fDataRateName).SetRatemeter(false, 1.).SetUnits("Mb");

   Par(fDataRateName).SetDebugLevel(1);
   Par(fEventRateName).SetDebugLevel(1);

//   CreateRateParameter("Device Readout", false, 1., "Input0","");
//   fEventsRate = CreateRateParameter("EventsRate", false, 3., "", "", "1/s", 0., 2500.);
   //fErrorRate = CreateRateParameter("Transport Error Rate", false, 3., "", "", "1/s", 0., 2500.);

}










void pexorplugin::ReadoutModule::BeforeModuleStart()
{
    DOUT1(("\n\npexorplugin::ReadoutModule::BeforeModuleStart"));

}


void pexorplugin::ReadoutModule::AfterModuleStop()
{
   //DOUT1(("\npexorplugin::ReadoutModule finished. Rate %5.1f Mb/s numoper:%7ld time:%5.1f s\n\n", Par(fDataRateName).AsDouble(), Par(fDataRateName).GetNumOper(), fRecvRate.GetTotalTime()));
	DOUT1(("\npexorplugin::ReadoutModule finished. Rate %5.1f Mb/s",Par(fDataRateName).AsDouble()));

}



void pexorplugin::ReadoutModule::ProcessInputEvent(dabc::Port* port)
{
	DoPexorReadout();
}

void pexorplugin::ReadoutModule::ProcessOutputEvent(dabc::Port* port)
{
    DoPexorReadout();
}


void pexorplugin::ReadoutModule::DoPexorReadout()
{
	dabc::Buffer ref;
	DOUT3(("pexorplugin::DoPexorReadout\n"));
	try {
		while (Input(0)->CanRecv()) {
			if (!CanSendToAllOutputs()) {
				DOUT3(
						("pexorplugin::ReadoutModule::DoPexorReadout - can not send to all outputs. skip event \n"));
				return;
			}
			ref = Input(0)->Recv();
			if (!ref.null()) {
				Par(fDataRateName).SetDouble(ref.GetTotalSize() / 1024. / 1024.);
				SendToAllOutputs(ref);
				Par(fEventRateName).SetInt(1);
			}
		}

	} catch (dabc::Exception& e) {
		DOUT1(
				("pexorplugin::ReadoutModule::DoPexorReadout - raised dabc exception %s", e.what()));
		ref.Release();
		// how do we treat this?
	} catch (std::exception& e) {
		DOUT1(
				("pexorplugin::ReadoutModule::DoPexorReadout - raised std exception %s ", e.what()));
		ref.Release();
	} catch (...) {
		DOUT1(
				("pexorplugin::ReadoutModule::DoPexorReadout - Unexpected exception!!!"));
		throw;
	}

}






