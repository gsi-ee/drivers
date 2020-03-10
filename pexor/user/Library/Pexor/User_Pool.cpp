/*
 * User_Pool.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "User_Pool.h"
#include "User_Buffer.h"

namespace pexor {


User_Pool::User_Pool(size_t buflen, int numbufs, std::string name) :
		pexor::BufferPool(pexor::BUFFER_USER, buflen,numbufs,name)
{
	fFreeIndices.clear();
}

User_Pool::~User_Pool()
{

}

pexor::Buffer *User_Pool::CreateBuffer()
{
	pexor::LockGuard g(&fBufMutex);
	fFreeIndices.push_back(fNumBuffers); // initially all new buffers are free

	return ( new pexor::User_Buffer(GetBufferSize()) );
}

pexor::Buffer* User_Pool::UseBuffer()
{
	try
	{
	pexor::LockGuard g(&fBufMutex);
		if(fFreeIndices.empty()) return 0;
		int ix=fFreeIndices.front();
		fFreeIndices.erase(fFreeIndices.begin());
		return fBuffers.at(ix); // checks out of range and may do exception
	}
	catch(std::exception &e)
	{
		PexorError("pexor::User_Pool UseBuffer got standard exception: %s ", e.what());
		return 0;
	}
	catch(...)
	{
		PexorError("pexor::User_Pool UseBuffergot  unknown exception!\n");
		return 0;
	}

}

bool  User_Pool::ReleaseBuffer(pexor::Buffer* buf)
{
	pexor::LockGuard g(&fBufMutex);
	int ix=0;
	std::vector<pexor::Buffer*>::iterator iter;
	 for(iter=fBuffers.begin(); iter!=fBuffers.end(); ++iter)
		 {
			 pexor::Buffer* entry=*iter;
			 if(entry==buf)
			 {
				 return (AddFreeIndex(ix));
			 }
			 ix++;
		 }
	return false;

}

bool User_Pool::AddFreeIndex(int i)
{
	pexor::LockGuard g(&fBufMutex);
	// look up if this index is already in our list:
	std::vector<int>::iterator iter;
		 for(iter=fFreeIndices.begin(); iter!=fFreeIndices.end(); ++iter)
			 {
				 if(*iter==i)
					 {
						 PexorWarning("pexor::User_Pool AddFreeIndex: buffer of index %d is already free!\n",i);
						 return false;
					 }
			 }
	fFreeIndices.push_back(i);
	return true;
}

bool User_Pool::RemoveFreeIndex(int i)
{
	pexor::LockGuard g(&fBufMutex);
	std::vector<int>::iterator iter;
	bool found=false;
	for(iter=fFreeIndices.begin(); iter!=fFreeIndices.end(); ++iter)
		 {
			 if(*iter==i)
				 {
					 fFreeIndices.erase(iter);
					 found=true;
					 break;
				 }
		 }
	if(found)
		{
		 // need to correct all free indices above the removed one, since buffer array was shrinked above the removed slot
			for(iter=fFreeIndices.begin(); iter!=fFreeIndices.end(); ++iter)
				 {
					 if(*iter > i)
							 (*iter)--;
				 }
			return true;
		}
	else
		{
			PexorWarning("pexor::User_Pool RemoveFreeIndex: buffer of index %d was not in free list!\n",i);
			return false;
		}
	}


int User_Pool::RemoveAllBuffers()
{
	fFreeIndices.clear();
	return (pexor::BufferPool::RemoveAllBuffers());

}

int User_Pool::RemoveBuffer(pexor::Buffer* buf)
{
	 pexor::LockGuard g(&fBufMutex);
	 int ix=pexor::BufferPool::RemoveBuffer(buf);
	 if(ix<0) return ix;
	 RemoveFreeIndex(ix);
	 return 0;
}

} // namespace
