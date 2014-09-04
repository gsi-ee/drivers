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
#include "pexorplugin/Device.h"
#include "dabc/Command.h"
#include "dabc/Manager.h"
#include "dabc/Application.h"

//#include "mbs/MbsTypeDefs.h"
//#include "mbs/Factory.h"

//#include "dabc/MemoryPool.h"
//#include "dabc/Buffer.h"
#include "dabc/Pointer.h"
#include "dabc/Port.h"

#include "pexorplugin/Factory.h"
#include "pexorplugin/Transport.h"
#include "pexorplugin/Input.h"

//#include "pexorplugin/ReadoutApplication.h"


#include "pexor/DMA_Buffer.h"


//#include "pexorplugin/random-coll.h"






const char* pexorplugin::xmlPexorID    = "PexorID"; //< id number N of pexor device file /dev/pexor-N
const char* pexorplugin::xmlPexorSFPSlaves	= "PexorNumSlaves_"; //<  prefix for the sfp numbers 0,1,2,3 indicating how many slaves are connected input
const char* pexorplugin::xmlRawFile    = "PexorOutFile"; //<  name of output lmd file
const char* pexorplugin::xmlDMABufLen	= "PexorDMALen"; //<  length of DMA buffers to allocate in driver
const char* pexorplugin::xmlDMABufNum	= "PexorDMABuffers"; //<  number of DMA buffers to allocate in driver
const char* pexorplugin::xmlDMAScatterGatherMode ="PexorDMAScatterGather"; //<  sg mode switch
const char* pexorplugin::xmlDMAZeroCopy ="PexorDMAZeroCopy"; //<  sg mode switch
const char* pexorplugin::xmlFormatMbs	= "PexorFormatMbs"; //<  switch Mbs formating already in transport buffer
const char* pexorplugin::xmlSingleMbsSubevt   = "PexorSingleMbsSubevent"; //<  use one single subevent for all sfps
const char* pexorplugin::xmlMbsSubevtCrate   = "PexorMbsSubcrate"; //<  define crate number for subevent header
const char* pexorplugin::xmlMbsSubevtControl   = "PexorMbsControl"; //<  define crate number for subevent header
const char* pexorplugin::xmlMbsSubevtProcid   = "PexorMbsProcid"; //<  define procid number for subevent header


const char* pexorplugin::xmlSyncRead	= "PexorSyncReadout"; //<  switch readout sync mode
//onst char* pexorplugin::xmlParallelRead	= "PexorParallelReadout"; //<  switch readout parallel token mode
const char* pexorplugin::xmlTriggeredRead        = "PexorUseTrigger"; //<  switch trigger mode
const char* pexorplugin::xmlTrixorConvTime      = "TrixorConversionTime"; //<  conversion time of TRIXOR module
const char* pexorplugin::xmlTrixorFastClearTime = "TrixorFastClearTime"; //<  fast clear time of TRIXOR module




//const char* pexorplugin::xmlExploderSubmem	= "ExploderSubmemSize"; //<  size of exploder submem test buffer

const char* pexorplugin::xmlModuleName	= "PexorModuleName"; //<  Name of readout module instance
const char* pexorplugin::xmlModuleThread	= "PexorModuleThread"; //< Name of thread for readout module
const char* pexorplugin::xmlDeviceName		= "PexorDeviceName";
const char* pexorplugin::xmlDeviceThread	= "PexorDeviceThread"; //<  Name of thread for readout module

const char* pexorplugin::nameReadoutAppClass   = "pexorplugin::ReadoutApplication";
const char* pexorplugin::nameDeviceClass   = "pexorplugin::Device";
const char* pexorplugin::nameTransportClass   = "pexorplugin::Transport";
const char* pexorplugin::nameReadoutModuleClass   = "pexorplugin::ReadoutModule";
const char* pexorplugin::nameInputPool   =    "PexorInputPool";
const char* pexorplugin::nameOutputPool   =    "PexorOutputPool";


