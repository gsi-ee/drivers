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
#include "pexorplugin/Transport.h"
#include "pexorplugin/Input.h"
#include "pexorplugin/Device.h"

#include "dabc/Port.h"
#include "dabc/logging.h"
#include "dabc/statistic.h"

pexorplugin::Transport::Transport(pexorplugin::Device* dev, pexorplugin::Input* inp, dabc::Command cmd, const dabc::PortRef& inpport)
   :  dabc::InputTransport(cmd, inpport, inp, true), fPexorDevice(dev), fPexorInput(inp)
{

}



pexorplugin::Transport::~Transport()
{

}

bool pexorplugin::Transport::StartTransport()
{
	DOUT1("StartTransport() %p\n", thread().GetObject());
	// start/stop acquisition (trigger) is independent of dabc running state!
	//fPexorDevice->StartAcquisition();
	return dabc::InputTransport::StartTransport();

}

bool pexorplugin::Transport::StopTransport()
{
	DOUT1("StopTransport() %p\n", thread().GetObject());
	//fPexorDevice->StopAcquisition();
	// start/stop acquisition (trigger) is independent of dabc running state!
	bool rev=dabc::InputTransport::StopTransport();
	DOUT1("\npexorplugin::Transport stopped with result %d. Total number of token errors: %7ld in %e s\n",  rev, fPexorInput->fErrorRate.GetNumOper(),  fPexorInput->fErrorRate.GetTotalTime());
	return rev;

}



void pexorplugin::Transport::ProcessPoolChanged(dabc::MemoryPool* pool)
{
   // TODO: check if this has any effect. Replace by other method to map dabc pool for sg dma
	DOUT1("############## pexorplugin::Transport::ProcessPoolChanged for memory pool %x",pool);
	fPexorDevice->MapDMAMemoryPool(pool);

}



