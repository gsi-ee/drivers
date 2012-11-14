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

#include "mbs/MbsTypeDefs.h"
//#include "mbs/Factory.h"

//#include "dabc/MemoryPool.h"
//#include "dabc/Buffer.h"
#include "dabc/Pointer.h"
#include "dabc/Port.h"

#include "pexorplugin/Factory.h"
#include "pexorplugin/Transport.h"

#include "pexorplugin/ReadoutApplication.h"


#include "pexor/DMA_Buffer.h"


#include "pexorplugin/random-coll.h"






const char* pexorplugin::xmlPexorID    = "PexorID"; // id number N of pexor device file /dev/pexor-N
const char* pexorplugin::xmlPexorSFPSlaves	= "PexorNumSlaves_"; // prefix for the sfp numbers 0,1,2,3 indicating how many slaves are connected input
const char* pexorplugin::xmlRawFile    = "PexorOutFile"; // name of output lmd file
const char* pexorplugin::xmlDMABufLen	= "PexorDMALen"; // length of DMA buffers to allocate in driver
const char* pexorplugin::xmlDMABufNum	= "PexorDMABuffers"; // number of DMA buffers to allocate in driver
const char* pexorplugin::xmlDMAScatterGatherMode ="PexorDMAScatterGather"; // sg mode switch
const char* pexorplugin::xmlDMAZeroCopy ="PexorDMAZeroCopy"; // sg mode switch
const char* pexorplugin::xmlFormatMbs	= "PexorFormatMbs"; // switch Mbs formating already in transport buffer
const char* pexorplugin::xmlSyncRead	= "PexorSyncReadout"; // switch readout sync mode
const char* pexorplugin::xmlParallelRead	= "PexorParallelReadout"; // switch readout parallel token mode
const char* pexorplugin::xmlTriggeredRead        = "PexorUseTrigger"; // switch trigger mode
const char* pexorplugin::xmlTrixorConvTime      = "TrixorConversionTime"; // conversion time of TRIXOR module
const char* pexorplugin::xmlTrixorFastClearTime = "TrixorFastClearTime"; // fast clear time of TRIXOR module


const char* pexorplugin::xmlExploderSubmem	= "ExploderSubmemSize"; // size of exploder submem test buffer

const char* pexorplugin::xmlModuleName	= "PexorModuleName"; // Name of readout module instance
const char* pexorplugin::xmlModuleThread	= "PexorModuleThread"; // Name of thread for readout module
const char* pexorplugin::xmlDeviceName		= "PexorDeviceName";
const char* pexorplugin::xmlDeviceThread	= "PexorDeviceThread"; // Name of thread for readout module

const char* pexorplugin::nameReadoutAppClass   = "pexorplugin::ReadoutApplication";
const char* pexorplugin::nameDeviceClass   = "pexorplugin::Device";
const char* pexorplugin::nameTransportClass   = "pexorplugin::Transport";
const char* pexorplugin::nameReadoutModuleClass   = "pexorplugin::ReadoutModule";
const char* pexorplugin::nameInputPool   =    "PexorInputPool";
const char* pexorplugin::nameOutputPool   =    "PexorOutputPool";



double pexorplugin::Device::fgdPeak[NUM_PEAK]   = { 200., 400., 653., 1024., 2800.};
double pexorplugin::Device::fgdSigma[NUM_PEAK]  = {  10.,  5., 153.,  104.,   38.};




unsigned int pexorplugin::Device::fgThreadnum=0;

pexorplugin::Device::Device(const char* name, dabc::Command cmd) :
dabc::Device(name),fBoard(0),fMbsFormat(true), fTestData(true),
fSynchronousRead(true), fParallelRead(true), fTriggeredRead(false), fMemoryTest(false),fSkipRequest(false),
fCurrentSFP(0),fReadLength(0),fSubmemSize(3600),fTrixConvTime(0x20), fTrixFClearTime(0x10),
fInitDone(false),fNumEvents(0),fuSeed(0)

