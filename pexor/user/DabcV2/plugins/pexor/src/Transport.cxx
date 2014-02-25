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
	fPexorDevice->StartTrigger();
	return dabc::InputTransport::StartTransport();

}

bool pexorplugin::Transport::StopTransport()
{
	DOUT1("StopTransport() %p\n", thread().GetObject());
	fPexorDevice->StopTrigger();
	bool rev=dabc::InputTransport::StopTransport();
	DOUT1("\npexorplugin::Transport stopped with result %d. Total number of token errors: %7ld in %e s\n",  rev, fPexorInput->fErrorRate.GetNumOper(),  fPexorInput->fErrorRate.GetTotalTime());
	return rev;

}

//unsigned pexorplugin::Transport::Read_Size()
//{
//	//return dabc::di_Error;
//     int res = fPexorDevice->GetReadLength();
//     DOUT3(("Read_Size()=%d\n",res));
//
//     return res>0 ? res : dabc::di_Error;
//}
//
//unsigned pexorplugin::Transport::Read_Start(dabc::Buffer& buf)
//{
//	  DOUT3(("Read_Start() with bufsize %d\n",buf.GetTotalSize()));
//
//  if (fPexorDevice->IsTriggeredRead() || fPexorDevice->IsSynchronousRead())
//    {
//      return dabc::di_Ok; // synchronous mode, all handled in Read_Complete
//    }
//  else
//    {
//      int res = 0;
//      if (fPexorDevice->IsParallelRead())
//        {
//          res = fPexorDevice->RequestAllTokens(buf, false);
//        }
//      else
//        {
//          res = fPexorDevice->RequestToken(buf, false);
//        } // if parallel
//
//// note: Read_Start() currently does not support skipping buffers or timeout
////      if ((unsigned) res == dabc::di_SkipBuffer)
////        {
////          fErrorRate.Packet(buf->GetDataSize());
////          return dabc::di_SkipBuffer;
////        }
////      if ((unsigned) res == dabc::di_RepeatTimeOut)
////             {
////                DOUT1(("pexorplugin::Transport() returns with timeout\n"));
////               return dabc::di_RepeatTimeOut;
////             }
//
//      return ((unsigned) res == dabc::di_Ok) ? dabc::di_Ok : dabc::di_Error;
//    } // if synchronous
//
//}
//
//
//unsigned pexorplugin::Transport::Read_Complete(dabc::Buffer& buf)
//{
//	DOUT3(("Read_Complete()\n"));
//  int res = 0;
//  if (fPexorDevice->IsParallelRead())
//    {
//      if (fPexorDevice->IsTriggeredRead() || fPexorDevice->IsSynchronousRead())
//        {
//          res = fPexorDevice->RequestAllTokens(buf, false); // for parallel read, we need async request before polling
//          res = fPexorDevice->ReceiveAllTokenBuffer(buf);
//        }
//      else
//        {
//          res = fPexorDevice->ReceiveAllTokenBuffer(buf);
//        }
//    }
//  else
//    {
//      if (fPexorDevice->IsTriggeredRead() || fPexorDevice->IsSynchronousRead())
//        {
//          res = fPexorDevice->RequestToken(buf, true);
//        }
//      else
//        {
//          res = fPexorDevice->ReceiveTokenBuffer(buf);
//        }
//    }
//  if ((unsigned) res == dabc::di_SkipBuffer)
//    {
//      fErrorRate.Packet(buf.GetTotalSize());
//      return dabc::di_SkipBuffer;
//    }
//  if ((unsigned) res == dabc::di_RepeatTimeOut)
//   {
//     DOUT1(("pexorplugin::Transport() returns with timeout\n"));
//     return dabc::di_RepeatTimeOut;
//   }
//
//  return res > 0 ? dabc::di_Ok : dabc::di_Error;
//}
//
//
//
///*bool pexorplugin::Transport::WriteBuffer(dabc::Buffer* buf)
//{
//	return dabc::di_Error;
//   //bool res = fPCIDevice->WritePCI(buf);
//   //return res;
//
//}*/


void pexorplugin::Transport::ProcessPoolChanged(dabc::MemoryPool* pool)
{
   // TODO: check if this has any effect. Replace by other method to map dabc pool for sg dma
	DOUT1("############## pexorplugin::Transport::ProcessPoolChanged for memory pool %x",pool);
	fPexorDevice->MapDMAMemoryPool(pool);

}



