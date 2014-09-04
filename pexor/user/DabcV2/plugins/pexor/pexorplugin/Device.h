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
#ifndef PEXORPLUGIN_Device
#define PEXORPLUGIN_Device

#include "dabc/Device.h"
#include "dabc/Object.h"
#include "dabc/MemoryPool.h"


//#include "dabc/Basic.h"
//#include "dabc/Pointer.h"

#include "pexor/PexorTwo.h"

#include "mbs/MbsTypeDefs.h"


/** number of connected sfps*/
#define PEXORPLUGIN_NUMSFP 4

// address map for slave (exploder): this is user specific data concerning the pexor board, so it is not available from PexorTwo.h
//#define REG_BUF0     0xFFFFD0 // base address for buffer 0 : 0x0000
//#define REG_BUF1     0xFFFFD4  // base address for buffer 1 : 0x20000
//#define REG_SUBMEM_NUM   0xFFFFD8 //num of channels 8
//#define REG_SUBMEM_OFF   0xFFFFDC // offset of channels 0x4000
//#define REG_MODID     0xFFFFE0
//#define REG_HEADER    0xFFFFE4
//#define REG_FOOTER    0xFFFFE8
//#define REG_DATA_LEN  0xFFFFEC
//
///** number of peaks in random spectrum*/
//#define NUM_PEAK 5



namespace pexorplugin {


extern const char* xmlPexorID; //< id number N of pexor device file /dev/pexor-N
  extern const char* xmlPexorSFPSlaves; //< prefix for the sfp numbers 0,1,2,3 indicating number of slave devices connected
  extern const char* xmlRawFile; //< name of output lmd file
  extern const char* xmlDMABufLen; //< length of DMA buffers to allocate in driver
  extern const char* xmlDMABufNum; 	//< number of DMA buffers
  extern const char* xmlDMAScatterGatherMode; //< switch scatter gather dma on/off
  extern const char* xmlDMAZeroCopy; //< switch zero copy scatter gather dma on/off
  extern const char* xmlExploderSubmem; //< exploder submem size for testbuffer
  extern const char* xmlFormatMbs; //< enable mbs formating already in device transport
  extern const char* xmlSingleMbsSubevt; //<  use one single subevent for all sfps
  extern const char* xmlMbsSubevtCrate; //<  define crate number for subevent header
  extern const char* xmlMbsSubevtControl; //<  define crate number for subevent header
  extern const char* xmlMbsSubevtProcid; //<  define procid number for subevent header
  extern const char* xmlSyncRead; //< switch synchronous or asynchronous token dma
  extern const char* xmlTriggeredRead; //< switch triggered or polling mode readout
  extern const char* xmlTrixorConvTime; //< conversion time of TRIXOR module
  extern const char* xmlTrixorFastClearTime; //< fast clear time of TRIXOR module
  extern const char* xmlModuleName; //< Name of readout module instance
  extern const char* xmlModuleThread; //< Name of readout thread
  extern const char* xmlDeviceName; //< Name of device instance
  extern const char* xmlDeviceThread; //< Name of readout thread

  extern const char* nameReadoutAppClass;
  extern const char* nameDeviceClass;
  extern const char* nameTransportClass;
  extern const char* nameReadoutModuleClass;
  extern const char* nameInputPool;
  extern const char* nameOutputPool;




   class Device : public dabc::Device {

      public:

         Device(const std::string& name, dabc::Command cmd);
         virtual ~Device();

         /** for zero copy DMA: map complete dabc pool for sg DMA of driver*/
         void MapDMAMemoryPool(dabc::MemoryPool* pool);

         /** Request token from current sfp. If synchronous is true, fill output buffer.
          * if mbs formating is enabled, put mbs headers into buffer
          * If synchronous mode false, return before getting dma buffer,
          * needs to call ReceivetokenBuffer afterwards.*/
         virtual int RequestToken(dabc::Buffer& buf, bool synchronous=true);

         int ReceiveTokenBuffer(dabc::Buffer& buf);

         /** for parallel readout mode: send request to all connected sfp chains in parallel.
          * If synchronous mode false, return before getting dma buffer,
          * needs to call ReceiveAllTokenBuffer afterwards
          * for synchronous mode true, fill one dabc buffer with subevents of different channels*/
         int RequestAllTokens(dabc::Buffer& buf, bool synchronous=true);


         int ReceiveAllTokenBuffer(dabc::Buffer& buf);




         virtual const char* ClassName() const { return "pexorplugin::Device"; }

         unsigned int GetDeviceNumber() { return fDeviceNum; }


         virtual int ExecuteCommand(dabc::Command cmd);


         virtual dabc::Transport* CreateTransport(dabc::Command cmd, const dabc::Reference& port);



         unsigned int GetReadLength(){return fReadLength;}

         bool IsSynchronousRead(){return fSynchronousRead;}

         //bool IsParallelRead(){return fParallelRead;}

         bool IsTriggeredRead(){return fTriggeredRead;}



         /** initialize trixor depending on the setup*/
         void InitTrixor();

         /** start data taking with trigger*/
         void StartTrigger();

         /** stop data taking with trigger*/
         void StopTrigger();