{
	fDeviceNum = Cfg(pexorplugin::xmlPexorID,cmd).AsInt(0);//GetCfgInt(pexorplugin::xmlPexorID, 0, cmd);
	fBoard=new pexor::PexorTwo(fDeviceNum);
	if(!fBoard->IsOpen())
			{
				EOUT(("**** Could not open pexor board!\n"));
				delete fBoard;
				return;
			}
	fZeroCopyMode=Cfg(pexorplugin::xmlDMAZeroCopy,cmd).AsBool(false); //GetCfgBool(pexorplugin::xmlDMAZeroCopy,false, cmd);
	DOUT1(("Setting zero copy mode to %d\n", fZeroCopyMode));
	bool sgmode=Cfg(pexorplugin::xmlDMAScatterGatherMode, cmd).AsBool(false); //GetCfgBool(pexorplugin::xmlDMAScatterGatherMode,false, cmd);
	fBoard->SetScatterGatherMode(sgmode);
	DOUT1(("Setting scatter gather mode to %d\n", sgmode));
	// initialize here the connected channels:

	 for (int sfp=0; sfp<PEXORPLUGIN_NUMSFP; sfp++)
		 {

			 fNumSlavesSFP[sfp]=Cfg(FORMAT(("%s%d",xmlPexorSFPSlaves, sfp)), cmd).AsInt(0);//GetCfgInt(FORMAT(("%s%d",xmlPexorSFPSlaves, sfp)), false,cmd);
			 fEnabledSFP[sfp] = fNumSlavesSFP[sfp]>0 ? true : false;
			 DOUT1(("Sfp %d is %s with %d slave devices.\n", sfp, (fEnabledSFP[sfp] ? "enabled" : "disabled"),  fNumSlavesSFP[sfp]));
			 if(fEnabledSFP[sfp])
				 {
					 int iret=fBoard->InitBus(sfp,fNumSlavesSFP[sfp]);
					 if(iret)
						 {
							 EOUT(("**** Error %d in PEXOR InitBus for sfp %d \n",iret,sfp));
							 delete fBoard;
							 return; // TODO: proper error handling
					 	  }
					 fDoubleBufID[sfp]=0;
				 }
		 }
	unsigned int size=Cfg(pexorplugin::xmlDMABufLen,cmd).AsInt(4096); //GetCfgInt(pexorplugin::xmlDMABufLen,4096, cmd);
    fReadLength=size; // initial value is maximum length of dma buffer
	//fReadLength=33000;
    if(fZeroCopyMode)
    {
    	// TODO: map here dabc memory pool completely to driver
    	// we do nothing here, all is done when ProcessPoolChanged is invoked

    }
    else
    {
		unsigned int numbufs=Cfg(pexorplugin::xmlDMABufNum,cmd).AsInt(20); //GetCfgInt(pexorplugin::xmlDMABufNum,20, cmd);
		int rev=fBoard->Add_DMA_Buffers(size,numbufs);
		if(rev)
			{
			 EOUT(("\n\nError %d on adding dma buffers\n",rev));
				return;
			}
    }
   fMbsFormat=Cfg(pexorplugin::xmlFormatMbs,cmd).AsBool(true);//GetCfgBool(pexorplugin::xmlFormatMbs,true, cmd);


   DOUT1(("Created PEXOR device %d\n", fDeviceNum));
   fTestData=true; // TODO: configure this from XML once we have read data to fetch
   //fTestData=GetCfgBool(pexorplugin::xml????,20, cmd);
   fSubmemSize=Cfg(pexorplugin::xmlExploderSubmem,cmd).AsInt(3600); //GetCfgInt(pexorplugin::xmlExploderSubmem,3600, cmd);
   if(fTestData)
	   {
		   if(!WriteTestBuffers())
		   {
			   EOUT(("\n\nError writing token test buffers to pexor device %d \n",fDeviceNum));
			    return;
		   }
		   DOUT1(("Wrote Test data to slaves."));
	   }

   fSynchronousRead=Cfg(pexorplugin::xmlSyncRead,cmd).AsBool(true); //GetCfgBool(pexorplugin::xmlSyncRead,true, cmd);

   fParallelRead=Cfg(pexorplugin::xmlParallelRead,cmd).AsBool(false); //GetCfgBool(pexorplugin::xmlParallelRead,false, cmd);

   fTriggeredRead=Cfg(pexorplugin::xmlTriggeredRead,cmd).AsBool(false); //GetCfgBool(pexorplugin::xmlTriggeredRead,false, cmd);

   fTrixConvTime=Cfg(pexorplugin::xmlTrixorConvTime,cmd).AsInt(0x200);//GetCfgInt(pexorplugin::xmlTrixorConvTime,0x200, cmd)
   fTrixFClearTime=Cfg(pexorplugin::xmlTrixorFastClearTime, cmd).AsInt(0x100);//GetCfgInt(pexorplugin::xmlTrixorFastClearTime,0x100, cmd);

   InitTrixor();

   // here the memory copy test switches:
   fMemoryTest=false; // put this to true to make memcopy performance test between driver buffer and dabc buffer
   fSkipRequest=false;

   fInitDone=true;
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
	 DOUT1(("SSSSSSS Starting MapDMAMemoryPool for pool:%s",pool->GetName()));
	// first clean up all previos buffers
	 fBoard->Reset(); // problematic when pool should change during DMA transfer?

	// then map dabc buffers to driver list:
	 unsigned numbufs = pool ? pool->GetNumBuffers() : 0;
	 DOUT3(("pexorplugin::Device::MapDMAMemoryPool transport map pools buffers blocks: %u", numblocks));
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
							 EOUT(("\n\nError registering buffer num:%d of pool:%s, addr:%p \n",bufid,pool->GetName(), addr));
							 continue;
					 }
	        	 // then tell the driver it should not use this dma buffer until we give it back:
	        	 pexor::DMA_Buffer* taken=0;
	        	 if((taken=fBoard->Take_DMA_Buffer(false))==0)
					{
						EOUT(("**** Could not take back DMA buffer %p for DABC!\n",addr));
						continue;
					}
	        	 if(taken->Data() != (int*) addr)
					 {
						 EOUT(("**** Mismatch of mapped DMA buffer %p and reserved buffer %p !\n",taken->Data(),addr));
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

  fBoard->StopAcquisition();
  // TODO: setters to disable irqs in non trigger mode
  fBoard->SetTriggerTimes(fTrixConvTime,fTrixFClearTime);
  fBoard->ResetTrigger();

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
   DOUT1(("_______pexorplugin::Device::ObjectCleanup..."));
   //if(fInitDone) fBoard->Reset();
   dabc::Device::ObjectCleanup();
}