unsigned int pexorplugin::Device::fgThreadnum=0;



pexorplugin::Device::Device(const std::string& name, dabc::Command cmd):
dabc::Device(name), fBoard(0),fMbsFormat(true),
fSingleSubevent(false), fSubeventSubcrate(0), fSubeventProcid(0), fSubeventControl(0),
fSynchronousRead(true),
fTriggeredRead(false), fMemoryTest(false),fSkipRequest(false),
fCurrentSFP(0),fReadLength(0),
fTrixConvTime(0x20), fTrixFClearTime(0x10),
fInitDone(false),fNumEvents(0)

{
	fDeviceNum = Cfg(pexorplugin::xmlPexorID,cmd).AsInt(0);
	fBoard=new pexor::PexorTwo(fDeviceNum);
	if(!fBoard->IsOpen())
			{
				EOUT(("**** Could not open pexor board!\n"));
				delete fBoard;
				return;
			}
	fZeroCopyMode=Cfg(pexorplugin::xmlDMAZeroCopy,cmd).AsBool(false);
	DOUT1("Setting zero copy mode to %d\n", fZeroCopyMode);
	bool sgmode=Cfg(pexorplugin::xmlDMAScatterGatherMode, cmd).AsBool(false);
	fBoard->SetScatterGatherMode(sgmode);
	DOUT1("Setting scatter gather mode to %d\n", sgmode);
	// initialize here the connected channels:

	 for (int sfp=0; sfp<PEXORPLUGIN_NUMSFP; sfp++)
		 {

			 fNumSlavesSFP[sfp]=Cfg(dabc::format("%s%d",xmlPexorSFPSlaves, sfp), cmd).AsInt(0);
			 fEnabledSFP[sfp] = fNumSlavesSFP[sfp]>0 ? true : false;
			 DOUT1("Sfp %d is %s with %d slave devices.\n", sfp, (fEnabledSFP[sfp] ? "enabled" : "disabled"),  fNumSlavesSFP[sfp]);
			 if(fEnabledSFP[sfp])
				 {
					 int iret=fBoard->InitBus(sfp,fNumSlavesSFP[sfp]);
					 if(iret)
						 {
							 EOUT("**** Error %d in PEXOR InitBus for sfp %d \n",iret,sfp);
							 delete fBoard;
							 return; // TODO: proper error handling
					 	  }
					 fDoubleBufID[sfp]=0;
				 }
		 }
	unsigned int size=Cfg(pexorplugin::xmlDMABufLen,cmd).AsInt(4096);
    fReadLength=size; // initial value is maximum length of dma buffer
	//fReadLength=33000;
    if(fZeroCopyMode)
    {
    	// TODO: map here dabc memory pool completely to driver
    	// we do nothing here, all is done when ProcessPoolChanged is invoked
        // NOTE: zero copy not working anymore
    }
    else
    {
		unsigned int numbufs=Cfg(pexorplugin::xmlDMABufNum,cmd).AsInt(20); //GetCfgInt(pexorplugin::xmlDMABufNum,20, cmd);
		int rev=fBoard->Add_DMA_Buffers(size,numbufs);
		if(rev)
			{
			 EOUT("\n\nError %d on adding dma buffers\n",rev);
				return;
			}
    }
   fMbsFormat=Cfg(pexorplugin::xmlFormatMbs,cmd).AsBool(true);
   fSingleSubevent=Cfg(pexorplugin::xmlSingleMbsSubevt,cmd).AsBool(false);
   fSubeventSubcrate=Cfg(pexorplugin::xmlMbsSubevtCrate,cmd).AsInt(0);
   fSubeventProcid=Cfg(pexorplugin::xmlMbsSubevtProcid,cmd).AsInt(fDeviceNum);
   fSubeventControl=Cfg(pexorplugin::xmlMbsSubevtControl,cmd).AsInt(0);
   DOUT1("Setting mbsformat=%d, singlesubevent=%d, with subcrate:%d, procid:%d, control:%d\n",
       fMbsFormat,fSingleSubevent, fSubeventSubcrate,fSubeventProcid, fSubeventControl);
   DOUT1("Created PEXOR device %d\n", fDeviceNum);

   fSynchronousRead=Cfg(pexorplugin::xmlSyncRead,cmd).AsBool(true); //GetCfgBool(pexorplugin::xmlSyncRead,true, cmd);
   fTriggeredRead=Cfg(pexorplugin::xmlTriggeredRead,cmd).AsBool(false); //GetCfgBool(pexorplugin::xmlTriggeredRead,false, cmd);

   fTrixConvTime=Cfg(pexorplugin::xmlTrixorConvTime,cmd).AsInt(0x200);//GetCfgInt(pexorplugin::xmlTrixorConvTime,0x200, cmd)
   fTrixFClearTime=Cfg(pexorplugin::xmlTrixorFastClearTime, cmd).AsInt(0x100);//GetCfgInt(pexorplugin::xmlTrixorFastClearTime,0x100, cmd);

   InitTrixor();

   // here the memory copy test switches:
   fMemoryTest=false; // put this to true to make memcopy performance test between driver buffer and dabc buffer
   fSkipRequest=false;

   //fInitDone=true; // do this in subclass after constructor has finnished.
}

