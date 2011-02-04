// $Id: TPexorMonEvent.h 478 2009-10-29 12:26:09Z linev $
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

#ifndef TEVENT_H
#define TEVENT_H

#include "TGo4EventElement.h"
#include <vector>

#define PEXOR_SFP_NUM 4
#define PEXOR_SLAVE_NUM 16
#define PEXOR_SUBMEM_NUM 8


class TPexorMonEvent : public TGo4EventElement {
   public:
      TPexorMonEvent();
      TPexorMonEvent(const char* name);
      virtual ~TPexorMonEvent();

      /** Method called by the framework to clear the event element. */
      void Clear(Option_t *t="");

    /* store submem values */
	std::vector<Int_t> fSubmem[PEXOR_SFP_NUM][PEXOR_SLAVE_NUM][PEXOR_SUBMEM_NUM];
	



   ClassDef(TPexorMonEvent,1)
};
#endif //TEVENT_H



