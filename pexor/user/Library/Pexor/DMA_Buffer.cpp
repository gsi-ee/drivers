/*
 * DMA_Buffer.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "DMA_Buffer.h"
#include "Board.h"
#include "DMA_Pool.h"

namespace pexor {

DMA_Buffer::DMA_Buffer(pexor::Board* device, size_t len) :
		pexor::Buffer(Buffer::NextPageSize(len)), fBoard(device)

{
	fData=fBoard->Map_DMA_Buffer(len);

}

DMA_Buffer::DMA_Buffer(int* ptr, size_t len) :
		pexor::Buffer(len), fBoard(0)

{
	fData=ptr;
	fSize=len;
}

DMA_Buffer::~DMA_Buffer()

{
	if(fBoard)
		fBoard->Delete_DMA_Buffer(this);
	DMA_Pool::fDMA_Memory-=fSize; //account globally for process memlock getrlimit
	//printf("deleted DMA buffer 0x%x, size:0x%x, gives total dma memory:0x%x",this,fSize,DMA_Pool::fDMA_Memory);
	if(DMA_Pool::fDMA_Memory<0)
      PexorError("NEVER COME HERE: pexor::DMA_Buffer destructor decreased total dma memory to %d bytes  < 0, double deletion?\n",this,DMA_Pool::fDMA_Memory);
}

} // namespace