pexorplugin::Device::~Device()
{
    fBoard->Reset(); // throw away optionally user buffer mappings hee
	delete fBoard;
}


void pexorplugin::Device::MapDMAMemoryPool(dabc::MemoryPool* pool)
{
	if(!fZeroCopyMode) return;
	if(!fInitDone) return;
	 DOUT1("SSSSSSS Starting MapDMAMemoryPool for pool:%s",pool->GetName());
	// first clean up all previos buffers
	 fBoard->Reset(); // problematic when pool should change during DMA transfer?

	// then map dabc buffers to driver list:
	 unsigned numbufs = pool ? pool->GetNumBuffers() : 0;
	 DOUT3("pexorplugin::Device::MapDMAMemoryPool transport map pools buffers blocks: %u", numblocks);
	 for (unsigned bufid=0; bufid<numbufs; bufid++)
		 {
	         //if (!pool->IsMemoryBlock(blockid)) continue;
//	         dabc::BufferNum_t bufnum = pool->GetNumBuffersInBlock(blockid);
//	         for (dabc::BufferNum_t n=0; n < bufnum; n++)
//				 {
//	        	 dabc::BufferId_t bufid = dabc::CodeBufferId(blockid, n);
///////////////////////OLD ^
		 unsigned bufsize = pool->GetBufferSize(bufid);
	        	 void* addr = pool->GetBufferLocation(bufid);
	        	 // first we map the buffer for sglist and register to driver lists:
	        	 if(fBoard->Register_DMA_Buffer((int*) addr, bufsize))
					 {
							 EOUT("\n\nError registering buffer num:%d of pool:%s, addr:%p \n",bufid,pool->GetName(), addr);
							 continue;
					 }
	        	 // then tell the driver it should not use this dma buffer until we give it back:
	        	 pexor::DMA_Buffer* taken=0;
	        	 if((taken=fBoard->Take_DMA_Buffer(false))==0)
					{
						EOUT("**** Could not take back DMA buffer %p for DABC!\n",addr);
						continue;
					}
	        	 if(taken->Data() != (int*) addr)
					 {
						 EOUT("**** Mismatch of mapped DMA buffer %p and reserved buffer %p !\n",taken->Data(),addr);
						 delete taken;
						 continue;
					 }
	        	 delete taken; // clean up wrapper for driver internal sg lists, we do not use Board class mempool!
//////////////////
	        	 //				 }


		 } //blockid



}




void pexorplugin::Device::InitTrixor()
{
//
if(fTriggeredRead)
  {
     fBoard->StopAcquisition();
  // TODO: setters to disable irqs in non trigger mode
    fBoard->SetTriggerTimes(fTrixConvTime,fTrixFClearTime);
    fBoard->ResetTrigger();
  }
}

