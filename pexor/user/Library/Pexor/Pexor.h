/*
 * Pexor.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef PEXOR_H_
#define PEXOR_H_

#include "Board.h"
#include "pexor_user.h"


namespace pexor {

/*
 * Implements functionality for the pexor driver
 * base class for all PEXOR board variants
 * subclass may do special things here
 * */

class Pexor: public pexor::Board {

	friend class pexor::DMA_Buffer;

public:
	Pexor(unsigned int boardid=0);

	virtual ~Pexor();

	virtual int Reset();

	/* raise a test irq*/
	int Test_IRQ();


	/* Frees DMA buffer taken from this board and put back to device memory pool.
	 * Return value gives error code from ioctl*/
	virtual int Free_DMA_Buffer(pexor::DMA_Buffer*);

	/* Take (reserve) a DMA buffer for usage in application and returns pointer
	 * will prevent this buffer from filling at dma receive until released by FreeDMA_Buffer.
	 * Returns 0 in case of error*/
	virtual pexor::DMA_Buffer* Take_DMA_Buffer();


	/* switches board into the dma operation mode.
		 * implementation for this board*/
	virtual int SetDMA(pexor::DmaMode mode);

		/* Get next filled DMA buffer; optionally wait until DMA is completed here.
		 * Returns 0 buffer in case of error
		 * Implemented in board subclass, since it may depend on specific ioctl calls
		 * TODO: specify timeout here?
		 * TODO: error handling via exceptions? */
	virtual pexor::DMA_Buffer* ReceiveDMA();

		/* TODO: single DMA write content of DMAbuffer from bufcursor with length to board RAM at boardoffset.*/
   virtual int WriteDMA(pexor::DMA_Buffer* buf, int length, int bufcursor=0, int boardoffset=0);

		/* TODO: single DMA read content of board RAM at boardoffset to DMAbuffer at bufcursor with length.*/
   virtual int ReadDMA(pexor::DMA_Buffer* buf, int length, int bufcursor=0, int boardoffset=0);

   /* write value to the address of the "bus" connected via the optical links.
   	 * The actual address space and range of values is reduced in this implementation.
   	 * Return value gives error code*/
   virtual int WriteBus(const unsigned long address, const unsigned long value, const unsigned long channel, const unsigned long device);


	/* reads value from the address of the "bus" connected via the optical links.
	 * The actual address space and range of values is reduced in this implementation.
	 * Return value gives error code.*/
   virtual int ReadBus(const unsigned long address, unsigned long& value, const unsigned long channel, const unsigned long device);


	/* initialize chain of devices connected to the "bus" at the optical links.
	 * channel and device may specify connection and maximum number of devices.
	 * Return value gives error code.*/
  virtual int InitBus(const unsigned long channel, const unsigned long maxdevice);




   /* write value to the "register" at address of the specified PCI bar.
   	 * Return value gives error code*/
   virtual int WriteRegister(const char bar, const unsigned int address, const unsigned int value);

   	/* read value from the "register" at address of the specified PCI bar.
   	 * Return value gives error code.*/
   virtual int ReadRegister(const char bar, const unsigned int address, unsigned int& value);


protected:

   /*
    * set device into the defined state. Returns 0 on success or error value from ioctl
   	*/
   	virtual int SetDeviceState(int state);

   	/* Allocate and map a DMA kernel buffer from driver*/
   	virtual int* Map_DMA_Buffer(size_t size);


	/* deletes DMA buffer taken from this board.
	 * Return value gives error code from ioctl
	 * Implementation here*/
   	virtual int Delete_DMA_Buffer(pexor::DMA_Buffer*);

};

}

#endif /* Pexor_H_ */
