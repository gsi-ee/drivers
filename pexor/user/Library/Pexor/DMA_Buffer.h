/*
 * DMA_Buffer.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef DMA_BUFFER_H_
#define DMA_BUFFER_H_

#include "Buffer.h"

namespace pexor {

class Board;


class DMA_Buffer: public pexor::Buffer
{
public:

	/* need reference to device that maps the buffers*/
	DMA_Buffer(pexor::Board* device, size_t length);

	/* construct just a wrapper for already dma mapped sg buffer.
	 * Avoid managing everything with pexor pools by this*/
	DMA_Buffer(int* ptr, size_t length);

	virtual ~DMA_Buffer();

private:

	pexor::Board* fBoard;


};

}

#endif /* DMA_BUFFER_H_ */
