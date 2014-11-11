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

#include "pexor/PexorTwo.h"

#include "mbs/MbsTypeDefs.h"

/** number of connected sfps*/
#define PEXORPLUGIN_NUMSFP 4

namespace pexorplugin
{

extern const char* xmlPexorID;    //< id number N of pexor device file /dev/pexor-N
extern const char* xmlPexorSFPSlaves;    //< prefix for the sfp numbers 0,1,2,3 indicating number of slave devices connected
extern const char* xmlRawFile;    //< name of output lmd file
extern const char* xmlDMABufLen;    //< length of DMA buffers to allocate in driver
extern const char* xmlDMABufNum;    //< number of DMA buffers
extern const char* xmlDMAScatterGatherMode;    //< switch scatter gather dma on/off
extern const char* xmlDMAZeroCopy;    //< switch zero copy scatter gather dma on/off
extern const char* xmlExploderSubmem;    //< exploder submem size for testbuffer
extern const char* xmlFormatMbs;    //< enable mbs formating already in device transport
extern const char* xmlSingleMbsSubevt;    //<  use one single subevent for all sfps
extern const char* xmlMultichannelRequest;  //<  enable channelpattern request with combined dma for multiple sfps
extern const char* xmlAutoTriggerRead ; //<  enable automatic readout of all configured token data in driver for each trigger
extern const char* xmlMbsSubevtCrate;    //<  define crate number for subevent header
extern const char* xmlMbsSubevtControl;    //<  define crate number for subevent header
extern const char* xmlMbsSubevtProcid;    //<  define procid number for subevent header
extern const char* xmlSyncRead;    //< switch synchronous or asynchronous token dma
extern const char* xmlTriggeredRead;    //< switch triggered or polling mode readout
extern const char* xmlDmaMode;          //<  switch between direct dma to host,  or token data buffering in pexor RAM
extern const char* xmlWaitTimeout;    //<  specify kernel waitqueue timeout for trigger and autoread buffers

extern const char* xmlTrixorConvTime;    //< conversion time of TRIXOR module
extern const char* xmlTrixorFastClearTime;    //< fast clear time of TRIXOR module
extern const char* xmlModuleName;    //< Name of readout module instance
extern const char* xmlModuleThread;    //< Name of readout thread
extern const char* xmlDeviceName;    //< Name of device instance
extern const char* xmlDeviceThread;    //< Name of readout thread

extern const char* nameReadoutAppClass;
extern const char* nameDeviceClass;
extern const char* nameTransportClass;
extern const char* nameReadoutModuleClass;
extern const char* nameInputPool;
extern const char* nameOutputPool;

extern const char* commandStartAcq;
extern const char* commandStopAcq;
extern const char* commandInitAcq;

extern const char* parDeviceDRate;

class Device: public dabc::Device
{

public:

  Device (const std::string& name, dabc::Command cmd);
  virtual ~Device ();

  /** here we may insert some actions to the device cleanup methods*/
  virtual bool DestroyByOwnThread();

  /** for zero copy DMA: map complete dabc pool for sg DMA of driver
   * NOTE: currently not used for standard daq, deprecated function of previous tests with emulated sg dma.
   * For pexor direct token dma mode, sg emulation is not applicable!*/
  void MapDMAMemoryPool (dabc::MemoryPool* pool);

  /** Request token from current sfp. If synchronous is true, fill output buffer.
   * if mbs formating is enabled, put mbs headers into buffer
   * If synchronous mode false, return before getting dma buffer,
   * needs to call ReceivetokenBuffer afterwards.
   * NOTE: this method is not used for default daq case, kept for user convencience to be called
   * in optional reimplementation of ReadStart/ReadComplete interface*/
  virtual int RequestToken (dabc::Buffer& buf, bool synchronous = true);


  /** Request tokens from all enabled sfp by channelpattern. If synchronous is true, fill output buffer.
     * if mbs formating is enabled, put mbs headers into buffer
     * If synchronous mode false, return before getting dma buffer,
     * needs to call ReceivetokenBuffer afterwards.*/
   virtual int RequestMultiToken (dabc::Buffer& buf, bool synchronous = true, uint16_t trigtype=mbs::tt_Event);

  /** Receive token buffer of currently active sfp after asynchronous RequestToken call.
   * NOTE: this method is not used for default daq case, kept for user convencience to be called
   * in optional reimplementation of ReadStart/ReadComplete interface */
  int ReceiveTokenBuffer (dabc::Buffer& buf);

  /** for parallel readout mode: send request to all connected sfp chains in parallel.
   * If synchronous mode false, return before getting dma buffer,
   * needs to call ReceiveAllTokenBuffer afterwards
   * for synchronous mode true, fill one dabc buffer with subevents of different channels*/
  int RequestAllTokens (dabc::Buffer& buf, bool synchronous = true, uint16_t trigtype=mbs::tt_Event);


  /** Receive dma buffers from token request on all channels and copy to dabc buffer buf.
   * Optionally data is formatted with mbs event and subevent headers. MBS trigger type may be specified
   * depending on trixor trigger or triggerless readout.*/
  int ReceiveAllTokenBuffer (dabc::Buffer& buf, uint16_t trigtype=mbs::tt_Event);

  /** For automatic kernelmodule trigger readout mode: wait for next filled buffer.
   * Copy and format it to dabc buffer. Pass actual trigtype back to caller.*/
  int ReceiveAutoTriggerBuffer(dabc::Buffer& buf, uint8_t& trigtype);


  virtual const char* ClassName () const
  {
    return "pexorplugin::Device";
  }

