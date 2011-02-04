/*
 * Buffer.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef BUFFER_H_
#define BUFFER_H_

#include <stdlib.h>
#include <vector>
#include <sys/mman.h>

namespace pexor {

class Buffer {
public:

	/* specify buffer length in bytes here
	 * Note that all pexor buffers will be set to multiples of system PAGESIZE (4k)
	 * so actual size is probably larger than constructor parameter*/
	Buffer(size_t bytes);


	virtual ~Buffer();

	/* utility to calculate size of next memory page above bytes.*/
	static size_t NextPageSize(size_t bytes);


	/* return pointer into buffer at position (integer counts)
	 * returns 0 if position is out of range
	 * TODO: exceptions?*/
	int* Cursor(int position)
		{
			if(position<0 || position >= Length() ) return 0;
			return fData + position;
		}

	int operator[](int i) const
			{
				return *(fData+i);
			}


    int* Data() const
    {
        return fData;
    }

    size_t  Size() const
    {
        return fSize;
    }

    int Length() const
    {
    	return Size()/sizeof(int);
    }


    size_t  UsedSize() const
        {
            return fUsedSize;
        }
    void SetUsedSize(size_t set)
		{
			size_t s=set;
			if(s>fSize) s=fSize;
			fUsedSize=s;
		}



     /* buffer compare operator. Each mismatching buffer element (integer) will store its index
      * in error indices list, thus incrementing error counter=length of this list*/
     bool operator==(const pexor::Buffer &other);

     bool operator!=(const pexor::Buffer &other)
    		 {
				 return !(*this == other);
    		 }


     /* Returns number of errors found in the previous compare operation by operator== .
      * if printstats is true, will print info about error statistics to current logger*/
     int ShowCompareErrors(bool printstats=true);


     /* assignment operator for deep copy of contents*/
    pexor::Buffer  &operator=(const pexor::Buffer &source);



protected:

	/* begin of Data field*/
	int* fData;

	/* allocated size in bytes*/
	size_t fSize;

	/* optional used size */
	size_t fUsedSize;

	/* indices of mismatching data elements from last compare operation.
	 * size is number of errors*/
	std::vector<int> fErrorIndices;


};

}

#endif /* BUFFER_H_ */
