/*
 * BufferPool.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "BufferPool.h"
#include "Buffer.h"

namespace pexor {


BufferPool::BufferPool(pexor::BufferType tp, size_t buflen, int numbufs, const std::string name)
: fBufferType(tp), fNumBuffers(numbufs), fBufsize(buflen), fName(name), fBufMutex(true)
{
	fBuffers.clear();
	//fFreeIndices.clear();
}

BufferPool::~BufferPool()
{
	RemoveAllBuffers();
}

bool BufferPool::Expand(int addnum, bool init)
{
	pexor::LockGuard g(&fBufMutex);
	if(init) RemoveAllBuffers();
	pexor::Buffer* buf=0;
	for(int i=0; i<addnum; ++i)
	{

		 try
			{
			 buf = CreateBuffer();
			 if(buf)
				 {
					 fBuffers.push_back(buf);
					 //fFreeIndices.push_back(fNumBuffers); // initially all new buffers are free
					 if(fBufsize!= buf->Size())
						 {
							 fBufsize=buf->Size();
							 PexorDebug("Buffer Pool %s resets size to %d.\n",GetName().c_str(),fBufsize);
						 }
					 fNumBuffers++;
				 }
			else
				 {
					// TODO: exception for error handling
					PexorError("pexor::BufferPool %s failed expanding to %d entries after adding %d buffers",GetName().c_str(),addnum, i);
					return false;
				 }
			 }
		 catch(std::exception &e)
			{
				 delete buf;
				 PexorError("pexor::BufferPool %s failed expanding to %d entries with standard exception: %s ",GetName().c_str(),addnum, e.what());
			     return false;
			}
		 catch(...)
			{
				 delete buf;
				 PexorError("pexor::BufferPool %s failed expanding to %d entries with unknown exception!\n",GetName().c_str(),addnum);
				 return false;
			}
	}
	return true;

}

void BufferPool::BuildPool()
{
	Expand(fNumBuffers, true);

}


int BufferPool::RemoveAllBuffers()
{
	PexorInfo("Buffer Pool %s removing all buffers...\n",GetName().c_str());
	std::vector<pexor::Buffer*> removebuffers;
   { // begin lock
	  pexor::LockGuard g(&fBufMutex);
	  removebuffers=fBuffers; // backup list of pools
	  fBuffers.clear(); // now "official" list is cleared
	  fNumBuffers=0;
	  //fFreeIndices.clear();
   } // end lock
   // delete  buffers in backuped list outside lock:
   std::vector<pexor::Buffer*>::const_iterator iter;
   for(iter=removebuffers.begin(); iter!=removebuffers.end(); ++iter)
	  {
		   pexor::Buffer* buf=*iter;
		   try
			{
			   delete buf;
			}
		 catch(std::exception &e)
			{
			   PexorError("pexor::BufferPool failed to remove buffer %lx with standard exception: %s ",buf, e.what());
			   continue;
			}
		 catch(...)
			{
			   PexorError("pexor::BufferPool failed to remove buffer %lx with unknown exception ",buf);
			   continue;
			}
	  }// for
   return 0;



}


int BufferPool::RemoveBuffer(pexor::Buffer* buf)
{
	 pexor::LockGuard g(&fBufMutex);
	 pexor::Buffer* rembuffer=0;
	 int ix=0;
	 std::vector<pexor::Buffer*>::iterator iter;
	 for(iter=fBuffers.begin(); iter!=fBuffers.end(); ++iter)
		  {
		   pexor::Buffer* entry=*iter;
			 try
				{
					 if(buf==entry)
					  {
						 PexorDebug("pexor::BufferPool removed buffer %lx ",buf);
						 fBuffers.erase(iter);
						 //RemoveFreeIndex(ix);
						 fNumBuffers--;
						 rembuffer=entry;
						 break;
					  }
					 ix++;
				}
			 catch(std::exception &e)
				{
				 PexorError("pexor::BufferPool failed to remove buffer %lx with standard exception: %s ",buf, e.what());
				   return -1;
				}
			 catch(...)
				{
				 PexorError("pexor::BufferPool failed to remove buffer %lx with unknown exception!\n",buf);
				   return -2;
				}
		  }
	delete rembuffer;
	return ix; // pass previous index of removed buffer (to subclass)


}





pexor::Buffer* BufferPool::FindBuffer(int* address)
{
	pexor::LockGuard g(&fBufMutex);
		std::vector<pexor::Buffer*>::iterator iter;
		for(iter=fBuffers.begin(); iter!=fBuffers.end(); ++iter)
			  {
			   pexor::Buffer* entry=*iter;
				 try
					{
						 if(entry->Data()==address)
						  {
							 PexorDebug("pexor::BufferPool found buffer %lx for pointer %lx", entry, address);
							 return entry;
						  }
					}
				 catch(std::exception &e)
					{
					 PexorError("pexor::BufferPool failed to find buffer of address %lx with standard exception: %s ",address, e.what());
					   return 0;
					}
				 catch(...)
					{
					 PexorError("pexor::BufferPool failed to remove buffer of address %lx with unknown exception!\n",address);
					   return 0;
					}
			  }// for
	return 0;

}


//pexor::Buffer* BufferPool::UseBuffer()
//{
//	try
//	{
//	pexor::LockGuard g(&fBufMutex);
//		if(fFreeIndices.empty()) return 0;
//		int ix=fFreeIndices.front();
//		fFreeIndices.erase(fFreeIndices.begin());
//		return fBuffers.at(ix); // checks out of range and may do exception
//	}
//	catch(std::exception &e)
//	{
//		PexorError("pexor::BufferPool UseBuffer got standard exception: %s ", e.what());
//		return 0;
//	}
//	catch(...)
//	{
//		PexorError("pexor::BufferPool UseBuffergot  unknown exception!\n");
//		return 0;
//	}
//
//}
//
//bool  BufferPool::ReleaseBuffer(pexor::Buffer* buf)
//{
//	pexor::LockGuard g(&fBufMutex);
//	int ix=0;
//	std::vector<pexor::Buffer*>::iterator iter;
//	 for(iter=fBuffers.begin(); iter!=fBuffers.end(); ++iter)
//		 {
//			 pexor::Buffer* entry=*iter;
//			 if(entry==buf)
//			 {
//				 return (AddFreeIndex(ix));
//			 }
//			 ix++;
//		 }
//	return false;
//
//}
//
//bool BufferPool::AddFreeIndex(int i)
//{
//	pexor::LockGuard g(&fBufMutex);
//	// look up if this index is already in our list:
//	std::vector<int>::iterator iter;
//		 for(iter=fFreeIndices.begin(); iter!=fFreeIndices.end(); ++iter)
//			 {
//				 if(*iter==i)
//					 {
//						 PexorWarning("pexor::BufferPool AddFreeIndex: buffer of index %d is already free!\n",i);
//						 return false;
//					 }
//			 }
//	fFreeIndices.push_back(i);
//	return true;
//}
//
//bool BufferPool::RemoveFreeIndex(int i)
//{
//	pexor::LockGuard g(&fBufMutex);
//	std::vector<int>::iterator iter;
//	bool found=false;
//	for(iter=fFreeIndices.begin(); iter!=fFreeIndices.end(); ++iter)
//		 {
//			 if(*iter==i)
//				 {
//					 fFreeIndices.erase(iter);
//					 found=true;
//					 break;
//				 }
//		 }
//	if(found)
//		{
//		 // need to correct all free indices above the removed one, since buffer array was shrinked above the removed slot
//			for(iter=fFreeIndices.begin(); iter!=fFreeIndices.end(); ++iter)
//				 {
//					 if(*iter > i)
//							 (*iter)--;
//				 }
//			return true;
//		}
//	else
//		{
//			PexorWarning("pexor::BufferPool RemoveFreeIndex: buffer of index %d was not in free list!\n",i);
//			return false;
//		}
//	}




} // namespace
