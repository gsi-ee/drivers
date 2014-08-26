/*
 * \file
 * DMA_Pool.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef DMA_POOL_H_
#define DMA_POOL_H_

#include "BufferPool.h"

namespace pexor {

class Board;

/**
 * collection of memory buffers mapped for DMA read
 * */

class DMA_Pool: public pexor::BufferPool {
public:
	DMA_Pool(pexor::Board* owner, size_t buflen, int numbufs, const std::string name);
	virtual ~DMA_Pool();

	/** Add addnum more buffers of defined buflen to the pool.
	 * Overwritten base class method to add limits functionality.*/
	virtual bool Expand(int addnum, bool init=false);

	/** Create mmaped DMA Buffer from kernel space*/
	virtual pexor::Buffer* CreateBuffer();


	/** returns next unused buffer to the user. This buffer will be locked in
	 * pool as used until ReleaseBuffer() is called. Returns 0 if no more buffer available*/
	virtual pexor::Buffer* UseBuffer();



	/** put used buffer back into free list. Returns false if buffer does not
	 * belong to the pool. If buffer is already free, returns also true*/
	virtual bool ReleaseBuffer(pexor::Buffer* buf);


protected:

	/** Test system limits for mmap and adjust them to match the sum of all pool memory + additional bytes
	 *  Returns -1 in case of error, 1 if limits were changed, 0 if no change was necessary*/
	static int AdjustLimits(int additional_bytes);


private:

	/** bookkeeping of mapped DMA memory from all pool instances.
	 * used for testing/adjusting the limits. */

	static int fDMA_Memory;


	/** back reference to the board device that owns the kernel buffers*/
	pexor::Board* fBoard;

};

}

#endif /* DMA_POOL_H_ */
