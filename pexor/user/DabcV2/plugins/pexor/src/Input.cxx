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
#include "pexorplugin/Input.h"
#include "pexorplugin/Device.h"

#include "dabc/Port.h"
#include "dabc/logging.h"
pexorplugin::Input::Input (pexorplugin::Device* dev) :
    dabc::DataInput (), fPexorDevice (dev)
// provide input and output buffers
{
  DOUT2 ("Created new pexorplugin::Input\n");
  fErrorRate.Reset();
}

pexorplugin::Input::~Input ()
{

}

double pexorplugin::Input::Read_Timeout ()
  {
    return (fPexorDevice->Read_Timeout ());
  }


unsigned pexorplugin::Input::Read_Size ()
{
  return (fPexorDevice->Read_Size ());
}

unsigned pexorplugin::Input::Read_Start (dabc::Buffer& buf)
{
  DOUT2 ("Read_Start() with bufsize %d\n", buf.GetTotalSize ());
  unsigned res = fPexorDevice->Read_Start (buf);
  return ((unsigned) res == dabc::di_Ok) ? dabc::di_Ok : dabc::di_Error;
}

unsigned pexorplugin::Input::Read_Complete (dabc::Buffer& buf)
{
  DOUT2 ("Read_Complete()\n");
  unsigned res = fPexorDevice->Read_Complete (buf);
  if ((unsigned) res == dabc::di_SkipBuffer)
  {
    fErrorRate.Packet (buf.GetTotalSize ());
    return dabc::di_SkipBuffer;
  }
  if ((unsigned) res == dabc::di_RepeatTimeOut)
  {
    DOUT3 ("pexorplugin::Input() returns with timeout\n");
    return dabc::di_RepeatTimeOut;
  }
  if ((unsigned) res == dabc::di_Error)
   {
      DOUT1 ("pexorplugin::Input() returns with Error\n");
      return dabc::di_Error;
   }
  return res > 0 ? dabc::di_Ok : dabc::di_Error; // JAM2016 probably this check fails on 64 bit system
}

