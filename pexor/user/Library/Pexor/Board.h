/*
 * Board.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef BOARD_H_
#define BOARD_H_

#include <string>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "Logger.h"
#include "DMA_Pool.h"
#include "Mutex.h"



namespace pexor {

class DMA_Buffer;
class User_Buffer;


enum DmaMode {
DMA_STOPPED = 0, 	// disabled DMA
DMA_SINGLE =1,   	// initiate a single DMA to next free buffer and stop
DMA_CHAINED = 2, 	// each filled DMA buffer will initate next DMA -> DMA "flow mode" in driver
DMA_AUTO = 3		// TODO: DMA is expected to be raised by board when data is received
};


/*
 * The abstract base class of all pexor boards
 * Contains generic methods to communicate with the char driver via dev file system
 * NOTE that all board specific ioctl operations must be implemented in subclasses!
 */

class Board {

	friend class pexor::DMA_Buffer;

public:
	Board(const std::string devicename="");
	virtual ~Board();

	bool IsOpen()
	{
		return (fFileHandle!=0);
	}

	/* Add numbufs DMA buffers of size to the board's buffer pool. Note that these are read buffers.*/
	int Add_DMA_Buffers(size_t size, int numbufs);



	/* allocate and map one new DMA buffer of size. ctor will use backpointer to us and
	 * calls method  Map_DMA_Buffer() as implemented in subclass.*/
		virtual pexor::DMA_Buffer* New_DMA_Buffer(size_t size);


	/* Frees DMA buffer taken from this board and put back to device memory pool.
	 * Return value gives error code from ioctl
	 * Implemented in subclass*/
	virtual int Free_DMA_Buffer(pexor::DMA_Buffer*)=0;



	/* Take (reserve) a DMA buffer for usage in application and returns pointer
	 * will prevent this buffer from filling at dma receive until released by FreeDMA_Buffer.
	 * boolean may checking mode of driver buffer agains Board internal mempool
	 * Returns 0 in case of error
	 * Implemented in subclass*/
	virtual pexor::DMA_Buffer* Take_DMA_Buffer(bool check=true)=0;


	/* Read length integers via PIO from board RAM offset position into user space buffer buf, starting at
	 * buffer offset bufcursor. Returns number of read bytes, or negative error code*/
	int Read(pexor::User_Buffer* buf, int length, int bufcursor=0, int boardoffset=0);

	/*
	 * Read length integers via PIO from board RAM offset position to destination user memory.
	 * Returns number of read bytes, or negative error code
	 * */
	int Read(int* destination, int length, int boardoffset=0);

	/* Write length integers via PIO to board at RAM offset position from user space buffer buf, starting at
		 * buffer offset bufcursor. Returns number of written bytes, or negative error code*/
	int Write(pexor::User_Buffer* buf, int length, int bufcursor=0, int boardoffset=0);


	/*
	 * Read length integers via PIO to board RAM offset position from source user memory.
	 * Returns number of written bytes, or negative error code
	 * */
	int Write (int* source, int length, int boardoffset=0);

	/* switches board into the dma operation mode.
	 * to be extended in subclass due to board specific ioctl values*/
	virtual int SetDMA(pexor::DmaMode mode);

	/* Get next filled DMA buffer; optionally wait until DMA is completed here.
	 * Returns 0 buffer in case of error
	 * Implemented in board subclass, since it may depend on specific ioctl calls
	 * TODO: specify timeout here? may define special ioctl to set driver timeout!
	 * TODO: error handling via exceptions? */
	virtual pexor::DMA_Buffer* ReceiveDMA()=0;

	/* TODO: single DMA write content of DMAbuffer from bufcursor with length to board RAM at boardoffset.*/
	virtual int WriteDMA(pexor::DMA_Buffer* buf, int length, int bufcursor=0, int boardoffset=0)=0;

	/* TODO: single DMA read content of board RAM at boardoffset to DMAbuffer at bufcursor with length.*/
	virtual int ReadDMA(pexor::DMA_Buffer* buf, int length, int bufcursor=0, int boardoffset=0)=0;

	/* write value to the address of the "bus" connected via the optical links.
	 * The actual address space and range of values depends on the implementation.
	 * optional parameters channel and device can be used to partition address space
	 * Return value gives error code*/
	virtual int WriteBus(const unsigned long address, const unsigned long value, const unsigned long channel, const unsigned long device)=0;

	/* reads value from the address of the "bus" connected via the optical links.
	 * The actual address space and range of values depends on the implementation.
	 * optional parameters channel and device can be used to partition address space
	 * Return value gives error code.*/
	virtual int ReadBus(const unsigned long address, unsigned long& value, const unsigned long channel, const unsigned long device)=0;

	/* write value to the "register" at address of the specified PCI bar.
	 * Return value gives error code*/
	virtual int WriteRegister(const char bar, const unsigned int address, const unsigned int value)=0;

	/* read value from the "register" at address of the specified PCI bar.
	 * Return value gives error code.*/
	virtual int ReadRegister(const char bar, const unsigned int address, unsigned int& value)=0;


	/* reset device to initial state*/
	virtual int Reset()=0;

protected:

	int Open();
	int Close();


	/* Create a new pool of numbufs dma buffers with size.
		 * Add it with unique name to list of pools associated with this device */
	int Add_DMA_Pool(size_t size, int numbufs, const std::string name="default-pool");

	int Remove_DMA_Pool(const std::string name);

	/* remove dma pools, delete their buffers*/
	int Remove_All_DMA_Pools();

	/* access pool by name. Returns 0 if no such pool*/
	pexor::DMA_Pool* Get_DMA_Pool(const std::string name);

	/* access pool by size. Returns first pool of size found. Returns 0 if no such pool in list.*/
	pexor::DMA_Pool* Get_DMA_Pool(size_t size);




	/* Allocate and map a DMA kernel buffer from driver. Implemented in subclass
	 * */
	virtual int* Map_DMA_Buffer(size_t size)=0;




	/* find dma buffer object that keeps the address (cookie) we got from the ReceiveDMA.
	 * Returns 0 if no such buffer contained in any of our DMA pools*/
	pexor::DMA_Buffer* Find_DMA_Buffer(int* address);


		/* deletes DMA buffer taken from this board.
		 * Return value gives error code from ioctl
		 * Implemented in subclass*/
	virtual int Delete_DMA_Buffer(pexor::DMA_Buffer*)=0;
	//void Define_DMA_Mode(pexor::DmaMode mode){fDmaMode=mode;}

	pexor::DmaMode& Is_DMA_Mode(){return fDmaMode;}




/* backup value for dmamode set to driver*/
pexor::DmaMode fDmaMode;


/* name of the device file in /dev */
std::string fDeviceName;

/* file descriptor of device file*/
int fFileHandle;

/* keeps all DMA buffer pools associated with this device.
 */
std::vector<pexor::DMA_Pool*> fDMA_Pools;

pexor::Mutex fPoolsMutex;


};

}

#endif /* BOARD_H_ */
