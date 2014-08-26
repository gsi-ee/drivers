/*
 * \file
 * User_Pool.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef USER_POOL_H_
#define USER_POOL_H_
#include "BufferPool.h"

namespace pexor {
/**
 * Memory collection of user buffers of same size
 * */
class User_Pool: public pexor::BufferPool {
public:
	User_Pool(size_t buflen, int numbufs, std::string name);
	virtual ~User_Pool();


	/** Create allocated user space buffer*/
	virtual pexor::Buffer* CreateBuffer();


	/** returns next unused buffer to the user. This buffer will be locked in
		 * pool as used until FreeBuffer() is called. Returns 0 if no more buffer available*/
		virtual pexor::Buffer* UseBuffer();



		/** put used buffer back into free list. Returns false if buffer does not
		 * belong to the pool. If buffer is already free, returns also true*/
		virtual bool ReleaseBuffer(pexor::Buffer* buf);


		/** remove and delete all buffers: add free list management here*/
		virtual int RemoveAllBuffers();


		/** remove buf from pool and delete it. 0 if success, negative if not found. add free list management for user pool*/
		virtual int RemoveBuffer(pexor::Buffer* buf);


private:

	/** adds index of free buffer i to free list. checks if index was already there and returns false if so.*/
		bool AddFreeIndex(int i);

		/** removes index of free buffer i from free list. checks if index was indeed there and returns false if not.*/
		bool RemoveFreeIndex(int i);


		/** contains the fBuffers indices for the "not in use" buffers*/
		std::vector  <int> fFreeIndices;


};

}

#endif /* USER_POOL_H_ */
