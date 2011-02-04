// $Id: TPexorMonAnalysis.cxx 585 2010-02-19 18:34:20Z goofy $
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum für Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TPexorMonAnalysis.h"

#include <stdlib.h>
#include "Riostream.h"

#include "Go4EventServer.h"
#include "TGo4StepFactory.h"
#include "TGo4AnalysisStep.h"
#include "TGo4Version.h"

//***********************************************************
TPexorMonAnalysis::TPexorMonAnalysis() :
   fMbsEvent(0),
   fEvents(0),
   fLastEvent(0)
{
}
//***********************************************************

// this constructor is called by go4analysis executable
TPexorMonAnalysis::TPexorMonAnalysis(int argc, char** argv) :
   TGo4Analysis(argc, argv),
   fMbsEvent(0),
   fEvents(0),
   fLastEvent(0)
{
   cout << "**** Create TPexorMonAnalysis name: " << argv[0] << endl;

   if (!TGo4Version::CheckVersion(__GO4BUILDVERSION__)) {
      cout << "****  Go4 version mismatch" << endl;
      exit(-1);
   }

   TGo4StepFactory* factory = new TGo4StepFactory("Factory");
   factory->DefEventProcessor("PexorMonProc","TPexorMonProc");// object name, class name
   factory->DefOutputEvent("PexorMonEvent","TPexorMonEvent"); // object name, class name

   Text_t lmdfile[512]; // source file
   sprintf(lmdfile,"%s/data/test.lmd",getenv("GO4SYS"));
   // TGo4EventSourceParameter* sourcepar = new TGo4MbsTransportParameter("r3b");
   TGo4EventSourceParameter* sourcepar = new TGo4MbsFileParameter(lmdfile);

   TGo4FileStoreParameter* storepar = new TGo4FileStoreParameter(Form("%sOutput", argv[0]));
   storepar->SetOverwriteMode(kTRUE);

   TGo4AnalysisStep* step = new TGo4AnalysisStep("Analysis", factory, sourcepar, storepar);

   step->SetSourceEnabled(kTRUE);
   step->SetStoreEnabled(kFALSE);
   step->SetProcessEnabled(kTRUE);
   step->SetErrorStopEnabled(kTRUE);

   // Now the first analysis step is set up.
   // Other steps could be created here
   AddAnalysisStep(step);

   // uncomment following line to define custom passwords for analysis server
   // DefineServerPasswords("PexorMonadmin", "PexorMonctrl", "PexorMonview");

}

//***********************************************************
TPexorMonAnalysis::~TPexorMonAnalysis()
{
   cout << "**** TPexorMonAnalysis: Delete instance" << endl;
}

//-----------------------------------------------------------
Int_t TPexorMonAnalysis::UserPreLoop()
{
   // all this is optional:
   cout << "**** TPexorMonAnalysis: PreLoop" << endl;
   // get pointer to input event (used in postloop and event function):
   fMbsEvent = dynamic_cast<TGo4MbsEvent*> (GetInputEvent("Analysis"));   // of step "Analysis"
   if(fMbsEvent) {
      // fileheader structure (lmd file only):
      s_filhe* fileheader=fMbsEvent->GetMbsSourceHeader();
      if(fileheader)
      {
         cout <<"\nInput file: "<<fileheader->filhe_file << endl;
         cout <<"Tapelabel:\t" << fileheader->filhe_label<<endl;
         cout <<"UserName:\t" << fileheader->filhe_user<<endl;
         cout <<"RunID:\t" << fileheader->filhe_run<<endl;
         cout <<"Explanation: "<<fileheader->filhe_exp <<endl;
         cout <<"Comments: "<<endl;
         Int_t numlines=fileheader->filhe_lines;
         for(Int_t i=0; i<numlines;++i)
         {
            cout<<"\t"<<fileheader->s_strings[i].string << endl;
         }
      }
   }
   fEvents=0; // event counter
   fLastEvent=0; // number of last event processed
   return 0;
}
//-----------------------------------------------------------
Int_t TPexorMonAnalysis::UserPostLoop()
{
   // all this is optional:
   cout << "**** TPexorMonAnalysis: PostLoop" << endl;
   cout << "Last event  #: " << fLastEvent << " Total events: " << fEvents << endl;
   fMbsEvent = 0; // reset to avoid invalid pointer if analysis is changed in between
   fEvents=0;
   return 0;
}

//-----------------------------------------------------------
Int_t TPexorMonAnalysis::UserEventFunc()
{
   // all this is optional:
   // This function is called once for each event after all steps.
   if(fMbsEvent) {
      fEvents++;
      fLastEvent=fMbsEvent->GetCount();
   }
   if(fEvents == 1 || IsNewInputFile()) {
      cout << "First event #: " << fLastEvent  << endl;
      SetNewInputFile(kFALSE); // we have to reset the newfile flag
   }
   return 0;
}
