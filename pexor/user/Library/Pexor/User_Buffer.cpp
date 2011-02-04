/*
 * User_Buffer.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "User_Buffer.h"

namespace pexor {

User_Buffer::User_Buffer(size_t size) : pexor::Buffer(size)

{
	fData= new int[Length()];
}

User_Buffer::~User_Buffer()
{
	delete [] fData;
}


}// namespace
