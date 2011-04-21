/*
 * DMA_Buffer.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "DMA_Buffer.h"
#include "Board.h"

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
}

} // namespace
