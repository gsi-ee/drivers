// $Id: TPexorMonProc.cxx 614 2010-04-13 15:39:33Z linev $
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

#include "TPexorMonProc.h"

#include <stdlib.h>
#include "Riostream.h"

#include "TH1.h"
#include "TH2.h"
#include "TCutG.h"
#include "TCanvas.h"
#include "TLine.h"

//#include "TGo4WinCond.h"
//#include "TGo4PolyCond.h"
//#include "TGo4CondArray.h"
#include "TGo4Picture.h"
#include "TGo4StepFactory.h"
#include "TGo4Analysis.h"
#include "TGo4Version.h"

unsigned int       TPexorMonProc::fNumSlavesSFP[]={4,2,2,2};


//***********************************************************
TPexorMonProc::TPexorMonProc() : TGo4EventProcessor()
{
}

//***********************************************************
TPexorMonProc::~TPexorMonProc()
{
   cout << "**** TPexorMonProc: Delete instance " << endl;
}

//***********************************************************
// this one is used in standard factory
TPexorMonProc::TPexorMonProc(const char* name) : TGo4EventProcessor(name)
{
   cout << "**** TPexorMonProc: Create instance " << name << endl;


   for(int sfp=0;sfp<PEXOR_SFP_NUM;sfp++)
	   {
	   if(TPexorMonProc::fNumSlavesSFP[sfp] > PEXOR_SLAVE_NUM)
			   TPexorMonProc::fNumSlavesSFP[sfp] = PEXOR_SLAVE_NUM;

	   for(unsigned int slave=0;slave<TPexorMonProc::fNumSlavesSFP[sfp];slave++)
	   {
		   for(int submem=0;submem<PEXOR_SUBMEM_NUM;submem++)
		   {
			   fHisSubmem[sfp][slave][submem]=MakeTH1('I', Form("SFP%d/Slave%02d/Submem%d_%d_%d",sfp,slave,sfp,slave,submem), Form("SFP %d Slave %d Submem %2d",sfp,slave,submem), 5000, 0, 5000);
		   }
//		   fOverview[sfp][slave]=GetPicture(picname.Data());
//		   if(fOverview[sfp][slave]==0)
//			   {
//			   // TODO: picture set up
//
//			   }

	   }
   }





}

//-----------------------------------------------------------
// event function
Bool_t TPexorMonProc::BuildEvent(TGo4EventElement* target)
{  // called by framework. We dont fill any output event here at all

   if ((GetInputEvent()==0) || (GetInputEvent()->IsA() != TGo4MbsEvent::Class())) {
      cout << "TPexorMonProc: no input MBS event found!" << endl;
      return kFALSE;
   }

   TGo4MbsEvent* evnt = (TGo4MbsEvent*) GetInputEvent();

   TPexorMonEvent* outevent=dynamic_cast<TPexorMonEvent*>(target);
   if(outevent==0)
   {
	   cout << "TPexorMonProc: wrong output event type!" << endl;
	   return kFALSE;
   }

//   if(evnt->GetTrigger() > 11) {
//      cout << "**** TPexorMonProc: Skip trigger event" << endl;
//      return kFALSE;
//   }




   evnt->ResetIterator();
   TGo4MbsSubEvent *psubevt(0);
   while((psubevt = evnt->NextSubEvent()) != 0) { // loop over subevents
      Int_t * pdata = psubevt->GetDataField();
      Int_t lwords = psubevt->GetIntLen();
      Int_t sfp=psubevt->GetSubcrate();
      if(sfp<0 || sfp>=PEXOR_SFP_NUM)
		  {
			  cout << "**** TPexorMonProc: Found invalid sfp number, skip event" << endl;
			  return kFALSE;

		  }


      // scan data for each submem and fill:
      UInt_t slave=0;
      bool isheaderread=false,isdsizeread=false;
      Int_t datasize=0,datawordsize=0;;
      Int_t submem=0;
      Int_t datacount=0;
      for( Int_t i=0; i<lwords; ++i)
      {
    	  Int_t currentdata=*(pdata++);
    	  if(!isheaderread)
				{
    		        Int_t hlength=0, dlength=0,trigid=0, modid=0,memid=0;
    		        hlength=((currentdata )  & 0xf0) >> 4; // just for check should be 3(byte)
    		        dlength=((currentdata )  & 0x0f); // should be 4 (byte)
    		        trigid = ((currentdata >> 8 ) & 0xff);
    		        modid =  ((currentdata >> 16 ) & 0xff);
    		        memid =  ((currentdata >> 24 ) & 0xff);
    		        //printf("Token header: hlen:0x%x dlen:0x%x trigid:0x%x modid:0x%x memid:0x%x \n",hlength,dlength,trigid,modid,memid);
    		        if(hlength!=3 ||  dlength!=4)
    		  		{
    		  			printf("Invalid token header data, assume we reached end of token, stop it!\n");
					
					evnt->Print();
					printf("hlen:%d dlen:%d trigid:%d modid:%d memid:%d \n",hlength,dlength,trigid,modid,memid);
					GO4_STOP_ANALYSIS;
    		  			return kTRUE;
    		  			//break;
    		  		}
    		        submem=memid;
    		        if(submem>=PEXOR_SUBMEM_NUM)
						{
							 printf("Token header: Error found submem 0x%x out of range:0x%x \n",submem,PEXOR_SUBMEM_NUM);
							 submem=0;
						}

    		        slave=modid;
    		        if(slave>=TPexorMonProc::fNumSlavesSFP[sfp])
						{
							 printf("Token header: Error found slave 0x%x out of range:0x%x \n",slave,TPexorMonProc::fNumSlavesSFP[sfp]);
							 slave=0;
						}
    		        isheaderread=true;
				}

    	  else if(!isdsizeread)
				{
					// read data size of next submem chunk
					datasize=currentdata;
					//printf("Reading datasize:0x%x bytes... \n",datasize);
					if(datasize==0)
					{
						printf("Zero datasize, assume we reached end of token.\n");
						break;
					}
					datawordsize=datasize/sizeof(int);
					isdsizeread=true;
				}
			else
				{

					if(datacount < datawordsize)
						{
							datacount++;
							// fill data into corresponding histogram:
							if(fHisSubmem[sfp][slave][submem]!=0)
								fHisSubmem[sfp][slave][submem]->Fill(currentdata);

							outevent->fSubmem[sfp][slave][submem].push_back(currentdata); // copy values to output event

						}
					else
						{
							//printf("Reached end of block at datacount 0x%x words... \n",datacount);
							--i; // repeat this word index for header scan
							--pdata;
							isheaderread=false; // get next header
							isdsizeread=false; // get next datasize
							datacount=0;
						}


				}


      } // for



   }// while subevents
   return kTRUE;
}
