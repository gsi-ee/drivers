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
#include "explodertest/Device.h"
#include "dabc/Command.h"
#include "dabc/Manager.h"
#include "dabc/Application.h"

#include "mbs/MbsTypeDefs.h"
#include "dabc/Pointer.h"
#include "dabc/Port.h"

#include "explodertest/Factory.h"

//#include "explodertest/ReadoutApplication.h"


#include "pexor/DMA_Buffer.h"
#include "pexor/PexorTwo.h"

#include "explodertest/random-coll.h"








const char* explodertest::xmlExploderSubmem	= "ExploderSubmemSize"; // size of exploder submem test buffer




double explodertest::Device::fgdPeak[NUM_PEAK]   = { 200., 400., 653., 1024., 2800.};
double explodertest::Device::fgdSigma[NUM_PEAK]  = {  10.,  5., 153.,  104.,   38.};







explodertest::Device::Device(const std::string& name, dabc::Command cmd):
pexorplugin::Device(name, cmd), fSubmemSize(3600), fuSeed(0)

{


   DOUT1("Created PEXOR device %d\n", fDeviceNum);
   fTestData=true; // TODO: configure this from XML once we have read data to fetch
   //fTestData=GetCfgBool(explodertest::xml????,20, cmd);

   fSubmemSize=Cfg(explodertest::xmlExploderSubmem,cmd).AsInt(3600);
   if(fTestData)
	   {
		   if(!WriteTestBuffers())
		   {
			   EOUT("\n\nError writing token test buffers to pexor device %d \n",fDeviceNum);
			    return;
		   }
		   DOUT1(("Wrote Test data to slaves."));
	   }


   fInitDone=true;
}

explodertest::Device::~Device()
{
}




int explodertest::Device::ExecuteCommand(dabc::Command cmd)
{
   DOUT1("explodertest::Device::ExecuteCommand-  %s", cmd.GetName());
   return pexorplugin::Device::ExecuteCommand(cmd);
}



unsigned explodertest::Device::Read_Start (dabc::Buffer& buf)
{
  return pexorplugin::Device::Read_Start (buf);
  // here it is possible to fill different values into buffer as composed from gosip token readout

}

unsigned explodertest::Device::Read_Complete (dabc::Buffer& buf)
{
  return pexorplugin::Device::Read_Complete (buf);
  // here it is possible to fill different values into buffer as composed from gosip token readout
}






bool  explodertest::Device::WriteTestBuffers()
{
	fuSeed=time(0);
	srand48(fuSeed);
	// loop over all connected sfps
	for(int ch=0; ch<PEXORPLUGIN_NUMSFP; ++ch)
	{
		if(!fEnabledSFP[ch]) continue;
		// loop over all slaves
		DOUT1("Writing test data for sfp %x ...\n", ch);
		for(unsigned int sl=0;sl<fNumSlavesSFP[ch];++sl)
		{
			int werrors=0;
			// get submemory addresses
			unsigned long base_dbuf0=0, base_dbuf1=0;
			unsigned long num_submem=0, submem_offset=0;
			unsigned long datadepth=fSubmemSize; // bytes per submemory

			int rev=fBoard->ReadBus(REG_BUF0, base_dbuf0, ch, sl);
			if(rev==0)
				{
				   DOUT1("Slave %x: Base address for Double Buffer 0  0x%x  \n", sl,base_dbuf0 );
				}
			else
				{
					EOUT("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 0 address)\n", rev, sl, REG_BUF0);
					return false;
				}
			rev=fBoard->ReadBus(REG_BUF1,base_dbuf1,ch,sl);
			if(rev==0)
				{
				   DOUT1("Slave %x: Base address for Double Buffer 1  0x%x  \n", sl,base_dbuf1 );
				}
			else
				{
				   EOUT("\n\ntoken Error %d in ReadBus: slave %x addr %x (double buffer 1 address)\n", rev, sl, REG_BUF1);
				   return false;
				}
			rev=fBoard->ReadBus(REG_SUBMEM_NUM,num_submem,ch,sl);
			if(rev==0)
				{
				   DOUT1("Slave %x: Number of SubMemories  0x%x  \n", sl,num_submem );
				}
		   else
			   {
				   EOUT("\n\ntoken Error %d in ReadBus: slave %x addr %x (num submem)\n", rev, sl, REG_SUBMEM_NUM);
				   return false;
			   }
		  rev=fBoard->ReadBus(REG_SUBMEM_OFF,submem_offset,ch,sl);
		  if(rev==0)
			  {
				   DOUT1("Slave %x: Offset of SubMemories to the Base address  0x%x  \n", sl,submem_offset );
			  }
		  else
			  {
				   EOUT("\n\ncheck_token Error %d in ReadBus: slave %x addr %x (submem offset)\n", rev, sl, REG_SUBMEM_OFF);
				   return false;
			  }
		rev=fBoard->WriteBus(REG_DATA_LEN,datadepth,ch,sl);
		if(rev)
				{
					 EOUT("\n\nError %d in WriteBus setting datadepth %d\n",rev,datadepth);
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
							EOUT("Error %d in WriteBus for submem %d of buffer 0, wordcount %d\n",rev,submem,i);
							werrors++;
							continue;
							//break;
						}

					rev= fBoard->WriteBus( submembase1 + i*4 , Random_Event(submem) , ch, sl);
					if(rev)
						{
							EOUT("Error %d in WriteBus for submem %d of buffer 1, wordcount %d\n",rev,submem,i);
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





double  explodertest::Device::gauss_rnd(double mean, double sigma)
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


double  explodertest::Device::get_int(double low, double high)
{
  return ((high-low)*drand48()+low);
   //return ((high-low)*gRandom->Rndm()+low);
}


unsigned long  explodertest::Device::Random_Event(int choice)
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



