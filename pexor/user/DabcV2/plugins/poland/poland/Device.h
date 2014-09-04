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

extern const char* xmlExploderSubmem;    // exploder submem size for testbuffer

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

  /** Forwarded interface for user defined readout:
   * User code may overwrite the default behaviour (gosip token dma)
   * For example, optionally some register settings may be added to buffer contents*/
  virtual unsigned Read_Start (dabc::Buffer& buf);

  /** Forwarded interface for user defined readout:
   * User code may overwrite the default behaviour (gosip token dma)
   * For example, optionally some register settings may be added to buffer contents*/
  virtual unsigned Read_Complete (dabc::Buffer& buf);

protected:

  /** fill token buffers of all slave devices with test event data*/
  bool InitQFWs ();


private:


};

}    // namespace

#endif