 /** Forwarded interface for user defined readout:
   * User code may overwrite the default behaviour (gosip token dma)
   * For example, optionally some register settings may be added to buffer contents*/
  virtual unsigned Read_Start (dabc::Buffer& buf);

  /** Forwarded interface for user defined readout:
   * User code may overwrite the default behaviour (gosip token dma)
   * For example, optionally some register settings may be added to buffer contents*/
  virtual unsigned Read_Complete (dabc::Buffer& buf);

      protected:
         virtual void ObjectCleanup();


         /** Insert mbs event header at location ptr in external buffer. Eventnumber will define event
          * sequence number, trigger marks current trigger type.
          * ptr is shifted to place after event header afterwards.
          * Return value is handle to event header structure
          * */
         mbs::EventHeader* PutMbsEventHeader(dabc::Pointer& ptr, mbs::EventNumType eventnumber, uint16_t trigger=mbs::tt_Event);

         /** Insert mbs subevent header at location ptr in external buffer. Id number subcrate, control nd procid
          * can be defined.ptr is shifted to place after subevent header afterwards.
          * Return value is handle to subevent header structure
          */
         mbs::SubeventHeader* PutMbsSubeventHeader(dabc::Pointer& ptr, int8_t subcrate, int8_t control, int16_t procid);



         /** copy contents of received dma buffer and optionally format for mbs*/
         int CopyOutputBuffer(pexor::DMA_Buffer* src, dabc::Buffer& dest);


         /** copy contents of received dma buffers src to destination buffer and optionally format for mbs*/
         int CombineTokenBuffers(pexor::DMA_Buffer** src, dabc::Buffer& dest);

         /** copy contents of dma token buffers src to subevent field pointed at by coursor; optionally format for mbs
          * returns increment of used size in target buffer. Use sfpnum as subevent identifier*/
         int CopySubevent(pexor::DMA_Buffer* src, dabc::Pointer& cursor, char sfpnum);


         /** switch sfp input index to next enabled one. returns false if no sfp is enabled in setup*/
         bool NextSFP();

//         /** fill token buffers of all slave devices with test event data*/
//         bool WriteTestBuffers();
//
//
//         /** random event functions stolen from TGo4MbsRandom code:*/
//         double  gauss_rnd(double mean, double sigma);
//         double  get_int(double low, double high);
//         unsigned long  Random_Event(int choice);


      protected:

       pexor::PexorTwo* fBoard;


      /** number X of pexor device (/dev/pexor-X) */
      unsigned int fDeviceNum;

      /** if true we put mbs headers already into transport buffer.
       * will contain subevents for each connected sfp*/
      bool fMbsFormat;

      /** For mbsformat: if true, use a single mbs subevent containing data of all sfps.
       * Subevent identifier (subcrate, procid, control) is configured by user via paramters.
       * Otherwise (default) buffer will contain one mbs subevent for each sfp, with sfpnumber labelling the
       * subcrate number.*/
      bool fSingleSubevent;

      /** For mbsformat: defines subevent subcrate id for case fSingleSubevent=true*/
      unsigned int fSubeventSubcrate;

      /** For mbsformat: defines subevent procid*/
      unsigned int fSubeventProcid;

      /** For mbsformat: defines subevent control*/
      unsigned int fSubeventControl;




//      /** fill token buffers of all slaves with generated test data*/
//      bool fTestData;

      /** if true, use synchrounous readout of token and dma. otherwise, decouple token request
       * from DMA buffer receiving*/
      bool fSynchronousRead;

      /** if true, use parallel token request. otherwise, request token in serial round robin sequence*/
//     bool fParallelRead;

      /** if true, request data only when trigger interrupt was received.
       * Otherwise request data immediately (polling mode)*/
      bool fTriggeredRead;

      /** flag to switch on memory speed measurements without acquisition*/
      bool fMemoryTest;

      /** switch to skip daq request*/
      bool fSkipRequest;

      /** zero copy DMA into dabc buffers*/
      bool fZeroCopyMode;


      /** array indicating which sfps are connected for readout*/
      bool fEnabledSFP[PEXORPLUGIN_NUMSFP];

      /** array indicating number of slaves in chain at each sfp*/
      unsigned int fNumSlavesSFP[PEXORPLUGIN_NUMSFP];

      /** id number of current exploder double buffer to request (0,1)*/
      int fDoubleBufID[PEXORPLUGIN_NUMSFP];

      /** index of currently read sfp. Used for the simple round robin readout into one transport*/
      unsigned char fCurrentSFP;

      /** actual payload length of read buffer*/
      unsigned int fReadLength;

//      /** size of each exploder submemory (byte). for test buffer set up*/
//      unsigned int fSubmemSize;

      /** trixor conversion time window (100ns units)*/
      unsigned short fTrixConvTime;

      /** trixor fast clear time window (100ns units)*/
      unsigned short fTrixFClearTime;

      /*** counter for transport threads, for unique naming.*/
      static unsigned int fgThreadnum;

      /** set true if initialization of board is successful*/
      bool fInitDone;

      /** Event number since device init*/
      unsigned int fNumEvents;


//      static double fgdPeak[];
//      static double fgdSigma[];
//
//      unsigned int fuSeed;



   };

} // namespace

#endif
