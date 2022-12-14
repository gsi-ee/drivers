/*
 * Pexor.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "Pexor.h"
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string>
#include <errno.h>

#include "DMA_Buffer.h"


namespace pexor {

Pexor::Pexor(unsigned int boardid) : Board(std::string("/dev/")),fbUseSGBuffers(false)
{
	PexorDebug("Pexor ctor");
	char devname[256];
	snprintf(devname,256,PEXORNAMEFMT, boardid);
	fDeviceName += std::string(devname);
	PexorInfo("pexor::Pexor ctor set device name to %s \n",fDeviceName.c_str());
	Open();
	//Reset(); // get rid of previous buffers in case of formerly crash
	           // JAM: we do not always want to have this, user must call it explicitely!
	           // note that DMA buffers are removed anyway at Close!
}

Pexor::~Pexor()

{
	PexorDebug("Pexor dtor");
	Close();
}

int Pexor::Reset()
{
    int rev=0, errsv=0;
	PexorDebug("Pexor::Reset()");
	rev=ioctl(fFileHandle, PEXOR_IOC_CLEAR_RCV_BUFFERS);
	errsv = errno;
	if(rev)
	{
	  PexorError("Error %d clearing receive buffers PEXOR 1 (%s) !\n",errsv, strerror (errsv));
	}
	Remove_All_DMA_Pools(); // get rid of dma buffer references, since reset will cleanup all remaining ones

// JAM2016-8 TEST TEST
	rev=ioctl(fFileHandle, PEXOR_IOC_RESET);
	errsv = errno;
	if(rev)
		{
			PexorError("Error %d (%s) resetting PEXOR 1 !\n",errsv, strerror (errsv));
		}
	return rev;

}


int Pexor::Register_DMA_Buffer(int* buf, size_t size)
{
	struct pexor_userbuf descriptor;
	descriptor.addr= (unsigned long) buf;
	descriptor.size = size;
	PexorInfo("Pexor::Register_DMA_Buffer() is sg mapping buffer %lx", descriptor.addr);
	int rev=::ioctl(fFileHandle, PEXOR_IOC_MAPBUFFER , &descriptor);
	int errsv=errno;
	if(rev)
		{
			PexorError("\n\nError %d sg-mapping buffer at  address %lx (%s)\n",errno,descriptor.addr, strerror (errsv));
			//delete buffer;
			return rev;
		}
	return 0;
}

int Pexor::Unregister_DMA_Buffer(int* buf)
{
	int rev=0;
	struct pexor_userbuf descriptor;
	descriptor.addr= (unsigned long) buf;

	PexorInfo("Pexor::Unregister_DMA_Buffer() is sg unmapping buffer %lx", descriptor.addr);
	rev=::ioctl(fFileHandle, PEXOR_IOC_UNMAPBUFFER , &descriptor);
	int errsv=errno;
	if(rev)
		{
			PexorError("\n\nError %d sg-unmapping buffer at  address %lx (%s)\n",errno,descriptor.addr, strerror (errsv));
			return rev;
		}
	return 0;
}

pexor::DMA_Buffer* Pexor::Map_Physical_DMA_Buffer(unsigned long physaddr, size_t size)
{

	/* TODO: replace this method by extension of New_DMA_Buffer and DMA_Buffer arguments by physaddr.
	 * In this case fully transparent with or without memory pool*/
	unsigned long  pagesize=sysconf( _SC_PAGE_SIZE );
	double rest= (physaddr % pagesize);
	if(rest)
		physaddr=physaddr-rest +pagesize;
	PexorInfo("Pexor::Map_Physical_DMA_Buffer is mapping buffer at %lx for size %d, pagesize:%d, rest:%d ",
			physaddr, size, pagesize, rest);

	int* userptr=Map_DMA_Buffer(size, physaddr);
	if(userptr==0)
		{
			PexorError("\n\nError mapping physical dma buffer at  address %lx\n",physaddr);
			return 0;
		}
	return new pexor::DMA_Buffer(userptr, size);
}

int  Pexor::Unmap_Physical_DMA_Buffer(pexor::DMA_Buffer* dbuf)
{
	int rev=Delete_DMA_Buffer(dbuf);
	if(rev==0) delete dbuf;
	return rev;
}



