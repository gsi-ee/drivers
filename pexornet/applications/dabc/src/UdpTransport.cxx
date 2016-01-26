// $Id: UdpTransport.cxx 3336 2015-10-29 14:56:25Z linev $

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

#include "pexornet/UdpTransport.h"

#include <errno.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/syscall.h>


#include "dabc/timing.h"
#include "dabc/Manager.h"

#include "pexornet_user.h"



pexornet::DataSocketAddon::DataSocketAddon(int fd, int nport, int mtu, double flush, bool debug, int maxloop, double reduce) :
   dabc::SocketAddon(fd),
   dabc::DataInput(),
   fNPort(nport),
   fTgtPtr(),
   fWaitMoreData(false),
   fMTU(mtu > 0 ? mtu : DEFAULT_MTU),
   fFlushTimeout(flush),
   fSendCnt(0),
   fMaxLoopCnt(maxloop > 1 ? maxloop : 1),
   fReduce(reduce < 1. ? reduce : 1.),
   fTotalRecvPacket(0),
   fTotalDiscardPacket(0),
   fTotalDiscard32Packet(0),
   fTotalRecvBytes(0),
   fTotalDiscardBytes(0),
   fTotalProducedBuffers(0),
   fSubeventSubcrate(1),
   fSubeventProcid(2),
   fSubeventControl(3),
   fNumEvents(0),
   fDebug(debug)
{
   fPid = syscall(SYS_gettid);
}

pexornet::DataSocketAddon::~DataSocketAddon()
{
}

void pexornet::DataSocketAddon::ProcessEvent(const dabc::EventId& evnt)
{
   if (evnt.GetCode() == evntSocketRead) {
      // inside method if necessary SetDoingInput(true); should be done

      // ignore events when not waiting for the new data
      if (!fWaitMoreData) return;

      unsigned res = ReadUdp();

      if (res != dabc::di_CallBack) {
         fWaitMoreData = false;
         MakeCallback(res);
      }

      return;
   }

   dabc::SocketAddon::ProcessEvent(evnt);
}

long pexornet::DataSocketAddon::Notify(const std::string& msg, int arg)
{
   if (msg == "TransportWantToStop") {

      if (fWaitMoreData) {
         DOUT2("pexornet::DataSocketAddon notified, stop waiting data");
         fWaitMoreData = false;
         MakeCallback(dabc::di_Ok);
      }

      return 0;
   }

   return dabc::SocketAddon::Notify(msg, arg);
}


double pexornet::DataSocketAddon::ProcessTimeout(double lastdiff)
{
   if (!fWaitMoreData) return -1;

   if ((fTgtPtr.distance_to_ownbuf()>0) && (fSendCnt==0)) {
      fWaitMoreData = false;
      MakeCallback(dabc::di_Ok);
      return -1;
   }

   // check buffer with period of fFlushTimeout
   return fFlushTimeout;
}


