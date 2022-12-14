/*
 * \file
 * PexorTwo.h
 *
 *  Created on: 27.01.2010
 *      Author: adamczew
 */

#ifndef PEXORTWO_H_
#define PEXORTWO_H_

#include "Pexor.h"


namespace pexor {

/**
 * Implements functionality for the PEXOR 2/3 board
 * will add sfp related functionality for exploder etc.
 * */

class PexorTwo: public pexor::Pexor {

	friend class pexor::DMA_Buffer;

public:
	PexorTwo(unsigned int id=0);

	virtual ~PexorTwo();


	/** Request next token buffer from all connected devices on channel.
	 * bufid (0,1) will switch the double buffer id on frontends
	 * if sync is true, method blocks until dma is complete and returns filled dma buffer
	 * if sync is false, method returns before buffer is complete
	 * and user must call WaitForToken() subsequently to get token buffer
	 * For sg DMA, optionally we can specify pointer to desired receive buffer and a write offset within
	 * this buffer to skip the mbs event headers in the DMA*/
   pexor::DMA_Buffer* RequestToken(const unsigned long channel, const int bufid, bool sync=false, bool directdma=true,int* dmabuf=0, unsigned int woffset=0);


   /** Request next token buffer from all connected devices on all sfps specified bitwise by channelmask:
    *  1: sfp 0, 2: sfp 1, 4: sfp 2, 8: sfp 3, 0xf: all four SFPs
    * This feature requires pexor driver with "direct dma" mode enabled (default)
    * bufid (0,1) will switch the double buffer id on frontends
    * if sync is true, method blocks until dma is complete and returns filled dma buffer
    * if sync is false, method returns before buffer is complete
    * and user must call WaitForToken() subsequently to get token buffer
    * NOTE: sg DMA emulation is not supported here due to direct dma mode to coherent host buffer)*/
   pexor::DMA_Buffer* RequestMultiToken(const unsigned long channelmask, const int bufid, bool sync=false, bool directdma=true);



   /** Request next token buffer from all connected devices on all sfps specified bitwise by channelmask:
    *  1: sfp 0, 2: sfp 1, 4: sfp 2, 8: sfp 3, 0xf: all four SFPs
    *  Method blocks until dma is complete and returns filled dma buffer
    *  In contrast to RequestMultiToken, this method does not use "direct dma", but
    *  parallel requests on all chains with subsequent DMA from PEXOR RAM to driver buffer
    *  This is protected against concurrent control ioctls to gosip*/
   pexor::DMA_Buffer* RequestReceiveAllTokens(const unsigned long channelmask, const int bufid, int* dmabuf=0, unsigned int woffset=0);


   /** Request next token buffer from all connected devices on all registered sfps in triggerless/asynchronous mode.
       *  Method returns filled dma buffer if any data was available.
       *  If no data was available, pointer value -1 is returned for further user polling.
       *  In contrast to RequestMultiToken, this method does not use "direct dma", but
       *  parallel requests on all chains with subsequent DMA from PEXOR RAM to driver buffer
       *  When called the next time, already requested sfp chains are not requested again, but checked for data arrival only.
       *  These request states are handled in kernel module.
       *  This is protected against concurrent control ioctls to gosip*/
   pexor::DMA_Buffer* RequestReceiveAsyncTokens (int* dmabuf=0, unsigned int woffset=0);


   /** Request next token buffer from all connected devices on all registered sfps in triggerless/asynchronous mode.
    *  Method returns filled dma buffer if any data was available.
    *  In contrast to RequestReceiveAsyncTokens, this method will poll internally and should always return a received buffer.
    *  Pointer value -1 is returned to indicate a receive error only!
    *  This is protected against concurrent control ioctls to gosip*/
   pexor::DMA_Buffer* RequestReceiveAsyncTokensPolling ();


   /** Just try to receive next buffer from used queue. This should be filled by kernel worker function automatically
    * after started.*/
   pexor::DMA_Buffer* ReceiveNextAsyncBuffer ();