int* Pexor::Map_DMA_Buffer(size_t size, unsigned long physaddr)
{
	if(fbUseSGBuffers)
		{
			// create or use user space buffer and do sg mapping in driver
			int* buffer = new int[size/sizeof(int)];
			if(Register_DMA_Buffer(buffer, size))
				{
					delete buffer;
					return 0;
				}
			return buffer;
		}
	else
		{
		// get buffer from driver and map it to user space
		PexorDebug("Pexor::Map_DMA_Buffer()");
		void* ptr= mmap(0,size, PROT_READ | PROT_WRITE , MAP_SHARED | MAP_LOCKED , fFileHandle, (off_t) physaddr); /* | PROT_WRITE , | MAP_LOCKED*/
		if(ptr==MAP_FAILED)
			{
				int er=errno;
				PexorError("\nError %d Mapping DMA buffer - %s\n", er,strerror(er));
				return 0;
			}
		return (int*) ptr;
	}
}

int Pexor::Delete_DMA_Buffer(pexor::DMA_Buffer *buffer)
{
	if(!buffer) return -1;
	int rev=0;
	struct pexor_userbuf descriptor;
	descriptor.addr= (unsigned long) buffer->Data();
	descriptor.size = buffer->Size();

	if(fbUseSGBuffers)
		{
		int* bufptr=(int*) (descriptor.addr);
		rev=Unregister_DMA_Buffer(bufptr);
		if(rev)
		{
			return rev;
		}
		delete [] bufptr;

		}
	else
		{
				// unmap virtual address and let driver delete associated kernel buffer
		rev= ::munmap((void*) descriptor.addr, descriptor.size);
		if(rev)
			{
				int er=errno;
				PexorError("\nError %d Unmapping DMA buffer at  address %lx - %s\n", er,descriptor.addr, strerror(er));
			}
		PexorDebug("Pexor::DeleteDMA_Buffer() deleting buffer at address %lx", descriptor.addr);
		rev=::ioctl(fFileHandle, PEXOR_IOC_DELBUFFER , &descriptor);
		if(rev)
			{
		        int er=errno;
				PexorError("\n\nError %d deleting buffer at  address %lx (%s)\n",er,descriptor.addr, strerror(er));
			}

		}
	return rev;

}


int Pexor::Free_DMA_Buffer(pexor::DMA_Buffer* buf)
{
	if(!buf) return -1;
	int rev=0;
	struct pexor_userbuf descriptor;
	descriptor.addr= (unsigned long) buf->Data();
	rev=ioctl(fFileHandle, PEXOR_IOC_FREEBUFFER, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d freeing buffer at  address %lx\n",er,descriptor.addr, strerror(er));
		}
	return rev;

}


pexor::DMA_Buffer* Pexor::Take_DMA_Buffer(bool checkpool)
{
	struct pexor_userbuf descriptor;
	pexor::DMA_Buffer* result;
	int rev=ioctl(fFileHandle, PEXOR_IOC_USEBUFFER, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("Take_DMA_Buffer: Error %d (%s) from driver \n",er, strerror(er));
			return 0;
		}



	int* rcvbuffer=(int*) descriptor.addr;

	if(checkpool)
	{
		result=Find_DMA_Buffer(rcvbuffer);
		if(result==0)
			{
				PexorError("Take_DMA_Buffer: address %x not mapped in board DMA pools (N.C.H.)\n",rcvbuffer);
			}
		else if(descriptor.size!=result->Size())
			{
				PexorWarning("Take_DMA_Buffer: descriptor size %d mismatch with corresponding DMA buffer size %d \n",descriptor.size, result->Size());
			}
		else{}
	}
	else
		{
			result=new pexor::DMA_Buffer(rcvbuffer, descriptor.size);
		}
	return result;
}

int Pexor::SetDeviceState(int state)
{
	int rev=ioctl(fFileHandle, PEXOR_IOC_SETSTATE, &state);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d setting to state %d (%s)\n",er,state, strerror(er));
		}
	return rev;
}



int Pexor::SetDMA(pexor::DmaMode mode)
{
	Board::SetDMA(mode);
	int rev=-1;
	switch(mode)
	{
		case pexor::DMA_STOPPED :   // disabled DMA
			rev=SetDeviceState(PEXOR_STATE_STOPPED);
			break;

		case pexor::DMA_SINGLE :  	// initiate a single DMA to next free buffer and stop
			rev=SetDeviceState(PEXOR_STATE_DMA_SINGLE);
			break;

		case pexor::DMA_CHAINED : 	// each filled DMA buffer will initate next DMA -> DMA "flow mode" in driver
			rev=SetDeviceState(PEXOR_STATE_DMA_FLOW);
			break;
		case pexor::DMA_AUTO :
		default:
			PexorWarning("Unknown DMA mode %d for PEXOR1\n",mode);
			break;
	};
return rev;
}