unsigned pexornet::DataSocketAddon::ReadUdp()
{
   if (fTgtPtr.null()) {
      // if call was done from socket, just do nothing and wait buffer
      DOUT0("UDP:%d ReadUdp at wrong moment - no buffer to read", fNPort);
      return dabc::di_Error;
   }

   if (fTgtPtr.rawsize() < fMTU) {
      DOUT0("UDP:%d Should never happen - rest size is smaller than MTU", fNPort);
      return dabc::di_Error;
   }

   int cnt = fMaxLoopCnt;

   while (cnt-- > 0) {


     dabc::Pointer headerPtr=fTgtPtr; // remember begin of header
     dabc::Pointer payloadPtr=fTgtPtr;
      // JAM2016: before we receive, shift pointer to mbs subevent and header length - size of pexornet header:
     int headroom=sizeof(mbs::SubeventHeader) + sizeof(mbs::EventHeader) - sizeof(pexornet_data_header);
     payloadPtr.shift(headroom);
     ssize_t res = recv(Socket(), payloadPtr.ptr(), fMTU, 0); // receive complete udp packet
     int errsav=errno;


      if (res == 0) {
         DOUT0("UDP:%d Seems to be, socket was closed", fNPort);
         return dabc::di_EndOfStream;
      }

      if (res<0) {
         // socket do not have data, one should enable event processing
         // otherwise we need to poll for the new data
         if (errno == EAGAIN) break;
         EOUT("Socket error");
         return dabc::di_Error;
      }

      struct pexornet_data_header* pexhead  = (struct pexornet_data_header*) payloadPtr.ptr();
      unsigned int msgsize=pexhead->datalen;
      unsigned int trigtype=pexhead->trigger.typ;


      std::string errmsg;
      if ((unsigned) res != msgsize + sizeof(struct pexornet_data_header) )
        {
        errmsg=dabc::format("UDP:%d did not read complete payload length %d (read %d bytes), error:%d (%s), ignore it!\n", msgsize + sizeof(struct pexornet_data_header), res,
              errsav, strerror (errsav));
          return dabc::di_Error;
        }
      // TODO: do we need trailer to compare as in hadaq here?

      if (!errmsg.empty()) {
         DOUT3("UDP:%d %s", fNPort, errmsg.c_str());
         if (fDebug && (dabc::lgr()->GetDebugLevel()>2)) {
            errmsg = dabc::format("   Packet length %d", res);
            uint32_t* ptr = (uint32_t*) payloadPtr.ptr();
            for (unsigned n=0;n<res/4;n++) {
               if (n%8 == 0) {
                  printf("   %s\n", errmsg.c_str());
                  errmsg = dabc::format("0x%04x:", n*4);
               }

               errmsg.append(dabc::format(" 0x%08x", (unsigned) ptr[n]));
            }
            printf("   %s\n",errmsg.c_str());
         }

         fTotalDiscardPacket++;
         fTotalDiscardBytes+=res;
         continue;
      }

      fTotalRecvPacket++;
      fTotalRecvBytes += res;

      // now overwrite headroom with correct mbs headers:
      unsigned int filled_size = 0, used_size = 0;
      mbs::EventHeader* evhdr = PutMbsEventHeader (headerPtr, fNumEvents, trigtype);
      if (evhdr == 0)
        return dabc::di_SkipBuffer;    // buffer too small error
      used_size += sizeof(mbs::EventHeader);
      mbs::SubeventHeader* subhdr = PutMbsSubeventHeader (headerPtr, fSubeventSubcrate, fSubeventControl, fSubeventProcid);
      if (subhdr == 0)
        return dabc::di_SkipBuffer;    // buffer too small error
      used_size += sizeof(mbs::SubeventHeader) + msgsize;
      filled_size += sizeof(mbs::SubeventHeader) + msgsize;

      subhdr->SetRawDataSize (filled_size - sizeof(mbs::SubeventHeader));
      evhdr->SetSubEventsSize (filled_size);



      fTgtPtr.shift(used_size);

      // when rest size is smaller that mtu, one should close buffer
      if (fTgtPtr.rawsize() < fMTU)
         return dabc::di_Ok; // this is end
   }

   SetDoingInput(true);
   return dabc::di_CallBack; // indicate that buffer reading will be finished by callback
}


void pexornet::DataSocketAddon::MakeCallback(unsigned arg)
{
   dabc::InputTransport* tr = dynamic_cast<dabc::InputTransport*> (fWorker());

   if (tr==0) {
      EOUT("Did not found InputTransport on other side worker %p", fWorker());
      SubmitWorkerCmd(dabc::Command("CloseTransport"));
   } else {
      // DOUT0("Activate CallBack with arg %u", arg);
      tr->Read_CallBack(arg);
   }
}


unsigned pexornet::DataSocketAddon::Read_Start(dabc::Buffer& buf)
{
   if (!fTgtPtr.null() || fWaitMoreData) {
      EOUT("Read_Start at wrong moment");
      return dabc::di_Error;
   }

   unsigned bufsize = ((unsigned) (buf.SegmentSize(0) * fReduce)) /4 * 4;

   fTgtPtr.reset(buf, 0, bufsize);

   if (fTgtPtr.rawsize() < fMTU) {
      EOUT("not enough space in the buffer - at least %u is required", fMTU);
      return dabc::di_Error;
   }

   unsigned res = ReadUdp();

   fWaitMoreData = (res == dabc::di_CallBack);

   // we are waiting for event callback, configure else timeout
   if (fWaitMoreData) {
      fSendCnt = 0;
      ActivateTimeout(fFlushTimeout);
   }

   DOUT3("pexornet::DataSocketAddon::Read_Start buf %u res %u", buf.GetTotalSize(), res);

   return res;
}

unsigned pexornet::DataSocketAddon::Read_Complete(dabc::Buffer& buf)
{
   if (fTgtPtr.null()) return dabc::di_Ok;

   unsigned fill_sz = fTgtPtr.distance_to_ownbuf();
   fTgtPtr.reset();

   if (fill_sz==0) EOUT("Zero bytes was read");
   buf.SetTypeId(mbs::mbt_MbsEvents);

   buf.SetTotalSize(fill_sz);

   fSendCnt++;
   fTotalProducedBuffers++;

   DOUT3("pexornet::DataSocketAddon::Read_Complete buf %u", buf.GetTotalSize());

//   DOUT0("Receiver %d produce buffer of size %u", fNPort, buf.GetTotalSize());
   return dabc::di_Ok;
}

