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
#include "pexorplugin/Input.h"
#include "pexorplugin/Device.h"

#include "dabc/Port.h"
#include "dabc/logging.h"
pexorplugin::Input::Input(pexorplugin::Device* dev) :
   dabc::DataInput(),
   fPexorDevice(dev)
   // provide input and output buffers
{

}



pexorplugin::Input::~Input()
{

}

//void pexorplugin::Input::StartTransport()
//{
//	DOUT1(("StartTransport() %p\n", thread().GetObject()));
//	fPexorDevice->StartTrigger();
//	dabc::DataTransport::StartTransport();
//
//}
//
//void pexorplugin::Transport::StopTransport()
//{
//	DOUT1(("StopTransport() %p\n", thread().GetObject()));
//	fPexorDevice->StopTrigger();
//	dabc::DataTransport::StopTransport();
//	DOUT1(("\npexorplugin::Transport stopped. total number of token errors: %7ld in %e s\n",  fErrorRate.GetNumOper(),  fErrorRate.GetTotalTime()));
//
//
//}

unsigned pexorplugin::Input::Read_Size()
{
	//return dabc::di_Error;
     int res = fPexorDevice->GetReadLength();
     DOUT3(("Read_Size()=%d\n",res));

     return res>0 ? res : dabc::di_Error;
}

unsigned pexorplugin::Input::Read_Start(dabc::Buffer& buf)
{
	  DOUT3(("Read_Start() with bufsize %d\n",buf.GetTotalSize()));

  if (fPexorDevice->IsTriggeredRead() || fPexorDevice->IsSynchronousRead())
    {
      return dabc::di_Ok; // synchronous mode, all handled in Read_Complete
    }
  else
    {
      int res = 0;
      if (fPexorDevice->IsParallelRead())
        {
          res = fPexorDevice->RequestAllTokens(buf, false);
        }
      else
        {
          res = fPexorDevice->RequestToken(buf, false);
        } // if parallel

// note: Read_Start() currently does not support skipping buffers or timeout
//      if ((unsigned) res == dabc::di_SkipBuffer)
//        {
//          fErrorRate.Packet(buf->GetDataSize());
//          return dabc::di_SkipBuffer;
//        }
//      if ((unsigned) res == dabc::di_RepeatTimeOut)
//             {
//                DOUT1(("pexorplugin::Input() returns with timeout\n"));
//               return dabc::di_RepeatTimeOut;
//             }

      return ((unsigned) res == dabc::di_Ok) ? dabc::di_Ok : dabc::di_Error;
    } // if synchronous

}


unsigned pexorplugin::Input::Read_Complete(dabc::Buffer& buf)
{
	DOUT3(("Read_Complete()\n"));
  int res = 0;
  if (fPexorDevice->IsParallelRead())
    {
      if (fPexorDevice->IsTriggeredRead() || fPexorDevice->IsSynchronousRead())
        {
          res = fPexorDevice->RequestAllTokens(buf, false); // for parallel read, we need async request before polling
          res = fPexorDevice->ReceiveAllTokenBuffer(buf);
        }
      else
        {
          res = fPexorDevice->ReceiveAllTokenBuffer(buf);
        }
    }
  else
    {
      if (fPexorDevice->IsTriggeredRead() || fPexorDevice->IsSynchronousRead())
        {
          res = fPexorDevice->RequestToken(buf, true);
        }
      else
        {
          res = fPexorDevice->ReceiveTokenBuffer(buf);
        }
    }
  if ((unsigned) res == dabc::di_SkipBuffer)
    {
      fErrorRate.Packet(buf.GetTotalSize());
      return dabc::di_SkipBuffer;
    }
  if ((unsigned) res == dabc::di_RepeatTimeOut)
   {
     DOUT1(("pexorplugin::Input() returns with timeout\n"));
     return dabc::di_RepeatTimeOut;
   }

  return res > 0 ? dabc::di_Ok : dabc::di_Error;
}



/*bool pexorplugin::Input::WriteBuffer(dabc::Buffer* buf)
{
	return dabc::di_Error;
   //bool res = fPCIDevice->WritePCI(buf);
   //return res;

}*/


//void pexorplugin::Input::ProcessPoolChanged(dabc::MemoryPool* pool)
//{
//	DOUT1(("############## pexorplugin::Input::ProcessPoolChanged for memory pool %x",pool));
//	fPexorDevice->MapDMAMemoryPool(pool);
//
//}



