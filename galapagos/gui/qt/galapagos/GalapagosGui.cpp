#include "GalapagosGui.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
#include <QMdiSubWindow>

#include <QDir>




#include "GalPackageWidget.h"
#include "GalKernelWidget.h"
#include "GalPatternWidget.h"
#include "GalPatternDisplay.h"

namespace gapg {

GalapagosGui::GalapagosGui (QWidget* parent) : gapg::BasicGui (parent)
{
  fSubWidgets.clear();

 fImplementationName="GALAPAGUI";
 fVersionString="Welcome to GalapaGUI!\n\t v0.50 of 8-Jan-2020 by JAM (j.adamczewski@gsi.de)";
 setWindowTitle(QString("%1").arg(fImplementationName));

 fSettings=new QSettings("GSI", fImplementationName);
 fLastFileDir = QDir::currentPath();

 BuildSetup();


  fPatternWidget = new GalPatternWidget(this);
  fKernelWidget = new GalKernelWidget(this);
  fPatternDisplay=new GalPatternDisplay(this);
  fKernelWidget->SetPatternDisplay(fPatternDisplay);
  fPatternWidget->SetPatternDisplay(fPatternDisplay);

  // JAM 2020: mind the order of subeditors, this is the order that ReadSettings will use
  // -> patterns must exist before kernels, kernels must exist before packages!
  AddSubWindow(fPatternWidget);
  AddSubWindow(fKernelWidget);
  AddSubWindow(fPatternDisplay);
  AddSubWindow(new GalPackageWidget(this));


  ConnectSlots();
  ReadSettings();
  show ();
  GAPG_LOCK_SLOT;
  GetRegisters();
  RefreshView();
  ClearOutputBtn_clicked(); // use version string of subclass
  GAPG_UNLOCK_SLOT;
}


GalapagosGui::~GalapagosGui ()
{
}







void GalapagosGui::ConnectSlots()
{
  BasicGui::ConnectSlots();
  // any extra slots here?


}



void GalapagosGui::ReadSettings()
{
  BasicGui::ReadSettings();
  // anything more that is not yet in subwindows here?

}
void GalapagosGui::WriteSettings()
{
  BasicGui::WriteSettings();
  // anything more that is not yet in subwindows here?
}







void GalapagosGui::EnableI2C ()
{
//  WriteGAPG ( GOS_I2C_DWR, 0x1000080);
//  WriteGAPG ( GOS_I2C_DWR, 0x2000020);
}

void GalapagosGui::DisableI2C ()
{
//  WriteGAPG ( GOS_I2C_DWR, 0x1000000);
}


void GalapagosGui::ResetSlave()
{

  sleep (1);
  printm("Did FAKE Initialize Galapagos Board");
}






void GalapagosGui::Dump()
{

  //  printm("SFP %d DEV:%d :)",fSFP, fSlave);
    for(int adc=0; adc<1; ++adc){
      for (int chan=0; chan<GAPG_CORES; ++chan){
        //int val=ReadADC_Galapagos(adc,chan);
        int val=adc*chan; // dummy
        if(val<0)
          printm("Read error for adc:%d chan:%d",adc,chan);
        else
          {
            if(fNumberBase==16)
              printm("Val (adc:0x%x chan:0x%x)=0x%x",adc,chan,val);
            else
              printm("Val (adc:%d chan:%d)=%d",adc,chan,val);
        }
      }

    }

}

void GalapagosGui::ApplyFileConfig(int )
{
    BasicGui::ApplyFileConfig(900); // adjust bus wait time to 900 us
}











void GalapagosGui::RefreshView ()
{

  //std::cout << "GalapagosGui::RefreshView"<<std::endl;
//  QString text;
//  QString pre;
//  fNumberBase == 16 ? pre = "0x" : pre = "";

  BasicGui::RefreshView ();
}





void GalapagosGui::EvaluateView ()
{
  //std::cout << "GalapagosGui::EvaluateView"<<std::endl;
  BasicGui::EvaluateView ();
}



void GalapagosGui::SetRegisters ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);

  QApplication::setOverrideCursor (Qt::WaitCursor);

//  for(uint8_t channel=0; channel<GAPG_CORES;++channel)
//  {
//    WriteGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t),  theSetup->GetChannelKernelID(channel));
//    WriteGAPG ( GAPG_CHANNEL_PATTERN_BASE + channel*sizeof(uint32_t),  theSetup->GetChannelPatternID(channel));
//  }
//  /** channel enabled registers:*/


    // before we have a running package, at least we mock up the enabled channels on hardware from current edited package:
  GalapagosPackage* pak=theSetup->GetKnownPackage(theSetup->GetCurrentPackageIndex());
  if(pak)
    {
        WriteGAPG ( GAPG_CHANNEL_ENABLE_LOW, pak->GetCoreControl_0());
        WriteGAPG ( GAPG_CHANNEL_ENABLE_HI,  pak->GetCoreControl_1());
    }

  /** possible master control register to start/stop processing*/
  uint32_t controlword=0;
  if(theSetup->IsGeneratorActive())
    controlword |= GAPG_BIT_MAIN_ENABLE;
  else
    controlword &= ~GAPG_BIT_MAIN_ENABLE;

  WriteGAPG ( GAPG_MAIN_CONTROL,  controlword);





  QApplication::restoreOverrideCursor ();

}

