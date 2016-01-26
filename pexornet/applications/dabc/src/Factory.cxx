// $Id: Factory.cxx 3377 2015-11-13 12:11:38Z linev $

/************************************************************
 * The Data Acquisition Backbone Core (DABC)                *
 ************************************************************
 * Copyright (C) 2009 -                                     *
 * GSI Helmholtzzentrum fuer Schwerionenforschung GmbH      *
 * Planckstr. 1, 64291 Darmstadt, Germany                   *
 * Contact:  http://dabc.gsi.de                             *
 ************************************************************
 * This software can be used under the GPL license          *
 * agreements as stated in LICENSE.txt file                 *
 * which is part of the distribution.                       *
 ************************************************************/

#include "pexornet/Factory.h"

#include "dabc/string.h"
#include "dabc/logging.h"
#include "dabc/Command.h"
#include "dabc/Manager.h"
#include "dabc/SocketThread.h"
#include "dabc/MemoryPool.h"
#include "dabc/Configuration.h"
#include "dabc/Port.h"
#include "dabc/Url.h"

#include "pexornet/UdpTransport.h"
#include "pexornet/ReadoutModule.h"


dabc::FactoryPlugin pexornetfactory(new pexornet::Factory("pexornet"));






dabc::Module* pexornet::Factory::CreateTransport(const dabc::Reference& port, const std::string& typ, dabc::Command cmd)
{
   dabc::Url url(typ);

   dabc::PortRef portref = port;

   if (!portref.IsInput() || (url.GetProtocol()!="pexornet") || url.GetFullName().empty())
      return dabc::Factory::CreateTransport(port, typ, cmd);


   std::string portname = portref.GetName();



   int nport = url.GetPort();
   if (nport<=0) { EOUT("Port not specified"); return 0; }

   int rcvbuflen = url.GetOptionInt("udpbuf", 200000);
   int fd = DataSocketAddon::OpenUdp(nport, rcvbuflen);
   if (fd<=0) { EOUT("Cannot open UDP socket for port %d", nport); return 0; }

   DOUT0("Start UDP transport with port %d", nport);

   int mtu = url.GetOptionInt("mtu", 64512);
   int maxloop = url.GetOptionInt("maxloop", 100);
   double flush = url.GetOptionDouble(dabc::xml_flush, 1.);
   double reduce = url.GetOptionDouble("reduce", 1.);
   bool debug = url.HasOption("debug");
   int udp_queue = url.GetOptionInt("upd_queue", 0);



   // for mbs header format configuration:
   int subcrate = url.GetOptionInt("cratid", 0);
   int procid = url.GetOptionInt("procid", 1);
   int control = url.GetOptionInt("ctrlid", 7);

   DataSocketAddon* addon = new DataSocketAddon(fd, nport, mtu, flush, debug, maxloop, reduce);

   addon->SetMbsId(subcrate,procid,control);


   if (udp_queue>0) cmd.SetInt("TransportQueue", udp_queue);


   return new pexornet::DataTransport(cmd, portref, addon);
}


dabc::Module* pexornet::Factory::CreateModule(const std::string& classname, const std::string& modulename, dabc::Command cmd)
{

   if (classname == "pexornet::ReadoutModule")
      return new pexornet::ReadoutModule(modulename, cmd);


   return dabc::Factory::CreateModule(classname, modulename, cmd);
}


void pexornet::Factory::Initialize()
{


}
