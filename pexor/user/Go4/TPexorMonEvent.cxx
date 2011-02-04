// $Id: TPexorMonEvent.cxx 478 2009-10-29 12:26:09Z linev $
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

#include "TPexorMonEvent.h"

#include "Riostream.h"

//***********************************************************
TPexorMonEvent::TPexorMonEvent() :
   TGo4EventElement()
{
    cout << "**** TPexorMonEvent: Create instance" << endl;
}
//***********************************************************
TPexorMonEvent::TPexorMonEvent(const char* name) :
   TGo4EventElement(name)
{
  cout << "**** TPexorMonEvent: Create instance " << name << endl;
}
//***********************************************************
TPexorMonEvent::~TPexorMonEvent()
{
  cout << "**** TPexorMonEvent: Delete instance " << endl;
}

//-----------------------------------------------------------
void  TPexorMonEvent::Clear(Option_t *t)
{
	for(int sfp=0; sfp<PEXOR_SFP_NUM;++sfp)
	{
		for(int slave=0; slave<PEXOR_SLAVE_NUM;++slave)
			{
				for(int mem=0; mem<PEXOR_SUBMEM_NUM;++mem)
						{
							fSubmem[sfp][slave][mem].clear();
						}
			}
	}
}
