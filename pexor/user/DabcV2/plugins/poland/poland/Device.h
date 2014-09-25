/********************************************************************
 * The Data Acquisition Backbone Core (DABC)
 ********************************************************************
 * Copyright (C) 2009- 
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH 
 * Planckstr. 1
 * 64291 Darmstadt
 * Germany
 * Contact:  http://dabc.gsi.de
 ********************************************************************
 * This software can be used under the GPL license agreements as stated
 * in LICENSE.txt file which is part of the distribution.
 ********************************************************************/
#ifndef POLAND_Device
#define POLAND_Device

#include "pexorplugin/Device.h"



namespace poland
{

extern const char* xmlOffsetTriggerType;    //< trigger type to read out frontend offfset values
extern const char* commandReadOffsets ;

class Device: public pexorplugin::Device
{

public:

  Device (const std::string& name, dabc::Command cmd);
  virtual ~Device ();

  virtual const char* ClassName () const
  {
    return "explodertest::Device";
  }

  virtual int ExecuteCommand (dabc::Command cmd);

//  /** Forwarded interface for user defined readout:
//   * User code may overwrite the default behaviour (gosip token dma)
//   * For example, optionally some register settings may be added to buffer contents*/
//  virtual unsigned Read_Start (dabc::Buffer& buf);
//
//  /** Forwarded interface for user defined readout:
//   * User code may overwrite the default behaviour (gosip token dma)
//   * For example, optionally some register settings may be added to buffer contents*/
//  virtual unsigned Read_Complete (dabc::Buffer& buf);

  /** interface for user subclass to implement different readout variants depending on the triggertype.
    * The default implementation will issue retry/timeout on start/stop acquisition trigger and
    * a standard token request with direct dma for all other trigger types.
    * Return value should be size of received buffer in bytes.*/
   virtual int User_Readout(dabc::Buffer& buf, uint8_t trigtype);


   /** generic initialization function for daq and frontends.
     * To be overwritten in subclass and callable by command interactively, without shutting down
     * application.*/
    virtual int InitDAQ();

protected:

  /** fill token buffers of all slave devices with test event data*/
  bool InitQFWs ();

  /** initiate read out of poland offset registers */
  int SendOffsetTrigger();

  /** trigger type to read out offset trigger */
  uint8_t fOffsetTrigType;


private:


};

}    // namespace

#endif
