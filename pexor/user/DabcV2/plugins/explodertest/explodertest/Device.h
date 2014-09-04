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
#ifndef EXPLODERTEST_Device
#define EXPLODERTEST_Device

#include "pexorplugin/Device.h"

/** number of connected sfps*/
#define PEXORPLUGIN_NUMSFP 4

// address map for slave (exploder): this is user specific data concerning the pexor board, so it is not available from PexorTwo.h
#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
#define REG_MODID     0xFFFFE0
#define REG_HEADER    0xFFFFE4
#define REG_FOOTER    0xFFFFE8
#define REG_DATA_LEN  0xFFFFEC

/** number of peaks in random spectrum*/
#define NUM_PEAK 5

namespace explodertest
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
  bool WriteTestBuffers ();

  /** random event functions stolen from TGo4MbsRandom code:*/
  double gauss_rnd (double mean, double sigma);
  /** random event functions stolen from TGo4MbsRandom code:*/
  double get_int (double low, double high);
  /** random event functions stolen from TGo4MbsRandom code:*/
  unsigned long Random_Event (int choice);

private:

  /** fill token buffers of all slaves with generated test data*/
  bool fTestData;

  /** size of each exploder submemory (byte). for test buffer set up*/
  unsigned int fSubmemSize;

  static double fgdPeak[];
  static double fgdSigma[];

  unsigned int fuSeed;

};

}    // namespace

#endif
