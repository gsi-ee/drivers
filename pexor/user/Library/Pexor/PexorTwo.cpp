/*
 * PexorTwo.cpp
 *
 *  Created on: 05.03.2010
 *      Author: adamczew
 */

#include "PexorTwo.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string>
#include <errno.h>

#include "DMA_Buffer.h"


namespace pexor {

PexorTwo::PexorTwo(unsigned int id) : Pexor(id)
{
	PexorDebug("PexorTwo ctor for id %d",id);
}

PexorTwo::~PexorTwo()

{
	PexorDebug("PexorTwo dtor");
}


pexor::DMA_Buffer* PexorTwo::RequestToken(const unsigned long channel, const int bufid, bool sync, bool directdma,int* dmabuf, unsigned int woffset)

{
	PexorDebug("PexorTwo::RequestToken with write offset 0x%x",woffset);
	int rev=0;
	struct pexor_token_io descriptor;
	descriptor.bufid=bufid;
	descriptor.sfp=channel;
	descriptor.sync=0;
	descriptor.directdma=0;
	descriptor.tkbuf.addr= (unsigned long) dmabuf;
	descriptor.offset=woffset;
	if(sync) descriptor.sync=1;
	if(directdma) descriptor.directdma=1;
	rev=ioctl(fFileHandle, PEXOR_IOC_REQUEST_TOKEN, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d on token request from channel 0x%x bufid:0x%x sync:%d- %s\n",
			    er, channel, bufid, sync, strerror(er));
			return (pexor::DMA_Buffer*) -1;;
		}
	if(!sync) return 0;
	return (PrepareReceivedBuffer(descriptor, dmabuf));
}



pexor::DMA_Buffer* PexorTwo::RequestMultiToken(const unsigned long channelmask, const int bufid, bool sync, bool directdma)
{
  PexorDebug("PexorTwo::RequestMultiToken with channelmask 0x%x", channelmask);
      int rev=0;
      struct pexor_token_io descriptor;
      descriptor.bufid=bufid;
      descriptor.sfp= (channelmask << 16); // upper bytes expected as sfp pattern by driver
      descriptor.sync=0;
      descriptor.directdma=0;
      descriptor.tkbuf.addr=0;
      descriptor.offset=0;
      if(sync) descriptor.sync=1;
      if(directdma) descriptor.directdma=1;
      rev=ioctl(fFileHandle, PEXOR_IOC_REQUEST_TOKEN, &descriptor);
      if(rev)
          {
              int er=errno;
              PexorError("\n\nError %d on token request from channelmask 0x%x bufid:0x%x sync:%d- %s\n",
                  er, channelmask, bufid, sync, strerror(er));
              return (pexor::DMA_Buffer*) -1;
          }
      if(!sync) return 0;
      return (PrepareReceivedBuffer(descriptor, 0));

}



pexor::DMA_Buffer* PexorTwo::WaitForToken(const unsigned long channel, bool directdma, int* dmabuf, unsigned int woffset, bool sync)
{
	int rev=0;
	struct pexor_token_io descriptor;
	descriptor.sfp=channel;
	descriptor.tkbuf.addr=(unsigned long) dmabuf;
	descriptor.offset=woffset;
    descriptor.directdma=0;
    descriptor.sync= sync ; // use sync property to supress kernel log warnings!
    if(directdma) descriptor.directdma=1;
	rev=ioctl(fFileHandle, PEXOR_IOC_WAIT_TOKEN, &descriptor);
	if(rev)
		{
	        if(sync) // do not complain in case of triggerless read out
	          {
	            int er=errno;
	            PexorError("\n\nError %d  on wait token from channel 0x%x - %s\n",er, channel, strerror(er));
	          }
	          return (pexor::DMA_Buffer*) -1;
		}
	return (PrepareReceivedBuffer(descriptor, dmabuf));

}



pexor::DMA_Buffer* PexorTwo::RequestReceiveAllTokens (const unsigned long channelmask, const int bufid, int* dmabuf,
    unsigned int woffset)
{
  int rev = 0;
  struct pexor_token_io descriptor;
  descriptor.sfp = (channelmask << 16);    // upper bytes expected as sfp pattern by driver
  descriptor.bufid = bufid;
  descriptor.tkbuf.addr = (unsigned long) dmabuf;
  descriptor.offset = woffset;
  descriptor.directdma = 0;
  rev = ioctl (fFileHandle, PEXOR_IOC_REQUEST_RECEIVE_TOKENS, &descriptor);
  if (rev)
  {
    int er = errno;
    PexorError("\n\nError %d  RequestReceiveAllTokens - %s\n", er, strerror(er));
    return (pexor::DMA_Buffer*) -1;
  }
  return (PrepareReceivedBuffer (descriptor, dmabuf));

}


