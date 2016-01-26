// $Id: UdpTransport.h 3336 2015-10-29 14:56:25Z linev $

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

#ifndef PEXORNET_UDPTRANSPORT_H
#define PEXORNET_UDPTRANSPORT_H

#ifndef DABC_SocketThread
#include "dabc/SocketThread.h"
#endif

#ifndef DABC_DataIO
#include "dabc/DataIO.h"
#endif

 #ifndef DABC_Pointer
#include "dabc/Pointer.h"
#endif

#ifndef DABC_DataTransport
#include "dabc/DataTransport.h"
#endif

#include "mbs/MbsTypeDefs.h"
#include "pexornet_user.h"

#include <sched.h>

#define DEFAULT_MTU 63 * 1024

namespace pexornet {

   class DataTransport;

   /** \brief %Addon for socket thread to handle UDP data stream from TRB */

   class DataSocketAddon : public dabc::SocketAddon,
                           public dabc::DataInput {
      protected:

         friend class DataTransport;

         int                fNPort;           ///< upd port number
         dabc::Pointer      fTgtPtr;          ///< pointer used to read data
         bool               fWaitMoreData;    ///< indicate that transport waits for more data
         unsigned           fMTU;             ///< maximal size of packet expected from TRB
         double             fFlushTimeout;    ///< time when buffer will be flushed
         int                fSendCnt;         ///< counter of send buffers since last timeout active
         int                fMaxLoopCnt;      ///< maximal number of UDP packets, read at once
         double             fReduce;          ///< reduce filled buffer size to let reformat data later

         uint64_t           fTotalRecvPacket;
         uint64_t           fTotalDiscardPacket;
         uint64_t           fTotalLostPacket;
         uint64_t           fTotalRecvBytes;
         uint64_t           fTotalDiscardBytes;
         uint64_t           fTotalProducedBuffers;


         /** For mbsformat: defines subevent subcrate id for case fSingleSubevent=true*/
          unsigned int fSubeventSubcrate;

          /** For mbsformat: defines subevent procid*/
          unsigned int fSubeventProcid;

          /** For mbsformat: defines subevent control*/
          unsigned int fSubeventControl;

          /** for mbs format: sequence number since readout start*/
          unsigned int fNumEvents;

          /** for lost packets check: last trigger statujs with local event counter etc*/
          struct pexornet_trigger_status fLastTrigStat;

         pid_t fPid;                        ///< process id
         bool   fDebug;                     ///< when true, produce more debug output

         virtual void ProcessEvent(const dabc::EventId&);
         virtual double ProcessTimeout(double lastdiff);

         void MakeCallback(unsigned sz);

         /* Use codes which are valid for Read_Start */
         unsigned ReadUdp();

         virtual dabc::WorkerAddon* Read_GetAddon() { return this; }

         /** Light-weight command interface, which can be used from worker */
         virtual long Notify(const std::string&, int);

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




      public:
         DataSocketAddon(int fd, int nport, int mtu, double flush, bool debug, int maxloop, double reduce);
         virtual ~DataSocketAddon();

         // this is interface from DataInput
         virtual unsigned Read_Size() { return dabc::di_DfltBufSize; }
         virtual unsigned Read_Start(dabc::Buffer& buf);
         virtual unsigned Read_Complete(dabc::Buffer& buf);
         virtual double Read_Timeout() { return 0.1; }

         void ClearCounters();

         static int OpenUdp(int nport, int rcvbuflen);

         void SetMbsId(int crate, int procid, int control)
         {
           fSubeventSubcrate=crate;
           fSubeventProcid=procid;
           fSubeventControl=control;
         }

   };

   // ================================================================


   class DataTransport : public dabc::InputTransport {

      protected:

         int            fIdNumber;
         std::string    fDataRateName;


//         virtual void ProcessTimerEvent(unsigned timer);
//
//         virtual int ExecuteCommand(dabc::Command cmd);

      public:
         DataTransport(dabc::Command, const dabc::PortRef& inpport, DataSocketAddon* addon);
         virtual ~DataTransport();

   };

}

#endif