void pexorplugin::Device::StartTrigger()
{
  if(fTriggeredRead)
    fBoard->StartAcquisition();

}

void pexorplugin::Device::StopTrigger()
{
  if(fTriggeredRead)
    fBoard->StopAcquisition();

}



void pexorplugin::Device::ObjectCleanup()
{
   DOUT1("_______pexorplugin::Device::ObjectCleanup...");
   //if(fInitDone) fBoard->Reset();
   dabc::Device::ObjectCleanup();
}

int pexorplugin::Device::ExecuteCommand(dabc::Command cmd)
{
//   if (cmd->IsName(DABC_PCI_COMMAND_SET_READ_REGION) && fInitDone)
//      {
//          unsigned int bar=cmd->GetInt(DABC_PCI_COMPAR_BAR,1);
//          unsigned int address=cmd->GetInt(DABC_PCI_COMPAR_ADDRESS, (0x8000 >> 2));
//          unsigned int length=cmd->GetInt(DABC_PCI_COMPAR_SIZE, 1024);
//          SetReadBuffer(bar, address, length);
//          DOUT1(("Command %s  sets PCI READ region to bar:%d, address:%p, length:%d",DABC_PCI_COMMAND_SET_READ_REGION,bar,address,length));
//          return cmd_true;
//      }
//    else if (cmd->IsName(DABC_PCI_COMMAND_SET_WRITE_REGION) && fInitDone)
//      {
//          unsigned int bar=cmd->GetInt(DABC_PCI_COMPAR_BAR,1);
//          unsigned int address=cmd->GetInt(DABC_PCI_COMPAR_ADDRESS, (0x8000 >> 2));
//          unsigned int length=cmd->GetInt(DABC_PCI_COMPAR_SIZE, 1024);
//          SetWriteBuffer(bar, address, length);
//          DOUT1(("Command %s  sets PCI WRITE region to bar:%d, address:%p, length:%d",DABC_PCI_COMMAND_SET_READ_REGION,bar,address,length));
//          return cmd_true;
//     }
//    else // TODO: reset board buffers here!


   DOUT1("pexorplugin::Device::ExecuteCommand-  %s", cmd.GetName());

   return dabc::Device::ExecuteCommand(cmd);
}

dabc::Transport*  pexorplugin::Device::CreateTransport(dabc::Command cmd, const dabc::Reference& port)
{
   if(!fInitDone) return 0;
   //   dabc::Url url(typ);
   //
   dabc::PortRef portref = port;
   pexorplugin::Input* dinput= new pexorplugin::Input(this);

   DOUT0("~~~~~~~~~~~~~~~~~ pexorplugin::Device::CreateTransport port %s isinp %s", portref.ItemName().c_str(), DBOOL(portref.IsInput()));
   pexorplugin::Transport* transport = new pexorplugin::Transport(this, dinput, cmd, portref);
   DOUT1("pexorplugin::Device::CreateTransport creates new transport instance %p", transport);
   DOUT3(("Device thread %p\n", thread().GetObject()));
   return transport;
}


