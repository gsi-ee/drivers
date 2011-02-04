/*
 * Board.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "Board.h"
#include "User_Buffer.h"
#include "DMA_Buffer.h"


namespace pexor {

Board::Board(const std::string devicename) :
		fDeviceName(devicename), fFileHandle(0), fPoolsMutex(true)

{
	//PexorPrint(MSG_INFO,"Board ctor is executed %s", "initially");
	PexorInfo("pexor::Board ctor for device %s",devicename.c_str());
	fDMA_Pools.clear();
}

Board::~Board()
{

}

int Board::Open()
{
	int er=0;
	if(fFileHandle) return -1;
	int filehandle = ::open(fDeviceName.c_str(), O_RDWR );
	if (filehandle < 0)
		{
			er=errno;
			PexorError("Error %d opening device %s - %s", er, fDeviceName.c_str(), strerror(er));
			return filehandle;
		}
	else
		{
			PexorInfo("Board successfully opened device %s", fDeviceName.c_str());
			fFileHandle=filehandle;
			return 0;
		}
}


int Board::Close()
{
	if(fFileHandle==0) return -1;
	Remove_All_DMA_Pools();
	int rev=0,er=0;
	PexorInfo("PEXOR BOARD closing device %s...\n",fDeviceName.c_str());
	rev= close(fFileHandle);
	if(rev)
		{
			er=errno;
			PexorError("ERROR %d on Board::Close() - %s",er, strerror(er));
		}
	fFileHandle = 0;
	return rev;
}

int Board::Read(int *destination, int length, int boardoffset)
{
	PexorDebug("Read(int *destination, int length, int boardoffset)");
	int rev = 0, er = 0;
	int localbuf=0;
	rev = pread(fFileHandle, destination, length * sizeof (int), boardoffset);
	if(rev<0)
	{
		er=errno;
		PexorError("ERROR %d on reading - %s",er, strerror(er));
	}
	return rev;
}

int Board::Read(pexor::User_Buffer *buf, int length, int bufcursor, int boardoffset)
{
	PexorDebug("Read(pexor::User_Buffer *buf, int length, int bufcursor, int boardoffset)");
	if(!buf)
		return -1;

	int *dest = buf->Cursor(bufcursor);
	if(!dest)
	{
		PexorWarning("Board::Read() cursor out of buffer range");
		return -1;
	}
	if(!buf->Cursor(bufcursor+length-1))
	{
		PexorWarning("Board::Read() - start cursor %d +  read length %d are exceeding buffer length %d",bufcursor, length, buf->Length());
		return -1;
	}
	PexorDebug("Read(pexor::User_Buffer *buf has dest=0x%x len=0x%x offs=0x%x",dest, length, boardoffset );

	return (Read(dest, length, boardoffset));
}


int Board::Write(pexor::User_Buffer *buf, int length, int bufcursor, int boardoffset)
{
	PexorDebug("Write(pexor::User_Buffer *buf, int length, int bufcursor, int boardoffset)");
	if(!buf)
		return -1;

	int *src = buf->Cursor(bufcursor);
	if(!src)
	{
		PexorWarning("Board::Write() cursor out of buffer range");
		return -1;
	}
	if(!buf->Cursor(bufcursor+length-1))
	{
		PexorWarning("Board::Write() - start cursor %d  + write length %d  are exceeding buffer length %d", bufcursor, length, buf->Length());
		return -1;
	}
	return (Write(src, length, boardoffset));
}


int Board::Write(int *source, int length, int boardoffset)
{
	PexorDebug("Write(int *source, int length, int boardoffset)");
	int rev = 0, er = 0;
	rev = pwrite(fFileHandle, source, length * sizeof (int), boardoffset);
	if(rev<0)
	{
		er=errno;
		PexorError("ERROR %d on writing - %s",rev,strerror(er));

	}
	return rev;
}

int Board::SetDMA(pexor::DmaMode mode)
{
	fDmaMode = mode;
}

int pexor::Board::Add_DMA_Pool(size_t size, int numbufs, const std::string name)
{
	 pexor::DMA_Pool* pool = new pexor::DMA_Pool(this, size, numbufs, name);
	 pexor::LockGuard g(&fPoolsMutex);
	 try
		{
			 fDMA_Pools.push_back(pool);
			 PexorInfo("Added DMA Pool %s for size %d with %d buffers",name.c_str(), size, numbufs);
		}
	 catch(std::exception &e)
		{
		 PexorError("pexor::Board failed to Add_DMA_Pool %s for size %d with %d buffers, got standard exception: %s ",name.c_str(), size, numbufs, e.what());
		   return -1;
		}
	 catch(...)
		{
		 PexorError("pexor::Board failed to Add_DMA_Pool %s for size %d with %d buffers, got unknown exception!\n",name.c_str(), size, numbufs);
		   return -2;
		}
	 return 0;
}




pexor::DMA_Pool * pexor::Board::Get_DMA_Pool(const std::string name)
{
    pexor::LockGuard g(&fPoolsMutex);
	std::vector<pexor::DMA_Pool*>::iterator iter;
	for(iter=fDMA_Pools.begin(); iter!=fDMA_Pools.end(); ++iter)
		{
			pexor::DMA_Pool* entry= *iter;
			 try
				{
				 if(entry->GetName()== name)
					return entry;
				}
			 catch(std::exception &e)
				{
				 PexorError("pexor::Board failed to find DMA pool by name %s with standard exception: %s ",name.c_str(), e.what());
				   return 0;
				}
			 catch(...)
				{
				 PexorError("pexor::Board failed to find DMA pool by name %s with unknown exception!\n",name.c_str());
				   return 0;
				}
		}
	return 0;
}

int pexor::Board::Remove_DMA_Pool(const std::string name)
{

	 pexor::DMA_Pool* rempool=0;
	 std::vector<pexor::DMA_Pool*>::iterator iter;
	 {
	 pexor::LockGuard g(&fPoolsMutex);
		 for(iter=fDMA_Pools.begin(); iter!=fDMA_Pools.end(); ++iter)
			  {
			   pexor::DMA_Pool* pool=*iter;
				 try
					{
						 std::string pname=pool->GetName();
						 if(name==pname)
						  {
							 PexorInfo("pexor::Board removed DMA pool %s ",name.c_str());
							 fDMA_Pools.erase(iter);
							 rempool=pool;
							 break;
						  }
					}
				 catch(std::exception &e)
					{
					 PexorError("pexor::Board failed to remove DMA pool %s with standard exception: %s ",name.c_str(), e.what());
					   return -1;
					}
				 catch(...)
					{
					 PexorError("pexor::Board failed to remove DMA pool %s with unknown exception!\n",name.c_str());
					   return -2;
					}
			  }
	 }  // end lockguard
	 delete rempool; // delete outside lockguard
     return 0;
    }

int pexor::Board::Add_DMA_Buffers(size_t size, int numbufs)
{
		// look if pool for desired size already exists
	pexor::LockGuard g(&fPoolsMutex);
	size=pexor::Buffer::NextPageSize(size);
	pexor::DMA_Pool * dmapool=Get_DMA_Pool(size);
	if(dmapool)
		{
			// expand it if there
			if(!dmapool->Expand(numbufs)) return -1;
		}
	else
		{
			// new pool
			std::ostringstream oss;
		    oss << "DMA_Pool_" << size <<"_b";
			return (Add_DMA_Pool(size,numbufs,oss.str()));
		}
	return 0;
}

pexor::DMA_Pool *pexor::Board::Get_DMA_Pool(size_t size)
{
    pexor::LockGuard g(&fPoolsMutex);
	std::vector<pexor::DMA_Pool*>::iterator iter;
		for(iter=fDMA_Pools.begin(); iter!=fDMA_Pools.end(); ++iter)
			{
				pexor::DMA_Pool* entry= *iter;
				 try
					{
						 if(entry->GetBufferSize()== size)
								 return entry;
					}
				 catch(std::exception &e)
					{
					 PexorError("pexor::Board failed to find DMA pool of size %d with standard exception: %s ",size, e.what());
					   return 0;
					}
				 catch(...)
					{
					   PexorError("pexor::Board failed to find DMA pool of size %d with unknown exception!\n",size);
					   return 0;
					}
			}
	return 0;
}

pexor::DMA_Buffer *Board::New_DMA_Buffer(size_t size)
{
	pexor::DMA_Buffer *buf = new pexor::DMA_Buffer(this, size);
	if(buf->Data()==0)
	{
		// something went wrong in the mapping: remove invalid buffer object
		PexorError("\nError  in Board::NewDMA_Buffer \n");
		delete buf;
		buf=0;
	}
	return buf;
}

int pexor::Board::Remove_All_DMA_Pools()
{
	PexorInfo("PEXOR BOARD removing all DMA pools ...\n");
	std::vector<pexor::DMA_Pool*> removepools;
	   { // begin lock
		  pexor::LockGuard g(&fPoolsMutex);
	      removepools=fDMA_Pools; // backup list of pools
	      fDMA_Pools.clear(); // now "official" list is cleared
	   } // end lock
	   // delete dim commands in backuped list outside lock:
	   std::vector<pexor::DMA_Pool*>::const_iterator iter;
	   for(iter=removepools.begin(); iter!=removepools.end(); ++iter)
	      {
			   pexor::DMA_Pool* pool=*iter;
			   std::string pname=pool->GetName();
			   try
	            {
	               delete pool;
	            }
	         catch(std::exception &e)
	            {
	        	   PexorError("pexor::Board failed to remove DMA pool %s with standard exception: %s ",pname.c_str(), e.what());
	               continue;
	            }
	         catch(...)
	            {
				   PexorError("pexor::Board failed to remove DMA pool %s with unknown exception ",pname.c_str());
	               continue;
	            }
	      }// for
	   return 0;

}

pexor::DMA_Buffer *pexor::Board::Find_DMA_Buffer(int *address)
{
    pexor::LockGuard g(&fPoolsMutex);
	std::vector<pexor::DMA_Pool*>::iterator iter;
	for(iter=fDMA_Pools.begin(); iter!=fDMA_Pools.end(); ++iter)
	{
		pexor::DMA_Pool* entry= *iter;
		try
			{
				// find the buffer in pool:
				pexor::DMA_Buffer* buf= dynamic_cast<pexor::DMA_Buffer*> ( entry->FindBuffer(address) );
				return buf;
			}
		 catch(std::exception &e)
			{
			 PexorError("pexor::Board failed to find DMA buffer of address %lx with standard exception: %s ",address, e.what());
			   return 0;
			}
		 catch(...)
			{
			 PexorError("pexor::Board failed to find DMA buffer of address %lx with unknown exception!\n",address);
			   return 0;
			}
	} // for
	return 0;
}



}//namespace