   /** for triggerless asynchronous readout: start polling acquisition in kernel module worker.
    * argument mode specifies ringbuffer (=1) or backpressure(=0) behaviour in output queue*/
   bool StartTriggerlessAcquisition(int mode=0);

   /** for triggerless asynchronous readout: stop polling acquisition in kernel module worker.
    * Already acquired buffers in output queue are not dropped by this operation.*/
   bool StopTriggerlessAcquisition();


   /** Wait until next token buffer has arrived for sfp channel
    * needs a previous RequestToken in async mode
    * directdma mode must match to that of previous RequestToken calls
    * For sg DMA, optionally we can specify pointer to desired receive buffer and a write offset within
	 * this buffer to skip the mbs event headers in the DMA
	 * sync flag is used to enable/disable warnings if wait encounters time out*/
   pexor::DMA_Buffer* WaitForToken(const unsigned long channel, bool directdma=true, int* dmabuf=0, unsigned int woffset=0,  bool sync=true);



   /** receive next token buffer that was filled from kernel module by automatic trigger readout.
    * This mode needs to be enabled by SetAutoTriggerReadout() method.
    * Buffer contains data of all configured sfp channels.*/
   pexor::DMA_Buffer* WaitForTriggerBuffer();

   /** Wait until a trigger interrupt occurs, indicating that there is new data on the frontends
    * Returns true if trigger was fired within the timeout interval specified in the driver
    * Returns false if timeout was expired without getting trigger interrupt*/
    bool  WaitForTrigger();

    /** Enable trigger module acquisition (send GO)*/
    bool StartAcquisition();

    /** Disable trigger module acquisition (send HALT)*/
    bool StopAcquisition();

    /** reset trixor, i.e. clear deadtime flag of trigger module. After this call DAQ
     * will accept a new trigger*/
    bool ResetTrigger();

    /** Set time windows for trigger module:
     * fast clear time, conversion time, see Trixor manual*/
    bool SetTriggerTimes(unsigned short fctime, unsigned short cvtime);


    /** switch board to automatic token readout for each trigger.
     * In this mode, user application simply calls WaitForTriggerBuffer
     * without the need to explicitely request token data. */
    bool SetAutoTriggerReadout(bool on=true, bool directdma=true);

    /** set timeout of kernel waiting queues for triggers and receive buffers
     * This will influence behaviour of methods WaitForTrigger and WaitForTriggerBuffer*/
    bool SetWaitTimeout(int seconds);


    /** Type of last received trigger. Is set by WaitForTrigger() */
    uint8_t GetTriggerType(){
        return fLastTriggerStatus.typ;
    }

    /** Event counter word of last received trigger. Is set by WaitForTrigger() */
    uint8_t GetLocalEventCounter(){
            return fLastTriggerStatus.lec;
    }

    /** Subevent invalid  state of last received trigger. Is set by WaitForTrigger() */
    uint8_t GetSubeventInvalid(){
      return fLastTriggerStatus.si;
    }

    /** Trigger mismatch state of last received trigger. Is set by WaitForTrigger() */
    uint8_t GetTriggerMismatch(){
         return fLastTriggerStatus.mis;
    }

    /** Delay interrupt line  of last received trigger. Is set by WaitForTrigger() */
    uint8_t GetDelayInterruptLine(){
            return fLastTriggerStatus.di;
       }

    /** Total dead time on/off  of last received trigger. Is set by WaitForTrigger() */
    uint8_t GetTotalDeadTime(){
               return fLastTriggerStatus.tdt;
          }

    /** Data ready state of last received trigger. Is set by WaitForTrigger() */
    uint8_t GetDataReady(){
                   return fLastTriggerStatus.eon;
              }
    /** print out most recent trigger status values*/
    bool DumpTriggerStatus();


protected:

    /** utility to convert driver descriptor to matching pexor dma buffer container*/
    pexor::DMA_Buffer* PrepareReceivedBuffer(struct pexor_token_io & descriptor, int* dmabuf=0);


    /** keep status variables of most recent trigger*/
    struct pexor_trigger_status fLastTriggerStatus;


};

}

#endif /* PexorTwo_H_ */
