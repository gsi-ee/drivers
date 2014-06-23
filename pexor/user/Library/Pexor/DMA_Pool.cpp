/*
 * DMA_Pool.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "DMA_Pool.h"
#include "DMA_Buffer.h"
#include "Board.h"

#include <sys/resource.h>

namespace pexor {

int DMA_Pool::fDMA_Memory=0;

DMA_Pool::DMA_Pool(pexor::Board* owner, size_t buflen, int numbufs, const std::string name):
		pexor::BufferPool(pexor::BUFFER_DMA_READ,buflen,numbufs, name ), fBoard(owner)
{
	BuildPool();
}

DMA_Pool::~DMA_Pool()
{

}

bool DMA_Pool::Expand(int addnum, bool init)
{
	int futurenum= addnum;
	int oldnum=GetBufferNum();
    if(init) futurenum -= oldnum; // correct for already allocated bufs that will be removed
	int addbytes=futurenum*GetBufferSize();
	if(DMA_Pool::AdjustLimits(addbytes))
		{
			PexorError("DMA_Pool::Expand could not adjust mapping limits to %d bytes",addbytes);
			return false;
		}
	bool rev=BufferPool::Expand(addnum,init);
	fDMA_Memory+= ( GetBufferNum()-oldnum) * GetBufferSize();
	return rev;
}


pexor::Buffer* DMA_Pool::CreateBuffer()
{
	return(fBoard->New_DMA_Buffer(GetBufferSize()));
}

int DMA_Pool::AdjustLimits(int additional_bytes)
{
	/* adjust the system limits before we expand the pool:*/
	struct rlimit locklimits;
	int er=0;
	/* analyze the current system limits here:*/
	if(getrlimit(RLIMIT_MEMLOCK,&locklimits))
		{
			er=errno;
			PexorError("\n\nError getting map locked limits, errno=%d - %s",er,strerror(er));
			return -1;
		}
	int current=locklimits.rlim_cur;
	int max=locklimits.rlim_max;
	PexorDebug("Found previous locked limits of current=%d bytes, max=%d bytes\n",current,max);
	int futuremem=fDMA_Memory+additional_bytes;
	if(futuremem>current)
		{
			locklimits.rlim_cur=futuremem;
			if(futuremem>max)
				locklimits.rlim_max=futuremem;
			/* use setrlimit to increase allowed locked pages size:*/
			if(setrlimit(RLIMIT_MEMLOCK,&locklimits))
			{
				er=errno;
				PexorError("Error setting map locked limits to %d bytes, errno=%d - %s",futuremem,er,strerror(er));
				return -1;
			}
			PexorInfo("DMA_Pool::AdjustLimits has set memory lock limits to current=%d bytes, max=%d bytes \n",locklimits.rlim_cur,locklimits.rlim_max);
			return 0;
		}
return 0;

}


pexor::Buffer* DMA_Pool::UseBuffer()
{
	return (fBoard->Take_DMA_Buffer());
}

bool  DMA_Pool::ReleaseBuffer(pexor::Buffer* buf)
{

	// check if buffer belongs to us:
	pexor::DMA_Buffer* dmabuf=dynamic_cast<pexor::DMA_Buffer*> (buf);
	if(dmabuf==0) return false;
	pexor::LockGuard g(&fBufMutex);
	std::vector<pexor::Buffer*>::iterator iter;
	for(iter=fBuffers.begin(); iter!=fBuffers.end(); ++iter)
		{
			pexor::Buffer* entry=*iter;
				 if(entry==dmabuf)
				 {
					 if(fBoard->Free_DMA_Buffer(dmabuf)!=0) return false;
					 return true;
				 }

			 }
	return false;
}



} //namespace
