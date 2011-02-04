/*
 * PexorOne.cpp
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#include "PexorOne.h"

namespace pexor {

PexorOne::PexorOne(unsigned int id) : Pexor(id)
{
	PexorDebug("PexorOne ctor for id %d",id);
}

PexorOne::~PexorOne()

{
	PexorDebug("PexorOne dtor");
}






} // namespace

