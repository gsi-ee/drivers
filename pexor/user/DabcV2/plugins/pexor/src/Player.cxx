// $Id: Player.cxx 2688 2014-11-06 08:52:54Z adamczew $

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

#include "pexorplugin/Player.h"

#include "dabc/Publisher.h"

#include <stdio.h>


const char* pexorplugin::xmlHTML_Path = "HtmlPath";

pexorplugin::Player::Player(const std::string& name, dabc::Command cmd) :
   dabc::ModuleAsync(name, cmd)
{

  std::string hpath= Cfg (pexorplugin::xmlHTML_Path, cmd).AsStr("${DABCSYS}/plugins/pexor/htm/");
  DOUT0("pexorplugin::Player is using user file path: %s", hpath.c_str());
  fWorkerHierarchy.Create("PEX", true);

   dabc::Hierarchy ui = fWorkerHierarchy.CreateHChild("ControlGUI");
   ui.SetField(dabc::prop_kind, "DABC.HTML");
   ui.SetField("_UserFilePath", hpath.c_str());
   ui.SetField("_UserFileMain", "main.htm");

   CreateTimer("update", 5., false);
   
   PublishPars("PEXOR/GUI");
}

pexorplugin::Player::~Player()
{
}

void pexorplugin::Player::ProcessTimerEvent(unsigned timer)
{
   dabc::Hierarchy ui = fWorkerHierarchy.FindChild("ControlGUI");
   DOUT2("Process timer '%s'", ui.GetField("_UserFilePath").AsStr().c_str());
}


int pexorplugin::Player::ExecuteCommand(dabc::Command cmd)
{
//   if (cmd.IsName("CmdGosip")) {
//
//      int sfp = cmd.GetInt("sfp", 0);
//      int dev = cmd.GetInt("dev", 0);
//
//      bool log_output = cmd.GetInt("log") > 0;
//
//      std::string addr;
//      if ((sfp<0) || (dev<0))
//         addr = "-- -1 -1";
//      else
//         addr = dabc::format("%d %d", sfp, dev);
//
//      std::vector<std::string> gosipcmd = cmd.GetField("cmd").AsStrVect();
//
//      std::vector<std::string> gosipres;
//      std::vector<std::string> gosiplog;
//
//      DOUT1("*** CmdGosip len %u ****", gosipcmd.size());
//      for (unsigned n=0;n<gosipcmd.size();n++) {
//
//         std::string currcmd = gosipcmd[n];
//
//         bool isreading = (currcmd.find("-r")==0);
//         bool iswriting = (currcmd.find("-w")==0);
//
//         if (!isreading && !iswriting && (currcmd[0]!='-')) {
//            isreading = true;
//            currcmd = std::string("-r adr ") + currcmd;
//         }
//
//         size_t ppp = currcmd.find("adr");
//         if (ppp!=std::string::npos) currcmd.replace(ppp,3,addr);
//
//         currcmd = "gosipcmd " + currcmd;
//
//         if (log_output) gosiplog.push_back(currcmd);
//
//         // DOUT0("CMD %s", currcmd.c_str());
//
//         FILE* pipe = popen(currcmd.c_str(), "r");
//
//         if (!pipe) {
//            gosipres.push_back("<err>");
//            break;
//         }
//
//         char buf[50000];
//         memset(buf, 0, sizeof(buf));
//         int totalsize = 0;
//
//         while(!feof(pipe) && (totalsize<200000)) {
//            int size = (int)fread(buf,1, sizeof(buf)-1, pipe); //cout<<buffer<<" size="<<size<<endl;
//            if (size<=0) break;
//            while ((size>0) && ((buf[size-1]==' ') || (buf[size-1]=='\n'))) size--;
//            buf[size] = 0;
//            totalsize+=size;
//            if (log_output && (size>0)) gosiplog.push_back(buf);
//            // DOUT0("Get %s", buf);
//         }
//
//         pclose(pipe);
//
//         if (iswriting) {
//
//            // DOUT0("Writing res:%s len %d", buf, strlen(buf));
//
//            if (strlen(buf) > 2) {
//               gosipres.push_back("<err>");
//               break;
//            }
//
//            gosipres.push_back("<ok>");
//            continue;
//         }
//
//         if (isreading) {
//            long value = 0;
//            if (!dabc::str_to_lint(buf,&value)) {
//               gosipres.push_back("<err>");
//               break;
//            }
//            // DOUT0("Reading ok %ld", value);
//            gosipres.push_back(dabc::format("%ld", value));
//            continue;
//         }
//
//         // no idea that should be returned by other commands
//         gosipres.push_back("<undef>");
//      }
//
//      while (gosipres.size() < gosipcmd.size()) gosipres.push_back("<skip>");
//
//      cmd.SetField("res", gosipres);
//      if (log_output)
//         cmd.SetField("log", gosiplog);
//
//      DOUT1("*** CmdGosip finished ****");
//
//
//      return dabc::cmd_true;
//   }

   return dabc::ModuleAsync::ExecuteCommand(cmd);
}