int  pexorplugin::Device::RequestToken(dabc::Buffer& buf, bool synchronous)
{
	if(!fInitDone) return dabc::di_Error;
	DOUT3("RequestToken is called");

	if(!NextSFP())
		{
			EOUT(("**** No SFP channel is enabled in configuration!\n"));
			return dabc::di_Error;
		}
	if(fTriggeredRead)
	  {
              // wait for trigger fired before fetching data:
              if(!fBoard->WaitForTrigger())
                {
                  // case of timeout, need dabc retry?
                  EOUT(("**** Timout of trigger, retry dabc request.. \n"));
                  return  dabc::di_RepeatTimeOut;
                }

	  }

	// new: decide if we have regular dma or zero copy:
	int* bptr=0;
	unsigned int headeroffset=0;
	if(fZeroCopyMode)
	{
		// get id and data offset
		bptr=(int*) buf.SegmentPtr();
		if(fMbsFormat)
			headeroffset=sizeof(mbs::EventHeader) + sizeof(mbs::SubeventHeader) + sizeof(int);

		DOUT3("Device RequestToken uses headeroffset :%x, mbs event:0x%x, subevent:0x%x",headeroffset, sizeof(mbs::EventHeader), sizeof(mbs::SubeventHeader));
		//
		// make buffer available for driver DMA:
		pexor::DMA_Buffer wrapper(bptr,buf.SegmentSize());
		if(fBoard->Free_DMA_Buffer(&wrapper))
		{
				EOUT("**** Could not make buffer %p available for DMA!\n",bptr);
				return dabc::di_Error;
		}
	}


	// now request token from board at current sfp:
	pexor::DMA_Buffer* tokbuf= fBoard->RequestToken(fCurrentSFP, fDoubleBufID[fCurrentSFP], synchronous, bptr, headeroffset); // synchronous dma mode here
	if((long int) tokbuf==-1) // TODO: handle error situations by exceptions later!
		{
			EOUT("**** Error in PEXOR Token Request from sfp %d !\n",fCurrentSFP);
			return  dabc::di_SkipBuffer;
		}
	fDoubleBufID[fCurrentSFP]= fDoubleBufID[fCurrentSFP]==0 ? 1 : 0;
	if(!synchronous) return dabc::di_Ok;
	
	if(fTriggeredRead)
	  fBoard->ResetTrigger();
	return (CopyOutputBuffer(tokbuf,buf));



}


int pexorplugin::Device::ReceiveTokenBuffer(dabc::Buffer& buf)
{
	// for asynchronous request, we need to put here again the check for zero copy dma:
	int* bptr=0;
	unsigned int headeroffset=0;
	if(fZeroCopyMode)
		{
			bptr=(int*) buf.SegmentPtr();
			if(fMbsFormat)
				headeroffset=sizeof(mbs::EventHeader) + sizeof(mbs::SubeventHeader)+ sizeof(int);
		}
	pexor::DMA_Buffer* tokbuf= fBoard->WaitForToken(fCurrentSFP,bptr,headeroffset);
	if(tokbuf==0)
		{
				EOUT("**** Error in PEXOR ReceiveTokenBuffer from sfp %d !\n",fCurrentSFP);
				return  dabc::di_SkipBuffer;
		}
	if(fTriggeredRead)
          fBoard->ResetTrigger();

	return (CopyOutputBuffer(tokbuf,buf));

}



int pexorplugin::Device::RequestAllTokens(dabc::Buffer& buf, bool synchronous)
{

	static pexor::DMA_Buffer* tokbuf[PEXORPLUGIN_NUMSFP];
	if(fTriggeredRead)
          {
              // wait for trigger fired before fetching data:
              if(!fBoard->WaitForTrigger())
                {
                  // case of timeout, need dabc retry?
                  EOUT(("**** Timout of trigger, retry dabc request.. \n"));
                  return  dabc::di_RepeatTimeOut;
                }
          }

	if((fMemoryTest && !fSkipRequest) || (!fMemoryTest))
	  {
            for( fCurrentSFP=0; fCurrentSFP < PEXORPLUGIN_NUMSFP; ++fCurrentSFP)
            {
                    tokbuf[fCurrentSFP]=0;
                    if(!fEnabledSFP[fCurrentSFP]) continue;
                    tokbuf[fCurrentSFP]= fBoard->RequestToken(fCurrentSFP, fDoubleBufID[fCurrentSFP], synchronous); // synchronous dma mode here
                    DOUT3("pexorplugin::Device::RequestAllTokens gets dma buffer 0x%x for sfp:%d ", tokbuf[fCurrentSFP], fCurrentSFP);

                    if((long int) tokbuf[fCurrentSFP]==-1) // TODO: handle error situations by exceptions later!
                            {
                                    EOUT("**** Error in PEXOR Token Request from sfp %d !\n",fCurrentSFP);
                                    return  dabc::di_SkipBuffer;
                            }
                    fDoubleBufID[fCurrentSFP]= fDoubleBufID[fCurrentSFP]==0 ? 1 : 0;
            }

	  }
	if(!synchronous) return dabc::di_Ok;
	fSkipRequest=true;
	return (CombineTokenBuffers(tokbuf,buf));

}