  unsigned int GetDeviceNumber ()
  {
    return fDeviceNum;
  }

  virtual int ExecuteCommand (dabc::Command cmd);

  virtual dabc::Transport* CreateTransport (dabc::Command cmd, const dabc::Reference& port);

  unsigned int GetReadLength ()
  {
    return fReadLength;
  }

  bool IsSynchronousRead ()
  {
    return fSynchronousRead;
  }

  bool IsTriggeredRead ()
  {
    return fTriggeredRead;
  }
  bool IsAutoReadout ()
   {
     return fAutoTriggerRead;
   }
  bool IsMultichannelMode()
  {
    return fMultichannelRequest;
  }
  bool IsDirectDMA()
    {
      return fDirectDMA;
    }

  /** initialize trixor depending on the setup*/
  void InitTrixor ();

  /** start data taking with trigger*/
  bool StartAcquisition ();

  /** stop data taking with trigger*/
  bool StopAcquisition ();

  bool IsAcquisitionRunning()
  {
    return fAqcuisitionRunning;
  }


  /** generic initialization function for daq and frontends.
   * To be overwritten in subclass and callable by command interactively, without shutting down
   * application.*/
  virtual int InitDAQ();

  /** Forwarded interface for user defined readout:
   * User code may overwrite the default behaviour (gosip token dma)
   * For example, optionally some register settings may be added to buffer contents*/
  virtual unsigned Read_Start (dabc::Buffer& buf);

  /** Forwarded interface for user defined readout:
   * User code may overwrite the default behaviour (gosip token dma)
   * For example, optionally some register settings may be added to buffer contents*/
  virtual unsigned Read_Complete (dabc::Buffer& buf);


  /** interface for user subclass to implement different readout variants depending on the triggertype.
   * The default implementation will issue retry/timeout on start/stop acquisition trigger and
   * a standard token request with direct dma for all other trigger types*/
  virtual int User_Readout(dabc::Buffer& buf, uint8_t trigtype);




protected:
  virtual void ObjectCleanup ();

  /** Insert mbs event header at location ptr in external buffer. Eventnumber will define event
   * sequence number, trigger marks current trigger type.
   * ptr is shifted to place after event header afterwards.
   * Return value is handle to event header structure
   * */
  mbs::EventHeader* PutMbsEventHeader (dabc::Pointer& ptr, mbs::EventNumType eventnumber, uint16_t trigger =
      mbs::tt_Event);

  /** Insert mbs subevent header at location ptr in external buffer. Id number subcrate, control nd procid
   * can be defined.ptr is shifted to place after subevent header afterwards.
   * Return value is handle to subevent header structure
   */
  mbs::SubeventHeader* PutMbsSubeventHeader (dabc::Pointer& ptr, int8_t subcrate, int8_t control, int16_t procid);

  /** Insert num padding words at location of ptr and increment ptr.
   * Padding words are formatted in mbs convention like 0xaddNNII*/
  int PutMbsPaddingWords(dabc::Pointer& ptr, uint8_t num);

  /** copy contents of received dma buffer and optionally format for mbs.*/
  int CopyOutputBuffer (pexor::DMA_Buffer* src, dabc::Buffer& dest, uint16_t trigtype=mbs::tt_Event);

  /** copy contents of received dma buffers src to destination buffer and optionally format for mbs.
   * mbs style trigger type can be set for event header.*/
  int CombineTokenBuffers (pexor::DMA_Buffer** src, dabc::Buffer& dest, uint16_t trigtype=mbs::tt_Event);

  /** copy contents of dma token buffers src to subevent field pointed at by coursor; optionally format for mbs
   * returns increment of used size in target buffer. Use sfpnum as subevent identifier*/
  int CopySubevent (pexor::DMA_Buffer* src, dabc::Pointer& cursor, char sfpnum);

  /** switch sfp input index to next enabled one. returns false if no sfp is enabled in setup.
   * For round robin readout of single sfps. Not used for default triggered daq implementation.*/
  bool NextSFP ();


  void SetDevInfoParName(const std::string& name)
  {
    fDevInfoName = name;
  }


  void SetInfo(const std::string& info, bool forceinfo=true);

protected:

  pexor::PexorTwo* fBoard;

  /** number X of pexor device (/dev/pexor-X) */
  unsigned int fDeviceNum;

  /** Name of info parameter for device messages*/
  std::string fDevInfoName;

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

  /** wait timeout in seconds for kernel receive queues*/
  int fWaitTimeout;

  /** flag for aquisition running state*/
  bool fAqcuisitionRunning;

  /** if true, use synchrounous readout of token and dma. otherwise, decouple token request
   * from DMA buffer receiving*/
  bool fSynchronousRead;

  /** if true, request data only when trigger interrupt was received.
   * Otherwise request data immediately (polling mode)*/
  bool fTriggeredRead;

  /** mode how token data is put to host buffers:
   * if true, dma of each channel's token data will be written directly to receiving buffer
   * if false, token data will be stored in pexor memory first and then transferred to host buffers separately
   * this mode is evaluated in kernel module*/
  bool fDirectDMA;

  /** if true, data is requested by frontends with sfp channel pattern
   * and driver-intrinsic filling of dma buffer from all channels.
   * Otherwise, use sequential round-robin token request with separate dma buffers combined in application.
   * The latter may also separate different mbs subvevents for each sfp.*/
  bool fMultichannelRequest;

  /** if true, data readout will be done automatically in driver kernel module.
   * The already filled token buffer is fetched for each incoming trigger  */
  bool fAutoTriggerRead;


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

};

}    // namespace

#endif
