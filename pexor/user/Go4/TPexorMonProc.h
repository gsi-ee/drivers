// $Id: TPexorMonProc.h 605 2010-03-30 09:11:12Z linev $
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

#ifndef TUNPACKPROCESSOR_H
#define TUNPACKPROCESSOR_H

#include "TGo4EventProcessor.h"

#include "TPexorMonEvent.h"



class TPexorMonProc : public TGo4EventProcessor {
   public:
      TPexorMonProc() ;
      TPexorMonProc(const char* name);
      virtual ~TPexorMonProc() ;

      Bool_t BuildEvent(TGo4EventElement*); // event processing function

   private:
      TH1           *fHisSubmem[PEXOR_SFP_NUM][PEXOR_SLAVE_NUM][PEXOR_SUBMEM_NUM];

      TGo4Picture   *fOverview[PEXOR_SFP_NUM][PEXOR_SLAVE_NUM];


      static unsigned int fNumSlavesSFP[PEXOR_SFP_NUM];

   ClassDef(TPexorMonProc,1)
};

#endif //TUNPACKPROCESSOR_H