int pexorplugin::Device::ReceiveAllTokenBuffer(dabc::Buffer& buf)
{
  static pexor::DMA_Buffer* tokbuf[PEXORPLUGIN_NUMSFP];
  static int oldbuflen = 0;
  for (int sfp = 0; sfp < PEXORPLUGIN_NUMSFP; ++sfp)
    {
      tokbuf[sfp] = 0;
      if (!fEnabledSFP[sfp])
        continue;
      if ((fMemoryTest && !fSkipRequest) || !fMemoryTest)
        {
          tokbuf[sfp] = fBoard->WaitForToken(sfp);
          if (tokbuf[sfp] == 0)
            {
              EOUT("**** Error in PEXOR ReceiveAllTokenBuffer from sfp %d !\n",sfp);
              return dabc::di_SkipBuffer;
            }
          oldbuflen = tokbuf[sfp]->UsedSize();
          DOUT3("pexorplugin::Device::ReceiveAllTokenBuffer got token buffer of len %d\n", oldbuflen);
        }
      else
        {
          tokbuf[sfp] = fBoard->Take_DMA_Buffer(); // get empty buffer to emulate sync
          tokbuf[sfp]->SetUsedSize(oldbuflen); // set to length of real dma buffer
          DOUT3("pexorplugin::Device::ReceiveAllTokenBuffer set dummy buffer len to %d\n", oldbuflen);
        }
    }
  if(!fSkipRequest)
    fSkipRequest = true; // switch off all subsequent requests after the first

  if(fTriggeredRead)
        fBoard->ResetTrigger();

  return (CombineTokenBuffers(tokbuf, buf));
}


int  pexorplugin::Device::CopyOutputBuffer(pexor::DMA_Buffer* tokbuf, dabc::Buffer& buf)
{
    dabc::Pointer ptr(buf);
  unsigned int filled_size = 0, used_size = 0;
  if (fMbsFormat)
    {
        mbs::EventHeader* evhdr=PutMbsEventHeader(ptr, fNumEvents);
//      mbs::EventHeader* evhdr = (mbs::EventHeader*) ptr();
//      evhdr->Init(fNumEvents);
//      ptr.shift(sizeof(mbs::EventHeader));
      used_size += sizeof(mbs::EventHeader);
      mbs::SubeventHeader* subhdr=PutMbsSubeventHeader(ptr,(fSingleSubevent ? fSubeventSubcrate : fCurrentSFP), fSubeventControl, fSubeventProcid);


//      mbs::SubeventHeader* subhdr = (mbs::SubeventHeader*) ptr();
//      subhdr->Init();
//      subhdr->iProcId =   fSubeventProcid; //default: fDeviceNum;
//      subhdr->iSubcrate = (fSingleSubevent ? fSubeventSubcrate : fCurrentSFP);
//      subhdr->iControl = fSubeventControl;
      filled_size += sizeof(mbs::SubeventHeader);
      used_size += sizeof(mbs::SubeventHeader);

       // here account for zero copy alignment: headers+int give 32 bytes before payload
        ptr.shift(sizeof(int));
        filled_size += sizeof(int);
        used_size += sizeof(int);
      // UsedSize contains the real received token data length, as set by driver
      subhdr->SetRawDataSize(tokbuf->UsedSize()+sizeof(int));
      filled_size += tokbuf->UsedSize();
      evhdr->SetSubEventsSize(filled_size);
      buf.SetTypeId(mbs::mbt_MbsEvents);
    }
   DOUT2("Token buffer size:%d, used size%d, target buffer size:%d\n", tokbuf->Size(),tokbuf->UsedSize(), buf.GetTotalSize());




   if (tokbuf->UsedSize() + used_size > buf.GetTotalSize())
    {
      EOUT("Token buffer used size %d + header sizes %d exceed available target buffer length %d \n", tokbuf->UsedSize(),used_size, buf.GetTotalSize());
      EOUT("Mbs Event header size is %d;  Mbs subevent header sizes: %d \n", sizeof(mbs::EventHeader), sizeof(mbs::SubeventHeader));
      EOUT("Mbs event filled size  %d\n", filled_size);
      EOUT("**** Error in PEXOR Token Request size, skip buffer!\n");
      return dabc::di_SkipBuffer;
    }

  used_size += tokbuf->UsedSize();




  if(!fZeroCopyMode)
	  {
		  ptr.copyfrom(tokbuf->Data(), tokbuf->UsedSize());
		  fBoard->Free_DMA_Buffer(tokbuf); // put dma buffer back to free lists
	  }
  else
	  {
		  delete tokbuf; // for zero copy mode, this is just a temporary wrapper. Will be freed before request
	  }
  buf.SetTotalSize(used_size);
  fNumEvents++;
  fReadLength = used_size; //adjust read length for next buffer to real token length
  return used_size;

}