pexor::DMA_Buffer* PexorTwo::RequestReceiveAsyncTokens (int* dmabuf,
    unsigned int woffset)
{
  int rev = 0;
  struct pexor_token_io descriptor;
  descriptor.tkbuf.addr = (unsigned long) dmabuf;
  descriptor.offset = woffset;
  descriptor.directdma = 0;
  rev = ioctl (fFileHandle, PEXOR_IOC_REQUEST_RECEIVE_ASYNC, &descriptor);
  if (rev)
  {
    // no error output when polling for the missing data
    //int er = errno;
    //PexorError("\n\nError %d  RequestReceiveAsyncTokens - %s\n", er, strerror(er));
    return 0; // return value indicates to poll again
 //   return (pexor::DMA_Buffer*) -1;
  }
  return (PrepareReceivedBuffer (descriptor, dmabuf));

}


///////////////////////////////////
// JAM16 ATWORK

pexor::DMA_Buffer* PexorTwo::RequestReceiveAsyncTokensPolling ()
{
  int rev = 0;
  struct pexor_token_io wrapper;
  struct pexor_userbuf data;
  bool requested=false;
  PexorDebug("RequestReceiveAsyncTokensPolling begins...\n");
  while (true)
  {
    rev = ioctl (fFileHandle, PEXOR_IOC_GET_ASYNC_BUFFER, &data);    // first fetch previously requested buffers from user queue
    if (rev == 0)
    {
      PexorDebug("RequestReceiveAsyncTokensPolling has data buffer of size %d.\n",data.size);
      // pass received data upwards to application:
      wrapper.tkbuf = data;
      return (PrepareReceivedBuffer (wrapper, 0));

    }
    else
    {
      ///// for DEBUG only
      //int er = errno;
      //PexorError("\n\nError %d  RequestReceiveAsyncTokensPolling from PEXOR_IOC_GET_ASYNC_BUFFER - %s\n", er, strerror(er));
      ///////////////////
      if (requested)
      {
        int er = errno;
        PexorError(
            "\n\nError %d  RequestReceiveAsyncTokensPolling: no data from PEXOR_IOC_GET_ASYNC_BUFFER although requested- %s\n", er, strerror(er));
        return (pexor::DMA_Buffer*) -1;    //
      }
      PexorDebug("RequestReceiveAsyncTokensPolling is requesting next polling ioctl... .\n");
      // no user buffer yet in queue, issue a new request first:
      rev = ioctl (fFileHandle, PEXOR_IOC_REQUEST_ASYNC_POLLING);    // polling is done inside kernel module

      if (rev)
      {
        int er = errno;
        if(er==EAGAIN){
          PexorInfo("RequestReceiveAsyncTokensPolling sees EAGAIN.\n");
          return 0;
        }
        //if(er==512) return 0; //ERESTARTSYS is 512 as errno? means timeout for receive polling, try again


        PexorError(
            "\n\nError %d  RequestReceiveAsyncTokensPolling from PEXOR_IOC_REQUEST_ASYNC_POLLING - %s\n", er, strerror(er));
        return (pexor::DMA_Buffer*) -1;    // this is a real error inside ioctl
      }
      requested = true;
    }
  }    // while true
  PexorInfo("RequestReceiveAsyncTokensPolling  NEVER COME HERE \n");
  return (pexor::DMA_Buffer*) -1; // NEVER COME HERE
}
/////////////////////////////////////////////////////////////////////////////////////////////////77
/////////////////////////////////////////////////////////////////////////////////////////////////



pexor::DMA_Buffer* PexorTwo::PrepareReceivedBuffer(struct pexor_token_io & descriptor, int* dmabuf)
{
	int* rcvbuffer=(int*) descriptor.tkbuf.addr;
	pexor::DMA_Buffer* result=0;
	if(dmabuf)
		{
			/* user specified sg target buffer, we check if this was filled:*/
			if(rcvbuffer!=dmabuf)
			{
				PexorError("Error when receiving buffer: address %x not matching provided sg buffer\n",rcvbuffer,dmabuf);
				return (pexor::DMA_Buffer*) -1;
			}
			result=new pexor::DMA_Buffer(rcvbuffer,descriptor.tkbuf.size); // type dummy to wrap existing user buffer
		}
	else
		{
		result=Find_DMA_Buffer(rcvbuffer);
		if(result==0)
			{
				PexorError("Error when receiving buffer: address %x not mapped in board DMA pools (N.C.H.)\n",rcvbuffer);
				return (pexor::DMA_Buffer*) -1;
			}
		}
	result->SetUsedSize(descriptor.tkbuf.size);
	return result;
}


