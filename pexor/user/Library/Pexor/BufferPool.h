/*
 * BufferPool.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef BUFFERPOOL_H_
#define BUFFERPOOL_H_

#include <vector>
#include <string>

#include "Logger.h"
#include "Mutex.h"


namespace pexor {

class Buffer;

enum BufferType {
BUFFER_NONE = 0,
BUFFER_USER =1,
BUFFER_DMA_READ = 2,
BUFFER_DMA_WRITE = 3,
};



/*
 * Manages a collection of Buffers of same size.
 * */
class BufferPool {
public:
	/* creates pool of numbufs with type tp, each with buflen length in bytes*/
	BufferPool(pexor::BufferType tp, size_t buflen, int numbufs, const std::string name);

	virtual ~BufferPool();

	/* Add addnum more buffers of defined buflen to the pool.
	 * If init is true, discard all old buffers and create addnum new ones*/
	virtual bool Expand(int addnum, bool init=false);


	/* remove and delete all buffers*/
	virtual int RemoveAllBuffers();


	/* remove buf from pool and delete it. returns previous index of removed buffer in vector, negative if not found*/
	virtual int RemoveBuffer(pexor::Buffer* buf);

	/* check if buffer is within this pool*/
	/*bool HasBuffer(pexor::Buffer* buf);*/

	/* returns next unused buffer to the user. This buffer will be locked in
	 * pool as used until FreeBuffer() is called. Returns 0 if no more buffer available*/
	virtual pexor::Buffer* UseBuffer()=0;

	/* searches buffer of virtual user address in pool and returns its handle-
	 * Returns 0 if buffer of address not found*/
	pexor::Buffer* FindBuffer(int* address);


	/* put used buffer back into free list. Returns false if buffer does not
	 * belong to the pool. If buffer is already free, returns also true*/
	virtual bool ReleaseBuffer(pexor::Buffer* buf)=0;

  /* optional identifier name*/
	std::string GetName() const
    {
        return fName;
    }

    /* the common buffer size in this pool (bytes)*/
    size_t GetBufferSize() const
		{
			return fBufsize;
		}

    /* the actual number of buffers in pool*/
    int GetBufferNum() const
   		{
   			return fBuffers.size();
   		}
    /* the actual number of buffers in pool*/
    pexor::BufferType GetBufferType() const
		{
			return fBufferType;
		}



protected:

	/* initialization method to be called by subclass constructor.
	 * Will fill pool of specified size using the subclass buffer factory method CreateBuffer*/
	void BuildPool();

	/* Create Buffer of specified type. Factory method for subclass*/
	virtual pexor::Buffer* CreateBuffer()=0;



//	/* adds index of free buffer i to free list. checks if index was already there and returns false if so.*/
//	bool AddFreeIndex(int i);
//
//	/* removes index of free buffer i from free list. checks if index was indeed there and returns false if not.*/
//	bool RemoveFreeIndex(int i);
	/* type id for buffers*/
	pexor::BufferType fBufferType;

	/* the actual number of buffers in pool*/
	int fNumBuffers;

	/* the common buffer size in this pool (bytes)*/
	size_t fBufsize;

	/* identifier*/
	std::string fName;


	/* the ownership list of the buffers */
	std::vector  <pexor::Buffer*> fBuffers;

//	/* contains the fBuffers indices for the "not in use" buffers*/
//	std::vector  <int> fFreeIndices;

		/* protects the buffer vectors*/
	pexor::Mutex fBufMutex;







};

}

#endif /* BUFFERPOOL_H_ */