int  pexorplugin::Device::CombineTokenBuffers(pexor::DMA_Buffer** src, dabc::Buffer& buf)
{

  dabc::Pointer ptr(buf);
  DOUT3("pexorplugin::Device::CombineTokenBuffers initial pointer is 0x%x", ptr.ptr());
  unsigned int filled_size = 0, used_size = 0;
  mbs::EventHeader* evhdr=0;
  mbs::SubeventHeader* subhdr=0;
  if (fMbsFormat)
    {
//      evhdr = (mbs::EventHeader*) ptr();
//      evhdr->Init(fNumEvents);
//      ptr.shift(sizeof(mbs::EventHeader));
//
      evhdr=PutMbsEventHeader(ptr,fNumEvents); // TODO: get current trigger type from trixor and set
      used_size += sizeof(mbs::EventHeader);
      if(fSingleSubevent)
      {
          // one common subevent for data of all sfps:
          subhdr=PutMbsSubeventHeader(ptr, fSubeventSubcrate, fSubeventControl, fSubeventProcid);
          used_size   += sizeof(mbs::SubeventHeader);
          filled_size += sizeof(mbs::SubeventHeader);
          // here account for zero copy alignment: headers+int give 32 bytes before payload
          // necessary for the explodertest unpacker up to now
          ptr.shift(sizeof(int));
          used_size   += sizeof(int);
          filled_size += sizeof(int);
          // TODO: do we need some padding words here for mbs tailored unpacker?
          // TODO: can we switch this behaviour by parameter

      }
    }
  DOUT3("pexorplugin::Device::CombineTokenBuffers output pointer after mbs header is 0x%x", ptr.ptr());
  for (int sfp = 0; sfp < PEXORPLUGIN_NUMSFP; ++sfp)
        {
          if (src[sfp] == 0)
            continue;
          int increment = CopySubevent(src[sfp], ptr, sfp);
          if (increment < 0)
            continue; // TODO: some error handling here
          used_size += increment;
          filled_size += increment;
          DOUT3("pexorplugin::Device::CombineTokenBuffers after sfp %d : used size:%d filled size:%d", (int) sfp, used_size, filled_size);
        }
  if (fMbsFormat)
      {
        if(fSingleSubevent) subhdr->SetRawDataSize(filled_size - sizeof(mbs::SubeventHeader));
        evhdr->SetSubEventsSize(filled_size);
        buf.SetTypeId(mbs::mbt_MbsEvents);
      }
  buf.SetTotalSize(used_size);
  fNumEvents++;
  return used_size;

}