bool  PexorTwo::WaitForTrigger()
{
	int rev=ioctl(fFileHandle, PEXOR_IOC_WAIT_TRIGGER, &fLastTriggerStatus);
	if(rev==PEXOR_TRIGGER_FIRED)
		return true;
	if(rev==PEXOR_TRIGGER_TIMEOUT)
		return false;
	int er=errno;
	PexorWarning("\nWaitForTrigger returned error %d  -  %s\n",er, strerror(errno));
	return false; // error or signal abort from ioctl

}


bool PexorTwo::DumpTriggerStatus()
{
  PexorInfo("Last Trigger type:0x%x, LocalEventCounter:0x%x, SubevtInvalid:0x%x, TrigMismatch:0x%x, DelayInterruptLine:0x%x, TotalDeadTime:0x%x, DataReady:0x%x",
      GetTriggerType(), GetLocalEventCounter(),GetSubeventInvalid(),GetTriggerMismatch(), GetDelayInterruptLine(), GetTotalDeadTime(), GetDataReady());
  return true;
}


bool PexorTwo::StartAcquisition()
{
  struct pexor_trixor_set desc;
  desc.command=PEXOR_TRIX_GO;
  int rev=ioctl(fFileHandle, PEXOR_IOC_SET_TRIXOR, &desc);
  int er=errno;
  if(rev==0)
      return true;
  else
    PexorWarning("\nStartAcquisition returned error %d  -  %s\n",er, strerror(er));
  return false;
}

  bool PexorTwo::StopAcquisition()
  {
    struct pexor_trixor_set desc;
    desc.command=PEXOR_TRIX_HALT;
    int rev=ioctl(fFileHandle, PEXOR_IOC_SET_TRIXOR, &desc);
    int er=errno;
    sleep(2); // try to give system time to receive stop trigger
    if(rev==0)
        return true;
    else
      PexorWarning("\nStopAcquisition returned error %d  -  %s\n",er, strerror(er));
    return false;
  }

  bool PexorTwo::ResetTrigger()
  {
    struct pexor_trixor_set desc;
    desc.command=PEXOR_TRIX_RES;
    int rev=ioctl(fFileHandle, PEXOR_IOC_SET_TRIXOR, &desc);
    int er=errno;
    if(rev==0)
        return true;
    else
      PexorWarning("\nResetTrixor returned error %d  -  %s\n",er, strerror(er));
   return false;
  }

bool  PexorTwo::SetTriggerTimes(unsigned short fctime, unsigned short cvtime)
  {
     struct pexor_trixor_set desc;
     desc.command=PEXOR_TRIX_TIMESET;
     desc.fct=fctime;
     desc.cvt=cvtime;
     int rev=ioctl(fFileHandle, PEXOR_IOC_SET_TRIXOR, &desc);
     int er=errno;
     if(rev==0)
         return true;
     else
       PexorWarning("\nSetTriggerTimes returned error %d  -  %s\n",er, strerror(errno));
     return false;

  }

bool PexorTwo::SetWaitTimeout (int seconds)
{
  int rev = ioctl (fFileHandle, PEXOR_IOC_SET_WAIT_TIMEOUT, &seconds);
  int er = errno;
  if (rev == 0)
    return true;
  else
    PexorWarning("\nSetWaitTimeout to %d seconds returned error %d  -  %s\n", seconds, er, strerror(errno));
  return false;

}



bool PexorTwo::SetAutoTriggerReadout(bool on, bool directdma)
{

  int rev= SetDeviceState(on ? PEXOR_STATE_TRIGGERED_READ: PEXOR_STATE_STOPPED);
  // todo: tell driver about directdma mode for auto readout

  return (rev ? false : true);
}

pexor::DMA_Buffer* PexorTwo::WaitForTriggerBuffer ()
{
  int rev = 0;
  struct pexor_token_io wrapper;
  struct pexor_trigger_readout descriptor;
  rev = ioctl (fFileHandle, PEXOR_IOC_WAITBUFFER, &descriptor);
  if (rev)
  {
    if(rev==PEXOR_TRIGGER_TIMEOUT)
    {
      PexorError("\nWaitForTriggerBuffer - trigger timeout!\n");
      return (pexor::DMA_Buffer*) -1;
    }

    int er = errno;
    PexorError("\n\nError %d  on WaitForTriggerBuffer - %s\n", er, strerror(er));
    return 0;
  }

  fLastTriggerStatus = descriptor.triggerstatus;
  wrapper.tkbuf = descriptor.data;
  return (PrepareReceivedBuffer (wrapper, 0));
  // TODO: change interface of PrepareReceived Buffer to userbuf instead of token_io
}



} // namespace

