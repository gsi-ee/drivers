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


pexor::DMA_Buffer* PexorTwo::RequestToken(const unsigned long channel, const int bufid, bool sync, int* dmabuf, unsigned int woffset)

{
	PexorDebug("PexorTwo::RequestToken with write offset 0x%x",woffset);
	int rev=0;
	struct pexor_token_io descriptor;
	descriptor.bufid=bufid;
	descriptor.sfp=channel;
	descriptor.sync=0;
	descriptor.tkbuf.addr= (unsigned long) dmabuf;
	descriptor.offset=woffset;
	if(sync) descriptor.sync=1;
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


pexor::DMA_Buffer* PexorTwo::WaitForToken(const unsigned long channel, int* dmabuf, unsigned int woffset)
{
	int rev=0;
	struct pexor_token_io descriptor;
	descriptor.sfp=channel;
	descriptor.tkbuf.addr=(unsigned long) dmabuf;
	descriptor.offset=woffset;
	rev=ioctl(fFileHandle, PEXOR_IOC_WAIT_TOKEN, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d  on wait token from channel 0x%x - %s\n",er, channel, strerror(er));
			return (pexor::DMA_Buffer*) -1;
		}
	return (PrepareReceivedBuffer(descriptor, dmabuf));


}

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
	int rev=ioctl(fFileHandle, PEXOR_IOC_WAIT_TRIGGER);
	if(rev==PEXOR_TRIGGER_FIRED)
		return true;
	if(rev==PEXOR_TRIGGER_TIMEOUT)
		return false;
	int er=errno;
	PexorWarning("\nWaitForTrigger returned error %d  -  %s\n",er, strerror(errno));
	return false; // error or signal abort from ioctl

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

} // namespace

