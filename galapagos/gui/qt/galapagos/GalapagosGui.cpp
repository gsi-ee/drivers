#include "GalapagosGui.h"

#include <stdlib.h>
#include <unistd.h>

#include <iostream>
//#include <QProcess>
//#include <stdlib.h>

//#include <QString>
//#include <QMessageBox>
//#include <QFileDialog>
//#include <QInputDialog>
//#include <QDateTime>
//#include <QTimer>
#include <QMdiSubWindow>

//#include <QFile>

#include <QDir>

//#include <okteta/piecetablebytearraymodel.h>

//#include <sstream>
//#include <string.h>
//#include <errno.h>
//#include <math.h>

// *********************************************************



#include "GalChannelWidget.h"
#include "GalSequenceWidget.h"
#include "GalPatternWidget.h"

/*
 *  Constructs a GalapagosGui which is a child of 'parent', with the
 *  name 'name'.'
 */
GalapagosGui::GalapagosGui (QWidget* parent) : BasicGui (parent)
{
  fSubWidgets.clear();

 fImplementationName="GALAPAGUI";
 fVersionString="Welcome to GalapaGUI!\n\t v0.17 of 30-Aug-2019 by JAM (j.adamczewski@gsi.de)";
 setWindowTitle(QString("%1").arg(fImplementationName));

 fSettings=new QSettings("GSI", fImplementationName);
 fLastFileDir = QDir::currentPath();

 BuildSetup();


  AddSubWindow(new GalChannelWidget(this));
  AddSubWindow(new GalSequenceWidget(this));
  AddSubWindow(new GalPatternWidget(this));


  ConnectSlots();
  ReadSettings();
  show ();
  GAPG_LOCK_SLOT
  GetRegisters();
  RefreshView();
  ClearOutputBtn_clicked(); // use version string of subclass
  GAPG_UNLOCK_SLOT
}


GalapagosGui::~GalapagosGui ()
{
}



void GalapagosGui::AddSubWindow(GalSubWidget* widget)
{
  Qt::WindowFlags wflags= Qt::CustomizeWindowHint | Qt::WindowMinMaxButtonsHint | Qt::WindowTitleHint;
  QMdiSubWindow* sub=mdiArea->addSubWindow(widget,wflags);
  sub->setAttribute(Qt::WA_DeleteOnClose, false);
  sub->setOption(QMdiSubWindow::RubberBandResize);
  sub->setOption(QMdiSubWindow::RubberBandMove); // JAM required for qt5 performance

  fSubWidgets.push_back(widget);
  widget->SetGalParent(this);
}



void GalapagosGui::ConnectSlots()
{
  for (std::vector<GalSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
    {
        (*it)->ConnectSlots();
    }
}



void GalapagosGui::ReadSettings()
{
  BasicGui::ReadSettings();
  theSetup_GET_FOR_CLASS(GalapagosSetup);
  if(fSettings)
    {
    for (std::vector<GalSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
       {
           (*it)->ReadSettings(fSettings);
       }
    }
}
void GalapagosGui::WriteSettings()
{
  BasicGui::WriteSettings();

  if(fSettings)
    {
    for (std::vector<GalSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
           {
               (*it)->WriteSettings(fSettings);
           }
    }
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
      for (int chan=0; chan<GAPG_CHANNELS; ++chan){
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

  for (std::vector<GalSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
    {
        (*it)->RefreshView();
    }

  BasicGui::RefreshView ();
}





void GalapagosGui::EvaluateView ()
{
  std::cout << "GalapagosGui::EvaluateView"<<std::endl;
  for (std::vector<GalSubWidget*>::iterator it = fSubWidgets.begin() ; it != fSubWidgets.end(); ++it)
  {
      (*it)->EvaluateView();
  }
}



void GalapagosGui::SetRegisters ()
{
  theSetup_GET_FOR_CLASS(GalapagosSetup);

  QApplication::setOverrideCursor (Qt::WaitCursor);

  for(uint8_t channel=0; channel<GAPG_CHANNELS;++channel)
  {
    WriteGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t),  theSetup->GetChannelSequenceID(channel));
  }
  /** channel enabled registers:*/
  WriteGAPG ( GAPG_CHANNEL_ENABLE_LOW, theSetup->GetChannelControl_0());
  WriteGAPG ( GAPG_CHANNEL_ENABLE_HI,  theSetup->GetChannelControl_1());

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
// read register values into structure with gosipcmd

 // return; //no readback from driver for the moment!

  theSetup_GET_FOR_CLASS(GalapagosSetup);
  QApplication::setOverrideCursor (Qt::WaitCursor);

  //std::cout << "GalapagosGui::GetRegisters()"<<std::endl;

  uint32_t status=ReadGAPG ( GAPG_MAIN_CONTROL);

  theSetup->SetGeneratorActive((status & GAPG_BIT_MAIN_ENABLE) == GAPG_BIT_MAIN_ENABLE);


  /** channel enabled registers:*/
    uint32_t chanlo= ReadGAPG ( GAPG_CHANNEL_ENABLE_LOW);
    uint32_t chanhi= ReadGAPG ( GAPG_CHANNEL_ENABLE_HI);

    theSetup->SetChannelControl_0(chanlo);
    theSetup->SetChannelControl_1(chanhi);


    for(uint8_t channel=0; channel<GAPG_CHANNELS;++channel)
     {
       uint32_t seqid=ReadGAPG ( GAPG_CHANNEL_SEQUENCE_BASE + channel*sizeof(uint32_t));
       if(!theSetup->SetChannelSequence(channel,seqid))
         {
           printm ("GetRegisters Warning- channel %d has unknown sequence id %d on hardware, fallback to id 1",channel,seqid);
           theSetup->SetChannelSequence(channel,1);
         }
     }

  
  QApplication::restoreOverrideCursor ();
}


BasicSetup* GalapagosGui::CreateSetup()
     {
        GalapagosSetup* setup=new GalapagosSetup();

        // here we mock up some default patterns that might be always available

        GalapagosSequence seq0(1,"SinglePulse");
        seq0.AddCommand("SINGLE PULSE 100;");
        seq0.Compile();
        setup->AddSequence(seq0);
        GalapagosSequence seq1(2,"DoublePulse");
        seq1.AddCommand("DOUBLE PULSE 100 500;");
        seq1.Compile();
        setup->AddSequence(seq1);
        GalapagosSequence seq2(3,"PulseSequenceNew");
        seq2.AddCommand("SEQUENCE PULSE 100 20 20000;");
        seq2.AddCommand("KEEP 0 200;");
        seq2.AddCommand("SEQUENCE PULSE 100 20 30000;");
        seq2.AddCommand("KEEP 0 100;");
        seq2.Compile();
        setup->AddSequence(seq2);


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




    

