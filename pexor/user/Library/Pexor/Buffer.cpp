/*
 * Buffer.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "Buffer.h"
#include "Logger.h"


#include <unistd.h> // sysconf(3)

namespace pexor {

Buffer::Buffer(size_t bytes) :
		fSize(bytes), fUsedSize(0), fData(0)

{
	fErrorIndices.clear();
}

Buffer::~Buffer()
{


}

size_t Buffer::NextPageSize(size_t bytes)
{
	int pagesize=sysconf( _SC_PAGE_SIZE );
	int rest=bytes % pagesize;
	if(rest)
		{
			bytes=bytes-rest + pagesize;
			PexorDebug("Buffer::NextPageSize expanding to %d bytes.\n",bytes);
		}
	return bytes;
}

bool Buffer::operator==(const pexor::Buffer &other)
{
	fErrorIndices.clear();
	if(this==&other) return true;
	int buflen=other.Length();
	if (buflen!=Length()) return false;
	bool isequal=true;
	const pexor::Buffer& myself = *this;
	PexorDebug("Buffer::operator== comparing %d integers:\n",buflen);
	for(int i=0; i<buflen;++i)
		{
			if(myself[i]!=other[i])
				{
					fErrorIndices.push_back(i);
					isequal=false;
				}
		}// for
return isequal;
}




int  Buffer::ShowCompareErrors(bool printstats)
{
	int ercnt=fErrorIndices.size();
	if(printstats)
		{
			PexorInfo("Buffer::operator== found %d compare errors (ratio %e) .\n",ercnt, (float) ercnt / (float) Length());
			PexorDebug("Printout of mismatching buffers:\n");
			for(int i=0; i<ercnt; ++i)
				{
					if((i%10)==0) PexorDebug("\n");
					int j=fErrorIndices[i];
					PexorDebug("b[%d]=%d\t",j,Cursor(j));
				}
		}
	return ercnt;
}


   /* assignment operator for deep copy of contents*/
pexor::Buffer& Buffer::operator=(const pexor::Buffer &source)
  {
	if (this != &source)
		{
			int buflen=source.Length();
			if (buflen>Length()) buflen=Length();
			// we do not change our buffer size here!
			// take minimum of both buffer sizes as copy length and optionally fill up rest with zeros:
			for(int i=0; i<buflen; ++i)
				{
					*(Data()+i) = *(source.Data()+i);
				}
			for(int i=buflen; i<Length(); ++i)
				{
					*(Data()+i)=0;
				}
		}
		return *this;
  }

}// namespace