int pexorplugin::Device::ExecuteCommand(dabc::Command cmd)
{
	if(!fInitDone) return dabc::cmd_false;
   int cmd_res = dabc::cmd_true;
//   if (cmd->IsName(DABC_PCI_COMMAND_SET_READ_REGION))
//      {
//          unsigned int bar=cmd->GetInt(DABC_PCI_COMPAR_BAR,1);
//          unsigned int address=cmd->GetInt(DABC_PCI_COMPAR_ADDRESS, (0x8000 >> 2));
//          unsigned int length=cmd->GetInt(DABC_PCI_COMPAR_SIZE, 1024);
//          SetReadBuffer(bar, address, length);
//          DOUT1(("Command %s  sets PCI READ region to bar:%d, address:%p, length:%d",DABC_PCI_COMMAND_SET_READ_REGION,bar,address,length));
//      }
//    else if (cmd->IsName(DABC_PCI_COMMAND_SET_WRITE_REGION))
//      {
//          unsigned int bar=cmd->GetInt(DABC_PCI_COMPAR_BAR,1);
//          unsigned int address=cmd->GetInt(DABC_PCI_COMPAR_ADDRESS, (0x8000 >> 2));
//          unsigned int length=cmd->GetInt(DABC_PCI_COMPAR_SIZE, 1024);
//          SetWriteBuffer(bar, address, length);
//          DOUT1(("Command %s  sets PCI WRITE region to bar:%d, address:%p, length:%d",DABC_PCI_COMMAND_SET_READ_REGION,bar,address,length));
//     }
//    else // TODO: reset board buffers here!


   DOUT1(("pexorplugin::Device::ExecuteCommand-  %s", cmd.GetName()));
     cmd_res = dabc::Device::ExecuteCommand(cmd);
return cmd_res;

}

dabc::Transport*  pexorplugin::Device::CreateTransport(dabc::Command cmd, dabc::Reference port)
//int pexorplugin::Device::CreateTransport(dabc::Command* cmd, dabc::Port* port)
{
   if(!fInitDone) return 0;
   pexorplugin::Transport* transport = new pexorplugin::Transport(this, port);
   DOUT1(("pexorplugin::Device::CreateTransport creates new transport instance %p", transport));
   DOUT3(("Device thread %p\n", thread().GetObject()));

   return transport;
}