void pexornet::DataSocketAddon::ClearCounters()
{
   fTotalRecvPacket = 0;
   fTotalDiscardPacket = 0;
   fTotalDiscard32Packet = 0;
   fTotalRecvBytes = 0;
   fTotalDiscardBytes = 0;
   fTotalProducedBuffers = 0;
}

int pexornet::DataSocketAddon::OpenUdp(int nport, int rcvbuflen)
{
   int fd = socket(PF_INET, SOCK_DGRAM, 0);
   if (fd < 0) return -1;

   if (!dabc::SocketThread::SetNonBlockSocket(fd)) {
      EOUT("Cannot set non-blocking mode");
      close(fd);
      return -1;
   }

   sockaddr_in addr;
   memset(&addr, 0, sizeof(addr));
   addr.sin_family = AF_INET;
   addr.sin_port = htons(nport);

   if (rcvbuflen > 0) {
       // for pexornet application: set receive buffer length _before_ bind:
       //         int rcvBufLenReq = 1 * (1 << 20);
       int rcvBufLenRet;
       socklen_t rcvBufLenLen = sizeof(rcvbuflen);
       if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuflen, rcvBufLenLen) == -1) {
          EOUT("Fail to setsockopt SO_RCVBUF %s", strerror(errno));
       }

      if (getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvBufLenRet, &rcvBufLenLen) == -1) {
          EOUT("fail to getsockopt SO_RCVBUF, ...): %s", strerror(errno));
      }

      if (rcvBufLenRet < rcvbuflen) {
         EOUT("UDP receive buffer length (%d) smaller than requested buffer length (%d)", rcvBufLenRet, rcvbuflen);
         rcvbuflen = rcvBufLenRet;
      }
   }

   if (!bind(fd, (struct sockaddr *) &addr, sizeof(addr))) return fd;
   close(fd);
   return -1;
}


mbs::EventHeader* pexornet::DataSocketAddon::PutMbsEventHeader (dabc::Pointer& ptr, mbs::EventNumType eventnumber,
    uint16_t trigger)
{
  // check if header would exceed buffer length.
  if (ptr.rawsize () < sizeof(mbs::EventHeader))
  {
    DOUT0(
        "pexornet::DataSocketAddon::PutMbsEventHeader fails because no more space in buffer, restsize=%d bytes", ptr.rawsize());
    return 0;
  }

  mbs::EventHeader* evhdr = (mbs::EventHeader*) ptr ();
  evhdr->Init (eventnumber);
  // put here trigger type
  evhdr->iTrigger = trigger;
  //fBoard->GetTriggerType();

  ptr.shift (sizeof(mbs::EventHeader));
  return evhdr;
}

mbs::SubeventHeader* pexornet::DataSocketAddon::PutMbsSubeventHeader (dabc::Pointer& ptr, int8_t subcrate, int8_t control,
    int16_t procid)
{
  if (ptr.rawsize () < sizeof(mbs::SubeventHeader))
  {
    DOUT0(
        "pexornet::DataSocketAddon::PutMbsSubeventHeader fails because no more space in buffer, restsize=%d bytes", ptr.rawsize());
    return 0;
  }
  mbs::SubeventHeader* subhdr = (mbs::SubeventHeader*) ptr ();
  subhdr->Init ();
  subhdr->iProcId = procid;
  subhdr->iSubcrate = subcrate;
  subhdr->iControl = control;
  ptr.shift (sizeof(mbs::SubeventHeader));
  return subhdr;
}





// =================================================================================

pexornet::DataTransport::DataTransport(dabc::Command cmd, const dabc::PortRef& inpport, DataSocketAddon* addon) :
   dabc::InputTransport(cmd, inpport, addon, true),
   fIdNumber(0),
   fDataRateName()
{
   // do not process to much events at once, let another transports a chance
   SetPortLoopLength(OutputName(), 2);
   fIdNumber = inpport.ItemSubId();

   DOUT0("Starting pexornet::DataTransport %s id %d", GetName(), fIdNumber);
   std::string ratesprefix = inpport.GetName();
   fDataRateName = ratesprefix + "-Datarate";
   CreatePar(fDataRateName).SetRatemeter(false, 5.).SetUnits("MB");
   //Par(fDataRateName).SetDebugLevel(1);
   SetPortRatemeter(OutputName(), Par(fDataRateName));



}

pexornet::DataTransport::~DataTransport()
{

}