int  pexorplugin::Device::CopySubevent(pexor::DMA_Buffer* tokbuf, dabc::Pointer& cursor, char sfpnum)
{
	unsigned int filled_size=0;
	DOUT2("pexorplugin::Device::CopySubevent has dma buffer 0x%x for sfp %d, output cursor pointer :0x%x", tokbuf, (int) sfpnum, cursor.ptr());
	if(fMbsFormat && !fSingleSubevent)
          {
	             mbs::SubeventHeader* subhdr=PutMbsSubeventHeader(cursor, sfpnum, fSubeventControl, fSubeventProcid);



//                  mbs::SubeventHeader* subhdr = (mbs::SubeventHeader*) cursor();
//                  subhdr->Init();
//                  subhdr->iProcId =   fSubeventProcid; //default: fDeviceNum;
//                  subhdr->iSubcrate = (fSingleSubevent ? fSubeventSubcrate : sfpnum);
//                  subhdr->iControl = fSubeventControl;
//                  cursor.shift(sizeof(mbs::SubeventHeader));

	             filled_size+=sizeof(mbs::SubeventHeader);
                  // here account for zero copy alignment: headers+int give 32 bytes before payload
                  cursor.shift(sizeof(int));
                  filled_size += sizeof(int);
                  subhdr->SetRawDataSize(tokbuf->UsedSize()+sizeof(int));
          }
	cursor.copyfrom(tokbuf->Data(),tokbuf->UsedSize());
	cursor.shift(tokbuf->UsedSize()); // NOTE: you have to shift current pointer yourself after copyfrom!!
	DOUT3("pexorplugin::Device::CopySubevent output cursor pointer after copyfrom  and shift is:0x%x",cursor.ptr());
	filled_size+=tokbuf->UsedSize();
	fBoard->Free_DMA_Buffer(tokbuf);
	DOUT2("---------- token used size :%d", tokbuf->UsedSize());
	DOUT2("---------- filledsize :%d", filled_size);

	return filled_size;
}

mbs::EventHeader* pexorplugin::Device::PutMbsEventHeader(dabc::Pointer& ptr, mbs::EventNumType eventnumber, uint16_t trigger)
{
  mbs::EventHeader* evhdr= (mbs::EventHeader*) ptr();
  evhdr->Init(eventnumber);
  ptr.shift(sizeof(mbs::EventHeader));
  return evhdr;
}

mbs::SubeventHeader* pexorplugin::Device::PutMbsSubeventHeader(dabc::Pointer& ptr, int8_t subcrate, int8_t control, int16_t procid)
{
  mbs::SubeventHeader* subhdr = (mbs::SubeventHeader*) ptr();
  subhdr->Init();
  subhdr->iProcId =   procid;
  subhdr->iSubcrate = subcrate;
  subhdr->iControl = control;
  ptr.shift(sizeof(mbs::SubeventHeader));
  return subhdr;
}






bool pexorplugin::Device::NextSFP()
{
	int loopcounter=0;
	do{
		fCurrentSFP++;
		if(fCurrentSFP>=PEXORPLUGIN_NUMSFP) fCurrentSFP=0;
		if(loopcounter++>PEXORPLUGIN_NUMSFP) return false; // no sfp is enabled,error
	} while(!fEnabledSFP[fCurrentSFP]);
	return true;
}


unsigned pexorplugin::Device::Read_Start (dabc::Buffer& buf)
{
  if (IsTriggeredRead() || IsSynchronousRead())
     {
       return dabc::di_Ok; // synchronous mode, all handled in Read_Complete
     }
   else
     {
           return (unsigned) (RequestAllTokens(buf, false));
     }
}

unsigned pexorplugin::Device::Read_Complete (dabc::Buffer& buf)
{
  int res=dabc::di_Ok;
  // on trigger, we always read all sfp channels! so always "parallel" mode for dabc
     if (IsTriggeredRead() || IsSynchronousRead())
       {
         res = RequestAllTokens(buf, false); // for parallel read, we need async request before polling
         fReadLength = ReceiveAllTokenBuffer(buf);
       }
     else
       {
         fReadLength= ReceiveAllTokenBuffer(buf);
       }
   return (unsigned) res;
}