int  pexorplugin::Device::RequestToken(dabc::Buffer& buf, bool synchronous)
{
	if(!fInitDone) return dabc::di_Error;
	DOUT3(("RequestToken is called"));

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

		DOUT3(("Device RequestToken uses headeroffset :%x, mbs event:0x%x, subevent:0x%x",headeroffset, sizeof(mbs::EventHeader), sizeof(mbs::SubeventHeader)));
		//
		// make buffer available for driver DMA:
		pexor::DMA_Buffer wrapper(bptr,buf.SegmentSize());
		if(fBoard->Free_DMA_Buffer(&wrapper))
		{
				EOUT(("**** Could not make buffer %p available for DMA!\n",bptr));
				return dabc::di_Error;
		}
	}


	// now request token from board at current sfp:
	pexor::DMA_Buffer* tokbuf= fBoard->RequestToken(fCurrentSFP, fDoubleBufID[fCurrentSFP], synchronous, bptr, headeroffset); // synchronous dma mode here
	if((long int) tokbuf==-1) // TODO: handle error situations by exceptions later!
		{
			EOUT(("**** Error in PEXOR Token Request from sfp %d !\n",fCurrentSFP));
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
				EOUT(("**** Error in PEXOR ReceiveTokenBuffer from sfp %d !\n",fCurrentSFP));
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
                    DOUT3(("pexorplugin::Device::RequestAllTokens gets dma buffer 0x%x for sfp:%d ", tokbuf[fCurrentSFP], fCurrentSFP));

                    if((long int) tokbuf[fCurrentSFP]==-1) // TODO: handle error situations by exceptions later!
                            {
                                    EOUT(("**** Error in PEXOR Token Request from sfp %d !\n",fCurrentSFP));
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
              EOUT(("**** Error in PEXOR ReceiveAllTokenBuffer from sfp %d !\n",sfp));
              return dabc::di_SkipBuffer;
            }
          oldbuflen = tokbuf[sfp]->UsedSize();
          DOUT3(("pexorplugin::Device::ReceiveAllTokenBuffer got token buffer of len %d\n", oldbuflen));
        }
      else
        {
          tokbuf[sfp] = fBoard->Take_DMA_Buffer(); // get empty buffer to emulate sync
          tokbuf[sfp]->SetUsedSize(oldbuflen); // set to length of real dma buffer
          DOUT3(("pexorplugin::Device::ReceiveAllTokenBuffer set dummy buffer len to %d\n", oldbuflen));
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

      mbs::EventHeader* evhdr = (mbs::EventHeader*) ptr();
      evhdr->Init(fNumEvents);
      ptr.shift(sizeof(mbs::EventHeader));
      used_size += sizeof(mbs::EventHeader);
      mbs::SubeventHeader* subhdr = (mbs::SubeventHeader*) ptr();
      subhdr->Init();
      subhdr->iProcId = fDeviceNum;
      subhdr->iSubcrate = fCurrentSFP;
      subhdr->iControl = 0;
      ptr.shift(sizeof(mbs::SubeventHeader));
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
   DOUT3(("Token buffer size:%d, used size%d, target buffer size:%d\n", tokbuf->Size(),tokbuf->UsedSize(), buf.GetTotalSize()));




   if (tokbuf->UsedSize() + used_size > buf.GetTotalSize())
    {
      EOUT(("Token buffer used size %d + header sizes %d exceed available target buffer length %d \n", tokbuf->UsedSize(),used_size, buf.GetTotalSize()));
      EOUT(("Mbs Event header size is %d;  Mbs subevent header sizes: %d \n", sizeof(mbs::EventHeader), sizeof(mbs::SubeventHeader)));
      EOUT(("Mbs event filled size  %d\n", filled_size));
      EOUT(("**** Error in PEXOR Token Request size, skip buffer!\n"));
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
  DOUT3(("pexorplugin::Device::CombineTokenBuffers initial pointer is 0x%x", ptr.ptr()));
  unsigned int filled_size = 0, used_size = 0;
  mbs::EventHeader* evhdr=0;
  if (fMbsFormat)
    {
      evhdr = (mbs::EventHeader*) ptr();
      evhdr->Init(fNumEvents);
      ptr.shift(sizeof(mbs::EventHeader));
      used_size += sizeof(mbs::EventHeader);
    }
  DOUT3(("pexorplugin::Device::CombineTokenBuffers output pointer after mbs header is 0x%x", ptr.ptr()));
  for (int sfp = 0; sfp < PEXORPLUGIN_NUMSFP; ++sfp)
        {
          if (src[sfp] == 0)
            continue;
          int increment = CopySubevent(src[sfp], ptr, sfp);
          if (increment < 0)
            continue; // TODO: some error handling here
          used_size += increment;
          filled_size += increment;
          DOUT3(("pexorplugin::Device::CombineTokenBuffers after sfp %d : used size:%d filled size:%d", (int) sfp, used_size, filled_size));
        }
  if (fMbsFormat)
      {
        evhdr->SetSubEventsSize(filled_size);
        buf.SetTypeId(mbs::mbt_MbsEvents);
      }
  buf.SetTotalSize(used_size);
  fNumEvents++;
  fReadLength = used_size; //adjust read length for next buffer to real token length
  //return -5; // debug
  return used_size;

}


int  pexorplugin::Device::CopySubevent(pexor::DMA_Buffer* tokbuf, dabc::Pointer& cursor, char sfpnum)
{
	unsigned int filled_size=0;
	DOUT3(("pexorplugin::Device::CopySubevent has dma buffer 0x%x for sfp %d, output cursor pointer :0x%x", tokbuf, (int) sfpnum, cursor.ptr()));
	if(fMbsFormat)
          {
                  mbs::SubeventHeader* subhdr = (mbs::SubeventHeader*) cursor();
                  subhdr->Init();
                  subhdr->iProcId =   fDeviceNum;
                  subhdr->iSubcrate = sfpnum;
                  subhdr->iControl = 0;
                  cursor.shift(sizeof(mbs::SubeventHeader));
                  filled_size+=sizeof(mbs::SubeventHeader);
                  // here account for zero copy alignment: headers+int give 32 bytes before payload
                  cursor.shift(sizeof(int));
                  filled_size += sizeof(int);
                  subhdr->SetRawDataSize(tokbuf->UsedSize()+sizeof(int));
          }
	cursor.copyfrom(tokbuf->Data(),tokbuf->UsedSize());
	cursor.shift(tokbuf->UsedSize()); // NOTE: you have to shift current pointer yourself after copyfrom!!
	DOUT3(("pexorplugin::Device::CopySubevent output cursor pointer after copyvfrom  and shift is:0x%x",cursor.ptr()));
	filled_size+=tokbuf->UsedSize();
	fBoard->Free_DMA_Buffer(tokbuf);
	DOUT3(("---------- token used size :%d", tokbuf->UsedSize()));
	DOUT3(("---------- filledsize :%d", filled_size));

	return filled_size;
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


bool  pexorplugin::Device::WriteTestBuffers()
{
	fuSeed=time(0);
	srand48(fuSeed);
	// loop over all connected sfps
	for(int ch=0; ch<PEXORPLUGIN_NUMSFP; ++ch)
	{
		if(!fEnabledSFP[ch]) continue;
		// loop over all slaves
		DOUT1(("Writing test data for sfp %x ...\n", ch));
		for(unsigned int sl=0;sl<fNumSlavesSFP[ch];++sl)
		{
			int werrors=0;
			// get submemory addresses
			unsigned long base_dbuf0=0, base_dbuf1=0;
			unsigned long num_submem=0, submem_offset=0;
			unsigned long datadepth=fSubmemSize; // bytes per submemory
			int rev=fBoard->ReadBus(REG_BUF0, base_dbuf0, ch,sl);
			if(rev==0)
				{
				   DOUT1(("Slave %x: Base address for Double Buffer 0  0x%x  \n", sl,base_dbuf0 ));
				}
			else
				{
					EOUT(("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 0 address)\n", rev, sl, REG_BUF0));
					return false;
				}
			rev=fBoard->ReadBus(REG_BUF1,base_dbuf1,ch,sl);
			if(rev==0)
				{
				   DOUT1(("Slave %x: Base address for Double Buffer 1  0x%x  \n", sl,base_dbuf1 ));
				}
			else
				{
				   EOUT(("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 1 address)\n", rev, sl, REG_BUF1));
				   return false;
				}
			rev=fBoard->ReadBus(REG_SUBMEM_NUM,num_submem,ch,sl);
			if(rev==0)
				{
				   DOUT1(("Slave %x: Number of SubMemories  0x%x  \n", sl,num_submem ));
				}
		   else
			   {
				   EOUT(("\n\ntoken Error %d in ReadBus: slave %x addr %x (num submem)\n", rev, sl, REG_SUBMEM_NUM));
				   return false;
			   }
		  rev=fBoard->ReadBus(REG_SUBMEM_OFF,submem_offset,ch,sl);
		  if(rev==0)
			  {
				   DOUT1(("Slave %x: Offset of SubMemories to the Base address  0x%x  \n", sl,submem_offset ));
			  }
		  else
			  {
				   EOUT(("\n\ncheck_token Error %d in ReadBus: slave %x addr %x (submem offset)\n", rev, sl, REG_SUBMEM_OFF));
				   return false;
			  }
		rev=fBoard->WriteBus(REG_DATA_LEN,datadepth,ch,sl);
		if(rev)
				{
					 EOUT(("\n\nError %d in WriteBus setting datadepth %d\n",rev,datadepth));
					 return false;
				}

		// write test data to submem
		for(unsigned int submem=0;submem<num_submem;++submem)
		 {
			 unsigned long submembase0=base_dbuf0+ submem*submem_offset;
			 unsigned long submembase1=base_dbuf1+ submem*submem_offset;
			 for(unsigned int i=0; i<datadepth/sizeof(int) ;++i)
				{
					int rev= fBoard->WriteBus( submembase0 + i*4 , Random_Event(submem) , ch, sl);
					if(rev)
						{
							EOUT(("Error %d in WriteBus for submem %d of buffer 0, wordcount %d\n",rev,submem,i));
							werrors++;
							continue;
							//break;
						}

					rev= fBoard->WriteBus( submembase1 + i*4 , Random_Event(submem) , ch, sl);
					if(rev)
						{
							EOUT(("Error %d in WriteBus for submem %d of buffer 1, wordcount %d\n",rev,submem,i));
							werrors++;
							continue;
							//break;
						}
				} // datadepth

		 } // submem
		printf("\nSlave %d has %d write errors on setting random event data.\n",sl,werrors);
	} // slaves

	} // channels
	return true;
}





double  pexorplugin::Device::gauss_rnd(double mean, double sigma)
{
  static int iset=0;
  static double gset;
  double v1, v2, s, u1;

  if(sigma < 0.)
    {
      v1 = drand48();
      return (log(1-v1)/sigma + mean);
    }
  else
    {
      if(iset == 0)
   {
     do
       {
         v1 = 2.*drand48()-1.;
         v2 = 2.*drand48()-1.;

         //v1 = 2.*gRandom->Rndm()-1.;
         //v2 = 2.*gRandom->Rndm()-1.;

         s = v1*v1+v2*v2;
       } while (s >= 1.0 || s == 0.0);

     u1 = sigma*v1*(sqrt(-2.*log(s)/s)) + mean;
     gset = u1;
     iset = 1;

     return (sigma*v2*(sqrt(-2.*log(s)/s)) + mean);
   }
      else
   {
     iset = 0;
     return gset;
   }
    }
}


double  pexorplugin::Device::get_int(double low, double high)
{
  return ((high-low)*drand48()+low);
   //return ((high-low)*gRandom->Rndm()+low);
}


unsigned long  pexorplugin::Device::Random_Event(int choice)
{

int cnt;
  switch(choice)
  {
     case 1:
        cnt = (int)(get_int(0., (double)NUM_PEAK));
        return ((unsigned long)(gauss_rnd(fgdPeak[cnt], fgdSigma[cnt])));
        break;
     case 2:
        cnt = (int)(get_int(0., (double)NUM_PEAK));
        return ((unsigned long)(p_dNormal(fgdPeak[cnt], fgdSigma[cnt], &fuSeed)));
        break;
     case 3:
        return ((unsigned long)(4096*p_dUniform(&fuSeed)));
        break;
     case 4:
        return ((unsigned long)(gauss_rnd(0., -.001)));
        break;
     case 5:
        return ((unsigned long)(p_dExponential(100., &fuSeed)));
        break;
     case 6:
        cnt = (int)(get_int(0., (double)NUM_PEAK));
        return ((unsigned long)((p_dExponential(200., &fuSeed)) + gauss_rnd(fgdPeak[cnt], fgdSigma[cnt])));
        break;
     case 7:
        cnt = (int)(get_int(3., (double)NUM_PEAK));
        return ((unsigned long)((4096*p_dUniform(&fuSeed)) + gauss_rnd(fgdPeak[cnt], fgdSigma[cnt])));
        break;

     default:
        return 0;
        break;
  }




   return 0;
}



