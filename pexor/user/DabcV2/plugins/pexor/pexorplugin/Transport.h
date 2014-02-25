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
#ifndef PEXORPLUGIN_Transport
#define PEXORPLUGIN_Transport

#include "dabc/DataTransport.h"
#include "dabc/statistic.h"

namespace pexorplugin {

   class Device;
   class Input;

   class Transport : public dabc::InputTransport {
      friend class Device;
      friend class Input;

      public:
         Transport(pexorplugin::Device*, pexorplugin::Input* inp, dabc::Command cmd, const dabc::PortRef& inpport);
         virtual ~Transport();

      protected:

//         virtual unsigned Read_Size();
//
//         virtual unsigned Read_Start(dabc::Buffer& buf);
//
//         virtual unsigned Read_Complete(dabc::Buffer& buf);
//
//         virtual double Read_Timeout() { return 10; }

         virtual void ProcessPoolChanged(dabc::MemoryPool* pool);

         pexorplugin::Device* fPexorDevice;

         pexorplugin::Input* fPexorInput;

//         dabc::Ratemeter      fErrorRate;

         virtual bool StartTransport();

         virtual bool StopTransport();

   };
}

#endif
