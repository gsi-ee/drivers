/*
 * PexorTwo.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef PEXORTWO_H_
#define PEXORTWO_H_

#include "Pexor.h"


namespace pexor {

/*
 * Implements functionality for the PEXOR 2/3 board
 * will add sfp related functionality for exploder etc.
 * */

class PexorTwo: public pexor::Pexor {

	friend class pexor::DMA_Buffer;

public:
	PexorTwo(unsigned int id=0);

	virtual ~PexorTwo();


	/* Request next token buffer from all connected devices on channel.
	 * bufid (0,1) will switch the double buffer id on frontends
	 * if sync is true, method blocks until dma is complete and returns filled dma buffer
	 * if sync is false, method returns before buffer is complete
	 * and user must call WaitForToken() subsequently to get token buffer*/
   pexor::DMA_Buffer* RequestToken(const unsigned long channel, const int bufid, bool sync=0);


   /* Wait until next token buffer has arrived for sfp channel
    * needs a previous RequestToken in async mode*/
   pexor::DMA_Buffer* WaitForToken(const unsigned long channel);



   /* Wait until a trigger interrupt occurs, indicating that there is new data on the frontends
    * Returns true if trigger was fired within the timeout interval specified in the driver
    * Returns false if timeout was expired without getting trigger interrupt*/
    bool  WaitForTrigger();

    /* Enable trigger module acquisition (send GO)*/
    bool StartAcquisition();

    /* Disable trigger module acquisition (send HALT)*/
    bool StopAcquisition();

    /* reset trixor, i.e. clear deadtime flag of trigger module. After this call DAQ
     * will accept a new trigger*/
    bool ResetTrigger();

    /* Set time windows for trigger module:
     * fast clear time, conversion time, see Trixor manual*/
    bool SetTriggerTimes(unsigned short fctime, unsigned short cvtime);



};

}

#endif /* PexorTwo_H_ */