int Pexor::WriteDMA(pexor::DMA_Buffer *buf, int length, int bufcursor, int boardoffset)
{
	PexorWarning("Pexor::WriteDMA() not yet implemented\n");
	return -1;
}



pexor::DMA_Buffer* Pexor::ReceiveDMA(bool checkmempool)
{

	int rev=0;
	struct pexor_userbuf descriptor;
	int* rcvbuffer;
		//if(Debugmode>0) printf("Wait for buffer...\n");
		rev=ioctl(fFileHandle, PEXOR_IOC_WAITBUFFER, &descriptor);
		if(rev)
			{
		        int er=errno;
				PexorError("\n\nError %d  (%s) waiting for next receive buffer\n",er,strerror(er));
				return 0;
			}
	rcvbuffer=(int*) descriptor.addr;
	if(!checkmempool)
	{
		return new pexor::DMA_Buffer (rcvbuffer,descriptor.size);
	}


	pexor::DMA_Buffer* result=Find_DMA_Buffer(rcvbuffer);
	if(result==0)
		{
			PexorError("Error when receiving buffer: address %x not mapped in board DMA pools (N.C.H.)\n",rcvbuffer);
		}
	else if(descriptor.size!=result->Size())
		{
			PexorWarning("DMA descriptor size %d mismatch with corresponding DMA buffer size %d \n",descriptor.size, result->Size());
		}
	else{}

	return result;
}



int Pexor::ReadDMA(pexor::DMA_Buffer *buf, int length, int bufcursor, int boardoffset)
{
	PexorWarning("Pexor::ReadDMA() not yet implemented\n");
	return -1;
}


int Pexor::Test_IRQ()
{
	PexorInfo("Pexor::Test_IRQ() tries to raise irq in board... \n");
	return (SetDeviceState(PEXOR_STATE_IR_TEST));

}

int Pexor::InitBus(const unsigned long channel, const unsigned long maxdevice)
{
	int rev=0;
	struct pexor_bus_io descriptor;
	descriptor.sfp=channel;
	descriptor.slave=maxdevice;
	rev=ioctl(fFileHandle, PEXOR_IOC_INIT_BUS, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d  on initializing channel %lx, maxdevices %lx - %s\n",er,channel,maxdevice, strerror(er));
		}
	return rev;

}



int Pexor::WriteBus(const unsigned long address, const unsigned long value, const int channel, const long device)
{
	int rev=0;
	struct pexor_bus_io descriptor;
	//PexorInfo("WriteBus writes %x to %x \n",value, address);
	descriptor.address=address;
	descriptor.value=value;
	descriptor.sfp=channel;
	descriptor.slave=device;
	rev=ioctl(fFileHandle, PEXOR_IOC_WRITE_BUS, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d  on writing value %lx to address %lx - %s\n",er,value,address, strerror(er));
		}
	return rev;
}


int Pexor::ReadBus(const unsigned long address, unsigned long& value, const int channel, const long device)
{
	int rev=0;
	struct pexor_bus_io descriptor;
	descriptor.address=address;
	descriptor.value=0;
	descriptor.sfp=channel;
	descriptor.slave=device;
	rev=ioctl(fFileHandle, PEXOR_IOC_READ_BUS, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d  on reading from address %lx - %s\n",er, address, strerror(er));
			return rev;
		}
	value=descriptor.value;
	return 0;
}

int  Pexor::WriteRegister(const char bar, const unsigned int address, const unsigned int value)
{
	int rev=0;
	struct pexor_reg_io descriptor;
	descriptor.bar=bar;
	descriptor.address=address;
	descriptor.value=value;
	rev=ioctl(fFileHandle, PEXOR_IOC_WRITE_REGISTER, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d  on writing register value %lx to address %lx - %s\n",er,value,address, strerror(er));
		}
	return rev;
}

int  Pexor::ReadRegister(const char bar, const unsigned int address, unsigned int& value)
{
	int rev=0;
	struct pexor_reg_io descriptor;
	descriptor.address=address;
	descriptor.value=0;
	descriptor.bar=bar;
	rev=ioctl(fFileHandle, PEXOR_IOC_READ_REGISTER, &descriptor);
	if(rev)
		{
	        int er=errno;
			PexorError("\n\nError %d  on reading from register address %lx - %s\n",er, address, strerror(er));
			return rev;
		}
	value=descriptor.value;
	return 0;
}





} // namespace