void GalapagosGui::GetRegisters ()
{
  // TODO: what do we want to see here once the package has been compiled and started?


 // return; //no readback from driver for the moment!

  theSetup_GET_FOR_CLASS(GalapagosSetup);
  QApplication::setOverrideCursor (Qt::WaitCursor);

  //std::cout << "GalapagosGui::GetRegisters()"<<std::endl;

  uint32_t status=ReadGAPG ( GAPG_MAIN_CONTROL);

  theSetup->SetGeneratorActive((status & GAPG_BIT_MAIN_ENABLE) == GAPG_BIT_MAIN_ENABLE);


  // active state ()

  /** channel enabled registers:*/
    uint32_t chanlo= ReadGAPG ( GAPG_CHANNEL_ENABLE_LOW);
    uint32_t chanhi= ReadGAPG ( GAPG_CHANNEL_ENABLE_HI);

    theSetup->SetCoreStatus_0(chanlo);
    theSetup->SetCoreStatus_1(chanhi);


//    for(uint8_t channel=0; channel<GAPG_CORES;++channel)
//     {
//       uint32_t seqid=ReadGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t));
//       if(!theSetup->SetChannelKernel(channel,seqid))
//         {
//           printm ("GetRegisters Warning- channel %d has unknown sequence id %d on hardware, fallback to id 1",channel,seqid);
//           theSetup->SetChannelKernel(channel,1);
//         }
//
//       uint32_t patid=ReadGAPG ( GAPG_CHANNEL_PATTERN_BASE + channel*sizeof(uint32_t));
//       if(!theSetup->SetChannelPattern(channel,patid))
//       {
//         printm ("GetRegisters Warning- channel %d has unknown pattern id %d on hardware, fallback to id 1",channel,patid);
//         theSetup->SetChannelPattern(channel,1);
//       }
//     }

  
  QApplication::restoreOverrideCursor ();
}


BasicSetup* GalapagosGui::CreateSetup()
     {
        GalapagosSetup* setup=new GalapagosSetup();

        // here we mock up some default patterns that might be always available

        GalapagosPackage pak0(1,"Default package");
        for(int i=0; i< GAPG_CORES; ++i)
          pak0.SetKernelID(i,2);
        setup->AddPackage(pak0);

        GalapagosPackage pak1(2,"TRB5");
        for(int i=0; i< GAPG_CORES; ++i)
          pak1.SetKernelID(i,1);
        setup->AddPackage(pak1);

        GalapagosKernel seq0(1,"SinglePulse");
        seq0.AddCommand("SINGLE PULSE 100;");
        seq0.Compile();
        seq0.SetPatternID(1);
        setup->AddKernel(seq0);
        GalapagosKernel seq1(2,"DoublePulse");
        seq1.AddCommand("DOUBLE PULSE 100 500;");
        seq1.Compile();
        seq1.SetPatternID(3);
        setup->AddKernel(seq1);
        GalapagosKernel seq2(3,"PulseKernelNew");
        seq2.AddCommand("SEQUENCE PULSE 100 20 20000;");
        seq2.AddCommand("KEEP 0 200;");
        seq2.AddCommand("SEQUENCE PULSE 100 20 30000;");
        seq2.AddCommand("KEEP 0 100;");
        seq2.Compile();
        seq2.SetPatternID(4);
        setup->AddKernel(seq2);


        GalapagosPattern pat0 (1, "Alternating10");
        for(int b=0;b<10;++b)
          pat0.AddByte(0xAA);
        setup->AddPattern(pat0);

        GalapagosPattern pat1 (2, "ByteSteps50");
        for(int b=0;b<50;++b)
          pat1.AddByte((b % 2)==0 ? 0xFF : 0x00);
        setup->AddPattern(pat1);

        GalapagosPattern pat2 (3, "WordSteps50");
        for(int b=0;b<50;++b)
          pat2.AddByte((b % 4)==0 ? 0xFF : 0x00);
        setup->AddPattern(pat2);

        GalapagosPattern pat3 (4, "Inc100");
        for(int b=0;b<100;++b)
        pat3.AddByte(b);
        setup->AddPattern(pat3);

      // std::cout <<"GalapagosGui:: CreateSetup" <<std::endl;
       return setup;
     }



}// namespace
    

