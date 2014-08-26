/*
 * \file
 * PexorOne.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef PEXORONE_H_
#define PEXORONE_H_

#include "Pexor.h"


namespace pexor {

/**
 * Implements reduced functionality for the PEXOR 1 board
 * In principle no extras to the base functionality
 * */

class PexorOne: public pexor::Pexor {

	friend class pexor::DMA_Buffer;

public:
	PexorOne(unsigned int id=0);

	virtual ~PexorOne();




};

}

#endif /* PEXORONE_H_ */
